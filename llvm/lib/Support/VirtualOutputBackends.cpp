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

using نطاق llvm;
using نطاق llvm::vfs;

فراغ ProxyOutputBackend::anchor() {}
فراغ OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  بنية NullOutputBackend : عام OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      أرجع const_cast<NullOutputBackend *>(هذا);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
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
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      𐡄𐡍 (Filter(Path, Config))
        أرجع ProxyOutputBackend::createFileImpl(Path, Config);
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
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  بنية ProxyOutputBackend2 : عام ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  بنية MirroringOutput final : عام OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      أرجع joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      أرجع joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { أرجع *هذا; }

    فراغ write_impl(ثابت حرف *Ptr, size_t Size) override {
      F1->getOS().اكتب(Ptr, Size);
      F2->getOS().اكتب(Ptr, Size);
    }
    فراغ pwrite_impl(ثابت حرف *Ptr, size_t Size, uint64_t Offset) override {
      هذا->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() ثابت override { أرجع F1->getOS().tell(); }
    size_t preferred_buffer_size() ثابت override {
      أرجع PreferredBufferSize;
    }
    فراغ reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    منطقي is_displayed() ثابت override {
      أرجع F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    منطقي has_colors() ثابت override {
      أرجع F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    فراغ enable_colors(منطقي enable) override {
      raw_pwrite_stream::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : PreferredBufferSize(std::max(F1->getOS().GetBufferSize(),
                                       F1->getOS().GetBufferSize())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t PreferredBufferSize;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  بنية MirroringOutputBackend : عام ProxyOutputBackend1,
                                  عام ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      𐡄𐡍 (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        أرجع std::move(E);
      𐡄𐡍 (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        أرجع joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      𐡄𐡍 (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        أرجع std::move(File2);
      }
      𐡄𐡍 (isa<NullOutputFileImpl>(*File2)) {
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
  𐡄𐡍 (!Config)
    Config = Settings.DefaultConfig;
  𐡄𐡍 (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  𐡄𐡍 (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  أرجع *Config;
}

نطاق {
صنف OnDiskOutputFile final : عام OutputFileImpl {
عام:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    أكد(FileOS && "Expected valid file");
    𐡄𐡍 (BufferOS)
      أرجع *BufferOS;
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
  Error tryToCreateTemporary(std::optional<صحيح> &FD);

  Error initializeFile(std::optional<صحيح> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   ثابت OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  ثابت std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

ساكن Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  أرجع handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    𐡄𐡍 (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      أرجع Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    𐡄𐡍 (std::error_code EC = sys::fs::create_directories(ParentPath))
      أرجع make_error<OutputError>(ParentPath, EC);
    أرجع CreateFile();
  });
}

ساكن sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  𐡄𐡍 (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  وإلا 𐡄𐡍 (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  𐡄𐡍 (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  أرجع OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<صحيح> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::path::extension(OutputPath);
  SmallString<128> ModelPath =
      StringRef(OutputPath).drop_back(OutputExtension.size());
  ModelPath += "-%%%%%%%%";
  ModelPath += OutputExtension;
  ModelPath += ".tmp";

  أرجع createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    صحيح NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    𐡄𐡍 (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      أرجع make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    𐡄𐡍 (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    أرجع Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<صحيح> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  𐡄𐡍 (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    𐡄𐡍 (sys::fs::exists(Status)) {
      𐡄𐡍 (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      𐡄𐡍 (Config.getAppend() && !sys::fs::can_write(OutputPath))
        أرجع make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  𐡄𐡍 (Config.getAtomicWrite())
    𐡄𐡍 (!errorToBool(tryToCreateTemporary(FD)))
      أرجع Error::success();

  // Not using a temporary file. Open the final output file.
  أرجع createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    صحيح NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    𐡄𐡍 (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      أرجع convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    𐡄𐡍 (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    أرجع Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  𐡄𐡍 (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    𐡄𐡍 (EC)
      أرجع make_error<OutputError>(OutputPath, EC);
  } وإلا {
    std::optional<صحيح> FD;
    𐡄𐡍 (Error E = initializeFile(FD))
      أرجع E;
    FileOS.emplace(*FD, /*shouldClose=*/𐡒𐡔𐡈);
  }

  // Buffer the stream if necessary.
  𐡄𐡍 (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  أرجع Error::success();
}

نطاق {
صنف OpenFileRAII {
  ساكن ثابت صحيح InvalidFd = -1;

عام:
  صحيح Fd = InvalidFd;

  ~OpenFileRAII() {
    𐡄𐡍 (Fd != InvalidFd)
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
areFilesDifferent(ثابت llvm::Twine &Source, ثابت llvm::Twine &Destination) {
  𐡄𐡍 (sys::fs::equivalent(Source, Destination))
    أرجع FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  𐡄𐡍 (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    أرجع convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  𐡄𐡍 (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    أرجع convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  𐡄𐡍 (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    أرجع FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  𐡄𐡍 (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    أرجع FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  𐡄𐡍 (Size != DestStatus.getSize())
    أرجع FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  𐡄𐡍 (Size == 0)
    أرجع FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  𐡄𐡍 (SourceRegionErr)
    أرجع convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  𐡄𐡍 (DestRegionErr)
    أرجع FileDifference::DifferentContents;

  𐡄𐡍 (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    أرجع FileDifference::DifferentContents;

  أرجع FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  𐡄𐡍 (!FileOS)
    أرجع Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  أرجع errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  𐡄𐡍 (auto E = reset())
    أرجع E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    𐡄𐡍 (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  𐡄𐡍 (!TempPath)
    أرجع Error::success();

  // See if we should append instead of move.
  𐡄𐡍 (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    𐡄𐡍 (!Content)
      أرجع convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    طالما (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      منطقي Owned;
      𐡄𐡍 (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        أرجع convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      𐡄𐡍 (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        𐡄𐡍 (EC)
          أرجع convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        𐡄𐡍 (Out.has_error())
          أرجع convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (فراغ)sys::fs::remove(*TempPath);
        أرجع Error::success();
      }
      // Someone else owns the lock on this file, wait.
      بدل (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      حالة WaitForUnlockResult::Success:
        [[fallthrough]];
      حالة WaitForUnlockResult::OwnerDied: {
        استمر; // try again to get the lock.
      }
      حالة WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        استمر;
      }
      }
      اكسر;
    }
  }

  𐡄𐡍 (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    𐡄𐡍 (!Result)
      أرجع Result.takeError();
    بدل (*Result) {
    حالة FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      أرجع Error::success();

    حالة FileDifference::SameContents:
      // Files are identical; remove the source file.
      (فراغ)sys::fs::remove(*TempPath);
      أرجع Error::success();

    حالة FileDifference::DifferentContents:
      اكسر; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  𐡄𐡍 (!RenameEC)
    أرجع Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (فراغ)sys::fs::remove(*TempPath);

  𐡄𐡍 (!RenameEC)
    أرجع Error::success();

  أرجع make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  𐡄𐡍 (auto E = reset())
    أرجع E;

  // Nothing on the filesystem to remove for stdout.
  𐡄𐡍 (OutputPath == "-")
    أرجع Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    أرجع EC;
  };

  // Clean up the file that's in-progress.
  𐡄𐡍 (!TempPath)
    أرجع convertToOutputError(OutputPath, discardPath(OutputPath));
  أرجع convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<حرف> &Path) ثابت {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  أرجع convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  𐡄𐡍 (Path != "-") {
    AbsPath = Path;
    𐡄𐡍 (Error E = makeAbsolute(AbsPath))
      أرجع std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  𐡄𐡍 (Error E = File->initializeStream())
    أرجع std::move(E);

  أرجع std::move(File);
}
