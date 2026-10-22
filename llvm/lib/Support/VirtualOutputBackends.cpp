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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::vfs;

ಶೂನ್ಯ ProxyOutputBackend::anchor() {}
ಶೂನ್ಯ OnDiskOutputBackend::anchor() {}

IntrusiveRefCntPtr<OutputBackend> vfs::makeNullOutputBackend() {
  ರಚನೆ NullOutputBackend : ಸಾರ್ವಜನಿಕ OutputBackend {
    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ಸ್ಥಿರಾಂಕ override {
      ಹಿಂತಿರುಗಿ const_cast<NullOutputBackend *>(ಇದು);
    }
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef ಮಾರ್ಗ, std::optional<OutputConfig>) override {
      ಹಿಂತಿರುಗಿ std::make_unique<NullOutputFileImpl>();
    }
  };

  ಹಿಂತಿರುಗಿ makeIntrusiveRefCnt<NullOutputBackend>();
}

IntrusiveRefCntPtr<OutputBackend> vfs::makeFilteringOutputBackend(
    IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
    std::function<ತಾರ್ಕಿಕ(StringRef, std::optional<OutputConfig>)> Filter) {
  ರಚನೆ FilteringOutputBackend : ಸಾರ್ವಜನಿಕ ProxyOutputBackend {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef ಮಾರ್ಗ,
                   std::optional<OutputConfig> Config) override {
      ಆದರೆ (Filter(ಮಾರ್ಗ, Config))
        ಹಿಂತಿರುಗಿ ProxyOutputBackend::createFileImpl(ಮಾರ್ಗ, Config);
      ಹಿಂತಿರುಗಿ std::make_unique<NullOutputFileImpl>();
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ಸ್ಥಿರಾಂಕ override {
      ಹಿಂತಿರುಗಿ makeIntrusiveRefCnt<FilteringOutputBackend>(
          getUnderlyingBackend().clone(), Filter);
    }

    FilteringOutputBackend(
        IntrusiveRefCntPtr<OutputBackend> UnderlyingBackend,
        std::function<ತಾರ್ಕಿಕ(StringRef, std::optional<OutputConfig>)> Filter)
        : ProxyOutputBackend(std::move(UnderlyingBackend)),
          Filter(std::move(Filter)) {
      ಪ್ರತಿಜ್ಞಾ(ಇದು->Filter && "Expected a non-null function");
    }
    std::function<ತಾರ್ಕಿಕ(StringRef, std::optional<OutputConfig>)> Filter;
  };

  ಹಿಂತಿರುಗಿ makeIntrusiveRefCnt<FilteringOutputBackend>(
      std::move(UnderlyingBackend), std::move(Filter));
}

