//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/LockFileManager.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/ErrorOr.h"
#અન્તર્ભાવ "llvm/Support/ExponentialBackoff.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/IOSandbox.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/Process.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cerrno>
#અન્તર્ભાવ <chrono>
#અન્તર્ભાવ <ctime>
#અન્તર્ભાવ <memory>
#અન્તર્ભાવ <system_error>
#અન્તર્ભાવ <tuple>

#ifdef _WIN32
#અન્તર્ભાવ <windows.h>
#endif
#જો LLVM_ON_UNIX
#અન્તર્ભાવ <unistd.h>
#endif

#જો defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#નહીંતર
#define USE_OSX_GETHOSTUUID 0
#endif

#જો USE_OSX_GETHOSTUUID
#અન્તર્ભાવ <uuid/uuid.h>
#endif

using નામાકાશ llvm;

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
  જો (!MBOrErr) {
    sys::fs::remove(LockFileName);
    પાછા std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  પૂર્ણાંક PID;
  જો (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    જો (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      પાછા Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  પાછા std::nullopt;
}

સ્થિર std::error_code getHostID(SmallVectorImpl<અક્ષર> &HostID) {
  HostID.clear();

#જો USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  સંરચના timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  જો (gethostuuid(uuid, &wait) != 0)
    પાછા errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#અથવાજો LLVM_ON_UNIX
  અક્ષર HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#નહીંતર
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  પાછા std::error_code();
}

તાર્કિક LockFileManager::processStillExecuting(StringRef HostID, પૂર્ણાંક PID) {
#જો LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  જો (getHostID(StoredHostID))
    પાછા સાચું; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  જો (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    પાછા ખોટું;
#endif

  પાછા સાચું;
}

નામાકાશ {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
વર્ગ RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  તાર્કિક RemoveImmediately;
જાહેર:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(સાચું) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    જો (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      પાછા;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  શૂન્ય lockAcquired() { RemoveImmediately = ખોટું; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<તાર્કિક> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  પ્રતિજ્ઞા(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  જો (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    પાછા createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  જો (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    પાછા ખોટું;
  }

  // Create a lock file that is unique to this instance.
  પૂર્ણાંક UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    જો (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      જો (!Dir.empty()) {
        જો (std::error_code DirEC = sys::fs::create_directories(Dir))
          પાછા createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    જો (EC)
      પાછા createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    જો (auto EC = getHostID(HostID))
      પાછા createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/સાચું);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    જો (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      પાછા std::move(Err);
    }
  }

  જ્યારે (સાચું) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    જો (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      પાછા સાચું;
    }

    જો (EC != errc::file_exists)
      પાછા createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    જો (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      પાછા ખોટું;
    }

    જો (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      ચાલુ;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    જો ((EC = sys::fs::remove(LockFileName)))
      પાછા createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  જો (!std::holds_alternative<OwnedByUs>(Owner))
    પાછા;

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
  પ્રતિજ્ઞા(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using નામાકાશ std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  જ્યારે (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    જો (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      પાછા WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    જો (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      પાછા WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  પાછા WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  પાછા sys::fs::remove(LockFileName);
}
