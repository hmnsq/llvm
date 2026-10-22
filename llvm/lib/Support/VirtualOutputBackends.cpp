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

#অন্তর্ভাব "llvm/Support/VirtualOutputBackends.h"
#অন্তর্ভাব "llvm/ADT/ScopeExit.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/IOSandbox.h"
#অন্তর্ভাব "llvm/Support/LockFileManager.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/Path.h"
#অন্তর্ভাব "llvm/Support/Process.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
#অন্তর্ভাব "llvm/Support/VirtualOutputConfig.h"
#অন্তর্ভাব "llvm/Support/VirtualOutputError.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

শূন্য ProxyOutputBackend::anchor() {}
শূন্য OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  কাঠামো NullOutputBackend : সর্বজনীন OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ধ্রুবক override {
      ফেরত const_cast<NullOutputBackend *>(এই);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      ফেরত std::make_unique<NullOutputFileImpl>();
    }
  };

  ফেরত makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<যৌক্তিক(StringRef, std::optional<OutputConfig>)> Filter) {
  কাঠামো FilteringOutputBackend : সর্বজনীন ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      যদি (Filter(Path, Config))
        ফেরত ProxyOutputBackend::createFileImpl(Path, Config);
      ফেরত std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ধ্রুবক override {
      ফেরত makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<যৌক্তিক(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      প্রতিজ্ঞা(এই->Filter && "Expected a non-null function");
    }
    std::function<যৌক্তিক(StringRef, std::optional<OutputConfig>)> Filter;
  };

  ফেরত makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  কাঠামো ProxyOutputBackend1 : সর্বজনীন ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  কাঠামো ProxyOutputBackend2 : সর্বজনীন ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  কাঠামো MirroringOutput final : সর্বজনীন OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      ফেরত joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ফেরত joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { ফেরত *এই; }

    শূন্য write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) override {
      F1->getOS().লিখ(Ptr, আকার_2);
      F2->getOS().লিখ(Ptr, আকার_2);
    }
    শূন্য pwrite_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2, uint64_t Offset) override {
      এই->flush();
      F1->getOS().pwrite(Ptr, আকার_2, Offset);
      F2->getOS().pwrite(Ptr, আকার_2, Offset);
    }
    uint64_t current_pos() ধ্রুবক override { ফেরত F1->getOS().tell(); }
    size_t preferred_বাফার_আকার() ধ্রুবক override {
      ফেরত Preferred_বাফার_আকার;
    }
    শূন্য reserveExtraSpace(uint64_t Extra_আকার) override {
      F1->getOS().reserveExtraSpace(Extra_আকার);
      F2->getOS().reserveExtraSpace(Extra_আকার);
    }
    যৌক্তিক is_displayed() ধ্রুবক override {
      ফেরত F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    যৌক্তিক has_colors() ধ্রুবক override {
      ফেরত F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    শূন্য enable_colors(যৌক্তিক enable) override {
      raw_pwrite_stream::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_বাফার_আকার(std::max(F1->getOS().Get_বাফার_আকার(),
                                       F1->getOS().Get_বাফার_আকার())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_বাফার_আকার;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  কাঠামো MirroringOutputBackend : সর্বজনীন ProxyOutputBackend1,
                                  সর্বজনীন ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      যদি (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        ফেরত std::move(E);
      যদি (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        ফেরত joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      যদি (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ফেরত std::move(File2);
      }
      যদি (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ফেরত std::move(File1);
      }
      ফেরত std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ধ্রুবক override {
      ফেরত IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    শূন্য Retain() ধ্রুবক { ProxyOutputBackend1::Retain(); }
    শূন্য Release() ধ্রুবক { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  প্রতিজ্ঞা(Backend1 && "Expected actual backend");
  প্রতিজ্ঞা(Backend2 && "Expected actual backend");
  ফেরত IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

স্থির OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ধ্রুবক OnDiskOutputBackend::OutputSettings &Settings) {
  যদি (!Config)
    Config = Settings.DefaultConfig;
  যদি (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  যদি (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  ফেরত *Config;
}

নামাকাশ {
শ্রেণী OnDiskOutputFile final : সর্বজনীন OutputFileImpl {
সর্বজনীন:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    প্রতিজ্ঞা(FileOS && "Expected valid file");
    যদি (বাফার_OS)
      ফেরত *বাফার_OS;
    ফেরত *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<পূর্ণসংখ্যা> &FD);

  Error initializeFile(std::optional<পূর্ণসংখ্যা> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   ধ্রুবক OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  ধ্রুবক std::string OutputPath;
  std::optional<std::string> অস্থায়ী_Path;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<বাফার_ostream> বাফার_OS;
};
} // end namespace

স্থির Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  ফেরত handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    যদি (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ফেরত Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    যদি (std::error_code EC = sys::fs::create_directories(ParentPath))
      ফেরত make_error<OutputError>(ParentPath, EC);
    ফেরত CreateFile();
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

  ফেরত OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<পূর্ণসংখ্যা> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::path::extension(OutputPath);
  SmallString<128> ModelPath =
      StringRef(OutputPath).drop_back(OutputExtension.আকার_3());
  ModelPath += "-%%%%%%%%";
  ModelPath += OutputExtension;
  ModelPath += ".tmp";

  ফেরত createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    পূর্ণসংখ্যা NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    যদি (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      ফেরত make_error<অস্থায়ী_File_Output_Error>(ModelPath, OutputPath, EC);

    যদি (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    অস্থায়ী_Path = UniquePath.str().str();
    FD.emplace(NewFD);
    ফেরত Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<পূর্ণসংখ্যা> &FD) {
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
        ফেরত make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  যদি (Config.getAtomicWrite())
    যদি (!errorToBool(tryToCreateTemporary(FD)))
      ফেরত Error::success();

  // Not using a temporary file. Open the final output file.
  ফেরত createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    পূর্ণসংখ্যা NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    যদি (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      ফেরত convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    যদি (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    ফেরত Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  যদি (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    যদি (EC)
      ফেরত make_error<OutputError>(OutputPath, EC);
  } অন্যথা {
    std::optional<পূর্ণসংখ্যা> FD;
    যদি (Error E = initializeFile(FD))
      ফেরত E;
    FileOS.emplace(*FD, /*shouldClose=*/সত্য);
  }

  // Buffer the stream if necessary.
  যদি (!FileOS->supportsSeeking() && !Config.getText())
    বাফার_OS.emplace(*FileOS);

  ফেরত Error::success();
}

নামাকাশ {
শ্রেণী OpenFileRAII {
  স্থির ধ্রুবক পূর্ণসংখ্যা InvalidFd = -1;

সর্বজনীন:
  পূর্ণসংখ্যা Fd = InvalidFd;

  ~OpenFileRAII() {
    যদি (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

গণনা শ্রেণী FileDifference : uint8_t {
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
areFilesDifferent(ধ্রুবক llvm::Twine &Source, ধ্রুবক llvm::Twine &Destination) {
  যদি (sys::fs::equivalent(Source, Destination))
    ফেরত FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  যদি (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    ফেরত convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  যদি (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    ফেরত convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  যদি (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ফেরত FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  যদি (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    ফেরত FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t আকার_2 = SourceStatus.get_আকার();
  যদি (আকার_2 != DestStatus.get_আকার())
    ফেরত FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  যদি (আকার_2 == 0)
    ফেরত FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, আকার_2, 0, SourceRegionErr);
  যদি (SourceRegionErr)
    ফেরত convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, আকার_2, 0, DestRegionErr);

  যদি (DestRegionErr)
    ফেরত FileDifference::DifferentContents;

  যদি (memcmp(SourceRegion.const_তথ্য(), DestRegion.const_তথ্য(), আকার_2) != 0)
    ফেরত FileDifference::DifferentContents;

  ফেরত FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  বাফার_OS.reset();
  যদি (!FileOS)
    ফেরত Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ফেরত errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (auto E = reset())
    ফেরত E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    যদি (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(অস্থায়ী_Path ? *অস্থায়ী_Path : OutputPath);
  });

  যদি (!অস্থায়ী_Path)
    ফেরত Error::success();

  // See if we should append instead of move.
  যদি (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = Memory_বাফার::getFile(*অস্থায়ী_Path);
    যদি (!Content)
      ফেরত convert_To_অস্থায়ী_File_Output_Error(*অস্থায়ী_Path, OutputPath,
                                          Content.getError());
    যখন (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      যৌক্তিক Owned;
      যদি (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        ফেরত convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      যদি (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        যদি (EC)
          ফেরত convertToOutputError(OutputPath, EC);
        Out << (*Content)->get_বাফার();
        Out.close();
        Lock.unsafeUnlock();
        যদি (Out.has_error())
          ফেরত convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (শূন্য)sys::fs::remove(*অস্থায়ী_Path);
        ফেরত Error::success();
      }
      // Someone else owns the lock on this file, wait.
      নির্বাচন (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      ক্ষেত্র Wait_For_Unlock_ফলাফল::Success:
        [[fallthrough]];
      ক্ষেত্র Wait_For_Unlock_ফলাফল::OwnerDied: {
        চালু; // try again to get the lock.
      }
      ক্ষেত্র Wait_For_Unlock_ফলাফল::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        চালু;
      }
      }
      বিরতি;
    }
  }

  যদি (Config.getOnlyIfDifferent()) {
    auto ফলাফল = areFilesDifferent(*অস্থায়ী_Path, OutputPath);
    যদি (!ফলাফল)
      ফেরত ফলাফল.takeError();
    নির্বাচন (*ফলাফল) {
    ক্ষেত্র FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ফেরত Error::success();

    ক্ষেত্র FileDifference::SameContents:
      // Files are identical; remove the source file.
      (শূন্য)sys::fs::remove(*অস্থায়ী_Path);
      ফেরত Error::success();

    ক্ষেত্র FileDifference::DifferentContents:
      বিরতি; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*অস্থায়ী_Path, OutputPath);
  যদি (!RenameEC)
    ফেরত Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*অস্থায়ী_Path, OutputPath);
  (শূন্য)sys::fs::remove(*অস্থায়ী_Path);

  যদি (!RenameEC)
    ফেরত Error::success();

  ফেরত make_error<অস্থায়ী_File_Output_Error>(*অস্থায়ী_Path, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  যদি (auto E = reset())
    ফেরত E;

  // Nothing on the filesystem to remove for stdout.
  যদি (OutputPath == "-")
    ফেরত Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    ফেরত EC;
  };

  // Clean up the file that's in-progress.
  যদি (!অস্থায়ী_Path)
    ফেরত convertToOutputError(OutputPath, discardPath(OutputPath));
  ফেরত convert_To_অস্থায়ী_File_Output_Error(*অস্থায়ী_Path, OutputPath,
                                      discardPath(*অস্থায়ী_Path));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<অক্ষর> &Path) ধ্রুবক {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  ফেরত convertToOutputError(StringRef(Path.তথ্য(), Path.আকার_3()),
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
      ফেরত std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  যদি (Error E = File->initializeStream())
    ফেরত std::move(E);

  ফেরত std::move(File);
}
