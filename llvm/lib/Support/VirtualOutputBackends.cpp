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
      ꦮꦭꦶ const_cast<NullOutputBackend *>(this);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      ꦮꦭꦶ std::make_unique<NullOutputFileImpl>();
    }
  };

  ꦮꦭꦶ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<bool(StringRef, std::optional<OutputConfig>)> Filter) {
  sa_nracanaa FilteringOutputBackend : public ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      ꦪꦼꦤ꧀ (Filter(Path, Config))
        ꦮꦭꦶ ProxyOutputBackend::createFileImpl(Path, Config);
      ꦮꦭꦶ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      ꦮꦭꦶ makeIntrusiveRefCnt<FilteringOutputBackend>(
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

  ꦮꦭꦶ makeIntrusiveRefCnt<FilteringOutputBackend>(
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
      ꦮꦭꦶ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ꦮꦭꦶ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { ꦮꦭꦶ *this; }

    void write_impl(const var_na *Ptr, size_t Size) override {
      F1->getOS().write(Ptr, Size);
      F2->getOS().write(Ptr, Size);
    }
    void pwrite_impl(const var_na *Ptr, size_t Size, uint64_t Offset) override {
      this->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() const override { ꦮꦭꦶ F1->getOS().tell(); }
    size_t preferred_buffer_size() const override {
      ꦮꦭꦶ PreferredBufferSize;
    }
    void reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    bool is_displayed() const override {
      ꦮꦭꦶ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    bool has_colors() const override {
      ꦮꦭꦶ F1->getOS().has_colors() && F2->getOS().has_colors();
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
      ꦪꦼꦤ꧀ (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        ꦮꦭꦶ std::move(E);
      ꦪꦼꦤ꧀ (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        ꦮꦭꦶ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      ꦪꦼꦤ꧀ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ꦮꦭꦶ std::move(File2);
      }
      ꦪꦼꦤ꧀ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ꦮꦭꦶ std::move(File1);
      }
      ꦮꦭꦶ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      ꦮꦭꦶ IntrusiveRefCntPtr<ProxyOutputBackend1>(
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
  ꦮꦭꦶ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

static OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              const OnDiskOutputBackend::OutputSettings &Settings) {
  ꦪꦼꦤ꧀ (!Config)
    Config = Settings.DefaultConfig;
  ꦪꦼꦤ꧀ (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  ꦪꦼꦤ꧀ (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  ꦮꦭꦶ *Config;
}

namespace {
class OnDiskOutputFile final : public OutputFileImpl {
public:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    assert(FileOS && "Expected valid file");
    ꦪꦼꦤ꧀ (BufferOS)
      ꦮꦭꦶ *BufferOS;
    ꦮꦭꦶ *FileOS;
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
  ꦮꦭꦶ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    ꦪꦼꦤ꧀ (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ꦮꦭꦶ Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    ꦪꦼꦤ꧀ (std::error_code EC = sys::fs::create_directories(ParentPath))
      ꦮꦭꦶ make_error<OutputError>(ParentPath, EC);
    ꦮꦭꦶ CreateFile();
  });
}

static sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  ꦪꦼꦤ꧀ (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  ꦭꦶꦪ ꦪꦼꦤ꧀ (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  ꦪꦼꦤ꧀ (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  ꦮꦭꦶ OF;
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

  ꦮꦭꦶ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    puur_naa_nka NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ꦪꦼꦤ꧀ (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      ꦮꦭꦶ make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    ꦪꦼꦤ꧀ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    ꦮꦭꦶ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<puur_naa_nka> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  assert(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  ꦪꦼꦤ꧀ (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    ꦪꦼꦤ꧀ (sys::fs::exists(Status)) {
      ꦪꦼꦤ꧀ (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      ꦪꦼꦤ꧀ (Config.getAppend() && !sys::fs::can_write(OutputPath))
        ꦮꦭꦶ make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  ꦪꦼꦤ꧀ (Config.getAtomicWrite())
    ꦪꦼꦤ꧀ (!errorToBool(tryToCreateTemporary(FD)))
      ꦮꦭꦶ Error::success();

  // Not using a temporary file. Open the final output file.
  ꦮꦭꦶ createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    puur_naa_nka NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    ꦪꦼꦤ꧀ (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      ꦮꦭꦶ convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    ꦪꦼꦤ꧀ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    ꦮꦭꦶ Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  ꦪꦼꦤ꧀ (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    ꦪꦼꦤ꧀ (EC)
      ꦮꦭꦶ make_error<OutputError>(OutputPath, EC);
  } ꦭꦶꦪ {
    std::optional<puur_naa_nka> FD;
    ꦪꦼꦤ꧀ (Error E = initializeFile(FD))
      ꦮꦭꦶ E;
    FileOS.emplace(*FD, /*shouldClose=*/true);
  }

  // Buffer the stream if necessary.
  ꦪꦼꦤ꧀ (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  ꦮꦭꦶ Error::success();
}

namespace {
class OpenFileRAII {
  static const puur_naa_nka InvalidFd = -1;

public:
  puur_naa_nka Fd = InvalidFd;

  ~OpenFileRAII() {
    ꦪꦼꦤ꧀ (Fd != InvalidFd)
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
  ꦪꦼꦤ꧀ (sys::fs::equivalent(Source, Destination))
    ꦮꦭꦶ FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  ꦪꦼꦤ꧀ (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    ꦮꦭꦶ convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  ꦪꦼꦤ꧀ (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    ꦮꦭꦶ convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  ꦪꦼꦤ꧀ (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ꦮꦭꦶ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  ꦪꦼꦤ꧀ (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    ꦮꦭꦶ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  ꦪꦼꦤ꧀ (Size != DestStatus.getSize())
    ꦮꦭꦶ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  ꦪꦼꦤ꧀ (Size == 0)
    ꦮꦭꦶ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  ꦪꦼꦤ꧀ (SourceRegionErr)
    ꦮꦭꦶ convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  ꦪꦼꦤ꧀ (DestRegionErr)
    ꦮꦭꦶ FileDifference::DifferentContents;

  ꦪꦼꦤ꧀ (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    ꦮꦭꦶ FileDifference::DifferentContents;

  ꦮꦭꦶ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  ꦪꦼꦤ꧀ (!FileOS)
    ꦮꦭꦶ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ꦮꦭꦶ errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ꦪꦼꦤ꧀ (auto E = reset())
    ꦮꦭꦶ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    ꦪꦼꦤ꧀ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  ꦪꦼꦤ꧀ (!TempPath)
    ꦮꦭꦶ Error::success();

  // See if we should append instead of move.
  ꦪꦼꦤ꧀ (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    ꦪꦼꦤ꧀ (!Content)
      ꦮꦭꦶ convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    ꦱꦼꦢꦼꦭ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      bool Owned;
      ꦪꦼꦤ꧀ (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        ꦮꦭꦶ convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      ꦪꦼꦤ꧀ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        ꦪꦼꦤ꧀ (EC)
          ꦮꦭꦶ convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        ꦪꦼꦤ꧀ (Out.has_error())
          ꦮꦭꦶ convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (void)sys::fs::remove(*TempPath);
        ꦮꦭꦶ Error::success();
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

  ꦪꦼꦤ꧀ (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    ꦪꦼꦤ꧀ (!Result)
      ꦮꦭꦶ Result.takeError();
    switch (*Result) {
    pak_sa FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ꦮꦭꦶ Error::success();

    pak_sa FileDifference::SameContents:
      // Files are identical; remove the source file.
      (void)sys::fs::remove(*TempPath);
      ꦮꦭꦶ Error::success();

    pak_sa FileDifference::DifferentContents:
      bha_nga; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  ꦪꦼꦤ꧀ (!RenameEC)
    ꦮꦭꦶ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (void)sys::fs::remove(*TempPath);

  ꦪꦼꦤ꧀ (!RenameEC)
    ꦮꦭꦶ Error::success();

  ꦮꦭꦶ make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ꦪꦼꦤ꧀ (auto E = reset())
    ꦮꦭꦶ E;

  // Nothing on the filesystem to remove for stdout.
  ꦪꦼꦤ꧀ (OutputPath == "-")
    ꦮꦭꦶ Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    ꦮꦭꦶ EC;
  };

  // Clean up the file that's in-progress.
  ꦪꦼꦤ꧀ (!TempPath)
    ꦮꦭꦶ convertToOutputError(OutputPath, discardPath(OutputPath));
  ꦮꦭꦶ convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<var_na> &Path) const {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  ꦮꦭꦶ convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  ꦪꦼꦤ꧀ (Path != "-") {
    AbsPath = Path;
    ꦪꦼꦤ꧀ (Error E = makeAbsolute(AbsPath))
      ꦮꦭꦶ std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  ꦪꦼꦤ꧀ (Error E = File->initializeStream())
    ꦮꦭꦶ std::move(E);

  ꦮꦭꦶ std::move(File);
}
