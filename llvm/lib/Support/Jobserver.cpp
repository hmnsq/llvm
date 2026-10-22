//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Jobserver.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <atomic>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <mutex>
#ਅਨ੍ਤਰ੍ਭਾਵ <new>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_TYPE "jobserver"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ FdPair {
  ਪੂਰਨ Read = -1;
  ਪੂਰਨ Write = -1;
  ਤਾਰਕਿਕ isValid() ਅਚਲ { ਵਾਪਸ Read >= 0 && Write >= 0; }
};

ਬਣਤਰ JobserverConfig {
  ਗਿਣਤੀ ਬਹੁਲਕ {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  ਬਹੁਲਕ The_ਬਹੁਲਕ = None;
  std::string ਰਸਤਾ;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
ਤਾਰਕਿਕ get_Prefixed_ਮੁੱਲ(StringRef ਨਿਵੇਸ਼, StringRef ਅਗੇਤਰ, StringRef &ਮੁੱਲ) {
  ਜੇ (ਨਿਵੇਸ਼.consume_front(ਅਗੇਤਰ)) {
    ਮੁੱਲ = ਨਿਵੇਸ਼;
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ਸਥਿਰ std::optional<FdPair> getFileDescriptorPair(StringRef ਨਿਵੇਸ਼) {
  FdPair FDs;
  ਜੇ (ਨਿਵੇਸ਼.consumeInteger(10, FDs.Read))
    ਵਾਪਸ std::nullopt;
  ਜੇ (!ਨਿਵੇਸ਼.consume_front(","))
    ਵਾਪਸ std::nullopt;
  ਜੇ (ਨਿਵੇਸ਼.consumeInteger(10, FDs.Write))
    ਵਾਪਸ std::nullopt;
  ਜੇ (!ਨਿਵੇਸ਼.ਖਾਲੀ_ਥਾਵਾਂ() || !FDs.isValid())
    ਵਾਪਸ std::nullopt;
  ਵਾਪਸ FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  ਜੇ (MakeFlags.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ Config;

  // Split the MAKEFLAGS string into arguments.
  Small_ਸਦਿਸ਼<StringRef, 8> ਦਲੀਲਾਂ;
  ਵੰਡੋ_String(MakeFlags, ਦਲੀਲਾਂ);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  ਜੇ (!ਦਲੀਲਾਂ.ਖਾਲੀ_ਥਾਵਾਂ() && !ਦਲੀਲਾਂ[0].starts_with("-") && ਦਲੀਲਾਂ[0].ਸ਼ਾਮਲ_ਹੈ('n'))
    ਵਾਪਸ Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  ਲਈ (StringRef Arg : ਦਲੀਲਾਂ) {
    StringRef ਮੁੱਲ;
    ਜੇ (get_Prefixed_ਮੁੱਲ(Arg, "--jobserver-auth=", ਮੁੱਲ)) {
      // Try to parse as a file descriptor pair first.
      ਜੇ (ਸਵੈਚਾਲਿਤ FDPair = getFileDescriptorPair(ਮੁੱਲ)) {
        Config.The_ਬਹੁਲਕ = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ਨਹੀਂਤਾਂ {
        StringRef Fifo_ਰਸਤਾ;
        // If not FDs, try to parse as a named pipe (fifo).
        ਜੇ (get_Prefixed_ਮੁੱਲ(ਮੁੱਲ, "fifo:", Fifo_ਰਸਤਾ)) {
          Config.The_ਬਹੁਲਕ = JobserverConfig::PosixFifo;
          Config.ਰਸਤਾ = Fifo_ਰਸਤਾ.str();
        } ਨਹੀਂਤਾਂ {
          // Otherwise, assume it's a Windows semaphore.
          Config.The_ਬਹੁਲਕ = JobserverConfig::Win32Semaphore;
          Config.ਰਸਤਾ = ਮੁੱਲ.str();
        }
      }
    } ਨਹੀਂਤਾਂ ਜੇ (get_Prefixed_ਮੁੱਲ(Arg, "--jobserver-fds=", ਮੁੱਲ)) {
      // This is an alternative, older syntax for the pipe-based server.
      ਜੇ (ਸਵੈਚਾਲਿਤ FDPair = getFileDescriptorPair(ਮੁੱਲ)) {
        Config.The_ਬਹੁਲਕ = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ਨਹੀਂਤਾਂ {
        ਵਾਪਸ createStringError(inconvertible_Error_ਕੋਡ(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਜੇ (Config.The_ਬਹੁਲਕ == JobserverConfig::PosixFifo ||
      Config.The_ਬਹੁਲਕ == JobserverConfig::PosixPipe)
    ਵਾਪਸ createStringError(
        inconvertible_Error_ਕੋਡ(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#ਨਹੀਂਤਾਂ
  ਜੇ (Config.The_ਬਹੁਲਕ == JobserverConfig::Win32Semaphore)
    ਵਾਪਸ createStringError(
        inconvertible_Error_ਕੋਡ(),
        "Semaphore-based jobserver is not supported on this platform");
#ਜੇ_ਅੰਤ
  ਵਾਪਸ Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = ਖਾਲੀ_ਸੂਚਕ;

} // namespace

ਨਾਮਾਕਾਸ਼ llvm {
ਵਰਗ JobserverClientImpl : ਜਨਤਕ JobserverClient {
  ਤਾਰਕਿਕ IsInitialized = ਝੂਠ;
  std::atomic<ਤਾਰਕਿਕ> Has_Implicit_ਥਾਂ{ਸੱਚ};
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Jobs = 0;

ਜਨਤਕ:
  JobserverClientImpl(ਅਚਲ JobserverConfig &Config);
  ~JobserverClientImpl() override;

  ਕੰਮ_ਥਾਂ tryAcquire() override;
  ਖਾਲੀ release(ਕੰਮ_ਥਾਂ ਥਾਂ) override;
  ਬਿਨਾਨਿਸ਼ਾਨੀ get_ਅੰਕ_Jobs() ਅਚਲ override { ਵਾਪਸ ਅੰਕ_Jobs; }

  ਤਾਰਕਿਕ isValid() ਅਚਲ { ਵਾਪਸ IsInitialized; }

ਨਿਜੀ:
#ਜੇ defined(LLVM_ON_UNIX)
  ਪੂਰਨ ReadFD = -1;
  ਪੂਰਨ WriteFD = -1;
  std::string Fifo_ਰਸਤਾ;
#ਜਾਂਜੇ defined(_WIN32)
  ਖਾਲੀ *Semaphore = ਖਾਲੀ_ਸੂਚਕ;
#ਜੇ_ਅੰਤ
};
} // namespace llvm

// Include the platform-specific parts of the class.
#ਜੇ defined(LLVM_ON_UNIX)
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/Jobserver.inc"
#ਜਾਂਜੇ defined(_WIN32)
#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/Jobserver.inc"
#ਨਹੀਂਤਾਂ
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ਅਚਲ JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = ਮੂਲ;
ਕੰਮ_ਥਾਂ JobserverClientImpl::tryAcquire() { ਵਾਪਸ ਕੰਮ_ਥਾਂ(); }
ਖਾਲੀ JobserverClientImpl::release(ਕੰਮ_ਥਾਂ ਥਾਂ) {}
#ਜੇ_ਅੰਤ

ਨਾਮਾਕਾਸ਼ llvm {
JobserverClient::~JobserverClient() = ਮੂਲ;

uint8_t ਕੰਮ_ਥਾਂ::get_Explicit_ਮੁੱਲ() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(isExplicit() && "Cannot get value of implicit or invalid slot");
  ਵਾਪਸ static_cast<uint8_t>(ਮੁੱਲ);
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
    ਅਚਲ ਅੱਖਰ *MakeFlagsEnv = getenv("MAKEFLAGS");
    ਜੇ (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      ਵਾਪਸ;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    ਸਵੈਚਾਲਿਤ ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    ਜੇ (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << ਮੰਜ਼ਿਲ_String(std::move(Err)) << "\n";
      ਵਾਪਸ;
    }

    JobserverConfig Config = *ConfigOrErr;
    ਜੇ (Config.The_ਬਹੁਲਕ == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      ਵਾਪਸ;
    }

    ਜੇ (Config.The_ਬਹੁਲਕ == JobserverConfig::PosixPipe) {
#ਜੇ defined(LLVM_ON_UNIX)
      ਜੇ (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        ਵਾਪਸ;
      }
#ਜੇ_ਅੰਤ
    }

    ਸਵੈਚਾਲਿਤ Client = std::make_unique<JobserverClientImpl>(Config);
    ਜੇ (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } ਨਹੀਂਤਾਂ
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  ਵਾਪਸ GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
ਖਾਲੀ JobserverClient::resetForTesting() {
  ਮਿਟਾਓ GJobserver;
  GJobserver = ਖਾਲੀ_ਸੂਚਕ;
  // Re-construct the std::once_flag in place to reset the singleton state.
  ਨਵਾਂ (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
