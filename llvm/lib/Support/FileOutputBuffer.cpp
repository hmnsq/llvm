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

#अन्तर्भाव "llvm/Support/FileOutputBuffer.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/Memory.h"
#अन्तर्भाव "llvm/Support/TimeProfiler.h"
#अन्तर्भाव <system_error>

#जर !defined(_MSC_VER) && !defined(__MINGW32__)
#अन्तर्भाव <unistd.h>
#अन्यथा
#अन्तर्भाव <io.h>
#endif

using नामाकाश llvm;
using नामाकाश llvm::sys;

नामाकाश {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
वर्ग OnDiskBuffer : सार्वजनिक FileOutputBuffer {
सार्वजनिक:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() स्थिरांक override { परत (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() स्थिरांक override {
    परत (uint8_t *)Buffer.data() + Buffer.size();
  }

  माप_प्रकार getBufferSize() स्थिरांक override { परत Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    परत Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  शून्य discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

खाजगी:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
वर्ग InMemoryBuffer : सार्वजनिक FileOutputBuffer {
सार्वजनिक:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::माप_प्रकार BufSize,
                 अचिन्हित Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() स्थिरांक override { परत (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() स्थिरांक override {
    परत (uint8_t *)Buffer.base() + BufferSize;
  }

  माप_प्रकार getBufferSize() स्थिरांक override { परत BufferSize; }

  Error commit() override {
    जर (FinalPath == "-") {
      llvm::outs() << StringRef((स्थिरांक अक्षर *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      परत Error::success();
    }

    using नामाकाश sys::fs;
    पूर्णांक FD;
    std::error_code EC;
    जर (स्वतः EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      परत errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/सत्य, /*unbuffered=*/सत्य);
    OS << StringRef((स्थिरांक अक्षर *)Buffer.base(), BufferSize);
    परत Error::success();
  }

खाजगी:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  माप_प्रकार BufferSize;
  अचिन्हित Mode;
};
} // namespace

स्थिर Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, माप_प्रकार Size, अचिन्हित Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  जर (EC)
    परत errorCodeToError(EC);
  परत std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

स्थिर Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, माप_प्रकार Size, अचिन्हित Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  जर (!FileOrErr)
    परत FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  जर (स्वतः EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    परत errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  जर (EC) {
    consumeError(File.discard());
    परत createInMemoryBuffer(Path, Size, Mode);
  }

  परत std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, माप_प्रकार Size, अचिन्हित Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  जर (Path == "-")
    परत createInMemoryBuffer("-", Size, /*Mode=*/0);

  अचिन्हित Mode = fs::all_read | fs::all_write;
  जर (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  जर (Size == 0)
    परत createInMemoryBuffer(Path, Size, Mode);

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
  निवड (Stat.type()) {
  स्थिती fs::file_type::directory_file:
    परत errorCodeToError(errc::is_a_directory);
  स्थिती fs::file_type::regular_file:
  स्थिती fs::file_type::file_not_found:
  स्थिती fs::file_type::status_error:
    जर (Flags & F_mmap)
      परत createInMemoryBuffer(Path, Size, Mode);
    अन्यथा
      परत createOnDiskBuffer(Path, Size, Mode);
  पूर्वनिर्धारित:
    परत createInMemoryBuffer(Path, Size, Mode);
  }
}
