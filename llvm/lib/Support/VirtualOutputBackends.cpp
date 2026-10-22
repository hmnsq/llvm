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

استفاده نطاق llvm;
استفاده نطاق llvm::vfs;

فراغ ProxyOutputBackend::anchor() {}
فراغ OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  بنية NullOutputBackend : عام OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      أرجع const_cast<NullOutputBackend *>(هذا);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef مسیر, std::optional<OutputConfig>) override {
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
    createFileImpl(StringRef مسیر,
                   std::optional<OutputConfig> Config) override {
      إذا (Filter(مسیر, Config))
        أرجع ProxyOutputBackend::createFileImpl(مسیر, Config);
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
    استفاده ProxyOutputBackend::ProxyOutputBackend;
  };
  بنية ProxyOutputBackend2 : عام ProxyOutputBackend {
    استفاده ProxyOutputBackend::ProxyOutputBackend;
  };
  بنية MirroringOutput final : عام OutputFileImpl, raw_pwrite_جریان {
    Error keep() final {
      flush();
      أرجع joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      أرجع joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_جریان &getOS() final { أرجع *هذا; }

    فراغ write_impl(ثابت حرف *Ptr, size_t اندازه) override {
      F1->getOS().اكتب(Ptr, اندازه);
      F2->getOS().اكتب(Ptr, اندازه);
    }
    فراغ pwrite_impl(ثابت حرف *Ptr, size_t اندازه, uint64_t جابه_جایی) override {
      هذا->flush();
      F1->getOS().pwrite(Ptr, اندازه, جابه_جایی);
      F2->getOS().pwrite(Ptr, اندازه, جابه_جایی);
    }
    uint64_t current_pos() ثابت override { أرجع F1->getOS().tell(); }
    size_t preferred_میانگیر_اندازه() ثابت override {
      أرجع Preferred_میانگیر_اندازه;
    }
    فراغ reserveExtraSpace(uint64_t Extra_اندازه) override {
      F1->getOS().reserveExtraSpace(Extra_اندازه);
      F2->getOS().reserveExtraSpace(Extra_اندازه);
    }
    منطقي is_displayed() ثابت override {
      أرجع F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    منطقي has_colors() ثابت override {
      أرجع F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    فراغ enable_colors(منطقي enable) override {
      raw_pwrite_جریان::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_میانگیر_اندازه(std::max(F1->getOS().Get_میانگیر_اندازه(),
                                       F1->getOS().Get_میانگیر_اندازه())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_میانگیر_اندازه;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  بنية MirroringOutputBackend : عام ProxyOutputBackend1,
                                  عام ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef مسیر,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      إذا (Error E =
              ProxyOutputBackend1::createFileImpl(مسیر, Config).moveInto(File1))
        أرجع std::move(E);
      إذا (Error E =
              ProxyOutputBackend2::createFileImpl(مسیر, Config).moveInto(File2))
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
    Config->مجموعه_No_Atomic_Write();
  إذا (!Settings.RemoveOnSignal)
    Config->مجموعه_No_Discard_On_Signal();
  أرجع *Config;
}

نطاق {
صنف OnDiskOutputFile final : عام OutputFileImpl {
عام:
  Error keep() override;
  Error discard() override;
  raw_pwrite_جریان &getOS() override {
    أكد(FileOS && "Expected valid file");
    إذا (میانگیر_OS)
      أرجع *میانگیر_OS;
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
  Error try_مقصد_Create_Temporary(std::optional<صحيح> &FD);

  Error initializeFile(std::optional<صحيح> &FD);
  Error initialize_جریان();
  Error reset();

  OnDiskOutputFile(StringRef Output_مسیر, std::optional<OutputConfig> Config,
                   ثابت OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        Output_مسیر(Output_مسیر.str()) {}

  OutputConfig Config;
  ثابت std::string Output_مسیر;
  std::optional<std::string> موقت_مسیر;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<میانگیر_ostream> میانگیر_OS;
};
} // end namespace

ساكن Error createDirectoriesOnDemand(StringRef Output_مسیر,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  أرجع handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    إذا (EC->convert_مقصد_Error_کد() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      أرجع Error(std::move(EC));

    StringRef والد_مسیر_2 = sys::مسیر_2::والد_مسیر(Output_مسیر);
    إذا (std::error_کد EC = sys::fs::create_directories(والد_مسیر_2))
      أرجع make_error<OutputError>(والد_مسیر_2, EC);
    أرجع CreateFile();
  });
}

ساكن sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags از_2 = sys::fs::از_None;
  إذا (Config.get_متن_With_CRLF())
    از_2 |= sys::fs::از_متن_With_CRLF;
  وإلا إذا (Config.get_متن())
    از_2 |= sys::fs::از_متن;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  إذا (Config.getAppend() && !Config.getAtomicWrite())
    از_2 |= sys::fs::از_Append;

  أرجع از_2;
}

Error OnDiskOutputFile::try_مقصد_Create_Temporary(std::optional<صحيح> &FD) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::مسیر_2::extension(Output_مسیر);
  SmallString<128> Model_مسیر =
      StringRef(Output_مسیر).drop_پشت(OutputExtension.اندازه_2());
  Model_مسیر += "-%%%%%%%%";
  Model_مسیر += OutputExtension;
  Model_مسیر += ".tmp";

  أرجع createDirectoriesOnDemand(Output_مسیر, Config, [&]() -> Error {
    صحيح NewFD;
    SmallString<128> Unique_مسیر;
    sys::fs::OpenFlags از_2 = generateFlagsFromConfig(Config);
    إذا (std::error_کد EC =
            sys::fs::createUniqueFile(Model_مسیر, NewFD, Unique_مسیر, از_2))
      أرجع make_error<موقت_File_Output_Error>(Model_مسیر, Output_مسیر, EC);

    إذا (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Unique_مسیر);

    موقت_مسیر = Unique_مسیر.str().str();
    FD.emplace(NewFD);
    أرجع Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<صحيح> &FD) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(Output_مسیر != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  إذا (Config.getAtomicWrite()) {
    sys::fs::file_وضعیت وضعیت;
    sys::fs::وضعیت_2(Output_مسیر, وضعیت);
    إذا (sys::fs::exists(وضعیت)) {
      إذا (!sys::fs::is_regular_file(وضعیت))
        Config.مجموعه_No_Atomic_Write();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      إذا (Config.getAppend() && !sys::fs::can_write(Output_مسیر))
        أرجع make_error<OutputError>(
            Output_مسیر,
            std::make_error_کد(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  إذا (Config.getAtomicWrite())
    إذا (!error_مقصد_Bool(try_مقصد_Create_Temporary(FD)))
      أرجع Error::success();

  // Not using a temporary file. Open the final output file.
  أرجع createDirectoriesOnDemand(Output_مسیر, Config, [&]() -> Error {
    صحيح NewFD;
    sys::fs::OpenFlags از_2 = generateFlagsFromConfig(Config);
    إذا (std::error_کد EC = sys::fs::openFileForWrite(
            Output_مسیر, NewFD, sys::fs::CD_CreateAlways, از_2))
      أرجع convert_مقصد_Output_Error(Output_مسیر, EC);
    FD.emplace(NewFD);

    إذا (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Output_مسیر);
    أرجع Error::success();
  });
}

Error OnDiskOutputFile::initialize_جریان() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  إذا (Output_مسیر == "-") {
    std::error_کد EC;
    FileOS.emplace(Output_مسیر, EC);
    إذا (EC)
      أرجع make_error<OutputError>(Output_مسیر, EC);
  } وإلا {
    std::optional<صحيح> FD;
    إذا (Error E = initializeFile(FD))
      أرجع E;
    FileOS.emplace(*FD, /*shouldClose=*/درست);
  }

  // Buffer the stream if necessary.
  إذا (!FileOS->supportsSeeking() && !Config.get_متن())
    میانگیر_OS.emplace(*FileOS);

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
areFilesDifferent(ثابت llvm::Twine &منبع, ثابت llvm::Twine &Destination) {
  إذا (sys::fs::equivalent(منبع, Destination))
    أرجع FileDifference::IdenticalFile;

  OpenFileRAII منبع_File;
  sys::fs::file_وضعیت منبع_وضعیت;
  // If we can't open the source file, fail.
  إذا (std::error_کد EC = sys::fs::openFileForRead(منبع, منبع_File.Fd))
    أرجع convert_مقصد_Output_Error(منبع, EC);

  // If we can't stat the source file, fail.
  إذا (std::error_کد EC = sys::fs::وضعیت_2(منبع_File.Fd, منبع_وضعیت))
    أرجع convert_مقصد_Output_Error(منبع, EC);

  OpenFileRAII DestFile;
  sys::fs::file_وضعیت Dest_وضعیت;
  // If we can't open the destination file, report different.
  إذا (std::error_کد Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    أرجع FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  إذا (std::error_کد Error = sys::fs::وضعیت_2(DestFile.Fd, Dest_وضعیت))
    أرجع FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t اندازه = منبع_وضعیت.get_اندازه();
  إذا (اندازه != Dest_وضعیت.get_اندازه())
    أرجع FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  إذا (اندازه == 0)
    أرجع FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_کد منبع_Region_Err;
  sys::fs::mapped_file_region منبع_Region(
      sys::fs::convertFDToNativeFile(منبع_File.Fd),
      sys::fs::mapped_file_region::readonly, اندازه, 0, منبع_Region_Err);
  إذا (منبع_Region_Err)
    أرجع convert_مقصد_Output_Error(منبع, منبع_Region_Err);

  std::error_کد DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, اندازه, 0, DestRegionErr);

  إذا (DestRegionErr)
    أرجع FileDifference::DifferentContents;

  إذا (memcmp(منبع_Region.const_داده(), DestRegion.const_داده(), اندازه) != 0)
    أرجع FileDifference::DifferentContents;

  أرجع FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  میانگیر_OS.reset();
  إذا (!FileOS)
    أرجع Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_کد EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  أرجع error_کد_مقصد_Error(EC);
}

Error OnDiskOutputFile::keep() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (خودکار E = reset())
    أرجع E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    إذا (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(موقت_مسیر ? *موقت_مسیر : Output_مسیر);
  });

  إذا (!موقت_مسیر)
    أرجع Error::success();

  // See if we should append instead of move.
  إذا (Config.getAppend() && Output_مسیر != "-") {
    // Read TempFile for the content to append.
    خودکار Content = حافظه_میانگیر::getFile(*موقت_مسیر);
    إذا (!Content)
      أرجع convert_مقصد_موقت_File_Output_Error(*موقت_مسیر, Output_مسیر,
                                          Content.getError());
    طالما (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::قفل_File_Manager قفل_3(Output_مسیر);
      منطقي Owned;
      إذا (Error Err = قفل_3.try_قفل().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        قفل_3.unsafeUnlock();
        أرجع convert_مقصد_Output_Error(
            Output_مسیر, std::make_error_کد(std::errc::no_قفل_موجود));
      }
      إذا (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_کد EC;
        llvm::raw_fd_ostream بیرون(Output_مسیر, EC, llvm::sys::fs::از_Append);
        إذا (EC)
          أرجع convert_مقصد_Output_Error(Output_مسیر, EC);
        بیرون << (*Content)->get_میانگیر();
        بیرون.close();
        قفل_3.unsafeUnlock();
        إذا (بیرون.has_error())
          أرجع convert_مقصد_Output_Error(Output_مسیر, بیرون.error());
        // Remove temp file and done.
        (فراغ)sys::fs::remove(*موقت_مسیر);
        أرجع Error::success();
      }
      // Someone else owns the lock on this file, wait.
      بدل (قفل_3.waitForUnlockFor(std::chrono::seconds(256))) {
      حالة Wait_For_Unlock_نتیجه::Success:
        [[fallthrough]];
      حالة Wait_For_Unlock_نتیجه::OwnerDied: {
        استمر; // try again to get the lock.
      }
      حالة Wait_For_Unlock_نتیجه::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        قفل_3.unsafeUnlock();
        استمر;
      }
      }
      اكسر;
    }
  }

  إذا (Config.getOnlyIfDifferent()) {
    خودکار نتیجه = areFilesDifferent(*موقت_مسیر, Output_مسیر);
    إذا (!نتیجه)
      أرجع نتیجه.takeError();
    بدل (*نتیجه) {
    حالة FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      أرجع Error::success();

    حالة FileDifference::SameContents:
      // Files are identical; remove the source file.
      (فراغ)sys::fs::remove(*موقت_مسیر);
      أرجع Error::success();

    حالة FileDifference::DifferentContents:
      اكسر; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_کد RenameEC = sys::fs::rename(*موقت_مسیر, Output_مسیر);
  إذا (!RenameEC)
    أرجع Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*موقت_مسیر, Output_مسیر);
  (فراغ)sys::fs::remove(*موقت_مسیر);

  إذا (!RenameEC)
    أرجع Error::success();

  أرجع make_error<موقت_File_Output_Error>(*موقت_مسیر, Output_مسیر, RenameEC);
}

Error OnDiskOutputFile::discard() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  إذا (خودکار E = reset())
    أرجع E;

  // Nothing on the filesystem to remove for stdout.
  إذا (Output_مسیر == "-")
    أرجع Error::success();

  خودکار discard_مسیر = [&](StringRef مسیر) {
    std::error_کد EC = sys::fs::remove(مسیر);
    sys::DontRemoveFileOnSignal(مسیر);
    أرجع EC;
  };

  // Clean up the file that's in-progress.
  إذا (!موقت_مسیر)
    أرجع convert_مقصد_Output_Error(Output_مسیر, discard_مسیر(Output_مسیر));
  أرجع convert_مقصد_موقت_File_Output_Error(*موقت_مسیر, Output_مسیر,
                                      discard_مسیر(*موقت_مسیر));
}

Error OnDiskOutputBackend::makeAbsolute(Small_وکتور_Impl<حرف> &مسیر) ثابت {
  // FIXME: Should this really call sys::fs::make_absolute?
  خودکار BypassSandbox = sys::sandbox::scopedDisable();
  أرجع convert_مقصد_Output_Error(StringRef(مسیر.داده(), مسیر.اندازه_2()),
                              sys::fs::make_absolute(مسیر));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef مسیر,
                                    std::optional<OutputConfig> Config) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Abs_مسیر;
  إذا (مسیر != "-") {
    Abs_مسیر = مسیر;
    إذا (Error E = makeAbsolute(Abs_مسیر))
      أرجع std::move(E);
    مسیر = Abs_مسیر;
  }

  خودکار File = std::make_unique<OnDiskOutputFile>(مسیر, Config, Settings);
  إذا (Error E = File->initialize_جریان())
    أرجع std::move(E);

  أرجع std::move(File);
}
