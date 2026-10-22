//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LockFileManager.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ErrorOr.h"
#ضمن "llvm/Support/ExponentialBackoff.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cerrno>
#ضمن <chrono>
#ضمن <ctime>
#ضمن <memory>
#ضمن <system_error>
#ضمن <tuple>

#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن <windows.h>
#جيڪڏهن_پورو
#إذا LLVM_ON_UNIX
#ضمن <unistd.h>
#جيڪڏهن_پورو

#إذا defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#وصف_ڪريو USE_OSX_GETHOSTUUID 1
#وإلا
#وصف_ڪريو USE_OSX_GETHOSTUUID 0
#جيڪڏهن_پورو

#إذا USE_OSX_GETHOSTUUID
#ضمن <uuid/uuid.h>
#جيڪڏهن_پورو

استعمال نطاق llvm;

/// Attempt to read the lock file with the given name, if it exists.
///
/// \param LockFileName The name of the lock file to read.
///
/// \returns The process ID of the process that owns this lock file
std::optional<تالو_File_Manager::OwnedByAnother>
تالو_File_Manager::read_تالو_File(StringRef تالو_File_Name) {
  // Read the owning host and PID out of the lock file. If it appears that the
  // owning process is dead, the lock file is invalid.
  ErrorOr<std::unique_ptr<يادگيري_بفر>> MBOrErr =
      يادگيري_بفر::getFile(تالو_File_Name);
  إذا (!MBOrErr) {
    sys::fs::remove(تالو_File_Name);
    أرجع std::nullopt;
  }
  يادگيري_بفر &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.get_بفر(), " ");
  PIDStr = PIDStr.substr(PIDStr.ڳوليو_پهريون_not_جو(' '));
  صحيح PID;
  إذا (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    إذا (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      أرجع Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(تالو_File_Name);
  أرجع std::nullopt;
}

ساكن std::error_ڪوڊ getHostID(Small_ويڪٽر_Impl<حرف> &HostID) {
  HostID.clear();

#إذا USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  بنية timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  إذا (gethostuuid(uuid, &wait) != 0)
    أرجع errno_As_Error_ڪوڊ();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.پڇاڙي());

#وإلاإذا LLVM_ON_UNIX
  حرف HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.پڇاڙي());

#وإلا
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.پڇاڙي());
#جيڪڏهن_پورو

  أرجع std::error_ڪوڊ();
}

