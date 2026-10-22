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

#અન્તર્ભાવ "llvm/Support/FileOutputBuffer.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/Memory.h"
#અન્તર્ભાવ "llvm/Support/TimeProfiler.h"
#અન્તર્ભાવ <system_error>

#જો !defined(_MSC_VER) && !defined(__MINGW32__)
#અન્તર્ભાવ <unistd.h>
#નહીંતર
#અન્તર્ભાવ <io.h>
#endif

using નામાકાશ llvm;
using નામાકાશ llvm::sys;

નામાકાશ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
વર્ગ OnDiskBuffer : જાહેર FileOutputBuffer {
જાહેર:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() અચલ override { પાછા (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() અચલ override {
    પાછા (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() અચલ override { પાછા Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    પાછા Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  શૂન્ય discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

ખાનગી:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
વર્ગ InMemoryBuffer : જાહેર FileOutputBuffer {
જાહેર:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() અચલ override { પાછા (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() અચલ override {
    પાછા (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() અચલ override { પાછા BufferSize; }

  Error commit() override {
    જો (FinalPath == "-") {
      llvm::outs() << StringRef((અચલ અક્ષર *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      પાછા Error::success();
    }

    using નામાકાશ sys::fs;
    પૂર્ણાંક FD;
    std::error_code EC;
    જો (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      પાછા errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/સાચું, /*unbuffered=*/સાચું);
    OS << StringRef((અચલ અક્ષર *)Buffer.base(), BufferSize);
    પાછા Error::success();
  }

ખાનગી:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

સ્થિર Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  જો (EC)
    પાછા errorCodeToError(EC);
  પાછા std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

સ્થિર Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  જો (!FileOrErr)
    પાછા FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  જો (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    પાછા errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  જો (EC) {
    consumeError(File.discard());
    પાછા createInMemoryBuffer(Path, Size, Mode);
  }

  પાછા std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  જો (Path == "-")
    પાછા createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  જો (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  જો (Size == 0)
    પાછા createInMemoryBuffer(Path, Size, Mode);

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
  પસંદગી (Stat.type()) {
  સ્થિતિ fs::file_type::directory_file:
    પાછા errorCodeToError(errc::is_a_directory);
  સ્થિતિ fs::file_type::regular_file:
  સ્થિતિ fs::file_type::file_not_found:
  સ્થિતિ fs::file_type::status_error:
    જો (Flags & F_mmap)
      પાછા createInMemoryBuffer(Path, Size, Mode);
    નહીંતર
      પાછા createOnDiskBuffer(Path, Size, Mode);
  મૂળભૂત:
    પાછા createInMemoryBuffer(Path, Size, Mode);
  }
}
