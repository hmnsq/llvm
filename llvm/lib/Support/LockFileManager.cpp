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

#که_تعریف_شوی _WIN32
#ضمن <windows.h>
#د_که_پای
#إذا LLVM_ON_UNIX
#ضمن <unistd.h>
#د_که_پای

#إذا defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#تعریف_کړه USE_OSX_GETHOSTUUID 1
#وإلا
#تعریف_کړه USE_OSX_GETHOSTUUID 0
#د_که_پای

#إذا USE_OSX_GETHOSTUUID
#ضمن <uuid/uuid.h>
#د_که_پای

کارول نطاق llvm;

/// Attempt to read the lock file with the given name, if it exists.
///
/// \param LockFileName The name of the lock file to read.
///
/// \returns The process ID of the process that owns this lock file
std::optional<قلف_File_Manager::OwnedByAnother>
قلف_File_Manager::read_قلف_File(StringRef قلف_File_Name) {
  // Read the owning host and PID out of the lock file. If it appears that the
  // owning process is dead, the lock file is invalid.
  ErrorOr<std::unique_ptr<حافظه_بفر>> MBOrErr =
      حافظه_بفر::getFile(قلف_File_Name);
  إذا (!MBOrErr) {
    sys::fs::remove(قلف_File_Name);
    أرجع std::nullopt;
  }
  حافظه_بفر &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.get_بفر(), " ");
  PIDStr = PIDStr.substr(PIDStr.پیدا_کړه_لومړی_not_د(' '));
  صحيح PID;
  إذا (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    إذا (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      أرجع Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(قلف_File_Name);
  أرجع std::nullopt;
}

ساكن std::error_کوډ getHostID(Small_ویکټور_Impl<حرف> &HostID) {
  HostID.clear();

#إذا USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  بنية timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  إذا (gethostuuid(uuid, &wait) != 0)
    أرجع errno_As_Error_کوډ();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.پای_ټکی());

#وإلاإذا LLVM_ON_UNIX
  حرف HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.پای_ټکی());

#وإلا
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.پای_ټکی());
#د_که_پای

  أرجع std::error_کوډ();
}

منطقي قلف_File_Manager::processStillExecuting(StringRef HostID, صحيح PID) {
#إذا LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  إذا (getHostID(StoredHostID))
    أرجع ریښتیا; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  إذا (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    أرجع خطأ;
#د_که_پای

  أرجع ریښتیا;
}

نطاق {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
صنف Remove_Unique_قلف_File_On_Signal {
  StringRef Filename;
  منطقي RemoveImmediately;
عام:
  Remove_Unique_قلف_File_On_Signal(StringRef Name)
  : Filename(Name), RemoveImmediately(ریښتیا) {
    sys::RemoveFileOnSignal(Filename, تش_اشاره);
  }

  ~Remove_Unique_قلف_File_On_Signal() {
    إذا (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      أرجع;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  فراغ قلف_Acquired() { RemoveImmediately = خطأ; }
};

} // end anonymous namespace

قلف_File_Manager::قلف_File_Manager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<منطقي> قلف_File_Manager::try_قلف() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  أكد(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  إذا (std::error_کوډ EC = sys::fs::make_absolute(AbsoluteFileName))
    أرجع createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  قلف_File_Name = AbsoluteFileName;
  قلف_File_Name += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  إذا (خپلکاری قلف_File_Owner = read_قلف_File(قلف_File_Name)) {
    Owner = std::move(*قلف_File_Owner);
    أرجع خطأ;
  }

  // Create a lock file that is unique to this instance.
  صحيح Unique_قلف_File_ID;
  {
    SmallString<128> Unique_قلف_File_نمونه = قلف_File_Name;
    Unique_قلف_File_نمونه += "-%%%%%%%%";
    SmallString<128> Unique_لار;
    std::error_کوډ EC = sys::fs::createUniqueFile(
        Unique_قلف_File_نمونه, Unique_قلف_File_ID, Unique_لار);
    إذا (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::لار_2::پلار_لار(Unique_قلف_File_نمونه);
      إذا (!Dir.تشه()) {
        إذا (std::error_کوډ DirEC = sys::fs::create_directories(Dir))
          أرجع createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(Unique_قلف_File_نمونه, Unique_قلف_File_ID,
                                     Unique_لار);
    }

    إذا (EC)
      أرجع createStringError(EC,
                               "failed to create unique file " + Unique_لار);

    Unique_قلف_File_Name = Unique_لار;
  }

  // Clean up the unique file on signal or scope exit.
  Remove_Unique_قلف_File_On_Signal RemoveUniqueFile(Unique_قلف_File_Name);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    إذا (خپلکاری EC = getHostID(HostID))
      أرجع createStringError(EC, "failed to get host id");

    raw_fd_ostream بهر(Unique_قلف_File_ID, /*shouldClose=*/ریښتیا);
    بهر << HostID << ' ' << sys::Process::getProcessId();
    بهر.close();

    إذا (بهر.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(بهر.error(),
                                    "failed to write to " + Unique_قلف_File_Name);
      // Don't call report_fatal_error.
      بهر.clear_error();
      أرجع std::move(Err);
    }
  }

  طالما (ریښتیا) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_کوډ EC =
        sys::fs::create_link(Unique_قلف_File_Name, قلف_File_Name);
    إذا (!EC) {
      RemoveUniqueFile.قلف_Acquired();
      Owner = OwnedByUs{};
      أرجع ریښتیا;
    }

    إذا (EC != errc::file_exists)
      أرجع createStringError(EC, "failed to create link " + قلف_File_Name +
                                       " to " + Unique_قلف_File_Name);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    إذا (خپلکاری قلف_File_Owner = read_قلف_File(قلف_File_Name)) {
      Owner = std::move(*قلف_File_Owner);
      أرجع خطأ;
    }

    إذا (!sys::fs::exists(قلف_File_Name)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      استمر;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    إذا ((EC = sys::fs::remove(قلف_File_Name)))
      أرجع createStringError(EC, "failed to remove lockfile " +
                                       Unique_قلف_File_Name);
  }
}

قلف_File_Manager::~قلف_File_Manager() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  إذا (!std::holds_alternative<OwnedByUs>(Owner))
    أرجع;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::remove(قلف_File_Name);
  sys::fs::remove(Unique_قلف_File_Name);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(Unique_قلف_File_Name);
}

Wait_For_Unlock_پایله
قلف_File_Manager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  خپلکاری *قلف_File_Owner = std::get_if<OwnedByAnother>(&Owner);
  أكد(قلف_File_Owner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  کارول نطاق std::chrono_literals;
  توانیز_Backoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  طالما (Backoff.wait_For_بل_Attempt()) {
    // FIXME: implement event-based waiting
    إذا (sys::fs::access(قلف_File_Name.c_str(), sys::fs::Access_ډېرپېښ::Exist) ==
        errc::no_such_file_or_directory)
      أرجع Wait_For_Unlock_پایله::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    إذا (!processStillExecuting(قلف_File_Owner->OwnerHostName,
                               قلف_File_Owner->OwnerPID))
      أرجع Wait_For_Unlock_پایله::OwnerDied;
  }

  // Give up.
  أرجع Wait_For_Unlock_پایله::Timeout;
}

std::error_کوډ قلف_File_Manager::unsafeUnlock() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  أرجع sys::fs::remove(قلف_File_Name);
}
