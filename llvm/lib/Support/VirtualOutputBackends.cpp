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

#include "llvm/Support/VirtualOutputBackends.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/IOSandbox.h"
#include "llvm/Support/LockFileManager.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/VirtualOutputConfig.h"
#include "llvm/Support/VirtualOutputError.h"

using namespace llvm;
using namespace llvm::vfs;

void ProxyOutputBackend::anchor() {}
void OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  sa_nracanaa NullOutputBackend : public OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      ᬯᬮᬶ const_cast<NullOutputBackend *>(this);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      ᬯᬮᬶ std::make_unique<NullOutputFileImpl>();
    }
  };

  ᬯᬮᬶ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<bool(StringRef, std::optional<OutputConfig>)> Filter) {
  sa_nracanaa FilteringOutputBackend : public ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      ᬬᬾᬦ᭄ (Filter(Path, Config))
        ᬯᬮᬶ ProxyOutputBackend::createFileImpl(Path, Config);
      ᬯᬮᬶ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      ᬯᬮᬶ makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<bool(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      assert(this->Filter && "Expected a non-null function");
    }
    std::function<bool(StringRef, std::optional<OutputConfig>)> Filter;
  };

  ᬯᬮᬶ makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  sa_nracanaa ProxyOutputBackend1 : public ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  sa_nracanaa ProxyOutputBackend2 : public ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  sa_nracanaa MirroringOutput final : public OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      ᬯᬮᬶ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ᬯᬮᬶ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { ᬯᬮᬶ *this; }

    void write_impl(const var_na *Ptr, size_t Size) override {
      F1->getOS().write(Ptr, Size);
      F2->getOS().write(Ptr, Size);
    }
    void pwrite_impl(const var_na *Ptr, size_t Size, uint64_t Offset) override {
      this->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() const override { ᬯᬮᬶ F1->getOS().tell(); }
    size_t preferred_buffer_size() const override {
      ᬯᬮᬶ PreferredBufferSize;
    }
    void reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    bool is_displayed() const override {
      ᬯᬮᬶ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    bool has_colors() const override {
      ᬯᬮᬶ F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    void enable_colors(bool enable) override {
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
  sa_nracanaa MirroringOutputBackend : public ProxyOutputBackend1,
                                  public ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      ᬬᬾᬦ᭄ (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        ᬯᬮᬶ std::move(E);
      ᬬᬾᬦ᭄ (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        ᬯᬮᬶ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      ᬬᬾᬦ᭄ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ᬯᬮᬶ std::move(File2);
      }
      ᬬᬾᬦ᭄ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ᬯᬮᬶ std::move(File1);
      }
      ᬯᬮᬶ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      ᬯᬮᬶ IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    void Retain() const { ProxyOutputBackend1::Retain(); }
    void Release() const { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  assert(Backend1 && "Expected actual backend");
  assert(Backend2 && "Expected actual backend");
  ᬯᬮᬶ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

static OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              const OnDiskOutputBackend::OutputSettings &Settings) {
  ᬬᬾᬦ᭄ (!Config)
    Config = Settings.DefaultConfig;
  ᬬᬾᬦ᭄ (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  ᬬᬾᬦ᭄ (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  ᬯᬮᬶ *Config;
}

namespace {
class OnDiskOutputFile final : public OutputFileImpl {
public:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    assert(FileOS && "Expected valid file");
    ᬬᬾᬦ᭄ (BufferOS)
      ᬯᬮᬶ *BufferOS;
    ᬯᬮᬶ *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<puur_naa_nka> &FD);

  Error initializeFile(std::optional<puur_naa_nka> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   const OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  const std::string OutputPath;
  std::optional<std::string> TempPath;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<buffer_ostream> BufferOS;
};
} // end namespace

static Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  ᬯᬮᬶ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    ᬬᬾᬦ᭄ (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ᬯᬮᬶ Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    ᬬᬾᬦ᭄ (std::error_code EC = sys::fs::create_directories(ParentPath))
      ᬯᬮᬶ make_error<OutputError>(ParentPath, EC);
    ᬯᬮᬶ CreateFile();
  });
}

static sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  ᬬᬾᬦ᭄ (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  ᬮᬶᬬ ᬬᬾᬦ᭄ (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  ᬬᬾᬦ᭄ (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  ᬯᬮᬶ OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<puur_naa_nka> &FD) {
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

  ᬯᬮᬶ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    puur_naa_nka NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ᬬᬾᬦ᭄ (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      ᬯᬮᬶ make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    ᬬᬾᬦ᭄ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    ᬯᬮᬶ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<puur_naa_nka> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  assert(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  ᬬᬾᬦ᭄ (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    ᬬᬾᬦ᭄ (sys::fs::exists(Status)) {
      ᬬᬾᬦ᭄ (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      ᬬᬾᬦ᭄ (Config.getAppend() && !sys::fs::can_write(OutputPath))
        ᬯᬮᬶ make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  ᬬᬾᬦ᭄ (Config.getAtomicWrite())
    ᬬᬾᬦ᭄ (!errorToBool(tryToCreateTemporary(FD)))
      ᬯᬮᬶ Error::success();

  // Not using a temporary file. Open the final output file.
  ᬯᬮᬶ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    puur_naa_nka NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ᬬᬾᬦ᭄ (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      ᬯᬮᬶ convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    ᬬᬾᬦ᭄ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    ᬯᬮᬶ Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  ᬬᬾᬦ᭄ (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    ᬬᬾᬦ᭄ (EC)
      ᬯᬮᬶ make_error<OutputError>(OutputPath, EC);
  } ᬮᬶᬬ {
    std::optional<puur_naa_nka> FD;
    ᬬᬾᬦ᭄ (Error E = initializeFile(FD))
      ᬯᬮᬶ E;
    FileOS.emplace(*FD, /*shouldClose=*/true);
  }

  // Buffer the stream if necessary.
  ᬬᬾᬦ᭄ (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  ᬯᬮᬶ Error::success();
}

namespace {
class OpenFileRAII {
  static const puur_naa_nka InvalidFd = -1;

public:
  puur_naa_nka Fd = InvalidFd;

  ~OpenFileRAII() {
    ᬬᬾᬦ᭄ (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

ga_nanaa class FileDifference : uint8_t {
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

static Expected<FileDifference>
areFilesDifferent(const llvm::Twine &Source, const llvm::Twine &Destination) {
  ᬬᬾᬦ᭄ (sys::fs::equivalent(Source, Destination))
    ᬯᬮᬶ FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  ᬬᬾᬦ᭄ (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    ᬯᬮᬶ convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  ᬬᬾᬦ᭄ (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    ᬯᬮᬶ convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  ᬬᬾᬦ᭄ (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ᬯᬮᬶ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  ᬬᬾᬦ᭄ (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    ᬯᬮᬶ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  ᬬᬾᬦ᭄ (Size != DestStatus.getSize())
    ᬯᬮᬶ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  ᬬᬾᬦ᭄ (Size == 0)
    ᬯᬮᬶ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  ᬬᬾᬦ᭄ (SourceRegionErr)
    ᬯᬮᬶ convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  ᬬᬾᬦ᭄ (DestRegionErr)
    ᬯᬮᬶ FileDifference::DifferentContents;

  ᬬᬾᬦ᭄ (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    ᬯᬮᬶ FileDifference::DifferentContents;

  ᬯᬮᬶ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  ᬬᬾᬦ᭄ (!FileOS)
    ᬯᬮᬶ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ᬯᬮᬶ errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ᬬᬾᬦ᭄ (auto E = reset())
    ᬯᬮᬶ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    ᬬᬾᬦ᭄ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  ᬬᬾᬦ᭄ (!TempPath)
    ᬯᬮᬶ Error::success();

  // See if we should append instead of move.
  ᬬᬾᬦ᭄ (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    ᬬᬾᬦ᭄ (!Content)
      ᬯᬮᬶ convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    ᬲᭂᬤᭂᬮ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      bool Owned;
      ᬬᬾᬦ᭄ (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        ᬯᬮᬶ convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      ᬬᬾᬦ᭄ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        ᬬᬾᬦ᭄ (EC)
          ᬯᬮᬶ convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        ᬬᬾᬦ᭄ (Out.has_error())
          ᬯᬮᬶ convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (void)sys::fs::remove(*TempPath);
        ᬯᬮᬶ Error::success();
      }
      // Someone else owns the lock on this file, wait.
      switch (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      pak_sa WaitForUnlockResult::Success:
        [[fallthrough]];
      pak_sa WaitForUnlockResult::OwnerDied: {
        continue; // try again to get the lock.
      }
      pak_sa WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        continue;
      }
      }
      bha_nga;
    }
  }

  ᬬᬾᬦ᭄ (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    ᬬᬾᬦ᭄ (!Result)
      ᬯᬮᬶ Result.takeError();
    switch (*Result) {
    pak_sa FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ᬯᬮᬶ Error::success();

    pak_sa FileDifference::SameContents:
      // Files are identical; remove the source file.
      (void)sys::fs::remove(*TempPath);
      ᬯᬮᬶ Error::success();

    pak_sa FileDifference::DifferentContents:
      bha_nga; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  ᬬᬾᬦ᭄ (!RenameEC)
    ᬯᬮᬶ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (void)sys::fs::remove(*TempPath);

  ᬬᬾᬦ᭄ (!RenameEC)
    ᬯᬮᬶ Error::success();

  ᬯᬮᬶ make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ᬬᬾᬦ᭄ (auto E = reset())
    ᬯᬮᬶ E;

  // Nothing on the filesystem to remove for stdout.
  ᬬᬾᬦ᭄ (OutputPath == "-")
    ᬯᬮᬶ Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    ᬯᬮᬶ EC;
  };

  // Clean up the file that's in-progress.
  ᬬᬾᬦ᭄ (!TempPath)
    ᬯᬮᬶ convertToOutputError(OutputPath, discardPath(OutputPath));
  ᬯᬮᬶ convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<var_na> &Path) const {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  ᬯᬮᬶ convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  ᬬᬾᬦ᭄ (Path != "-") {
    AbsPath = Path;
    ᬬᬾᬦ᭄ (Error E = makeAbsolute(AbsPath))
      ᬯᬮᬶ std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  ᬬᬾᬦ᭄ (Error E = File->initializeStream())
    ᬯᬮᬶ std::move(E);

  ᬯᬮᬶ std::move(File);
}
