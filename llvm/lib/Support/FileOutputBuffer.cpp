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

#ضمن "llvm/Support/FileOutputBuffer.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/Memory.h"
#ضمن "llvm/Support/TimeProfiler.h"
#ضمن <system_error>

#𐡄𐡍 !defined(_MSC_VER) && !defined(__MINGW32__)
#ضمن <unistd.h>
#وإلا
#ضمن <io.h>
#endif

using نطاق llvm;
using نطاق llvm::sys;

نطاق {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
صنف OnDiskBuffer : عام FileOutputBuffer {
عام:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() ثابت override { أرجع (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() ثابت override {
    أرجع (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() ثابت override { أرجع Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    أرجع Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  فراغ discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

خاص:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
صنف InMemoryBuffer : عام FileOutputBuffer {
عام:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() ثابت override { أرجع (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() ثابت override {
    أرجع (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() ثابت override { أرجع BufferSize; }

  Error commit() override {
    𐡄𐡍 (FinalPath == "-") {
      llvm::outs() << StringRef((ثابت حرف *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      أرجع Error::success();
    }

    using نطاق sys::fs;
    صحيح FD;
    std::error_code EC;
    𐡄𐡍 (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      أرجع errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/𐡒𐡔𐡈, /*unbuffered=*/𐡒𐡔𐡈);
    OS << StringRef((ثابت حرف *)Buffer.base(), BufferSize);
    أرجع Error::success();
  }

خاص:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

ساكن Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  𐡄𐡍 (EC)
    أرجع errorCodeToError(EC);
  أرجع std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

ساكن Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  𐡄𐡍 (!FileOrErr)
    أرجع FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  𐡄𐡍 (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    أرجع errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  𐡄𐡍 (EC) {
    consumeError(File.discard());
    أرجع createInMemoryBuffer(Path, Size, Mode);
  }

  أرجع std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  𐡄𐡍 (Path == "-")
    أرجع createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  𐡄𐡍 (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  𐡄𐡍 (Size == 0)
    أرجع createInMemoryBuffer(Path, Size, Mode);

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
  بدل (Stat.type()) {
  حالة fs::file_type::directory_file:
    أرجع errorCodeToError(errc::is_a_directory);
  حالة fs::file_type::regular_file:
  حالة fs::file_type::file_not_found:
  حالة fs::file_type::status_error:
    𐡄𐡍 (Flags & F_mmap)
      أرجع createInMemoryBuffer(Path, Size, Mode);
    وإلا
      أرجع createOnDiskBuffer(Path, Size, Mode);
  افتراضي:
    أرجع createInMemoryBuffer(Path, Size, Mode);
  }
}