IntrusiveRefCntPtr<OutputBackend>
vfs::makeMirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                                IntrusiveRefCntPtr<OutputBackend> Backend2) {
  ರಚನೆ ProxyOutputBackend1 : ಸಾರ್ವಜನಿಕ ProxyOutputBackend {
    ಬಳಸಿ ProxyOutputBackend::ProxyOutputBackend;
  };
  ರಚನೆ ProxyOutputBackend2 : ಸಾರ್ವಜನಿಕ ProxyOutputBackend {
    ಬಳಸಿ ProxyOutputBackend::ProxyOutputBackend;
  };
  ರಚನೆ MirroringOutput final : ಸಾರ್ವಜನಿಕ OutputFileImpl, raw_pwrite_ಹರಿವು {
    Error keep() final {
      flush();
      ಹಿಂತಿರುಗಿ joinErrors(F1->keep(), F2->keep());
    }
    Error discard() final {
      flush();
      ಹಿಂತಿರುಗಿ joinErrors(F1->discard(), F2->discard());
    }
    raw_pwrite_ಹರಿವು &getOS() final { ಹಿಂತಿರುಗಿ *ಇದು; }

    ಶೂನ್ಯ write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) override {
      F1->getOS().ಲಿಖ(Ptr, ಗಾತ್ರ_2);
      F2->getOS().ಲಿಖ(Ptr, ಗಾತ್ರ_2);
    }
    ಶೂನ್ಯ pwrite_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2, uint64_t ಸ್ಥಾನಾಂತರ) override {
      ಇದು->flush();
      F1->getOS().pwrite(Ptr, ಗಾತ್ರ_2, ಸ್ಥಾನಾಂತರ);
      F2->getOS().pwrite(Ptr, ಗಾತ್ರ_2, ಸ್ಥಾನಾಂತರ);
    }
    uint64_t current_pos() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ F1->getOS().tell(); }
    size_t preferred_ಬಫರ್_ಗಾತ್ರ() ಸ್ಥಿರಾಂಕ override {
      ಹಿಂತಿರುಗಿ Preferred_ಬಫರ್_ಗಾತ್ರ;
    }
    ಶೂನ್ಯ reserveExtraSpace(uint64_t Extra_ಗಾತ್ರ) override {
      F1->getOS().reserveExtraSpace(Extra_ಗಾತ್ರ);
      F2->getOS().reserveExtraSpace(Extra_ಗಾತ್ರ);
    }
    ತಾರ್ಕಿಕ is_displayed() ಸ್ಥಿರಾಂಕ override {
      ಹಿಂತಿರುಗಿ F1->getOS().is_displayed() && F2->getOS().is_displayed();
    }
    ತಾರ್ಕಿಕ has_colors() ಸ್ಥಿರಾಂಕ override {
      ಹಿಂತಿರುಗಿ F1->getOS().has_colors() && F2->getOS().has_colors();
    }
    ಶೂನ್ಯ enable_colors(ತಾರ್ಕಿಕ enable) override {
      raw_pwrite_ಹರಿವು::enable_colors(enable);
      F1->getOS().enable_colors(enable);
      F2->getOS().enable_colors(enable);
    }

    MirroringOutput(std::unique_ptr<OutputFileImpl> F1,
                    std::unique_ptr<OutputFileImpl> F2)
        : Preferred_ಬಫರ್_ಗಾತ್ರ(std::max(F1->getOS().Get_ಬಫರ್_ಗಾತ್ರ(),
                                       F1->getOS().Get_ಬಫರ್_ಗಾತ್ರ())),
          F1(std::move(F1)), F2(std::move(F2)) {}

    size_t Preferred_ಬಫರ್_ಗಾತ್ರ;
    std::unique_ptr<OutputFileImpl> F1;
    std::unique_ptr<OutputFileImpl> F2;
  };
  ರಚನೆ MirroringOutputBackend : ಸಾರ್ವಜನಿಕ ProxyOutputBackend1,
                                  ಸಾರ್ವಜನಿಕ ProxyOutputBackend2 {
    Expected<std::unique_ptr<OutputFileImpl>>
    createFileImpl(StringRef ಮಾರ್ಗ,
                   std::optional<OutputConfig> Config) override {
      std::unique_ptr<OutputFileImpl> File1;
      std::unique_ptr<OutputFileImpl> File2;
      ಆದರೆ (Error E =
              ProxyOutputBackend1::createFileImpl(ಮಾರ್ಗ, Config).moveInto(File1))
        ಹಿಂತಿರುಗಿ std::move(E);
      ಆದರೆ (Error E =
              ProxyOutputBackend2::createFileImpl(ಮಾರ್ಗ, Config).moveInto(File2))
        ಹಿಂತಿರುಗಿ joinErrors(std::move(E), File1->discard());

      // Skip the extra indirection if one of these is a null output.
      ಆದರೆ (isa<NullOutputFileImpl>(*File1)) {
        consumeError(File1->discard());
        ಹಿಂತಿರುಗಿ std::move(File2);
      }
      ಆದರೆ (isa<NullOutputFileImpl>(*File2)) {
        consumeError(File2->discard());
        ಹಿಂತಿರುಗಿ std::move(File1);
      }
      ಹಿಂತಿರುಗಿ std::make_unique<MirroringOutput>(std::move(File1),
                                               std::move(File2));
    }

    IntrusiveRefCntPtr<OutputBackend> cloneImpl() ಸ್ಥಿರಾಂಕ override {
      ಹಿಂತಿರುಗಿ IntrusiveRefCntPtr<ProxyOutputBackend1>(
          makeIntrusiveRefCnt<MirroringOutputBackend>(
              ProxyOutputBackend1::getUnderlyingBackend().clone(),
              ProxyOutputBackend2::getUnderlyingBackend().clone()));
    }
    ಶೂನ್ಯ Retain() ಸ್ಥಿರಾಂಕ { ProxyOutputBackend1::Retain(); }
    ಶೂನ್ಯ Release() ಸ್ಥಿರಾಂಕ { ProxyOutputBackend1::Release(); }

    MirroringOutputBackend(IntrusiveRefCntPtr<OutputBackend> Backend1,
                           IntrusiveRefCntPtr<OutputBackend> Backend2)
        : ProxyOutputBackend1(std::move(Backend1)),
          ProxyOutputBackend2(std::move(Backend2)) {}
  };

  ಪ್ರತಿಜ್ಞಾ(Backend1 && "Expected actual backend");
  ಪ್ರತಿಜ್ಞಾ(Backend2 && "Expected actual backend");
  ಹಿಂತಿರುಗಿ IntrusiveRefCntPtr<ProxyOutputBackend1>(
      makeIntrusiveRefCnt<MirroringOutputBackend>(std::move(Backend1),
                                                  std::move(Backend2)));
}

