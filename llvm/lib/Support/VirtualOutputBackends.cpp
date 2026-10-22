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
      ပြန်ပေး const_cast<NullOutputBackend *>(this);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      ပြန်ပေး std::make_unique<NullOutputFileImpl>();
    }
  };

  ပြန်ပေး makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<bool(StringRef, std::optional<OutputConfig>)> Filter) {
  sa_nracanaa FilteringOutputBackend : public ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      အကယ်၍ (Filter(Path, Config))
        ပြန်ပေး ProxyOutputBackend::createFileImpl(Path, Config);
      ပြန်ပေး std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      ပြန်ပေး makeIntrusiveRefCnt<FilteringOutputBackend>(
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

  ပြန်ပေး makeIntrusiveRefCnt<FilteringOutputBackend>(
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
      ပြန်ပေး joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ပြန်ပေး joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { ပြန်ပေး *this; }

    void write_impl(const var_na *Ptr, size_t Size) override {
      F1->getOS().write(Ptr, Size);
      F2->getOS().write(Ptr, Size);
    }
    void pwrite_impl(const var_na *Ptr, size_t Size, uint64_t Offset) override {
      this->flush();
      F1->getOS().pwrite(Ptr, Size, Offset);
      F2->getOS().pwrite(Ptr, Size, Offset);
    }
    uint64_t current_pos() const override { ပြန်ပေး F1->getOS().tell(); }
    size_t preferred_buffer_size() const override {
      ပြန်ပေး PreferredBufferSize;
    }
    void reserveExtraSpace(uint64_t ExtraSize) override {
      F1->getOS().reserveExtraSpace(ExtraSize);
      F2->getOS().reserveExtraSpace(ExtraSize);
    }
    bool is_displayed() const override {
      ပြန်ပေး F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    bool has_colors() const override {
      ပြန်ပေး F1->getOS().has_colors() && F2->getOS().has_colors();
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
      အကယ်၍ (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        ပြန်ပေး std::move(E);
      အကယ်၍ (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        ပြန်ပေး joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      အကယ်၍ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ပြန်ပေး std::move(File2);
      }
      အကယ်၍ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ပြန်ပေး std::move(File1);
      }
      ပြန်ပေး std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override {
      ပြန်ပေး IntrusiveRefCntPtr<ProxyOutputBackend1>(
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
  ပြန်ပေး IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

static OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              const OnDiskOutputBackend::OutputSettings &Settings) {
  အကယ်၍ (!Config)
    Config = Settings.DefaultConfig;
  အကယ်၍ (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  အကယ်၍ (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  ပြန်ပေး *Config;
}

namespace {
class OnDiskOutputFile final : public OutputFileImpl {
public:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    assert(FileOS && "Expected valid file");
    အကယ်၍ (BufferOS)
      ပြန်ပေး *BufferOS;
    ပြန်ပေး *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<ကိန်းပြည့်> &FD);

  Error initializeFile(std::optional<ကိန်းပြည့်> &FD);
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
  ပြန်ပေး handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    အကယ်၍ (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ပြန်ပေး Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    အကယ်၍ (std::error_code EC = sys::fs::create_directories(ParentPath))
      ပြန်ပေး make_error<OutputError>(ParentPath, EC);
    ပြန်ပေး CreateFile();
  });
}

static sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  အကယ်၍ (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  မဟုတ်ပါက အကယ်၍ (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  အကယ်၍ (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  ပြန်ပေး OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<ကိန်းပြည့်> &FD) {
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

  ပြန်ပေး createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    ကိန်းပြည့် NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    အကယ်၍ (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      ပြန်ပေး make_error<TempFileOutputError>(ModelPath, OutputPath, EC);

    အကယ်၍ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    TempPath = UniquePath.str().str();
    FD.emplace(NewFD);
    ပြန်ပေး Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<ကိန်းပြည့်> &FD) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  assert(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  အကယ်၍ (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    အကယ်၍ (sys::fs::exists(Status)) {
      အကယ်၍ (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      အကယ်၍ (Config.getAppend() && !sys::fs::can_write(OutputPath))
        ပြန်ပေး make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  အကယ်၍ (Config.getAtomicWrite())
    အကယ်၍ (!errorToBool(tryToCreateTemporary(FD)))
      ပြန်ပေး Error::success();

  // Not using a temporary file. Open the final output file.
  ပြန်ပေး createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    ကိန်းပြည့် NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    အကယ်၍ (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      ပြန်ပေး convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    အကယ်၍ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    ပြန်ပေး Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  အကယ်၍ (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    အကယ်၍ (EC)
      ပြန်ပေး make_error<OutputError>(OutputPath, EC);
  } မဟုတ်ပါက {
    std::optional<ကိန်းပြည့်> FD;
    အကယ်၍ (Error E = initializeFile(FD))
      ပြန်ပေး E;
    FileOS.emplace(*FD, /*shouldClose=*/true);
  }

  // Buffer the stream if necessary.
  အကယ်၍ (!FileOS->supportsSeeking() && !Config.getText())
    BufferOS.emplace(*FileOS);

  ပြန်ပေး Error::success();
}

namespace {
class OpenFileRAII {
  static const ကိန်းပြည့် InvalidFd = -1;

public:
  ကိန်းပြည့် Fd = InvalidFd;

  ~OpenFileRAII() {
    အကယ်၍ (Fd != InvalidFd)
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
  အကယ်၍ (sys::fs::equivalent(Source, Destination))
    ပြန်ပေး FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  အကယ်၍ (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    ပြန်ပေး convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  အကယ်၍ (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    ပြန်ပေး convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  အကယ်၍ (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ပြန်ပေး FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  အကယ်၍ (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    ပြန်ပေး FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t Size = SourceStatus.getSize();
  အကယ်၍ (Size != DestStatus.getSize())
    ပြန်ပေး FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  အကယ်၍ (Size == 0)
    ပြန်ပေး FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, SourceRegionErr);
  အကယ်၍ (SourceRegionErr)
    ပြန်ပေး convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, Size, 0, DestRegionErr);

  အကယ်၍ (DestRegionErr)
    ပြန်ပေး FileDifference::DifferentContents;

  အကယ်၍ (memcmp(SourceRegion.const_data(), DestRegion.const_data(), Size) != 0)
    ပြန်ပေး FileDifference::DifferentContents;

  ပြန်ပေး FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  BufferOS.reset();
  အကယ်၍ (!FileOS)
    ပြန်ပေး Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ပြန်ပေး errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  အကယ်၍ (auto E = reset())
    ပြန်ပေး E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    အကယ်၍ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(TempPath ? *TempPath : OutputPath);
  });

  အကယ်၍ (!TempPath)
    ပြန်ပေး Error::success();

  // See if we should append instead of move.
  အကယ်၍ (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    auto Content = MemoryBuffer::getFile(*TempPath);
    အကယ်၍ (!Content)
      ပြန်ပေး convertToTempFileOutputError(*TempPath, OutputPath,
                                          Content.getError());
    နေစဉ် (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      bool Owned;
      အကယ်၍ (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        ပြန်ပေး convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      အကယ်၍ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        အကယ်၍ (EC)
          ပြန်ပေး convertToOutputError(OutputPath, EC);
        Out << (*Content)->getBuffer();
        Out.close();
        Lock.unsafeUnlock();
        အကယ်၍ (Out.has_error())
          ပြန်ပေး convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (void)sys::fs::remove(*TempPath);
        ပြန်ပေး Error::success();
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

  အကယ်၍ (Config.getOnlyIfDifferent()) {
    auto Result = areFilesDifferent(*TempPath, OutputPath);
    အကယ်၍ (!Result)
      ပြန်ပေး Result.takeError();
    switch (*Result) {
    pak_sa FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ပြန်ပေး Error::success();

    pak_sa FileDifference::SameContents:
      // Files are identical; remove the source file.
      (void)sys::fs::remove(*TempPath);
      ပြန်ပေး Error::success();

    pak_sa FileDifference::DifferentContents:
      bha_nga; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::rename(*TempPath, OutputPath);
  အကယ်၍ (!RenameEC)
    ပြန်ပေး Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*TempPath, OutputPath);
  (void)sys::fs::remove(*TempPath);

  အကယ်၍ (!RenameEC)
    ပြန်ပေး Error::success();

  ပြန်ပေး make_error<TempFileOutputError>(*TempPath, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  အကယ်၍ (auto E = reset())
    ပြန်ပေး E;

  // Nothing on the filesystem to remove for stdout.
  အကယ်၍ (OutputPath == "-")
    ပြန်ပေး Error::success();

  auto discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::remove(Path);
    sys::DontRemoveFileOnSignal(Path);
    ပြန်ပေး EC;
  };

  // Clean up the file that's in-progress.
  အကယ်၍ (!TempPath)
    ပြန်ပေး convertToOutputError(OutputPath, discardPath(OutputPath));
  ပြန်ပေး convertToTempFileOutputError(*TempPath, OutputPath,
                                      discardPath(*TempPath));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<var_na> &Path) const {
  // FIXME: Should this really call sys::fs::make_absolute?
  auto BypassSandbox = sys::sandbox::scopedDisable();
  ပြန်ပေး convertToOutputError(StringRef(Path.data(), Path.size()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  အကယ်၍ (Path != "-") {
    AbsPath = Path;
    အကယ်၍ (Error E = makeAbsolute(AbsPath))
      ပြန်ပေး std::move(E);
    Path = AbsPath;
  }

  auto File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  အကယ်၍ (Error E = File->initializeStream())
    ပြန်ပေး std::move(E);

  ပြန်ပေး std::move(File);
}
