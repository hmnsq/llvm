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

#अन्तर्भाव "llvm/Support/FileOutputBuffer.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/Memory.h"
#अन्तर्भाव "llvm/Support/TimeProfiler.h"
#अन्तर्भाव <system_error>

#यदि !defined(_MSC_VER) && !defined(__MINGW32__)
#अन्तर्भाव <unistd.h>
#अन्यथा
#अन्तर्भाव <io.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::sys;

नामाकाश {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
वर्ग On_Disk_बफर : सार्वजनिक File_Output_बफर {
सार्वजनिक:
  On_Disk_बफर(StringRef मार्गः, fs::अस्थायी_File अस्थायी, fs::mapped_file_region Buf)
      : File_Output_बफर(मार्गः), बफर(std::move(Buf)), अस्थायी(std::move(अस्थायी)) {}

  uint8_t *get_बफर_Start() ध्रुव override { प्रत्यागम (uint8_t *)बफर.दत्त(); }

  uint8_t *get_बफर_अन्तः() ध्रुव override {
    प्रत्यागम (uint8_t *)बफर.दत्त() + बफर.आकार_2();
  }

  size_t get_बफर_आकार() ध्रुव override { प्रत्यागम बफर.आकार_2(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    बफर.unmap();

    // Atomically replace the existing file with the new one.
    प्रत्यागम अस्थायी.keep(Final_मार्गः);
  }

  ~On_Disk_बफर() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    बफर.unmap();
    consumeError(अस्थायी.discard());
  }

  शून्य discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(अस्थायी.discard());
  }

गुप्त:
  fs::mapped_file_region बफर;
  fs::अस्थायी_File अस्थायी;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
वर्ग अन्तर्_स्मृतिः_बफर : सार्वजनिक File_Output_बफर {
सार्वजनिक:
  अन्तर्_स्मृतिः_बफर(StringRef मार्गः, स्मृतिः_खण्डम् Buf, std::size_t Buf_आकार,
                 अचिह्नित बहुलकम्)
      : File_Output_बफर(मार्गः), बफर(Buf), बफर_आकार(Buf_आकार),
        बहुलकम्(बहुलकम्) {}

  uint8_t *get_बफर_Start() ध्रुव override { प्रत्यागम (uint8_t *)बफर.आधारः_2(); }

  uint8_t *get_बफर_अन्तः() ध्रुव override {
    प्रत्यागम (uint8_t *)बफर.आधारः_2() + बफर_आकार;
  }

  size_t get_बफर_आकार() ध्रुव override { प्रत्यागम बफर_आकार; }

  Error commit() override {
    यदि (Final_मार्गः == "-") {
      llvm::outs() << StringRef((ध्रुव वर्ण *)बफर.आधारः_2(), बफर_आकार);
      llvm::outs().flush();
      प्रत्यागम Error::success();
    }

    उपयुज्य नामाकाश sys::fs;
    पूर्णाङ्क FD;
    std::error_संहिता EC;
    यदि (स्वचालित EC =
            openFileForWrite(Final_मार्गः, FD, CD_CreateAlways, सम्बन्धि_Delete, बहुलकम्))
      प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/सत्य, /*unbuffered=*/सत्य);
    OS << StringRef((ध्रुव वर्ण *)बफर.आधारः_2(), बफर_आकार);
    प्रत्यागम Error::success();
  }

गुप्त:
  // Buffer may actually contain a larger memory block than BufferSize
  Owning_स्मृतिः_खण्डम् बफर;
  size_t बफर_आकार;
  अचिह्नित बहुलकम्;
};
} // namespace

स्थिर Expected<std::unique_ptr<अन्तर्_स्मृतिः_बफर>>
create_अन्तर्_स्मृतिः_बफर(StringRef मार्गः, size_t आकार, अचिह्नित बहुलकम्) {
  std::error_संहिता EC;
  स्मृतिः_खण्डम् MB = स्मृतिः_2::allocate_Mapped_स्मृतिः(
      आकार, शून्यसूचकः, sys::स्मृतिः_2::MF_READ | sys::स्मृतिः_2::MF_WRITE, EC);
  यदि (EC)
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
  प्रत्यागम std::make_unique<अन्तर्_स्मृतिः_बफर>(मार्गः, MB, आकार, बहुलकम्);
}

स्थिर Expected<std::unique_ptr<File_Output_बफर>>
create_On_Disk_बफर(StringRef मार्गः, size_t आकार, अचिह्नित बहुलकम्) {
  Expected<fs::अस्थायी_File> FileOrErr =
      fs::अस्थायी_File::create(मार्गः + ".tmp%%%%%%%", बहुलकम्);
  यदि (!FileOrErr)
    प्रत्यागम FileOrErr.takeError();
  fs::अस्थायी_File File = std::move(*FileOrErr);

  यदि (स्वचालित EC = fs::resize_file_before_mapping_readwrite(File.FD, आकार)) {
    consumeError(File.discard());
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
  }

  // Mmap it.
  std::error_संहिता EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, आकार, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  यदि (EC) {
    consumeError(File.discard());
    प्रत्यागम create_अन्तर्_स्मृतिः_बफर(मार्गः, आकार, बहुलकम्);
  }

  प्रत्यागम std::make_unique<On_Disk_बफर>(मार्गः, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_बफर>>
File_Output_बफर::create(StringRef मार्गः, size_t आकार, अचिह्नित Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  यदि (मार्गः == "-")
    प्रत्यागम create_अन्तर्_स्मृतिः_बफर("-", आकार, /*Mode=*/0);

  अचिह्नित बहुलकम् = fs::all_read | fs::all_write;
  यदि (Flags & F_executable)
    बहुलकम् |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  यदि (आकार == 0)
    प्रत्यागम create_अन्तर्_स्मृतिः_बफर(मार्गः, आकार, बहुलकम्);

  fs::file_स्थितिः Stat;
  fs::स्थितिः_2(मार्गः, Stat);

  // Usually, we want to create OnDiskBuffer to create a temporary file in
  // the same directory as the destination file and atomically replaces it
  // by rename(2).
  //
  // However, if the destination file is a special file, we don't want to
  // use rename (e.g. we don't want to replace /dev/null with a regular
  // file.) If that's the case, we create an in-memory buffer, open the
  // destination file and write to it on commit().
  विकल्प (Stat.type()) {
  पक्ष fs::file_type::directory_file:
    प्रत्यागम error_संहिता_गन्तव्यम्_Error(errc::is_a_directory);
  पक्ष fs::file_type::regular_file:
  पक्ष fs::file_type::file_not_प्राप्तम्:
  पक्ष fs::file_type::स्थितिः_error:
    यदि (Flags & F_mmap)
      प्रत्यागम create_अन्तर्_स्मृतिः_बफर(मार्गः, आकार, बहुलकम्);
    अन्यथा
      प्रत्यागम create_On_Disk_बफर(मार्गः, आकार, बहुलकम्);
  स्वभाव:
    प्रत्यागम create_अन्तर्_स्मृतिः_बफर(मार्गः, आकार, बहुलकम्);
  }
}