منطقي تالو_File_Manager::processStillExecuting(StringRef HostID, صحيح PID) {
#إذا LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  إذا (getHostID(StoredHostID))
    أرجع سچ; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  إذا (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    أرجع خطأ;
#جيڪڏهن_پورو

  أرجع سچ;
}

نطاق {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
صنف Remove_Unique_تالو_File_On_Signal {
  StringRef Filename;
  منطقي RemoveImmediately;
عام:
  Remove_Unique_تالو_File_On_Signal(StringRef Name)
  : Filename(Name), RemoveImmediately(سچ) {
    sys::RemoveFileOnSignal(Filename, خالي_اشارو);
  }

  ~Remove_Unique_تالو_File_On_Signal() {
    إذا (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      أرجع;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  فراغ تالو_Acquired() { RemoveImmediately = خطأ; }
};

} // end anonymous namespace

تالو_File_Manager::تالو_File_Manager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<منطقي> تالو_File_Manager::try_تالو() {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  إذا (std::error_ڪوڊ EC = sys::fs::make_absolute(AbsoluteFileName))
    أرجع createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  تالو_File_Name = AbsoluteFileName;
  تالو_File_Name += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  إذا (خودڪار تالو_File_Owner = read_تالو_File(تالو_File_Name)) {
    Owner = std::move(*تالو_File_Owner);
    أرجع خطأ;
  }

  // Create a lock file that is unique to this instance.
  صحيح Unique_تالو_File_ID;
  {
    SmallString<128> Unique_تالو_File_نمونو = تالو_File_Name;
    Unique_تالو_File_نمونو += "-%%%%%%%%";
    SmallString<128> Unique_رستو;
    std::error_ڪوڊ EC = sys::fs::createUniqueFile(
        Unique_تالو_File_نمونو, Unique_تالو_File_ID, Unique_رستو);
    إذا (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::رستو_2::والد_رستو(Unique_تالو_File_نمونو);
      إذا (!Dir.خالي_جايون()) {
        إذا (std::error_ڪوڊ DirEC = sys::fs::create_directories(Dir))
          أرجع createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(Unique_تالو_File_نمونو, Unique_تالو_File_ID,
                                     Unique_رستو);
    }

    إذا (EC)
      أرجع createStringError(EC,
                               "failed to create unique file " + Unique_رستو);

    Unique_تالو_File_Name = Unique_رستو;
  }

  // Clean up the unique file on signal or scope exit.
  Remove_Unique_تالو_File_On_Signal RemoveUniqueFile(Unique_تالو_File_Name);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    إذا (خودڪار EC = getHostID(HostID))
      أرجع createStringError(EC, "failed to get host id");

    raw_fd_ostream ٻاهريون(Unique_تالو_File_ID, /*shouldClose=*/سچ);
    ٻاهريون << HostID << ' ' << sys::Process::getProcessId();
    ٻاهريون.close();

    إذا (ٻاهريون.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(ٻاهريون.error(),
                                    "failed to write to " + Unique_تالو_File_Name);
      // Don't call report_fatal_error.
      ٻاهريون.clear_error();
      أرجع std::move(Err);
    }
  }

  طالما (سچ) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_ڪوڊ EC =
        sys::fs::create_link(Unique_تالو_File_Name, تالو_File_Name);
    إذا (!EC) {
      RemoveUniqueFile.تالو_Acquired();
      Owner = OwnedByUs{};
      أرجع سچ;
    }

    إذا (EC != errc::file_exists)
      أرجع createStringError(EC, "failed to create link " + تالو_File_Name +
                                       " to " + Unique_تالو_File_Name);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    إذا (خودڪار تالو_File_Owner = read_تالو_File(تالو_File_Name)) {
      Owner = std::move(*تالو_File_Owner);
      أرجع خطأ;
    }

    إذا (!sys::fs::exists(تالو_File_Name)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      استمر;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    إذا ((EC = sys::fs::remove(تالو_File_Name)))
      أرجع createStringError(EC, "failed to remove lockfile " +
                                       Unique_تالو_File_Name);
  }
}

تالو_File_Manager::~تالو_File_Manager() {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (!std::holds_alternative<OwnedByUs>(Owner))
    أرجع;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::remove(تالو_File_Name);
  sys::fs::remove(Unique_تالو_File_Name);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(Unique_تالو_File_Name);
}

Wait_For_Unlock_نتيجو
تالو_File_Manager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  خودڪار *تالو_File_Owner = std::get_if<OwnedByAnother>(&Owner);
  أكد(تالو_File_Owner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  استعمال نطاق std::chrono_literals;
  قوت_نمائي_Backoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  طالما (Backoff.wait_For_ايندڙ_Attempt()) {
    // FIXME: implement event-based waiting
    إذا (sys::fs::access(تالو_File_Name.c_str(), sys::fs::Access_گهڻائي::Exist) ==
        errc::no_such_file_or_directory)
      أرجع Wait_For_Unlock_نتيجو::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    إذا (!processStillExecuting(تالو_File_Owner->OwnerHostName,
                               تالو_File_Owner->OwnerPID))
      أرجع Wait_For_Unlock_نتيجو::OwnerDied;
  }

  // Give up.
  أرجع Wait_For_Unlock_نتيجو::Timeout;
}

std::error_ڪوڊ تالو_File_Manager::unsafeUnlock() {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  أرجع sys::fs::remove(تالو_File_Name);
}
