//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Jobserver.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Error.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <new>

#define DEBUG_TYPE "jobserver"

using namespace llvm;

namespace {
struct FdPair {
  цео Read = -1;
  цео Write = -1;
  bool isValid() const { врати Read >= 0 && Write >= 0; }
};

struct JobserverConfig {
  enum Mode {
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
bool getPrefixedValue(StringRef Input, StringRef Prefix, StringRef &Value) {
  ако (Input.consume_front(Prefix)) {
    Value = Input;
    врати true;
  }
  врати false;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
static std::optional<FdPair> getFileDescriptorPair(StringRef Input) {
  FdPair FDs;
  ако (Input.consumeInteger(10, FDs.Read))
    врати std::nullopt;
  ако (!Input.consume_front(","))
    врати std::nullopt;
  ако (Input.consumeInteger(10, FDs.Write))
    врати std::nullopt;
  ако (!Input.empty() || !FDs.isValid())
    врати std::nullopt;
  врати FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  ако (MakeFlags.empty())
    врати Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  ако (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    врати Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  за (StringRef Arg : Args) {
    StringRef Value;
    ако (getPrefixedValue(Arg, "--jobserver-auth=", Value)) {
      // Try to parse as a file descriptor pair first.
      ако (auto FDPair = getFileDescriptorPair(Value)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } инако {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        ако (getPrefixedValue(Value, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } инако {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = Value.str();
        }
      }
    } инако ако (getPrefixedValue(Arg, "--jobserver-fds=", Value)) {
      // This is an alternative, older syntax for the pipe-based server.
      ако (auto FDPair = getFileDescriptorPair(Value)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } инако {
        врати createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  ако (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    врати createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#инако
  ако (Config.TheMode == JobserverConfig::Win32Semaphore)
    врати createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  врати Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

namespace llvm {
class JobserverClientImpl : public JobserverClient {
  bool IsInitialized = false;
  std::atomic<bool> HasImplicitSlot{true};
  unsigned NumJobs = 0;

public:
  JobserverClientImpl(const JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  void release(JobSlot Slot) override;
  unsigned getNumJobs() const override { врати NumJobs; }

  bool isValid() const { врати IsInitialized; }

private:
#ако defined(LLVM_ON_UNIX)
  цео ReadFD = -1;
  цео WriteFD = -1;
  std::string FifoPath;
#elif defined(_WIN32)
  void *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#ако defined(LLVM_ON_UNIX)
#include "Unix/Jobserver.inc"
#elif defined(_WIN32)
#include "Windows/Jobserver.inc"
#инако
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(const JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = default;
JobSlot JobserverClientImpl::tryAcquire() { врати JobSlot(); }
void JobserverClientImpl::release(JobSlot Slot) {}
#endif

namespace llvm {
JobserverClient::~JobserverClient() = default;

uint8_t JobSlot::getExplicitValue() const {
  assert(isExplicit() && "Cannot get value of implicit or invalid slot");
  врати static_cast<uint8_t>(Value);
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
    const char *MakeFlagsEnv = getenv("MAKEFLAGS");
    ако (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      врати;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    ако (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      врати;
    }

    JobserverConfig Config = *ConfigOrErr;
    ако (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      врати;
    }

    ако (Config.TheMode == JobserverConfig::PosixPipe) {
#ако defined(LLVM_ON_UNIX)
      ако (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        врати;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    ако (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } инако
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  врати GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
void JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  new (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
