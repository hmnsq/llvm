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

#অন্তর্ভাব "llvm/Support/FileOutputBuffer.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/Memory.h"
#অন্তর্ভাব "llvm/Support/TimeProfiler.h"
#অন্তর্ভাব <system_error>

#যদি !defined(_MSC_VER) && !defined(__MINGW32__)
#অন্তর্ভাব <unistd.h>
#অন্যথা
#অন্তর্ভাব <io.h>
#endif

using নামাকাশ llvm;
using নামাকাশ llvm::sys;

নামাকাশ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
শ্রেণী On_Disk_বাফার : সর্বজনীন File_Output_বাফার {
সর্বজনীন:
  On_Disk_বাফার(StringRef Path, fs::অস্থায়ী_File অস্থায়ী, fs::mapped_file_region Buf)
      : File_Output_বাফার(Path), বাফার(std::move(Buf)), অস্থায়ী(std::move(অস্থায়ী)) {}

  uint8_t *get_বাফার_Start() ধ্রুবক override { ফেরত (uint8_t *)বাফার.তথ্য(); }

  uint8_t *get_বাফার_End() ধ্রুবক override {
    ফেরত (uint8_t *)বাফার.তথ্য() + বাফার.আকার_3();
  }

  size_t get_বাফার_আকার() ধ্রুবক override { ফেরত বাফার.আকার_3(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    বাফার.unmap();

    // Atomically replace the existing file with the new one.
    ফেরত অস্থায়ী.keep(FinalPath);
  }

  ~On_Disk_বাফার() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    বাফার.unmap();
    consumeError(অস্থায়ী.discard());
  }

  শূন্য discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(অস্থায়ী.discard());
  }

ব্যক্তিগত:
  fs::mapped_file_region বাফার;
  fs::অস্থায়ী_File অস্থায়ী;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
শ্রেণী In_Memory_বাফার : সর্বজনীন File_Output_বাফার {
সর্বজনীন:
  In_Memory_বাফার(StringRef Path, MemoryBlock Buf, std::size_t Buf_আকার,
                 unsigned Mode)
      : File_Output_বাফার(Path), বাফার(Buf), বাফার_আকার(Buf_আকার),
        Mode(Mode) {}

  uint8_t *get_বাফার_Start() ধ্রুবক override { ফেরত (uint8_t *)বাফার.base(); }

  uint8_t *get_বাফার_End() ধ্রুবক override {
    ফেরত (uint8_t *)বাফার.base() + বাফার_আকার;
  }

  size_t get_বাফার_আকার() ধ্রুবক override { ফেরত বাফার_আকার; }

  Error commit() override {
    যদি (FinalPath == "-") {
      llvm::outs() << StringRef((ধ্রুবক অক্ষর *)বাফার.base(), বাফার_আকার);
      llvm::outs().flush();
      ফেরত Error::success();
    }

    using নামাকাশ sys::fs;
    পূর্ণসংখ্যা FD;
    std::error_code EC;
    যদি (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      ফেরত errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/সত্য, /*unbuffered=*/সত্য);
    OS << StringRef((ধ্রুবক অক্ষর *)বাফার.base(), বাফার_আকার);
    ফেরত Error::success();
  }

ব্যক্তিগত:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock বাফার;
  size_t বাফার_আকার;
  unsigned Mode;
};
} // namespace

স্থির Expected<std::unique_ptr<In_Memory_বাফার>>
create_In_Memory_বাফার(StringRef Path, size_t আকার_2, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      আকার_2, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  যদি (EC)
    ফেরত errorCodeToError(EC);
  ফেরত std::make_unique<In_Memory_বাফার>(Path, MB, আকার_2, Mode);
}

স্থির Expected<std::unique_ptr<File_Output_বাফার>>
create_On_Disk_বাফার(StringRef Path, size_t আকার_2, unsigned Mode) {
  Expected<fs::অস্থায়ী_File> FileOrErr =
      fs::অস্থায়ী_File::create(Path + ".tmp%%%%%%%", Mode);
  যদি (!FileOrErr)
    ফেরত FileOrErr.takeError();
  fs::অস্থায়ী_File File = std::move(*FileOrErr);

  যদি (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, আকার_2)) {
    consumeError(File.discard());
    ফেরত errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, আকার_2, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  যদি (EC) {
    consumeError(File.discard());
    ফেরত create_In_Memory_বাফার(Path, আকার_2, Mode);
  }

  ফেরত std::make_unique<On_Disk_বাফার>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_বাফার>>
File_Output_বাফার::create(StringRef Path, size_t আকার_2, unsigned Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  যদি (Path == "-")
    ফেরত create_In_Memory_বাফার("-", আকার_2, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  যদি (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  যদি (আকার_2 == 0)
    ফেরত create_In_Memory_বাফার(Path, আকার_2, Mode);

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
  নির্বাচন (Stat.type()) {
  ক্ষেত্র fs::file_type::directory_file:
    ফেরত errorCodeToError(errc::is_a_directory);
  ক্ষেত্র fs::file_type::regular_file:
  ক্ষেত্র fs::file_type::file_not_found:
  ক্ষেত্র fs::file_type::status_error:
    যদি (Flags & F_mmap)
      ফেরত create_In_Memory_বাফার(Path, আকার_2, Mode);
    অন্যথা
      ফেরত create_On_Disk_বাফার(Path, আকার_2, Mode);
  পূর্বনির্ধারিত:
    ফেরত create_In_Memory_বাফার(Path, আকার_2, Mode);
  }
}
