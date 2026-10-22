//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/Jobserver.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"

#അന്തര്ഭാവ <atomic>
#അന്തര്ഭാവ <memory>
#അന്തര്ഭാവ <mutex>
#അന്തര്ഭാവ <new>

#define DEBUG_TYPE "jobserver"

using നാമാകാശ llvm;

നാമാകാശ {
ഘടന FdPair {
  പൂർണ്ണസംഖ്യ Read = -1;
  പൂർണ്ണസംഖ്യ Write = -1;
  തർക്കം isValid() സ്ഥിരം { തിരികെ Read >= 0 && Write >= 0; }
};

ഘടന JobserverConfig {
  എണ്ണൽ Mode {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  Mode TheMode = None;
  std::string Path;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
തർക്കം get_Prefixed_മൂല്യം(StringRef നിവേശം, StringRef Prefix, StringRef &മൂല്യം) {
  എങ്കിൽ (നിവേശം.consume_front(Prefix)) {
    മൂല്യം = നിവേശം;
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
സ്ഥിതം std::optional<FdPair> getFileDescriptorPair(StringRef നിവേശം) {
  FdPair FDs;
  എങ്കിൽ (നിവേശം.consumeInteger(10, FDs.Read))
    തിരികെ std::nullopt;
  എങ്കിൽ (!നിവേശം.consume_front(","))
    തിരികെ std::nullopt;
  എങ്കിൽ (നിവേശം.consumeInteger(10, FDs.Write))
    തിരികെ std::nullopt;
  എങ്കിൽ (!നിവേശം.empty() || !FDs.isValid())
    തിരികെ std::nullopt;
  തിരികെ FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  എങ്കിൽ (MakeFlags.empty())
    തിരികെ Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  എങ്കിൽ (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    തിരികെ Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  വേണ്ടി (StringRef Arg : Args) {
    StringRef മൂല്യം;
    എങ്കിൽ (get_Prefixed_മൂല്യം(Arg, "--jobserver-auth=", മൂല്യം)) {
      // Try to parse as a file descriptor pair first.
      എങ്കിൽ (auto FDPair = getFileDescriptorPair(മൂല്യം)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } അല്ലെങ്കിൽ {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        എങ്കിൽ (get_Prefixed_മൂല്യം(മൂല്യം, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } അല്ലെങ്കിൽ {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = മൂല്യം.str();
        }
      }
    } അല്ലെങ്കിൽ എങ്കിൽ (get_Prefixed_മൂല്യം(Arg, "--jobserver-fds=", മൂല്യം)) {
      // This is an alternative, older syntax for the pipe-based server.
      എങ്കിൽ (auto FDPair = getFileDescriptorPair(മൂല്യം)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } അല്ലെങ്കിൽ {
        തിരികെ createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  എങ്കിൽ (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    തിരികെ createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#അല്ലെങ്കിൽ
  എങ്കിൽ (Config.TheMode == JobserverConfig::Win32Semaphore)
    തിരികെ createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  തിരികെ Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

നാമാകാശ llvm {
വർഗ്ഗം JobserverClientImpl : പൊതു JobserverClient {
  തർക്കം IsInitialized = അസത്യം;
  std::atomic<തർക്കം> HasImplicitSlot{സത്യം};
  unsigned അക്കം_Jobs = 0;

പൊതു:
  JobserverClientImpl(സ്ഥിരം JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  ശൂന്യം release(JobSlot Slot) override;
  unsigned get_അക്കം_Jobs() സ്ഥിരം override { തിരികെ അക്കം_Jobs; }

  തർക്കം isValid() സ്ഥിരം { തിരികെ IsInitialized; }

സ്വകാര്യ:
#എങ്കിൽ defined(LLVM_ON_UNIX)
  പൂർണ്ണസംഖ്യ ReadFD = -1;
  പൂർണ്ണസംഖ്യ WriteFD = -1;
  std::string FifoPath;
#അല്ലെങ്കിലെങ്കിൽ defined(_WIN32)
  ശൂന്യം *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#എങ്കിൽ defined(LLVM_ON_UNIX)
#അന്തര്ഭാവ "Unix/Jobserver.inc"
#അല്ലെങ്കിലെങ്കിൽ defined(_WIN32)
#അന്തര്ഭാവ "Windows/Jobserver.inc"
#അല്ലെങ്കിൽ
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(സ്ഥിരം JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = സ്ഥിരസ്ഥിതി;
JobSlot JobserverClientImpl::tryAcquire() { തിരികെ JobSlot(); }
ശൂന്യം JobserverClientImpl::release(JobSlot Slot) {}
#endif

നാമാകാശ llvm {
JobserverClient::~JobserverClient() = സ്ഥിരസ്ഥിതി;

uint8_t JobSlot::get_Explicit_മൂല്യം() സ്ഥിരം {
  പ്രതിജ്ഞാ(isExplicit() && "Cannot get value of implicit or invalid slot");
  തിരികെ static_cast<uint8_t>(മൂല്യം);
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
    സ്ഥിരം അക്ഷരം *MakeFlagsEnv = getenv("MAKEFLAGS");
    എങ്കിൽ (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      തിരികെ;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    എങ്കിൽ (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      തിരികെ;
    }

    JobserverConfig Config = *ConfigOrErr;
    എങ്കിൽ (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      തിരികെ;
    }

    എങ്കിൽ (Config.TheMode == JobserverConfig::PosixPipe) {
#എങ്കിൽ defined(LLVM_ON_UNIX)
      എങ്കിൽ (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        തിരികെ;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    എങ്കിൽ (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } അല്ലെങ്കിൽ
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  തിരികെ GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
ശൂന്യം JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  പുതിയ (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
