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

#అన్తర్భావ "llvm/Support/FileOutputBuffer.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/Memory.h"
#అన్తర్భావ "llvm/Support/TimeProfiler.h"
#అన్తర్భావ <system_error>

#ఒకవేళ !defined(_MSC_VER) && !defined(__MINGW32__)
#అన్తర్భావ <unistd.h>
#లేకపోతే
#అన్తర్భావ <io.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::sys;

నామాకాశ {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
తరగతి On_Disk_బఫరు : బహిరంగ File_Output_బఫరు {
బహిరంగ:
  On_Disk_బఫరు(StringRef మార్గం, fs::తాత్కాలిక_File తాత్కాలిక, fs::mapped_file_region Buf)
      : File_Output_బఫరు(మార్గం), బఫరు(std::move(Buf)), తాత్కాలిక(std::move(తాత్కాలిక)) {}

  uint8_t *get_బఫరు_ప్రారంభం() స్థిరరాశి override { తిరిగి (uint8_t *)బఫరు.దత్తాంశం(); }

  uint8_t *get_బఫరు_చివర() స్థిరరాశి override {
    తిరిగి (uint8_t *)బఫరు.దత్తాంశం() + బఫరు.కొలత_2();
  }

  size_t get_బఫరు_కొలత() స్థిరరాశి override { తిరిగి బఫరు.కొలత_2(); }

  Error commit() override {
    llvm::సమయం_Trace_Scope సమయం_Scope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    బఫరు.unmap();

    // Atomically replace the existing file with the new one.
    తిరిగి తాత్కాలిక.keep(Final_మార్గం);
  }

  ~On_Disk_బఫరు() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    బఫరు.unmap();
    consumeError(తాత్కాలిక.discard());
  }

  శూన్య discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(తాత్కాలిక.discard());
  }

అంతరంగ:
  fs::mapped_file_region బఫరు;
  fs::తాత్కాలిక_File తాత్కాలిక;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
తరగతి లోపల_జ్ఞాపకం_బఫరు : బహిరంగ File_Output_బఫరు {
బహిరంగ:
  లోపల_జ్ఞాపకం_బఫరు(StringRef మార్గం, జ్ఞాపకం_ఖండం Buf, std::size_t Buf_కొలత,
                 గుర్తులేని బహుళకం)
      : File_Output_బఫరు(మార్గం), బఫరు(Buf), బఫరు_కొలత(Buf_కొలత),
        బహుళకం(బహుళకం) {}

  uint8_t *get_బఫరు_ప్రారంభం() స్థిరరాశి override { తిరిగి (uint8_t *)బఫరు.పునాది_2(); }

  uint8_t *get_బఫరు_చివర() స్థిరరాశి override {
    తిరిగి (uint8_t *)బఫరు.పునాది_2() + బఫరు_కొలత;
  }

  size_t get_బఫరు_కొలత() స్థిరరాశి override { తిరిగి బఫరు_కొలత; }

  Error commit() override {
    ఒకవేళ (Final_మార్గం == "-") {
      llvm::outs() << StringRef((స్థిరరాశి అక్షరం *)బఫరు.పునాది_2(), బఫరు_కొలత);
      llvm::outs().flush();
      తిరిగి Error::success();
    }

    ఉపయోగించి నామాకాశ sys::fs;
    పూర్ణాంకం FD;
    std::error_సంకేతం EC;
    ఒకవేళ (స్వయంచాలక EC =
            openFileForWrite(Final_మార్గం, FD, CD_CreateAlways, యొక్క_Delete, బహుళకం))
      తిరిగి error_సంకేతం_గమ్యం_Error(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/నిజం, /*unbuffered=*/నిజం);
    OS << StringRef((స్థిరరాశి అక్షరం *)బఫరు.పునాది_2(), బఫరు_కొలత);
    తిరిగి Error::success();
  }

అంతరంగ:
  // Buffer may actually contain a larger memory block than BufferSize
  Owning_జ్ఞాపకం_ఖండం బఫరు;
  size_t బఫరు_కొలత;
  గుర్తులేని బహుళకం;
};
} // namespace

