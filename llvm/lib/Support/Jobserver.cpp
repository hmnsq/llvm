//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Jobserver.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Error.h"

#شامل <atomic>
#شامل <memory>
#شامل <mutex>
#شامل <new>

#تعریف_کرو DEBUG_TYPE "jobserver"

استعمال نامخلا llvm;

نامخلا {
ساخت FdPair {
  صحیح Read = -1;
  صحیح Write = -1;
  منطقی isValid() ثابت { واپس Read >= 0 && Write >= 0; }
};

ساخت JobserverConfig {
  شمار منوال {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  منوال The_منوال = None;
  std::string راستہ;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
منطقی get_Prefixed_قدر(StringRef داخلہ, StringRef سابقہ, StringRef &قدر) {
  اگر (داخلہ.consume_front(سابقہ)) {
    قدر = داخلہ;
    واپس سچ;
  }
  واپس جھوٹ;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ساکن std::optional<FdPair> getFileDescriptorPair(StringRef داخلہ) {
  FdPair FDs;
  اگر (داخلہ.consumeInteger(10, FDs.Read))
    واپس std::nullopt;
  اگر (!داخلہ.consume_front(","))
    واپس std::nullopt;
  اگر (داخلہ.consumeInteger(10, FDs.Write))
    واپس std::nullopt;
  اگر (!داخلہ.خالی_خانے() || !FDs.isValid())
    واپس std::nullopt;
  واپس FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  اگر (MakeFlags.خالی_خانے())
    واپس Config;

  // Split the MAKEFLAGS string into arguments.
  Small_سمتیہ<StringRef, 8> دلائل;
  بانٹو_String(MakeFlags, دلائل);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  اگر (!دلائل.خالی_خانے() && !دلائل[0].starts_with("-") && دلائل[0].شامل_ہے('n'))
    واپس Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  برائے (StringRef Arg : دلائل) {
    StringRef قدر;
    اگر (get_Prefixed_قدر(Arg, "--jobserver-auth=", قدر)) {
      // Try to parse as a file descriptor pair first.
      اگر (خودکار FDPair = getFileDescriptorPair(قدر)) {
        Config.The_منوال = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ورنہ {
        StringRef Fifo_راستہ;
        // If not FDs, try to parse as a named pipe (fifo).
        اگر (get_Prefixed_قدر(قدر, "fifo:", Fifo_راستہ)) {
          Config.The_منوال = JobserverConfig::PosixFifo;
          Config.راستہ = Fifo_راستہ.str();
        } ورنہ {
          // Otherwise, assume it's a Windows semaphore.
          Config.The_منوال = JobserverConfig::Win32Semaphore;
          Config.راستہ = قدر.str();
        }
      }
    } ورنہ اگر (get_Prefixed_قدر(Arg, "--jobserver-fds=", قدر)) {
      // This is an alternative, older syntax for the pipe-based server.
      اگر (خودکار FDPair = getFileDescriptorPair(قدر)) {
        Config.The_منوال = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ورنہ {
        واپس createStringError(inconvertible_Error_رمز(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#اگر_تعریف_شدہ _WIN32
  اگر (Config.The_منوال == JobserverConfig::PosixFifo ||
      Config.The_منوال == JobserverConfig::PosixPipe)
    واپس createStringError(
        inconvertible_Error_رمز(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#ورنہ
  اگر (Config.The_منوال == JobserverConfig::Win32Semaphore)
    واپس createStringError(
        inconvertible_Error_رمز(),
        "Semaphore-based jobserver is not supported on this platform");
#اگر_ختم
  واپس Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = خالی_اشاریہ;

} // namespace

نامخلا llvm {
جماعت JobserverClientImpl : عوامی JobserverClient {
  منطقی IsInitialized = جھوٹ;
  std::atomic<منطقی> Has_Implicit_خانہ{سچ};
  غیرعلامتی عدد_Jobs = 0;

عوامی:
  JobserverClientImpl(ثابت JobserverConfig &Config);
  ~JobserverClientImpl() override;

  کام_خانہ tryAcquire() override;
  خالی release(کام_خانہ خانہ) override;
  غیرعلامتی get_عدد_Jobs() ثابت override { واپس عدد_Jobs; }

  منطقی isValid() ثابت { واپس IsInitialized; }

نجی:
#اگر defined(LLVM_ON_UNIX)
  صحیح ReadFD = -1;
  صحیح WriteFD = -1;
  std::string Fifo_راستہ;
#وگرنہ_اگر defined(_WIN32)
  خالی *Semaphore = خالی_اشاریہ;
#اگر_ختم
};
} // namespace llvm

// Include the platform-specific parts of the class.
#اگر defined(LLVM_ON_UNIX)
#شامل "Unix/Jobserver.inc"
#وگرنہ_اگر defined(_WIN32)
#شامل "Windows/Jobserver.inc"
#ورنہ
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ثابت JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = طےشدہ;
کام_خانہ JobserverClientImpl::tryAcquire() { واپس کام_خانہ(); }
خالی JobserverClientImpl::release(کام_خانہ خانہ) {}
#اگر_ختم

نامخلا llvm {
JobserverClient::~JobserverClient() = طےشدہ;

uint8_t کام_خانہ::get_Explicit_قدر() ثابت {
  أكد(isExplicit() && "Cannot get value of implicit or invalid slot");
  واپس static_cast<uint8_t>(قدر);
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
    ثابت حرف *MakeFlagsEnv = getenv("MAKEFLAGS");
    اگر (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      واپس;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    خودکار ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    اگر (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << مقصد_String(std::move(Err)) << "\n";
      واپس;
    }

    JobserverConfig Config = *ConfigOrErr;
    اگر (Config.The_منوال == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      واپس;
    }

    اگر (Config.The_منوال == JobserverConfig::PosixPipe) {
#اگر defined(LLVM_ON_UNIX)
      اگر (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        واپس;
      }
#اگر_ختم
    }

    خودکار Client = std::make_unique<JobserverClientImpl>(Config);
    اگر (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } ورنہ
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  واپس GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
خالی JobserverClient::resetForTesting() {
  حذف GJobserver;
  GJobserver = خالی_اشاریہ;
  // Re-construct the std::once_flag in place to reset the singleton state.
  نیا (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
