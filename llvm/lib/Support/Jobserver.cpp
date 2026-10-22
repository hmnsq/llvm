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

#परिभाषय DEBUG_TYPE "jobserver"

उपयुज्य नामाकाश llvm;

नामाकाश {
संरचना FdPair {
  पूर्णाङ्क Read = -1;
  पूर्णाङ्क Write = -1;
  तार्किक isValid() ध्रुव { प्रत्यागम Read >= 0 && Write >= 0; }
};

संरचना JobserverConfig {
  गणना बहुलकम् {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  बहुलकम् The_बहुलकम् = None;
  std::string मार्गः;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
तार्किक get_Prefixed_मानम्(StringRef निवेशः, StringRef उपसर्गः, StringRef &मानम्) {
  यदि (निवेशः.consume_front(उपसर्गः)) {
    मानम् = निवेशः;
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
स्थिर std::optional<FdPair> getFileDescriptorPair(StringRef निवेशः) {
  FdPair FDs;
  यदि (निवेशः.consumeInteger(10, FDs.Read))
    प्रत्यागम std::nullopt;
  यदि (!निवेशः.consume_front(","))
    प्रत्यागम std::nullopt;
  यदि (निवेशः.consumeInteger(10, FDs.Write))
    प्रत्यागम std::nullopt;
  यदि (!निवेशः.रिक्तानि() || !FDs.isValid())
    प्रत्यागम std::nullopt;
  प्रत्यागम FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  यदि (MakeFlags.रिक्तानि())
    प्रत्यागम Config;

  // Split the MAKEFLAGS string into arguments.
  Small_सदिशम्<StringRef, 8> तर्काः;
  विभज_String(MakeFlags, तर्काः);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  यदि (!तर्काः.रिक्तानि() && !तर्काः[0].starts_with("-") && तर्काः[0].अन्तर्भवति('n'))
    प्रत्यागम Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  प्रति (StringRef Arg : तर्काः) {
    StringRef मानम्;
    यदि (get_Prefixed_मानम्(Arg, "--jobserver-auth=", मानम्)) {
      // Try to parse as a file descriptor pair first.
      यदि (स्वचालित FDPair = getFileDescriptorPair(मानम्)) {
        Config.The_बहुलकम् = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        StringRef Fifo_मार्गः;
        // If not FDs, try to parse as a named pipe (fifo).
        यदि (get_Prefixed_मानम्(मानम्, "fifo:", Fifo_मार्गः)) {
          Config.The_बहुलकम् = JobserverConfig::PosixFifo;
          Config.मार्गः = Fifo_मार्गः.str();
        } अन्यथा {
          // Otherwise, assume it's a Windows semaphore.
          Config.The_बहुलकम् = JobserverConfig::Win32Semaphore;
          Config.मार्गः = मानम्.str();
        }
      }
    } अन्यथा यदि (get_Prefixed_मानम्(Arg, "--jobserver-fds=", मानम्)) {
      // This is an alternative, older syntax for the pipe-based server.
      यदि (स्वचालित FDPair = getFileDescriptorPair(मानम्)) {
        Config.The_बहुलकम् = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        प्रत्यागम createStringError(inconvertible_Error_संहिता(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#यदि_परिभाषितम् _WIN32
  यदि (Config.The_बहुलकम् == JobserverConfig::PosixFifo ||
      Config.The_बहुलकम् == JobserverConfig::PosixPipe)
    प्रत्यागम createStringError(
        inconvertible_Error_संहिता(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#अन्यथा
  यदि (Config.The_बहुलकम् == JobserverConfig::Win32Semaphore)
    प्रत्यागम createStringError(
        inconvertible_Error_संहिता(),
        "Semaphore-based jobserver is not supported on this platform");
#यदि_अन्तः
  प्रत्यागम Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = शून्यसूचकः;

} // namespace

नामाकाश llvm {
वर्ग JobserverClientImpl : सार्वजनिक JobserverClient {
  तार्किक IsInitialized = असत्य;
  std::atomic<तार्किक> HasImplicitSlot{सत्य};
  अचिह्नित अङ्कः_Jobs = 0;

सार्वजनिक:
  JobserverClientImpl(ध्रुव JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  शून्य release(JobSlot Slot) override;
  अचिह्नित get_अङ्कः_Jobs() ध्रुव override { प्रत्यागम अङ्कः_Jobs; }

  तार्किक isValid() ध्रुव { प्रत्यागम IsInitialized; }

गुप्त:
#यदि defined(LLVM_ON_UNIX)
  पूर्णाङ्क ReadFD = -1;
  पूर्णाङ्क WriteFD = -1;
  std::string Fifo_मार्गः;
#नोचेद्यदि defined(_WIN32)
  शून्य *Semaphore = शून्यसूचकः;
#यदि_अन्तः
};
} // namespace llvm

// Include the platform-specific parts of the class.
#यदि defined(LLVM_ON_UNIX)
#अन्तर्भाव "Unix/Jobserver.inc"
#नोचेद्यदि defined(_WIN32)
#अन्तर्भाव "Windows/Jobserver.inc"
#अन्यथा
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ध्रुव JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = स्वभाव;
JobSlot JobserverClientImpl::tryAcquire() { प्रत्यागम JobSlot(); }
शून्य JobserverClientImpl::release(JobSlot Slot) {}
#यदि_अन्तः

नामाकाश llvm {
JobserverClient::~JobserverClient() = स्वभाव;

uint8_t JobSlot::get_Explicit_मानम्() ध्रुव {
  प्रतिज्ञा(isExplicit() && "Cannot get value of implicit or invalid slot");
  प्रत्यागम static_cast<uint8_t>(मानम्);
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
    ध्रुव वर्ण *MakeFlagsEnv = getenv("MAKEFLAGS");
    यदि (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      प्रत्यागम;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    स्वचालित ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    यदि (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << गन्तव्यम्_String(std::move(Err)) << "\n";
      प्रत्यागम;
    }

    JobserverConfig Config = *ConfigOrErr;
    यदि (Config.The_बहुलकम् == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      प्रत्यागम;
    }

    यदि (Config.The_बहुलकम् == JobserverConfig::PosixPipe) {
#यदि defined(LLVM_ON_UNIX)
      यदि (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        प्रत्यागम;
      }
#यदि_अन्तः
    }

    स्वचालित Client = std::make_unique<JobserverClientImpl>(Config);
    यदि (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } अन्यथा
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  प्रत्यागम GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
शून्य JobserverClient::resetForTesting() {
  विलोपय GJobserver;
  GJobserver = शून्यसूचकः;
  // Re-construct the std::once_flag in place to reset the singleton state.
  नूतन (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