ಸ್ಥಿರ OutputConfig
applySettings(std::optional<OutputConfig> &&Config,
              ಸ್ಥಿರಾಂಕ OnDiskOutputBackend::OutputSettings &Settings) {
  ಆದರೆ (!Config)
    Config = Settings.DefaultConfig;
  ಆದರೆ (!Settings.UseTemporaries)
    Config->ಗಣ_No_Atomic_Write();
  ಆದರೆ (!Settings.RemoveOnSignal)
    Config->ಗಣ_No_Discard_On_Signal();
  ಹಿಂತಿರುಗಿ *Config;
}

ನಾಮಾಕಾಶ {
ವರ್ಗ OnDiskOutputFile final : ಸಾರ್ವಜನಿಕ OutputFileImpl {
ಸಾರ್ವಜನಿಕ:
  Error keep() override;
  Error discard() override;
  raw_pwrite_ಹರಿವು &getOS() override {
    ಪ್ರತಿಜ್ಞಾ(FileOS && "Expected valid file");
    ಆದರೆ (ಬಫರ್_OS)
      ಹಿಂತಿರುಗಿ *ಬಫರ್_OS;
    ಹಿಂತಿರುಗಿ *FileOS;
  }

  /// Attempt to open a temporary file for \p OutputPath.
  ///
  /// This tries to open a uniquely-named temporary file for \p OutputPath,
  /// possibly also creating any missing directories if \a
  /// OnDiskOutputConfig::UseTemporaryCreateMissingDirectories is set in \a
  /// Config.
  ///
  /// \post FD and \a TempPath are initialized if this is successful.
  Error try_ಗಮ್ಯ_Create_Temporary(std::optional<ಪೂರ್ಣಾಂಕ> &FD);

  Error initializeFile(std::optional<ಪೂರ್ಣಾಂಕ> &FD);
  Error initialize_ಹರಿವು();
  Error reset();

  OnDiskOutputFile(StringRef Output_ಮಾರ್ಗ, std::optional<OutputConfig> Config,
                   ಸ್ಥಿರಾಂಕ OnDiskOutputBackend::OutputSettings &Settings)
      : Config(applySettings(std::move(Config), Settings)),
        Output_ಮಾರ್ಗ(Output_ಮಾರ್ಗ.str()) {}

  OutputConfig Config;
  ಸ್ಥಿರಾಂಕ std::string Output_ಮಾರ್ಗ;
  std::optional<std::string> ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ;
  std::optional<raw_fd_ostream> FileOS;
  std::optional<ಬಫರ್_ostream> ಬಫರ್_OS;
};
} // end namespace

