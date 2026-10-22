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
#پایان_اگر

استفاده نطاق llvm;
استفاده نطاق llvm::sys;

نطاق {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
صنف On_Disk_میانگیر : عام File_Output_میانگیر {
عام:
  On_Disk_میانگیر(StringRef مسیر, fs::موقت_File موقت, fs::mapped_file_region Buf)
      : File_Output_میانگیر(مسیر), میانگیر(std::move(Buf)), موقت(std::move(موقت)) {}

  uint8_t *get_میانگیر_شروع() ثابت override { أرجع (uint8_t *)میانگیر.داده(); }

  uint8_t *get_میانگیر_انتها() ثابت override {
    أرجع (uint8_t *)میانگیر.داده() + میانگیر.اندازه_2();
  }

  size_t get_میانگیر_اندازه() ثابت override { أرجع میانگیر.اندازه_2(); }

  Error commit() override {
    llvm::زمان_Trace_Scope زمان_Scope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    میانگیر.unmap();

    // Atomically replace the existing file with the new one.
    أرجع موقت.keep(Final_مسیر);
  }

  ~On_Disk_میانگیر() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    میانگیر.unmap();
    consumeError(موقت.discard());
  }

  فراغ discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(موقت.discard());
  }

خاص:
  fs::mapped_file_region میانگیر;
  fs::موقت_File موقت;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
صنف درون_حافظه_میانگیر : عام File_Output_میانگیر {
عام:
  درون_حافظه_میانگیر(StringRef مسیر, حافظه_بلوک Buf, std::size_t Buf_اندازه,
                 بیعلامت نما)
      : File_Output_میانگیر(مسیر), میانگیر(Buf), میانگیر_اندازه(Buf_اندازه),
        نما(نما) {}

  uint8_t *get_میانگیر_شروع() ثابت override { أرجع (uint8_t *)میانگیر.پایه_2(); }

  uint8_t *get_میانگیر_انتها() ثابت override {
    أرجع (uint8_t *)میانگیر.پایه_2() + میانگیر_اندازه;
  }

  size_t get_میانگیر_اندازه() ثابت override { أرجع میانگیر_اندازه; }

  Error commit() override {
    إذا (Final_مسیر == "-") {
      llvm::outs() << StringRef((ثابت حرف *)میانگیر.پایه_2(), میانگیر_اندازه);
      llvm::outs().flush();
      أرجع Error::success();
    }

    استفاده نطاق sys::fs;
    صحيح FD;
    std::error_کد EC;
    إذا (خودکار EC =
            openFileForWrite(Final_مسیر, FD, CD_CreateAlways, از_Delete, نما))
      أرجع error_کد_مقصد_Error(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/درست, /*unbuffered=*/درست);
    OS << StringRef((ثابت حرف *)میانگیر.پایه_2(), میانگیر_اندازه);
    أرجع Error::success();
  }

خاص:
  // Buffer may actually contain a larger memory block than BufferSize
  Owning_حافظه_بلوک میانگیر;
  size_t میانگیر_اندازه;
  بیعلامت نما;
};
} // namespace

ساكن Expected<std::unique_ptr<درون_حافظه_میانگیر>>
create_درون_حافظه_میانگیر(StringRef مسیر, size_t اندازه, بیعلامت نما) {
  std::error_کد EC;
  حافظه_بلوک MB = حافظه_2::allocate_Mapped_حافظه(
      اندازه, اشاره_تهی, sys::حافظه_2::MF_READ | sys::حافظه_2::MF_WRITE, EC);
  إذا (EC)
    أرجع error_کد_مقصد_Error(EC);
  أرجع std::make_unique<درون_حافظه_میانگیر>(مسیر, MB, اندازه, نما);
}

ساكن Expected<std::unique_ptr<File_Output_میانگیر>>
create_On_Disk_میانگیر(StringRef مسیر, size_t اندازه, بیعلامت نما) {
  Expected<fs::موقت_File> FileOrErr =
      fs::موقت_File::create(مسیر + ".tmp%%%%%%%", نما);
  إذا (!FileOrErr)
    أرجع FileOrErr.takeError();
  fs::موقت_File File = std::move(*FileOrErr);

  إذا (خودکار EC = fs::resize_file_before_mapping_readwrite(File.FD, اندازه)) {
    consumeError(File.discard());
    أرجع error_کد_مقصد_Error(EC);
  }

  // Mmap it.
  std::error_کد EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, اندازه, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  إذا (EC) {
    consumeError(File.discard());
    أرجع create_درون_حافظه_میانگیر(مسیر, اندازه, نما);
  }

  أرجع std::make_unique<On_Disk_میانگیر>(مسیر, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_میانگیر>>
File_Output_میانگیر::create(StringRef مسیر, size_t اندازه, بیعلامت Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  إذا (مسیر == "-")
    أرجع create_درون_حافظه_میانگیر("-", اندازه, /*Mode=*/0);

  بیعلامت نما = fs::all_read | fs::all_write;
  إذا (Flags & F_executable)
    نما |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  إذا (اندازه == 0)
    أرجع create_درون_حافظه_میانگیر(مسیر, اندازه, نما);

  fs::file_وضعیت Stat;
  fs::وضعیت_2(مسیر, Stat);

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
    أرجع error_کد_مقصد_Error(errc::is_a_directory);
  حالة fs::file_type::regular_file:
  حالة fs::file_type::file_not_یافت:
  حالة fs::file_type::وضعیت_error:
    إذا (Flags & F_mmap)
      أرجع create_درون_حافظه_میانگیر(مسیر, اندازه, نما);
    وإلا
      أرجع create_On_Disk_میانگیر(مسیر, اندازه, نما);
  افتراضي:
    أرجع create_درون_حافظه_میانگیر(مسیر, اندازه, نما);
  }
}
