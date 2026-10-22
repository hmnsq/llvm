//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/Jobserver.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/Error.h"

#समावेश <atomic>
#समावेश <memory>
#समावेश <mutex>
#समावेश <new>

#घोषणा DEBUG_TYPE "jobserver"

using namespace llvm;

namespace {
काष्ठा FdPair {
  पूर्णांक Read = -1;
  पूर्णांक Write = -1;
  bool isValid() स्थिर { वापस Read >= 0 && Write >= 0; }
};

काष्ठा JobserverConfig {
  क्रमागत Mode {
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
bool get_Prefixed_मूल्य(StringRef निवेश, StringRef Prefix, StringRef &मूल्य) {
  अगर (निवेश.consume_front(Prefix)) {
    मूल्य = निवेश;
    वापस true;
  }
  वापस false;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
अटल std::optional<FdPair> getFileDescriptorPair(StringRef निवेश) {
  FdPair FDs;
  अगर (निवेश.consumeInteger(10, FDs.Read))
    वापस std::nullopt;
  अगर (!निवेश.consume_front(","))
    वापस std::nullopt;
  अगर (निवेश.consumeInteger(10, FDs.Write))
    वापस std::nullopt;
  अगर (!निवेश.empty() || !FDs.isValid())
    वापस std::nullopt;
  वापस FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  अगर (MakeFlags.empty())
    वापस Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  अगर (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    वापस Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  क्रम (StringRef Arg : Args) {
    StringRef मूल्य;
    अगर (get_Prefixed_मूल्य(Arg, "--jobserver-auth=", मूल्य)) {
      // Try to parse as a file descriptor pair first.
      अगर (स्वतः FDPair = getFileDescriptorPair(मूल्य)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        अगर (get_Prefixed_मूल्य(मूल्य, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } अन्यथा {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = मूल्य.str();
        }
      }
    } अन्यथा अगर (get_Prefixed_मूल्य(Arg, "--jobserver-fds=", मूल्य)) {
      // This is an alternative, older syntax for the pipe-based server.
      अगर (स्वतः FDPair = getFileDescriptorPair(मूल्य)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        वापस createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#अगर_घोषित _WIN32
  अगर (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    वापस createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#अन्यथा
  अगर (Config.TheMode == JobserverConfig::Win32Semaphore)
    वापस createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#पूर्ण_अगर
  वापस Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

namespace llvm {
श्रेणी JobserverClientImpl : खुला JobserverClient {
  bool IsInitialized = false;
  std::atomic<bool> HasImplicitSlot{true};
  अचिन्हित अंक_Jobs = 0;

खुला:
  JobserverClientImpl(स्थिर JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  व्योम release(JobSlot Slot) override;
  अचिन्हित get_अंक_Jobs() स्थिर override { वापस अंक_Jobs; }

  bool isValid() स्थिर { वापस IsInitialized; }

निजी:
#अगर defined(LLVM_ON_UNIX)
  पूर्णांक ReadFD = -1;
  पूर्णांक WriteFD = -1;
  std::string FifoPath;
#या_अगर defined(_WIN32)
  व्योम *Semaphore = nullptr;
#पूर्ण_अगर
};
} // namespace llvm

// Include the platform-specific parts of the class.
#अगर defined(LLVM_ON_UNIX)
#समावेश "Unix/Jobserver.inc"
#या_अगर defined(_WIN32)
#समावेश "Windows/Jobserver.inc"
#अन्यथा
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(स्थिर JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = शेष;
JobSlot JobserverClientImpl::tryAcquire() { वापस JobSlot(); }
व्योम JobserverClientImpl::release(JobSlot Slot) {}
#पूर्ण_अगर

namespace llvm {
JobserverClient::~JobserverClient() = शेष;

uint8_t JobSlot::get_Explicit_मूल्य() स्थिर {
  निश्चित(isExplicit() && "Cannot get value of implicit or invalid slot");
  वापस static_cast<uint8_t>(मूल्य);
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
    स्थिर अक्षर *MakeFlagsEnv = दो_पर्या("MAKEFLAGS");
    अगर (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      वापस;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    स्वतः ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    अगर (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      वापस;
    }

    JobserverConfig Config = *ConfigOrErr;
    अगर (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      वापस;
    }

    अगर (Config.TheMode == JobserverConfig::PosixPipe) {
#अगर defined(LLVM_ON_UNIX)
      अगर (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        वापस;
      }
#पूर्ण_अगर
    }

    स्वतः Client = std::make_unique<JobserverClientImpl>(Config);
    अगर (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } अन्यथा
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  वापस GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
व्योम JobserverClient::resetForTesting() {
  मिटाओ GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  नया (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
