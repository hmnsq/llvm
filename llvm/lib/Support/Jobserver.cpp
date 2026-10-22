//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Jobserver.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"

#ಅನ್ತರ್ಭಾವ <atomic>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <mutex>
#ಅನ್ತರ್ಭಾವ <new>

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_TYPE "jobserver"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {
ರಚನೆ FdPair {
  ಪೂರ್ಣಾಂಕ Read = -1;
  ಪೂರ್ಣಾಂಕ Write = -1;
  ತಾರ್ಕಿಕ isValid() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Read >= 0 && Write >= 0; }
};

ರಚನೆ JobserverConfig {
  ಎಣಿಕೆ ಬಹುಲಕ {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  ಬಹುಲಕ The_ಬಹುಲಕ = None;
  std::string ಮಾರ್ಗ;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
ತಾರ್ಕಿಕ get_Prefixed_ಮೌಲ್ಯ(StringRef ಒಳಹರಿವು, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ, StringRef &ಮೌಲ್ಯ) {
  ಆದರೆ (ಒಳಹರಿವು.consume_front(ಪೂರ್ವಪ್ರತ್ಯಯ)) {
    ಮೌಲ್ಯ = ಒಳಹರಿವು;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ಸ್ಥಿರ std::optional<FdPair> getFileDescriptorPair(StringRef ಒಳಹರಿವು) {
  FdPair FDs;
  ಆದರೆ (ಒಳಹರಿವು.consumeInteger(10, FDs.Read))
    ಹಿಂತಿರುಗಿ std::nullopt;
  ಆದರೆ (!ಒಳಹರಿವು.consume_front(","))
    ಹಿಂತಿರುಗಿ std::nullopt;
  ಆದರೆ (ಒಳಹರಿವು.consumeInteger(10, FDs.Write))
    ಹಿಂತಿರುಗಿ std::nullopt;
  ಆದರೆ (!ಒಳಹರಿವು.ಖಾಲಿಗಳು() || !FDs.isValid())
    ಹಿಂತಿರುಗಿ std::nullopt;
  ಹಿಂತಿರುಗಿ FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  ಆದರೆ (MakeFlags.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ Config;

  // Split the MAKEFLAGS string into arguments.
  Small_ಸದಿಶ<StringRef, 8> ವಾದಗಳು;
  ವಿಭಜಿಸು_String(MakeFlags, ವಾದಗಳು);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  ಆದರೆ (!ವಾದಗಳು.ಖಾಲಿಗಳು() && !ವಾದಗಳು[0].starts_with("-") && ವಾದಗಳು[0].ಒಳಗೊಂಡಿದೆ('n'))
    ಹಿಂತಿರುಗಿ Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  ಗಾಗಿ (StringRef Arg : ವಾದಗಳು) {
    StringRef ಮೌಲ್ಯ;
    ಆದರೆ (get_Prefixed_ಮೌಲ್ಯ(Arg, "--jobserver-auth=", ಮೌಲ್ಯ)) {
      // Try to parse as a file descriptor pair first.
      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ FDPair = getFileDescriptorPair(ಮೌಲ್ಯ)) {
        Config.The_ಬಹುಲಕ = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ಇಲ್ಲದಿದ್ದರೆ {
        StringRef Fifo_ಮಾರ್ಗ;
        // If not FDs, try to parse as a named pipe (fifo).
        ಆದರೆ (get_Prefixed_ಮೌಲ್ಯ(ಮೌಲ್ಯ, "fifo:", Fifo_ಮಾರ್ಗ)) {
          Config.The_ಬಹುಲಕ = JobserverConfig::PosixFifo;
          Config.ಮಾರ್ಗ = Fifo_ಮಾರ್ಗ.str();
        } ಇಲ್ಲದಿದ್ದರೆ {
          // Otherwise, assume it's a Windows semaphore.
          Config.The_ಬಹುಲಕ = JobserverConfig::Win32Semaphore;
          Config.ಮಾರ್ಗ = ಮೌಲ್ಯ.str();
        }
      }
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (get_Prefixed_ಮೌಲ್ಯ(Arg, "--jobserver-fds=", ಮೌಲ್ಯ)) {
      // This is an alternative, older syntax for the pipe-based server.
      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ FDPair = getFileDescriptorPair(ಮೌಲ್ಯ)) {
        Config.The_ಬಹುಲಕ = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಹಿಂತಿರುಗಿ createStringError(inconvertible_Error_ಸಂಕೇತ(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಆದರೆ (Config.The_ಬಹುಲಕ == JobserverConfig::PosixFifo ||
      Config.The_ಬಹುಲಕ == JobserverConfig::PosixPipe)
    ಹಿಂತಿರುಗಿ createStringError(
        inconvertible_Error_ಸಂಕೇತ(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (Config.The_ಬಹುಲಕ == JobserverConfig::Win32Semaphore)
    ಹಿಂತಿರುಗಿ createStringError(
        inconvertible_Error_ಸಂಕೇತ(),
        "Semaphore-based jobserver is not supported on this platform");
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = ಶೂನ್ಯಸೂಚಕ;

} // namespace

ನಾಮಾಕಾಶ llvm {
ವರ್ಗ JobserverClientImpl : ಸಾರ್ವಜನಿಕ JobserverClient {
  ತಾರ್ಕಿಕ IsInitialized = ಸುಳ್ಳು;
  std::atomic<ತಾರ್ಕಿಕ> Has_Implicit_ಸ್ಥಾನ{ನಿಜ};
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Jobs = 0;

ಸಾರ್ವಜನಿಕ:
  JobserverClientImpl(ಸ್ಥಿರಾಂಕ JobserverConfig &Config);
  ~JobserverClientImpl() override;

  ಕೆಲಸ_ಸ್ಥಾನ tryAcquire() override;
  ಶೂನ್ಯ release(ಕೆಲಸ_ಸ್ಥಾನ ಸ್ಥಾನ) override;
  ಚಿಹ್ನೆರಹಿತ get_ಅಂಕೆ_Jobs() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ ಅಂಕೆ_Jobs; }

  ತಾರ್ಕಿಕ isValid() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ IsInitialized; }

ಖಾಸಗಿ:
#ಆದರೆ defined(LLVM_ON_UNIX)
  ಪೂರ್ಣಾಂಕ ReadFD = -1;
  ಪೂರ್ಣಾಂಕ WriteFD = -1;
  std::string Fifo_ಮಾರ್ಗ;
#ಅಥವಾಆದರೆ defined(_WIN32)
  ಶೂನ್ಯ *Semaphore = ಶೂನ್ಯಸೂಚಕ;
#ಆದರೆ_ಅಂತ್ಯ
};
} // namespace llvm

// Include the platform-specific parts of the class.
#ಆದರೆ defined(LLVM_ON_UNIX)
#ಅನ್ತರ್ಭಾವ "Unix/Jobserver.inc"
#ಅಥವಾಆದರೆ defined(_WIN32)
#ಅನ್ತರ್ಭಾವ "Windows/Jobserver.inc"
#ಇಲ್ಲದಿದ್ದರೆ
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ಸ್ಥಿರಾಂಕ JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = ಪೂರ್ವನಿಯೋಜಿತ;
ಕೆಲಸ_ಸ್ಥಾನ JobserverClientImpl::tryAcquire() { ಹಿಂತಿರುಗಿ ಕೆಲಸ_ಸ್ಥಾನ(); }
ಶೂನ್ಯ JobserverClientImpl::release(ಕೆಲಸ_ಸ್ಥಾನ ಸ್ಥಾನ) {}
#ಆದರೆ_ಅಂತ್ಯ

ನಾಮಾಕಾಶ llvm {
JobserverClient::~JobserverClient() = ಪೂರ್ವನಿಯೋಜಿತ;

uint8_t ಕೆಲಸ_ಸ್ಥಾನ::get_Explicit_ಮೌಲ್ಯ() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(isExplicit() && "Cannot get value of implicit or invalid slot");
  ಹಿಂತಿರುಗಿ static_cast<uint8_t>(ಮೌಲ್ಯ);
}

/// This is the main entry point for acquiring a jobserver client. It uses a
/// std::call_once to ensure the singleton `GJobserver` instance is created
/// safely in a multi-threaded environment. On first call, it reads the
/// `MAKEFLAGS` environment variable, parses it, and attempts to construct and
/// initialize a `JobserverClientImpl`. If successful, the global instance is
/// stored in `GJobserver`. Subsequent calls will return the existing instance.
JobserverClient *JobserverClient::getInstance() {
  std::call_once(GJobserverOnceFlag, []() {
    LLVM_DEBUG(
        dbgs()
        << "JobserverClient::getInstance() called for the first time.\n");
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *MakeFlagsEnv = getenv("MAKEFLAGS");
    ಆದರೆ (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      ಹಿಂತಿರುಗಿ;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    ಸ್ವಯಂಚಾಲಿತ ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    ಆದರೆ (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << ಗಮ್ಯ_String(std::move(Err)) << "\n";
      ಹಿಂತಿರುಗಿ;
    }

    JobserverConfig Config = *ConfigOrErr;
    ಆದರೆ (Config.The_ಬಹುಲಕ == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (Config.The_ಬಹುಲಕ == JobserverConfig::PosixPipe) {
#ಆದರೆ defined(LLVM_ON_UNIX)
      ಆದರೆ (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        ಹಿಂತಿರುಗಿ;
      }
#ಆದರೆ_ಅಂತ್ಯ
    }

    ಸ್ವಯಂಚಾಲಿತ Client = std::make_unique<JobserverClientImpl>(Config);
    ಆದರೆ (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } ಇಲ್ಲದಿದ್ದರೆ
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  ಹಿಂತಿರುಗಿ GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
ಶೂನ್ಯ JobserverClient::resetForTesting() {
  ಅಳಿಸು GJobserver;
  GJobserver = ಶೂನ್ಯಸೂಚಕ;
  // Re-construct the std::once_flag in place to reset the singleton state.
  ಹೊಸ (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
