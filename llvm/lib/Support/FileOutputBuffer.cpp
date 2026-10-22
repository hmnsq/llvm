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

#شامل "llvm/Support/FileOutputBuffer.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/Memory.h"
#شامل "llvm/Support/TimeProfiler.h"
#شامل <system_error>

#اگر !defined(_MSC_VER) && !defined(__MINGW32__)
#شامل <unistd.h>
#وگرنه
#شامل <io.h>
#endif

using فضاینام llvm;
using فضاینام llvm::sys;

فضاینام {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
رده OnDiskBuffer : عمومی FileOutputBuffer {
عمومی:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() ثابت override { بازگشت (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() ثابت override {
    بازگشت (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() ثابت override { بازگشت Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    بازگشت Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  تهی discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

خصوصی:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
رده InMemoryBuffer : عمومی FileOutputBuffer {
عمومی:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() ثابت override { بازگشت (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() ثابت override {
    بازگشت (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() ثابت override { بازگشت BufferSize; }

  Error commit() override {
    اگر (FinalPath == "-") {
      llvm::outs() << StringRef((ثابت نویسه *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      بازگشت Error::success();
    }

    using فضاینام sys::fs;
    صحیح FD;
    std::error_code EC;
    اگر (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      بازگشت errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/درست, /*unbuffered=*/درست);
    OS << StringRef((ثابت نویسه *)Buffer.base(), BufferSize);
    بازگشت Error::success();
  }

خصوصی:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

ایستا Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  اگر (EC)
    بازگشت errorCodeToError(EC);
  بازگشت std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

ایستا Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  اگر (!FileOrErr)
    بازگشت FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  اگر (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    بازگشت errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  اگر (EC) {
    consumeError(File.discard());
    بازگشت createInMemoryBuffer(Path, Size, Mode);
  }

  بازگشت std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  اگر (Path == "-")
    بازگشت createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  اگر (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  اگر (Size == 0)
    بازگشت createInMemoryBuffer(Path, Size, Mode);

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
  انتخاب (Stat.type()) {
  حالت fs::file_type::directory_file:
    بازگشت errorCodeToError(errc::is_a_directory);
  حالت fs::file_type::regular_file:
  حالت fs::file_type::file_not_found:
  حالت fs::file_type::status_error:
    اگر (Flags & F_mmap)
      بازگشت createInMemoryBuffer(Path, Size, Mode);
    وگرنه
      بازگشت createOnDiskBuffer(Path, Size, Mode);
  پیشفرض:
    بازگشت createInMemoryBuffer(Path, Size, Mode);
  }
}
