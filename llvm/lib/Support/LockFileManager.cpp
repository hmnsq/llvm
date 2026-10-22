//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/LockFileManager.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/ErrorOr.h"
#অন্তর্ভাব "llvm/Support/ExponentialBackoff.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/IOSandbox.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/Path.h"
#অন্তর্ভাব "llvm/Support/Process.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cerrno>
#অন্তর্ভাব <chrono>
#অন্তর্ভাব <ctime>
#অন্তর্ভাব <memory>
#অন্তর্ভাব <system_error>
#অন্তর্ভাব <tuple>

#ifdef _WIN32
#অন্তর্ভাব <windows.h>
#endif
#যদি LLVM_ON_UNIX
#অন্তর্ভাব <unistd.h>
#endif

#যদি defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#অন্যথা
#define USE_OSX_GETHOSTUUID 0
#endif

#যদি USE_OSX_GETHOSTUUID
#অন্তর্ভাব <uuid/uuid.h>
#endif

using নামাকাশ llvm;

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
  যদি (!MBOrErr) {
    sys::fs::remove(LockFileName);
    প্রত্যাগম std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  পূর্ণাঙ্ক PID;
  যদি (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    যদি (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      প্রত্যাগম Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  প্রত্যাগম std::nullopt;
}

স্থির std::error_code getHostID(SmallVectorImpl<বর্ণ> &HostID) {
  HostID.clear();

#যদি USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  সংরচনা timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  যদি (gethostuuid(uuid, &wait) != 0)
    প্রত্যাগম errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#নোচেদ্যদি LLVM_ON_UNIX
  বর্ণ HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#অন্যথা
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  প্রত্যাগম std::error_code();
}

তার্কিক LockFileManager::processStillExecuting(StringRef HostID, পূর্ণাঙ্ক PID) {
#যদি LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  যদি (getHostID(StoredHostID))
    প্রত্যাগম সত্য; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  যদি (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    প্রত্যাগম অসত্য;
#endif

  প্রত্যাগম সত্য;
}

নামাকাশ {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
বর্গ RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  তার্কিক RemoveImmediately;
সার্বজনিক:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(সত্য) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    যদি (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      প্রত্যাগম;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  শূন্য lockAcquired() { RemoveImmediately = অসত্য; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<তার্কিক> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  প্রতিজ্ঞা(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  যদি (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    প্রত্যাগম createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  যদি (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    প্রত্যাগম অসত্য;
  }

  // Create a lock file that is unique to this instance.
  পূর্ণাঙ্ক UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    যদি (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      যদি (!Dir.empty()) {
        যদি (std::error_code DirEC = sys::fs::create_directories(Dir))
          প্রত্যাগম createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    যদি (EC)
      প্রত্যাগম createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    যদি (auto EC = getHostID(HostID))
      প্রত্যাগম createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/সত্য);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    যদি (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      প্রত্যাগম std::move(Err);
    }
  }

  যাবত্ (সত্য) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    যদি (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      প্রত্যাগম সত্য;
    }

    যদি (EC != errc::file_exists)
      প্রত্যাগম createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    যদি (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      প্রত্যাগম অসত্য;
    }

    যদি (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      অনুবর্ত;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    যদি ((EC = sys::fs::remove(LockFileName)))
      প্রত্যাগম createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (!std::holds_alternative<OwnedByUs>(Owner))
    প্রত্যাগম;

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
  প্রতিজ্ঞা(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using নামাকাশ std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  যাবত্ (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    যদি (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      প্রত্যাগম WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    যদি (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      প্রত্যাগম WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  প্রত্যাগম WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  প্রত্যাগম sys::fs::remove(LockFileName);
}
