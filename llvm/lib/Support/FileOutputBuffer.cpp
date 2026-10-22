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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileOutputBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Memory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/TimeProfiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>

#ਜੇ !defined(_MSC_VER) && !defined(__MINGW32__)
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <io.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::sys;

ਨਾਮਾਕਾਸ਼ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
ਵਰਗ On_Disk_ਬਫਰ : ਜਨਤਕ File_Output_ਬਫਰ {
ਜਨਤਕ:
  On_Disk_ਬਫਰ(StringRef ਰਸਤਾ, fs::ਆਰਜ਼ੀ_File ਆਰਜ਼ੀ, fs::mapped_file_region Buf)
      : File_Output_ਬਫਰ(ਰਸਤਾ), ਬਫਰ(std::move(Buf)), ਆਰਜ਼ੀ(std::move(ਆਰਜ਼ੀ)) {}

  uint8_t *get_ਬਫਰ_ਸ਼ੁਰੂ() ਅਚਲ override { ਵਾਪਸ (uint8_t *)ਬਫਰ.ਅੰਕੜੇ(); }

  uint8_t *get_ਬਫਰ_ਸਿਰਾ() ਅਚਲ override {
    ਵਾਪਸ (uint8_t *)ਬਫਰ.ਅੰਕੜੇ() + ਬਫਰ.ਮਾਪ_2();
  }

  size_t get_ਬਫਰ_ਮਾਪ() ਅਚਲ override { ਵਾਪਸ ਬਫਰ.ਮਾਪ_2(); }

  Error commit() override {
    llvm::ਸਮਾਂ_Trace_Scope ਸਮਾਂ_Scope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    ਬਫਰ.unmap();

    // Atomically replace the existing file with the new one.
    ਵਾਪਸ ਆਰਜ਼ੀ.keep(Final_ਰਸਤਾ);
  }

  ~On_Disk_ਬਫਰ() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    ਬਫਰ.unmap();
    consumeError(ਆਰਜ਼ੀ.discard());
  }

  ਖਾਲੀ discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(ਆਰਜ਼ੀ.discard());
  }

ਨਿਜੀ:
  fs::mapped_file_region ਬਫਰ;
  fs::ਆਰਜ਼ੀ_File ਆਰਜ਼ੀ;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
ਵਰਗ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ : ਜਨਤਕ File_Output_ਬਫਰ {
ਜਨਤਕ:
  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ(StringRef ਰਸਤਾ, ਸਮ੍ਰਿਤੀ_ਖੰਡ Buf, std::size_t Buf_ਮਾਪ,
                 ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ)
      : File_Output_ਬਫਰ(ਰਸਤਾ), ਬਫਰ(Buf), ਬਫਰ_ਮਾਪ(Buf_ਮਾਪ),
        ਬਹੁਲਕ(ਬਹੁਲਕ) {}

  uint8_t *get_ਬਫਰ_ਸ਼ੁਰੂ() ਅਚਲ override { ਵਾਪਸ (uint8_t *)ਬਫਰ.ਅਧਾਰ_2(); }

  uint8_t *get_ਬਫਰ_ਸਿਰਾ() ਅਚਲ override {
    ਵਾਪਸ (uint8_t *)ਬਫਰ.ਅਧਾਰ_2() + ਬਫਰ_ਮਾਪ;
  }

  size_t get_ਬਫਰ_ਮਾਪ() ਅਚਲ override { ਵਾਪਸ ਬਫਰ_ਮਾਪ; }

  Error commit() override {
    ਜੇ (Final_ਰਸਤਾ == "-") {
      llvm::outs() << StringRef((ਅਚਲ ਅੱਖਰ *)ਬਫਰ.ਅਧਾਰ_2(), ਬਫਰ_ਮਾਪ);
      llvm::outs().flush();
      ਵਾਪਸ Error::success();
    }

    ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ sys::fs;
    ਪੂਰਨ FD;
    std::error_ਕੋਡ EC;
    ਜੇ (ਸਵੈਚਾਲਿਤ EC =
            openFileForWrite(Final_ਰਸਤਾ, FD, CD_CreateAlways, ਦਾ_Delete, ਬਹੁਲਕ))
      ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/ਸੱਚ, /*unbuffered=*/ਸੱਚ);
    OS << StringRef((ਅਚਲ ਅੱਖਰ *)ਬਫਰ.ਅਧਾਰ_2(), ਬਫਰ_ਮਾਪ);
    ਵਾਪਸ Error::success();
  }

