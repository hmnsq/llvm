//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/LockFileManager.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ErrorOr.h"
#अन्तर्भाव "llvm/Support/ExponentialBackoff.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cerrno>
#अन्तर्भाव <chrono>
#अन्तर्भाव <ctime>
#अन्तर्भाव <memory>
#अन्तर्भाव <system_error>
#अन्तर्भाव <tuple>

#ifdef _WIN32
#अन्तर्भाव <windows.h>
#endif
#सचे LLVM_ON_UNIX
#अन्तर्भाव <unistd.h>
#endif

#सचे defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#अञ्ञथा
#define USE_OSX_GETHOSTUUID 0
#endif

#सचे USE_OSX_GETHOSTUUID
#अन्तर्भाव <uuid/uuid.h>
#endif

using नामाकाश llvm;

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
  सचे (!MBOrErr) {
    sys::fs::remove(LockFileName);
    पटिनिवत्त std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  पूर्णाङ्क PID;
  सचे (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    सचे (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      पटिनिवत्त Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  पटिनिवत्त std::nullopt;
}

स्थिर std::error_code getHostID(SmallVectorImpl<वर्ण> &HostID) {
  HostID.clear();

#सचे USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  संरचना timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  सचे (gethostuuid(uuid, &wait) != 0)
    पटिनिवत्त errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#नोचेद्यदि LLVM_ON_UNIX
  वर्ण HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#अञ्ञथा
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  पटिनिवत्त std::error_code();
}

तार्किक LockFileManager::processStillExecuting(StringRef HostID, पूर्णाङ्क PID) {
#सचे LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  सचे (getHostID(StoredHostID))
    पटिनिवत्त सच्च; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  सचे (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    पटिनिवत्त मिच्छा;
#endif

  पटिनिवत्त सच्च;
}

नामाकाश {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
वग्ग RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  तार्किक RemoveImmediately;
सार्वजनिक:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(सच्च) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    सचे (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      पटिनिवत्त;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  शून्य lockAcquired() { RemoveImmediately = मिच्छा; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<तार्किक> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  सचे (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    पटिनिवत्त createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  सचे (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    पटिनिवत्त मिच्छा;
  }

  // Create a lock file that is unique to this instance.
  पूर्णाङ्क UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    सचे (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      सचे (!Dir.empty()) {
        सचे (std::error_code DirEC = sys::fs::create_directories(Dir))
          पटिनिवत्त createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    सचे (EC)
      पटिनिवत्त createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    सचे (auto EC = getHostID(HostID))
      पटिनिवत्त createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/सच्च);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    सचे (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      पटिनिवत्त std::move(Err);
    }
  }

  याव (सच्च) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    सचे (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      पटिनिवत्त सच्च;
    }

    सचे (EC != errc::file_exists)
      पटिनिवत्त createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    सचे (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      पटिनिवत्त मिच्छा;
    }

    सचे (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      अनुवर्त;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    सचे ((EC = sys::fs::remove(LockFileName)))
      पटिनिवत्त createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  सचे (!std::holds_alternative<OwnedByUs>(Owner))
    पटिनिवत्त;

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
  प्रतिज्ञा(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using नामाकाश std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  याव (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    सचे (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      पटिनिवत्त WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    सचे (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      पटिनिवत्त WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  पटिनिवत्त WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  पटिनिवत्त sys::fs::remove(LockFileName);
}
