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
  int Read = -1;
  int Write = -1;
  bool isValid() const { return Read >= 0 && Write >= 0; }
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
bool get_Prefixed_हालागा(StringRef पासोक, StringRef Prefix, StringRef &हालागा) {
  if (पासोक.consume_front(Prefix)) {
    हालागा = पासोक;
    return true;
  }
  return false;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
static std::optional<FdPair> getFileDescriptorPair(StringRef पासोक) {
  FdPair FDs;
  if (पासोक.consumeInteger(10, FDs.Read))
    return std::nullopt;
  if (!पासोक.consume_front(","))
    return std::nullopt;
  if (पासोक.consumeInteger(10, FDs.Write))
    return std::nullopt;
  if (!पासोक.empty() || !FDs.isValid())
    return std::nullopt;
  return FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  if (MakeFlags.empty())
    return Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> मगा_कातविरान;
  SplitString(MakeFlags, मगा_कातविरान);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  if (!मगा_कातविरान.empty() && !मगा_कातविरान[0].starts_with("-") && मगा_कातविरान[0].contains('n'))
    return Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  for (StringRef Arg : मगा_कातविरान) {
    StringRef हालागा;
    if (get_Prefixed_हालागा(Arg, "--jobserver-auth=", हालागा)) {
      // Try to parse as a file descriptor pair first.
      if (auto FDPair = getFileDescriptorPair(हालागा)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } else {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        if (get_Prefixed_हालागा(हालागा, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } else {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = हालागा.str();
        }
      }
    } else if (get_Prefixed_हालागा(Arg, "--jobserver-fds=", हालागा)) {
      // This is an alternative, older syntax for the pipe-based server.
      if (auto FDPair = getFileDescriptorPair(हालागा)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } else {
        return createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  if (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    return createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#else
  if (Config.TheMode == JobserverConfig::Win32Semaphore)
    return createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  return Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

namespace llvm {
class JobserverClientImpl : public JobserverClient {
  bool IsInitialized = false;
  std::atomic<bool> HasImplicitSlot{true};
  unsigned नुमेरो_Jobs = 0;

public:
  JobserverClientImpl(const JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  void release(JobSlot Slot) override;
  unsigned get_नुमेरो_Jobs() const override { return नुमेरो_Jobs; }

  bool isValid() const { return IsInitialized; }

private:
#if defined(LLVM_ON_UNIX)
  int ReadFD = -1;
  int WriteFD = -1;
  std::string FifoPath;
#elif defined(_WIN32)
  void *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#if defined(LLVM_ON_UNIX)
#include "Unix/Jobserver.inc"
#elif defined(_WIN32)
#include "Windows/Jobserver.inc"
#else
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(const JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = default;
JobSlot JobserverClientImpl::tryAcquire() { return JobSlot(); }
void JobserverClientImpl::release(JobSlot Slot) {}
#endif

namespace llvm {
JobserverClient::~JobserverClient() = default;

uint8_t JobSlot::get_Explicit_हालागा() const {
  assert(isExplicit() && "Cannot get value of implicit or invalid slot");
  return static_cast<uint8_t>(हालागा);
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
    if (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      return;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    if (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      return;
    }

    JobserverConfig Config = *ConfigOrErr;
    if (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      return;
    }

    if (Config.TheMode == JobserverConfig::PosixPipe) {
#if defined(LLVM_ON_UNIX)
      if (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        return;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    if (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } else
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  return GJobserver;
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
