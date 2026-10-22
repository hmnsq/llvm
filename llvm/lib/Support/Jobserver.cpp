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

#تعریف_کن DEBUG_TYPE "jobserver"

استفاده نطاق llvm;

نطاق {
بنية FdPair {
  صحيح Read = -1;
  صحيح Write = -1;
  منطقي isValid() ثابت { أرجع Read >= 0 && Write >= 0; }
};

بنية JobserverConfig {
  تعداد نما {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  نما The_نما = None;
  std::string مسیر;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
منطقي get_Prefixed_مقدار(StringRef درون_داد, StringRef پیشوند, StringRef &مقدار) {
  إذا (درون_داد.consume_front(پیشوند)) {
    مقدار = درون_داد;
    أرجع درست;
  }
  أرجع خطأ;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ساكن std::optional<FdPair> getFileDescriptorPair(StringRef درون_داد) {
  FdPair FDs;
  إذا (درون_داد.consumeInteger(10, FDs.Read))
    أرجع std::nullopt;
  إذا (!درون_داد.consume_front(","))
    أرجع std::nullopt;
  إذا (درون_داد.consumeInteger(10, FDs.Write))
    أرجع std::nullopt;
  إذا (!درون_داد.خالی() || !FDs.isValid())
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
  إذا (MakeFlags.خالی())
    أرجع Config;

  // Split the MAKEFLAGS string into arguments.
  Small_وکتور<StringRef, 8> آرگومانها;
  تقسیم_String(MakeFlags, آرگومانها);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  إذا (!آرگومانها.خالی() && !آرگومانها[0].starts_with("-") && آرگومانها[0].شامل_است('n'))
    أرجع Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  لكل (StringRef Arg : آرگومانها) {
    StringRef مقدار;
    إذا (get_Prefixed_مقدار(Arg, "--jobserver-auth=", مقدار)) {
      // Try to parse as a file descriptor pair first.
      إذا (خودکار FDPair = getFileDescriptorPair(مقدار)) {
        Config.The_نما = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } وإلا {
        StringRef Fifo_مسیر;
        // If not FDs, try to parse as a named pipe (fifo).
        إذا (get_Prefixed_مقدار(مقدار, "fifo:", Fifo_مسیر)) {
          Config.The_نما = JobserverConfig::PosixFifo;
          Config.مسیر = Fifo_مسیر.str();
        } وإلا {
          // Otherwise, assume it's a Windows semaphore.
          Config.The_نما = JobserverConfig::Win32Semaphore;
          Config.مسیر = مقدار.str();
        }
      }
    } وإلا إذا (get_Prefixed_مقدار(Arg, "--jobserver-fds=", مقدار)) {
      // This is an alternative, older syntax for the pipe-based server.
      إذا (خودکار FDPair = getFileDescriptorPair(مقدار)) {
        Config.The_نما = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } وإلا {
        أرجع createStringError(inconvertible_Error_کد(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#اگر_تعریف_شده _WIN32
  إذا (Config.The_نما == JobserverConfig::PosixFifo ||
      Config.The_نما == JobserverConfig::PosixPipe)
    أرجع createStringError(
        inconvertible_Error_کد(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#وإلا
  إذا (Config.The_نما == JobserverConfig::Win32Semaphore)
    أرجع createStringError(
        inconvertible_Error_کد(),
        "Semaphore-based jobserver is not supported on this platform");
#پایان_اگر
  أرجع Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = اشاره_تهی;

} // namespace

نطاق llvm {
صنف JobserverClientImpl : عام JobserverClient {
  منطقي IsInitialized = خطأ;
  std::atomic<منطقي> Has_Implicit_جایگاه{درست};
  بیعلامت رقم_Jobs = 0;

عام:
  JobserverClientImpl(ثابت JobserverConfig &Config);
  ~JobserverClientImpl() override;

  کار_جایگاه tryAcquire() override;
  فراغ release(کار_جایگاه جایگاه) override;
  بیعلامت get_رقم_Jobs() ثابت override { أرجع رقم_Jobs; }

  منطقي isValid() ثابت { أرجع IsInitialized; }

خاص:
#إذا defined(LLVM_ON_UNIX)
  صحيح ReadFD = -1;
  صحيح WriteFD = -1;
  std::string Fifo_مسیر;
#وإلاإذا defined(_WIN32)
  فراغ *Semaphore = اشاره_تهی;
#پایان_اگر
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
کار_جایگاه JobserverClientImpl::tryAcquire() { أرجع کار_جایگاه(); }
فراغ JobserverClientImpl::release(کار_جایگاه جایگاه) {}
#پایان_اگر

نطاق llvm {
JobserverClient::~JobserverClient() = افتراضي;

uint8_t کار_جایگاه::get_Explicit_مقدار() ثابت {
  أكد(isExplicit() && "Cannot get value of implicit or invalid slot");
  أرجع static_cast<uint8_t>(مقدار);
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
             << مقصد_String(std::move(Err)) << "\n";
      أرجع;
    }

    JobserverConfig Config = *ConfigOrErr;
    إذا (Config.The_نما == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      أرجع;
    }

    إذا (Config.The_نما == JobserverConfig::PosixPipe) {
#إذا defined(LLVM_ON_UNIX)
      إذا (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        أرجع;
      }
#پایان_اگر
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
  حذف GJobserver;
  GJobserver = اشاره_تهی;
  // Re-construct the std::once_flag in place to reset the singleton state.
  جديد (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
