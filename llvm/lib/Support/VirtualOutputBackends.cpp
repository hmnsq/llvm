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
      החזר const_cast<NullOutputBackend *>(هذا);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      החזר std::make_unique<NullOutputFileImpl>();
    }
  };

  החזר makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<منطقي(StringRef, std::optional<OutputConfig>)> Filter) {
  بنية FilteringOutputBackend : عام ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      אם (Filter(Path, Config))
        החזר ProxyOutputBackend::createFileImpl(Path, Config);
      החזר std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      החזר makeIntrusiveRefCnt<FilteringOutputBackend>(
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

  החזר makeIntrusiveRefCnt<FilteringOutputBackend>(
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
      החזר joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      החזר joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { החזר *هذا; }

    فراغ write_impl(ثابت حرف *Ptr, size_t Size) override {
      F1->getOS().اكتب(Ptr, Size);
      F2->getOS().اكتب(Ptr, Size);
    }
    فراغ pwrite_impl(ثابت حرف *Ptr, size_t Size, uint64_t Offset) override {
      هذا->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() ثابت override { החזר F1->getOS().tell(); }
    size_t preferred_buffer_size() ثابت override {
      החזר PreferredBufferSize;
    }
    فراغ reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    منطقي is_displayed() ثابت override {
      החזר F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    منطقي has_colors() ثابت override {
      החזר F1->getOS().has_colors() && F2->getOS().has_colors();
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
      אם (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        החזר std::move(E);
      אם (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        החזר joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      אם (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        החזר std::move(File2);
      }
      אם (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        החזר std::move(File1);
      }
      החזר std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      החזר IntrusiveRefCntPtr<ProxyOutputBackend1>(
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
  החזר IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ساكن OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ثابت OnDiskOutputBackend::OutputSettings &Settings) {
  אם (!Config)
    Config = Settings.DefaultConfig;
  אם (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  אם (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  החזר *Config;
}

نطاق {
מחלקה OnDiskOutputFile final : عام OutputFileImpl {
عام:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    أكد(FileOS && "Expected valid file");
    אם (BufferOS)
      החזר *BufferOS;
    החזר *FileOS;
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
  החזר handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    אם (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      החזר Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    אם (std::error_code EC = sys::fs::create_directories(ParentPath))
      החזר make_error<OutputError>(ParentPath, EC);
    החזר CreateFile();
  });
}

ساكن sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  אם (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  אחרת אם (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  אם (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  החזר OF;
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

  החזר createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    صحيح NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    אם (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      החזר make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    אם (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    החזר Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<صحيح> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  אם (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    אם (sys::fs::exists(Status)) {
      אם (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      אם (Config.getAppend() && !sys::fs::can_write(OutputPath))
        החזר make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  אם (Config.getAtomicWrite())
    אם (!errorToBool(tryToCreateTemporary(FD)))
      החזר Error::success();

  // Not using a temporary file. Open the final output file.
  החזר createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    صحيح NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    אם (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      החזר convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    אם (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    החזר Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  אם (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    אם (EC)
      החזר make_error<OutputError>(OutputPath, EC);
  } אחרת {
    std::optional<صحيح> FD;
    אם (Error E = initializeFile(FD))
      החזר E;
    FileOS.emplace(*FD, /*shouldClose=*/אמת);
  }

  // Buffer the stream if necessary.
  אם (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  החזר Error::success();
}

نطاق {
מחלקה OpenFileRAII {
  ساكن ثابت صحيح InvalidFd = -1;

عام:
  صحيح Fd = InvalidFd;

  ~OpenFileRAII() {
    אם (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

تعداد מחלקה FileDifference : uint8_t {
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
  אם (sys::fs::equivalent(Source, Destination))
    החזר FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  אם (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    החזר convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  אם (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    החזר convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  אם (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    החזר FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  אם (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    החזר FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  אם (Size != DestStatus.getSize())
    החזר FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  אם (Size == 0)
    החזר FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  אם (SourceRegionErr)
    החזר convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  אם (DestRegionErr)
    החזר FileDifference::DifferentContents;

  אם (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    החזר FileDifference::DifferentContents;

  החזר FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  אם (!FileOS)
    החזר Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  החזר errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  אם (auto E = reset())
    החזר E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    אם (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  אם (!TempPath)
    החזר Error::success();

  // See if we should append instead of move.
  אם (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    אם (!Content)
      החזר convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    כלעוד (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      منطقي Owned;
      אם (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        החזר convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      אם (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        אם (EC)
          החזר convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        אם (Out.has_error())
          החזר convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (فراغ)sys::fs::remove(*TempPath);
        החזר Error::success();
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

  אם (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    אם (!Result)
      החזר Result.takeError();
    بدل (*Result) {
    حالة FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      החזר Error::success();

    حالة FileDifference::SameContents:
      // Files are identical; remove the source file.
      (فراغ)sys::fs::remove(*TempPath);
      החזר Error::success();

    حالة FileDifference::DifferentContents:
      اكسر; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  אם (!RenameEC)
    החזר Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (فراغ)sys::fs::remove(*TempPath);

  אם (!RenameEC)
    החזר Error::success();

  החזר make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  אם (auto E = reset())
    החזר E;

  // Nothing on the filesystem to remove for stdout.
  אם (OutputPath == "-")
    החזר Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    החזר EC;
  };

  // Clean up the file that's in-progress.
  אם (!TempPath)
    החזר convertToOutputError(OutputPath, discardPath(OutputPath));
  החזר convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<حرف> &Path) ثابت {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  החזר convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  אם (Path != "-") {
    AbsPath = Path;
    אם (Error E = makeAbsolute(AbsPath))
      החזר std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  אם (Error E = File->initializeStream())
    החזר std::move(E);

  החזר std::move(File);
}
