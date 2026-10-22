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

کارول نطاق llvm;
کارول نطاق llvm::vfs;

فراغ ProxyOutputBackend::anchor() {}
فراغ OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  بنية NullOutputBackend : عام OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      أرجع const_cast<NullOutputBackend *>(هذا);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef لار, std::optional<OutputConfig>) override {
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
    createFileImpl(StringRef لار,
                   std::optional<OutputConfig> Config) override {
      إذا (Filter(لار, Config))
        أرجع ProxyOutputBackend::createFileImpl(لار, Config);
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
    کارول ProxyOutputBackend::ProxyOutputBackend;
  };
  بنية ProxyOutputBackend2 : عام ProxyOutputBackend {
    کارول ProxyOutputBackend::ProxyOutputBackend;
  };
  بنية MirroringOutput final : عام OutputFileImpl, raw_pwrite_بهیر {
    Error keep() final {
      flush();
      أرجع joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      أرجع joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_بهیر &getOS() final { أرجع *هذا; }

    فراغ write_impl(ثابت حرف *Ptr, size_t اندازه) override {
      F1->getOS().اكتب(Ptr, اندازه);
      F2->getOS().اكتب(Ptr, اندازه);
    }
    فراغ pwrite_impl(ثابت حرف *Ptr, size_t اندازه, uint64_t دننه_واټن) override {
      هذا->flush();
      F1->getOS().pwrite(Ptr, اندازه, دننه_واټن);
      F2->getOS().pwrite(Ptr, اندازه, دننه_واټن);
    }
    uint64_t current_pos() ثابت override { أرجع F1->getOS().tell(); }
    size_t preferred_بفر_اندازه() ثابت override {
      أرجع Preferred_بفر_اندازه;
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
      raw_pwrite_بهیر::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_بفر_اندازه(std::max(F1->getOS().Get_بفر_اندازه(),
                                       F1->getOS().Get_بفر_اندازه())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_بفر_اندازه;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  بنية MirroringOutputBackend : عام ProxyOutputBackend1,
                                  عام ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef لار,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      إذا (Error E =
              ProxyOutputBackend1::createFileImpl(لار, Config).moveInto(File1))
        أرجع std::move(E);
      إذا (Error E =
              ProxyOutputBackend2::createFileImpl(لار, Config).moveInto(File2))
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
    Config->سېټ_No_Atomic_Write();
  إذا (!Settings.RemoveOnSignal)
    Config->سېټ_No_Discard_On_Signal();
  أرجع *Config;
}

نطاق {
صنف OnDiskOutputFile final : عام OutputFileImpl {
عام:
  Error keep() override;
  Error discard() override;
  raw_pwrite_بهیر &getOS() override {
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
  Error initialize_بهیر();
  Error reset();

  OnDiskOutputFile(StringRef Output_لار, std::optional<OutputConfig> Config,
                   ثابت OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        Output_لار(Output_لار.str()) {}

  OutputConfig Config;
  ثابت std::string Output_لار;
  std::optional<std::string> لنډمهاله_لار;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<بفر_ostream> بفر_OS;
};
} // end namespace

ساكن Error createDirectoriesOnDemand(StringRef Output_لار,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  أرجع handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    إذا (EC->convert_منزل_Error_کوډ() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      أرجع Error(std::move(EC));

    StringRef پلار_لار_2 = sys::لار_2::پلار_لار(Output_لار);
    إذا (std::error_کوډ EC = sys::fs::create_directories(پلار_لار_2))
      أرجع make_error<OutputError>(پلار_لار_2, EC);
    أرجع CreateFile();
  });
}

ساكن sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags د_2 = sys::fs::د_None;
  إذا (Config.get_متن_With_CRLF())
    د_2 |= sys::fs::د_متن_With_CRLF;
  وإلا إذا (Config.get_متن())
    د_2 |= sys::fs::د_متن;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  إذا (Config.getAppend() && !Config.getAtomicWrite())
    د_2 |= sys::fs::د_Append;

  أرجع د_2;
}

Error OnDiskOutputFile::try_منزل_Create_Temporary(std::optional<صحيح> &FD) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::لار_2::extension(Output_لار);
  SmallString<128> Model_لار =
      StringRef(Output_لار).drop_شا(OutputExtension.اندازه_2());
  Model_لار += "-%%%%%%%%";
  Model_لار += OutputExtension;
  Model_لار += ".tmp";

  أرجع createDirectoriesOnDemand(Output_لار, Config, [&]() -> Error {
    صحيح NewFD;
    SmallString<128> Unique_لار;
    sys::fs::OpenFlags د_2 = generateFlagsFromConfig(Config);
    إذا (std::error_کوډ EC =
            sys::fs::createUniqueFile(Model_لار, NewFD, Unique_لار, د_2))
      أرجع make_error<لنډمهاله_File_Output_Error>(Model_لار, Output_لار, EC);

    إذا (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Unique_لار);

    لنډمهاله_لار = Unique_لار.str().str();
    FD.emplace(NewFD);
    أرجع Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<صحيح> &FD) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  أكد(Output_لار != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  إذا (Config.getAtomicWrite()) {
    sys::fs::file_حالت حالت;
    sys::fs::حالت_2(Output_لار, حالت);
    إذا (sys::fs::exists(حالت)) {
      إذا (!sys::fs::is_regular_file(حالت))
        Config.سېټ_No_Atomic_Write();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      إذا (Config.getAppend() && !sys::fs::can_write(Output_لار))
        أرجع make_error<OutputError>(
            Output_لار,
            std::make_error_کوډ(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  إذا (Config.getAtomicWrite())
    إذا (!error_منزل_Bool(try_منزل_Create_Temporary(FD)))
      أرجع Error::success();

  // Not using a temporary file. Open the final output file.
  أرجع createDirectoriesOnDemand(Output_لار, Config, [&]() -> Error {
    صحيح NewFD;
    sys::fs::OpenFlags د_2 = generateFlagsFromConfig(Config);
    إذا (std::error_کوډ EC = sys::fs::openFileForWrite(
            Output_لار, NewFD, sys::fs::CD_CreateAlways, د_2))
      أرجع convert_منزل_Output_Error(Output_لار, EC);
    FD.emplace(NewFD);

    إذا (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Output_لار);
    أرجع Error::success();
  });
}

Error OnDiskOutputFile::initialize_بهیر() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  إذا (Output_لار == "-") {
    std::error_کوډ EC;
    FileOS.emplace(Output_لار, EC);
    إذا (EC)
      أرجع make_error<OutputError>(Output_لار, EC);
  } وإلا {
    std::optional<صحيح> FD;
    إذا (Error E = initializeFile(FD))
      أرجع E;
    FileOS.emplace(*FD, /*shouldClose=*/ریښتیا);
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
areFilesDifferent(ثابت llvm::Twine &سرچینه, ثابت llvm::Twine &Destination) {
  إذا (sys::fs::equivalent(سرچینه, Destination))
    أرجع FileDifference::IdenticalFile;

  OpenFileRAII سرچینه_File;
  sys::fs::file_حالت سرچینه_حالت;
  // If we can't open the source file, fail.
  إذا (std::error_کوډ EC = sys::fs::openFileForRead(سرچینه, سرچینه_File.Fd))
    أرجع convert_منزل_Output_Error(سرچینه, EC);

  // If we can't stat the source file, fail.
  إذا (std::error_کوډ EC = sys::fs::حالت_2(سرچینه_File.Fd, سرچینه_حالت))
    أرجع convert_منزل_Output_Error(سرچینه, EC);

  OpenFileRAII DestFile;
  sys::fs::file_حالت Dest_حالت;
  // If we can't open the destination file, report different.
  إذا (std::error_کوډ Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    أرجع FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  إذا (std::error_کوډ Error = sys::fs::حالت_2(DestFile.Fd, Dest_حالت))
    أرجع FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t اندازه = سرچینه_حالت.get_اندازه();
  إذا (اندازه != Dest_حالت.get_اندازه())
    أرجع FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  إذا (اندازه == 0)
    أرجع FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_کوډ سرچینه_Region_Err;
  sys::fs::mapped_file_region سرچینه_Region(
      sys::fs::convertFDToNativeFile(سرچینه_File.Fd),
      sys::fs::mapped_file_region::readonly, اندازه, 0, سرچینه_Region_Err);
  إذا (سرچینه_Region_Err)
    أرجع convert_منزل_Output_Error(سرچینه, سرچینه_Region_Err);

  std::error_کوډ DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, اندازه, 0, DestRegionErr);

  إذا (DestRegionErr)
    أرجع FileDifference::DifferentContents;

  إذا (memcmp(سرچینه_Region.const_معلومات(), DestRegion.const_معلومات(), اندازه) != 0)
    أرجع FileDifference::DifferentContents;

  أرجع FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  بفر_OS.reset();
  إذا (!FileOS)
    أرجع Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_کوډ EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  أرجع error_کوډ_منزل_Error(EC);
}

Error OnDiskOutputFile::keep() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  إذا (خپلکاری E = reset())
    أرجع E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    إذا (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(لنډمهاله_لار ? *لنډمهاله_لار : Output_لار);
  });

  إذا (!لنډمهاله_لار)
    أرجع Error::success();

  // See if we should append instead of move.
  إذا (Config.getAppend() && Output_لار != "-") {
    // Read TempFile for the content to append.
    خپلکاری Content = حافظه_بفر::getFile(*لنډمهاله_لار);
    إذا (!Content)
      أرجع convert_منزل_لنډمهاله_File_Output_Error(*لنډمهاله_لار, Output_لار,
                                          Content.getError());
    طالما (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::قلف_File_Manager قلف_2(Output_لار);
      منطقي Owned;
      إذا (Error Err = قلف_2.try_قلف().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        قلف_2.unsafeUnlock();
        أرجع convert_منزل_Output_Error(
            Output_لار, std::make_error_کوډ(std::errc::no_قلف_شته));
      }
      إذا (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_کوډ EC;
        llvm::raw_fd_ostream بهر(Output_لار, EC, llvm::sys::fs::د_Append);
        إذا (EC)
          أرجع convert_منزل_Output_Error(Output_لار, EC);
        بهر << (*Content)->get_بفر();
        بهر.close();
        قلف_2.unsafeUnlock();
        إذا (بهر.has_error())
          أرجع convert_منزل_Output_Error(Output_لار, بهر.error());
        // Remove temp file and done.
        (فراغ)sys::fs::remove(*لنډمهاله_لار);
        أرجع Error::success();
      }
      // Someone else owns the lock on this file, wait.
      بدل (قلف_2.waitForUnlockFor(std::chrono::seconds(256))) {
      حالة Wait_For_Unlock_پایله::Success:
        [[fallthrough]];
      حالة Wait_For_Unlock_پایله::OwnerDied: {
        استمر; // try again to get the lock.
      }
      حالة Wait_For_Unlock_پایله::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        قلف_2.unsafeUnlock();
        استمر;
      }
      }
      اكسر;
    }
  }

  إذا (Config.getOnlyIfDifferent()) {
    خپلکاری پایله = areFilesDifferent(*لنډمهاله_لار, Output_لار);
    إذا (!پایله)
      أرجع پایله.takeError();
    بدل (*پایله) {
    حالة FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      أرجع Error::success();

    حالة FileDifference::SameContents:
      // Files are identical; remove the source file.
      (فراغ)sys::fs::remove(*لنډمهاله_لار);
      أرجع Error::success();

    حالة FileDifference::DifferentContents:
      اكسر; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_کوډ RenameEC = sys::fs::rename(*لنډمهاله_لار, Output_لار);
  إذا (!RenameEC)
    أرجع Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*لنډمهاله_لار, Output_لار);
  (فراغ)sys::fs::remove(*لنډمهاله_لار);

  إذا (!RenameEC)
    أرجع Error::success();

  أرجع make_error<لنډمهاله_File_Output_Error>(*لنډمهاله_لار, Output_لار, RenameEC);
}

Error OnDiskOutputFile::discard() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  إذا (خپلکاری E = reset())
    أرجع E;

  // Nothing on the filesystem to remove for stdout.
  إذا (Output_لار == "-")
    أرجع Error::success();

  خپلکاری discard_لار = [&](StringRef لار) {
    std::error_کوډ EC = sys::fs::remove(لار);
    sys::DontRemoveFileOnSignal(لار);
    أرجع EC;
  };

  // Clean up the file that's in-progress.
  إذا (!لنډمهاله_لار)
    أرجع convert_منزل_Output_Error(Output_لار, discard_لار(Output_لار));
  أرجع convert_منزل_لنډمهاله_File_Output_Error(*لنډمهاله_لار, Output_لار,
                                      discard_لار(*لنډمهاله_لار));
}

Error OnDiskOutputBackend::makeAbsolute(Small_ویکټور_Impl<حرف> &لار) ثابت {
  // FIXME: Should this really call sys::fs::make_absolute?
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();
  أرجع convert_منزل_Output_Error(StringRef(لار.معلومات(), لار.اندازه_2()),
                              sys::fs::make_absolute(لار));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef لار,
                                    std::optional<OutputConfig> Config) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Abs_لار;
  إذا (لار != "-") {
    Abs_لار = لار;
    إذا (Error E = makeAbsolute(Abs_لار))
      أرجع std::move(E);
    لار = Abs_لار;
  }

  خپلکاری File = std::make_unique<OnDiskOutputFile>(لار, Config, Settings);
  إذا (Error E = File->initialize_بهیر())
    أرجع std::move(E);

  أرجع std::move(File);
}
