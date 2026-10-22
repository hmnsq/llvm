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
  정수 Read = -1;
  정수 Write = -1;
  bool isValid() const { 반환 Read >= 0 && Write >= 0; }
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
  만약 (Input.consume_front(Prefix)) {
    Value = Input;
    반환 true;
  }
  반환 false;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
static std::optional<FdPair> getFileDescriptorPair(StringRef Input) {
  FdPair FDs;
  만약 (Input.consumeInteger(10, FDs.Read))
    반환 std::nullopt;
  만약 (!Input.consume_front(","))
    반환 std::nullopt;
  만약 (Input.consumeInteger(10, FDs.Write))
    반환 std::nullopt;
  만약 (!Input.empty() || !FDs.isValid())
    반환 std::nullopt;
  반환 FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  만약 (MakeFlags.empty())
    반환 Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  만약 (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    반환 Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  위해 (StringRef Arg : Args) {
    StringRef Value;
    만약 (getPrefixedValue(Arg, "--jobserver-auth=", Value)) {
      // Try to parse as a file descriptor pair first.
      만약 (auto FDPair = getFileDescriptorPair(Value)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } 아니면 {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        만약 (getPrefixedValue(Value, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } 아니면 {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = Value.str();
        }
      }
    } 아니면 만약 (getPrefixedValue(Arg, "--jobserver-fds=", Value)) {
      // This is an alternative, older syntax for the pipe-based server.
      만약 (auto FDPair = getFileDescriptorPair(Value)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } 아니면 {
        반환 createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  만약 (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    반환 createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#아니면
  만약 (Config.TheMode == JobserverConfig::Win32Semaphore)
    반환 createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  반환 Config;
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
  unsigned getNumJobs() const override { 반환 NumJobs; }

  bool isValid() const { 반환 IsInitialized; }

private:
#만약 defined(LLVM_ON_UNIX)
  정수 ReadFD = -1;
  정수 WriteFD = -1;
  std::string FifoPath;
#elif defined(_WIN32)
  void *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#만약 defined(LLVM_ON_UNIX)
#include "Unix/Jobserver.inc"
#elif defined(_WIN32)
#include "Windows/Jobserver.inc"
#아니면
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(const JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = default;
JobSlot JobserverClientImpl::tryAcquire() { 반환 JobSlot(); }
void JobserverClientImpl::release(JobSlot Slot) {}
#endif

namespace llvm {
JobserverClient::~JobserverClient() = default;

uint8_t JobSlot::getExplicitValue() const {
  assert(isExplicit() && "Cannot get value of implicit or invalid slot");
  반환 static_cast<uint8_t>(Value);
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
    만약 (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      반환;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    만약 (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      반환;
    }

    JobserverConfig Config = *ConfigOrErr;
    만약 (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      반환;
    }

    만약 (Config.TheMode == JobserverConfig::PosixPipe) {
#만약 defined(LLVM_ON_UNIX)
      만약 (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        반환;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    만약 (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } 아니면
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  반환 GJobserver;
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
