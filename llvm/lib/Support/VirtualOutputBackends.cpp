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

#अन्तर्भाव "llvm/Support/VirtualOutputBackends.h"
#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/LockFileManager.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/VirtualOutputConfig.h"
#अन्तर्भाव "llvm/Support/VirtualOutputError.h"

using नामाकाश llvm;
using नामाकाश llvm::vfs;

शून्य ProxyOutputBackend::anchor() {}
शून्य OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  संरचना NullOutputBackend : सार्वजनिक OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ध्रुव override {
      पटिनिवत्त const_cast<NullOutputBackend *>(एतत्);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      पटिनिवत्त std::make_unique<NullOutputFileImpl>();
    }
  };

  पटिनिवत्त makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<तार्किक(StringRef, std::optional<OutputConfig>)> Filter) {
  संरचना FilteringOutputBackend : सार्वजनिक ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      सचे (Filter(Path, Config))
        पटिनिवत्त ProxyOutputBackend::createFileImpl(Path, Config);
      पटिनिवत्त std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ध्रुव override {
      पटिनिवत्त makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<तार्किक(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      प्रतिज्ञा(एतत्->Filter && "Expected a non-null function");
    }
    std::function<तार्किक(StringRef, std::optional<OutputConfig>)> Filter;
  };

  पटिनिवत्त makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  संरचना ProxyOutputBackend1 : सार्वजनिक ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  संरचना ProxyOutputBackend2 : सार्वजनिक ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  संरचना MirroringOutput final : सार्वजनिक OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      पटिनिवत्त joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      पटिनिवत्त joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { पटिनिवत्त *एतत्; }

    शून्य write_impl(ध्रुव वर्ण *Ptr, size_t Size) override {
      F1->getOS().लिख(Ptr, Size);
      F2->getOS().लिख(Ptr, Size);
    }
    शून्य pwrite_impl(ध्रुव वर्ण *Ptr, size_t Size, uint64_t Offset) override {
      एतत्->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() ध्रुव override { पटिनिवत्त F1->getOS().tell(); }
    size_t preferred_buffer_size() ध्रुव override {
      पटिनिवत्त PreferredBufferSize;
    }
    शून्य reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    तार्किक is_displayed() ध्रुव override {
      पटिनिवत्त F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    तार्किक has_colors() ध्रुव override {
      पटिनिवत्त F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    शून्य enable_colors(तार्किक enable) override {
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
  संरचना MirroringOutputBackend : सार्वजनिक ProxyOutputBackend1,
                                  सार्वजनिक ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      सचे (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        पटिनिवत्त std::move(E);
      सचे (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        पटिनिवत्त joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      सचे (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        पटिनिवत्त std::move(File2);
      }
      सचे (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        पटिनिवत्त std::move(File1);
      }
      पटिनिवत्त std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ध्रुव override {
      पटिनिवत्त IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    शून्य Retain() ध्रुव { ProxyOutputBackend1::Retain(); }
    शून्य Release() ध्रुव { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  प्रतिज्ञा(Backend1 && "Expected actual backend");
  प्रतिज्ञा(Backend2 && "Expected actual backend");
  पटिनिवत्त IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

स्थिर OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ध्रुव OnDiskOutputBackend::OutputSettings &Settings) {
  सचे (!Config)
    Config = Settings.DefaultConfig;
  सचे (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  सचे (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  पटिनिवत्त *Config;
}

नामाकाश {
वग्ग OnDiskOutputFile final : सार्वजनिक OutputFileImpl {
सार्वजनिक:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    प्रतिज्ञा(FileOS && "Expected valid file");
    सचे (BufferOS)
      पटिनिवत्त *BufferOS;
    पटिनिवत्त *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<पूर्णाङ्क> &FD);

  Error initializeFile(std::optional<पूर्णाङ्क> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   ध्रुव OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  ध्रुव std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

स्थिर Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  पटिनिवत्त handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    सचे (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      पटिनिवत्त Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    सचे (std::error_code EC = sys::fs::create_directories(ParentPath))
      पटिनिवत्त make_error<OutputError>(ParentPath, EC);
    पटिनिवत्त CreateFile();
  });
}

स्थिर sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  सचे (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  अञ्ञथा सचे (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  सचे (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  पटिनिवत्त OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<पूर्णाङ्क> &FD) {
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

  पटिनिवत्त createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    पूर्णाङ्क NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    सचे (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      पटिनिवत्त make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    सचे (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    पटिनिवत्त Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<पूर्णाङ्क> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  सचे (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    सचे (sys::fs::exists(Status)) {
      सचे (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      सचे (Config.getAppend() && !sys::fs::can_write(OutputPath))
        पटिनिवत्त make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  सचे (Config.getAtomicWrite())
    सचे (!errorToBool(tryToCreateTemporary(FD)))
      पटिनिवत्त Error::success();

  // Not using a temporary file. Open the final output file.
  पटिनिवत्त createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    पूर्णाङ्क NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    सचे (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      पटिनिवत्त convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    सचे (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    पटिनिवत्त Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  सचे (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    सचे (EC)
      पटिनिवत्त make_error<OutputError>(OutputPath, EC);
  } अञ्ञथा {
    std::optional<पूर्णाङ्क> FD;
    सचे (Error E = initializeFile(FD))
      पटिनिवत्त E;
    FileOS.emplace(*FD, /*shouldClose=*/सच्च);
  }

  // Buffer the stream if necessary.
  सचे (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  पटिनिवत्त Error::success();
}

नामाकाश {
वग्ग OpenFileRAII {
  स्थिर ध्रुव पूर्णाङ्क InvalidFd = -1;

सार्वजनिक:
  पूर्णाङ्क Fd = InvalidFd;

  ~OpenFileRAII() {
    सचे (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

गणना वग्ग FileDifference : uint8_t {
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

स्थिर Expected<FileDifference>
areFilesDifferent(ध्रुव llvm::Twine &Source, ध्रुव llvm::Twine &Destination) {
  सचे (sys::fs::equivalent(Source, Destination))
    पटिनिवत्त FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  सचे (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    पटिनिवत्त convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  सचे (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    पटिनिवत्त convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  सचे (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    पटिनिवत्त FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  सचे (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    पटिनिवत्त FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  सचे (Size != DestStatus.getSize())
    पटिनिवत्त FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  सचे (Size == 0)
    पटिनिवत्त FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  सचे (SourceRegionErr)
    पटिनिवत्त convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  सचे (DestRegionErr)
    पटिनिवत्त FileDifference::DifferentContents;

  सचे (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    पटिनिवत्त FileDifference::DifferentContents;

  पटिनिवत्त FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  सचे (!FileOS)
    पटिनिवत्त Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  पटिनिवत्त errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  सचे (auto E = reset())
    पटिनिवत्त E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    सचे (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  सचे (!TempPath)
    पटिनिवत्त Error::success();

  // See if we should append instead of move.
  सचे (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    सचे (!Content)
      पटिनिवत्त convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    याव (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      तार्किक Owned;
      सचे (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        पटिनिवत्त convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      सचे (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        सचे (EC)
          पटिनिवत्त convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        सचे (Out.has_error())
          पटिनिवत्त convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (शून्य)sys::fs::remove(*TempPath);
        पटिनिवत्त Error::success();
      }
      // Someone else owns the lock on this file, wait.
      विकल्प (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      पक्ष WaitForUnlockResult::Success:
        [[fallthrough]];
      पक्ष WaitForUnlockResult::OwnerDied: {
        अनुवर्त; // try again to get the lock.
      }
      पक्ष WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        अनुवर्त;
      }
      }
      भङ्ग;
    }
  }

  सचे (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    सचे (!Result)
      पटिनिवत्त Result.takeError();
    विकल्प (*Result) {
    पक्ष FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      पटिनिवत्त Error::success();

    पक्ष FileDifference::SameContents:
      // Files are identical; remove the source file.
      (शून्य)sys::fs::remove(*TempPath);
      पटिनिवत्त Error::success();

    पक्ष FileDifference::DifferentContents:
      भङ्ग; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  सचे (!RenameEC)
    पटिनिवत्त Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (शून्य)sys::fs::remove(*TempPath);

  सचे (!RenameEC)
    पटिनिवत्त Error::success();

  पटिनिवत्त make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  सचे (auto E = reset())
    पटिनिवत्त E;

  // Nothing on the filesystem to remove for stdout.
  सचे (OutputPath == "-")
    पटिनिवत्त Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    पटिनिवत्त EC;
  };

  // Clean up the file that's in-progress.
  सचे (!TempPath)
    पटिनिवत्त convertToOutputError(OutputPath, discardPath(OutputPath));
  पटिनिवत्त convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<वर्ण> &Path) ध्रुव {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  पटिनिवत्त convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  सचे (Path != "-") {
    AbsPath = Path;
    सचे (Error E = makeAbsolute(AbsPath))
      पटिनिवत्त std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  सचे (Error E = File->initializeStream())
    पटिनिवत्त std::move(E);

  पटिनिवत्त std::move(File);
}
