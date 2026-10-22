//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/Jobserver.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Error.h"

#अन्तर्भाव <atomic>
#अन्तर्भाव <memory>
#अन्तर्भाव <mutex>
#अन्तर्भाव <new>

#define DEBUG_TYPE "jobserver"

using नामाकाश llvm;

नामाकाश {
संरचना FdPair {
  पूर्णाङ्क Read = -1;
  पूर्णाङ्क Write = -1;
  तार्किक isValid() अचर { फर्काऊ Read >= 0 && Write >= 0; }
};

संरचना JobserverConfig {
  गणना Mode {
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
तार्किक get_Prefixed_मान(StringRef आगत, StringRef Prefix, StringRef &मान) {
  यदि (आगत.consume_front(Prefix)) {
    मान = आगत;
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
स्थिर std::optional<FdPair> getFileDescriptorPair(StringRef आगत) {
  FdPair FDs;
  यदि (आगत.consumeInteger(10, FDs.Read))
    फर्काऊ std::nullopt;
  यदि (!आगत.consume_front(","))
    फर्काऊ std::nullopt;
  यदि (आगत.consumeInteger(10, FDs.Write))
    फर्काऊ std::nullopt;
  यदि (!आगत.empty() || !FDs.isValid())
    फर्काऊ std::nullopt;
  फर्काऊ FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  यदि (MakeFlags.empty())
    फर्काऊ Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  यदि (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    फर्काऊ Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  लागि (StringRef Arg : Args) {
    StringRef मान;
    यदि (get_Prefixed_मान(Arg, "--jobserver-auth=", मान)) {
      // Try to parse as a file descriptor pair first.
      यदि (स्वतः FDPair = getFileDescriptorPair(मान)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        यदि (get_Prefixed_मान(मान, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } अन्यथा {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = मान.str();
        }
      }
    } अन्यथा यदि (get_Prefixed_मान(Arg, "--jobserver-fds=", मान)) {
      // This is an alternative, older syntax for the pipe-based server.
      यदि (स्वतः FDPair = getFileDescriptorPair(मान)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        फर्काऊ createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  यदि (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    फर्काऊ createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#अन्यथा
  यदि (Config.TheMode == JobserverConfig::Win32Semaphore)
    फर्काऊ createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  फर्काऊ Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

नामाकाश llvm {
वर्ग JobserverClientImpl : सार्वजनिक JobserverClient {
  तार्किक IsInitialized = असत्य;
  std::atomic<तार्किक> HasImplicitSlot{सत्य};
  अचिन्हित अङ्क_Jobs = 0;

सार्वजनिक:
  JobserverClientImpl(अचर JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  शून्य release(JobSlot Slot) override;
  अचिन्हित get_अङ्क_Jobs() अचर override { फर्काऊ अङ्क_Jobs; }

  तार्किक isValid() अचर { फर्काऊ IsInitialized; }

गुप्त:
#यदि defined(LLVM_ON_UNIX)
  पूर्णाङ्क ReadFD = -1;
  पूर्णाङ्क WriteFD = -1;
  std::string FifoPath;
#अथवायदि defined(_WIN32)
  शून्य *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#यदि defined(LLVM_ON_UNIX)
#अन्तर्भाव "Unix/Jobserver.inc"
#अथवायदि defined(_WIN32)
#अन्तर्भाव "Windows/Jobserver.inc"
#अन्यथा
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(अचर JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = स्वभाव;
JobSlot JobserverClientImpl::tryAcquire() { फर्काऊ JobSlot(); }
शून्य JobserverClientImpl::release(JobSlot Slot) {}
#endif

नामाकाश llvm {
JobserverClient::~JobserverClient() = स्वभाव;

uint8_t JobSlot::get_Explicit_मान() अचर {
  प्रतिज्ञा(isExplicit() && "Cannot get value of implicit or invalid slot");
  फर्काऊ static_cast<uint8_t>(मान);
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
    अचर वर्ण *MakeFlagsEnv = दो_पर्या("MAKEFLAGS");
    यदि (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      फर्काऊ;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    स्वतः ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    यदि (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      फर्काऊ;
    }

    JobserverConfig Config = *ConfigOrErr;
    यदि (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      फर्काऊ;
    }

    यदि (Config.TheMode == JobserverConfig::PosixPipe) {
#यदि defined(LLVM_ON_UNIX)
      यदि (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        फर्काऊ;
      }
#endif
    }

    स्वतः Client = std::make_unique<JobserverClientImpl>(Config);
    यदि (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } अन्यथा
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  फर्काऊ GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
शून्य JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  नूतन (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
