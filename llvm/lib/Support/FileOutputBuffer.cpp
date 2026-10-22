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
#د_که_پای

کارول نطاق llvm;
کارول نطاق llvm::sys;

نطاق {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
صنف On_Disk_بفر : عام File_Output_بفر {
عام:
  On_Disk_بفر(StringRef لار, fs::لنډمهاله_File لنډمهاله, fs::mapped_file_region Buf)
      : File_Output_بفر(لار), بفر(std::move(Buf)), لنډمهاله(std::move(لنډمهاله)) {}

  uint8_t *get_بفر_پیل() ثابت override { أرجع (uint8_t *)بفر.معلومات(); }

  uint8_t *get_بفر_پای_ټکی() ثابت override {
    أرجع (uint8_t *)بفر.معلومات() + بفر.اندازه_2();
  }

  size_t get_بفر_اندازه() ثابت override { أرجع بفر.اندازه_2(); }

  Error commit() override {
    llvm::وخت_Trace_Scope وخت_Scope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    بفر.unmap();

    // Atomically replace the existing file with the new one.
    أرجع لنډمهاله.keep(Final_لار);
  }

  ~On_Disk_بفر() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    بفر.unmap();
    consumeError(لنډمهاله.discard());
  }

  فراغ discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(لنډمهاله.discard());
  }

خاص:
  fs::mapped_file_region بفر;
  fs::لنډمهاله_File لنډمهاله;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
صنف دننه_حافظه_بفر : عام File_Output_بفر {
عام:
  دننه_حافظه_بفر(StringRef لار, حافظه_بلاک Buf, std::size_t Buf_اندازه,
                 بینښه ډېرپېښ)
      : File_Output_بفر(لار), بفر(Buf), بفر_اندازه(Buf_اندازه),
        ډېرپېښ(ډېرپېښ) {}

  uint8_t *get_بفر_پیل() ثابت override { أرجع (uint8_t *)بفر.بنسټ_2(); }

  uint8_t *get_بفر_پای_ټکی() ثابت override {
    أرجع (uint8_t *)بفر.بنسټ_2() + بفر_اندازه;
  }

  size_t get_بفر_اندازه() ثابت override { أرجع بفر_اندازه; }

  Error commit() override {
    إذا (Final_لار == "-") {
      llvm::outs() << StringRef((ثابت حرف *)بفر.بنسټ_2(), بفر_اندازه);
      llvm::outs().flush();
      أرجع Error::success();
    }

    کارول نطاق sys::fs;
    صحيح FD;
    std::error_کوډ EC;
    إذا (خپلکاری EC =
            openFileForWrite(Final_لار, FD, CD_CreateAlways, د_Delete, ډېرپېښ))
      أرجع error_کوډ_منزل_Error(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/ریښتیا, /*unbuffered=*/ریښتیا);
    OS << StringRef((ثابت حرف *)بفر.بنسټ_2(), بفر_اندازه);
    أرجع Error::success();
  }

خاص:
  // Buffer may actually contain a larger memory block than BufferSize
  Owning_حافظه_بلاک بفر;
  size_t بفر_اندازه;
  بینښه ډېرپېښ;
};
} // namespace

ساكن Expected<std::unique_ptr<دننه_حافظه_بفر>>
create_دننه_حافظه_بفر(StringRef لار, size_t اندازه, بینښه ډېرپېښ) {
  std::error_کوډ EC;
  حافظه_بلاک MB = حافظه_2::allocate_Mapped_حافظه(
      اندازه, تش_اشاره, sys::حافظه_2::MF_READ | sys::حافظه_2::MF_WRITE, EC);
  إذا (EC)
    أرجع error_کوډ_منزل_Error(EC);
  أرجع std::make_unique<دننه_حافظه_بفر>(لار, MB, اندازه, ډېرپېښ);
}

ساكن Expected<std::unique_ptr<File_Output_بفر>>
create_On_Disk_بفر(StringRef لار, size_t اندازه, بینښه ډېرپېښ) {
  Expected<fs::لنډمهاله_File> FileOrErr =
      fs::لنډمهاله_File::create(لار + ".tmp%%%%%%%", ډېرپېښ);
  إذا (!FileOrErr)
    أرجع FileOrErr.takeError();
  fs::لنډمهاله_File File = std::move(*FileOrErr);

  إذا (خپلکاری EC = fs::resize_file_before_mapping_readwrite(File.FD, اندازه)) {
    consumeError(File.discard());
    أرجع error_کوډ_منزل_Error(EC);
  }

  // Mmap it.
  std::error_کوډ EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, اندازه, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  إذا (EC) {
    consumeError(File.discard());
    أرجع create_دننه_حافظه_بفر(لار, اندازه, ډېرپېښ);
  }

  أرجع std::make_unique<On_Disk_بفر>(لار, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_بفر>>
File_Output_بفر::create(StringRef لار, size_t اندازه, بینښه Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  إذا (لار == "-")
    أرجع create_دننه_حافظه_بفر("-", اندازه, /*Mode=*/0);

  بینښه ډېرپېښ = fs::all_read | fs::all_write;
  إذا (Flags & F_executable)
    ډېرپېښ |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  إذا (اندازه == 0)
    أرجع create_دننه_حافظه_بفر(لار, اندازه, ډېرپېښ);

  fs::file_حالت Stat;
  fs::حالت_2(لار, Stat);

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
    أرجع error_کوډ_منزل_Error(errc::is_a_directory);
  حالة fs::file_type::regular_file:
  حالة fs::file_type::file_not_وموندل:
  حالة fs::file_type::حالت_error:
    إذا (Flags & F_mmap)
      أرجع create_دننه_حافظه_بفر(لار, اندازه, ډېرپېښ);
    وإلا
      أرجع create_On_Disk_بفر(لار, اندازه, ډېرپېښ);
  افتراضي:
    أرجع create_دننه_حافظه_بفر(لار, اندازه, ډېرپېښ);
  }
}
