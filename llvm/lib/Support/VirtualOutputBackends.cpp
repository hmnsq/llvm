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

using فضاینام llvm;
using فضاینام llvm::vfs;

تهی ProxyOutputBackend::anchor() {}
تهی OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  ساختار NullOutputBackend : عمومی OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      بازگشت const_cast<NullOutputBackend *>(این);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      بازگشت std::make_unique<NullOutputFileImpl>();
    }
  };

  بازگشت makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<منطقی(StringRef, std::optional<OutputConfig>)> Filter) {
  ساختار FilteringOutputBackend : عمومی ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      اگر (Filter(Path, Config))
        بازگشت ProxyOutputBackend::createFileImpl(Path, Config);
      بازگشت std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      بازگشت makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<منطقی(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      أكد(این->Filter && "Expected a non-null function");
    }
    std::function<منطقی(StringRef, std::optional<OutputConfig>)> Filter;
  };

  بازگشت makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  ساختار ProxyOutputBackend1 : عمومی ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  ساختار ProxyOutputBackend2 : عمومی ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  ساختار MirroringOutput final : عمومی OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      بازگشت joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      بازگشت joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { بازگشت *این; }

    تهی write_impl(ثابت نویسه *Ptr, size_t Size) override {
      F1->getOS().اكتب(Ptr, Size);
      F2->getOS().اكتب(Ptr, Size);
    }
    تهی pwrite_impl(ثابت نویسه *Ptr, size_t Size, uint64_t Offset) override {
      این->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() ثابت override { بازگشت F1->getOS().tell(); }
    size_t preferred_buffer_size() ثابت override {
      بازگشت PreferredBufferSize;
    }
    تهی reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    منطقی is_displayed() ثابت override {
      بازگشت F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    منطقی has_colors() ثابت override {
      بازگشت F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    تهی enable_colors(منطقی enable) override {
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
  ساختار MirroringOutputBackend : عمومی ProxyOutputBackend1,
                                  عمومی ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      اگر (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        بازگشت std::move(E);
      اگر (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        بازگشت joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      اگر (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        بازگشت std::move(File2);
      }
      اگر (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        بازگشت std::move(File1);
      }
      بازگشت std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ثابت override {
      بازگشت IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    تهی Retain() ثابت { ProxyOutputBackend1::Retain(); }
    تهی Release() ثابت { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  أكد(Backend1 && "Expected actual backend");
  أكد(Backend2 && "Expected actual backend");
  بازگشت IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ایستا OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ثابت OnDiskOutputBackend::OutputSettings &Settings) {
  اگر (!Config)
    Config = Settings.DefaultConfig;
  اگر (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  اگر (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  بازگشت *Config;
}

فضاینام {
رده OnDiskOutputFile final : عمومی OutputFileImpl {
عمومی:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    أكد(FileOS && "Expected valid file");
    اگر (BufferOS)
      بازگشت *BufferOS;
    بازگشت *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<صحیح> &FD);

  Error initializeFile(std::optional<صحیح> &FD);
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

ایستا Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  بازگشت handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    اگر (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      بازگشت Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    اگر (std::error_code EC = sys::fs::create_directories(ParentPath))
      بازگشت make_error<OutputError>(ParentPath, EC);
    بازگشت CreateFile();
  });
}

ایستا sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  اگر (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  وگرنه اگر (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  اگر (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  بازگشت OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<صحیح> &FD) {
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

  بازگشت createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    صحیح NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    اگر (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      بازگشت make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    اگر (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    بازگشت Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<صحیح> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  اگر (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    اگر (sys::fs::exists(Status)) {
      اگر (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      اگر (Config.getAppend() && !sys::fs::can_write(OutputPath))
        بازگشت make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  اگر (Config.getAtomicWrite())
    اگر (!errorToBool(tryToCreateTemporary(FD)))
      بازگشت Error::success();

  // Not using a temporary file. Open the final output file.
  بازگشت createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    صحیح NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    اگر (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      بازگشت convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    اگر (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    بازگشت Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  اگر (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    اگر (EC)
      بازگشت make_error<OutputError>(OutputPath, EC);
  } وگرنه {
    std::optional<صحیح> FD;
    اگر (Error E = initializeFile(FD))
      بازگشت E;
    FileOS.emplace(*FD, /*shouldClose=*/درست);
  }

  // Buffer the stream if necessary.
  اگر (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  بازگشت Error::success();
}

فضاینام {
رده OpenFileRAII {
  ایستا ثابت صحیح InvalidFd = -1;

عمومی:
  صحیح Fd = InvalidFd;

  ~OpenFileRAII() {
    اگر (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

شمارش رده FileDifference : uint8_t {
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

ایستا Expected<FileDifference>
areFilesDifferent(ثابت llvm::Twine &Source, ثابت llvm::Twine &Destination) {
  اگر (sys::fs::equivalent(Source, Destination))
    بازگشت FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  اگر (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    بازگشت convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  اگر (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    بازگشت convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  اگر (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    بازگشت FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  اگر (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    بازگشت FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  اگر (Size != DestStatus.getSize())
    بازگشت FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  اگر (Size == 0)
    بازگشت FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  اگر (SourceRegionErr)
    بازگشت convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  اگر (DestRegionErr)
    بازگشت FileDifference::DifferentContents;

  اگر (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    بازگشت FileDifference::DifferentContents;

  بازگشت FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  اگر (!FileOS)
    بازگشت Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  بازگشت errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  اگر (auto E = reset())
    بازگشت E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    اگر (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  اگر (!TempPath)
    بازگشت Error::success();

  // See if we should append instead of move.
  اگر (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    اگر (!Content)
      بازگشت convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    وقتی (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      منطقی Owned;
      اگر (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        بازگشت convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      اگر (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        اگر (EC)
          بازگشت convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        اگر (Out.has_error())
          بازگشت convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (تهی)sys::fs::remove(*TempPath);
        بازگشت Error::success();
      }
      // Someone else owns the lock on this file, wait.
      انتخاب (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      حالت WaitForUnlockResult::Success:
        [[fallthrough]];
      حالت WaitForUnlockResult::OwnerDied: {
        ادامه; // try again to get the lock.
      }
      حالت WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        ادامه;
      }
      }
      بشکن;
    }
  }

  اگر (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    اگر (!Result)
      بازگشت Result.takeError();
    انتخاب (*Result) {
    حالت FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      بازگشت Error::success();

    حالت FileDifference::SameContents:
      // Files are identical; remove the source file.
      (تهی)sys::fs::remove(*TempPath);
      بازگشت Error::success();

    حالت FileDifference::DifferentContents:
      بشکن; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  اگر (!RenameEC)
    بازگشت Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (تهی)sys::fs::remove(*TempPath);

  اگر (!RenameEC)
    بازگشت Error::success();

  بازگشت make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  اگر (auto E = reset())
    بازگشت E;

  // Nothing on the filesystem to remove for stdout.
  اگر (OutputPath == "-")
    بازگشت Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    بازگشت EC;
  };

  // Clean up the file that's in-progress.
  اگر (!TempPath)
    بازگشت convertToOutputError(OutputPath, discardPath(OutputPath));
  بازگشت convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<نویسه> &Path) ثابت {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  بازگشت convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  اگر (Path != "-") {
    AbsPath = Path;
    اگر (Error E = makeAbsolute(AbsPath))
      بازگشت std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  اگر (Error E = File->initializeStream())
    بازگشت std::move(E);

  بازگشت std::move(File);
}