ਨਿਜੀ:
  // Buffer may actually contain a larger memory block than BufferSize
  Owning_ਸਮ੍ਰਿਤੀ_ਖੰਡ ਬਫਰ;
  size_t ਬਫਰ_ਮਾਪ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ;
};
} // namespace

ਸਥਿਰ Expected<std::unique_ptr<ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
create_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ(StringRef ਰਸਤਾ, size_t ਮਾਪ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ) {
  std::error_ਕੋਡ EC;
  ਸਮ੍ਰਿਤੀ_ਖੰਡ MB = ਸਮ੍ਰਿਤੀ_2::allocate_Mapped_ਸਮ੍ਰਿਤੀ(
      ਮਾਪ, ਖਾਲੀ_ਸੂਚਕ, sys::ਸਮ੍ਰਿਤੀ_2::MF_READ | sys::ਸਮ੍ਰਿਤੀ_2::MF_WRITE, EC);
  ਜੇ (EC)
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
  ਵਾਪਸ std::make_unique<ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ>(ਰਸਤਾ, MB, ਮਾਪ, ਬਹੁਲਕ);
}

ਸਥਿਰ Expected<std::unique_ptr<File_Output_ਬਫਰ>>
create_On_Disk_ਬਫਰ(StringRef ਰਸਤਾ, size_t ਮਾਪ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ) {
  Expected<fs::ਆਰਜ਼ੀ_File> FileOrErr =
      fs::ਆਰਜ਼ੀ_File::create(ਰਸਤਾ + ".tmp%%%%%%%", ਬਹੁਲਕ);
  ਜੇ (!FileOrErr)
    ਵਾਪਸ FileOrErr.takeError();
  fs::ਆਰਜ਼ੀ_File File = std::move(*FileOrErr);

  ਜੇ (ਸਵੈਚਾਲਿਤ EC = fs::resize_file_before_mapping_readwrite(File.FD, ਮਾਪ)) {
    consumeError(File.discard());
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
  }

  // Mmap it.
  std::error_ਕੋਡ EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, ਮਾਪ, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  ਜੇ (EC) {
    consumeError(File.discard());
    ਵਾਪਸ create_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ(ਰਸਤਾ, ਮਾਪ, ਬਹੁਲਕ);
  }

  ਵਾਪਸ std::make_unique<On_Disk_ਬਫਰ>(ਰਸਤਾ, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_ਬਫਰ>>
File_Output_ਬਫਰ::create(StringRef ਰਸਤਾ, size_t ਮਾਪ, ਬਿਨਾਨਿਸ਼ਾਨੀ Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  ਜੇ (ਰਸਤਾ == "-")
    ਵਾਪਸ create_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ("-", ਮਾਪ, /*Mode=*/0);

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ = fs::all_read | fs::all_write;
  ਜੇ (Flags & F_executable)
    ਬਹੁਲਕ |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  ਜੇ (ਮਾਪ == 0)
    ਵਾਪਸ create_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ(ਰਸਤਾ, ਮਾਪ, ਬਹੁਲਕ);

  fs::file_ਸਥਿਤੀ Stat;
  fs::ਸਥਿਤੀ_2(ਰਸਤਾ, Stat);

  // Usually, we want to create OnDiskBuffer to create a temporary file in
  // the same directory as the destination file and atomically replaces it
  // by rename(2).
  //
  // However, if the destination file is a special file, we don't want to
  // use rename (e.g. we don't want to replace /dev/null with a regular
  // file.) If that's the case, we create an in-memory buffer, open the
  // destination file and write to it on commit().
  ਚੋਣ (Stat.type()) {
  ਹਾਲਤ fs::file_type::directory_file:
    ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(errc::is_a_directory);
  ਹਾਲਤ fs::file_type::regular_file:
  ਹਾਲਤ fs::file_type::file_not_ਮਿਲਿਆ:
  ਹਾਲਤ fs::file_type::ਸਥਿਤੀ_error:
    ਜੇ (Flags & F_mmap)
      ਵਾਪਸ create_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ(ਰਸਤਾ, ਮਾਪ, ਬਹੁਲਕ);
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ create_On_Disk_ਬਫਰ(ਰਸਤਾ, ਮਾਪ, ਬਹੁਲਕ);
  ਮੂਲ:
    ਵਾਪਸ create_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਬਫਰ(ਰਸਤਾ, ਮਾਪ, ਬਹੁਲਕ);
  }
}
