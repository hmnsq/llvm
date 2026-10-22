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

#شامل "llvm/Support/FileOutputBuffer.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/Memory.h"
#شامل "llvm/Support/TimeProfiler.h"
#شامل <system_error>

#اگر !defined(_MSC_VER) && !defined(__MINGW32__)
#شامل <unistd.h>
#ورنہ
#شامل <io.h>
#اگر_ختم

استعمال نامخلا llvm;
استعمال نامخلا llvm::sys;

نامخلا {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
جماعت On_Disk_بفر : عوامی File_Output_بفر {
عوامی:
  On_Disk_بفر(StringRef راستہ, fs::عارضی_File عارضی, fs::mapped_file_region Buf)
      : File_Output_بفر(راستہ), بفر(std::move(Buf)), عارضی(std::move(عارضی)) {}

  uint8_t *get_بفر_آغاز() ثابت override { واپس (uint8_t *)بفر.ڈیٹا(); }

  uint8_t *get_بفر_انتہا() ثابت override {
    واپس (uint8_t *)بفر.ڈیٹا() + بفر.سائز_2();
  }

  size_t get_بفر_سائز() ثابت override { واپس بفر.سائز_2(); }

  Error commit() override {
    llvm::وقت_Trace_Scope وقت_Scope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    بفر.unmap();

    // Atomically replace the existing file with the new one.
    واپس عارضی.keep(Final_راستہ);
  }

  ~On_Disk_بفر() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    بفر.unmap();
    consumeError(عارضی.discard());
  }

  خالی discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(عارضی.discard());
  }

نجی:
  fs::mapped_file_region بفر;
  fs::عارضی_File عارضی;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
جماعت اندر_حافظہ_بفر : عوامی File_Output_بفر {
عوامی:
  اندر_حافظہ_بفر(StringRef راستہ, حافظہ_بلاک Buf, std::size_t Buf_سائز,
                 غیرعلامتی منوال)
      : File_Output_بفر(راستہ), بفر(Buf), بفر_سائز(Buf_سائز),
        منوال(منوال) {}

  uint8_t *get_بفر_آغاز() ثابت override { واپس (uint8_t *)بفر.بنیاد_2(); }

  uint8_t *get_بفر_انتہا() ثابت override {
    واپس (uint8_t *)بفر.بنیاد_2() + بفر_سائز;
  }

  size_t get_بفر_سائز() ثابت override { واپس بفر_سائز; }

  Error commit() override {
    اگر (Final_راستہ == "-") {
      llvm::outs() << StringRef((ثابت حرف *)بفر.بنیاد_2(), بفر_سائز);
      llvm::outs().flush();
      واپس Error::success();
    }

    استعمال نامخلا sys::fs;
    صحیح FD;
    std::error_رمز EC;
    اگر (خودکار EC =
            openFileForWrite(Final_راستہ, FD, CD_CreateAlways, کا_Delete, منوال))
      واپس error_رمز_مقصد_Error(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/سچ, /*unbuffered=*/سچ);
    OS << StringRef((ثابت حرف *)بفر.بنیاد_2(), بفر_سائز);
    واپس Error::success();
  }

نجی:
  // Buffer may actually contain a larger memory block than BufferSize
  Owning_حافظہ_بلاک بفر;
  size_t بفر_سائز;
  غیرعلامتی منوال;
};
} // namespace

ساکن Expected<std::unique_ptr<اندر_حافظہ_بفر>>
create_اندر_حافظہ_بفر(StringRef راستہ, size_t سائز, غیرعلامتی منوال) {
  std::error_رمز EC;
  حافظہ_بلاک MB = حافظہ_2::allocate_Mapped_حافظہ(
      سائز, خالی_اشاریہ, sys::حافظہ_2::MF_READ | sys::حافظہ_2::MF_WRITE, EC);
  اگر (EC)
    واپس error_رمز_مقصد_Error(EC);
  واپس std::make_unique<اندر_حافظہ_بفر>(راستہ, MB, سائز, منوال);
}

ساکن Expected<std::unique_ptr<File_Output_بفر>>
create_On_Disk_بفر(StringRef راستہ, size_t سائز, غیرعلامتی منوال) {
  Expected<fs::عارضی_File> FileOrErr =
      fs::عارضی_File::create(راستہ + ".tmp%%%%%%%", منوال);
  اگر (!FileOrErr)
    واپس FileOrErr.takeError();
  fs::عارضی_File File = std::move(*FileOrErr);

  اگر (خودکار EC = fs::resize_file_before_mapping_readwrite(File.FD, سائز)) {
    consumeError(File.discard());
    واپس error_رمز_مقصد_Error(EC);
  }

  // Mmap it.
  std::error_رمز EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, سائز, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  اگر (EC) {
    consumeError(File.discard());
    واپس create_اندر_حافظہ_بفر(راستہ, سائز, منوال);
  }

  واپس std::make_unique<On_Disk_بفر>(راستہ, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_بفر>>
File_Output_بفر::create(StringRef راستہ, size_t سائز, غیرعلامتی Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  اگر (راستہ == "-")
    واپس create_اندر_حافظہ_بفر("-", سائز, /*Mode=*/0);

  غیرعلامتی منوال = fs::all_read | fs::all_write;
  اگر (Flags & F_executable)
    منوال |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  اگر (سائز == 0)
    واپس create_اندر_حافظہ_بفر(راستہ, سائز, منوال);

  fs::file_حالت Stat;
  fs::حالت_3(راستہ, Stat);

  // Usually, we want to create OnDiskBuffer to create a temporary file in
  // the same directory as the destination file and atomically replaces it
  // by rename(2).
  //
  // However, if the destination file is a special file, we don't want to
  // use rename (e.g. we don't want to replace /dev/null with a regular
  // file.) If that's the case, we create an in-memory buffer, open the
  // destination file and write to it on commit().
  انتخاب (Stat.type()) {
  حالت fs::file_type::directory_file:
    واپس error_رمز_مقصد_Error(errc::is_a_directory);
  حالت fs::file_type::regular_file:
  حالت fs::file_type::file_not_ملا:
  حالت fs::file_type::حالت_error:
    اگر (Flags & F_mmap)
      واپس create_اندر_حافظہ_بفر(راستہ, سائز, منوال);
    ورنہ
      واپس create_On_Disk_بفر(راستہ, سائز, منوال);
  طےشدہ:
    واپس create_اندر_حافظہ_بفر(راستہ, سائز, منوال);
  }
}