స్థిర Expected<std::unique_ptr<లోపల_జ్ఞాపకం_బఫరు>>
create_లోపల_జ్ఞాపకం_బఫరు(StringRef మార్గం, size_t కొలత, గుర్తులేని బహుళకం) {
  std::error_సంకేతం EC;
  జ్ఞాపకం_ఖండం MB = జ్ఞాపకం_2::allocate_Mapped_జ్ఞాపకం(
      కొలత, శూన్యసూచిక, sys::జ్ఞాపకం_2::MF_READ | sys::జ్ఞాపకం_2::MF_WRITE, EC);
  ఒకవేళ (EC)
    తిరిగి error_సంకేతం_గమ్యం_Error(EC);
  తిరిగి std::make_unique<లోపల_జ్ఞాపకం_బఫరు>(మార్గం, MB, కొలత, బహుళకం);
}

స్థిర Expected<std::unique_ptr<File_Output_బఫరు>>
create_On_Disk_బఫరు(StringRef మార్గం, size_t కొలత, గుర్తులేని బహుళకం) {
  Expected<fs::తాత్కాలిక_File> FileOrErr =
      fs::తాత్కాలిక_File::create(మార్గం + ".tmp%%%%%%%", బహుళకం);
  ఒకవేళ (!FileOrErr)
    తిరిగి FileOrErr.takeError();
  fs::తాత్కాలిక_File File = std::move(*FileOrErr);

  ఒకవేళ (స్వయంచాలక EC = fs::resize_file_before_mapping_readwrite(File.FD, కొలత)) {
    consumeError(File.discard());
    తిరిగి error_సంకేతం_గమ్యం_Error(EC);
  }

  // Mmap it.
  std::error_సంకేతం EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, కొలత, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  ఒకవేళ (EC) {
    consumeError(File.discard());
    తిరిగి create_లోపల_జ్ఞాపకం_బఫరు(మార్గం, కొలత, బహుళకం);
  }

  తిరిగి std::make_unique<On_Disk_బఫరు>(మార్గం, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<File_Output_బఫరు>>
File_Output_బఫరు::create(StringRef మార్గం, size_t కొలత, గుర్తులేని Flags) {
  // Handle "-" as stdout just like llvm::raw_ostream does.
  ఒకవేళ (మార్గం == "-")
    తిరిగి create_లోపల_జ్ఞాపకం_బఫరు("-", కొలత, /*Mode=*/0);

  గుర్తులేని బహుళకం = fs::all_read | fs::all_write;
  ఒకవేళ (Flags & F_executable)
    బహుళకం |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  ఒకవేళ (కొలత == 0)
    తిరిగి create_లోపల_జ్ఞాపకం_బఫరు(మార్గం, కొలత, బహుళకం);

  fs::file_స్థితి Stat;
  fs::స్థితి_3(మార్గం, Stat);

  // Usually, we want to create OnDiskBuffer to create a temporary file in
  // the same directory as the destination file and atomically replaces it
  // by rename(2).
  //
  // However, if the destination file is a special file, we don't want to
  // use rename (e.g. we don't want to replace /dev/null with a regular
  // file.) If that's the case, we create an in-memory buffer, open the
  // destination file and write to it on commit().
  ఎంపిక (Stat.type()) {
  స్థితి fs::file_type::directory_file:
    తిరిగి error_సంకేతం_గమ్యం_Error(errc::is_a_directory);
  స్థితి fs::file_type::regular_file:
  స్థితి fs::file_type::file_not_దొరికింది:
  స్థితి fs::file_type::స్థితి_error:
    ఒకవేళ (Flags & F_mmap)
      తిరిగి create_లోపల_జ్ఞాపకం_బఫరు(మార్గం, కొలత, బహుళకం);
    లేకపోతే
      తిరిగి create_On_Disk_బఫరు(మార్గం, కొలత, బహుళకం);
  అప్రమేయ:
    తిరిగి create_లోపల_జ్ఞాపకం_బఫరు(మార్గం, కొలత, బహుళకం);
  }
}
