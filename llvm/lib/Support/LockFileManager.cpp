//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LockFileManager.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorOr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ExponentialBackoff.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/IOSandbox.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cerrno>
#ਅਨ੍ਤਰ੍ਭਾਵ <chrono>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctime>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>
#ਅਨ੍ਤਰ੍ਭਾਵ <tuple>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>
#ਜੇ_ਅੰਤ
#ਜੇ LLVM_ON_UNIX
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_OSX_GETHOSTUUID 1
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_OSX_GETHOSTUUID 0
#ਜੇ_ਅੰਤ

#ਜੇ USE_OSX_GETHOSTUUID
#ਅਨ੍ਤਰ੍ਭਾਵ <uuid/uuid.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// Attempt to read the lock file with the given name, if it exists.
///
/// \param LockFileName The name of the lock file to read.
///
/// \returns The process ID of the process that owns this lock file
std::optional<ਤਾਲਾ_File_Manager::OwnedByAnother>
ਤਾਲਾ_File_Manager::read_ਤਾਲਾ_File(StringRef ਤਾਲਾ_File_Name) {
  // Read the owning host and PID out of the lock file. If it appears that the
  // owning process is dead, the lock file is invalid.
  ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> MBOrErr =
      ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(ਤਾਲਾ_File_Name);
  ਜੇ (!MBOrErr) {
    sys::fs::remove(ਤਾਲਾ_File_Name);
    ਵਾਪਸ std::nullopt;
  }
  ਸਮ੍ਰਿਤੀ_ਬਫਰ &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.get_ਬਫਰ(), " ");
  PIDStr = PIDStr.substr(PIDStr.ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ(' '));
  ਪੂਰਨ PID;
  ਜੇ (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    ਜੇ (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      ਵਾਪਸ Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(ਤਾਲਾ_File_Name);
  ਵਾਪਸ std::nullopt;
}

ਸਥਿਰ std::error_ਕੋਡ getHostID(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &HostID) {
  HostID.clear();

#ਜੇ USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  ਬਣਤਰ timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  ਜੇ (gethostuuid(uuid, &wait) != 0)
    ਵਾਪਸ errno_As_Error_ਕੋਡ();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.ਸਿਰਾ());

#ਜਾਂਜੇ LLVM_ON_UNIX
  ਅੱਖਰ HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.ਸਿਰਾ());

#ਨਹੀਂਤਾਂ
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.ਸਿਰਾ());
#ਜੇ_ਅੰਤ

  ਵਾਪਸ std::error_ਕੋਡ();
}

