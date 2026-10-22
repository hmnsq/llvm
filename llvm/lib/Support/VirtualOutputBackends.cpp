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

#অন্তর্ভাৱ "llvm/Support/VirtualOutputBackends.h"
#অন্তর্ভাৱ "llvm/ADT/ScopeExit.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"
#অন্তর্ভাৱ "llvm/Support/IOSandbox.h"
#অন্তর্ভাৱ "llvm/Support/LockFileManager.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাৱ "llvm/Support/Path.h"
#অন্তর্ভাৱ "llvm/Support/Process.h"
#অন্তর্ভাৱ "llvm/Support/Signals.h"
#অন্তর্ভাৱ "llvm/Support/VirtualOutputConfig.h"
#অন্তর্ভাৱ "llvm/Support/VirtualOutputError.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

শূন্য ProxyOutputBackend::anchor() {}
শূন্য OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  সংরচনা NullOutputBackend : সার্ৱজনিক OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ধ্রুৱ override {
      ঘূৰাই const_cast<NullOutputBackend *>(এতত্);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      ঘূৰাই std::make_unique<NullOutputFileImpl>();
    }
  };

  ঘূৰাই makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<তার্কিক(StringRef, std::optional<OutputConfig>)> Filter) {
  সংরচনা FilteringOutputBackend : সার্ৱজনিক ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      যদি (Filter(Path, Config))
        ঘূৰাই ProxyOutputBackend::createFileImpl(Path, Config);
      ঘূৰাই std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ধ্রুৱ override {
      ঘূৰাই makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<তার্কিক(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      প্রতিজ্ঞা(এতত্->Filter && "Expected a non-null function");
    }
    std::function<তার্কিক(StringRef, std::optional<OutputConfig>)> Filter;
  };

  ঘূৰাই makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  সংরচনা ProxyOutputBackend1 : সার্ৱজনিক ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  সংরচনা ProxyOutputBackend2 : সার্ৱজনিক ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  সংরচনা MirroringOutput final : সার্ৱজনিক OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      ঘূৰাই joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ঘূৰাই joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { ঘূৰাই *এতত্; }

    শূন্য write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) override {
      F1->getOS().লিখ(Ptr, Size);
      F2->getOS().লিখ(Ptr, Size);
    }
    শূন্য pwrite_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size, uint64_t Offset) override {
      এতত্->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() ধ্রুৱ override { ঘূৰাই F1->getOS().tell(); }
    size_t preferred_buffer_size() ধ্রুৱ override {
      ঘূৰাই PreferredBufferSize;
    }
    শূন্য reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    তার্কিক is_displayed() ধ্রুৱ override {
      ঘূৰাই F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    তার্কিক has_colors() ধ্রুৱ override {
      ঘূৰাই F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    শূন্য enable_colors(তার্কিক enable) override {
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
  সংরচনা MirroringOutputBackend : সার্ৱজনিক ProxyOutputBackend1,
                                  সার্ৱজনিক ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      যদি (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        ঘূৰাই std::move(E);
      যদি (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        ঘূৰাই joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      যদি (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ঘূৰাই std::move(File2);
      }
      যদি (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ঘূৰাই std::move(File1);
      }
      ঘূৰাই std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ধ্রুৱ override {
      ঘূৰাই IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    শূন্য Retain() ধ্রুৱ { ProxyOutputBackend1::Retain(); }
    শূন্য Release() ধ্রুৱ { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  প্রতিজ্ঞা(Backend1 && "Expected actual backend");
  প্রতিজ্ঞা(Backend2 && "Expected actual backend");
  ঘূৰাই IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

স্থির OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ধ্রুৱ OnDiskOutputBackend::OutputSettings &Settings) {
  যদি (!Config)
    Config = Settings.DefaultConfig;
  যদি (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  যদি (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  ঘূৰাই *Config;
}

নামাকাশ {
শ্ৰেণী OnDiskOutputFile final : সার্ৱজনিক OutputFileImpl {
সার্ৱজনিক:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    প্রতিজ্ঞা(FileOS && "Expected valid file");
    যদি (BufferOS)
      ঘূৰাই *BufferOS;
    ঘূৰাই *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<পূর্ণাঙ্ক> &FD);

  Error initializeFile(std::optional<পূর্ণাঙ্ক> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   ধ্রুৱ OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  ধ্রুৱ std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

স্থির Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  ঘূৰাই handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    যদি (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ঘূৰাই Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    যদি (std::error_code EC = sys::fs::create_directories(ParentPath))
      ঘূৰাই make_error<OutputError>(ParentPath, EC);
    ঘূৰাই CreateFile();
  });
}

স্থির sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  যদি (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  অন্যথা যদি (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  যদি (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  ঘূৰাই OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<পূর্ণাঙ্ক> &FD) {
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

  ঘূৰাই createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    পূর্ণাঙ্ক NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    যদি (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      ঘূৰাই make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    যদি (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    ঘূৰাই Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<পূর্ণাঙ্ক> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  প্রতিজ্ঞা(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  যদি (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    যদি (sys::fs::exists(Status)) {
      যদি (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      যদি (Config.getAppend() && !sys::fs::can_write(OutputPath))
        ঘূৰাই make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  যদি (Config.getAtomicWrite())
    যদি (!errorToBool(tryToCreateTemporary(FD)))
      ঘূৰাই Error::success();

  // Not using a temporary file. Open the final output file.
  ঘূৰাই createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    পূর্ণাঙ্ক NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    যদি (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      ঘূৰাই convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    যদি (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    ঘূৰাই Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  যদি (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    যদি (EC)
      ঘূৰাই make_error<OutputError>(OutputPath, EC);
  } অন্যথা {
    std::optional<পূর্ণাঙ্ক> FD;
    যদি (Error E = initializeFile(FD))
      ঘূৰাই E;
    FileOS.emplace(*FD, /*shouldClose=*/সঁচা);
  }

  // Buffer the stream if necessary.
  যদি (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  ঘূৰাই Error::success();
}

নামাকাশ {
শ্ৰেণী OpenFileRAII {
  স্থির ধ্রুৱ পূর্ণাঙ্ক InvalidFd = -1;

সার্ৱজনিক:
  পূর্ণাঙ্ক Fd = InvalidFd;

  ~OpenFileRAII() {
    যদি (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

গণনা শ্ৰেণী FileDifference : uint8_t {
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

স্থির Expected<FileDifference>
areFilesDifferent(ধ্রুৱ llvm::Twine &Source, ধ্রুৱ llvm::Twine &Destination) {
  যদি (sys::fs::equivalent(Source, Destination))
    ঘূৰাই FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  যদি (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    ঘূৰাই convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  যদি (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    ঘূৰাই convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  যদি (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ঘূৰাই FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  যদি (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    ঘূৰাই FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  যদি (Size != DestStatus.getSize())
    ঘূৰাই FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  যদি (Size == 0)
    ঘূৰাই FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  যদি (SourceRegionErr)
    ঘূৰাই convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  যদি (DestRegionErr)
    ঘূৰাই FileDifference::DifferentContents;

  যদি (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    ঘূৰাই FileDifference::DifferentContents;

  ঘূৰাই FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  যদি (!FileOS)
    ঘূৰাই Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ঘূৰাই errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (auto E = reset())
    ঘূৰাই E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    যদি (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  যদি (!TempPath)
    ঘূৰাই Error::success();

  // See if we should append instead of move.
  যদি (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    যদি (!Content)
      ঘূৰাই convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    যেতিয়ালৈ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      তার্কিক Owned;
      যদি (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        ঘূৰাই convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      যদি (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        যদি (EC)
          ঘূৰাই convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        যদি (Out.has_error())
          ঘূৰাই convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (শূন্য)sys::fs::remove(*TempPath);
        ঘূৰাই Error::success();
      }
      // Someone else owns the lock on this file, wait.
      ৱিকল্প (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      পক্ষ WaitForUnlockResult::Success:
        [[fallthrough]];
      পক্ষ WaitForUnlockResult::OwnerDied: {
        অনুৱর্ত; // try again to get the lock.
      }
      পক্ষ WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        অনুৱর্ত;
      }
      }
      ভঙ্গ;
    }
  }

  যদি (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    যদি (!Result)
      ঘূৰাই Result.takeError();
    ৱিকল্প (*Result) {
    পক্ষ FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ঘূৰাই Error::success();

    পক্ষ FileDifference::SameContents:
      // Files are identical; remove the source file.
      (শূন্য)sys::fs::remove(*TempPath);
      ঘূৰাই Error::success();

    পক্ষ FileDifference::DifferentContents:
      ভঙ্গ; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  যদি (!RenameEC)
    ঘূৰাই Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (শূন্য)sys::fs::remove(*TempPath);

  যদি (!RenameEC)
    ঘূৰাই Error::success();

  ঘূৰাই make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  যদি (auto E = reset())
    ঘূৰাই E;

  // Nothing on the filesystem to remove for stdout.
  যদি (OutputPath == "-")
    ঘূৰাই Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    ঘূৰাই EC;
  };

  // Clean up the file that's in-progress.
  যদি (!TempPath)
    ঘূৰাই convertToOutputError(OutputPath, discardPath(OutputPath));
  ঘূৰাই convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<ৱর্ণ> &Path) ধ্রুৱ {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  ঘূৰাই convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  যদি (Path != "-") {
    AbsPath = Path;
    যদি (Error E = makeAbsolute(AbsPath))
      ঘূৰাই std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  যদি (Error E = File->initializeStream())
    ঘূৰাই std::move(E);

  ঘূৰাই std::move(File);
}
