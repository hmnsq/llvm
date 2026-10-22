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

#அந்தர்பாவ "llvm/Support/FileOutputBuffer.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/Memory.h"
#அந்தர்பாவ "llvm/Support/TimeProfiler.h"
#அந்தர்பாவ <system_error>

#எனில் !defined(_MSC_VER) && !defined(__MINGW32__)
#அந்தர்பாவ <unistd.h>
#இல்லையெனில்
#அந்தர்பாவ <io.h>
#endif

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::sys;

நாமாகாஶ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
வகுப்பு OnDiskBuffer : பொது FileOutputBuffer {
பொது:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() மாறிலி override { திரும்பு (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() மாறிலி override {
    திரும்பு (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() மாறிலி override { திரும்பு Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    திரும்பு Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  வெறுமை discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

தனிப்பட்ட:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
வகுப்பு InMemoryBuffer : பொது FileOutputBuffer {
பொது:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() மாறிலி override { திரும்பு (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() மாறிலி override {
    திரும்பு (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() மாறிலி override { திரும்பு BufferSize; }

  Error commit() override {
    எனில் (FinalPath == "-") {
      llvm::outs() << StringRef((மாறிலி எழுத்து *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      திரும்பு Error::success();
    }

    using நாமாகாஶ sys::fs;
    முழு FD;
    std::error_code EC;
    எனில் (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      திரும்பு errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/உண்மை, /*unbuffered=*/உண்மை);
    OS << StringRef((மாறிலி எழுத்து *)Buffer.base(), BufferSize);
    திரும்பு Error::success();
  }

தனிப்பட்ட:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

நிலையான Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  எனில் (EC)
    திரும்பு errorCodeToError(EC);
  திரும்பு std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

நிலையான Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  எனில் (!FileOrErr)
    திரும்பு FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  எனில் (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    திரும்பு errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  எனில் (EC) {
    consumeError(File.discard());
    திரும்பு createInMemoryBuffer(Path, Size, Mode);
  }

  திரும்பு std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  எனில் (Path == "-")
    திரும்பு createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  எனில் (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  எனில் (Size == 0)
    திரும்பு createInMemoryBuffer(Path, Size, Mode);

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
  தேர்வு (Stat.type()) {
  நிலை fs::file_type::directory_file:
    திரும்பு errorCodeToError(errc::is_a_directory);
  நிலை fs::file_type::regular_file:
  நிலை fs::file_type::file_not_found:
  நிலை fs::file_type::status_error:
    எனில் (Flags & F_mmap)
      திரும்பு createInMemoryBuffer(Path, Size, Mode);
    இல்லையெனில்
      திரும்பு createOnDiskBuffer(Path, Size, Mode);
  இயல்பு:
    திரும்பு createInMemoryBuffer(Path, Size, Mode);
  }
}
