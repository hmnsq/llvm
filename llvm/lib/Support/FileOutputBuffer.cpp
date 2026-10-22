//===- FileOutputBuffer.cpp - File Output Buffer ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Utility for creating a in-memory buffer that will be written to a file.
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/FileOutputBuffer.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/Memory.h"
#অন্তর্ভাব "llvm/Support/TimeProfiler.h"
#অন্তর্ভাব <system_error>

#যদি !defined(_MSC_VER) && !defined(__MINGW32__)
#অন্তর্ভাব <unistd.h>
#অন্যথা
#অন্তর্ভাব <io.h>
#endif

using নামাকাশ llvm;
using নামাকাশ llvm::sys;

নামাকাশ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
বর্গ OnDiskBuffer : সার্বজনিক FileOutputBuffer {
সার্বজনিক:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() ধ্রুব override { প্রত্যাগম (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() ধ্রুব override {
    প্রত্যাগম (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() ধ্রুব override { প্রত্যাগম Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    প্রত্যাগম Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  শূন্য discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

গুপ্ত:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
বর্গ InMemoryBuffer : সার্বজনিক FileOutputBuffer {
সার্বজনিক:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() ধ্রুব override { প্রত্যাগম (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() ধ্রুব override {
    প্রত্যাগম (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() ধ্রুব override { প্রত্যাগম BufferSize; }

  Error commit() override {
    যদি (FinalPath == "-") {
      llvm::outs() << StringRef((ধ্রুব বর্ণ *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      প্রত্যাগম Error::success();
    }

    using নামাকাশ sys::fs;
    পূর্ণাঙ্ক FD;
    std::error_code EC;
    যদি (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      প্রত্যাগম errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/সত্য, /*unbuffered=*/সত্য);
    OS << StringRef((ধ্রুব বর্ণ *)Buffer.base(), BufferSize);
    প্রত্যাগম Error::success();
  }

গুপ্ত:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

স্থির Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  যদি (EC)
    প্রত্যাগম errorCodeToError(EC);
  প্রত্যাগম std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

স্থির Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  যদি (!FileOrErr)
    প্রত্যাগম FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  যদি (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    প্রত্যাগম errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  যদি (EC) {
    consumeError(File.discard());
    প্রত্যাগম createInMemoryBuffer(Path, Size, Mode);
  }

  প্রত্যাগম std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  যদি (Path == "-")
    প্রত্যাগম createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  যদি (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  যদি (Size == 0)
    প্রত্যাগম createInMemoryBuffer(Path, Size, Mode);

  fs::file_status Stat;
  fs::status(Path, Stat);

  // Usually, we want to create OnDiskBuffer to create a temporary file in
  // the same directory as the destination file and atomically replaces it
  // by rename(2).
  //
  // However, if the destination file is a special file, we don't want to
  // use rename (e.g. we don't want to replace /dev/null with a regular
  // file.) If that's the case, we create an in-memory buffer, open the
  // destination file and write to it on commit().
  বিকল্প (Stat.type()) {
  পক্ষ fs::file_type::directory_file:
    প্রত্যাগম errorCodeToError(errc::is_a_directory);
  পক্ষ fs::file_type::regular_file:
  পক্ষ fs::file_type::file_not_found:
  পক্ষ fs::file_type::status_error:
    যদি (Flags & F_mmap)
      প্রত্যাগম createInMemoryBuffer(Path, Size, Mode);
    অন্যথা
      প্রত্যাগম createOnDiskBuffer(Path, Size, Mode);
  স্বভাব:
    প্রত্যাগম createInMemoryBuffer(Path, Size, Mode);
  }
}
