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

#شامل "llvm/Support/VirtualOutputBackends.h"
#شامل "llvm/ADT/ScopeExit.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/LockFileManager.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/VirtualOutputConfig.h"
#شامل "llvm/Support/VirtualOutputError.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::vfs;

خالی ProxyOutputBackend::anchor() {}
خالی OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  ساخت NullOutputBackend : عوامی OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      واپس const_cast<NullOutputBackend *>(یہ);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef راستہ, std::optional<OutputConfig>) override {
      واپس std::make_unique<NullOutputFileImpl>();
    }
  };

  واپس makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<منطقی(StringRef, std::optional<OutputConfig>)> Filter) {
  ساخت FilteringOutputBackend : عوامی ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef راستہ,
                   std::optional<OutputConfig> Config) override {
      اگر (Filter(راستہ, Config))
        واپس ProxyOutputBackend::createFileImpl(راستہ, Config);
      واپس std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      واپس makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<منطقی(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      أكد(یہ->Filter && "Expected a non-null function");
    }
    std::function<منطقی(StringRef, std::optional<OutputConfig>)> Filter;
  };

  واپس makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  ساخت ProxyOutputBackend1 : عوامی ProxyOutputBackend {
    استعمال ProxyOutputBackend::ProxyOutputBackend;
  };
  ساخت ProxyOutputBackend2 : عوامی ProxyOutputBackend {
    استعمال ProxyOutputBackend::ProxyOutputBackend;
  };
  ساخت MirroringOutput final : عوامی OutputFileImpl, raw_pwrite_دھارا {
    Error keep() final {
      flush();
      واپس joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      واپس joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_دھارا &getOS() final { واپس *یہ; }

    خالی write_impl(ثابت حرف *Ptr, size_t سائز) override {
      F1->getOS().اكتب(Ptr, سائز);
      F2->getOS().اكتب(Ptr, سائز);
    }
    خالی pwrite_impl(ثابت حرف *Ptr, size_t سائز, uint64_t انحراف) override {
      یہ->flush();
      F1->getOS().pwrite(Ptr, سائز, انحراف);
      F2->getOS().pwrite(Ptr, سائز, انحراف);
    }
    uint64_t current_pos() ثابت override { واپس F1->getOS().tell(); }
    size_t preferred_بفر_سائز() ثابت override {
      واپس Preferred_بفر_سائز;
    }
    خالی reserveExtraSpace(uint64_t Extra_سائز) override {
      F1->getOS().reserveExtraSpace(Extra_سائز);
      F2->getOS().reserveExtraSpace(Extra_سائز);
    }
    منطقی is_displayed() ثابت override {
      واپس F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    منطقی has_colors() ثابت override {
      واپس F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    خالی enable_colors(منطقی enable) override {
      raw_pwrite_دھارا::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_بفر_سائز(std::max(F1->getOS().Get_بفر_سائز(),
                                       F1->getOS().Get_بفر_سائز())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_بفر_سائز;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  ساخت MirroringOutputBackend : عوامی ProxyOutputBackend1,
                                  عوامی ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef راستہ,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      اگر (Error E =
              ProxyOutputBackend1::createFileImpl(راستہ, Config).moveInto(File1))
        واپس std::move(E);
      اگر (Error E =
              ProxyOutputBackend2::createFileImpl(راستہ, Config).moveInto(File2))
        واپس joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      اگر (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        واپس std::move(File2);
      }
      اگر (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        واپس std::move(File1);
      }
      واپس std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      واپس IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    خالی Retain() ثابت { ProxyOutputBackend1::Retain(); }
    خالی Release() ثابت { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  أكد(Backend1 && "Expected actual backend");
  أكد(Backend2 && "Expected actual backend");
  واپس IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ساکن OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ثابت OnDiskOutputBackend::OutputSettings &Settings) {
  اگر (!Config)
    Config = Settings.DefaultConfig;
  اگر (!Settings.UseTemporaries)
    Config->سیٹ_No_Atomic_Write();
  اگر (!Settings.RemoveOnSignal)
    Config->سیٹ_No_Discard_On_Signal();
  واپس *Config;
}

نامخلا {
جماعت OnDiskOutputFile final : عوامی OutputFileImpl {
عوامی:
  Error keep() override;
  Error discard() override;
  raw_pwrite_دھارا &getOS() override {
    أكد(FileOS && "Expected valid file");
    اگر (بفر_OS)
      واپس *بفر_OS;
    واپس *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error try_مقصد_Create_Temporary(std::optional<صحیح> &FD);

  Error initializeFile(std::optional<صحیح> &FD);
  Error initialize_دھارا();
  Error reset();

  OnDiskOutputFile(StringRef Output_راستہ, std::optional<OutputConfig> Config,
                   ثابت OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        Output_راستہ(Output_راستہ.str()) {}

  OutputConfig Config;
  ثابت std::string Output_راستہ;
  std::optional<std::string> عارضی_راستہ;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<بفر_ostream> بفر_OS;
};
} // end namespace

ساکن Error createDirectoriesOnDemand(StringRef Output_راستہ,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  واپس handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    اگر (EC->convert_مقصد_Error_رمز() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      واپس Error(std::move(EC));

    StringRef والد_راستہ_2 = sys::راستہ_2::والد_راستہ(Output_راستہ);
    اگر (std::error_رمز EC = sys::fs::create_directories(والد_راستہ_2))
      واپس make_error<OutputError>(والد_راستہ_2, EC);
    واپس CreateFile();
  });
}

ساکن sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags کا_2 = sys::fs::کا_None;
  اگر (Config.get_متن_With_CRLF())
    کا_2 |= sys::fs::کا_متن_With_CRLF;
  ورنہ اگر (Config.get_متن())
    کا_2 |= sys::fs::کا_متن;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  اگر (Config.getAppend() && !Config.getAtomicWrite())
    کا_2 |= sys::fs::کا_Append;

  واپس کا_2;
}

Error OnDiskOutputFile::try_مقصد_Create_Temporary(std::optional<صحیح> &FD) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::راستہ_2::extension(Output_راستہ);
  SmallString<128> Model_راستہ =
      StringRef(Output_راستہ).drop_عقب(OutputExtension.سائز_2());
  Model_راستہ += "-%%%%%%%%";
  Model_راستہ += OutputExtension;
  Model_راستہ += ".tmp";

  واپس createDirectoriesOnDemand(Output_راستہ, Config, [&]() -> Error {
    صحیح NewFD;
    SmallString<128> Unique_راستہ;
    sys::fs::OpenFlags کا_2 = generateFlagsFromConfig(Config);
    اگر (std::error_رمز EC =
            sys::fs::createUniqueFile(Model_راستہ, NewFD, Unique_راستہ, کا_2))
      واپس make_error<عارضی_File_Output_Error>(Model_راستہ, Output_راستہ, EC);

    اگر (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Unique_راستہ);

    عارضی_راستہ = Unique_راستہ.str().str();
    FD.emplace(NewFD);
    واپس Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<صحیح> &FD) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(Output_راستہ != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  اگر (Config.getAtomicWrite()) {
    sys::fs::file_حالت حالت_2;
    sys::fs::حالت_3(Output_راستہ, حالت_2);
    اگر (sys::fs::exists(حالت_2)) {
      اگر (!sys::fs::is_regular_file(حالت_2))
        Config.سیٹ_No_Atomic_Write();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      اگر (Config.getAppend() && !sys::fs::can_write(Output_راستہ))
        واپس make_error<OutputError>(
            Output_راستہ,
            std::make_error_رمز(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  اگر (Config.getAtomicWrite())
    اگر (!error_مقصد_Bool(try_مقصد_Create_Temporary(FD)))
      واپس Error::success();

  // Not using a temporary file. Open the final output file.
  واپس createDirectoriesOnDemand(Output_راستہ, Config, [&]() -> Error {
    صحیح NewFD;
    sys::fs::OpenFlags کا_2 = generateFlagsFromConfig(Config);
    اگر (std::error_رمز EC = sys::fs::openFileForWrite(
            Output_راستہ, NewFD, sys::fs::CD_CreateAlways, کا_2))
      واپس convert_مقصد_Output_Error(Output_راستہ, EC);
    FD.emplace(NewFD);

    اگر (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Output_راستہ);
    واپس Error::success();
  });
}

Error OnDiskOutputFile::initialize_دھارا() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  اگر (Output_راستہ == "-") {
    std::error_رمز EC;
    FileOS.emplace(Output_راستہ, EC);
    اگر (EC)
      واپس make_error<OutputError>(Output_راستہ, EC);
  } ورنہ {
    std::optional<صحیح> FD;
    اگر (Error E = initializeFile(FD))
      واپس E;
    FileOS.emplace(*FD, /*shouldClose=*/سچ);
  }

  // Buffer the stream if necessary.
  اگر (!FileOS->supportsSeeking() && !Config.get_متن())
    بفر_OS.emplace(*FileOS);

  واپس Error::success();
}

نامخلا {
جماعت OpenFileRAII {
  ساکن ثابت صحیح InvalidFd = -1;

عوامی:
  صحیح Fd = InvalidFd;

  ~OpenFileRAII() {
    اگر (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

شمار جماعت FileDifference : uint8_t {
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

ساکن Expected<FileDifference>
areFilesDifferent(ثابت llvm::Twine &ماخذ, ثابت llvm::Twine &Destination) {
  اگر (sys::fs::equivalent(ماخذ, Destination))
    واپس FileDifference::IdenticalFile;

  OpenFileRAII ماخذ_File;
  sys::fs::file_حالت ماخذ_حالت;
  // If we can't open the source file, fail.
  اگر (std::error_رمز EC = sys::fs::openFileForRead(ماخذ, ماخذ_File.Fd))
    واپس convert_مقصد_Output_Error(ماخذ, EC);

  // If we can't stat the source file, fail.
  اگر (std::error_رمز EC = sys::fs::حالت_3(ماخذ_File.Fd, ماخذ_حالت))
    واپس convert_مقصد_Output_Error(ماخذ, EC);

  OpenFileRAII DestFile;
  sys::fs::file_حالت Dest_حالت;
  // If we can't open the destination file, report different.
  اگر (std::error_رمز Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    واپس FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  اگر (std::error_رمز Error = sys::fs::حالت_3(DestFile.Fd, Dest_حالت))
    واپس FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t سائز = ماخذ_حالت.get_سائز();
  اگر (سائز != Dest_حالت.get_سائز())
    واپس FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  اگر (سائز == 0)
    واپس FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_رمز ماخذ_Region_Err;
  sys::fs::mapped_file_region ماخذ_Region(
      sys::fs::convertFDToNativeFile(ماخذ_File.Fd),
      sys::fs::mapped_file_region::readonly, سائز, 0, ماخذ_Region_Err);
  اگر (ماخذ_Region_Err)
    واپس convert_مقصد_Output_Error(ماخذ, ماخذ_Region_Err);

  std::error_رمز DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, سائز, 0, DestRegionErr);

  اگر (DestRegionErr)
    واپس FileDifference::DifferentContents;

  اگر (memcmp(ماخذ_Region.const_ڈیٹا(), DestRegion.const_ڈیٹا(), سائز) != 0)
    واپس FileDifference::DifferentContents;

  واپس FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  بفر_OS.reset();
  اگر (!FileOS)
    واپس Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_رمز EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  واپس error_رمز_مقصد_Error(EC);
}

Error OnDiskOutputFile::keep() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  اگر (خودکار E = reset())
    واپس E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    اگر (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(عارضی_راستہ ? *عارضی_راستہ : Output_راستہ);
  });

  اگر (!عارضی_راستہ)
    واپس Error::success();

  // See if we should append instead of move.
  اگر (Config.getAppend() && Output_راستہ != "-") {
    // Read TempFile for the content to append.
    خودکار Content = حافظہ_بفر::getFile(*عارضی_راستہ);
    اگر (!Content)
      واپس convert_مقصد_عارضی_File_Output_Error(*عارضی_راستہ, Output_راستہ,
                                          Content.getError());
    جب (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::تالا_File_Manager تالا_3(Output_راستہ);
      منطقی Owned;
      اگر (Error Err = تالا_3.try_تالا().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        تالا_3.unsafeUnlock();
        واپس convert_مقصد_Output_Error(
            Output_راستہ, std::make_error_رمز(std::errc::no_تالا_دستیاب));
      }
      اگر (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_رمز EC;
        llvm::raw_fd_ostream برآمد(Output_راستہ, EC, llvm::sys::fs::کا_Append);
        اگر (EC)
          واپس convert_مقصد_Output_Error(Output_راستہ, EC);
        برآمد << (*Content)->get_بفر();
        برآمد.close();
        تالا_3.unsafeUnlock();
        اگر (برآمد.has_error())
          واپس convert_مقصد_Output_Error(Output_راستہ, برآمد.error());
        // Remove temp file and done.
        (خالی)sys::fs::remove(*عارضی_راستہ);
        واپس Error::success();
      }
      // Someone else owns the lock on this file, wait.
      انتخاب (تالا_3.waitForUnlockFor(std::chrono::seconds(256))) {
      حالت Wait_For_Unlock_نتیجہ::Success:
        [[fallthrough]];
      حالت Wait_For_Unlock_نتیجہ::OwnerDied: {
        جاری; // try again to get the lock.
      }
      حالت Wait_For_Unlock_نتیجہ::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        تالا_3.unsafeUnlock();
        جاری;
      }
      }
      توڑو;
    }
  }

  اگر (Config.getOnlyIfDifferent()) {
    خودکار نتیجہ = areFilesDifferent(*عارضی_راستہ, Output_راستہ);
    اگر (!نتیجہ)
      واپس نتیجہ.takeError();
    انتخاب (*نتیجہ) {
    حالت FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      واپس Error::success();

    حالت FileDifference::SameContents:
      // Files are identical; remove the source file.
      (خالی)sys::fs::remove(*عارضی_راستہ);
      واپس Error::success();

    حالت FileDifference::DifferentContents:
      توڑو; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_رمز RenameEC = sys::fs::rename(*عارضی_راستہ, Output_راستہ);
  اگر (!RenameEC)
    واپس Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*عارضی_راستہ, Output_راستہ);
  (خالی)sys::fs::remove(*عارضی_راستہ);

  اگر (!RenameEC)
    واپس Error::success();

  واپس make_error<عارضی_File_Output_Error>(*عارضی_راستہ, Output_راستہ, RenameEC);
}

Error OnDiskOutputFile::discard() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  اگر (خودکار E = reset())
    واپس E;

  // Nothing on the filesystem to remove for stdout.
  اگر (Output_راستہ == "-")
    واپس Error::success();

  خودکار discard_راستہ = [&](StringRef راستہ) {
    std::error_رمز EC = sys::fs::remove(راستہ);
    sys::DontRemoveFileOnSignal(راستہ);
    واپس EC;
  };

  // Clean up the file that's in-progress.
  اگر (!عارضی_راستہ)
    واپس convert_مقصد_Output_Error(Output_راستہ, discard_راستہ(Output_راستہ));
  واپس convert_مقصد_عارضی_File_Output_Error(*عارضی_راستہ, Output_راستہ,
                                      discard_راستہ(*عارضی_راستہ));
}

Error OnDiskOutputBackend::makeAbsolute(Small_سمتیہ_Impl<حرف> &راستہ) ثابت {
  // FIXME: Should this really call sys::fs::make_absolute?
  خودکار BypassSandbox = sys::sandbox::scopedDisable();
  واپس convert_مقصد_Output_Error(StringRef(راستہ.ڈیٹا(), راستہ.سائز_2()),
                              sys::fs::make_absolute(راستہ));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef راستہ,
                                    std::optional<OutputConfig> Config) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Abs_راستہ;
  اگر (راستہ != "-") {
    Abs_راستہ = راستہ;
    اگر (Error E = makeAbsolute(Abs_راستہ))
      واپس std::move(E);
    راستہ = Abs_راستہ;
  }

  خودکار File = std::make_unique<OnDiskOutputFile>(راستہ, Config, Settings);
  اگر (Error E = File->initialize_دھارا())
    واپس std::move(E);

  واپس std::move(File);
}
