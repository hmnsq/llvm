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

#إذا !defined(_MSC_VER) && !defined(__MINGW32__)
#ضمن <unistd.h>
#وإلا
#ضمن <io.h>
#جيڪڏهن_پورو

استعمال نطاق llvm;
استعمال نطاق llvm::sys;

نطاق {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
صنف On_Disk_بفر : عام File_Output_بفر {
عام:
  On_Disk_بفر(StringRef رستو, fs::عارضي_File عارضي, fs::mapped_file_region Buf)
      : File_Output_بفر(رستو), بفر(std::move(Buf)), عارضي(std::move(عارضي)) {}

  uint8_t *get_بفر_شروعات() ثابت override { أرجع (uint8_t *)بفر.معلومات(); }

  uint8_t *get_بفر_پڇاڙي() ثابت override {
    أرجع (uint8_t *)بفر.معلومات() + بفر.سائيز_2();
  }

  size_t get_بفر_سائيز() ثابت override { أرجع بفر.سائيز_2(); }

  Error commit() override {
    llvm::وقت_Trace_Scope وقت_Scope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    بفر.unmap();

    // Atomically replace the existing file with the new one.
    أرجع عارضي.keep(Final_رستو);
  }

  ~On_Disk_بفر() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    بفر.unmap();
    consumeError(عارضي.discard());
  }

  فراغ discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(عارضي.discard());
  }

خاص:
  fs::mapped_file_region بفر;
  fs::عارضي_File عارضي;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
صنف اندريون_يادگيري_بفر : عام File_Output_بفر {
عام:
  اندريون_يادگيري_بفر(StringRef رستو, يادگيري_بلاڪ Buf, std::size_t Buf_سائيز,
                 بنانشان گهڻائي)
      : File_Output_بفر(رستو), بفر(Buf), بفر_سائيز(Buf_سائيز),
        گهڻائي(گهڻائي) {}

  uint8_t *get_بفر_شروعات() ثابت override { أرجع (uint8_t *)بفر.بنياد_2(); }

  uint8_t *get_بفر_پڇاڙي() ثابت override {
    أرجع (uint8_t *)بفر.بنياد_2() + بفر_سائيز;
  }

  size_t get_بفر_سائيز() ثابت override { أرجع بفر_سائيز; }

  Error commit() override {
    إذا (Final_رستو == "-") {
      llvm::outs() << StringRef((ثابت حرف *)بفر.بنياد_2(), بفر_سائيز);
      llvm::outs().flush();
      أرجع Error::success();
    }

    استعمال نطاق sys::fs;
    صحيح FD;
    std::error_ڪوڊ EC;
    إذا (خودڪار EC =
            openFileForWrite(Final_رستو, FD, CD_CreateAlways, جو_Delete, گهڻائي))
      أرجع error_ڪوڊ_منزل_Error(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/سچ, /*unbuffered=*/سچ);
    OS << StringRef((ثابت حرف *)بفر.بنياد_2(), بفر_سائيز);
    أرجع Error::success();
  }

خاص:
  // Buffer may actually contain a larger memory block than BufferSize
  Owning_يادگيري_بلاڪ بفر;
  size_t بفر_سائيز;
  بنانشان گهڻائي;
};
} // namespace

ساكن Expected<std::unique_ptr<اندريون_يادگيري_بفر>>
create_اندريون_يادگيري_بفر(StringRef رستو, size_t سائيز, بنانشان گهڻائي) {
  std::error_ڪوڊ EC;
  يادگيري_بلاڪ MB = يادگيري_2::allocate_Mapped_يادگيري(
      سائيز, خالي_اشارو, sys::يادگيري_2::MF_READ | sys::يادگيري_2::MF_WRITE, EC);
  إذا (EC)
    أرجع error_ڪوڊ_منزل_Error(EC);
  أرجع std::make_unique<اندريون_يادگيري_بفر>(رستو, MB, سائيز, گهڻائي);
}

ساكن Expected<std::unique_ptr<File_Output_بفر>>
create_On_Disk_بفر(StringRef رستو, size_t سائيز, بنانشان گهڻائي) {
  Expected<fs::عارضي_File> FileOrErr =
      fs::عارضي_File::create(رستو + ".tmp%%%%%%%", گهڻائي);
  إذا (!FileOrErr)
    أرجع FileOrErr.takeError();
  fs::عارضي_File File = std::move(*FileOrErr);

  إذا (خودڪار EC = fs::resize_file_before_mapping_readwrite(File.FD, سائيز)) {
    consumeError(File.discard());
    أرجع error_ڪوڊ_منزل_Error(EC);
  }

  // Mmap it.
  std::error_ڪوڊ EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, سائيز, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  إذا (EC) {
    consumeError(File.discard());
    أرجع create_اندريون_يادگيري_بفر(رستو, سائيز, گهڻائي);
  }

  أرجع std::make_unique<On_Disk_بفر>(رستو, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_بفر>>
File_Output_بفر::create(StringRef رستو, size_t سائيز, بنانشان Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  إذا (رستو == "-")
    أرجع create_اندريون_يادگيري_بفر("-", سائيز, /*Mode=*/0);

  بنانشان گهڻائي = fs::all_read | fs::all_write;
  إذا (Flags & F_executable)
    گهڻائي |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  إذا (سائيز == 0)
    أرجع create_اندريون_يادگيري_بفر(رستو, سائيز, گهڻائي);

  fs::file_حالت Stat;
  fs::حالت_2(رستو, Stat);

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
    أرجع error_ڪوڊ_منزل_Error(errc::is_a_directory);
  حالة fs::file_type::regular_file:
  حالة fs::file_type::file_not_مليو:
  حالة fs::file_type::حالت_error:
    إذا (Flags & F_mmap)
      أرجع create_اندريون_يادگيري_بفر(رستو, سائيز, گهڻائي);
    وإلا
      أرجع create_On_Disk_بفر(رستو, سائيز, گهڻائي);
  افتراضي:
    أرجع create_اندريون_يادگيري_بفر(رستو, سائيز, گهڻائي);
  }
}
