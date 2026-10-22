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

#ಅನ್ತರ್ಭಾವ "llvm/Support/FileOutputBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Memory.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/TimeProfiler.h"
#ಅನ್ತರ್ಭಾವ <system_error>

#ಯದಿ !defined(_MSC_VER) && !defined(__MINGW32__)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <io.h>
#endif

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::sys;

ನಾಮಾಕಾಶ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
ವರ್ಗ OnDiskBuffer : ಸಾರ್ವಜನಿಕ FileOutputBuffer {
ಸಾರ್ವಜನಿಕ:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    ಪ್ರತ್ಯಾಗಮ Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  ಶೂನ್ಯ discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

ಗುಪ್ತ:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
ವರ್ಗ InMemoryBuffer : ಸಾರ್ವಜನಿಕ FileOutputBuffer {
ಸಾರ್ವಜನಿಕ:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ BufferSize; }

  Error commit() override {
    ಯದಿ (FinalPath == "-") {
      llvm::outs() << StringRef((ಧ್ರುವ ವರ್ಣ *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      ಪ್ರತ್ಯಾಗಮ Error::success();
    }

    using ನಾಮಾಕಾಶ sys::fs;
    ಪೂರ್ಣಾಙ್ಕ FD;
    std::error_code EC;
    ಯದಿ (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/ಸತ್ಯ, /*unbuffered=*/ಸತ್ಯ);
    OS << StringRef((ಧ್ರುವ ವರ್ಣ *)Buffer.base(), BufferSize);
    ಪ್ರತ್ಯಾಗಮ Error::success();
  }

ಗುಪ್ತ:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

ಸ್ಥಿರ Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
  ಪ್ರತ್ಯಾಗಮ std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

ಸ್ಥಿರ Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  ಯದಿ (!FileOrErr)
    ಪ್ರತ್ಯಾಗಮ FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  ಯದಿ (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  ಯದಿ (EC) {
    consumeError(File.discard());
    ಪ್ರತ್ಯಾಗಮ createInMemoryBuffer(Path, Size, Mode);
  }

  ಪ್ರತ್ಯಾಗಮ std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  ಯದಿ (Path == "-")
    ಪ್ರತ್ಯಾಗಮ createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  ಯದಿ (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  ಯದಿ (Size == 0)
    ಪ್ರತ್ಯಾಗಮ createInMemoryBuffer(Path, Size, Mode);

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
  ವಿಕಲ್ಪ (Stat.type()) {
  ಪಕ್ಷ fs::file_type::directory_file:
    ಪ್ರತ್ಯಾಗಮ errorCodeToError(errc::is_a_directory);
  ಪಕ್ಷ fs::file_type::regular_file:
  ಪಕ್ಷ fs::file_type::file_not_found:
  ಪಕ್ಷ fs::file_type::status_error:
    ಯದಿ (Flags & F_mmap)
      ಪ್ರತ್ಯಾಗಮ createInMemoryBuffer(Path, Size, Mode);
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ createOnDiskBuffer(Path, Size, Mode);
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ createInMemoryBuffer(Path, Size, Mode);
  }
}