ಸ್ಥಿರ Error createDirectoriesOnDemand(StringRef Output_ಮಾರ್ಗ,
                                       OutputConfig Config,
                                       llvm::function_ref<Error()> CreateFile) {
  ಹಿಂತಿರುಗಿ handleErrors(CreateFile(), [&](std::unique_ptr<ECError> EC) {
    ಆದರೆ (EC->convert_ಗಮ್ಯ_Error_ಸಂಕೇತ() != std::errc::no_such_file_or_directory ||
        Config.getNoImplyCreateDirectories())
      ಹಿಂತಿರುಗಿ Error(std::move(EC));

    StringRef ಪೋಷಕ_ಮಾರ್ಗ_2 = sys::ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(Output_ಮಾರ್ಗ);
    ಆದರೆ (std::error_ಸಂಕೇತ EC = sys::fs::create_directories(ಪೋಷಕ_ಮಾರ್ಗ_2))
      ಹಿಂತಿರುಗಿ make_error<OutputError>(ಪೋಷಕ_ಮಾರ್ಗ_2, EC);
    ಹಿಂತಿರುಗಿ CreateFile();
  });
}

ಸ್ಥಿರ sys::fs::OpenFlags generateFlagsFromConfig(OutputConfig Config) {
  sys::fs::OpenFlags ದ_2 = sys::fs::ದ_None;
  ಆದರೆ (Config.get_ಪಠ್ಯ_With_CRLF())
    ದ_2 |= sys::fs::ದ_ಪಠ್ಯ_With_CRLF;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Config.get_ಪಠ್ಯ())
    ದ_2 |= sys::fs::ದ_ಪಠ್ಯ;
  // Don't pass OF_Append if writting to temporary since OF_Append is
  // not Atomic Append
  ಆದರೆ (Config.getAppend() && !Config.getAtomicWrite())
    ದ_2 |= sys::fs::ದ_Append;

  ಹಿಂತಿರುಗಿ ದ_2;
}

Error OnDiskOutputFile::try_ಗಮ್ಯ_Create_Temporary(std::optional<ಪೂರ್ಣಾಂಕ> &FD) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  // Create a temporary file.
  // Insert -%%%%%%%% before the extension (if any), and because some tools
  // (noticeable, clang's own GlobalModuleIndex.cpp) glob for build
  // artifacts, also append .tmp.
  StringRef OutputExtension = sys::ಮಾರ್ಗ_2::extension(Output_ಮಾರ್ಗ);
  SmallString<128> Model_ಮಾರ್ಗ =
      StringRef(Output_ಮಾರ್ಗ).drop_ಹಿಂಭಾಗ(OutputExtension.ಗಾತ್ರ_3());
  Model_ಮಾರ್ಗ += "-%%%%%%%%";
  Model_ಮಾರ್ಗ += OutputExtension;
  Model_ಮಾರ್ಗ += ".tmp";

  ಹಿಂತಿರುಗಿ createDirectoriesOnDemand(Output_ಮಾರ್ಗ, Config, [&]() -> Error {
    ಪೂರ್ಣಾಂಕ NewFD;
    SmallString<128> Unique_ಮಾರ್ಗ;
    sys::fs::OpenFlags ದ_2 = generateFlagsFromConfig(Config);
    ಆದರೆ (std::error_ಸಂಕೇತ EC =
            sys::fs::createUniqueFile(Model_ಮಾರ್ಗ, NewFD, Unique_ಮಾರ್ಗ, ದ_2))
      ಹಿಂತಿರುಗಿ make_error<ತಾತ್ಕಾಲಿಕ_File_Output_Error>(Model_ಮಾರ್ಗ, Output_ಮಾರ್ಗ, EC);

    ಆದರೆ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Unique_ಮಾರ್ಗ);

    ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ = Unique_ಮಾರ್ಗ.str().str();
    FD.emplace(NewFD);
    ಹಿಂತಿರುಗಿ Error::success();
  });
}

