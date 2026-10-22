//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/Jobserver.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Error.h"

#ضمن <atomic>
#ضمن <memory>
#ضمن <mutex>
#ضمن <new>

#تعریف_کرو DEBUG_TYPE "jobserver"

ورتوں نطاق llvm;

نطاق {
بنية FdPair {
  صحيح Read = -1;
  صحيح Write = -1;
  منطقي isValid() ثابت { أرجع Read >= 0 && Write >= 0; }
};

بنية JobserverConfig {
  تعداد منوال {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  منوال The_منوال = None;
  std::string رستہ;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
منطقي get_Prefixed_قدر(StringRef داخلہ, StringRef اگیتر, StringRef &قدر) {
  إذا (داخلہ.consume_front(اگیتر)) {
    قدر = داخلہ;
    أرجع سچ;
  }
  أرجع خطأ;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ساكن std::optional<FdPair> getFileDescriptorPair(StringRef داخلہ) {
  FdPair FDs;
  إذا (داخلہ.consumeInteger(10, FDs.Read))
    أرجع std::nullopt;
  إذا (!داخلہ.consume_front(","))
    أرجع std::nullopt;
  إذا (داخلہ.consumeInteger(10, FDs.Write))
    أرجع std::nullopt;
  إذا (!داخلہ.خالی_تھاواں() || !FDs.isValid())
    أرجع std::nullopt;
  أرجع FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  إذا (MakeFlags.خالی_تھاواں())
    أرجع Config;

  // Split the MAKEFLAGS string into arguments.
  Small_سمتیہ<StringRef, 8> دلیلاں;
  ونڈو_String(MakeFlags, دلیلاں);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  إذا (!دلیلاں.خالی_تھاواں() && !دلیلاں[0].starts_with("-") && دلیلاں[0].شامل_اے('n'))
    أرجع Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  لكل (StringRef Arg : دلیلاں) {
    StringRef قدر;
    إذا (get_Prefixed_قدر(Arg, "--jobserver-auth=", قدر)) {
      // Try to parse as a file descriptor pair first.
      إذا (خودکار FDPair = getFileDescriptorPair(قدر)) {
        Config.The_منوال = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } وإلا {
        StringRef Fifo_رستہ;
        // If not FDs, try to parse as a named pipe (fifo).
        إذا (get_Prefixed_قدر(قدر, "fifo:", Fifo_رستہ)) {
          Config.The_منوال = JobserverConfig::PosixFifo;
          Config.رستہ = Fifo_رستہ.str();
        } وإلا {
          // Otherwise, assume it's a Windows semaphore.
          Config.The_منوال = JobserverConfig::Win32Semaphore;
          Config.رستہ = قدر.str();
        }
      }
    } وإلا إذا (get_Prefixed_قدر(Arg, "--jobserver-fds=", قدر)) {
      // This is an alternative, older syntax for the pipe-based server.
      إذا (خودکار FDPair = getFileDescriptorPair(قدر)) {
        Config.The_منوال = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } وإلا {
        أرجع createStringError(inconvertible_Error_کوڈ(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#جے_تعریف_شدہ _WIN32
  إذا (Config.The_منوال == JobserverConfig::PosixFifo ||
      Config.The_منوال == JobserverConfig::PosixPipe)
    أرجع createStringError(
        inconvertible_Error_کوڈ(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#وإلا
  إذا (Config.The_منوال == JobserverConfig::Win32Semaphore)
    أرجع createStringError(
        inconvertible_Error_کوڈ(),
        "Semaphore-based jobserver is not supported on this platform");
#جے_انت
  أرجع Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = خالی_سوچک;

} // namespace

نطاق llvm {
صنف JobserverClientImpl : عام JobserverClient {
  منطقي IsInitialized = خطأ;
  std::atomic<منطقي> Has_Implicit_تھاں{سچ};
  بنانشانی عدد_Jobs = 0;

عام:
  JobserverClientImpl(ثابت JobserverConfig &Config);
  ~JobserverClientImpl() override;

  کم_تھاں tryAcquire() override;
  فراغ release(کم_تھاں تھاں) override;
  بنانشانی get_عدد_Jobs() ثابت override { أرجع عدد_Jobs; }

  منطقي isValid() ثابت { أرجع IsInitialized; }

خاص:
#إذا defined(LLVM_ON_UNIX)
  صحيح ReadFD = -1;
  صحيح WriteFD = -1;
  std::string Fifo_رستہ;
#وإلاإذا defined(_WIN32)
  فراغ *Semaphore = خالی_سوچک;
#جے_انت
};
} // namespace llvm

// Include the platform-specific parts of the class.
#إذا defined(LLVM_ON_UNIX)
#ضمن "Unix/Jobserver.inc"
#وإلاإذا defined(_WIN32)
#ضمن "Windows/Jobserver.inc"
#وإلا
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ثابت JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = افتراضي;
کم_تھاں JobserverClientImpl::tryAcquire() { أرجع کم_تھاں(); }
فراغ JobserverClientImpl::release(کم_تھاں تھاں) {}
#جے_انت

نطاق llvm {
JobserverClient::~JobserverClient() = افتراضي;

uint8_t کم_تھاں::get_Explicit_قدر() ثابت {
  أكد(isExplicit() && "Cannot get value of implicit or invalid slot");
  أرجع static_cast<uint8_t>(قدر);
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
    إذا (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      أرجع;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    خودکار ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    إذا (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << منزل_String(std::move(Err)) << "\n";
      أرجع;
    }

    JobserverConfig Config = *ConfigOrErr;
    إذا (Config.The_منوال == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      أرجع;
    }

    إذا (Config.The_منوال == JobserverConfig::PosixPipe) {
#إذا defined(LLVM_ON_UNIX)
      إذا (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        أرجع;
      }
#جے_انت
    }

    خودکار Client = std::make_unique<JobserverClientImpl>(Config);
    إذا (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } وإلا
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  أرجع GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
فراغ JobserverClient::resetForTesting() {
  مٹاؤ GJobserver;
  GJobserver = خالی_سوچک;
  // Re-construct the std::once_flag in place to reset the singleton state.
  جديد (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
