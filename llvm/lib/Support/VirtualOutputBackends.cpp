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
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() अचर override {
      फर्काऊ const_cast<NullOutputBackend *>(एतत्);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      फर्काऊ std::make_unique<NullOutputFileImpl>();
    }
  };

  फर्काऊ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<तार्किक(StringRef, std::optional<OutputConfig>)> Filter) {
  संरचना FilteringOutputBackend : सार्वजनिक ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      यदि (Filter(Path, Config))
        फर्काऊ ProxyOutputBackend::createFileImpl(Path, Config);
      फर्काऊ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() अचर override {
      फर्काऊ makeIntrusiveRefCnt<FilteringOutputBackend>(
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

  फर्काऊ makeIntrusiveRefCnt<FilteringOutputBackend>(
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
      फर्काऊ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      फर्काऊ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { फर्काऊ *एतत्; }

    शून्य write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) override {
      F1->getOS().लिख(Ptr, Size);
      F2->getOS().लिख(Ptr, Size);
    }
    शून्य pwrite_impl(अचर वर्ण *Ptr, माप_प्रकार Size, uint64_t Offset) override {
      एतत्->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() अचर override { फर्काऊ F1->getOS().tell(); }
    माप_प्रकार preferred_buffer_size() अचर override {
      फर्काऊ PreferredBufferSize;
    }
    शून्य reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    तार्किक is_displayed() अचर override {
      फर्काऊ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    तार्किक has_colors() अचर override {
      फर्काऊ F1->getOS().has_colors() && F2->getOS().has_colors();
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

    माप_प्रकार PreferredBufferSize;
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
      यदि (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        फर्काऊ std::move(E);
      यदि (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        फर्काऊ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      यदि (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        फर्काऊ std::move(File2);
      }
      यदि (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        फर्काऊ std::move(File1);
      }
      फर्काऊ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() अचर override {
      फर्काऊ IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    शून्य Retain() अचर { ProxyOutputBackend1::Retain(); }
    शून्य Release() अचर { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  प्रतिज्ञा(Backend1 && "Expected actual backend");
  प्रतिज्ञा(Backend2 && "Expected actual backend");
  फर्काऊ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

स्थिर OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              अचर OnDiskOutputBackend::OutputSettings &Settings) {
  यदि (!Config)
    Config = Settings.DefaultConfig;
  यदि (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  यदि (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  फर्काऊ *Config;
}

नामाकाश {
वर्ग OnDiskOutputFile final : सार्वजनिक OutputFileImpl {
सार्वजनिक:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    प्रतिज्ञा(FileOS && "Expected valid file");
    यदि (BufferOS)
      फर्काऊ *BufferOS;
    फर्काऊ *FileOS;
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
                   अचर OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  अचर std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

स्थिर Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  फर्काऊ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    यदि (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      फर्काऊ Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    यदि (std::error_code EC = sys::fs::create_directories(ParentPath))
      फर्काऊ make_error<OutputError>(ParentPath, EC);
    फर्काऊ CreateFile();
  });
}

स्थिर sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  यदि (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  अन्यथा यदि (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  यदि (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  फर्काऊ OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<पूर्णाङ्क> &FD) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

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

  फर्काऊ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    पूर्णाङ्क NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    यदि (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      फर्काऊ make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    यदि (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    फर्काऊ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<पूर्णाङ्क> &FD) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  यदि (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    यदि (sys::fs::exists(Status)) {
      यदि (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      यदि (Config.getAppend() && !sys::fs::can_write(OutputPath))
        फर्काऊ make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  यदि (Config.getAtomicWrite())
    यदि (!errorToBool(tryToCreateTemporary(FD)))
      फर्काऊ Error::success();

  // Not using a temporary file. Open the final output file.
  फर्काऊ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    पूर्णाङ्क NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    यदि (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      फर्काऊ convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    यदि (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    फर्काऊ Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  यदि (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    यदि (EC)
      फर्काऊ make_error<OutputError>(OutputPath, EC);
  } अन्यथा {
    std::optional<पूर्णाङ्क> FD;
    यदि (Error E = initializeFile(FD))
      फर्काऊ E;
    FileOS.emplace(*FD, /*shouldClose=*/सत्य);
  }

  // Buffer the stream if necessary.
  यदि (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  फर्काऊ Error::success();
}

नामाकाश {
वर्ग OpenFileRAII {
  स्थिर अचर पूर्णाङ्क InvalidFd = -1;

सार्वजनिक:
  पूर्णाङ्क Fd = InvalidFd;

  ~OpenFileRAII() {
    यदि (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

गणना वर्ग FileDifference : uint8_t {
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
areFilesDifferent(अचर llvm::Twine &Source, अचर llvm::Twine &Destination) {
  यदि (sys::fs::equivalent(Source, Destination))
    फर्काऊ FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  यदि (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    फर्काऊ convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  यदि (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    फर्काऊ convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  यदि (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    फर्काऊ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  यदि (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    फर्काऊ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  यदि (Size != DestStatus.getSize())
    फर्काऊ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  यदि (Size == 0)
    फर्काऊ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  यदि (SourceRegionErr)
    फर्काऊ convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  यदि (DestRegionErr)
    फर्काऊ FileDifference::DifferentContents;

  यदि (स_भेद(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    फर्काऊ FileDifference::DifferentContents;

  फर्काऊ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  यदि (!FileOS)
    फर्काऊ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  फर्काऊ errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  यदि (स्वतः E = reset())
    फर्काऊ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    यदि (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  यदि (!TempPath)
    फर्काऊ Error::success();

  // See if we should append instead of move.
  यदि (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    स्वतः Content = MemoryBuffer::getFile(*TempPath);
    यदि (!Content)
      फर्काऊ convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    जबसम्म (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      तार्किक Owned;
      यदि (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        फर्काऊ convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      यदि (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        यदि (EC)
          फर्काऊ convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.बंद();
        Lock.unsafeUnlock();
        यदि (Out.has_error())
          फर्काऊ convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (शून्य)sys::fs::हटाओ(*TempPath);
        फर्काऊ Error::success();
      }
      // Someone else owns the lock on this file, wait.
      विकल्प (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      पक्ष WaitForUnlockResult::Success:
        [[fallthrough]];
      पक्ष WaitForUnlockResult::OwnerDied: {
        जारी; // try again to get the lock.
      }
      पक्ष WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        जारी;
      }
      }
      रोक;
    }
  }

  यदि (Config.getOnlyIfDifferent()) {
    स्वतः Result = areFilesDifferent(*TempPath, OutputPath);
    यदि (!Result)
      फर्काऊ Result.takeError();
    विकल्प (*Result) {
    पक्ष FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      फर्काऊ Error::success();

    पक्ष FileDifference::SameContents:
      // Files are identical; remove the source file.
      (शून्य)sys::fs::हटाओ(*TempPath);
      फर्काऊ Error::success();

    पक्ष FileDifference::DifferentContents:
      रोक; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::नाम(*TempPath, OutputPath);
  यदि (!RenameEC)
    फर्काऊ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (शून्य)sys::fs::हटाओ(*TempPath);

  यदि (!RenameEC)
    फर्काऊ Error::success();

  फर्काऊ make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  यदि (स्वतः E = reset())
    फर्काऊ E;

  // Nothing on the filesystem to remove for stdout.
  यदि (OutputPath == "-")
    फर्काऊ Error::success();

  स्वतः discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::हटाओ(Path);
    sys::DontRemoveFileOnSignal(Path);
    फर्काऊ EC;
  };

  // Clean up the file that's in-progress.
  यदि (!TempPath)
    फर्काऊ convertToOutputError(OutputPath, discardPath(OutputPath));
  फर्काऊ convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<वर्ण> &Path) अचर {
  // FIXME: Should this really call sys::fs::make_absolute?
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();
  फर्काऊ convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  यदि (Path != "-") {
    AbsPath = Path;
    यदि (Error E = makeAbsolute(AbsPath))
      फर्काऊ std::move(E);
    Path = AbsPath;
  }

  स्वतः File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  यदि (Error E = File->initializeStream())
    फर्काऊ std::move(E);

  फर्काऊ std::move(File);
}