ਤਾਰਕਿਕ ਤਾਲਾ_File_Manager::processStillExecuting(StringRef HostID, ਪੂਰਨ PID) {
#ਜੇ LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  ਜੇ (getHostID(StoredHostID))
    ਵਾਪਸ ਸੱਚ; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  ਜੇ (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    ਵਾਪਸ ਝੂਠ;
#ਜੇ_ਅੰਤ

  ਵਾਪਸ ਸੱਚ;
}

ਨਾਮਾਕਾਸ਼ {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
ਵਰਗ Remove_Unique_ਤਾਲਾ_File_On_Signal {
  StringRef Filename;
  ਤਾਰਕਿਕ RemoveImmediately;
ਜਨਤਕ:
  Remove_Unique_ਤਾਲਾ_File_On_Signal(StringRef Name)
  : Filename(Name), RemoveImmediately(ਸੱਚ) {
    sys::RemoveFileOnSignal(Filename, ਖਾਲੀ_ਸੂਚਕ);
  }

  ~Remove_Unique_ਤਾਲਾ_File_On_Signal() {
    ਜੇ (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      ਵਾਪਸ;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  ਖਾਲੀ ਤਾਲਾ_Acquired() { RemoveImmediately = ਝੂਠ; }
};

} // end anonymous namespace

ਤਾਲਾ_File_Manager::ਤਾਲਾ_File_Manager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<ਤਾਰਕਿਕ> ਤਾਲਾ_File_Manager::try_ਤਾਲਾ() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਪ੍ਰਤਿਜ੍ਞਾ(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  ਜੇ (std::error_ਕੋਡ EC = sys::fs::make_absolute(AbsoluteFileName))
    ਵਾਪਸ createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  ਤਾਲਾ_File_Name = AbsoluteFileName;
  ਤਾਲਾ_File_Name += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  ਜੇ (ਸਵੈਚਾਲਿਤ ਤਾਲਾ_File_Owner = read_ਤਾਲਾ_File(ਤਾਲਾ_File_Name)) {
    Owner = std::move(*ਤਾਲਾ_File_Owner);
    ਵਾਪਸ ਝੂਠ;
  }

  // Create a lock file that is unique to this instance.
  ਪੂਰਨ Unique_ਤਾਲਾ_File_ID;
  {
    SmallString<128> Unique_ਤਾਲਾ_File_ਨਮੂਨਾ = ਤਾਲਾ_File_Name;
    Unique_ਤਾਲਾ_File_ਨਮੂਨਾ += "-%%%%%%%%";
    SmallString<128> Unique_ਰਸਤਾ;
    std::error_ਕੋਡ EC = sys::fs::createUniqueFile(
        Unique_ਤਾਲਾ_File_ਨਮੂਨਾ, Unique_ਤਾਲਾ_File_ID, Unique_ਰਸਤਾ);
    ਜੇ (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(Unique_ਤਾਲਾ_File_ਨਮੂਨਾ);
      ਜੇ (!Dir.ਖਾਲੀ_ਥਾਵਾਂ()) {
        ਜੇ (std::error_ਕੋਡ DirEC = sys::fs::create_directories(Dir))
          ਵਾਪਸ createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(Unique_ਤਾਲਾ_File_ਨਮੂਨਾ, Unique_ਤਾਲਾ_File_ID,
                                     Unique_ਰਸਤਾ);
    }

    ਜੇ (EC)
      ਵਾਪਸ createStringError(EC,
                               "failed to create unique file " + Unique_ਰਸਤਾ);

    Unique_ਤਾਲਾ_File_Name = Unique_ਰਸਤਾ;
  }

  // Clean up the unique file on signal or scope exit.
  Remove_Unique_ਤਾਲਾ_File_On_Signal RemoveUniqueFile(Unique_ਤਾਲਾ_File_Name);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    ਜੇ (ਸਵੈਚਾਲਿਤ EC = getHostID(HostID))
      ਵਾਪਸ createStringError(EC, "failed to get host id");

    raw_fd_ostream ਬਾਹਰਲਾ(Unique_ਤਾਲਾ_File_ID, /*shouldClose=*/ਸੱਚ);
    ਬਾਹਰਲਾ << HostID << ' ' << sys::Process::getProcessId();
    ਬਾਹਰਲਾ.close();

    ਜੇ (ਬਾਹਰਲਾ.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(ਬਾਹਰਲਾ.error(),
                                    "failed to write to " + Unique_ਤਾਲਾ_File_Name);
      // Don't call report_fatal_error.
      ਬਾਹਰਲਾ.clear_error();
      ਵਾਪਸ std::move(Err);
    }
  }

  ਜਦੋਂ (ਸੱਚ) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_ਕੋਡ EC =
        sys::fs::create_link(Unique_ਤਾਲਾ_File_Name, ਤਾਲਾ_File_Name);
    ਜੇ (!EC) {
      RemoveUniqueFile.ਤਾਲਾ_Acquired();
      Owner = OwnedByUs{};
      ਵਾਪਸ ਸੱਚ;
    }

    ਜੇ (EC != errc::file_exists)
      ਵਾਪਸ createStringError(EC, "failed to create link " + ਤਾਲਾ_File_Name +
                                       " to " + Unique_ਤਾਲਾ_File_Name);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    ਜੇ (ਸਵੈਚਾਲਿਤ ਤਾਲਾ_File_Owner = read_ਤਾਲਾ_File(ਤਾਲਾ_File_Name)) {
      Owner = std::move(*ਤਾਲਾ_File_Owner);
      ਵਾਪਸ ਝੂਠ;
    }

    ਜੇ (!sys::fs::exists(ਤਾਲਾ_File_Name)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      ਜਾਰੀ;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    ਜੇ ((EC = sys::fs::remove(ਤਾਲਾ_File_Name)))
      ਵਾਪਸ createStringError(EC, "failed to remove lockfile " +
                                       Unique_ਤਾਲਾ_File_Name);
  }
}

ਤਾਲਾ_File_Manager::~ਤਾਲਾ_File_Manager() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਜੇ (!std::holds_alternative<OwnedByUs>(Owner))
    ਵਾਪਸ;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::remove(ਤਾਲਾ_File_Name);
  sys::fs::remove(Unique_ਤਾਲਾ_File_Name);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(Unique_ਤਾਲਾ_File_Name);
}

Wait_For_Unlock_ਨਤੀਜਾ
ਤਾਲਾ_File_Manager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਸਵੈਚਾਲਿਤ *ਤਾਲਾ_File_Owner = std::get_if<OwnedByAnother>(&Owner);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਤਾਲਾ_File_Owner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ std::chrono_literals;
  ਘਾਤਾਂਕੀ_Backoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  ਜਦੋਂ (Backoff.wait_For_ਅਗਲਾ_Attempt()) {
    // FIXME: implement event-based waiting
    ਜੇ (sys::fs::access(ਤਾਲਾ_File_Name.c_str(), sys::fs::Access_ਬਹੁਲਕ::Exist) ==
        errc::no_such_file_or_directory)
      ਵਾਪਸ Wait_For_Unlock_ਨਤੀਜਾ::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    ਜੇ (!processStillExecuting(ਤਾਲਾ_File_Owner->OwnerHostName,
                               ਤਾਲਾ_File_Owner->OwnerPID))
      ਵਾਪਸ Wait_For_Unlock_ਨਤੀਜਾ::OwnerDied;
  }

  // Give up.
  ਵਾਪਸ Wait_For_Unlock_ਨਤੀਜਾ::Timeout;
}

std::error_ਕੋਡ ਤਾਲਾ_File_Manager::unsafeUnlock() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਵਾਪਸ sys::fs::remove(ਤਾਲਾ_File_Name);
}