Error OnDiskOutputFile::initializeFile(std::optional<ಪೂರ್ಣಾಂಕ> &FD) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ(Output_ಮಾರ್ಗ != "-" && "Unexpected request for FD of stdout");

  // Disable temporary file for other non-regular files, and if we get a status
  // object, also check if in append mode we can write and disable write-through
  // buffers if appropriate.
  ಆದರೆ (Config.getAtomicWrite()) {
    sys::fs::file_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;
    sys::fs::ಸ್ಥಿತಿ_3(Output_ಮಾರ್ಗ, ಸ್ಥಿತಿ_2);
    ಆದರೆ (sys::fs::exists(ಸ್ಥಿತಿ_2)) {
      ಆದರೆ (!sys::fs::is_regular_file(ಸ್ಥಿತಿ_2))
        Config.ಗಣ_No_Atomic_Write();

      // In append mode, we will open the file for writing which will need write
      // permission. Fail now if it is already clear that we can't write to the
      // final destination.
      // In non-append mode, we will delete and replace the file. Permission
      // bits of the file itself are irrelevant in this case.
      ಆದರೆ (Config.getAppend() && !sys::fs::can_write(Output_ಮಾರ್ಗ))
        ಹಿಂತಿರುಗಿ make_error<OutputError>(
            Output_ಮಾರ್ಗ,
            std::make_error_ಸಂಕೇತ(std::errc::operation_not_permitted));
    }
  }

  // If (still) using a temporary file, try to create it (and return success if
  // that works).
  ಆದರೆ (Config.getAtomicWrite())
    ಆದರೆ (!error_ಗಮ್ಯ_Bool(try_ಗಮ್ಯ_Create_Temporary(FD)))
      ಹಿಂತಿರುಗಿ Error::success();

  // Not using a temporary file. Open the final output file.
  ಹಿಂತಿರುಗಿ createDirectoriesOnDemand(Output_ಮಾರ್ಗ, Config, [&]() -> Error {
    ಪೂರ್ಣಾಂಕ NewFD;
    sys::fs::OpenFlags ದ_2 = generateFlagsFromConfig(Config);
    ಆದರೆ (std::error_ಸಂಕೇತ EC = sys::fs::openFileForWrite(
            Output_ಮಾರ್ಗ, NewFD, sys::fs::CD_CreateAlways, ದ_2))
      ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(Output_ಮಾರ್ಗ, EC);
    FD.emplace(NewFD);

    ಆದರೆ (Config.getDiscardOnSignal())
      sys::RemoveFileOnSignal(Output_ಮಾರ್ಗ);
    ಹಿಂತಿರುಗಿ Error::success();
  });
}

Error OnDiskOutputFile::initialize_ಹರಿವು() {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  // Open the file stream.
  ಆದರೆ (Output_ಮಾರ್ಗ == "-") {
    std::error_ಸಂಕೇತ EC;
    FileOS.emplace(Output_ಮಾರ್ಗ, EC);
    ಆದರೆ (EC)
      ಹಿಂತಿರುಗಿ make_error<OutputError>(Output_ಮಾರ್ಗ, EC);
  } ಇಲ್ಲದಿದ್ದರೆ {
    std::optional<ಪೂರ್ಣಾಂಕ> FD;
    ಆದರೆ (Error E = initializeFile(FD))
      ಹಿಂತಿರುಗಿ E;
    FileOS.emplace(*FD, /*shouldClose=*/ನಿಜ);
  }

  // Buffer the stream if necessary.
  ಆದರೆ (!FileOS->supportsSeeking() && !Config.get_ಪಠ್ಯ())
    ಬಫರ್_OS.emplace(*FileOS);

  ಹಿಂತಿರುಗಿ Error::success();
}

