//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/LockFileManager.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorOr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ExponentialBackoff.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cerrno>
#ಅನ್ತರ್ಭಾವ <chrono>
#ಅನ್ತರ್ಭಾವ <ctime>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <system_error>
#ಅನ್ತರ್ಭಾವ <tuple>

#ifdef _WIN32
#ಅನ್ತರ್ಭಾವ <windows.h>
#endif
#ಯದಿ LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ <unistd.h>
#endif

#ಯದಿ defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#ಅನ್ಯಥಾ
#define USE_OSX_GETHOSTUUID 0
#endif

#ಯದಿ USE_OSX_GETHOSTUUID
#ಅನ್ತರ್ಭಾವ <uuid/uuid.h>
#endif

using ನಾಮಾಕಾಶ llvm;

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
  ಯದಿ (!MBOrErr) {
    sys::fs::remove(LockFileName);
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  ಪೂರ್ಣಾಙ್ಕ PID;
  ಯದಿ (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    ಯದಿ (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      ಪ್ರತ್ಯಾಗಮ Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

ಸ್ಥಿರ std::error_code getHostID(SmallVectorImpl<ವರ್ಣ> &HostID) {
  HostID.clear();

#ಯದಿ USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  ಸಂರಚನಾ timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  ಯದಿ (gethostuuid(uuid, &wait) != 0)
    ಪ್ರತ್ಯಾಗಮ errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#ನೋಚೇದ್ಯದಿ LLVM_ON_UNIX
  ವರ್ಣ HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#ಅನ್ಯಥಾ
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ತಾರ್ಕಿಕ LockFileManager::processStillExecuting(StringRef HostID, ಪೂರ್ಣಾಙ್ಕ PID) {
#ಯದಿ LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  ಯದಿ (getHostID(StoredHostID))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  ಯದಿ (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
#endif

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ನಾಮಾಕಾಶ {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
ವರ್ಗ RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  ತಾರ್ಕಿಕ RemoveImmediately;
ಸಾರ್ವಜನಿಕ:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(ಸತ್ಯ) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    ಯದಿ (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      ಪ್ರತ್ಯಾಗಮ;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  ಶೂನ್ಯ lockAcquired() { RemoveImmediately = ಅಸತ್ಯ; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<ತಾರ್ಕಿಕ> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  ಯದಿ (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    ಪ್ರತ್ಯಾಗಮ createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  ಯದಿ (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // Create a lock file that is unique to this instance.
  ಪೂರ್ಣಾಙ್ಕ UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    ಯದಿ (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      ಯದಿ (!Dir.empty()) {
        ಯದಿ (std::error_code DirEC = sys::fs::create_directories(Dir))
          ಪ್ರತ್ಯಾಗಮ createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    ಯದಿ (EC)
      ಪ್ರತ್ಯಾಗಮ createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    ಯದಿ (auto EC = getHostID(HostID))
      ಪ್ರತ್ಯಾಗಮ createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/ಸತ್ಯ);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    ಯದಿ (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      ಪ್ರತ್ಯಾಗಮ std::move(Err);
    }
  }

  ಯಾವತ್ (ಸತ್ಯ) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    ಯದಿ (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }

    ಯದಿ (EC != errc::file_exists)
      ಪ್ರತ್ಯಾಗಮ createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    ಯದಿ (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }

    ಯದಿ (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      ಅನುವರ್ತ;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    ಯದಿ ((EC = sys::fs::remove(LockFileName)))
      ಪ್ರತ್ಯಾಗಮ createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಯದಿ (!std::holds_alternative<OwnedByUs>(Owner))
    ಪ್ರತ್ಯಾಗಮ;

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
  ಪ್ರತಿಜ್ಞಾ(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using ನಾಮಾಕಾಶ std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  ಯಾವತ್ (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    ಯದಿ (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      ಪ್ರತ್ಯಾಗಮ WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    ಯದಿ (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      ಪ್ರತ್ಯಾಗಮ WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  ಪ್ರತ್ಯಾಗಮ WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತ್ಯಾಗಮ sys::fs::remove(LockFileName);
}
