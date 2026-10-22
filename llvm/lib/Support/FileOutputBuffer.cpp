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

#ಅನ್ತರ್ಭಾವ "llvm/Support/FileOutputBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Memory.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/TimeProfiler.h"
#ಅನ್ತರ್ಭಾವ <system_error>

#ಆದರೆ !defined(_MSC_VER) && !defined(__MINGW32__)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <io.h>
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::sys;

ನಾಮಾಕಾಶ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
ವರ್ಗ On_Disk_ಬಫರ್ : ಸಾರ್ವಜನಿಕ File_Output_ಬಫರ್ {
ಸಾರ್ವಜನಿಕ:
  On_Disk_ಬಫರ್(StringRef ಮಾರ್ಗ, fs::ತಾತ್ಕಾಲಿಕ_File ತಾತ್ಕಾಲಿಕ, fs::mapped_file_region Buf)
      : File_Output_ಬಫರ್(ಮಾರ್ಗ), ಬಫರ್(std::move(Buf)), ತಾತ್ಕಾಲಿಕ(std::move(ತಾತ್ಕಾಲಿಕ)) {}

  uint8_t *get_ಬಫರ್_ಆರಂಭ() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ (uint8_t *)ಬಫರ್.ದತ್ತಾಂಶ(); }

  uint8_t *get_ಬಫರ್_ಕೊನೆ() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ (uint8_t *)ಬಫರ್.ದತ್ತಾಂಶ() + ಬಫರ್.ಗಾತ್ರ_3();
  }

  size_t get_ಬಫರ್_ಗಾತ್ರ() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ ಬಫರ್.ಗಾತ್ರ_3(); }

  Error commit() override {
    llvm::ಸಮಯ_Trace_Scope ಸಮಯ_Scope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    ಬಫರ್.unmap();

    // Atomically replace the existing file with the new one.
    ಹಿಂತಿರುಗಿ ತಾತ್ಕಾಲಿಕ.keep(Final_ಮಾರ್ಗ);
  }

  ~On_Disk_ಬಫರ್() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    ಬಫರ್.unmap();
    consumeError(ತಾತ್ಕಾಲಿಕ.discard());
  }

  ಶೂನ್ಯ discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(ತಾತ್ಕಾಲಿಕ.discard());
  }

ಖಾಸಗಿ:
  fs::mapped_file_region ಬಫರ್;
  fs::ತಾತ್ಕಾಲಿಕ_File ತಾತ್ಕಾಲಿಕ;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
ವರ್ಗ ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್ : ಸಾರ್ವಜನಿಕ File_Output_ಬಫರ್ {
ಸಾರ್ವಜನಿಕ:
  ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್(StringRef ಮಾರ್ಗ, ಸ್ಮರಣೆ_ಖಂಡ Buf, std::size_t Buf_ಗಾತ್ರ,
                 ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ)
      : File_Output_ಬಫರ್(ಮಾರ್ಗ), ಬಫರ್(Buf), ಬಫರ್_ಗಾತ್ರ(Buf_ಗಾತ್ರ),
        ಬಹುಲಕ(ಬಹುಲಕ) {}

  uint8_t *get_ಬಫರ್_ಆರಂಭ() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ (uint8_t *)ಬಫರ್.ತಳ_2(); }

  uint8_t *get_ಬಫರ್_ಕೊನೆ() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ (uint8_t *)ಬಫರ್.ತಳ_2() + ಬಫರ್_ಗಾತ್ರ;
  }

  size_t get_ಬಫರ್_ಗಾತ್ರ() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ ಬಫರ್_ಗಾತ್ರ; }

  Error commit() override {
    ಆದರೆ (Final_ಮಾರ್ಗ == "-") {
      llvm::outs() << StringRef((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ಬಫರ್.ತಳ_2(), ಬಫರ್_ಗಾತ್ರ);
      llvm::outs().flush();
      ಹಿಂತಿರುಗಿ Error::success();
    }

    ಬಳಸಿ ನಾಮಾಕಾಶ sys::fs;
    ಪೂರ್ಣಾಂಕ FD;
    std::error_ಸಂಕೇತ EC;
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC =
            openFileForWrite(Final_ಮಾರ್ಗ, FD, CD_CreateAlways, ದ_Delete, ಬಹುಲಕ))
      ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/ನಿಜ, /*unbuffered=*/ನಿಜ);
    OS << StringRef((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ಬಫರ್.ತಳ_2(), ಬಫರ್_ಗಾತ್ರ);
    ಹಿಂತಿರುಗಿ Error::success();
  }

