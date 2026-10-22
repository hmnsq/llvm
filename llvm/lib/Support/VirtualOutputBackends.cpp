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

#ضمن "llvm/Support/VirtualOutputBackends.h"
#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/LockFileManager.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/VirtualOutputConfig.h"
#ضمن "llvm/Support/VirtualOutputError.h"

ورتوں نطاق llvm;
ورتوں نطاق llvm::vfs;

فراغ ProxyOutputBackend::anchor() {}
فراغ OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  بنية NullOutputBackend : عام OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      أرجع const_cast<NullOutputBackend *>(هذا);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef رستہ, std::optional<OutputConfig>) override {
      أرجع std::make_unique<NullOutputFileImpl>();
    }
  };

  أرجع makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<منطقي(StringRef, std::optional<OutputConfig>)> Filter) {
  بنية FilteringOutputBackend : عام ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef رستہ,
                   std::optional<OutputConfig> Config) override {
      إذا (Filter(رستہ, Config))
        أرجع ProxyOutputBackend::createFileImpl(رستہ, Config);
      أرجع std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      أرجع makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<منطقي(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      أكد(هذا->Filter && "Expected a non-null function");
    }
    std::function<منطقي(StringRef, std::optional<OutputConfig>)> Filter;
  };

  أرجع makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  بنية ProxyOutputBackend1 : عام ProxyOutputBackend {
    ورتوں ProxyOutputBackend::ProxyOutputBackend;
  };
  بنية ProxyOutputBackend2 : عام ProxyOutputBackend {
    ورتوں ProxyOutputBackend::ProxyOutputBackend;
  };
  بنية MirroringOutput final : عام OutputFileImpl, raw_pwrite_دھارا {
    Error keep() final {
      flush();
      أرجع joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      أرجع joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_دھارا &getOS() final { أرجع *هذا; }

    فراغ write_impl(ثابت حرف *Ptr, size_t ناپ) override {
      F1->getOS().اكتب(Ptr, ناپ);
      F2->getOS().اكتب(Ptr, ناپ);
    }
    فراغ pwrite_impl(ثابت حرف *Ptr, size_t ناپ, uint64_t وتھ) override {
      هذا->flush();
      F1->getOS().pwrite(Ptr, ناپ, وتھ);
      F2->getOS().pwrite(Ptr, ناپ, وتھ);
    }
    uint64_t current_pos() ثابت override { أرجع F1->getOS().tell(); }
    size_t preferred_بفر_ناپ() ثابت override {
      أرجع Preferred_بفر_ناپ;
    }
    فراغ reserveExtraSpace(uint64_t Extra_ناپ) override {
      F1->getOS().reserveExtraSpace(Extra_ناپ);
      F2->getOS().reserveExtraSpace(Extra_ناپ);
    }
    منطقي is_displayed() ثابت override {
      أرجع F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    منطقي has_colors() ثابت override {
      أرجع F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    فراغ enable_colors(منطقي enable) override {
      raw_pwrite_دھارا::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_بفر_ناپ(std::max(F1->getOS().Get_بفر_ناپ(),
                                       F1->getOS().Get_بفر_ناپ())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_بفر_ناپ;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  بنية MirroringOutputBackend : عام ProxyOutputBackend1,
                                  عام ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef رستہ,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      إذا (Error E =
              ProxyOutputBackend1::createFileImpl(رستہ, Config).moveInto(File1))
        أرجع std::move(E);
      إذا (Error E =
              ProxyOutputBackend2::createFileImpl(رستہ, Config).moveInto(File2))
        أرجع joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      إذا (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        أرجع std::move(File2);
      }
      إذا (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        أرجع std::move(File1);
      }
      أرجع std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      أرجع IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    فراغ Retain() ثابت { ProxyOutputBackend1::Retain(); }
    فراغ Release() ثابت { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  أكد(Backend1 && "Expected actual backend");
  أكد(Backend2 && "Expected actual backend");
  أرجع IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ساكن OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ثابت OnDiskOutputBackend::OutputSettings &Settings) {
  إذا (!Config)
    Config = Settings.DefaultConfig;
  إذا (!Settings.UseTemporaries)
    Config->سیٹ_No_Atomic_Write();
  إذا (!Settings.RemoveOnSignal)
    Config->سیٹ_No_Discard_On_Signal();
  أرجع *Config;
}

نطاق {
صنف OnDiskOutputFile final : عام OutputFileImpl {
عام:
  Error keep() override;
  Error discard() override;
  raw_pwrite_دھارا &getOS() override {
    أكد(FileOS && "Expected valid file");
    إذا (بفر_OS)
      أرجع *بفر_OS;
    أرجع *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error try_منزل_Create_Temporary(std::optional<صحيح> &FD);

  Error initializeFile(std::optional<صحيح> &FD);
  Error initialize_دھارا();
  Error reset();

  OnDiskOutputFile(StringRef Output_رستہ, std::optional<OutputConfig> Config,
                   ثابت OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        Output_رستہ(Output_رستہ.str()) {}

  OutputConfig Config;
  ثابت std::string Output_رستہ;
  std::optional<std::string> عارضی_رستہ;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<بفر_ostream> بفر_OS;
};
} // end namespace

ساكن Error createDirectoriesOnDemand(StringRef Output_رستہ,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  أرجع handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    إذا (EC->convert_منزل_Error_کوڈ() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      أرجع Error(std::move(EC));

    StringRef ماپا_رستہ_2 = sys::رستہ_2::ماپا_رستہ(Output_رستہ);
    إذا (std::error_کوڈ EC = sys::fs::create_directories(ماپا_رستہ_2))
      أرجع make_error<OutputError>(ماپا_رستہ_2, EC);
    أرجع CreateFile();
  });
}

ساكن sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags دا_2 = sys::fs::دا_None;
  إذا (Config.get_متن_With_CRLF())
    دا_2 |= sys::fs::دا_متن_With_CRLF;
  وإلا إذا (Config.get_متن())
    دا_2 |= sys::fs::دا_متن;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  إذا (Config.getAppend() && !Config.getAtomicWrite())
    دا_2 |= sys::fs::دا_Append;

  أرجع دا_2;
}

Error OnDiskOutputFile::try_منزل_Create_Temporary(std::optional<صحيح> &FD) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::رستہ_2::extension(Output_رستہ);
  SmallString<128> Model_رستہ =
      StringRef(Output_رستہ).drop_پٹھ(OutputExtension.ناپ_2());
  Model_رستہ += "-%%%%%%%%";
  Model_رستہ += OutputExtension;
  Model_رستہ += ".tmp";

  أرجع createDirectoriesOnDemand(Output_رستہ, Config, [&]() -> Error {
    صحيح NewFD;
    SmallString<128> Unique_رستہ;
    sys::fs::OpenFlags دا_2 = generateFlagsFromConfig(Config);
    إذا (std::error_کوڈ EC =
            sys::fs::createUniqueFile(Model_رستہ, NewFD, Unique_رستہ, دا_2))
      أرجع make_error<عارضی_File_Output_Error>(Model_رستہ, Output_رستہ, EC);

    إذا (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Unique_رستہ);

    عارضی_رستہ = Unique_رستہ.str().str();
    FD.emplace(NewFD);
    أرجع Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<صحيح> &FD) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(Output_رستہ != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  إذا (Config.getAtomicWrite()) {
    sys::fs::file_حالت حالت;
    sys::fs::حالت_2(Output_رستہ, حالت);
    إذا (sys::fs::exists(حالت)) {
      إذا (!sys::fs::is_regular_file(حالت))
        Config.سیٹ_No_Atomic_Write();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      إذا (Config.getAppend() && !sys::fs::can_write(Output_رستہ))
        أرجع make_error<OutputError>(
            Output_رستہ,
            std::make_error_کوڈ(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  إذا (Config.getAtomicWrite())
    إذا (!error_منزل_Bool(try_منزل_Create_Temporary(FD)))
      أرجع Error::success();

  // Not using a temporary file. Open the final output file.
  أرجع createDirectoriesOnDemand(Output_رستہ, Config, [&]() -> Error {
    صحيح NewFD;
    sys::fs::OpenFlags دا_2 = generateFlagsFromConfig(Config);
    إذا (std::error_کوڈ EC = sys::fs::openFileForWrite(
            Output_رستہ, NewFD, sys::fs::CD_CreateAlways, دا_2))
      أرجع convert_منزل_Output_Error(Output_رستہ, EC);
    FD.emplace(NewFD);

    إذا (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Output_رستہ);
    أرجع Error::success();
  });
}

Error OnDiskOutputFile::initialize_دھارا() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  إذا (Output_رستہ == "-") {
    std::error_کوڈ EC;
    FileOS.emplace(Output_رستہ, EC);
    إذا (EC)
      أرجع make_error<OutputError>(Output_رستہ, EC);
  } وإلا {
    std::optional<صحيح> FD;
    إذا (Error E = initializeFile(FD))
      أرجع E;
    FileOS.emplace(*FD, /*shouldClose=*/سچ);
  }

  // Buffer the stream if necessary.
  إذا (!FileOS->supportsSeeking() && !Config.get_متن())
    بفر_OS.emplace(*FileOS);

  أرجع Error::success();
}

نطاق {
صنف OpenFileRAII {
  ساكن ثابت صحيح InvalidFd = -1;

عام:
  صحيح Fd = InvalidFd;

  ~OpenFileRAII() {
    إذا (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

تعداد صنف FileDifference : uint8_t {
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

ساكن Expected<FileDifference>
areFilesDifferent(ثابت llvm::Twine &سروت, ثابت llvm::Twine &Destination) {
  إذا (sys::fs::equivalent(سروت, Destination))
    أرجع FileDifference::IdenticalFile;

  OpenFileRAII سروت_File;
  sys::fs::file_حالت سروت_حالت;
  // If we can't open the source file, fail.
  إذا (std::error_کوڈ EC = sys::fs::openFileForRead(سروت, سروت_File.Fd))
    أرجع convert_منزل_Output_Error(سروت, EC);

  // If we can't stat the source file, fail.
  إذا (std::error_کوڈ EC = sys::fs::حالت_2(سروت_File.Fd, سروت_حالت))
    أرجع convert_منزل_Output_Error(سروت, EC);

  OpenFileRAII DestFile;
  sys::fs::file_حالت Dest_حالت;
  // If we can't open the destination file, report different.
  إذا (std::error_کوڈ Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    أرجع FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  إذا (std::error_کوڈ Error = sys::fs::حالت_2(DestFile.Fd, Dest_حالت))
    أرجع FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t ناپ = سروت_حالت.get_ناپ();
  إذا (ناپ != Dest_حالت.get_ناپ())
    أرجع FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  إذا (ناپ == 0)
    أرجع FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_کوڈ سروت_Region_Err;
  sys::fs::mapped_file_region سروت_Region(
      sys::fs::convertFDToNativeFile(سروت_File.Fd),
      sys::fs::mapped_file_region::readonly, ناپ, 0, سروت_Region_Err);
  إذا (سروت_Region_Err)
    أرجع convert_منزل_Output_Error(سروت, سروت_Region_Err);

  std::error_کوڈ DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, ناپ, 0, DestRegionErr);

  إذا (DestRegionErr)
    أرجع FileDifference::DifferentContents;

  إذا (memcmp(سروت_Region.const_انکڑے(), DestRegion.const_انکڑے(), ناپ) != 0)
    أرجع FileDifference::DifferentContents;

  أرجع FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  بفر_OS.reset();
  إذا (!FileOS)
    أرجع Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_کوڈ EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  أرجع error_کوڈ_منزل_Error(EC);
}

Error OnDiskOutputFile::keep() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (خودکار E = reset())
    أرجع E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    إذا (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(عارضی_رستہ ? *عارضی_رستہ : Output_رستہ);
  });

  إذا (!عارضی_رستہ)
    أرجع Error::success();

  // See if we should append instead of move.
  إذا (Config.getAppend() && Output_رستہ != "-") {
    // Read TempFile for the content to append.
    خودکار Content = حافظہ_بفر::getFile(*عارضی_رستہ);
    إذا (!Content)
      أرجع convert_منزل_عارضی_File_Output_Error(*عارضی_رستہ, Output_رستہ,
                                          Content.getError());
    طالما (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::جندرا_File_Manager جندرا_2(Output_رستہ);
      منطقي Owned;
      إذا (Error Err = جندرا_2.try_جندرا().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        جندرا_2.unsafeUnlock();
        أرجع convert_منزل_Output_Error(
            Output_رستہ, std::make_error_کوڈ(std::errc::no_جندرا_دستیاب));
      }
      إذا (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_کوڈ EC;
        llvm::raw_fd_ostream باہرلا(Output_رستہ, EC, llvm::sys::fs::دا_Append);
        إذا (EC)
          أرجع convert_منزل_Output_Error(Output_رستہ, EC);
        باہرلا << (*Content)->get_بفر();
        باہرلا.close();
        جندرا_2.unsafeUnlock();
        إذا (باہرلا.has_error())
          أرجع convert_منزل_Output_Error(Output_رستہ, باہرلا.error());
        // Remove temp file and done.
        (فراغ)sys::fs::remove(*عارضی_رستہ);
        أرجع Error::success();
      }
      // Someone else owns the lock on this file, wait.
      بدل (جندرا_2.waitForUnlockFor(std::chrono::seconds(256))) {
      حالة Wait_For_Unlock_نتیجہ::Success:
        [[fallthrough]];
      حالة Wait_For_Unlock_نتیجہ::OwnerDied: {
        استمر; // try again to get the lock.
      }
      حالة Wait_For_Unlock_نتیجہ::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        جندرا_2.unsafeUnlock();
        استمر;
      }
      }
      اكسر;
    }
  }

  إذا (Config.getOnlyIfDifferent()) {
    خودکار نتیجہ = areFilesDifferent(*عارضی_رستہ, Output_رستہ);
    إذا (!نتیجہ)
      أرجع نتیجہ.takeError();
    بدل (*نتیجہ) {
    حالة FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      أرجع Error::success();

    حالة FileDifference::SameContents:
      // Files are identical; remove the source file.
      (فراغ)sys::fs::remove(*عارضی_رستہ);
      أرجع Error::success();

    حالة FileDifference::DifferentContents:
      اكسر; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_کوڈ RenameEC = sys::fs::rename(*عارضی_رستہ, Output_رستہ);
  إذا (!RenameEC)
    أرجع Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*عارضی_رستہ, Output_رستہ);
  (فراغ)sys::fs::remove(*عارضی_رستہ);

  إذا (!RenameEC)
    أرجع Error::success();

  أرجع make_error<عارضی_File_Output_Error>(*عارضی_رستہ, Output_رستہ, RenameEC);
}

Error OnDiskOutputFile::discard() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  إذا (خودکار E = reset())
    أرجع E;

  // Nothing on the filesystem to remove for stdout.
  إذا (Output_رستہ == "-")
    أرجع Error::success();

  خودکار discard_رستہ = [&](StringRef رستہ) {
    std::error_کوڈ EC = sys::fs::remove(رستہ);
    sys::DontRemoveFileOnSignal(رستہ);
    أرجع EC;
  };

  // Clean up the file that's in-progress.
  إذا (!عارضی_رستہ)
    أرجع convert_منزل_Output_Error(Output_رستہ, discard_رستہ(Output_رستہ));
  أرجع convert_منزل_عارضی_File_Output_Error(*عارضی_رستہ, Output_رستہ,
                                      discard_رستہ(*عارضی_رستہ));
}

Error OnDiskOutputBackend::makeAbsolute(Small_سمتیہ_Impl<حرف> &رستہ) ثابت {
  // FIXME: Should this really call sys::fs::make_absolute?
  خودکار BypassSandbox = sys::sandbox::scopedDisable();
  أرجع convert_منزل_Output_Error(StringRef(رستہ.انکڑے(), رستہ.ناپ_2()),
                              sys::fs::make_absolute(رستہ));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef رستہ,
                                    std::optional<OutputConfig> Config) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Abs_رستہ;
  إذا (رستہ != "-") {
    Abs_رستہ = رستہ;
    إذا (Error E = makeAbsolute(Abs_رستہ))
      أرجع std::move(E);
    رستہ = Abs_رستہ;
  }

  خودکار File = std::make_unique<OnDiskOutputFile>(رستہ, Config, Settings);
  إذا (Error E = File->initialize_دھارا())
    أرجع std::move(E);

  أرجع std::move(File);
}
