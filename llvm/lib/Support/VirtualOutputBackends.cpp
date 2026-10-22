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

#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputBackends.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ScopeExit.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LockFileManager.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputConfig.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputError.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::vfs;

ಶೂನ್ಯ ProxyOutputBackend::anchor() {}
ಶೂನ್ಯ OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  ಸಂರಚನಾ NullOutputBackend : ಸಾರ್ವಜನಿಕ OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ಧ್ರುವ override {
      ಪ್ರತ್ಯಾಗಮ const_cast<NullOutputBackend *>(ಏತತ್);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      ಪ್ರತ್ಯಾಗಮ std::make_unique<NullOutputFileImpl>();
    }
  };

  ಪ್ರತ್ಯಾಗಮ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<ತಾರ್ಕಿಕ(StringRef, std::optional<OutputConfig>)> Filter) {
  ಸಂರಚನಾ FilteringOutputBackend : ಸಾರ್ವಜನಿಕ ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      ಯದಿ (Filter(Path, Config))
        ಪ್ರತ್ಯಾಗಮ ProxyOutputBackend::createFileImpl(Path, Config);
      ಪ್ರತ್ಯಾಗಮ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ಧ್ರುವ override {
      ಪ್ರತ್ಯಾಗಮ makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<ತಾರ್ಕಿಕ(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      ಪ್ರತಿಜ್ಞಾ(ಏತತ್->Filter && "Expected a non-null function");
    }
    std::function<ತಾರ್ಕಿಕ(StringRef, std::optional<OutputConfig>)> Filter;
  };

  ಪ್ರತ್ಯಾಗಮ makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  ಸಂರಚನಾ ProxyOutputBackend1 : ಸಾರ್ವಜನಿಕ ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  ಸಂರಚನಾ ProxyOutputBackend2 : ಸಾರ್ವಜನಿಕ ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  ಸಂರಚನಾ MirroringOutput final : ಸಾರ್ವಜನಿಕ OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      ಪ್ರತ್ಯಾಗಮ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ಪ್ರತ್ಯಾಗಮ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { ಪ್ರತ್ಯಾಗಮ *ಏತತ್; }

    ಶೂನ್ಯ write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) override {
      F1->getOS().ಲಿಖ(Ptr, Size);
      F2->getOS().ಲಿಖ(Ptr, Size);
    }
    ಶೂನ್ಯ pwrite_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size, uint64_t Offset) override {
      ಏತತ್->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ F1->getOS().tell(); }
    size_t preferred_buffer_size() ಧ್ರುವ override {
      ಪ್ರತ್ಯಾಗಮ PreferredBufferSize;
    }
    ಶೂನ್ಯ reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    ತಾರ್ಕಿಕ is_displayed() ಧ್ರುವ override {
      ಪ್ರತ್ಯಾಗಮ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    ತಾರ್ಕಿಕ has_colors() ಧ್ರುವ override {
      ಪ್ರತ್ಯಾಗಮ F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    ಶೂನ್ಯ enable_colors(ತಾರ್ಕಿಕ enable) override {
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
  ಸಂರಚನಾ MirroringOutputBackend : ಸಾರ್ವಜನಿಕ ProxyOutputBackend1,
                                  ಸಾರ್ವಜನಿಕ ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      ಯದಿ (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        ಪ್ರತ್ಯಾಗಮ std::move(E);
      ಯದಿ (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        ಪ್ರತ್ಯಾಗಮ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      ಯದಿ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ಪ್ರತ್ಯಾಗಮ std::move(File2);
      }
      ಯದಿ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ಪ್ರತ್ಯಾಗಮ std::move(File1);
      }
      ಪ್ರತ್ಯಾಗಮ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ಧ್ರುವ override {
      ಪ್ರತ್ಯಾಗಮ IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    ಶೂನ್ಯ Retain() ಧ್ರುವ { ProxyOutputBackend1::Retain(); }
    ಶೂನ್ಯ Release() ಧ್ರುವ { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  ಪ್ರತಿಜ್ಞಾ(Backend1 && "Expected actual backend");
  ಪ್ರತಿಜ್ಞಾ(Backend2 && "Expected actual backend");
  ಪ್ರತ್ಯಾಗಮ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ಸ್ಥಿರ OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ಧ್ರುವ OnDiskOutputBackend::OutputSettings &Settings) {
  ಯದಿ (!Config)
    Config = Settings.DefaultConfig;
  ಯದಿ (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  ಯದಿ (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  ಪ್ರತ್ಯಾಗಮ *Config;
}

ನಾಮಾಕಾಶ {
ವರ್ಗ OnDiskOutputFile final : ಸಾರ್ವಜನಿಕ OutputFileImpl {
ಸಾರ್ವಜನಿಕ:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    ಪ್ರತಿಜ್ಞಾ(FileOS && "Expected valid file");
    ಯದಿ (BufferOS)
      ಪ್ರತ್ಯಾಗಮ *BufferOS;
    ಪ್ರತ್ಯಾಗಮ *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<ಪೂರ್ಣಾಙ್ಕ> &FD);

  Error initializeFile(std::optional<ಪೂರ್ಣಾಙ್ಕ> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   ಧ್ರುವ OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  ಧ್ರುವ std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

ಸ್ಥಿರ Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  ಪ್ರತ್ಯಾಗಮ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    ಯದಿ (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ಪ್ರತ್ಯಾಗಮ Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    ಯದಿ (std::error_code EC = sys::fs::create_directories(ParentPath))
      ಪ್ರತ್ಯಾಗಮ make_error<OutputError>(ParentPath, EC);
    ಪ್ರತ್ಯಾಗಮ CreateFile();
  });
}

ಸ್ಥಿರ sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  ಯದಿ (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  ಅನ್ಯಥಾ ಯದಿ (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  ಯದಿ (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  ಪ್ರತ್ಯಾಗಮ OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<ಪೂರ್ಣಾಙ್ಕ> &FD) {
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

  ಪ್ರತ್ಯಾಗಮ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    ಪೂರ್ಣಾಙ್ಕ NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ಯದಿ (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      ಪ್ರತ್ಯಾಗಮ make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    ಯದಿ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    ಪ್ರತ್ಯಾಗಮ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<ಪೂರ್ಣಾಙ್ಕ> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  ಯದಿ (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    ಯದಿ (sys::fs::exists(Status)) {
      ಯದಿ (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      ಯದಿ (Config.getAppend() && !sys::fs::can_write(OutputPath))
        ಪ್ರತ್ಯಾಗಮ make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  ಯದಿ (Config.getAtomicWrite())
    ಯದಿ (!errorToBool(tryToCreateTemporary(FD)))
      ಪ್ರತ್ಯಾಗಮ Error::success();

  // Not using a temporary file. Open the final output file.
  ಪ್ರತ್ಯಾಗಮ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    ಪೂರ್ಣಾಙ್ಕ NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ಯದಿ (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      ಪ್ರತ್ಯಾಗಮ convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    ಯದಿ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    ಪ್ರತ್ಯಾಗಮ Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  ಯದಿ (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    ಯದಿ (EC)
      ಪ್ರತ್ಯಾಗಮ make_error<OutputError>(OutputPath, EC);
  } ಅನ್ಯಥಾ {
    std::optional<ಪೂರ್ಣಾಙ್ಕ> FD;
    ಯದಿ (Error E = initializeFile(FD))
      ಪ್ರತ್ಯಾಗಮ E;
    FileOS.emplace(*FD, /*shouldClose=*/ಸತ್ಯ);
  }

  // Buffer the stream if necessary.
  ಯದಿ (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  ಪ್ರತ್ಯಾಗಮ Error::success();
}

ನಾಮಾಕಾಶ {
ವರ್ಗ OpenFileRAII {
  ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ InvalidFd = -1;

ಸಾರ್ವಜನಿಕ:
  ಪೂರ್ಣಾಙ್ಕ Fd = InvalidFd;

  ~OpenFileRAII() {
    ಯದಿ (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

ಗಣನಾ ವರ್ಗ FileDifference : uint8_t {
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

ಸ್ಥಿರ Expected<FileDifference>
areFilesDifferent(ಧ್ರುವ llvm::Twine &Source, ಧ್ರುವ llvm::Twine &Destination) {
  ಯದಿ (sys::fs::equivalent(Source, Destination))
    ಪ್ರತ್ಯಾಗಮ FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  ಯದಿ (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    ಪ್ರತ್ಯಾಗಮ convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  ಯದಿ (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    ಪ್ರತ್ಯಾಗಮ convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  ಯದಿ (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ಪ್ರತ್ಯಾಗಮ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  ಯದಿ (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    ಪ್ರತ್ಯಾಗಮ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  ಯದಿ (Size != DestStatus.getSize())
    ಪ್ರತ್ಯಾಗಮ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  ಯದಿ (Size == 0)
    ಪ್ರತ್ಯಾಗಮ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  ಯದಿ (SourceRegionErr)
    ಪ್ರತ್ಯಾಗಮ convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  ಯದಿ (DestRegionErr)
    ಪ್ರತ್ಯಾಗಮ FileDifference::DifferentContents;

  ಯದಿ (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    ಪ್ರತ್ಯಾಗಮ FileDifference::DifferentContents;

  ಪ್ರತ್ಯಾಗಮ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  ಯದಿ (!FileOS)
    ಪ್ರತ್ಯಾಗಮ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಯದಿ (auto E = reset())
    ಪ್ರತ್ಯಾಗಮ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    ಯದಿ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  ಯದಿ (!TempPath)
    ಪ್ರತ್ಯಾಗಮ Error::success();

  // See if we should append instead of move.
  ಯದಿ (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    ಯದಿ (!Content)
      ಪ್ರತ್ಯಾಗಮ convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    ಯಾವತ್ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      ತಾರ್ಕಿಕ Owned;
      ಯದಿ (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        ಪ್ರತ್ಯಾಗಮ convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      ಯದಿ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        ಯದಿ (EC)
          ಪ್ರತ್ಯಾಗಮ convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        ಯದಿ (Out.has_error())
          ಪ್ರತ್ಯಾಗಮ convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (ಶೂನ್ಯ)sys::fs::remove(*TempPath);
        ಪ್ರತ್ಯಾಗಮ Error::success();
      }
      // Someone else owns the lock on this file, wait.
      ವಿಕಲ್ಪ (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      ಪಕ್ಷ WaitForUnlockResult::Success:
        [[fallthrough]];
      ಪಕ್ಷ WaitForUnlockResult::OwnerDied: {
        ಅನುವರ್ತ; // try again to get the lock.
      }
      ಪಕ್ಷ WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        ಅನುವರ್ತ;
      }
      }
      ಭಙ್ಗ;
    }
  }

  ಯದಿ (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    ಯದಿ (!Result)
      ಪ್ರತ್ಯಾಗಮ Result.takeError();
    ವಿಕಲ್ಪ (*Result) {
    ಪಕ್ಷ FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ಪ್ರತ್ಯಾಗಮ Error::success();

    ಪಕ್ಷ FileDifference::SameContents:
      // Files are identical; remove the source file.
      (ಶೂನ್ಯ)sys::fs::remove(*TempPath);
      ಪ್ರತ್ಯಾಗಮ Error::success();

    ಪಕ್ಷ FileDifference::DifferentContents:
      ಭಙ್ಗ; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  ಯದಿ (!RenameEC)
    ಪ್ರತ್ಯಾಗಮ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (ಶೂನ್ಯ)sys::fs::remove(*TempPath);

  ಯದಿ (!RenameEC)
    ಪ್ರತ್ಯಾಗಮ Error::success();

  ಪ್ರತ್ಯಾಗಮ make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ಯದಿ (auto E = reset())
    ಪ್ರತ್ಯಾಗಮ E;

  // Nothing on the filesystem to remove for stdout.
  ಯದಿ (OutputPath == "-")
    ಪ್ರತ್ಯಾಗಮ Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    ಪ್ರತ್ಯಾಗಮ EC;
  };

  // Clean up the file that's in-progress.
  ಯದಿ (!TempPath)
    ಪ್ರತ್ಯಾಗಮ convertToOutputError(OutputPath, discardPath(OutputPath));
  ಪ್ರತ್ಯಾಗಮ convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<ವರ್ಣ> &Path) ಧ್ರುವ {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  ಪ್ರತ್ಯಾಗಮ convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  ಯದಿ (Path != "-") {
    AbsPath = Path;
    ಯದಿ (Error E = makeAbsolute(AbsPath))
      ಪ್ರತ್ಯಾಗಮ std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  ಯದಿ (Error E = File->initializeStream())
    ಪ್ರತ್ಯಾಗಮ std::move(E);

  ಪ್ರತ್ಯಾಗಮ std::move(File);
}
