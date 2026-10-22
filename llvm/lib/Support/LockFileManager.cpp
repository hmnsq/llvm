//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/LockFileManager.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ErrorOr.h"
#شامل "llvm/Support/ExponentialBackoff.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cerrno>
#شامل <chrono>
#شامل <ctime>
#شامل <memory>
#شامل <system_error>
#شامل <tuple>

#ifdef _WIN32
#شامل <windows.h>
#endif
#اگر LLVM_ON_UNIX
#شامل <unistd.h>
#endif

#اگر defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#وگرنه
#define USE_OSX_GETHOSTUUID 0
#endif

#اگر USE_OSX_GETHOSTUUID
#شامل <uuid/uuid.h>
#endif

using فضاینام llvm;

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
  اگر (!MBOrErr) {
    sys::fs::remove(LockFileName);
    بازگشت std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  صحیح PID;
  اگر (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    اگر (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      بازگشت Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  بازگشت std::nullopt;
}

ایستا std::error_code getHostID(SmallVectorImpl<نویسه> &HostID) {
  HostID.clear();

#اگر USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  ساختار timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  اگر (gethostuuid(uuid, &wait) != 0)
    بازگشت errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#وگرنهاگر LLVM_ON_UNIX
  نویسه HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#وگرنه
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  بازگشت std::error_code();
}

منطقی LockFileManager::processStillExecuting(StringRef HostID, صحیح PID) {
#اگر LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  اگر (getHostID(StoredHostID))
    بازگشت درست; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  اگر (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    بازگشت نادرست;
#endif

  بازگشت درست;
}

فضاینام {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
رده RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  منطقی RemoveImmediately;
عمومی:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(درست) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    اگر (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      بازگشت;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  تهی lockAcquired() { RemoveImmediately = نادرست; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<منطقی> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  اگر (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    بازگشت createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  اگر (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    بازگشت نادرست;
  }

  // Create a lock file that is unique to this instance.
  صحیح UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    اگر (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      اگر (!Dir.empty()) {
        اگر (std::error_code DirEC = sys::fs::create_directories(Dir))
          بازگشت createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    اگر (EC)
      بازگشت createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    اگر (auto EC = getHostID(HostID))
      بازگشت createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/درست);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    اگر (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      بازگشت std::move(Err);
    }
  }

  وقتی (درست) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    اگر (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      بازگشت درست;
    }

    اگر (EC != errc::file_exists)
      بازگشت createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    اگر (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      بازگشت نادرست;
    }

    اگر (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      ادامه;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    اگر ((EC = sys::fs::remove(LockFileName)))
      بازگشت createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  اگر (!std::holds_alternative<OwnedByUs>(Owner))
    بازگشت;

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
  using فضاینام std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  وقتی (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    اگر (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      بازگشت WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    اگر (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      بازگشت WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  بازگشت WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  بازگشت sys::fs::remove(LockFileName);
}
