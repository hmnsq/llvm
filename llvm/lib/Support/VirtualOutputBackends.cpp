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
  struct NullOutputBackend : public OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      반환 const_cast<NullOutputBackend *>(this);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      반환 std::make_unique<NullOutputFileImpl>();
    }
  };

  반환 makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<bool(StringRef, std::optional<OutputConfig>)> Filter) {
  struct FilteringOutputBackend : public ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      만약 (Filter(Path, Config))
        반환 ProxyOutputBackend::createFileImpl(Path, Config);
      반환 std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      반환 makeIntrusiveRefCnt<FilteringOutputBackend>(
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

  반환 makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  struct ProxyOutputBackend1 : public ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  struct ProxyOutputBackend2 : public ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  struct MirroringOutput final : public OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      flush();
      반환 joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      반환 joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { 반환 *this; }

    void write_impl(const char *Ptr, size_t Size) override {
      F1->getOS().write(Ptr, Size);
      F2->getOS().write(Ptr, Size);
    }
    void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) override {
      this->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() const override { 반환 F1->getOS().tell(); }
    size_t preferred_buffer_size() const override {
      반환 PreferredBufferSize;
    }
    void reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    bool is_displayed() const override {
      반환 F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    bool has_colors() const override {
      반환 F1->getOS().has_colors() && F2->getOS().has_colors();
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
  struct MirroringOutputBackend : public ProxyOutputBackend1,
                                  public ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      만약 (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        반환 std::move(E);
      만약 (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        반환 joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      만약 (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        반환 std::move(File2);
      }
      만약 (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        반환 std::move(File1);
      }
      반환 std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      반환 IntrusiveRefCntPtr<ProxyOutputBackend1>(
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
  반환 IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

static OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              const OnDiskOutputBackend::OutputSettings &Settings) {
  만약 (!Config)
    Config = Settings.DefaultConfig;
  만약 (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  만약 (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  반환 *Config;
}

namespace {
class OnDiskOutputFile final : public OutputFileImpl {
public:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    assert(FileOS && "Expected valid file");
    만약 (BufferOS)
      반환 *BufferOS;
    반환 *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<정수> &FD);

  Error initializeFile(std::optional<정수> &FD);
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
  반환 handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    만약 (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      반환 Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    만약 (std::error_code EC = sys::fs::create_directories(ParentPath))
      반환 make_error<OutputError>(ParentPath, EC);
    반환 CreateFile();
  });
}

static sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  만약 (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  아니면 만약 (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  만약 (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  반환 OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<정수> &FD) {
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

  반환 createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    정수 NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    만약 (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      반환 make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    만약 (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    반환 Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<정수> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  assert(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  만약 (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    만약 (sys::fs::exists(Status)) {
      만약 (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      만약 (Config.getAppend() && !sys::fs::can_write(OutputPath))
        반환 make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  만약 (Config.getAtomicWrite())
    만약 (!errorToBool(tryToCreateTemporary(FD)))
      반환 Error::success();

  // Not using a temporary file. Open the final output file.
  반환 createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    정수 NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    만약 (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      반환 convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    만약 (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    반환 Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  만약 (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    만약 (EC)
      반환 make_error<OutputError>(OutputPath, EC);
  } 아니면 {
    std::optional<정수> FD;
    만약 (Error E = initializeFile(FD))
      반환 E;
    FileOS.emplace(*FD, /*shouldClose=*/true);
  }

  // Buffer the stream if necessary.
  만약 (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  반환 Error::success();
}

namespace {
class OpenFileRAII {
  static const 정수 InvalidFd = -1;

public:
  정수 Fd = InvalidFd;

  ~OpenFileRAII() {
    만약 (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

enum class FileDifference : uint8_t {
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
  만약 (sys::fs::equivalent(Source, Destination))
    반환 FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  만약 (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    반환 convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  만약 (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    반환 convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  만약 (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    반환 FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  만약 (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    반환 FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  만약 (Size != DestStatus.getSize())
    반환 FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  만약 (Size == 0)
    반환 FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  만약 (SourceRegionErr)
    반환 convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  만약 (DestRegionErr)
    반환 FileDifference::DifferentContents;

  만약 (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    반환 FileDifference::DifferentContents;

  반환 FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  만약 (!FileOS)
    반환 Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  반환 errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  만약 (auto E = reset())
    반환 E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    만약 (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  만약 (!TempPath)
    반환 Error::success();

  // See if we should append instead of move.
  만약 (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    만약 (!Content)
      반환 convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    동안 (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      bool Owned;
      만약 (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        반환 convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      만약 (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        만약 (EC)
          반환 convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        만약 (Out.has_error())
          반환 convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (void)sys::fs::remove(*TempPath);
        반환 Error::success();
      }
      // Someone else owns the lock on this file, wait.
      switch (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      case WaitForUnlockResult::Success:
        [[fallthrough]];
      case WaitForUnlockResult::OwnerDied: {
        continue; // try again to get the lock.
      }
      case WaitForUnlockResult::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        continue;
      }
      }
      break;
    }
  }

  만약 (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    만약 (!Result)
      반환 Result.takeError();
    switch (*Result) {
    case FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      반환 Error::success();

    case FileDifference::SameContents:
      // Files are identical; remove the source file.
      (void)sys::fs::remove(*TempPath);
      반환 Error::success();

    case FileDifference::DifferentContents:
      break; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  만약 (!RenameEC)
    반환 Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (void)sys::fs::remove(*TempPath);

  만약 (!RenameEC)
    반환 Error::success();

  반환 make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  만약 (auto E = reset())
    반환 E;

  // Nothing on the filesystem to remove for stdout.
  만약 (OutputPath == "-")
    반환 Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    반환 EC;
  };

  // Clean up the file that's in-progress.
  만약 (!TempPath)
    반환 convertToOutputError(OutputPath, discardPath(OutputPath));
  반환 convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<char> &Path) const {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  반환 convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  만약 (Path != "-") {
    AbsPath = Path;
    만약 (Error E = makeAbsolute(AbsPath))
      반환 std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  만약 (Error E = File->initializeStream())
    반환 std::move(E);

  반환 std::move(File);
}
