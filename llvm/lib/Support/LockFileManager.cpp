//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/LockFileManager.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/ErrorOr.h"
#समावेश "llvm/Support/ExponentialBackoff.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/IOSandbox.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/Process.h"
#समावेश "llvm/Support/Signals.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cerrno>
#समावेश <chrono>
#समावेश <ctime>
#समावेश <memory>
#समावेश <system_error>
#समावेश <tuple>

#अगर_घोषित _WIN32
#समावेश <windows.h>
#पूर्ण_अगर
#अगर LLVM_ON_UNIX
#समावेश <unistd.h>
#पूर्ण_अगर

#अगर defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#घोषणा USE_OSX_GETHOSTUUID 1
#अन्यथा
#घोषणा USE_OSX_GETHOSTUUID 0
#पूर्ण_अगर

#अगर USE_OSX_GETHOSTUUID
#समावेश <uuid/uuid.h>
#पूर्ण_अगर

using namespace llvm;

/// Attempt to read the lock file with the given name, if it exists.
///
/// \param LockFileName The name of the lock file to read.
///
/// \returns The process ID of the process that owns this lock file
std::optional<LockFileManager::OwnedByAnother>
LockFileManager::readLockFile(StringRef LockFileName) {
  // Read the owning host and PID out of the lock file. If it appears that the
  // owning process is dead, the lock file is invalid.
  ErrorOr<std::unique_ptr<Memory_बफर>> MBOrErr =
      Memory_बफर::getFile(LockFileName);
  अगर (!MBOrErr) {
    sys::fs::हटाओ(LockFileName);
    वापस std::nullopt;
  }
  Memory_बफर &MB = *MBOrErr.लो();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.get_बफर(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  पूर्णांक PID;
  अगर (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    अगर (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      वापस Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::हटाओ(LockFileName);
  वापस std::nullopt;
}

अटल std::error_code getHostID(SmallVectorImpl<अक्षर> &HostID) {
  HostID.सफा();

#अगर USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  काष्ठा timespec रुको = {1, 0}; // 1 second.
  uuid_t uuid;
  अगर (gethostuuid(uuid, &रुको) != 0)
    वापस errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#या_अगर LLVM_ON_UNIX
  अक्षर HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#अन्यथा
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#पूर्ण_अगर

  वापस std::error_code();
}

bool LockFileManager::processStillExecuting(StringRef HostID, पूर्णांक PID) {
#अगर LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  अगर (getHostID(StoredHostID))
    वापस true; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  अगर (StoredHostID == HostID && getsid(PID) == -1 && त्रुटि_सं == ESRCH)
    वापस false;
#पूर्ण_अगर

  वापस true;
}

namespace {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
श्रेणी RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  bool RemoveImmediately;
खुला:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(true) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    अगर (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      वापस;
    }
    sys::fs::हटाओ(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  व्योम lockAcquired() { RemoveImmediately = false; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<bool> LockFileManager::tryLock() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  निश्चित(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  अगर (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    वापस createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  अगर (स्वतः LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    वापस false;
  }

  // Create a lock file that is unique to this instance.
  पूर्णांक UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    अगर (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      अगर (!Dir.empty()) {
        अगर (std::error_code DirEC = sys::fs::create_directories(Dir))
          वापस createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    अगर (EC)
      वापस createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    अगर (स्वतः EC = getHostID(HostID))
      वापस createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/true);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.बंद();

    अगर (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      वापस std::move(Err);
    }
  }

  जबतक (true) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    अगर (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      वापस true;
    }

    अगर (EC != errc::file_exists)
      वापस createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    अगर (स्वतः LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      वापस false;
    }

    अगर (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      जारी;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    अगर ((EC = sys::fs::हटाओ(LockFileName)))
      वापस createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  अगर (!std::holds_alternative<OwnedByUs>(Owner))
    वापस;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::हटाओ(LockFileName);
  sys::fs::हटाओ(UniqueLockFileName);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(UniqueLockFileName);
}

Wait_For_Unlock_परिणाम
LockFileManager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  स्वतः *LockFileOwner = std::get_if<OwnedByAnother>(&Owner);
  निश्चित(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using namespace std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  जबतक (Backoff.wait_For_अग्र_Attempt()) {
    // FIXME: implement event-based waiting
    अगर (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      वापस Wait_For_Unlock_परिणाम::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    अगर (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      वापस Wait_For_Unlock_परिणाम::OwnerDied;
  }

  // Give up.
  वापस Wait_For_Unlock_परिणाम::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  वापस sys::fs::हटाओ(LockFileName);
}
