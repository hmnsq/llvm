//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the VirtualOutputBackend types, including:
/// * NullOutputBackend: Outputs to NullOutputBackend are discarded.
/// * FilteringOutputBackend: Filter paths from output.
/// * MirroringOutputBackend: Mirror the output into two different backend.
/// * OnDiskOutputBackend: Write output files to disk.
///
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/VirtualOutputBackends.h"
#అన్తర్భావ "llvm/ADT/ScopeExit.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/IOSandbox.h"
#అన్తర్భావ "llvm/Support/LockFileManager.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/Signals.h"
#అన్తర్భావ "llvm/Support/VirtualOutputConfig.h"
#అన్తర్భావ "llvm/Support/VirtualOutputError.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::vfs;

శూన్య ProxyOutputBackend::anchor() {}
శూన్య OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  నిర్మాణం NullOutputBackend : బహిరంగ OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() స్థిరరాశి override {
      తిరిగి const_cast<NullOutputBackend *>(ఇది);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef మార్గం, std::optional<OutputConfig>) override {
      తిరిగి std::make_unique<NullOutputFileImpl>();
    }
  };

  తిరిగి makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<తార్కిక(StringRef, std::optional<OutputConfig>)> Filter) {
  నిర్మాణం FilteringOutputBackend : బహిరంగ ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef మార్గం,
                   std::optional<OutputConfig> Config) override {
      ఒకవేళ (Filter(మార్గం, Config))
        తిరిగి ProxyOutputBackend::createFileImpl(మార్గం, Config);
      తిరిగి std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() స్థిరరాశి override {
      తిరిగి makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<తార్కిక(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      ప్రతిజ్ఞా(ఇది->Filter && "Expected a non-null function");
    }
    std::function<తార్కిక(StringRef, std::optional<OutputConfig>)> Filter;
  };

  తిరిగి makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  నిర్మాణం ProxyOutputBackend1 : బహిరంగ ProxyOutputBackend {
    ఉపయోగించి ProxyOutputBackend::ProxyOutputBackend;
  };
  నిర్మాణం ProxyOutputBackend2 : బహిరంగ ProxyOutputBackend {
    ఉపయోగించి ProxyOutputBackend::ProxyOutputBackend;
  };
  నిర్మాణం MirroringOutput final : బహిరంగ OutputFileImpl, raw_pwrite_ప్రవాహం {
    Error keep() final {
      flush();
      తిరిగి joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      తిరిగి joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_ప్రవాహం &getOS() final { తిరిగి *ఇది; }

    శూన్య write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) override {
      F1->getOS().లిఖ(Ptr, కొలత);
      F2->getOS().లిఖ(Ptr, కొలత);
    }
    శూన్య pwrite_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత, uint64_t స్థానభ్రంశం) override {
      ఇది->flush();
      F1->getOS().pwrite(Ptr, కొలత, స్థానభ్రంశం);
      F2->getOS().pwrite(Ptr, కొలత, స్థానభ్రంశం);
    }
    uint64_t current_pos() స్థిరరాశి override { తిరిగి F1->getOS().tell(); }
    size_t preferred_బఫరు_కొలత() స్థిరరాశి override {
      తిరిగి Preferred_బఫరు_కొలత;
    }
    శూన్య reserveExtraSpace(uint64_t Extra_కొలత) override {
      F1->getOS().reserveExtraSpace(Extra_కొలత);
      F2->getOS().reserveExtraSpace(Extra_కొలత);
    }
    తార్కిక is_displayed() స్థిరరాశి override {
      తిరిగి F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    తార్కిక has_colors() స్థిరరాశి override {
      తిరిగి F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    శూన్య enable_colors(తార్కిక enable) override {
      raw_pwrite_ప్రవాహం::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_బఫరు_కొలత(std::max(F1->getOS().Get_బఫరు_కొలత(),
                                       F1->getOS().Get_బఫరు_కొలత())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_బఫరు_కొలత;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  నిర్మాణం MirroringOutputBackend : బహిరంగ ProxyOutputBackend1,
                                  బహిరంగ ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef మార్గం,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      ఒకవేళ (Error E =
              ProxyOutputBackend1::createFileImpl(మార్గం, Config).moveInto(File1))
        తిరిగి std::move(E);
      ఒకవేళ (Error E =
              ProxyOutputBackend2::createFileImpl(మార్గం, Config).moveInto(File2))
        తిరిగి joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      ఒకవేళ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        తిరిగి std::move(File2);
      }
      ఒకవేళ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        తిరిగి std::move(File1);
      }
      తిరిగి std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() స్థిరరాశి override {
      తిరిగి IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    శూన్య Retain() స్థిరరాశి { ProxyOutputBackend1::Retain(); }
    శూన్య Release() స్థిరరాశి { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  ప్రతిజ్ఞా(Backend1 && "Expected actual backend");
  ప్రతిజ్ఞా(Backend2 && "Expected actual backend");
  తిరిగి IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

స్థిర OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              స్థిరరాశి OnDiskOutputBackend::OutputSettings &Settings) {
  ఒకవేళ (!Config)
    Config = Settings.DefaultConfig;
  ఒకవేళ (!Settings.UseTemporaries)
    Config->సమితి_No_Atomic_Write();
  ఒకవేళ (!Settings.RemoveOnSignal)
    Config->సమితి_No_Discard_On_Signal();
  తిరిగి *Config;
}

నామాకాశ {
తరగతి OnDiskOutputFile final : బహిరంగ OutputFileImpl {
బహిరంగ:
  Error keep() override;
  Error discard() override;
  raw_pwrite_ప్రవాహం &getOS() override {
    ప్రతిజ్ఞా(FileOS && "Expected valid file");
    ఒకవేళ (బఫరు_OS)
      తిరిగి *బఫరు_OS;
    తిరిగి *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error try_గమ్యం_Create_Temporary(std::optional<పూర్ణాంకం> &FD);

  Error initializeFile(std::optional<పూర్ణాంకం> &FD);
  Error initialize_ప్రవాహం();
  Error reset();

  OnDiskOutputFile(StringRef Output_మార్గం, std::optional<OutputConfig> Config,
                   స్థిరరాశి OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        Output_మార్గం(Output_మార్గం.str()) {}

  OutputConfig Config;
  స్థిరరాశి std::string Output_మార్గం;
  std::optional<std::string> తాత్కాలిక_మార్గం;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<బఫరు_ostream> బఫరు_OS;
};
} // end namespace

స్థిర Error createDirectoriesOnDemand(StringRef Output_మార్గం,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  తిరిగి handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    ఒకవేళ (EC->convert_గమ్యం_Error_సంకేతం() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      తిరిగి Error(std::move(EC));

    StringRef మాతృ_మార్గం_2 = sys::మార్గం_2::మాతృ_మార్గం(Output_మార్గం);
    ఒకవేళ (std::error_సంకేతం EC = sys::fs::create_directories(మాతృ_మార్గం_2))
      తిరిగి make_error<OutputError>(మాతృ_మార్గం_2, EC);
    తిరిగి CreateFile();
  });
}

స్థిర sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags యొక్క_2 = sys::fs::యొక్క_None;
  ఒకవేళ (Config.get_పాఠ్యం_With_CRLF())
    యొక్క_2 |= sys::fs::యొక్క_పాఠ్యం_With_CRLF;
  లేకపోతే ఒకవేళ (Config.get_పాఠ్యం())
    యొక్క_2 |= sys::fs::యొక్క_పాఠ్యం;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  ఒకవేళ (Config.getAppend() && !Config.getAtomicWrite())
    యొక్క_2 |= sys::fs::యొక్క_Append;

  తిరిగి యొక్క_2;
}

Error OnDiskOutputFile::try_గమ్యం_Create_Temporary(std::optional<పూర్ణాంకం> &FD) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::మార్గం_2::extension(Output_మార్గం);
  SmallString<128> Model_మార్గం =
      StringRef(Output_మార్గం).drop_వెనుక(OutputExtension.కొలత_2());
  Model_మార్గం += "-%%%%%%%%";
  Model_మార్గం += OutputExtension;
  Model_మార్గం += ".tmp";

  తిరిగి createDirectoriesOnDemand(Output_మార్గం, Config, [&]() -> Error {
    పూర్ణాంకం NewFD;
    SmallString<128> Unique_మార్గం;
    sys::fs::OpenFlags యొక్క_2 = generateFlagsFromConfig(Config);
    ఒకవేళ (std::error_సంకేతం EC =
            sys::fs::createUniqueFile(Model_మార్గం, NewFD, Unique_మార్గం, యొక్క_2))
      తిరిగి make_error<తాత్కాలిక_File_Output_Error>(Model_మార్గం, Output_మార్గం, EC);

    ఒకవేళ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Unique_మార్గం);

    తాత్కాలిక_మార్గం = Unique_మార్గం.str().str();
    FD.emplace(NewFD);
    తిరిగి Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<పూర్ణాంకం> &FD) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ప్రతిజ్ఞా(Output_మార్గం != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  ఒకవేళ (Config.getAtomicWrite()) {
    sys::fs::file_స్థితి స్థితి_2;
    sys::fs::స్థితి_3(Output_మార్గం, స్థితి_2);
    ఒకవేళ (sys::fs::exists(స్థితి_2)) {
      ఒకవేళ (!sys::fs::is_regular_file(స్థితి_2))
        Config.సమితి_No_Atomic_Write();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      ఒకవేళ (Config.getAppend() && !sys::fs::can_write(Output_మార్గం))
        తిరిగి make_error<OutputError>(
            Output_మార్గం,
            std::make_error_సంకేతం(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  ఒకవేళ (Config.getAtomicWrite())
    ఒకవేళ (!error_గమ్యం_Bool(try_గమ్యం_Create_Temporary(FD)))
      తిరిగి Error::success();

  // Not using a temporary file. Open the final output file.
  తిరిగి createDirectoriesOnDemand(Output_మార్గం, Config, [&]() -> Error {
    పూర్ణాంకం NewFD;
    sys::fs::OpenFlags యొక్క_2 = generateFlagsFromConfig(Config);
    ఒకవేళ (std::error_సంకేతం EC = sys::fs::openFileForWrite(
            Output_మార్గం, NewFD, sys::fs::CD_CreateAlways, యొక్క_2))
      తిరిగి convert_గమ్యం_Output_Error(Output_మార్గం, EC);
    FD.emplace(NewFD);

    ఒకవేళ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Output_మార్గం);
    తిరిగి Error::success();
  });
}

Error OnDiskOutputFile::initialize_ప్రవాహం() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  ఒకవేళ (Output_మార్గం == "-") {
    std::error_సంకేతం EC;
    FileOS.emplace(Output_మార్గం, EC);
    ఒకవేళ (EC)
      తిరిగి make_error<OutputError>(Output_మార్గం, EC);
  } లేకపోతే {
    std::optional<పూర్ణాంకం> FD;
    ఒకవేళ (Error E = initializeFile(FD))
      తిరిగి E;
    FileOS.emplace(*FD, /*shouldClose=*/నిజం);
  }

  // Buffer the stream if necessary.
  ఒకవేళ (!FileOS->supportsSeeking() && !Config.get_పాఠ్యం())
    బఫరు_OS.emplace(*FileOS);

  తిరిగి Error::success();
}

నామాకాశ {
తరగతి OpenFileRAII {
  స్థిర స్థిరరాశి పూర్ణాంకం InvalidFd = -1;

బహిరంగ:
  పూర్ణాంకం Fd = InvalidFd;

  ~OpenFileRAII() {
    ఒకవేళ (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

గణన తరగతి FileDifference : uint8_t {
  /// The source and destination paths refer to the exact same file.
  IdenticalFile,
  /// The source and destination paths refer to separate files with identical
  /// contents.
  SameContents,
  /// The source and destination paths refer to separate files with different
  /// contents.
  DifferentContents
};
} // end anonymous namespace

స్థిర Expected<FileDifference>
areFilesDifferent(స్థిరరాశి llvm::Twine &ఆధారం, స్థిరరాశి llvm::Twine &Destination) {
  ఒకవేళ (sys::fs::equivalent(ఆధారం, Destination))
    తిరిగి FileDifference::IdenticalFile;

  OpenFileRAII ఆధారం_File;
  sys::fs::file_స్థితి ఆధారం_స్థితి;
  // If we can't open the source file, fail.
  ఒకవేళ (std::error_సంకేతం EC = sys::fs::openFileForRead(ఆధారం, ఆధారం_File.Fd))
    తిరిగి convert_గమ్యం_Output_Error(ఆధారం, EC);

  // If we can't stat the source file, fail.
  ఒకవేళ (std::error_సంకేతం EC = sys::fs::స్థితి_3(ఆధారం_File.Fd, ఆధారం_స్థితి))
    తిరిగి convert_గమ్యం_Output_Error(ఆధారం, EC);

  OpenFileRAII DestFile;
  sys::fs::file_స్థితి Dest_స్థితి;
  // If we can't open the destination file, report different.
  ఒకవేళ (std::error_సంకేతం Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    తిరిగి FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  ఒకవేళ (std::error_సంకేతం Error = sys::fs::స్థితి_3(DestFile.Fd, Dest_స్థితి))
    తిరిగి FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t కొలత = ఆధారం_స్థితి.get_కొలత();
  ఒకవేళ (కొలత != Dest_స్థితి.get_కొలత())
    తిరిగి FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  ఒకవేళ (కొలత == 0)
    తిరిగి FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_సంకేతం ఆధారం_Region_Err;
  sys::fs::mapped_file_region ఆధారం_Region(
      sys::fs::convertFDToNativeFile(ఆధారం_File.Fd),
      sys::fs::mapped_file_region::readonly, కొలత, 0, ఆధారం_Region_Err);
  ఒకవేళ (ఆధారం_Region_Err)
    తిరిగి convert_గమ్యం_Output_Error(ఆధారం, ఆధారం_Region_Err);

  std::error_సంకేతం DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, కొలత, 0, DestRegionErr);

  ఒకవేళ (DestRegionErr)
    తిరిగి FileDifference::DifferentContents;

  ఒకవేళ (memcmp(ఆధారం_Region.const_దత్తాంశం(), DestRegion.const_దత్తాంశం(), కొలత) != 0)
    తిరిగి FileDifference::DifferentContents;

  తిరిగి FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  బఫరు_OS.reset();
  ఒకవేళ (!FileOS)
    తిరిగి Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_సంకేతం EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  తిరిగి error_సంకేతం_గమ్యం_Error(EC);
}

Error OnDiskOutputFile::keep() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ఒకవేళ (స్వయంచాలక E = reset())
    తిరిగి E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    ఒకవేళ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(తాత్కాలిక_మార్గం ? *తాత్కాలిక_మార్గం : Output_మార్గం);
  });

  ఒకవేళ (!తాత్కాలిక_మార్గం)
    తిరిగి Error::success();

  // See if we should append instead of move.
  ఒకవేళ (Config.getAppend() && Output_మార్గం != "-") {
    // Read TempFile for the content to append.
    స్వయంచాలక Content = జ్ఞాపకం_బఫరు::getFile(*తాత్కాలిక_మార్గం);
    ఒకవేళ (!Content)
      తిరిగి convert_గమ్యం_తాత్కాలిక_File_Output_Error(*తాత్కాలిక_మార్గం, Output_మార్గం,
                                          Content.getError());
    అయితే (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::తాళం_File_Manager తాళం_3(Output_మార్గం);
      తార్కిక Owned;
      ఒకవేళ (Error Err = తాళం_3.try_తాళం().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        తాళం_3.unsafeUnlock();
        తిరిగి convert_గమ్యం_Output_Error(
            Output_మార్గం, std::make_error_సంకేతం(std::errc::no_తాళం_అందుబాటు));
      }
      ఒకవేళ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_సంకేతం EC;
        llvm::raw_fd_ostream బహిర్గతం(Output_మార్గం, EC, llvm::sys::fs::యొక్క_Append);
        ఒకవేళ (EC)
          తిరిగి convert_గమ్యం_Output_Error(Output_మార్గం, EC);
        బహిర్గతం << (*Content)->get_బఫరు();
        బహిర్గతం.close();
        తాళం_3.unsafeUnlock();
        ఒకవేళ (బహిర్గతం.has_error())
          తిరిగి convert_గమ్యం_Output_Error(Output_మార్గం, బహిర్గతం.error());
        // Remove temp file and done.
        (శూన్య)sys::fs::remove(*తాత్కాలిక_మార్గం);
        తిరిగి Error::success();
      }
      // Someone else owns the lock on this file, wait.
      ఎంపిక (తాళం_3.waitForUnlockFor(std::chrono::seconds(256))) {
      స్థితి Wait_For_Unlock_ఫలితం::Success:
        [[fallthrough]];
      స్థితి Wait_For_Unlock_ఫలితం::OwnerDied: {
        కొనసాగు; // try again to get the lock.
      }
      స్థితి Wait_For_Unlock_ఫలితం::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        తాళం_3.unsafeUnlock();
        కొనసాగు;
      }
      }
      ఆపు;
    }
  }

  ఒకవేళ (Config.getOnlyIfDifferent()) {
    స్వయంచాలక ఫలితం = areFilesDifferent(*తాత్కాలిక_మార్గం, Output_మార్గం);
    ఒకవేళ (!ఫలితం)
      తిరిగి ఫలితం.takeError();
    ఎంపిక (*ఫలితం) {
    స్థితి FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      తిరిగి Error::success();

    స్థితి FileDifference::SameContents:
      // Files are identical; remove the source file.
      (శూన్య)sys::fs::remove(*తాత్కాలిక_మార్గం);
      తిరిగి Error::success();

    స్థితి FileDifference::DifferentContents:
      ఆపు; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_సంకేతం RenameEC = sys::fs::rename(*తాత్కాలిక_మార్గం, Output_మార్గం);
  ఒకవేళ (!RenameEC)
    తిరిగి Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*తాత్కాలిక_మార్గం, Output_మార్గం);
  (శూన్య)sys::fs::remove(*తాత్కాలిక_మార్గం);

  ఒకవేళ (!RenameEC)
    తిరిగి Error::success();

  తిరిగి make_error<తాత్కాలిక_File_Output_Error>(*తాత్కాలిక_మార్గం, Output_మార్గం, RenameEC);
}

Error OnDiskOutputFile::discard() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ఒకవేళ (స్వయంచాలక E = reset())
    తిరిగి E;

  // Nothing on the filesystem to remove for stdout.
  ఒకవేళ (Output_మార్గం == "-")
    తిరిగి Error::success();

  స్వయంచాలక discard_మార్గం = [&](StringRef మార్గం) {
    std::error_సంకేతం EC = sys::fs::remove(మార్గం);
    sys::DontRemoveFileOnSignal(మార్గం);
    తిరిగి EC;
  };

  // Clean up the file that's in-progress.
  ఒకవేళ (!తాత్కాలిక_మార్గం)
    తిరిగి convert_గమ్యం_Output_Error(Output_మార్గం, discard_మార్గం(Output_మార్గం));
  తిరిగి convert_గమ్యం_తాత్కాలిక_File_Output_Error(*తాత్కాలిక_మార్గం, Output_మార్గం,
                                      discard_మార్గం(*తాత్కాలిక_మార్గం));
}

Error OnDiskOutputBackend::makeAbsolute(Small_సదిశ_Impl<అక్షరం> &మార్గం) స్థిరరాశి {
  // FIXME: Should this really call sys::fs::make_absolute?
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();
  తిరిగి convert_గమ్యం_Output_Error(StringRef(మార్గం.దత్తాంశం(), మార్గం.కొలత_2()),
                              sys::fs::make_absolute(మార్గం));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef మార్గం,
                                    std::optional<OutputConfig> Config) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Abs_మార్గం;
  ఒకవేళ (మార్గం != "-") {
    Abs_మార్గం = మార్గం;
    ఒకవేళ (Error E = makeAbsolute(Abs_మార్గం))
      తిరిగి std::move(E);
    మార్గం = Abs_మార్గం;
  }

  స్వయంచాలక File = std::make_unique<OnDiskOutputFile>(మార్గం, Config, Settings);
  ఒకవేళ (Error E = File->initialize_ప్రవాహం())
    తిరిగి std::move(E);

  తిరిగి std::move(File);
}
