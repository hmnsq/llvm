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

#അന്തര്ഭാവ "llvm/Support/FileOutputBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/Memory.h"
#അന്തര്ഭാവ "llvm/Support/TimeProfiler.h"
#അന്തര്ഭാവ <system_error>

#എങ്കിൽ !defined(_MSC_VER) && !defined(__MINGW32__)
#അന്തര്ഭാവ <unistd.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <io.h>
#endif

using നാമാകാശ llvm;
using നാമാകാശ llvm::sys;

നാമാകാശ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
വർഗ്ഗം OnDiskBuffer : പൊതു FileOutputBuffer {
പൊതു:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() സ്ഥിരം override { തിരികെ (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() സ്ഥിരം override {
    തിരികെ (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() സ്ഥിരം override { തിരികെ Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    തിരികെ Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  ശൂന്യം discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

സ്വകാര്യ:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
വർഗ്ഗം InMemoryBuffer : പൊതു FileOutputBuffer {
പൊതു:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() സ്ഥിരം override { തിരികെ (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() സ്ഥിരം override {
    തിരികെ (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() സ്ഥിരം override { തിരികെ BufferSize; }

  Error commit() override {
    എങ്കിൽ (FinalPath == "-") {
      llvm::outs() << StringRef((സ്ഥിരം അക്ഷരം *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      തിരികെ Error::success();
    }

    using നാമാകാശ sys::fs;
    പൂർണ്ണസംഖ്യ FD;
    std::error_code EC;
    എങ്കിൽ (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      തിരികെ errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/സത്യം, /*unbuffered=*/സത്യം);
    OS << StringRef((സ്ഥിരം അക്ഷരം *)Buffer.base(), BufferSize);
    തിരികെ Error::success();
  }

സ്വകാര്യ:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

സ്ഥിതം Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  എങ്കിൽ (EC)
    തിരികെ errorCodeToError(EC);
  തിരികെ std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

സ്ഥിതം Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  എങ്കിൽ (!FileOrErr)
    തിരികെ FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  എങ്കിൽ (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    തിരികെ errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  എങ്കിൽ (EC) {
    consumeError(File.discard());
    തിരികെ createInMemoryBuffer(Path, Size, Mode);
  }

  തിരികെ std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  എങ്കിൽ (Path == "-")
    തിരികെ createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  എങ്കിൽ (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  എങ്കിൽ (Size == 0)
    തിരികെ createInMemoryBuffer(Path, Size, Mode);

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
  തിരഞ്ഞെടുപ്പ് (Stat.type()) {
  അവസ്ഥ fs::file_type::directory_file:
    തിരികെ errorCodeToError(errc::is_a_directory);
  അവസ്ഥ fs::file_type::regular_file:
  അവസ്ഥ fs::file_type::file_not_found:
  അവസ്ഥ fs::file_type::status_error:
    എങ്കിൽ (Flags & F_mmap)
      തിരികെ createInMemoryBuffer(Path, Size, Mode);
    അല്ലെങ്കിൽ
      തിരികെ createOnDiskBuffer(Path, Size, Mode);
  സ്ഥിരസ്ഥിതി:
    തിരികെ createInMemoryBuffer(Path, Size, Mode);
  }
}
