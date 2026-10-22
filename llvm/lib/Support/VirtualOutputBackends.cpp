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

#அந்தர்பாவ "llvm/Support/VirtualOutputBackends.h"
#அந்தர்பாவ "llvm/ADT/ScopeExit.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/IOSandbox.h"
#அந்தர்பாவ "llvm/Support/LockFileManager.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
#அந்தர்பாவ "llvm/Support/VirtualOutputConfig.h"
#அந்தர்பாவ "llvm/Support/VirtualOutputError.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::vfs;

வெறுமை ProxyOutputBackend::anchor() {}
வெறுமை OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  கட்டமைப்பு NullOutputBackend : பொது OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() மாறிலி override {
      திரும்பு const_cast<NullOutputBackend *>(இது);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      திரும்பு std::make_unique<NullOutputFileImpl>();
    }
  };

  திரும்பு makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<தருக்க(StringRef, std::optional<OutputConfig>)> Filter) {
  கட்டமைப்பு FilteringOutputBackend : பொது ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      எனில் (Filter(Path, Config))
        திரும்பு ProxyOutputBackend::createFileImpl(Path, Config);
      திரும்பு std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() மாறிலி override {
      திரும்பு makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<தருக்க(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      ப்ரதிஜ்ஞா(இது->Filter && "Expected a non-null function");
    }
    std::function<தருக்க(StringRef, std::optional<OutputConfig>)> Filter;
  };

  திரும்பு makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  கட்டமைப்பு ProxyOutputBackend1 : பொது ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  கட்டமைப்பு ProxyOutputBackend2 : பொது ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  கட்டமைப்பு MirroringOutput final : பொது OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      திரும்பு joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      திரும்பு joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { திரும்பு *இது; }

    வெறுமை write_impl(மாறிலி எழுத்து *Ptr, size_t Size) override {
      F1->getOS().லிக(Ptr, Size);
      F2->getOS().லிக(Ptr, Size);
    }
    வெறுமை pwrite_impl(மாறிலி எழுத்து *Ptr, size_t Size, uint64_t Offset) override {
      இது->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() மாறிலி override { திரும்பு F1->getOS().tell(); }
    size_t preferred_buffer_size() மாறிலி override {
      திரும்பு PreferredBufferSize;
    }
    வெறுமை reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    தருக்க is_displayed() மாறிலி override {
      திரும்பு F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    தருக்க has_colors() மாறிலி override {
      திரும்பு F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    வெறுமை enable_colors(தருக்க enable) override {
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
  கட்டமைப்பு MirroringOutputBackend : பொது ProxyOutputBackend1,
                                  பொது ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      எனில் (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        திரும்பு std::move(E);
      எனில் (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        திரும்பு joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      எனில் (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        திரும்பு std::move(File2);
      }
      எனில் (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        திரும்பு std::move(File1);
      }
      திரும்பு std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() மாறிலி override {
      திரும்பு IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    வெறுமை Retain() மாறிலி { ProxyOutputBackend1::Retain(); }
    வெறுமை Release() மாறிலி { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  ப்ரதிஜ்ஞா(Backend1 && "Expected actual backend");
  ப்ரதிஜ்ஞா(Backend2 && "Expected actual backend");
  திரும்பு IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

நிலையான OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              மாறிலி OnDiskOutputBackend::OutputSettings &Settings) {
  எனில் (!Config)
    Config = Settings.DefaultConfig;
  எனில் (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  எனில் (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  திரும்பு *Config;
}

நாமாகாஶ {
வகுப்பு OnDiskOutputFile final : பொது OutputFileImpl {
பொது:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    ப்ரதிஜ்ஞா(FileOS && "Expected valid file");
    எனில் (BufferOS)
      திரும்பு *BufferOS;
    திரும்பு *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<முழு> &FD);

  Error initializeFile(std::optional<முழு> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   மாறிலி OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  மாறிலி std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

நிலையான Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  திரும்பு handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    எனில் (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      திரும்பு Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    எனில் (std::error_code EC = sys::fs::create_directories(ParentPath))
      திரும்பு make_error<OutputError>(ParentPath, EC);
    திரும்பு CreateFile();
  });
}

நிலையான sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  எனில் (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  இல்லையெனில் எனில் (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  எனில் (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  திரும்பு OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<முழு> &FD) {
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

  திரும்பு createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    முழு NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    எனில் (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      திரும்பு make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    எனில் (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    திரும்பு Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<முழு> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ப்ரதிஜ்ஞா(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  எனில் (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    எனில் (sys::fs::exists(Status)) {
      எனில் (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      எனில் (Config.getAppend() && !sys::fs::can_write(OutputPath))
        திரும்பு make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  எனில் (Config.getAtomicWrite())
    எனில் (!errorToBool(tryToCreateTemporary(FD)))
      திரும்பு Error::success();

  // Not using a temporary file. Open the final output file.
  திரும்பு createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    முழு NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    எனில் (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      திரும்பு convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    எனில் (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    திரும்பு Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  எனில் (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    எனில் (EC)
      திரும்பு make_error<OutputError>(OutputPath, EC);
  } இல்லையெனில் {
    std::optional<முழு> FD;
    எனில் (Error E = initializeFile(FD))
      திரும்பு E;
    FileOS.emplace(*FD, /*shouldClose=*/உண்மை);
  }

  // Buffer the stream if necessary.
  எனில் (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  திரும்பு Error::success();
}

நாமாகாஶ {
வகுப்பு OpenFileRAII {
  நிலையான மாறிலி முழு InvalidFd = -1;

பொது:
  முழு Fd = InvalidFd;

  ~OpenFileRAII() {
    எனில் (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

எண்ணிக்கை வகுப்பு FileDifference : uint8_t {
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

நிலையான Expected<FileDifference>
areFilesDifferent(மாறிலி llvm::Twine &Source, மாறிலி llvm::Twine &Destination) {
  எனில் (sys::fs::equivalent(Source, Destination))
    திரும்பு FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  எனில் (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    திரும்பு convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  எனில் (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    திரும்பு convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  எனில் (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    திரும்பு FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  எனில் (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    திரும்பு FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  எனில் (Size != DestStatus.getSize())
    திரும்பு FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  எனில் (Size == 0)
    திரும்பு FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  எனில் (SourceRegionErr)
    திரும்பு convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  எனில் (DestRegionErr)
    திரும்பு FileDifference::DifferentContents;

  எனில் (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    திரும்பு FileDifference::DifferentContents;

  திரும்பு FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  எனில் (!FileOS)
    திரும்பு Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  திரும்பு errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  எனில் (auto E = reset())
    திரும்பு E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    எனில் (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  எனில் (!TempPath)
    திரும்பு Error::success();

  // See if we should append instead of move.
  எனில் (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    எனில் (!Content)
      திரும்பு convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    போது (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      தருக்க Owned;
      எனில் (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        திரும்பு convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      எனில் (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        எனில் (EC)
          திரும்பு convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        எனில் (Out.has_error())
          திரும்பு convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (வெறுமை)sys::fs::remove(*TempPath);
        திரும்பு Error::success();
      }
      // Someone else owns the lock on this file, wait.
      தேர்வு (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      நிலை WaitForUnlockResult::Success:
        [[fallthrough]];
      நிலை WaitForUnlockResult::OwnerDied: {
        தொடர்; // try again to get the lock.
      }
      நிலை WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        தொடர்;
      }
      }
      நிறுத்து;
    }
  }

  எனில் (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    எனில் (!Result)
      திரும்பு Result.takeError();
    தேர்வு (*Result) {
    நிலை FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      திரும்பு Error::success();

    நிலை FileDifference::SameContents:
      // Files are identical; remove the source file.
      (வெறுமை)sys::fs::remove(*TempPath);
      திரும்பு Error::success();

    நிலை FileDifference::DifferentContents:
      நிறுத்து; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  எனில் (!RenameEC)
    திரும்பு Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (வெறுமை)sys::fs::remove(*TempPath);

  எனில் (!RenameEC)
    திரும்பு Error::success();

  திரும்பு make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  எனில் (auto E = reset())
    திரும்பு E;

  // Nothing on the filesystem to remove for stdout.
  எனில் (OutputPath == "-")
    திரும்பு Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    திரும்பு EC;
  };

  // Clean up the file that's in-progress.
  எனில் (!TempPath)
    திரும்பு convertToOutputError(OutputPath, discardPath(OutputPath));
  திரும்பு convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<எழுத்து> &Path) மாறிலி {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  திரும்பு convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  எனில் (Path != "-") {
    AbsPath = Path;
    எனில் (Error E = makeAbsolute(AbsPath))
      திரும்பு std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  எனில் (Error E = File->initializeStream())
    திரும்பு std::move(E);

  திரும்பு std::move(File);
}