ಖಾಸಗಿ:
  // Buffer may actually contain a larger memory block than BufferSize
  Owning_ಸ್ಮರಣೆ_ಖಂಡ ಬಫರ್;
  size_t ಬಫರ್_ಗಾತ್ರ;
  ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ;
};
} // namespace

ಸ್ಥಿರ Expected<std::unique_ptr<ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್>>
create_ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್(StringRef ಮಾರ್ಗ, size_t ಗಾತ್ರ_2, ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ) {
  std::error_ಸಂಕೇತ EC;
  ಸ್ಮರಣೆ_ಖಂಡ MB = ಸ್ಮರಣೆ_2::allocate_Mapped_ಸ್ಮರಣೆ(
      ಗಾತ್ರ_2, ಶೂನ್ಯಸೂಚಕ, sys::ಸ್ಮರಣೆ_2::MF_READ | sys::ಸ್ಮರಣೆ_2::MF_WRITE, EC);
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
  ಹಿಂತಿರುಗಿ std::make_unique<ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್>(ಮಾರ್ಗ, MB, ಗಾತ್ರ_2, ಬಹುಲಕ);
}

ಸ್ಥಿರ Expected<std::unique_ptr<File_Output_ಬಫರ್>>
create_On_Disk_ಬಫರ್(StringRef ಮಾರ್ಗ, size_t ಗಾತ್ರ_2, ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ) {
  Expected<fs::ತಾತ್ಕಾಲಿಕ_File> FileOrErr =
      fs::ತಾತ್ಕಾಲಿಕ_File::create(ಮಾರ್ಗ + ".tmp%%%%%%%", ಬಹುಲಕ);
  ಆದರೆ (!FileOrErr)
    ಹಿಂತಿರುಗಿ FileOrErr.takeError();
  fs::ತಾತ್ಕಾಲಿಕ_File File = std::move(*FileOrErr);

  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = fs::resize_file_before_mapping_readwrite(File.FD, ಗಾತ್ರ_2)) {
    consumeError(File.discard());
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
  }

  // Mmap it.
  std::error_ಸಂಕೇತ EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, ಗಾತ್ರ_2, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  ಆದರೆ (EC) {
    consumeError(File.discard());
    ಹಿಂತಿರುಗಿ create_ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್(ಮಾರ್ಗ, ಗಾತ್ರ_2, ಬಹುಲಕ);
  }

  ಹಿಂತಿರುಗಿ std::make_unique<On_Disk_ಬಫರ್>(ಮಾರ್ಗ, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_ಬಫರ್>>
File_Output_ಬಫರ್::create(StringRef ಮಾರ್ಗ, size_t ಗಾತ್ರ_2, ಚಿಹ್ನೆರಹಿತ Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  ಆದರೆ (ಮಾರ್ಗ == "-")
    ಹಿಂತಿರುಗಿ create_ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್("-", ಗಾತ್ರ_2, /*Mode=*/0);

  ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ = fs::all_read | fs::all_write;
  ಆದರೆ (Flags & F_executable)
    ಬಹುಲಕ |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  ಆದರೆ (ಗಾತ್ರ_2 == 0)
    ಹಿಂತಿರುಗಿ create_ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್(ಮಾರ್ಗ, ಗಾತ್ರ_2, ಬಹುಲಕ);

  fs::file_ಸ್ಥಿತಿ Stat;
  fs::ಸ್ಥಿತಿ_3(ಮಾರ್ಗ, Stat);

  // Usually, we want to create OnDiskBuffer to create a temporary file in
  // the same directory as the destination file and atomically replaces it
  // by rename(2).
  //
  // However, if the destination file is a special file, we don't want to
  // use rename (e.g. we don't want to replace /dev/null with a regular
  // file.) If that's the case, we create an in-memory buffer, open the
  // destination file and write to it on commit().
  ಆಯ್ಕೆ (Stat.type()) {
  ಸ್ಥಿತಿ fs::file_type::directory_file:
    ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(errc::is_a_directory);
  ಸ್ಥಿತಿ fs::file_type::regular_file:
  ಸ್ಥಿತಿ fs::file_type::file_not_ಸಿಕ್ಕಿತು:
  ಸ್ಥಿತಿ fs::file_type::ಸ್ಥಿತಿ_error:
    ಆದರೆ (Flags & F_mmap)
      ಹಿಂತಿರುಗಿ create_ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್(ಮಾರ್ಗ, ಗಾತ್ರ_2, ಬಹುಲಕ);
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ create_On_Disk_ಬಫರ್(ಮಾರ್ಗ, ಗಾತ್ರ_2, ಬಹುಲಕ);
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ create_ಒಳ_ಸ್ಮರಣೆ_ಬಫರ್(ಮಾರ್ಗ, ಗಾತ್ರ_2, ಬಹುಲಕ);
  }
}
