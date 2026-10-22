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

#include "llvm/Support/FileOutputBuffer.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Memory.h"
#include "llvm/Support/TimeProfiler.h"
#include <system_error>

#如果 !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#否则
#include <io.h>
#endif

using namespace llvm;
using namespace llvm::sys;

namespace {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
class OnDiskBuffer : public FileOutputBuffer {
public:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() const override { 返回 (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() const override {
    返回 (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() const override { 返回 Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    返回 Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  void discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

private:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
class InMemoryBuffer : public FileOutputBuffer {
public:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() const override { 返回 (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() const override {
    返回 (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() const override { 返回 BufferSize; }

  Error commit() override {
    如果 (FinalPath == "-") {
      llvm::outs() << StringRef((const char *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      返回 Error::success();
    }

    using namespace sys::fs;
    整数 FD;
    std::error_code EC;
    如果 (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      返回 errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/true, /*unbuffered=*/true);
    OS << StringRef((const char *)Buffer.base(), BufferSize);
    返回 Error::success();
  }

private:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

static Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  如果 (EC)
    返回 errorCodeToError(EC);
  返回 std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

static Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  如果 (!FileOrErr)
    返回 FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  如果 (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    返回 errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  如果 (EC) {
    consumeError(File.discard());
    返回 createInMemoryBuffer(Path, Size, Mode);
  }

  返回 std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  如果 (Path == "-")
    返回 createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  如果 (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  如果 (Size == 0)
    返回 createInMemoryBuffer(Path, Size, Mode);

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
  switch (Stat.type()) {
  case fs::file_type::directory_file:
    返回 errorCodeToError(errc::is_a_directory);
  case fs::file_type::regular_file:
  case fs::file_type::file_not_found:
  case fs::file_type::status_error:
    如果 (Flags & F_mmap)
      返回 createInMemoryBuffer(Path, Size, Mode);
    否则
      返回 createOnDiskBuffer(Path, Size, Mode);
  default:
    返回 createInMemoryBuffer(Path, Size, Mode);
  }
}
