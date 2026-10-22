//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/LockFileManager.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/ErrorOr.h"
#അന്തര്ഭാവ "llvm/Support/ExponentialBackoff.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/IOSandbox.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/Support/Signals.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cerrno>
#അന്തര്ഭാവ <chrono>
#അന്തര്ഭാവ <ctime>
#അന്തര്ഭാവ <memory>
#അന്തര്ഭാവ <system_error>
#അന്തര്ഭാവ <tuple>

#ifdef _WIN32
#അന്തര്ഭാവ <windows.h>
#endif
#എങ്കിൽ LLVM_ON_UNIX
#അന്തര്ഭാവ <unistd.h>
#endif

#എങ്കിൽ defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#അല്ലെങ്കിൽ
#define USE_OSX_GETHOSTUUID 0
#endif

#എങ്കിൽ USE_OSX_GETHOSTUUID
#അന്തര്ഭാവ <uuid/uuid.h>
#endif

using നാമാകാശ llvm;

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
  എങ്കിൽ (!MBOrErr) {
    sys::fs::remove(LockFileName);
    തിരികെ std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  പൂർണ്ണസംഖ്യ PID;
  എങ്കിൽ (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    എങ്കിൽ (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      തിരികെ Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  തിരികെ std::nullopt;
}

സ്ഥിതം std::error_code getHostID(SmallVectorImpl<അക്ഷരം> &HostID) {
  HostID.clear();

#എങ്കിൽ USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  ഘടന timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  എങ്കിൽ (gethostuuid(uuid, &wait) != 0)
    തിരികെ errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#അല്ലെങ്കിലെങ്കിൽ LLVM_ON_UNIX
  അക്ഷരം HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#അല്ലെങ്കിൽ
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  തിരികെ std::error_code();
}

തർക്കം LockFileManager::processStillExecuting(StringRef HostID, പൂർണ്ണസംഖ്യ PID) {
#എങ്കിൽ LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  എങ്കിൽ (getHostID(StoredHostID))
    തിരികെ സത്യം; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  എങ്കിൽ (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    തിരികെ അസത്യം;
#endif

  തിരികെ സത്യം;
}

നാമാകാശ {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
വർഗ്ഗം RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  തർക്കം RemoveImmediately;
പൊതു:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(സത്യം) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    എങ്കിൽ (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      തിരികെ;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  ശൂന്യം lockAcquired() { RemoveImmediately = അസത്യം; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<തർക്കം> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  പ്രതിജ്ഞാ(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  എങ്കിൽ (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    തിരികെ createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  എങ്കിൽ (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    തിരികെ അസത്യം;
  }

  // Create a lock file that is unique to this instance.
  പൂർണ്ണസംഖ്യ UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    എങ്കിൽ (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      എങ്കിൽ (!Dir.empty()) {
        എങ്കിൽ (std::error_code DirEC = sys::fs::create_directories(Dir))
          തിരികെ createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    എങ്കിൽ (EC)
      തിരികെ createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    എങ്കിൽ (auto EC = getHostID(HostID))
      തിരികെ createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/സത്യം);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    എങ്കിൽ (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      തിരികെ std::move(Err);
    }
  }

  ആകുമ്പോൾ (സത്യം) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    എങ്കിൽ (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      തിരികെ സത്യം;
    }

    എങ്കിൽ (EC != errc::file_exists)
      തിരികെ createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    എങ്കിൽ (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      തിരികെ അസത്യം;
    }

    എങ്കിൽ (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      തുടരുക;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    എങ്കിൽ ((EC = sys::fs::remove(LockFileName)))
      തിരികെ createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  എങ്കിൽ (!std::holds_alternative<OwnedByUs>(Owner))
    തിരികെ;

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
  പ്രതിജ്ഞാ(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using നാമാകാശ std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  ആകുമ്പോൾ (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    എങ്കിൽ (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      തിരികെ WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    എങ്കിൽ (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      തിരികെ WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  തിരികെ WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  തിരികെ sys::fs::remove(LockFileName);
}
