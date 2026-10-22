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

#समावेश "llvm/Support/FileOutputBuffer.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/Memory.h"
#समावेश "llvm/Support/TimeProfiler.h"
#समावेश <system_error>

#अगर !defined(_MSC_VER) && !defined(__MINGW32__)
#समावेश <unistd.h>
#अन्यथा
#समावेश <io.h>
#पूर्ण_अगर

using namespace llvm;
using namespace llvm::sys;

namespace {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
श्रेणी On_Disk_बफर : खुला File_Output_बफर {
खुला:
  On_Disk_बफर(StringRef Path, fs::अस्थायी_File अस्थायी, fs::mapped_file_region Buf)
      : File_Output_बफर(Path), बफर(std::move(Buf)), अस्थायी(std::move(अस्थायी)) {}

  uint8_t *get_बफर_Start() स्थिर override { वापस (uint8_t *)बफर.तथ्य(); }

  uint8_t *get_बफर_End() स्थिर override {
    वापस (uint8_t *)बफर.तथ्य() + बफर.आकार_2();
  }

  माप_प्रकार get_बफर_आकार() स्थिर override { वापस बफर.आकार_2(); }

  Error commit() override {
    llvm::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    बफर.unmap();

    // Atomically replace the existing file with the new one.
    वापस अस्थायी.keep(FinalPath);
  }

  ~On_Disk_बफर() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    बफर.unmap();
    consumeError(अस्थायी.discard());
  }

  व्योम discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(अस्थायी.discard());
  }

निजी:
  fs::mapped_file_region बफर;
  fs::अस्थायी_File अस्थायी;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
श्रेणी In_Memory_बफर : खुला File_Output_बफर {
खुला:
  In_Memory_बफर(StringRef Path, MemoryBlock Buf, std::माप_प्रकार Buf_आकार,
                 अचिन्हित Mode)
      : File_Output_बफर(Path), बफर(Buf), बफर_आकार(Buf_आकार),
        Mode(Mode) {}

  uint8_t *get_बफर_Start() स्थिर override { वापस (uint8_t *)बफर.base(); }

  uint8_t *get_बफर_End() स्थिर override {
    वापस (uint8_t *)बफर.base() + बफर_आकार;
  }

  माप_प्रकार get_बफर_आकार() स्थिर override { वापस बफर_आकार; }

  Error commit() override {
    अगर (FinalPath == "-") {
      llvm::म_दो() << StringRef((स्थिर अक्षर *)बफर.base(), बफर_आकार);
      llvm::म_दो().सफाई();
      वापस Error::success();
    }

    using namespace sys::fs;
    पूर्णांक FD;
    std::error_code EC;
    अगर (स्वतः EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      वापस errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/true, /*unbuffered=*/true);
    OS << StringRef((स्थिर अक्षर *)बफर.base(), बफर_आकार);
    वापस Error::success();
  }

निजी:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock बफर;
  माप_प्रकार बफर_आकार;
  अचिन्हित Mode;
};
} // namespace

अटल Expected<std::unique_ptr<In_Memory_बफर>>
create_In_Memory_बफर(StringRef Path, माप_प्रकार आकार, अचिन्हित Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      आकार, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  अगर (EC)
    वापस errorCodeToError(EC);
  वापस std::make_unique<In_Memory_बफर>(Path, MB, आकार, Mode);
}

अटल Expected<std::unique_ptr<File_Output_बफर>>
create_On_Disk_बफर(StringRef Path, माप_प्रकार आकार, अचिन्हित Mode) {
  Expected<fs::अस्थायी_File> FileOrErr =
      fs::अस्थायी_File::create(Path + ".tmp%%%%%%%", Mode);
  अगर (!FileOrErr)
    वापस FileOrErr.takeError();
  fs::अस्थायी_File File = std::move(*FileOrErr);

  अगर (स्वतः EC = fs::resize_file_before_mapping_readwrite(File.FD, आकार)) {
    consumeError(File.discard());
    वापस errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, आकार, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  अगर (EC) {
    consumeError(File.discard());
    वापस create_In_Memory_बफर(Path, आकार, Mode);
  }

  वापस std::make_unique<On_Disk_बफर>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_बफर>>
File_Output_बफर::create(StringRef Path, माप_प्रकार आकार, अचिन्हित Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  अगर (Path == "-")
    वापस create_In_Memory_बफर("-", आकार, /*Mode=*/0);

  अचिन्हित Mode = fs::all_read | fs::all_write;
  अगर (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  अगर (आकार == 0)
    वापस create_In_Memory_बफर(Path, आकार, Mode);

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
  चयन (Stat.type()) {
  हाल fs::file_type::directory_file:
    वापस errorCodeToError(errc::is_a_directory);
  हाल fs::file_type::regular_file:
  हाल fs::file_type::file_not_found:
  हाल fs::file_type::status_error:
    अगर (Flags & F_mmap)
      वापस create_In_Memory_बफर(Path, आकार, Mode);
    अन्यथा
      वापस create_On_Disk_बफर(Path, आकार, Mode);
  शेष:
    वापस create_In_Memory_बफर(Path, आकार, Mode);
  }
}
