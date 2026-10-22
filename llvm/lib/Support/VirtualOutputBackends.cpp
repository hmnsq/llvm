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

#अन्तर्भाव "llvm/Support/VirtualOutputBackends.h"
#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/LockFileManager.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/VirtualOutputConfig.h"
#अन्तर्भाव "llvm/Support/VirtualOutputError.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::vfs;

शून्य ProxyOutputBackend::anchor() {}
शून्य OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  संरचना NullOutputBackend : सार्वजनिक OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ध्रुव override {
      प्रत्यागम const_cast<NullOutputBackend *>(एतत्);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef मार्गः, std::optional<OutputConfig>) override {
      प्रत्यागम std::make_unique<NullOutputFileImpl>();
    }
  };

  प्रत्यागम makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<तार्किक(StringRef, std::optional<OutputConfig>)> Filter) {
  संरचना FilteringOutputBackend : सार्वजनिक ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef मार्गः,
                   std::optional<OutputConfig> Config) override {
      यदि (Filter(मार्गः, Config))
        प्रत्यागम ProxyOutputBackend::createFileImpl(मार्गः, Config);
      प्रत्यागम std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ध्रुव override {
      प्रत्यागम makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<तार्किक(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      प्रतिज्ञा(एतत्->Filter && "Expected a non-null function");
    }
    std::function<तार्किक(StringRef, std::optional<OutputConfig>)> Filter;
  };

  प्रत्यागम makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  संरचना ProxyOutputBackend1 : सार्वजनिक ProxyOutputBackend {
    उपयुज्य ProxyOutputBackend::ProxyOutputBackend;
  };
  संरचना ProxyOutputBackend2 : सार्वजनिक ProxyOutputBackend {
    उपयुज्य ProxyOutputBackend::ProxyOutputBackend;
  };
  संरचना MirroringOutput final : सार्वजनिक OutputFileImpl, raw_pwrite_प्रवाहः {
    Error keep() final {
      flush();
      प्रत्यागम joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      प्रत्यागम joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_प्रवाहः &getOS() final { प्रत्यागम *एतत्; }

    शून्य write_impl(ध्रुव वर्ण *Ptr, size_t आकार) override {
      F1->getOS().लिख(Ptr, आकार);
      F2->getOS().लिख(Ptr, आकार);
    }
    शून्य pwrite_impl(ध्रुव वर्ण *Ptr, size_t आकार, uint64_t विस्थापनम्) override {
      एतत्->flush();
      F1->getOS().pwrite(Ptr, आकार, विस्थापनम्);
      F2->getOS().pwrite(Ptr, आकार, विस्थापनम्);
    }
    uint64_t current_pos() ध्रुव override { प्रत्यागम F1->getOS().tell(); }
    size_t preferred_बफर_आकार() ध्रुव override {
      प्रत्यागम Preferred_बफर_आकार;
    }
    शून्य reserveExtraSpace(uint64_t Extra_आकार) override {
      F1->getOS().reserveExtraSpace(Extra_आकार);
      F2->getOS().reserveExtraSpace(Extra_आकार);
    }
    तार्किक is_displayed() ध्रुव override {
      प्रत्यागम F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    तार्किक has_colors() ध्रुव override {
      प्रत्यागम F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    शून्य enable_colors(तार्किक enable) override {
      raw_pwrite_प्रवाहः::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_बफर_आकार(std::max(F1->getOS().Get_बफर_आकार(),
                                       F1->getOS().Get_बफर_आकार())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_बफर_आकार;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  संरचना MirroringOutputBackend : सार्वजनिक ProxyOutputBackend1,
                                  सार्वजनिक ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef मार्गः,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      यदि (Error E =
              ProxyOutputBackend1::createFileImpl(मार्गः, Config).moveInto(File1))
        प्रत्यागम std::move(E);
      यदि (Error E =
              ProxyOutputBackend2::createFileImpl(मार्गः, Config).moveInto(File2))
        प्रत्यागम joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      यदि (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        प्रत्यागम std::move(File2);
      }
      यदि (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        प्रत्यागम std::move(File1);
      }
      प्रत्यागम std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ध्रुव override {
      प्रत्यागम IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    शून्य Retain() ध्रुव { ProxyOutputBackend1::Retain(); }
    शून्य Release() ध्रुव { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  प्रतिज्ञा(Backend1 && "Expected actual backend");
  प्रतिज्ञा(Backend2 && "Expected actual backend");
  प्रत्यागम IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

स्थिर OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ध्रुव OnDiskOutputBackend::OutputSettings &Settings) {
  यदि (!Config)
    Config = Settings.DefaultConfig;
  यदि (!Settings.UseTemporaries)
    Config->समुच्चयः_No_Atomic_Write();
  यदि (!Settings.RemoveOnSignal)
    Config->समुच्चयः_No_Discard_On_Signal();
  प्रत्यागम *Config;
}

नामाकाश {
वर्ग OnDiskOutputFile final : सार्वजनिक OutputFileImpl {
सार्वजनिक:
  Error keep() override;
  Error discard() override;
  raw_pwrite_प्रवाहः &getOS() override {
    प्रतिज्ञा(FileOS && "Expected valid file");
    यदि (बफर_OS)
      प्रत्यागम *बफर_OS;
    प्रत्यागम *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error try_गन्तव्यम्_Create_Temporary(std::optional<पूर्णाङ्क> &FD);

  Error initializeFile(std::optional<पूर्णाङ्क> &FD);
  Error initialize_प्रवाहः();
  Error reset();

  OnDiskOutputFile(StringRef Output_मार्गः, std::optional<OutputConfig> Config,
                   ध्रुव OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        Output_मार्गः(Output_मार्गः.str()) {}

  OutputConfig Config;
  ध्रुव std::string Output_मार्गः;
  std::optional<std::string> अस्थायी_मार्गः;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<बफर_ostream> बफर_OS;
};
} // end namespace

स्थिर Error createDirectoriesOnDemand(StringRef Output_मार्गः,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  प्रत्यागम handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    यदि (EC->convert_गन्तव्यम्_Error_संहिता() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      प्रत्यागम Error(std::move(EC));

    StringRef Parent_मार्गः = sys::मार्गः_2::parent_मार्गः(Output_मार्गः);
    यदि (std::error_संहिता EC = sys::fs::create_directories(Parent_मार्गः))
      प्रत्यागम make_error<OutputError>(Parent_मार्गः, EC);
    प्रत्यागम CreateFile();
  });
}

स्थिर sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags सम्बन्धि_2 = sys::fs::सम्बन्धि_None;
  यदि (Config.getTextWithCRLF())
    सम्बन्धि_2 |= sys::fs::सम्बन्धि_Text_With_CRLF;
  अन्यथा यदि (Config.getText())
    सम्बन्धि_2 |= sys::fs::सम्बन्धि_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  यदि (Config.getAppend() && !Config.getAtomicWrite())
    सम्बन्धि_2 |= sys::fs::सम्बन्धि_Append;

  प्रत्यागम सम्बन्धि_2;
}

Error OnDiskOutputFile::try_गन्तव्यम्_Create_Temporary(std::optional<पूर्णाङ्क> &FD) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::मार्गः_2::extension(Output_मार्गः);
  SmallString<128> Model_मार्गः =
      StringRef(Output_मार्गः).drop_पृष्ठभागः(OutputExtension.आकार_2());
  Model_मार्गः += "-%%%%%%%%";
  Model_मार्गः += OutputExtension;
  Model_मार्गः += ".tmp";

  प्रत्यागम createDirectoriesOnDemand(Output_मार्गः, Config, [&]() -> Error {
    पूर्णाङ्क NewFD;
    SmallString<128> Unique_मार्गः;
    sys::fs::OpenFlags सम्बन्धि_2 = generateFlagsFromConfig(Config);
    यदि (std::error_संहिता EC =
            sys::fs::createUniqueFile(Model_मार्गः, NewFD, Unique_मार्गः, सम्बन्धि_2))
      प्रत्यागम make_error<अस्थायी_File_Output_Error>(Model_मार्गः, Output_मार्गः, EC);

    यदि (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Unique_मार्गः);

    अस्थायी_मार्गः = Unique_मार्गः.str().str();
    FD.emplace(NewFD);
    प्रत्यागम Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<पूर्णाङ्क> &FD) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(Output_मार्गः != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  यदि (Config.getAtomicWrite()) {
    sys::fs::file_स्थितिः स्थितिः;
    sys::fs::स्थितिः_2(Output_मार्गः, स्थितिः);
    यदि (sys::fs::exists(स्थितिः)) {
      यदि (!sys::fs::is_regular_file(स्थितिः))
        Config.समुच्चयः_No_Atomic_Write();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      यदि (Config.getAppend() && !sys::fs::can_write(Output_मार्गः))
        प्रत्यागम make_error<OutputError>(
            Output_मार्गः,
            std::make_error_संहिता(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  यदि (Config.getAtomicWrite())
    यदि (!error_गन्तव्यम्_Bool(try_गन्तव्यम्_Create_Temporary(FD)))
      प्रत्यागम Error::success();

  // Not using a temporary file. Open the final output file.
  प्रत्यागम createDirectoriesOnDemand(Output_मार्गः, Config, [&]() -> Error {
    पूर्णाङ्क NewFD;
    sys::fs::OpenFlags सम्बन्धि_2 = generateFlagsFromConfig(Config);
    यदि (std::error_संहिता EC = sys::fs::openFileForWrite(
            Output_मार्गः, NewFD, sys::fs::CD_CreateAlways, सम्बन्धि_2))
      प्रत्यागम convert_गन्तव्यम्_Output_Error(Output_मार्गः, EC);
    FD.emplace(NewFD);

    यदि (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Output_मार्गः);
    प्रत्यागम Error::success();
  });
}

Error OnDiskOutputFile::initialize_प्रवाहः() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  यदि (Output_मार्गः == "-") {
    std::error_संहिता EC;
    FileOS.emplace(Output_मार्गः, EC);
    यदि (EC)
      प्रत्यागम make_error<OutputError>(Output_मार्गः, EC);
  } अन्यथा {
    std::optional<पूर्णाङ्क> FD;
    यदि (Error E = initializeFile(FD))
      प्रत्यागम E;
    FileOS.emplace(*FD, /*shouldClose=*/सत्य);
  }

  // Buffer the stream if necessary.
  यदि (!FileOS->supportsSeeking() && !Config.getText())
    बफर_OS.emplace(*FileOS);

  प्रत्यागम Error::success();
}

नामाकाश {
वर्ग OpenFileRAII {
  स्थिर ध्रुव पूर्णाङ्क InvalidFd = -1;

सार्वजनिक:
  पूर्णाङ्क Fd = InvalidFd;

  ~OpenFileRAII() {
    यदि (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

गणना वर्ग FileDifference : uint8_t {
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

स्थिर Expected<FileDifference>
areFilesDifferent(ध्रुव llvm::Twine &Source, ध्रुव llvm::Twine &Destination) {
  यदि (sys::fs::equivalent(Source, Destination))
    प्रत्यागम FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_स्थितिः Source_स्थितिः;
  // If we can't open the source file, fail.
  यदि (std::error_संहिता EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    प्रत्यागम convert_गन्तव्यम्_Output_Error(Source, EC);

  // If we can't stat the source file, fail.
  यदि (std::error_संहिता EC = sys::fs::स्थितिः_2(SourceFile.Fd, Source_स्थितिः))
    प्रत्यागम convert_गन्तव्यम्_Output_Error(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_स्थितिः Dest_स्थितिः;
  // If we can't open the destination file, report different.
  यदि (std::error_संहिता Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    प्रत्यागम FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  यदि (std::error_संहिता Error = sys::fs::स्थितिः_2(DestFile.Fd, Dest_स्थितिः))
    प्रत्यागम FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t आकार = Source_स्थितिः.get_आकार();
  यदि (आकार != Dest_स्थितिः.get_आकार())
    प्रत्यागम FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  यदि (आकार == 0)
    प्रत्यागम FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_संहिता SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, आकार, 0, SourceRegionErr);
  यदि (SourceRegionErr)
    प्रत्यागम convert_गन्तव्यम्_Output_Error(Source, SourceRegionErr);

  std::error_संहिता DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, आकार, 0, DestRegionErr);

  यदि (DestRegionErr)
    प्रत्यागम FileDifference::DifferentContents;

  यदि (memcmp(SourceRegion.const_दत्त(), DestRegion.const_दत्त(), आकार) != 0)
    प्रत्यागम FileDifference::DifferentContents;

  प्रत्यागम FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  बफर_OS.reset();
  यदि (!FileOS)
    प्रत्यागम Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_संहिता EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
}

Error OnDiskOutputFile::keep() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  यदि (स्वचालित E = reset())
    प्रत्यागम E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    यदि (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(अस्थायी_मार्गः ? *अस्थायी_मार्गः : Output_मार्गः);
  });

  यदि (!अस्थायी_मार्गः)
    प्रत्यागम Error::success();

  // See if we should append instead of move.
  यदि (Config.getAppend() && Output_मार्गः != "-") {
    // Read TempFile for the content to append.
    स्वचालित Content = स्मृतिः_बफर::getFile(*अस्थायी_मार्गः);
    यदि (!Content)
      प्रत्यागम convert_गन्तव्यम्_अस्थायी_File_Output_Error(*अस्थायी_मार्गः, Output_मार्गः,
                                          Content.getError());
    यावत् (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::तालकम्_File_Manager तालकम्_2(Output_मार्गः);
      तार्किक Owned;
      यदि (Error Err = तालकम्_2.try_तालकम्().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        तालकम्_2.unsafeUnlock();
        प्रत्यागम convert_गन्तव्यम्_Output_Error(
            Output_मार्गः, std::make_error_संहिता(std::errc::no_तालकम्_उपलब्धम्));
      }
      यदि (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_संहिता EC;
        llvm::raw_fd_ostream बहिर्गमनम्(Output_मार्गः, EC, llvm::sys::fs::सम्बन्धि_Append);
        यदि (EC)
          प्रत्यागम convert_गन्तव्यम्_Output_Error(Output_मार्गः, EC);
        बहिर्गमनम् << (*Content)->get_बफर();
        बहिर्गमनम्.close();
        तालकम्_2.unsafeUnlock();
        यदि (बहिर्गमनम्.has_error())
          प्रत्यागम convert_गन्तव्यम्_Output_Error(Output_मार्गः, बहिर्गमनम्.error());
        // Remove temp file and done.
        (शून्य)sys::fs::remove(*अस्थायी_मार्गः);
        प्रत्यागम Error::success();
      }
      // Someone else owns the lock on this file, wait.
      विकल्प (तालकम्_2.waitForUnlockFor(std::chrono::seconds(256))) {
      पक्ष Wait_For_Unlock_परिणाम::Success:
        [[fallthrough]];
      पक्ष Wait_For_Unlock_परिणाम::OwnerDied: {
        अनुवर्त; // try again to get the lock.
      }
      पक्ष Wait_For_Unlock_परिणाम::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        तालकम्_2.unsafeUnlock();
        अनुवर्त;
      }
      }
      भङ्ग;
    }
  }

  यदि (Config.getOnlyIfDifferent()) {
    स्वचालित परिणाम = areFilesDifferent(*अस्थायी_मार्गः, Output_मार्गः);
    यदि (!परिणाम)
      प्रत्यागम परिणाम.takeError();
    विकल्प (*परिणाम) {
    पक्ष FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      प्रत्यागम Error::success();

    पक्ष FileDifference::SameContents:
      // Files are identical; remove the source file.
      (शून्य)sys::fs::remove(*अस्थायी_मार्गः);
      प्रत्यागम Error::success();

    पक्ष FileDifference::DifferentContents:
      भङ्ग; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_संहिता RenameEC = sys::fs::rename(*अस्थायी_मार्गः, Output_मार्गः);
  यदि (!RenameEC)
    प्रत्यागम Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*अस्थायी_मार्गः, Output_मार्गः);
  (शून्य)sys::fs::remove(*अस्थायी_मार्गः);

  यदि (!RenameEC)
    प्रत्यागम Error::success();

  प्रत्यागम make_error<अस्थायी_File_Output_Error>(*अस्थायी_मार्गः, Output_मार्गः, RenameEC);
}

Error OnDiskOutputFile::discard() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  यदि (स्वचालित E = reset())
    प्रत्यागम E;

  // Nothing on the filesystem to remove for stdout.
  यदि (Output_मार्गः == "-")
    प्रत्यागम Error::success();

  स्वचालित discard_मार्गः = [&](StringRef मार्गः) {
    std::error_संहिता EC = sys::fs::remove(मार्गः);
    sys::DontRemoveFileOnSignal(मार्गः);
    प्रत्यागम EC;
  };

  // Clean up the file that's in-progress.
  यदि (!अस्थायी_मार्गः)
    प्रत्यागम convert_गन्तव्यम्_Output_Error(Output_मार्गः, discard_मार्गः(Output_मार्गः));
  प्रत्यागम convert_गन्तव्यम्_अस्थायी_File_Output_Error(*अस्थायी_मार्गः, Output_मार्गः,
                                      discard_मार्गः(*अस्थायी_मार्गः));
}

Error OnDiskOutputBackend::makeAbsolute(Small_सदिशम्_Impl<वर्ण> &मार्गः) ध्रुव {
  // FIXME: Should this really call sys::fs::make_absolute?
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();
  प्रत्यागम convert_गन्तव्यम्_Output_Error(StringRef(मार्गः.दत्त(), मार्गः.आकार_2()),
                              sys::fs::make_absolute(मार्गः));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef मार्गः,
                                    std::optional<OutputConfig> Config) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Abs_मार्गः;
  यदि (मार्गः != "-") {
    Abs_मार्गः = मार्गः;
    यदि (Error E = makeAbsolute(Abs_मार्गः))
      प्रत्यागम std::move(E);
    मार्गः = Abs_मार्गः;
  }

  स्वचालित File = std::make_unique<OnDiskOutputFile>(मार्गः, Config, Settings);
  यदि (Error E = File->initialize_प्रवाहः())
    प्रत्यागम std::move(E);

  प्रत्यागम std::move(File);
}
