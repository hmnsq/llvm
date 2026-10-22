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
#जर LLVM_ON_UNIX
#अन्तर्भाव <unistd.h>
#endif

#जर defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#अन्यथा
#define USE_OSX_GETHOSTUUID 0
#endif

#जर USE_OSX_GETHOSTUUID
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
  जर (!MBOrErr) {
    sys::fs::हटाओ(LockFileName);
    परत std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  पूर्णांक PID;
  जर (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    जर (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      परत Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::हटाओ(LockFileName);
  परत std::nullopt;
}

स्थिर std::error_code getHostID(SmallVectorImpl<अक्षर> &HostID) {
  HostID.clear();

#जर USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  संरचना timespec रुको = {1, 0}; // 1 second.
  uuid_t uuid;
  जर (gethostuuid(uuid, &रुको) != 0)
    परत errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#किंवाजर LLVM_ON_UNIX
  अक्षर HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#अन्यथा
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  परत std::error_code();
}

तार्किक LockFileManager::processStillExecuting(StringRef HostID, पूर्णांक PID) {
#जर LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  जर (getHostID(StoredHostID))
    परत सत्य; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  जर (StoredHostID == HostID && getsid(PID) == -1 && त्रुटि_सं == ESRCH)
    परत असत्य;
#endif

  परत सत्य;
}

नामाकाश {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
वर्ग RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  तार्किक RemoveImmediately;
सार्वजनिक:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(सत्य) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    जर (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      परत;
    }
    sys::fs::हटाओ(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  शून्य lockAcquired() { RemoveImmediately = असत्य; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<तार्किक> LockFileManager::tryLock() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  जर (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    परत createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  जर (स्वतः LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    परत असत्य;
  }

  // Create a lock file that is unique to this instance.
  पूर्णांक UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    जर (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      जर (!Dir.empty()) {
        जर (std::error_code DirEC = sys::fs::create_directories(Dir))
          परत createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    जर (EC)
      परत createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    जर (स्वतः EC = getHostID(HostID))
      परत createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/सत्य);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.बंद();

    जर (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      परत std::move(Err);
    }
  }

  पर्यंत (सत्य) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    जर (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      परत सत्य;
    }

    जर (EC != errc::file_exists)
      परत createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    जर (स्वतः LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      परत असत्य;
    }

    जर (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      सुरूठेवा;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    जर ((EC = sys::fs::हटाओ(LockFileName)))
      परत createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  जर (!std::holds_alternative<OwnedByUs>(Owner))
    परत;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::हटाओ(LockFileName);
  sys::fs::हटाओ(UniqueLockFileName);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(UniqueLockFileName);
}

WaitForUnlockResult
LockFileManager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  स्वतः *LockFileOwner = std::get_if<OwnedByAnother>(&Owner);
  प्रतिज्ञा(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using नामाकाश std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  पर्यंत (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    जर (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      परत WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    जर (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      परत WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  परत WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  परत sys::fs::हटाओ(LockFileName);
}