ನಾಮಾಕಾಶ {
ವರ್ಗ OpenFileRAII {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ InvalidFd = -1;

ಸಾರ್ವಜನಿಕ:
  ಪೂರ್ಣಾಂಕ Fd = InvalidFd;

  ~OpenFileRAII() {
    ಆದರೆ (Fd != InvalidFd)
      llvm::sys::Process::SafelyCloseFileDescriptor(Fd);
  }
};

ಎಣಿಕೆ ವರ್ಗ FileDifference : uint8_t {
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
areFilesDifferent(ಸ್ಥಿರಾಂಕ llvm::Twine &ಮೂಲ, ಸ್ಥಿರಾಂಕ llvm::Twine &Destination) {
  ಆದರೆ (sys::fs::equivalent(ಮೂಲ, Destination))
    ಹಿಂತಿರುಗಿ FileDifference::IdenticalFile;

  OpenFileRAII ಮೂಲ_File;
  sys::fs::file_ಸ್ಥಿತಿ ಮೂಲ_ಸ್ಥಿತಿ;
  // If we can't open the source file, fail.
  ಆದರೆ (std::error_ಸಂಕೇತ EC = sys::fs::openFileForRead(ಮೂಲ, ಮೂಲ_File.Fd))
    ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(ಮೂಲ, EC);

  // If we can't stat the source file, fail.
  ಆದರೆ (std::error_ಸಂಕೇತ EC = sys::fs::ಸ್ಥಿತಿ_3(ಮೂಲ_File.Fd, ಮೂಲ_ಸ್ಥಿತಿ))
    ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(ಮೂಲ, EC);

  OpenFileRAII DestFile;
  sys::fs::file_ಸ್ಥಿತಿ Dest_ಸ್ಥಿತಿ;
  // If we can't open the destination file, report different.
  ಆದರೆ (std::error_ಸಂಕೇತ Error =
          sys::fs::openFileForRead(Destination, DestFile.Fd))
    ಹಿಂತಿರುಗಿ FileDifference::DifferentContents;

  // If we can't open the destination file, report different.
  ಆದರೆ (std::error_ಸಂಕೇತ Error = sys::fs::ಸ್ಥಿತಿ_3(DestFile.Fd, Dest_ಸ್ಥಿತಿ))
    ಹಿಂತಿರುಗಿ FileDifference::DifferentContents;

  // If the files are different sizes, they must be different.
  uint64_t ಗಾತ್ರ_2 = ಮೂಲ_ಸ್ಥಿತಿ.get_ಗಾತ್ರ();
  ಆದರೆ (ಗಾತ್ರ_2 != Dest_ಸ್ಥಿತಿ.get_ಗಾತ್ರ())
    ಹಿಂತಿರುಗಿ FileDifference::DifferentContents;

  // If both files are zero size, they must be the same.
  ಆದರೆ (ಗಾತ್ರ_2 == 0)
    ಹಿಂತಿರುಗಿ FileDifference::SameContents;

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_ಸಂಕೇತ ಮೂಲ_Region_Err;
  sys::fs::mapped_file_region ಮೂಲ_Region(
      sys::fs::convertFDToNativeFile(ಮೂಲ_File.Fd),
      sys::fs::mapped_file_region::readonly, ಗಾತ್ರ_2, 0, ಮೂಲ_Region_Err);
  ಆದರೆ (ಮೂಲ_Region_Err)
    ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(ಮೂಲ, ಮೂಲ_Region_Err);

  std::error_ಸಂಕೇತ DestRegionErr;
  sys::fs::mapped_file_region DestRegion(
      sys::fs::convertFDToNativeFile(DestFile.Fd),
      sys::fs::mapped_file_region::readonly, ಗಾತ್ರ_2, 0, DestRegionErr);

  ಆದರೆ (DestRegionErr)
    ಹಿಂತಿರುಗಿ FileDifference::DifferentContents;

  ಆದರೆ (memcmp(ಮೂಲ_Region.const_ದತ್ತಾಂಶ(), DestRegion.const_ದತ್ತಾಂಶ(), ಗಾತ್ರ_2) != 0)
    ಹಿಂತಿರುಗಿ FileDifference::DifferentContents;

  ಹಿಂತಿರುಗಿ FileDifference::SameContents;
}

Error OnDiskOutputFile::reset() {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ಬಫರ್_OS.reset();
  ಆದರೆ (!FileOS)
    ಹಿಂತಿರುಗಿ Error::success();

  // Remember the error in raw_fd_ostream to be reported later.
  std::error_ಸಂಕೇತ EC = FileOS->error();
  // Clear the error to avoid fatal error when reset.
  FileOS->clear_error();
  FileOS.reset();
  ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
}

Error OnDiskOutputFile::keep() {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ E = reset())
    ಹಿಂತಿರುಗಿ E;

  // Close the file descriptor and remove crash cleanup before exit.
  llvm::scope_exit RemoveDiscardOnSignal([&]() {
    ಆದರೆ (Config.getDiscardOnSignal())
      sys::DontRemoveFileOnSignal(ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ ? *ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ : Output_ಮಾರ್ಗ);
  });

  ಆದರೆ (!ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ)
    ಹಿಂತಿರುಗಿ Error::success();

  // See if we should append instead of move.
  ಆದರೆ (Config.getAppend() && Output_ಮಾರ್ಗ != "-") {
    // Read TempFile for the content to append.
    ಸ್ವಯಂಚಾಲಿತ Content = ಸ್ಮರಣೆ_ಬಫರ್::getFile(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ);
    ಆದರೆ (!Content)
      ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_ತಾತ್ಕಾಲಿಕ_File_Output_Error(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ, Output_ಮಾರ್ಗ,
                                          Content.getError());
    ಆಗ (1) {
      // Attempt to lock the output file.
      // Only one process is allowed to append to this file at a time.
      llvm::ಬೀಗ_File_Manager ಬೀಗ_3(Output_ಮಾರ್ಗ);
      ತಾರ್ಕಿಕ Owned;
      ಆದರೆ (Error Err = ಬೀಗ_3.try_ಬೀಗ().moveInto(Owned)) {
        // If we error acquiring a lock, we cannot ensure appends
        // to the trace file are atomic - cannot ensure output correctness.
        ಬೀಗ_3.unsafeUnlock();
        ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(
            Output_ಮಾರ್ಗ, std::make_error_ಸಂಕೇತ(std::errc::no_ಬೀಗ_ಲಭ್ಯ));
      }
      ಆದರೆ (Owned) {
        // Lock acquired, perform the write and release the lock.
        std::error_ಸಂಕೇತ EC;
        llvm::raw_fd_ostream ಹೊರಹೋಗು(Output_ಮಾರ್ಗ, EC, llvm::sys::fs::ದ_Append);
        ಆದರೆ (EC)
          ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(Output_ಮಾರ್ಗ, EC);
        ಹೊರಹೋಗು << (*Content)->get_ಬಫರ್();
        ಹೊರಹೋಗು.close();
        ಬೀಗ_3.unsafeUnlock();
        ಆದರೆ (ಹೊರಹೋಗು.has_error())
          ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(Output_ಮಾರ್ಗ, ಹೊರಹೋಗು.error());
        // Remove temp file and done.
        (ಶೂನ್ಯ)sys::fs::remove(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ);
        ಹಿಂತಿರುಗಿ Error::success();
      }
      // Someone else owns the lock on this file, wait.
      ಆಯ್ಕೆ (ಬೀಗ_3.waitForUnlockFor(std::chrono::seconds(256))) {
      ಸ್ಥಿತಿ Wait_For_Unlock_ಫಲಿತಾಂಶ::Success:
        [[fallthrough]];
      ಸ್ಥಿತಿ Wait_For_Unlock_ಫಲಿತಾಂಶ::OwnerDied: {
        ಮುಂದುವರಿ; // try again to get the lock.
      }
      ಸ್ಥಿತಿ Wait_For_Unlock_ಫಲಿತಾಂಶ::Timeout: {
        // We could error on timeout to avoid potentially hanging forever, but
        // it may be more likely that an interrupted process failed to clear
        // the lock, causing other waiting processes to time-out. Let's clear
        // the lock and try again right away. If we do start seeing compiler
        // hangs in this location, we will need to re-consider.
        ಬೀಗ_3.unsafeUnlock();
        ಮುಂದುವರಿ;
      }
      }
      ನಿಲ್ಲಿಸು;
    }
  }

  ಆದರೆ (Config.getOnlyIfDifferent()) {
    ಸ್ವಯಂಚಾಲಿತ ಫಲಿತಾಂಶ = areFilesDifferent(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ, Output_ಮಾರ್ಗ);
    ಆದರೆ (!ಫಲಿತಾಂಶ)
      ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ.takeError();
    ಆಯ್ಕೆ (*ಫಲಿತಾಂಶ) {
    ಸ್ಥಿತಿ FileDifference::IdenticalFile:
      // Do nothing for a self-move.
      ಹಿಂತಿರುಗಿ Error::success();

    ಸ್ಥಿತಿ FileDifference::SameContents:
      // Files are identical; remove the source file.
      (ಶೂನ್ಯ)sys::fs::remove(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ);
      ಹಿಂತಿರುಗಿ Error::success();

    ಸ್ಥಿತಿ FileDifference::DifferentContents:
      ನಿಲ್ಲಿಸು; // Rename the file.
    }
  }

  // Move temporary to the final output path and remove it if that fails.
  std::error_ಸಂಕೇತ RenameEC = sys::fs::rename(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ, Output_ಮಾರ್ಗ);
  ಆದರೆ (!RenameEC)
    ಹಿಂತಿರುಗಿ Error::success();

  // FIXME: TempPath should be in the same directory as OutputPath but try to
  // copy the output to see if makes any difference. If this path is used,
  // investigate why we need to copy.
  RenameEC = sys::fs::copy_file(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ, Output_ಮಾರ್ಗ);
  (ಶೂನ್ಯ)sys::fs::remove(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ);

  ಆದರೆ (!RenameEC)
    ಹಿಂತಿರುಗಿ Error::success();

  ಹಿಂತಿರುಗಿ make_error<ತಾತ್ಕಾಲಿಕ_File_Output_Error>(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ, Output_ಮಾರ್ಗ, RenameEC);
}

Error OnDiskOutputFile::discard() {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  // Destroy the streams to flush them.
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ E = reset())
    ಹಿಂತಿರುಗಿ E;

  // Nothing on the filesystem to remove for stdout.
  ಆದರೆ (Output_ಮಾರ್ಗ == "-")
    ಹಿಂತಿರುಗಿ Error::success();

  ಸ್ವಯಂಚಾಲಿತ discard_ಮಾರ್ಗ = [&](StringRef ಮಾರ್ಗ) {
    std::error_ಸಂಕೇತ EC = sys::fs::remove(ಮಾರ್ಗ);
    sys::DontRemoveFileOnSignal(ಮಾರ್ಗ);
    ಹಿಂತಿರುಗಿ EC;
  };

  // Clean up the file that's in-progress.
  ಆದರೆ (!ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ)
    ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(Output_ಮಾರ್ಗ, discard_ಮಾರ್ಗ(Output_ಮಾರ್ಗ));
  ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_ತಾತ್ಕಾಲಿಕ_File_Output_Error(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ, Output_ಮಾರ್ಗ,
                                      discard_ಮಾರ್ಗ(*ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ));
}

