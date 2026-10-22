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
      ܦܢܐ const_cast<NullOutputBackend *>(هذا);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      ܦܢܐ std::make_unique<NullOutputFileImpl>();
    }
  };

  ܦܢܐ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<منطقي(StringRef, std::optional<OutputConfig>)> Filter) {
  بنية FilteringOutputBackend : عام ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      ܐܢ (Filter(Path, Config))
        ܦܢܐ ProxyOutputBackend::createFileImpl(Path, Config);
      ܦܢܐ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      ܦܢܐ makeIntrusiveRefCnt<FilteringOutputBackend>(
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

  ܦܢܐ makeIntrusiveRefCnt<FilteringOutputBackend>(
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
      ܦܢܐ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ܦܢܐ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { ܦܢܐ *هذا; }

    فراغ write_impl(ثابت حرف *Ptr, size_t Size) override {
      F1->getOS().اكتب(Ptr, Size);
      F2->getOS().اكتب(Ptr, Size);
    }
    فراغ pwrite_impl(ثابت حرف *Ptr, size_t Size, uint64_t Offset) override {
      هذا->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() ثابت override { ܦܢܐ F1->getOS().tell(); }
    size_t preferred_buffer_size() ثابت override {
      ܦܢܐ PreferredBufferSize;
    }
    فراغ reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    منطقي is_displayed() ثابت override {
      ܦܢܐ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    منطقي has_colors() ثابت override {
      ܦܢܐ F1->getOS().has_colors() && F2->getOS().has_colors();
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
      ܐܢ (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        ܦܢܐ std::move(E);
      ܐܢ (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        ܦܢܐ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      ܐܢ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ܦܢܐ std::move(File2);
      }
      ܐܢ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ܦܢܐ std::move(File1);
      }
      ܦܢܐ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      ܦܢܐ IntrusiveRefCntPtr<ProxyOutputBackend1>(
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
  ܦܢܐ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ساكن OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ثابت OnDiskOutputBackend::OutputSettings &Settings) {
  ܐܢ (!Config)
    Config = Settings.DefaultConfig;
  ܐܢ (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  ܐܢ (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  ܦܢܐ *Config;
}

نطاق {
صنف OnDiskOutputFile final : عام OutputFileImpl {
عام:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    أكد(FileOS && "Expected valid file");
    ܐܢ (BufferOS)
      ܦܢܐ *BufferOS;
    ܦܢܐ *FileOS;
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
  ܦܢܐ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    ܐܢ (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ܦܢܐ Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    ܐܢ (std::error_code EC = sys::fs::create_directories(ParentPath))
      ܦܢܐ make_error<OutputError>(ParentPath, EC);
    ܦܢܐ CreateFile();
  });
}

ساكن sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  ܐܢ (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  ܐܚܪܢܐ ܐܢ (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  ܐܢ (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  ܦܢܐ OF;
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

  ܦܢܐ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    صحيح NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ܐܢ (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      ܦܢܐ make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    ܐܢ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    ܦܢܐ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<صحيح> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  ܐܢ (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    ܐܢ (sys::fs::exists(Status)) {
      ܐܢ (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      ܐܢ (Config.getAppend() && !sys::fs::can_write(OutputPath))
        ܦܢܐ make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  ܐܢ (Config.getAtomicWrite())
    ܐܢ (!errorToBool(tryToCreateTemporary(FD)))
      ܦܢܐ Error::success();

  // Not using a temporary file. Open the final output file.
  ܦܢܐ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    صحيح NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ܐܢ (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      ܦܢܐ convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    ܐܢ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    ܦܢܐ Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  ܐܢ (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    ܐܢ (EC)
      ܦܢܐ make_error<OutputError>(OutputPath, EC);
  } ܐܚܪܢܐ {
    std::optional<صحيح> FD;
    ܐܢ (Error E = initializeFile(FD))
      ܦܢܐ E;
    FileOS.emplace(*FD, /*shouldClose=*/ܫܪܪܐ);
  }

  // Buffer the stream if necessary.
  ܐܢ (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  ܦܢܐ Error::success();
}

نطاق {
صنف OpenFileRAII {
  ساكن ثابت صحيح InvalidFd = -1;

عام:
  صحيح Fd = InvalidFd;

  ~OpenFileRAII() {
    ܐܢ (Fd != InvalidFd)
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
  ܐܢ (sys::fs::equivalent(Source, Destination))
    ܦܢܐ FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  ܐܢ (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    ܦܢܐ convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  ܐܢ (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    ܦܢܐ convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  ܐܢ (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ܦܢܐ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  ܐܢ (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    ܦܢܐ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  ܐܢ (Size != DestStatus.getSize())
    ܦܢܐ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  ܐܢ (Size == 0)
    ܦܢܐ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  ܐܢ (SourceRegionErr)
    ܦܢܐ convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  ܐܢ (DestRegionErr)
    ܦܢܐ FileDifference::DifferentContents;

  ܐܢ (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    ܦܢܐ FileDifference::DifferentContents;

  ܦܢܐ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  ܐܢ (!FileOS)
    ܦܢܐ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ܦܢܐ errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ܐܢ (auto E = reset())
    ܦܢܐ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    ܐܢ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  ܐܢ (!TempPath)
    ܦܢܐ Error::success();

  // See if we should append instead of move.
  ܐܢ (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    ܐܢ (!Content)
      ܦܢܐ convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    ܟܡܐ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      منطقي Owned;
      ܐܢ (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        ܦܢܐ convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      ܐܢ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        ܐܢ (EC)
          ܦܢܐ convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        ܐܢ (Out.has_error())
          ܦܢܐ convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (فراغ)sys::fs::remove(*TempPath);
        ܦܢܐ Error::success();
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

  ܐܢ (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    ܐܢ (!Result)
      ܦܢܐ Result.takeError();
    بدل (*Result) {
    حالة FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ܦܢܐ Error::success();

    حالة FileDifference::SameContents:
      // Files are identical; remove the source file.
      (فراغ)sys::fs::remove(*TempPath);
      ܦܢܐ Error::success();

    حالة FileDifference::DifferentContents:
      اكسر; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  ܐܢ (!RenameEC)
    ܦܢܐ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (فراغ)sys::fs::remove(*TempPath);

  ܐܢ (!RenameEC)
    ܦܢܐ Error::success();

  ܦܢܐ make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ܐܢ (auto E = reset())
    ܦܢܐ E;

  // Nothing on the filesystem to remove for stdout.
  ܐܢ (OutputPath == "-")
    ܦܢܐ Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    ܦܢܐ EC;
  };

  // Clean up the file that's in-progress.
  ܐܢ (!TempPath)
    ܦܢܐ convertToOutputError(OutputPath, discardPath(OutputPath));
  ܦܢܐ convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<حرف> &Path) ثابت {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  ܦܢܐ convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  ܐܢ (Path != "-") {
    AbsPath = Path;
    ܐܢ (Error E = makeAbsolute(AbsPath))
      ܦܢܐ std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  ܐܢ (Error E = File->initializeStream())
    ܦܢܐ std::move(E);

  ܦܢܐ std::move(File);
}
