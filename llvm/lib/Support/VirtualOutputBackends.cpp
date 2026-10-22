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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputBackends.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ScopeExit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/IOSandbox.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LockFileManager.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputConfig.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputError.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::vfs;

ਖਾਲੀ ProxyOutputBackend::anchor() {}
ਖਾਲੀ OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  ਬਣਤਰ NullOutputBackend : ਜਨਤਕ OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ਅਚਲ override {
      ਵਾਪਸ const_cast<NullOutputBackend *>(ਇਹ);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef ਰਸਤਾ, std::optional<OutputConfig>) override {
      ਵਾਪਸ std::make_unique<NullOutputFileImpl>();
    }
  };

  ਵਾਪਸ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<ਤਾਰਕਿਕ(StringRef, std::optional<OutputConfig>)> Filter) {
  ਬਣਤਰ FilteringOutputBackend : ਜਨਤਕ ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef ਰਸਤਾ,
                   std::optional<OutputConfig> Config) override {
      ਜੇ (Filter(ਰਸਤਾ, Config))
        ਵਾਪਸ ProxyOutputBackend::createFileImpl(ਰਸਤਾ, Config);
      ਵਾਪਸ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ਅਚਲ override {
      ਵਾਪਸ makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<ਤਾਰਕਿਕ(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      ਪ੍ਰਤਿਜ੍ਞਾ(ਇਹ->Filter && "Expected a non-null function");
    }
    std::function<ਤਾਰਕਿਕ(StringRef, std::optional<OutputConfig>)> Filter;
  };

  ਵਾਪਸ makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  ਬਣਤਰ ProxyOutputBackend1 : ਜਨਤਕ ProxyOutputBackend {
    ਵਰਤੋਂ ProxyOutputBackend::ProxyOutputBackend;
  };
  ਬਣਤਰ ProxyOutputBackend2 : ਜਨਤਕ ProxyOutputBackend {
    ਵਰਤੋਂ ProxyOutputBackend::ProxyOutputBackend;
  };
  ਬਣਤਰ MirroringOutput final : ਜਨਤਕ OutputFileImpl, raw_pwrite_ਧਾਰਾ {
    Error keep() final {
      flush();
      ਵਾਪਸ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ਵਾਪਸ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_ਧਾਰਾ &getOS() final { ਵਾਪਸ *ਇਹ; }

    ਖਾਲੀ write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) override {
      F1->getOS().ਲਿਖ(Ptr, ਮਾਪ);
      F2->getOS().ਲਿਖ(Ptr, ਮਾਪ);
    }
    ਖਾਲੀ pwrite_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ, uint64_t ਵਿੱਥ) override {
      ਇਹ->flush();
      F1->getOS().pwrite(Ptr, ਮਾਪ, ਵਿੱਥ);
      F2->getOS().pwrite(Ptr, ਮਾਪ, ਵਿੱਥ);
    }
    uint64_t current_pos() ਅਚਲ override { ਵਾਪਸ F1->getOS().tell(); }
    size_t preferred_ਬਫਰ_ਮਾਪ() ਅਚਲ override {
      ਵਾਪਸ Preferred_ਬਫਰ_ਮਾਪ;
    }
    ਖਾਲੀ reserveExtraSpace(uint64_t Extra_ਮਾਪ) override {
      F1->getOS().reserveExtraSpace(Extra_ਮਾਪ);
      F2->getOS().reserveExtraSpace(Extra_ਮਾਪ);
    }
    ਤਾਰਕਿਕ is_displayed() ਅਚਲ override {
      ਵਾਪਸ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    ਤਾਰਕਿਕ has_colors() ਅਚਲ override {
      ਵਾਪਸ F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    ਖਾਲੀ enable_colors(ਤਾਰਕਿਕ enable) override {
      raw_pwrite_ਧਾਰਾ::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_ਬਫਰ_ਮਾਪ(std::max(F1->getOS().Get_ਬਫਰ_ਮਾਪ(),
                                       F1->getOS().Get_ਬਫਰ_ਮਾਪ())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_ਬਫਰ_ਮਾਪ;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  ਬਣਤਰ MirroringOutputBackend : ਜਨਤਕ ProxyOutputBackend1,
                                  ਜਨਤਕ ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef ਰਸਤਾ,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      ਜੇ (Error E =
              ProxyOutputBackend1::createFileImpl(ਰਸਤਾ, Config).moveInto(File1))
        ਵਾਪਸ std::move(E);
      ਜੇ (Error E =
              ProxyOutputBackend2::createFileImpl(ਰਸਤਾ, Config).moveInto(File2))
        ਵਾਪਸ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      ਜੇ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ਵਾਪਸ std::move(File2);
      }
      ਜੇ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ਵਾਪਸ std::move(File1);
      }
      ਵਾਪਸ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ਅਚਲ override {
      ਵਾਪਸ IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    ਖਾਲੀ Retain() ਅਚਲ { ProxyOutputBackend1::Retain(); }
    ਖਾਲੀ Release() ਅਚਲ { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  ਪ੍ਰਤਿਜ੍ਞਾ(Backend1 && "Expected actual backend");
  ਪ੍ਰਤਿਜ੍ਞਾ(Backend2 && "Expected actual backend");
  ਵਾਪਸ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ਸਥਿਰ OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ਅਚਲ OnDiskOutputBackend::OutputSettings &Settings) {
  ਜੇ (!Config)
    Config = Settings.DefaultConfig;
  ਜੇ (!Settings.UseTemporaries)
    Config->ਸੈੱਟ_No_Atomic_Write();
  ਜੇ (!Settings.RemoveOnSignal)
    Config->ਸੈੱਟ_No_Discard_On_Signal();
  ਵਾਪਸ *Config;
}

ਨਾਮਾਕਾਸ਼ {
ਵਰਗ OnDiskOutputFile final : ਜਨਤਕ OutputFileImpl {
ਜਨਤਕ:
  Error keep() override;
  Error discard() override;
  raw_pwrite_ਧਾਰਾ &getOS() override {
    ਪ੍ਰਤਿਜ੍ਞਾ(FileOS && "Expected valid file");
    ਜੇ (ਬਫਰ_OS)
      ਵਾਪਸ *ਬਫਰ_OS;
    ਵਾਪਸ *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error try_ਮੰਜ਼ਿਲ_Create_Temporary(std::optional<ਪੂਰਨ> &FD);

  Error initializeFile(std::optional<ਪੂਰਨ> &FD);
  Error initialize_ਧਾਰਾ();
  Error reset();

  OnDiskOutputFile(StringRef Output_ਰਸਤਾ, std::optional<OutputConfig> Config,
                   ਅਚਲ OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        Output_ਰਸਤਾ(Output_ਰਸਤਾ.str()) {}

  OutputConfig Config;
  ਅਚਲ std::string Output_ਰਸਤਾ;
  std::optional<std::string> ਆਰਜ਼ੀ_ਰਸਤਾ;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<ਬਫਰ_ostream> ਬਫਰ_OS;
};
} // end namespace

ਸਥਿਰ Error createDirectoriesOnDemand(StringRef Output_ਰਸਤਾ,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  ਵਾਪਸ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    ਜੇ (EC->convert_ਮੰਜ਼ਿਲ_Error_ਕੋਡ() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ਵਾਪਸ Error(std::move(EC));

    StringRef ਮਾਪਾ_ਰਸਤਾ_2 = sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(Output_ਰਸਤਾ);
    ਜੇ (std::error_ਕੋਡ EC = sys::fs::create_directories(ਮਾਪਾ_ਰਸਤਾ_2))
      ਵਾਪਸ make_error<OutputError>(ਮਾਪਾ_ਰਸਤਾ_2, EC);
    ਵਾਪਸ CreateFile();
  });
}

ਸਥਿਰ sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags ਦਾ_2 = sys::fs::ਦਾ_None;
  ਜੇ (Config.get_ਪਾਠ_With_CRLF())
    ਦਾ_2 |= sys::fs::ਦਾ_ਪਾਠ_With_CRLF;
  ਨਹੀਂਤਾਂ ਜੇ (Config.get_ਪਾਠ())
    ਦਾ_2 |= sys::fs::ਦਾ_ਪਾਠ;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  ਜੇ (Config.getAppend() && !Config.getAtomicWrite())
    ਦਾ_2 |= sys::fs::ਦਾ_Append;

  ਵਾਪਸ ਦਾ_2;
}

Error OnDiskOutputFile::try_ਮੰਜ਼ਿਲ_Create_Temporary(std::optional<ਪੂਰਨ> &FD) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::ਰਸਤਾ_2::extension(Output_ਰਸਤਾ);
  SmallString<128> Model_ਰਸਤਾ =
      StringRef(Output_ਰਸਤਾ).drop_ਪਿੱਠ(OutputExtension.ਮਾਪ_2());
  Model_ਰਸਤਾ += "-%%%%%%%%";
  Model_ਰਸਤਾ += OutputExtension;
  Model_ਰਸਤਾ += ".tmp";

  ਵਾਪਸ createDirectoriesOnDemand(Output_ਰਸਤਾ, Config, [&]() -> Error {
    ਪੂਰਨ NewFD;
    SmallString<128> Unique_ਰਸਤਾ;
    sys::fs::OpenFlags ਦਾ_2 = generateFlagsFromConfig(Config);
    ਜੇ (std::error_ਕੋਡ EC =
            sys::fs::createUniqueFile(Model_ਰਸਤਾ, NewFD, Unique_ਰਸਤਾ, ਦਾ_2))
      ਵਾਪਸ make_error<ਆਰਜ਼ੀ_File_Output_Error>(Model_ਰਸਤਾ, Output_ਰਸਤਾ, EC);

    ਜੇ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Unique_ਰਸਤਾ);

    ਆਰਜ਼ੀ_ਰਸਤਾ = Unique_ਰਸਤਾ.str().str();
    FD.emplace(NewFD);
    ਵਾਪਸ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<ਪੂਰਨ> &FD) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਪ੍ਰਤਿਜ੍ਞਾ(Output_ਰਸਤਾ != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  ਜੇ (Config.getAtomicWrite()) {
    sys::fs::file_ਸਥਿਤੀ ਸਥਿਤੀ;
    sys::fs::ਸਥਿਤੀ_2(Output_ਰਸਤਾ, ਸਥਿਤੀ);
    ਜੇ (sys::fs::exists(ਸਥਿਤੀ)) {
      ਜੇ (!sys::fs::is_regular_file(ਸਥਿਤੀ))
        Config.ਸੈੱਟ_No_Atomic_Write();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      ਜੇ (Config.getAppend() && !sys::fs::can_write(Output_ਰਸਤਾ))
        ਵਾਪਸ make_error<OutputError>(
            Output_ਰਸਤਾ,
            std::make_error_ਕੋਡ(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  ਜੇ (Config.getAtomicWrite())
    ਜੇ (!error_ਮੰਜ਼ਿਲ_Bool(try_ਮੰਜ਼ਿਲ_Create_Temporary(FD)))
      ਵਾਪਸ Error::success();

  // Not using a temporary file. Open the final output file.
  ਵਾਪਸ createDirectoriesOnDemand(Output_ਰਸਤਾ, Config, [&]() -> Error {
    ਪੂਰਨ NewFD;
    sys::fs::OpenFlags ਦਾ_2 = generateFlagsFromConfig(Config);
    ਜੇ (std::error_ਕੋਡ EC = sys::fs::openFileForWrite(
            Output_ਰਸਤਾ, NewFD, sys::fs::CD_CreateAlways, ਦਾ_2))
      ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(Output_ਰਸਤਾ, EC);
    FD.emplace(NewFD);

    ਜੇ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Output_ਰਸਤਾ);
    ਵਾਪਸ Error::success();
  });
}

Error OnDiskOutputFile::initialize_ਧਾਰਾ() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  ਜੇ (Output_ਰਸਤਾ == "-") {
    std::error_ਕੋਡ EC;
    FileOS.emplace(Output_ਰਸਤਾ, EC);
    ਜੇ (EC)
      ਵਾਪਸ make_error<OutputError>(Output_ਰਸਤਾ, EC);
  } ਨਹੀਂਤਾਂ {
    std::optional<ਪੂਰਨ> FD;
    ਜੇ (Error E = initializeFile(FD))
      ਵਾਪਸ E;
    FileOS.emplace(*FD, /*shouldClose=*/ਸੱਚ);
  }

  // Buffer the stream if necessary.
  ਜੇ (!FileOS->supportsSeeking() && !Config.get_ਪਾਠ())
    ਬਫਰ_OS.emplace(*FileOS);

  ਵਾਪਸ Error::success();
}

ਨਾਮਾਕਾਸ਼ {
ਵਰਗ OpenFileRAII {
  ਸਥਿਰ ਅਚਲ ਪੂਰਨ InvalidFd = -1;

ਜਨਤਕ:
  ਪੂਰਨ Fd = InvalidFd;

  ~OpenFileRAII() {
    ਜੇ (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

ਗਿਣਤੀ ਵਰਗ FileDifference : uint8_t {
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

ਸਥਿਰ Expected<FileDifference>
areFilesDifferent(ਅਚਲ llvm::Twine &ਸਰੋਤ, ਅਚਲ llvm::Twine &Destination) {
  ਜੇ (sys::fs::equivalent(ਸਰੋਤ, Destination))
    ਵਾਪਸ FileDifference::IdenticalFile;

  OpenFileRAII ਸਰੋਤ_File;
  sys::fs::file_ਸਥਿਤੀ ਸਰੋਤ_ਸਥਿਤੀ;
  // If we can't open the source file, fail.
  ਜੇ (std::error_ਕੋਡ EC = sys::fs::openFileForRead(ਸਰੋਤ, ਸਰੋਤ_File.Fd))
    ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(ਸਰੋਤ, EC);

  // If we can't stat the source file, fail.
  ਜੇ (std::error_ਕੋਡ EC = sys::fs::ਸਥਿਤੀ_2(ਸਰੋਤ_File.Fd, ਸਰੋਤ_ਸਥਿਤੀ))
    ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(ਸਰੋਤ, EC);

  OpenFileRAII DestFile;
  sys::fs::file_ਸਥਿਤੀ Dest_ਸਥਿਤੀ;
  // If we can't open the destination file, report different.
  ਜੇ (std::error_ਕੋਡ Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ਵਾਪਸ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  ਜੇ (std::error_ਕੋਡ Error = sys::fs::ਸਥਿਤੀ_2(DestFile.Fd, Dest_ਸਥਿਤੀ))
    ਵਾਪਸ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t ਮਾਪ = ਸਰੋਤ_ਸਥਿਤੀ.get_ਮਾਪ();
  ਜੇ (ਮਾਪ != Dest_ਸਥਿਤੀ.get_ਮਾਪ())
    ਵਾਪਸ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  ਜੇ (ਮਾਪ == 0)
    ਵਾਪਸ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_ਕੋਡ ਸਰੋਤ_Region_Err;
  sys::fs::mapped_file_region ਸਰੋਤ_Region(
      sys::fs::convertFDToNativeFile(ਸਰੋਤ_File.Fd),
      sys::fs::mapped_file_region::readonly, ਮਾਪ, 0, ਸਰੋਤ_Region_Err);
  ਜੇ (ਸਰੋਤ_Region_Err)
    ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(ਸਰੋਤ, ਸਰੋਤ_Region_Err);

  std::error_ਕੋਡ DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, ਮਾਪ, 0, DestRegionErr);

  ਜੇ (DestRegionErr)
    ਵਾਪਸ FileDifference::DifferentContents;

  ਜੇ (memcmp(ਸਰੋਤ_Region.const_ਅੰਕੜੇ(), DestRegion.const_ਅੰਕੜੇ(), ਮਾਪ) != 0)
    ਵਾਪਸ FileDifference::DifferentContents;

  ਵਾਪਸ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ਬਫਰ_OS.reset();
  ਜੇ (!FileOS)
    ਵਾਪਸ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_ਕੋਡ EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
}

Error OnDiskOutputFile::keep() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਜੇ (ਸਵੈਚਾਲਿਤ E = reset())
    ਵਾਪਸ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    ਜੇ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(ਆਰਜ਼ੀ_ਰਸਤਾ ? *ਆਰਜ਼ੀ_ਰਸਤਾ : Output_ਰਸਤਾ);
  });

  ਜੇ (!ਆਰਜ਼ੀ_ਰਸਤਾ)
    ਵਾਪਸ Error::success();

  // See if we should append instead of move.
  ਜੇ (Config.getAppend() && Output_ਰਸਤਾ != "-") {
    // Read TempFile for the content to append.
    ਸਵੈਚਾਲਿਤ Content = ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(*ਆਰਜ਼ੀ_ਰਸਤਾ);
    ਜੇ (!Content)
      ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_ਆਰਜ਼ੀ_File_Output_Error(*ਆਰਜ਼ੀ_ਰਸਤਾ, Output_ਰਸਤਾ,
                                          Content.getError());
    ਜਦੋਂ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::ਤਾਲਾ_File_Manager ਤਾਲਾ_2(Output_ਰਸਤਾ);
      ਤਾਰਕਿਕ Owned;
      ਜੇ (Error Err = ਤਾਲਾ_2.try_ਤਾਲਾ().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        ਤਾਲਾ_2.unsafeUnlock();
        ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(
            Output_ਰਸਤਾ, std::make_error_ਕੋਡ(std::errc::no_ਤਾਲਾ_ਉਪਲਬਧ));
      }
      ਜੇ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_ਕੋਡ EC;
        llvm::raw_fd_ostream ਬਾਹਰਲਾ(Output_ਰਸਤਾ, EC, llvm::sys::fs::ਦਾ_Append);
        ਜੇ (EC)
          ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(Output_ਰਸਤਾ, EC);
        ਬਾਹਰਲਾ << (*Content)->get_ਬਫਰ();
        ਬਾਹਰਲਾ.close();
        ਤਾਲਾ_2.unsafeUnlock();
        ਜੇ (ਬਾਹਰਲਾ.has_error())
          ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(Output_ਰਸਤਾ, ਬਾਹਰਲਾ.error());
        // Remove temp file and done.
        (ਖਾਲੀ)sys::fs::remove(*ਆਰਜ਼ੀ_ਰਸਤਾ);
        ਵਾਪਸ Error::success();
      }
      // Someone else owns the lock on this file, wait.
      ਚੋਣ (ਤਾਲਾ_2.waitForUnlockFor(std::chrono::seconds(256))) {
      ਹਾਲਤ Wait_For_Unlock_ਨਤੀਜਾ::Success:
        [[fallthrough]];
      ਹਾਲਤ Wait_For_Unlock_ਨਤੀਜਾ::OwnerDied: {
        ਜਾਰੀ; // try again to get the lock.
      }
      ਹਾਲਤ Wait_For_Unlock_ਨਤੀਜਾ::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        ਤਾਲਾ_2.unsafeUnlock();
        ਜਾਰੀ;
      }
      }
      ਰੋਕੋ;
    }
  }

  ਜੇ (Config.getOnlyIfDifferent()) {
    ਸਵੈਚਾਲਿਤ ਨਤੀਜਾ = areFilesDifferent(*ਆਰਜ਼ੀ_ਰਸਤਾ, Output_ਰਸਤਾ);
    ਜੇ (!ਨਤੀਜਾ)
      ਵਾਪਸ ਨਤੀਜਾ.takeError();
    ਚੋਣ (*ਨਤੀਜਾ) {
    ਹਾਲਤ FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ਵਾਪਸ Error::success();

    ਹਾਲਤ FileDifference::SameContents:
      // Files are identical; remove the source file.
      (ਖਾਲੀ)sys::fs::remove(*ਆਰਜ਼ੀ_ਰਸਤਾ);
      ਵਾਪਸ Error::success();

    ਹਾਲਤ FileDifference::DifferentContents:
      ਰੋਕੋ; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_ਕੋਡ RenameEC = sys::fs::rename(*ਆਰਜ਼ੀ_ਰਸਤਾ, Output_ਰਸਤਾ);
  ਜੇ (!RenameEC)
    ਵਾਪਸ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*ਆਰਜ਼ੀ_ਰਸਤਾ, Output_ਰਸਤਾ);
  (ਖਾਲੀ)sys::fs::remove(*ਆਰਜ਼ੀ_ਰਸਤਾ);

  ਜੇ (!RenameEC)
    ਵਾਪਸ Error::success();

  ਵਾਪਸ make_error<ਆਰਜ਼ੀ_File_Output_Error>(*ਆਰਜ਼ੀ_ਰਸਤਾ, Output_ਰਸਤਾ, RenameEC);
}

Error OnDiskOutputFile::discard() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ਜੇ (ਸਵੈਚਾਲਿਤ E = reset())
    ਵਾਪਸ E;

  // Nothing on the filesystem to remove for stdout.
  ਜੇ (Output_ਰਸਤਾ == "-")
    ਵਾਪਸ Error::success();

  ਸਵੈਚਾਲਿਤ discard_ਰਸਤਾ = [&](StringRef ਰਸਤਾ) {
    std::error_ਕੋਡ EC = sys::fs::remove(ਰਸਤਾ);
    sys::DontRemoveFileOnSignal(ਰਸਤਾ);
    ਵਾਪਸ EC;
  };

  // Clean up the file that's in-progress.
  ਜੇ (!ਆਰਜ਼ੀ_ਰਸਤਾ)
    ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(Output_ਰਸਤਾ, discard_ਰਸਤਾ(Output_ਰਸਤਾ));
  ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_ਆਰਜ਼ੀ_File_Output_Error(*ਆਰਜ਼ੀ_ਰਸਤਾ, Output_ਰਸਤਾ,
                                      discard_ਰਸਤਾ(*ਆਰਜ਼ੀ_ਰਸਤਾ));
}

Error OnDiskOutputBackend::makeAbsolute(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ) ਅਚਲ {
  // FIXME: Should this really call sys::fs::make_absolute?
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();
  ਵਾਪਸ convert_ਮੰਜ਼ਿਲ_Output_Error(StringRef(ਰਸਤਾ.ਅੰਕੜੇ(), ਰਸਤਾ.ਮਾਪ_2()),
                              sys::fs::make_absolute(ਰਸਤਾ));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef ਰਸਤਾ,
                                    std::optional<OutputConfig> Config) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Abs_ਰਸਤਾ;
  ਜੇ (ਰਸਤਾ != "-") {
    Abs_ਰਸਤਾ = ਰਸਤਾ;
    ਜੇ (Error E = makeAbsolute(Abs_ਰਸਤਾ))
      ਵਾਪਸ std::move(E);
    ਰਸਤਾ = Abs_ਰਸਤਾ;
  }

  ਸਵੈਚਾਲਿਤ File = std::make_unique<OnDiskOutputFile>(ਰਸਤਾ, Config, Settings);
  ਜੇ (Error E = File->initialize_ਧਾਰਾ())
    ਵਾਪਸ std::move(E);

  ਵਾਪਸ std::move(File);
}
