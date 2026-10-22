//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/Jobserver.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/Error.h"

#అన్తర్భావ <atomic>
#అన్తర్భావ <memory>
#అన్తర్భావ <mutex>
#అన్తర్భావ <new>

#నిర్వచించు DEBUG_TYPE "jobserver"

ఉపయోగించి నామాకాశ llvm;

నామాకాశ {
నిర్మాణం FdPair {
  పూర్ణాంకం Read = -1;
  పూర్ణాంకం Write = -1;
  తార్కిక isValid() స్థిరరాశి { తిరిగి Read >= 0 && Write >= 0; }
};

నిర్మాణం JobserverConfig {
  గణన బహుళకం {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  బహుళకం The_బహుళకం = None;
  std::string మార్గం;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
తార్కిక get_Prefixed_విలువ(StringRef ఆదానం, StringRef ఉపసర్గ, StringRef &విలువ) {
  ఒకవేళ (ఆదానం.consume_front(ఉపసర్గ)) {
    విలువ = ఆదానం;
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
స్థిర std::optional<FdPair> getFileDescriptorPair(StringRef ఆదానం) {
  FdPair FDs;
  ఒకవేళ (ఆదానం.consumeInteger(10, FDs.Read))
    తిరిగి std::nullopt;
  ఒకవేళ (!ఆదానం.consume_front(","))
    తిరిగి std::nullopt;
  ఒకవేళ (ఆదానం.consumeInteger(10, FDs.Write))
    తిరిగి std::nullopt;
  ఒకవేళ (!ఆదానం.ఖాళీలు() || !FDs.isValid())
    తిరిగి std::nullopt;
  తిరిగి FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  ఒకవేళ (MakeFlags.ఖాళీలు())
    తిరిగి Config;

  // Split the MAKEFLAGS string into arguments.
  Small_సదిశ<StringRef, 8> వాదనలు;
  విభజించు_String(MakeFlags, వాదనలు);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  ఒకవేళ (!వాదనలు.ఖాళీలు() && !వాదనలు[0].starts_with("-") && వాదనలు[0].కలిగియుంది('n'))
    తిరిగి Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  కొరకు (StringRef Arg : వాదనలు) {
    StringRef విలువ;
    ఒకవేళ (get_Prefixed_విలువ(Arg, "--jobserver-auth=", విలువ)) {
      // Try to parse as a file descriptor pair first.
      ఒకవేళ (స్వయంచాలక FDPair = getFileDescriptorPair(విలువ)) {
        Config.The_బహుళకం = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } లేకపోతే {
        StringRef Fifo_మార్గం;
        // If not FDs, try to parse as a named pipe (fifo).
        ఒకవేళ (get_Prefixed_విలువ(విలువ, "fifo:", Fifo_మార్గం)) {
          Config.The_బహుళకం = JobserverConfig::PosixFifo;
          Config.మార్గం = Fifo_మార్గం.str();
        } లేకపోతే {
          // Otherwise, assume it's a Windows semaphore.
          Config.The_బహుళకం = JobserverConfig::Win32Semaphore;
          Config.మార్గం = విలువ.str();
        }
      }
    } లేకపోతే ఒకవేళ (get_Prefixed_విలువ(Arg, "--jobserver-fds=", విలువ)) {
      // This is an alternative, older syntax for the pipe-based server.
      ఒకవేళ (స్వయంచాలక FDPair = getFileDescriptorPair(విలువ)) {
        Config.The_బహుళకం = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } లేకపోతే {
        తిరిగి createStringError(inconvertible_Error_సంకేతం(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#నిర్వచితమైతే _WIN32
  ఒకవేళ (Config.The_బహుళకం == JobserverConfig::PosixFifo ||
      Config.The_బహుళకం == JobserverConfig::PosixPipe)
    తిరిగి createStringError(
        inconvertible_Error_సంకేతం(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#లేకపోతే
  ఒకవేళ (Config.The_బహుళకం == JobserverConfig::Win32Semaphore)
    తిరిగి createStringError(
        inconvertible_Error_సంకేతం(),
        "Semaphore-based jobserver is not supported on this platform");
#అయితే_ముగింపు
  తిరిగి Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = శూన్యసూచిక;

} // namespace

నామాకాశ llvm {
తరగతి JobserverClientImpl : బహిరంగ JobserverClient {
  తార్కిక IsInitialized = అబద్ధం;
  std::atomic<తార్కిక> Has_Implicit_స్థానం{నిజం};
  గుర్తులేని అంకె_Jobs = 0;

బహిరంగ:
  JobserverClientImpl(స్థిరరాశి JobserverConfig &Config);
  ~JobserverClientImpl() override;

  పని_స్థానం tryAcquire() override;
  శూన్య release(పని_స్థానం స్థానం) override;
  గుర్తులేని get_అంకె_Jobs() స్థిరరాశి override { తిరిగి అంకె_Jobs; }

  తార్కిక isValid() స్థిరరాశి { తిరిగి IsInitialized; }

అంతరంగ:
#ఒకవేళ defined(LLVM_ON_UNIX)
  పూర్ణాంకం ReadFD = -1;
  పూర్ణాంకం WriteFD = -1;
  std::string Fifo_మార్గం;
#లేదావేళ defined(_WIN32)
  శూన్య *Semaphore = శూన్యసూచిక;
#అయితే_ముగింపు
};
} // namespace llvm

// Include the platform-specific parts of the class.
#ఒకవేళ defined(LLVM_ON_UNIX)
#అన్తర్భావ "Unix/Jobserver.inc"
#లేదావేళ defined(_WIN32)
#అన్తర్భావ "Windows/Jobserver.inc"
#లేకపోతే
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(స్థిరరాశి JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = అప్రమేయ;
పని_స్థానం JobserverClientImpl::tryAcquire() { తిరిగి పని_స్థానం(); }
శూన్య JobserverClientImpl::release(పని_స్థానం స్థానం) {}
#అయితే_ముగింపు

నామాకాశ llvm {
JobserverClient::~JobserverClient() = అప్రమేయ;

uint8_t పని_స్థానం::get_Explicit_విలువ() స్థిరరాశి {
  ప్రతిజ్ఞా(isExplicit() && "Cannot get value of implicit or invalid slot");
  తిరిగి static_cast<uint8_t>(విలువ);
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
    స్థిరరాశి అక్షరం *MakeFlagsEnv = getenv("MAKEFLAGS");
    ఒకవేళ (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      తిరిగి;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    స్వయంచాలక ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    ఒకవేళ (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << గమ్యం_String(std::move(Err)) << "\n";
      తిరిగి;
    }

    JobserverConfig Config = *ConfigOrErr;
    ఒకవేళ (Config.The_బహుళకం == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      తిరిగి;
    }

    ఒకవేళ (Config.The_బహుళకం == JobserverConfig::PosixPipe) {
#ఒకవేళ defined(LLVM_ON_UNIX)
      ఒకవేళ (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        తిరిగి;
      }
#అయితే_ముగింపు
    }

    స్వయంచాలక Client = std::make_unique<JobserverClientImpl>(Config);
    ఒకవేళ (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } లేకపోతే
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  తిరిగి GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
శూన్య JobserverClient::resetForTesting() {
  తొలగించు GJobserver;
  GJobserver = శూన్యసూచిక;
  // Re-construct the std::once_flag in place to reset the singleton state.
  కొత్త (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
