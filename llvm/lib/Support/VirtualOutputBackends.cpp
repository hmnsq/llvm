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

#અન્તર્ભાવ "llvm/Support/VirtualOutputBackends.h"
#અન્તર્ભાવ "llvm/ADT/ScopeExit.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/IOSandbox.h"
#અન્તર્ભાવ "llvm/Support/LockFileManager.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/Process.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
#અન્તર્ભાવ "llvm/Support/VirtualOutputConfig.h"
#અન્તર્ભાવ "llvm/Support/VirtualOutputError.h"

using નામાકાશ llvm;
using નામાકાશ llvm::vfs;

શૂન્ય ProxyOutputBackend::anchor() {}
શૂન્ય OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  સંરચના NullOutputBackend : જાહેર OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() અચલ override {
      પાછા const_cast<NullOutputBackend *>(આ);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      પાછા std::make_unique<NullOutputFileImpl>();
    }
  };

  પાછા makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<તાર્કિક(StringRef, std::optional<OutputConfig>)> Filter) {
  સંરચના FilteringOutputBackend : જાહેર ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      જો (Filter(Path, Config))
        પાછા ProxyOutputBackend::createFileImpl(Path, Config);
      પાછા std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() અચલ override {
      પાછા makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<તાર્કિક(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      પ્રતિજ્ઞા(આ->Filter && "Expected a non-null function");
    }
    std::function<તાર્કિક(StringRef, std::optional<OutputConfig>)> Filter;
  };

  પાછા makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  સંરચના ProxyOutputBackend1 : જાહેર ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  સંરચના ProxyOutputBackend2 : જાહેર ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  સંરચના MirroringOutput final : જાહેર OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      પાછા joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      પાછા joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { પાછા *આ; }

    શૂન્ય write_impl(અચલ અક્ષર *Ptr, size_t Size) override {
      F1->getOS().લિખ(Ptr, Size);
      F2->getOS().લિખ(Ptr, Size);
    }
    શૂન્ય pwrite_impl(અચલ અક્ષર *Ptr, size_t Size, uint64_t Offset) override {
      આ->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() અચલ override { પાછા F1->getOS().tell(); }
    size_t preferred_buffer_size() અચલ override {
      પાછા PreferredBufferSize;
    }
    શૂન્ય reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    તાર્કિક is_displayed() અચલ override {
      પાછા F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    તાર્કિક has_colors() અચલ override {
      પાછા F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    શૂન્ય enable_colors(તાર્કિક enable) override {
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
  સંરચના MirroringOutputBackend : જાહેર ProxyOutputBackend1,
                                  જાહેર ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      જો (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        પાછા std::move(E);
      જો (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        પાછા joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      જો (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        પાછા std::move(File2);
      }
      જો (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        પાછા std::move(File1);
      }
      પાછા std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() અચલ override {
      પાછા IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    શૂન્ય Retain() અચલ { ProxyOutputBackend1::Retain(); }
    શૂન્ય Release() અચલ { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  પ્રતિજ્ઞા(Backend1 && "Expected actual backend");
  પ્રતિજ્ઞા(Backend2 && "Expected actual backend");
  પાછા IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

સ્થિર OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              અચલ OnDiskOutputBackend::OutputSettings &Settings) {
  જો (!Config)
    Config = Settings.DefaultConfig;
  જો (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  જો (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  પાછા *Config;
}

નામાકાશ {
વર્ગ OnDiskOutputFile final : જાહેર OutputFileImpl {
જાહેર:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    પ્રતિજ્ઞા(FileOS && "Expected valid file");
    જો (BufferOS)
      પાછા *BufferOS;
    પાછા *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<પૂર્ણાંક> &FD);

  Error initializeFile(std::optional<પૂર્ણાંક> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   અચલ OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  અચલ std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

સ્થિર Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  પાછા handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    જો (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      પાછા Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    જો (std::error_code EC = sys::fs::create_directories(ParentPath))
      પાછા make_error<OutputError>(ParentPath, EC);
    પાછા CreateFile();
  });
}

સ્થિર sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  જો (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  નહીંતર જો (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  જો (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  પાછા OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<પૂર્ણાંક> &FD) {
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

  પાછા createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    પૂર્ણાંક NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    જો (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      પાછા make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    જો (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    પાછા Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<પૂર્ણાંક> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  પ્રતિજ્ઞા(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  જો (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    જો (sys::fs::exists(Status)) {
      જો (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      જો (Config.getAppend() && !sys::fs::can_write(OutputPath))
        પાછા make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  જો (Config.getAtomicWrite())
    જો (!errorToBool(tryToCreateTemporary(FD)))
      પાછા Error::success();

  // Not using a temporary file. Open the final output file.
  પાછા createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    પૂર્ણાંક NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    જો (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      પાછા convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    જો (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    પાછા Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  જો (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    જો (EC)
      પાછા make_error<OutputError>(OutputPath, EC);
  } નહીંતર {
    std::optional<પૂર્ણાંક> FD;
    જો (Error E = initializeFile(FD))
      પાછા E;
    FileOS.emplace(*FD, /*shouldClose=*/સાચું);
  }

  // Buffer the stream if necessary.
  જો (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  પાછા Error::success();
}

નામાકાશ {
વર્ગ OpenFileRAII {
  સ્થિર અચલ પૂર્ણાંક InvalidFd = -1;

જાહેર:
  પૂર્ણાંક Fd = InvalidFd;

  ~OpenFileRAII() {
    જો (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

ગણના વર્ગ FileDifference : uint8_t {
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

સ્થિર Expected<FileDifference>
areFilesDifferent(અચલ llvm::Twine &Source, અચલ llvm::Twine &Destination) {
  જો (sys::fs::equivalent(Source, Destination))
    પાછા FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  જો (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    પાછા convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  જો (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    પાછા convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  જો (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    પાછા FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  જો (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    પાછા FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  જો (Size != DestStatus.getSize())
    પાછા FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  જો (Size == 0)
    પાછા FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  જો (SourceRegionErr)
    પાછા convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  જો (DestRegionErr)
    પાછા FileDifference::DifferentContents;

  જો (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    પાછા FileDifference::DifferentContents;

  પાછા FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  જો (!FileOS)
    પાછા Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  પાછા errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  જો (auto E = reset())
    પાછા E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    જો (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  જો (!TempPath)
    પાછા Error::success();

  // See if we should append instead of move.
  જો (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    જો (!Content)
      પાછા convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    જ્યારે (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      તાર્કિક Owned;
      જો (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        પાછા convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      જો (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        જો (EC)
          પાછા convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        જો (Out.has_error())
          પાછા convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (શૂન્ય)sys::fs::remove(*TempPath);
        પાછા Error::success();
      }
      // Someone else owns the lock on this file, wait.
      પસંદગી (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      સ્થિતિ WaitForUnlockResult::Success:
        [[fallthrough]];
      સ્થિતિ WaitForUnlockResult::OwnerDied: {
        ચાલુ; // try again to get the lock.
      }
      સ્થિતિ WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        ચાલુ;
      }
      }
      તોડો;
    }
  }

  જો (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    જો (!Result)
      પાછા Result.takeError();
    પસંદગી (*Result) {
    સ્થિતિ FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      પાછા Error::success();

    સ્થિતિ FileDifference::SameContents:
      // Files are identical; remove the source file.
      (શૂન્ય)sys::fs::remove(*TempPath);
      પાછા Error::success();

    સ્થિતિ FileDifference::DifferentContents:
      તોડો; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  જો (!RenameEC)
    પાછા Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (શૂન્ય)sys::fs::remove(*TempPath);

  જો (!RenameEC)
    પાછા Error::success();

  પાછા make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  જો (auto E = reset())
    પાછા E;

  // Nothing on the filesystem to remove for stdout.
  જો (OutputPath == "-")
    પાછા Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    પાછા EC;
  };

  // Clean up the file that's in-progress.
  જો (!TempPath)
    પાછા convertToOutputError(OutputPath, discardPath(OutputPath));
  પાછા convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<અક્ષર> &Path) અચલ {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  પાછા convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  જો (Path != "-") {
    AbsPath = Path;
    જો (Error E = makeAbsolute(AbsPath))
      પાછા std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  જો (Error E = File->initializeStream())
    પાછા std::move(E);

  પાછા std::move(File);
}
