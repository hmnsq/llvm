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

#অন্তর্ভাৱ "llvm/Support/FileOutputBuffer.h"
#অন্তর্ভাৱ "llvm/Support/Errc.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"
#অন্তর্ভাৱ "llvm/Support/Memory.h"
#অন্তর্ভাৱ "llvm/Support/TimeProfiler.h"
#অন্তর্ভাৱ <system_error>

#যদি !defined(_MSC_VER) && !defined(__MINGW32__)
#অন্তর্ভাৱ <unistd.h>
#অন্যথা
#অন্তর্ভাৱ <io.h>
#endif

using নামাকাশ llvm;
using নামাকাশ llvm::sys;

নামাকাশ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
শ্ৰেণী OnDiskBuffer : সার্ৱজনিক FileOutputBuffer {
সার্ৱজনিক:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() ধ্রুৱ override { ঘূৰাই (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() ধ্রুৱ override {
    ঘূৰাই (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() ধ্রুৱ override { ঘূৰাই Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    ঘূৰাই Temp.keep(FinalPath);
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
শ্ৰেণী InMemoryBuffer : সার্ৱজনিক FileOutputBuffer {
সার্ৱজনিক:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() ধ্রুৱ override { ঘূৰাই (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() ধ্রুৱ override {
    ঘূৰাই (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() ধ্রুৱ override { ঘূৰাই BufferSize; }

  Error commit() override {
    যদি (FinalPath == "-") {
      llvm::outs() << StringRef((ধ্রুৱ ৱর্ণ *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      ঘূৰাই Error::success();
    }

    using নামাকাশ sys::fs;
    পূর্ণাঙ্ক FD;
    std::error_code EC;
    যদি (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      ঘূৰাই errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/সঁচা, /*unbuffered=*/সঁচা);
    OS << StringRef((ধ্রুৱ ৱর্ণ *)Buffer.base(), BufferSize);
    ঘূৰাই Error::success();
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
    ঘূৰাই errorCodeToError(EC);
  ঘূৰাই std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

স্থির Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  যদি (!FileOrErr)
    ঘূৰাই FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  যদি (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    ঘূৰাই errorCodeToError(EC);
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
    ঘূৰাই createInMemoryBuffer(Path, Size, Mode);
  }

  ঘূৰাই std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  যদি (Path == "-")
    ঘূৰাই createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  যদি (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  যদি (Size == 0)
    ঘূৰাই createInMemoryBuffer(Path, Size, Mode);

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
  ৱিকল্প (Stat.type()) {
  পক্ষ fs::file_type::directory_file:
    ঘূৰাই errorCodeToError(errc::is_a_directory);
  পক্ষ fs::file_type::regular_file:
  পক্ষ fs::file_type::file_not_found:
  পক্ষ fs::file_type::status_error:
    যদি (Flags & F_mmap)
      ঘূৰাই createInMemoryBuffer(Path, Size, Mode);
    অন্যথা
      ঘূৰাই createOnDiskBuffer(Path, Size, Mode);
  স্ৱভাৱ:
    ঘূৰাই createInMemoryBuffer(Path, Size, Mode);
  }
}
