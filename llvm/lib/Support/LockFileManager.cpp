//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LockFileManager.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ErrorOr.h"
#ضمن "llvm/Support/ExponentialBackoff.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cerrno>
#ضمن <chrono>
#ضمن <ctime>
#ضمن <memory>
#ضمن <system_error>
#ضمن <tuple>

#ifdef _WIN32
#ضمن <windows.h>
#endif
#ܐܢ LLVM_ON_UNIX
#ضمن <unistd.h>
#endif

#ܐܢ defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#ܐܚܪܢܐ
#define USE_OSX_GETHOSTUUID 0
#endif

#ܐܢ USE_OSX_GETHOSTUUID
#ضمن <uuid/uuid.h>
#endif

using نطاق llvm;

/// Attempt to read the lock file with the given name, if it exists.
///
/// \param LockFileName The name of the lock file to read.
///
/// \returns The process ID of the process that owns this lock file
std::optional<LockFileManager::OwnedByAnother>
LockFileManager::readLockFile(StringRef LockFileName) {
  // Read the owning host and PID out of the lock file. If it appears that the
  // owning process is dead, the lock file is invalid.
  ErrorOr<std::unique_ptr<MemoryBuffer>> MBOrErr =
      MemoryBuffer::getFile(LockFileName);
  ܐܢ (!MBOrErr) {
    sys::fs::remove(LockFileName);
    ܦܢܐ std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  صحيح PID;
  ܐܢ (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    ܐܢ (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      ܦܢܐ Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  ܦܢܐ std::nullopt;
}

ساكن std::error_code getHostID(SmallVectorImpl<حرف> &HostID) {
  HostID.clear();

#ܐܢ USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  بنية timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  ܐܢ (gethostuuid(uuid, &wait) != 0)
    ܦܢܐ errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#وإلاإذا LLVM_ON_UNIX
  حرف HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#ܐܚܪܢܐ
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  ܦܢܐ std::error_code();
}

منطقي LockFileManager::processStillExecuting(StringRef HostID, صحيح PID) {
#ܐܢ LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  ܐܢ (getHostID(StoredHostID))
    ܦܢܐ ܫܪܪܐ; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  ܐܢ (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    ܦܢܐ ܕܓܠܐ;
#endif

  ܦܢܐ ܫܪܪܐ;
}

نطاق {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
صنف RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  منطقي RemoveImmediately;
عام:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(ܫܪܪܐ) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    ܐܢ (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      ܦܢܐ;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  فراغ lockAcquired() { RemoveImmediately = ܕܓܠܐ; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<منطقي> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  ܐܢ (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    ܦܢܐ createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  ܐܢ (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    ܦܢܐ ܕܓܠܐ;
  }

  // Create a lock file that is unique to this instance.
  صحيح UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    ܐܢ (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      ܐܢ (!Dir.empty()) {
        ܐܢ (std::error_code DirEC = sys::fs::create_directories(Dir))
          ܦܢܐ createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    ܐܢ (EC)
      ܦܢܐ createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    ܐܢ (auto EC = getHostID(HostID))
      ܦܢܐ createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/ܫܪܪܐ);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    ܐܢ (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      ܦܢܐ std::move(Err);
    }
  }

  ܟܡܐ (ܫܪܪܐ) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    ܐܢ (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      ܦܢܐ ܫܪܪܐ;
    }

    ܐܢ (EC != errc::file_exists)
      ܦܢܐ createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    ܐܢ (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      ܦܢܐ ܕܓܠܐ;
    }

    ܐܢ (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      استمر;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    ܐܢ ((EC = sys::fs::remove(LockFileName)))
      ܦܢܐ createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ܐܢ (!std::holds_alternative<OwnedByUs>(Owner))
    ܦܢܐ;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::remove(LockFileName);
  sys::fs::remove(UniqueLockFileName);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(UniqueLockFileName);
}

WaitForUnlockResult
LockFileManager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  auto *LockFileOwner = std::get_if<OwnedByAnother>(&Owner);
  أكد(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using نطاق std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  ܟܡܐ (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    ܐܢ (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      ܦܢܐ WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    ܐܢ (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      ܦܢܐ WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  ܦܢܐ WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ܦܢܐ sys::fs::remove(LockFileName);
}