Error OnDiskOutputBackend::makeAbsolute(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ) ಸ್ಥಿರಾಂಕ {
  // FIXME: Should this really call sys::fs::make_absolute?
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();
  ಹಿಂತಿರುಗಿ convert_ಗಮ್ಯ_Output_Error(StringRef(ಮಾರ್ಗ.ದತ್ತಾಂಶ(), ಮಾರ್ಗ.ಗಾತ್ರ_3()),
                              sys::fs::make_absolute(ಮಾರ್ಗ));
}

Expected<std::unique_ptr<OutputFileImpl>>
OnDiskOutputBackend::createFileImpl(StringRef ಮಾರ್ಗ,
                                    std::optional<OutputConfig> Config) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Abs_ಮಾರ್ಗ;
  ಆದರೆ (ಮಾರ್ಗ != "-") {
    Abs_ಮಾರ್ಗ = ಮಾರ್ಗ;
    ಆದರೆ (Error E = makeAbsolute(Abs_ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ std::move(E);
    ಮಾರ್ಗ = Abs_ಮಾರ್ಗ;
  }

  ಸ್ವಯಂಚಾಲಿತ File = std::make_unique<OnDiskOutputFile>(ಮಾರ್ಗ, Config, Settings);
  ಆದರೆ (Error E = File->initialize_ಹರಿವು())
    ಹಿಂತಿರುಗಿ std::move(E);

  ಹಿಂತಿರುಗಿ std::move(File);
}
