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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileOutputBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Memory.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/TimeProfiler.h"
#ଅନ୍ତର୍ଭାଵ <system_error>

#ଯଦି !defined(_MSC_VER) && !defined(__MINGW32__)
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <io.h>
#endif

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::sys;

ନାମାକାଶ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
ଶ୍ରେଣୀ OnDiskBuffer : ସର୍ବସାଧାରଣ FileOutputBuffer {
ସର୍ବସାଧାରଣ:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() ଅଚଳ override { ଫେରନ୍ତୁ (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() ଅଚଳ override {
    ଫେରନ୍ତୁ (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() ଅଚଳ override { ଫେରନ୍ତୁ Buffer.size(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    ଫେରନ୍ତୁ Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  ଶୂନ୍ୟ discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

ବ୍ୟକ୍ତିଗତ:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
ଶ୍ରେଣୀ InMemoryBuffer : ସର୍ବସାଧାରଣ FileOutputBuffer {
ସର୍ବସାଧାରଣ:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() ଅଚଳ override { ଫେରନ୍ତୁ (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() ଅଚଳ override {
    ଫେରନ୍ତୁ (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() ଅଚଳ override { ଫେରନ୍ତୁ BufferSize; }

  Error commit() override {
    ଯଦି (FinalPath == "-") {
      llvm::outs() << StringRef((ଅଚଳ ଅକ୍ଷର *)Buffer.base(), BufferSize);
      llvm::outs().flush();
      ଫେରନ୍ତୁ Error::success();
    }

    using ନାମାକାଶ sys::fs;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD;
    std::error_code EC;
    ଯଦି (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      ଫେରନ୍ତୁ errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/ସତ୍ୟ, /*unbuffered=*/ସତ୍ୟ);
    OS << StringRef((ଅଚଳ ଅକ୍ଷର *)Buffer.base(), BufferSize);
    ଫେରନ୍ତୁ Error::success();
  }

ବ୍ୟକ୍ତିଗତ:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

ସ୍ଥିର Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  ଯଦି (EC)
    ଫେରନ୍ତୁ errorCodeToError(EC);
  ଫେରନ୍ତୁ std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

ସ୍ଥିର Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  ଯଦି (!FileOrErr)
    ଫେରନ୍ତୁ FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  ଯଦି (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    ଫେରନ୍ତୁ errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  ଯଦି (EC) {
    consumeError(File.discard());
    ଫେରନ୍ତୁ createInMemoryBuffer(Path, Size, Mode);
  }

  ଫେରନ୍ତୁ std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  ଯଦି (Path == "-")
    ଫେରନ୍ତୁ createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  ଯଦି (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  ଯଦି (Size == 0)
    ଫେରନ୍ତୁ createInMemoryBuffer(Path, Size, Mode);

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
  ବାଛ (Stat.type()) {
  ସ୍ଥିତି fs::file_type::directory_file:
    ଫେରନ୍ତୁ errorCodeToError(errc::is_a_directory);
  ସ୍ଥିତି fs::file_type::regular_file:
  ସ୍ଥିତି fs::file_type::file_not_found:
  ସ୍ଥିତି fs::file_type::status_error:
    ଯଦି (Flags & F_mmap)
      ଫେରନ୍ତୁ createInMemoryBuffer(Path, Size, Mode);
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ createOnDiskBuffer(Path, Size, Mode);
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ createInMemoryBuffer(Path, Size, Mode);
  }
}
