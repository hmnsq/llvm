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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputBackends.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ScopeExit.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/IOSandbox.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/LockFileManager.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signals.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputConfig.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputError.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::vfs;

ଶୂନ୍ୟ ProxyOutputBackend::anchor() {}
ଶୂନ୍ୟ OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  ସଂରଚନା NullOutputBackend : ସର୍ବସାଧାରଣ OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ଅଚଳ override {
      ଫେରନ୍ତୁ const_cast<NullOutputBackend *>(ଏହି);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      ଫେରନ୍ତୁ std::make_unique<NullOutputFileImpl>();
    }
  };

  ଫେରନ୍ତୁ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<ତାର୍କିକ(StringRef, std::optional<OutputConfig>)> Filter) {
  ସଂରଚନା FilteringOutputBackend : ସର୍ବସାଧାରଣ ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      ଯଦି (Filter(Path, Config))
        ଫେରନ୍ତୁ ProxyOutputBackend::createFileImpl(Path, Config);
      ଫେରନ୍ତୁ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ଅଚଳ override {
      ଫେରନ୍ତୁ makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<ତାର୍କିକ(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      ପ୍ରତିଜ୍ଞା(ଏହି->Filter && "Expected a non-null function");
    }
    std::function<ତାର୍କିକ(StringRef, std::optional<OutputConfig>)> Filter;
  };

  ଫେରନ୍ତୁ makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  ସଂରଚନା ProxyOutputBackend1 : ସର୍ବସାଧାରଣ ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  ସଂରଚନା ProxyOutputBackend2 : ସର୍ବସାଧାରଣ ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  ସଂରଚନା MirroringOutput final : ସର୍ବସାଧାରଣ OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      ଫେରନ୍ତୁ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ଫେରନ୍ତୁ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { ଫେରନ୍ତୁ *ଏହି; }

    ଶୂନ୍ୟ write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) override {
      F1->getOS().ଲିଖ(Ptr, Size);
      F2->getOS().ଲିଖ(Ptr, Size);
    }
    ଶୂନ୍ୟ pwrite_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size, uint64_t Offset) override {
      ଏହି->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() ଅଚଳ override { ଫେରନ୍ତୁ F1->getOS().tell(); }
    size_t preferred_buffer_size() ଅଚଳ override {
      ଫେରନ୍ତୁ PreferredBufferSize;
    }
    ଶୂନ୍ୟ reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    ତାର୍କିକ is_displayed() ଅଚଳ override {
      ଫେରନ୍ତୁ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    ତାର୍କିକ has_colors() ଅଚଳ override {
      ଫେରନ୍ତୁ F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    ଶୂନ୍ୟ enable_colors(ତାର୍କିକ enable) override {
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
  ସଂରଚନା MirroringOutputBackend : ସର୍ବସାଧାରଣ ProxyOutputBackend1,
                                  ସର୍ବସାଧାରଣ ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      ଯଦି (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        ଫେରନ୍ତୁ std::move(E);
      ଯଦି (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        ଫେରନ୍ତୁ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      ଯଦି (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ଫେରନ୍ତୁ std::move(File2);
      }
      ଯଦି (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ଫେରନ୍ତୁ std::move(File1);
      }
      ଫେରନ୍ତୁ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ଅଚଳ override {
      ଫେରନ୍ତୁ IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    ଶୂନ୍ୟ Retain() ଅଚଳ { ProxyOutputBackend1::Retain(); }
    ଶୂନ୍ୟ Release() ଅଚଳ { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  ପ୍ରତିଜ୍ଞା(Backend1 && "Expected actual backend");
  ପ୍ରତିଜ୍ଞା(Backend2 && "Expected actual backend");
  ଫେରନ୍ତୁ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ସ୍ଥିର OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ଅଚଳ OnDiskOutputBackend::OutputSettings &Settings) {
  ଯଦି (!Config)
    Config = Settings.DefaultConfig;
  ଯଦି (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  ଯଦି (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  ଫେରନ୍ତୁ *Config;
}

ନାମାକାଶ {
ଶ୍ରେଣୀ OnDiskOutputFile final : ସର୍ବସାଧାରଣ OutputFileImpl {
ସର୍ବସାଧାରଣ:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    ପ୍ରତିଜ୍ଞା(FileOS && "Expected valid file");
    ଯଦି (BufferOS)
      ଫେରନ୍ତୁ *BufferOS;
    ଫେରନ୍ତୁ *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> &FD);

  Error initializeFile(std::optional<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   ଅଚଳ OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  ଅଚଳ std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

ସ୍ଥିର Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  ଫେରନ୍ତୁ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    ଯଦି (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ଫେରନ୍ତୁ Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    ଯଦି (std::error_code EC = sys::fs::create_directories(ParentPath))
      ଫେରନ୍ତୁ make_error<OutputError>(ParentPath, EC);
    ଫେରନ୍ତୁ CreateFile();
  });
}

ସ୍ଥିର sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  ଯଦି (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  ଅନ୍ୟଥା ଯଦି (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  ଯଦି (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  ଫେରନ୍ତୁ OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> &FD) {
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

  ଫେରନ୍ତୁ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ଯଦି (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      ଫେରନ୍ତୁ make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    ଯଦି (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    ଫେରନ୍ତୁ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ପ୍ରତିଜ୍ଞା(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  ଯଦି (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    ଯଦି (sys::fs::exists(Status)) {
      ଯଦି (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      ଯଦି (Config.getAppend() && !sys::fs::can_write(OutputPath))
        ଫେରନ୍ତୁ make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  ଯଦି (Config.getAtomicWrite())
    ଯଦି (!errorToBool(tryToCreateTemporary(FD)))
      ଫେରନ୍ତୁ Error::success();

  // Not using a temporary file. Open the final output file.
  ଫେରନ୍ତୁ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ଯଦି (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      ଫେରନ୍ତୁ convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    ଯଦି (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    ଫେରନ୍ତୁ Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  ଯଦି (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    ଯଦି (EC)
      ଫେରନ୍ତୁ make_error<OutputError>(OutputPath, EC);
  } ଅନ୍ୟଥା {
    std::optional<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> FD;
    ଯଦି (Error E = initializeFile(FD))
      ଫେରନ୍ତୁ E;
    FileOS.emplace(*FD, /*shouldClose=*/ସତ୍ୟ);
  }

  // Buffer the stream if necessary.
  ଯଦି (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  ଫେରନ୍ତୁ Error::success();
}

ନାମାକାଶ {
ଶ୍ରେଣୀ OpenFileRAII {
  ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା InvalidFd = -1;

ସର୍ବସାଧାରଣ:
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Fd = InvalidFd;

  ~OpenFileRAII() {
    ଯଦି (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

ଗଣନା ଶ୍ରେଣୀ FileDifference : uint8_t {
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

ସ୍ଥିର Expected<FileDifference>
areFilesDifferent(ଅଚଳ llvm::Twine &Source, ଅଚଳ llvm::Twine &Destination) {
  ଯଦି (sys::fs::equivalent(Source, Destination))
    ଫେରନ୍ତୁ FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  ଯଦି (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    ଫେରନ୍ତୁ convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  ଯଦି (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    ଫେରନ୍ତୁ convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  ଯଦି (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ଫେରନ୍ତୁ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  ଯଦି (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    ଫେରନ୍ତୁ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  ଯଦି (Size != DestStatus.getSize())
    ଫେରନ୍ତୁ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  ଯଦି (Size == 0)
    ଫେରନ୍ତୁ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  ଯଦି (SourceRegionErr)
    ଫେରନ୍ତୁ convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  ଯଦି (DestRegionErr)
    ଫେରନ୍ତୁ FileDifference::DifferentContents;

  ଯଦି (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    ଫେରନ୍ତୁ FileDifference::DifferentContents;

  ଫେରନ୍ତୁ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  ଯଦି (!FileOS)
    ଫେରନ୍ତୁ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ଫେରନ୍ତୁ errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ଯଦି (auto E = reset())
    ଫେରନ୍ତୁ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    ଯଦି (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  ଯଦି (!TempPath)
    ଫେରନ୍ତୁ Error::success();

  // See if we should append instead of move.
  ଯଦି (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    ଯଦି (!Content)
      ଫେରନ୍ତୁ convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    ଯେତେବେଳେ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      ତାର୍କିକ Owned;
      ଯଦି (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        ଫେରନ୍ତୁ convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      ଯଦି (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        ଯଦି (EC)
          ଫେରନ୍ତୁ convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        ଯଦି (Out.has_error())
          ଫେରନ୍ତୁ convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (ଶୂନ୍ୟ)sys::fs::remove(*TempPath);
        ଫେରନ୍ତୁ Error::success();
      }
      // Someone else owns the lock on this file, wait.
      ବାଛ (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      ସ୍ଥିତି WaitForUnlockResult::Success:
        [[fallthrough]];
      ସ୍ଥିତି WaitForUnlockResult::OwnerDied: {
        ଜାରି; // try again to get the lock.
      }
      ସ୍ଥିତି WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        ଜାରି;
      }
      }
      ଭାଙ୍ଗ;
    }
  }

  ଯଦି (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    ଯଦି (!Result)
      ଫେରନ୍ତୁ Result.takeError();
    ବାଛ (*Result) {
    ସ୍ଥିତି FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ଫେରନ୍ତୁ Error::success();

    ସ୍ଥିତି FileDifference::SameContents:
      // Files are identical; remove the source file.
      (ଶୂନ୍ୟ)sys::fs::remove(*TempPath);
      ଫେରନ୍ତୁ Error::success();

    ସ୍ଥିତି FileDifference::DifferentContents:
      ଭାଙ୍ଗ; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  ଯଦି (!RenameEC)
    ଫେରନ୍ତୁ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (ଶୂନ୍ୟ)sys::fs::remove(*TempPath);

  ଯଦି (!RenameEC)
    ଫେରନ୍ତୁ Error::success();

  ଫେରନ୍ତୁ make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ଯଦି (auto E = reset())
    ଫେରନ୍ତୁ E;

  // Nothing on the filesystem to remove for stdout.
  ଯଦି (OutputPath == "-")
    ଫେରନ୍ତୁ Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    ଫେରନ୍ତୁ EC;
  };

  // Clean up the file that's in-progress.
  ଯଦି (!TempPath)
    ଫେରନ୍ତୁ convertToOutputError(OutputPath, discardPath(OutputPath));
  ଫେରନ୍ତୁ convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<ଅକ୍ଷର> &Path) ଅଚଳ {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  ଫେରନ୍ତୁ convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  ଯଦି (Path != "-") {
    AbsPath = Path;
    ଯଦି (Error E = makeAbsolute(AbsPath))
      ଫେରନ୍ତୁ std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  ଯଦି (Error E = File->initializeStream())
    ଫେରନ୍ତୁ std::move(E);

  ଫେରନ୍ତୁ std::move(File);
}
