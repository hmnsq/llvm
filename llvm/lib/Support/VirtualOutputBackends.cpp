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

#അന്തര്ഭാവ "llvm/Support/VirtualOutputBackends.h"
#അന്തര്ഭാവ "llvm/ADT/ScopeExit.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/IOSandbox.h"
#അന്തര്ഭാവ "llvm/Support/LockFileManager.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/Support/Signals.h"
#അന്തര്ഭാവ "llvm/Support/VirtualOutputConfig.h"
#അന്തര്ഭാവ "llvm/Support/VirtualOutputError.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::vfs;

ശൂന്യം ProxyOutputBackend::anchor() {}
ശൂന്യം OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  ഘടന NullOutputBackend : പൊതു OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() സ്ഥിരം override {
      തിരികെ const_cast<NullOutputBackend *>(ഇത്);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      തിരികെ std::make_unique<NullOutputFileImpl>();
    }
  };

  തിരികെ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<തർക്കം(StringRef, std::optional<OutputConfig>)> Filter) {
  ഘടന FilteringOutputBackend : പൊതു ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      എങ്കിൽ (Filter(Path, Config))
        തിരികെ ProxyOutputBackend::createFileImpl(Path, Config);
      തിരികെ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() സ്ഥിരം override {
      തിരികെ makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<തർക്കം(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      പ്രതിജ്ഞാ(ഇത്->Filter && "Expected a non-null function");
    }
    std::function<തർക്കം(StringRef, std::optional<OutputConfig>)> Filter;
  };

  തിരികെ makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  ഘടന ProxyOutputBackend1 : പൊതു ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  ഘടന ProxyOutputBackend2 : പൊതു ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  ഘടന MirroringOutput final : പൊതു OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      തിരികെ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      തിരികെ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { തിരികെ *ഇത്; }

    ശൂന്യം write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) override {
      F1->getOS().ലിഖ(Ptr, Size);
      F2->getOS().ലിഖ(Ptr, Size);
    }
    ശൂന്യം pwrite_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size, uint64_t Offset) override {
      ഇത്->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() സ്ഥിരം override { തിരികെ F1->getOS().tell(); }
    size_t preferred_buffer_size() സ്ഥിരം override {
      തിരികെ PreferredBufferSize;
    }
    ശൂന്യം reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    തർക്കം is_displayed() സ്ഥിരം override {
      തിരികെ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    തർക്കം has_colors() സ്ഥിരം override {
      തിരികെ F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    ശൂന്യം enable_colors(തർക്കം enable) override {
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
  ഘടന MirroringOutputBackend : പൊതു ProxyOutputBackend1,
                                  പൊതു ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      എങ്കിൽ (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        തിരികെ std::move(E);
      എങ്കിൽ (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        തിരികെ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      എങ്കിൽ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        തിരികെ std::move(File2);
      }
      എങ്കിൽ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        തിരികെ std::move(File1);
      }
      തിരികെ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() സ്ഥിരം override {
      തിരികെ IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    ശൂന്യം Retain() സ്ഥിരം { ProxyOutputBackend1::Retain(); }
    ശൂന്യം Release() സ്ഥിരം { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  പ്രതിജ്ഞാ(Backend1 && "Expected actual backend");
  പ്രതിജ്ഞാ(Backend2 && "Expected actual backend");
  തിരികെ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

സ്ഥിതം OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              സ്ഥിരം OnDiskOutputBackend::OutputSettings &Settings) {
  എങ്കിൽ (!Config)
    Config = Settings.DefaultConfig;
  എങ്കിൽ (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  എങ്കിൽ (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  തിരികെ *Config;
}

നാമാകാശ {
വർഗ്ഗം OnDiskOutputFile final : പൊതു OutputFileImpl {
പൊതു:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    പ്രതിജ്ഞാ(FileOS && "Expected valid file");
    എങ്കിൽ (BufferOS)
      തിരികെ *BufferOS;
    തിരികെ *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<പൂർണ്ണസംഖ്യ> &FD);

  Error initializeFile(std::optional<പൂർണ്ണസംഖ്യ> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   സ്ഥിരം OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  സ്ഥിരം std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

സ്ഥിതം Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  തിരികെ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    എങ്കിൽ (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      തിരികെ Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    എങ്കിൽ (std::error_code EC = sys::fs::create_directories(ParentPath))
      തിരികെ make_error<OutputError>(ParentPath, EC);
    തിരികെ CreateFile();
  });
}

സ്ഥിതം sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  എങ്കിൽ (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  അല്ലെങ്കിൽ എങ്കിൽ (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  എങ്കിൽ (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  തിരികെ OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<പൂർണ്ണസംഖ്യ> &FD) {
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

  തിരികെ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    പൂർണ്ണസംഖ്യ NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    എങ്കിൽ (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      തിരികെ make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    എങ്കിൽ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    തിരികെ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<പൂർണ്ണസംഖ്യ> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  പ്രതിജ്ഞാ(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  എങ്കിൽ (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    എങ്കിൽ (sys::fs::exists(Status)) {
      എങ്കിൽ (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      എങ്കിൽ (Config.getAppend() && !sys::fs::can_write(OutputPath))
        തിരികെ make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  എങ്കിൽ (Config.getAtomicWrite())
    എങ്കിൽ (!errorToBool(tryToCreateTemporary(FD)))
      തിരികെ Error::success();

  // Not using a temporary file. Open the final output file.
  തിരികെ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    പൂർണ്ണസംഖ്യ NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    എങ്കിൽ (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      തിരികെ convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    എങ്കിൽ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    തിരികെ Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  എങ്കിൽ (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    എങ്കിൽ (EC)
      തിരികെ make_error<OutputError>(OutputPath, EC);
  } അല്ലെങ്കിൽ {
    std::optional<പൂർണ്ണസംഖ്യ> FD;
    എങ്കിൽ (Error E = initializeFile(FD))
      തിരികെ E;
    FileOS.emplace(*FD, /*shouldClose=*/സത്യം);
  }

  // Buffer the stream if necessary.
  എങ്കിൽ (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  തിരികെ Error::success();
}

നാമാകാശ {
വർഗ്ഗം OpenFileRAII {
  സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ InvalidFd = -1;

പൊതു:
  പൂർണ്ണസംഖ്യ Fd = InvalidFd;

  ~OpenFileRAII() {
    എങ്കിൽ (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

എണ്ണൽ വർഗ്ഗം FileDifference : uint8_t {
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

സ്ഥിതം Expected<FileDifference>
areFilesDifferent(സ്ഥിരം llvm::Twine &Source, സ്ഥിരം llvm::Twine &Destination) {
  എങ്കിൽ (sys::fs::equivalent(Source, Destination))
    തിരികെ FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  എങ്കിൽ (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    തിരികെ convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  എങ്കിൽ (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    തിരികെ convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  എങ്കിൽ (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    തിരികെ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  എങ്കിൽ (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    തിരികെ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  എങ്കിൽ (Size != DestStatus.getSize())
    തിരികെ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  എങ്കിൽ (Size == 0)
    തിരികെ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  എങ്കിൽ (SourceRegionErr)
    തിരികെ convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  എങ്കിൽ (DestRegionErr)
    തിരികെ FileDifference::DifferentContents;

  എങ്കിൽ (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    തിരികെ FileDifference::DifferentContents;

  തിരികെ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  എങ്കിൽ (!FileOS)
    തിരികെ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  തിരികെ errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  എങ്കിൽ (auto E = reset())
    തിരികെ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    എങ്കിൽ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  എങ്കിൽ (!TempPath)
    തിരികെ Error::success();

  // See if we should append instead of move.
  എങ്കിൽ (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    എങ്കിൽ (!Content)
      തിരികെ convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    ആകുമ്പോൾ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      തർക്കം Owned;
      എങ്കിൽ (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        തിരികെ convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      എങ്കിൽ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        എങ്കിൽ (EC)
          തിരികെ convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        എങ്കിൽ (Out.has_error())
          തിരികെ convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (ശൂന്യം)sys::fs::remove(*TempPath);
        തിരികെ Error::success();
      }
      // Someone else owns the lock on this file, wait.
      തിരഞ്ഞെടുപ്പ് (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      അവസ്ഥ WaitForUnlockResult::Success:
        [[fallthrough]];
      അവസ്ഥ WaitForUnlockResult::OwnerDied: {
        തുടരുക; // try again to get the lock.
      }
      അവസ്ഥ WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        തുടരുക;
      }
      }
      നിർത്തുക;
    }
  }

  എങ്കിൽ (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    എങ്കിൽ (!Result)
      തിരികെ Result.takeError();
    തിരഞ്ഞെടുപ്പ് (*Result) {
    അവസ്ഥ FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      തിരികെ Error::success();

    അവസ്ഥ FileDifference::SameContents:
      // Files are identical; remove the source file.
      (ശൂന്യം)sys::fs::remove(*TempPath);
      തിരികെ Error::success();

    അവസ്ഥ FileDifference::DifferentContents:
      നിർത്തുക; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  എങ്കിൽ (!RenameEC)
    തിരികെ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (ശൂന്യം)sys::fs::remove(*TempPath);

  എങ്കിൽ (!RenameEC)
    തിരികെ Error::success();

  തിരികെ make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  എങ്കിൽ (auto E = reset())
    തിരികെ E;

  // Nothing on the filesystem to remove for stdout.
  എങ്കിൽ (OutputPath == "-")
    തിരികെ Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    തിരികെ EC;
  };

  // Clean up the file that's in-progress.
  എങ്കിൽ (!TempPath)
    തിരികെ convertToOutputError(OutputPath, discardPath(OutputPath));
  തിരികെ convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<അക്ഷരം> &Path) സ്ഥിരം {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  തിരികെ convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  എങ്കിൽ (Path != "-") {
    AbsPath = Path;
    എങ്കിൽ (Error E = makeAbsolute(AbsPath))
      തിരികെ std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  എങ്കിൽ (Error E = File->initializeStream())
    തിരികെ std::move(E);

  തിരികെ std::move(File);
}
