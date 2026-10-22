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

#समावेश "llvm/Support/VirtualOutputBackends.h"
#समावेश "llvm/ADT/ScopeExit.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/IOSandbox.h"
#समावेश "llvm/Support/LockFileManager.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/Process.h"
#समावेश "llvm/Support/Signals.h"
#समावेश "llvm/Support/VirtualOutputConfig.h"
#समावेश "llvm/Support/VirtualOutputError.h"

using namespace llvm;
using namespace llvm::vfs;

व्योम ProxyOutputBackend::anchor() {}
व्योम OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  काष्ठा NullOutputBackend : खुला OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() स्थिर override {
      वापस const_cast<NullOutputBackend *>(यह);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path, std::optional<OutputConfig>) override {
      वापस std::make_unique<NullOutputFileImpl>();
    }
  };

  वापस makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<bool(StringRef, std::optional<OutputConfig>)> Filter) {
  काष्ठा FilteringOutputBackend : खुला ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      अगर (Filter(Path, Config))
        वापस ProxyOutputBackend::createFileImpl(Path, Config);
      वापस std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() स्थिर override {
      वापस makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<bool(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      निश्चित(यह->Filter && "Expected a non-null function");
    }
    std::function<bool(StringRef, std::optional<OutputConfig>)> Filter;
  };

  वापस makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  काष्ठा ProxyOutputBackend1 : खुला ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  काष्ठा ProxyOutputBackend2 : खुला ProxyOutputBackend {
    using ProxyOutputBackend::ProxyOutputBackend;
  };
  काष्ठा MirroringOutput final : खुला OutputFileImpl, raw_pwrite_stream {
    Error keep() final {
      सफाई();
      वापस joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      सफाई();
      वापस joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_stream &getOS() final { वापस *यह; }

    व्योम write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) override {
      F1->getOS().ग_लिखो(Ptr, आकार);
      F2->getOS().ग_लिखो(Ptr, आकार);
    }
    व्योम pwrite_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार, uint64_t Offset) override {
      यह->सफाई();
      F1->getOS().pwrite(Ptr, आकार, Offset);
      F2->getOS().pwrite(Ptr, आकार, Offset);
    }
    uint64_t current_pos() स्थिर override { वापस F1->getOS().tell(); }
    माप_प्रकार preferred_बफर_आकार() स्थिर override {
      वापस Preferred_बफर_आकार;
    }
    व्योम reserveExtraSpace(uint64_t Extra_आकार) override {
      F1->getOS().reserveExtraSpace(Extra_आकार);
      F2->getOS().reserveExtraSpace(Extra_आकार);
    }
    bool is_displayed() स्थिर override {
      वापस F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    bool has_colors() स्थिर override {
      वापस F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    व्योम enable_colors(bool enable) override {
      raw_pwrite_stream::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_बफर_आकार(std::max(F1->getOS().Get_बफर_आकार(),
                                       F1->getOS().Get_बफर_आकार())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    माप_प्रकार Preferred_बफर_आकार;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  काष्ठा MirroringOutputBackend : खुला ProxyOutputBackend1,
                                  खुला ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef Path,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      अगर (Error E =
              ProxyOutputBackend1::createFileImpl(Path, Config).moveInto(File1))
        वापस std::move(E);
      अगर (Error E =
              ProxyOutputBackend2::createFileImpl(Path, Config).moveInto(File2))
        वापस joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      अगर (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        वापस std::move(File2);
      }
      अगर (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        वापस std::move(File1);
      }
      वापस std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() स्थिर override {
      वापस IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    व्योम Retain() स्थिर { ProxyOutputBackend1::Retain(); }
    व्योम Release() स्थिर { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  निश्चित(Backend1 && "Expected actual backend");
  निश्चित(Backend2 && "Expected actual backend");
  वापस IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

अटल OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              स्थिर OnDiskOutputBackend::OutputSettings &Settings) {
  अगर (!Config)
    Config = Settings.DefaultConfig;
  अगर (!Settings.UseTemporaries)
    Config->setNoAtomicWrite();
  अगर (!Settings.RemoveOnSignal)
    Config->setNoDiscardOnSignal();
  वापस *Config;
}

namespace {
श्रेणी OnDiskOutputFile final : खुला OutputFileImpl {
खुला:
  Error keep() override;
  Error discard() override;
  raw_pwrite_stream &getOS() override {
    निश्चित(FileOS && "Expected valid file");
    अगर (बफर_OS)
      वापस *बफर_OS;
    वापस *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error tryToCreateTemporary(std::optional<पूर्णांक> &FD);

  Error initializeFile(std::optional<पूर्णांक> &FD);
  Error initializeStream();
  Error reset();

  OnDiskOutputFile(StringRef OutputPath, std::optional<OutputConfig> Config,
                   स्थिर OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        OutputPath(OutputPath.str()) {}

  OutputConfig Config;
  स्थिर std::string OutputPath;
  std::optional<std::string> अस्थायी_Path;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<बफर_ostream> बफर_OS;
};
} // end namespace

अटल Error createDirectoriesOnDemand(StringRef OutputPath,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  वापस handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    अगर (EC->convertToErrorCode() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      वापस Error(std::move(EC));

    StringRef ParentPath = sys::path::parent_path(OutputPath);
    अगर (std::error_code EC = sys::fs::create_directories(ParentPath))
      वापस make_error<OutputError>(ParentPath, EC);
    वापस CreateFile();
  });
}

अटल sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags OF = sys::fs::OF_None;
  अगर (Config.getTextWithCRLF())
    OF |= sys::fs::OF_TextWithCRLF;
  अन्यथा अगर (Config.getText())
    OF |= sys::fs::OF_Text;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  अगर (Config.getAppend() && !Config.getAtomicWrite())
    OF |= sys::fs::OF_Append;

  वापस OF;
}

Error OnDiskOutputFile::tryToCreateTemporary(std::optional<पूर्णांक> &FD) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::path::extension(OutputPath);
  SmallString<128> ModelPath =
      StringRef(OutputPath).drop_back(OutputExtension.आकार_2());
  ModelPath += "-%%%%%%%%";
  ModelPath += OutputExtension;
  ModelPath += ".tmp";

  वापस createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    पूर्णांक NewFD;
    SmallString<128> UniquePath;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    अगर (std::error_code EC =
            sys::fs::createUniqueFile(ModelPath, NewFD, UniquePath, OF))
      वापस make_error<अस्थायी_File_Output_Error>(ModelPath, OutputPath, EC);

    अगर (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(UniquePath);

    अस्थायी_Path = UniquePath.str().str();
    FD.emplace(NewFD);
    वापस Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<पूर्णांक> &FD) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  निश्चित(OutputPath != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  अगर (Config.getAtomicWrite()) {
    sys::fs::file_status Status;
    sys::fs::status(OutputPath, Status);
    अगर (sys::fs::exists(Status)) {
      अगर (!sys::fs::is_regular_file(Status))
        Config.setNoAtomicWrite();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      अगर (Config.getAppend() && !sys::fs::can_write(OutputPath))
        वापस make_error<OutputError>(
            OutputPath,
            std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  अगर (Config.getAtomicWrite())
    अगर (!errorToBool(tryToCreateTemporary(FD)))
      वापस Error::success();

  // Not using a temporary file. Open the final output file.
  वापस createDirectoriesOnDemand(OutputPath, Config, [&]() -> Error {
    पूर्णांक NewFD;
    sys::fs::OpenFlags OF = generateFlagsFromConfig(Config);
    अगर (std::error_code EC = sys::fs::openFileForWrite(
            OutputPath, NewFD, sys::fs::CD_CreateAlways, OF))
      वापस convertToOutputError(OutputPath, EC);
    FD.emplace(NewFD);

    अगर (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(OutputPath);
    वापस Error::success();
  });
}

Error OnDiskOutputFile::initializeStream() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  अगर (OutputPath == "-") {
    std::error_code EC;
    FileOS.emplace(OutputPath, EC);
    अगर (EC)
      वापस make_error<OutputError>(OutputPath, EC);
  } अन्यथा {
    std::optional<पूर्णांक> FD;
    अगर (Error E = initializeFile(FD))
      वापस E;
    FileOS.emplace(*FD, /*shouldClose=*/true);
  }

  // Buffer the stream if necessary.
  अगर (!FileOS->supportsSeeking() && !Config.getText())
    बफर_OS.emplace(*FileOS);

  वापस Error::success();
}

namespace {
श्रेणी OpenFileRAII {
  अटल स्थिर पूर्णांक InvalidFd = -1;

खुला:
  पूर्णांक Fd = InvalidFd;

  ~OpenFileRAII() {
    अगर (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

क्रमागत श्रेणी FileDifference : uint8_t {
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

अटल Expected<FileDifference>
areFilesDifferent(स्थिर llvm::Twine &Source, स्थिर llvm::Twine &Destination) {
  अगर (sys::fs::equivalent(Source, Destination))
    वापस FileDifference::IdenticalFile;

  OpenFileRAII SourceFile;
  sys::fs::file_status SourceStatus;
  // If we can't open the source file, fail.
  अगर (std::error_code EC = sys::fs::openFileForRead(Source, SourceFile.Fd))
    वापस convertToOutputError(Source, EC);

  // If we can't stat the source file, fail.
  अगर (std::error_code EC = sys::fs::status(SourceFile.Fd, SourceStatus))
    वापस convertToOutputError(Source, EC);

  OpenFileRAII DestFile;
  sys::fs::file_status DestStatus;
  // If we can't open the destination file, report different.
  अगर (std::error_code Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    वापस FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  अगर (std::error_code Error = sys::fs::status(DestFile.Fd, DestStatus))
    वापस FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t आकार = SourceStatus.get_आकार();
  अगर (आकार != DestStatus.get_आकार())
    वापस FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  अगर (आकार == 0)
    वापस FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code SourceRegionErr;
  sys::fs::mapped_file_region SourceRegion(
      sys::fs::convertFDToNativeFile(SourceFile.Fd),
      sys::fs::mapped_file_region::readonly, आकार, 0, SourceRegionErr);
  अगर (SourceRegionErr)
    वापस convertToOutputError(Source, SourceRegionErr);

  std::error_code DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, आकार, 0, DestRegionErr);

  अगर (DestRegionErr)
    वापस FileDifference::DifferentContents;

  अगर (स_भेद(SourceRegion.const_तथ्य(), DestRegion.const_तथ्य(), आकार) != 0)
    वापस FileDifference::DifferentContents;

  वापस FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  बफर_OS.reset();
  अगर (!FileOS)
    वापस Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_code EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  वापस errorCodeToError(EC);
}

Error OnDiskOutputFile::keep() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  अगर (स्वतः E = reset())
    वापस E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    अगर (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(अस्थायी_Path ? *अस्थायी_Path : OutputPath);
  });

  अगर (!अस्थायी_Path)
    वापस Error::success();

  // See if we should append instead of move.
  अगर (Config.getAppend() && OutputPath != "-") {
    // Read TempFile for the content to append.
    स्वतः Content = Memory_बफर::getFile(*अस्थायी_Path);
    अगर (!Content)
      वापस convert_To_अस्थायी_File_Output_Error(*अस्थायी_Path, OutputPath,
                                          Content.getError());
    जबतक (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::LockFileManager Lock(OutputPath);
      bool Owned;
      अगर (Error Err = Lock.tryLock().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        Lock.unsafeUnlock();
        वापस convertToOutputError(
            OutputPath, std::make_error_code(std::errc::no_lock_available));
      }
      अगर (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_code EC;
        llvm::raw_fd_ostream Out(OutputPath, EC, llvm::sys::fs::OF_Append);
        अगर (EC)
          वापस convertToOutputError(OutputPath, EC);
        Out << (*Content)->get_बफर();
        Out.बंद();
        Lock.unsafeUnlock();
        अगर (Out.has_error())
          वापस convertToOutputError(OutputPath, Out.error());
        // Remove temp file and done.
        (व्योम)sys::fs::हटाओ(*अस्थायी_Path);
        वापस Error::success();
      }
      // Someone else owns the lock on this file, wait.
      चयन (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
      हाल Wait_For_Unlock_परिणाम::Success:
        [[fallthrough]];
      हाल Wait_For_Unlock_परिणाम::OwnerDied: {
        जारी; // try again to get the lock.
      }
      हाल Wait_For_Unlock_परिणाम::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        Lock.unsafeUnlock();
        जारी;
      }
      }
      अवरोध;
    }
  }

  अगर (Config.getOnlyIfDifferent()) {
    स्वतः परिणाम = areFilesDifferent(*अस्थायी_Path, OutputPath);
    अगर (!परिणाम)
      वापस परिणाम.takeError();
    चयन (*परिणाम) {
    हाल FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      वापस Error::success();

    हाल FileDifference::SameContents:
      // Files are identical; remove the source file.
      (व्योम)sys::fs::हटाओ(*अस्थायी_Path);
      वापस Error::success();

    हाल FileDifference::DifferentContents:
      अवरोध; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_code RenameEC = sys::fs::नाम(*अस्थायी_Path, OutputPath);
  अगर (!RenameEC)
    वापस Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*अस्थायी_Path, OutputPath);
  (व्योम)sys::fs::हटाओ(*अस्थायी_Path);

  अगर (!RenameEC)
    वापस Error::success();

  वापस make_error<अस्थायी_File_Output_Error>(*अस्थायी_Path, OutputPath, RenameEC);
}

Error OnDiskOutputFile::discard() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  अगर (स्वतः E = reset())
    वापस E;

  // Nothing on the filesystem to remove for stdout.
  अगर (OutputPath == "-")
    वापस Error::success();

  स्वतः discardPath = [&](StringRef Path) {
    std::error_code EC = sys::fs::हटाओ(Path);
    sys::DontRemoveFileOnSignal(Path);
    वापस EC;
  };

  // Clean up the file that's in-progress.
  अगर (!अस्थायी_Path)
    वापस convertToOutputError(OutputPath, discardPath(OutputPath));
  वापस convert_To_अस्थायी_File_Output_Error(*अस्थायी_Path, OutputPath,
                                      discardPath(*अस्थायी_Path));
}

Error OnDiskOutputBackend::makeAbsolute(SmallVectorImpl<अक्षर> &Path) स्थिर {
  // FIXME: Should this really call sys::fs::make_absolute?
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();
  वापस convertToOutputError(StringRef(Path.तथ्य(), Path.आकार_2()),
                              sys::fs::make_absolute(Path));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef Path,
                                    std::optional<OutputConfig> Config) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> AbsPath;
  अगर (Path != "-") {
    AbsPath = Path;
    अगर (Error E = makeAbsolute(AbsPath))
      वापस std::move(E);
    Path = AbsPath;
  }

  स्वतः File = std::make_unique<OnDiskOutputFile>(Path, Config, Settings);
  अगर (Error E = File->initializeStream())
    वापस std::move(E);

  वापस std::move(File);
}
