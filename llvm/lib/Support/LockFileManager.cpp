//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/LockFileManager.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorOr.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ExponentialBackoff.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/IOSandbox.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signals.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cerrno>
#ଅନ୍ତର୍ଭାଵ <chrono>
#ଅନ୍ତର୍ଭାଵ <ctime>
#ଅନ୍ତର୍ଭାଵ <memory>
#ଅନ୍ତର୍ଭାଵ <system_error>
#ଅନ୍ତର୍ଭାଵ <tuple>

#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ <windows.h>
#endif
#ଯଦି LLVM_ON_UNIX
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#endif

#ଯଦି defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#define USE_OSX_GETHOSTUUID 1
#ଅନ୍ୟଥା
#define USE_OSX_GETHOSTUUID 0
#endif

#ଯଦି USE_OSX_GETHOSTUUID
#ଅନ୍ତର୍ଭାଵ <uuid/uuid.h>
#endif

using ନାମାକାଶ llvm;

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
  ଯଦି (!MBOrErr) {
    sys::fs::remove(LockFileName);
    ଫେରନ୍ତୁ std::nullopt;
  }
  MemoryBuffer &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.getBuffer(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_of(' '));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା PID;
  ଯଦି (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    ଯଦି (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      ଫେରନ୍ତୁ Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(LockFileName);
  ଫେରନ୍ତୁ std::nullopt;
}

ସ୍ଥିର std::error_code getHostID(SmallVectorImpl<ଅକ୍ଷର> &HostID) {
  HostID.clear();

#ଯଦି USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  ସଂରଚନା timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  ଯଦି (gethostuuid(uuid, &wait) != 0)
    ଫେରନ୍ତୁ errnoAsErrorCode();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.end());

#ନଚେତଯଦି LLVM_ON_UNIX
  ଅକ୍ଷର HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.end());

#ଅନ୍ୟଥା
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.end());
#endif

  ଫେରନ୍ତୁ std::error_code();
}

ତାର୍କିକ LockFileManager::processStillExecuting(StringRef HostID, ପୂର୍ଣ୍ଣସଂଖ୍ୟା PID) {
#ଯଦି LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  ଯଦି (getHostID(StoredHostID))
    ଫେରନ୍ତୁ ସତ୍ୟ; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  ଯଦି (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
#endif

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ନାମାକାଶ {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
ଶ୍ରେଣୀ RemoveUniqueLockFileOnSignal {
  StringRef Filename;
  ତାର୍କିକ RemoveImmediately;
ସର୍ବସାଧାରଣ:
  RemoveUniqueLockFileOnSignal(StringRef Name)
  : Filename(Name), RemoveImmediately(ସତ୍ୟ) {
    sys::RemoveFileOnSignal(Filename, nullptr);
  }

  ~RemoveUniqueLockFileOnSignal() {
    ଯଦି (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      ଫେରନ୍ତୁ;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  ଶୂନ୍ୟ lockAcquired() { RemoveImmediately = ମିଥ୍ୟା; }
};

} // end anonymous namespace

LockFileManager::LockFileManager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<ତାର୍କିକ> LockFileManager::tryLock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ପ୍ରତିଜ୍ଞା(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  ଯଦି (std::error_code EC = sys::fs::make_absolute(AbsoluteFileName))
    ଫେରନ୍ତୁ createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  LockFileName = AbsoluteFileName;
  LockFileName += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  ଯଦି (auto LockFileOwner = readLockFile(LockFileName)) {
    Owner = std::move(*LockFileOwner);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // Create a lock file that is unique to this instance.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା UniqueLockFileID;
  {
    SmallString<128> UniqueLockFilePattern = LockFileName;
    UniqueLockFilePattern += "-%%%%%%%%";
    SmallString<128> UniquePath;
    std::error_code EC = sys::fs::createUniqueFile(
        UniqueLockFilePattern, UniqueLockFileID, UniquePath);
    ଯଦି (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::path::parent_path(UniqueLockFilePattern);
      ଯଦି (!Dir.empty()) {
        ଯଦି (std::error_code DirEC = sys::fs::create_directories(Dir))
          ଫେରନ୍ତୁ createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(UniqueLockFilePattern, UniqueLockFileID,
                                     UniquePath);
    }

    ଯଦି (EC)
      ଫେରନ୍ତୁ createStringError(EC,
                               "failed to create unique file " + UniquePath);

    UniqueLockFileName = UniquePath;
  }

  // Clean up the unique file on signal or scope exit.
  RemoveUniqueLockFileOnSignal RemoveUniqueFile(UniqueLockFileName);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    ଯଦି (auto EC = getHostID(HostID))
      ଫେରନ୍ତୁ createStringError(EC, "failed to get host id");

    raw_fd_ostream Out(UniqueLockFileID, /*shouldClose=*/ସତ୍ୟ);
    Out << HostID << ' ' << sys::Process::getProcessId();
    Out.close();

    ଯଦି (Out.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(Out.error(),
                                    "failed to write to " + UniqueLockFileName);
      // Don't call report_fatal_error.
      Out.clear_error();
      ଫେରନ୍ତୁ std::move(Err);
    }
  }

  ଯେତେବେଳେ (ସତ୍ୟ) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_code EC =
        sys::fs::create_link(UniqueLockFileName, LockFileName);
    ଯଦି (!EC) {
      RemoveUniqueFile.lockAcquired();
      Owner = OwnedByUs{};
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }

    ଯଦି (EC != errc::file_exists)
      ଫେରନ୍ତୁ createStringError(EC, "failed to create link " + LockFileName +
                                       " to " + UniqueLockFileName);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    ଯଦି (auto LockFileOwner = readLockFile(LockFileName)) {
      Owner = std::move(*LockFileOwner);
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }

    ଯଦି (!sys::fs::exists(LockFileName)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      ଜାରି;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    ଯଦି ((EC = sys::fs::remove(LockFileName)))
      ଫେରନ୍ତୁ createStringError(EC, "failed to remove lockfile " +
                                       UniqueLockFileName);
  }
}

LockFileManager::~LockFileManager() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ଯଦି (!std::holds_alternative<OwnedByUs>(Owner))
    ଫେରନ୍ତୁ;

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
  ପ୍ରତିଜ୍ଞା(LockFileOwner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  using ନାମାକାଶ std::chrono_literals;
  ExponentialBackoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  ଯେତେବେଳେ (Backoff.waitForNextAttempt()) {
    // FIXME: implement event-based waiting
    ଯଦି (sys::fs::access(LockFileName.c_str(), sys::fs::AccessMode::Exist) ==
        errc::no_such_file_or_directory)
      ଫେରନ୍ତୁ WaitForUnlockResult::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    ଯଦି (!processStillExecuting(LockFileOwner->OwnerHostName,
                               LockFileOwner->OwnerPID))
      ଫେରନ୍ତୁ WaitForUnlockResult::OwnerDied;
  }

  // Give up.
  ଫେରନ୍ତୁ WaitForUnlockResult::Timeout;
}

std::error_code LockFileManager::unsafeUnlock() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ଫେରନ୍ତୁ sys::fs::remove(LockFileName);
}
