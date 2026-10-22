//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/LockFileManager.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/ErrorOr.h"
#அந்தர்பாவ "llvm/Support/ExponentialBackoff.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/IOSandbox.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cerrno>
#அந்தர்பாவ <chrono>
#அந்தர்பாவ <ctime>
#அந்தர்பாவ <memory>
#அந்தர்பாவ <system_error>
#அந்தர்பாவ <tuple>

#ifdef _WIN32
#அந்தர்பாவ <windows.h>
#endif
#எனில் LLVM_ON_UNIX
#அந்தர்பாவ <unistd.h>
#endif

#எனில் defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#இல்லையெனில்
#define USE_OSX_GETHOSTUUID 0
#endif

#எனில் USE_OSX_GETHOSTUUID
#அந்தர்பாவ <uuid/uuid.h>
#endif

using நாமாகாஶ llvm;

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
  எனில் (!MBOrErr) {
    sys::fs::remove(LockFileName);
    திரும்பு std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  முழு PID;
  எனில் (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    எனில் (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      திரும்பு Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  திரும்பு std::nullopt;
}

நிலையான std::error_code getHostID(SmallVectorImpl<எழுத்து> &HostID) {
  HostID.clear();

#எனில் USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  கட்டமைப்பு timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  எனில் (gethostuuid(uuid, &wait) != 0)
    திரும்பு errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#அல்லதெனில் LLVM_ON_UNIX
  எழுத்து HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#இல்லையெனில்
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  திரும்பு std::error_code();
}

தருக்க LockFileManager::processStillExecuting(StringRef HostID, முழு PID) {
#எனில் LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  எனில் (getHostID(StoredHostID))
    திரும்பு உண்மை; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  எனில் (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    திரும்பு பொய்;
#endif

  திரும்பு உண்மை;
}

நாமாகாஶ {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
வகுப்பு RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  தருக்க RemoveImmediately;
பொது:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(உண்மை) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    எனில் (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      திரும்பு;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  வெறுமை lockAcquired() { RemoveImmediately = பொய்; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<தருக்க> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ப்ரதிஜ்ஞா(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  எனில் (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    திரும்பு createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  எனில் (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    திரும்பு பொய்;
  }

  // Create a lock file that is unique to this instance.
  முழு UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    எனில் (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      எனில் (!Dir.empty()) {
        எனில் (std::error_code DirEC = sys::fs::create_directories(Dir))
          திரும்பு createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    எனில் (EC)
      திரும்பு createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    எனில் (auto EC = getHostID(HostID))
      திரும்பு createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/உண்மை);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    எனில் (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      திரும்பு std::move(Err);
    }
  }

  போது (உண்மை) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    எனில் (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      திரும்பு உண்மை;
    }

    எனில் (EC != errc::file_exists)
      திரும்பு createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    எனில் (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      திரும்பு பொய்;
    }

    எனில் (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      தொடர்;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    எனில் ((EC = sys::fs::remove(LockFileName)))
      திரும்பு createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  எனில் (!std::holds_alternative<OwnedByUs>(Owner))
    திரும்பு;

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
  ப்ரதிஜ்ஞா(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using நாமாகாஶ std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  போது (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    எனில் (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      திரும்பு WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    எனில் (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      திரும்பு WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  திரும்பு WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  திரும்பு sys::fs::remove(LockFileName);
}
