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

#اگر_تعریف_شدہ _WIN32
#شامل <windows.h>
#اگر_ختم
#اگر LLVM_ON_UNIX
#شامل <unistd.h>
#اگر_ختم

#اگر defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#تعریف_کرو USE_OSX_GETHOSTUUID 1
#ورنہ
#تعریف_کرو USE_OSX_GETHOSTUUID 0
#اگر_ختم

#اگر USE_OSX_GETHOSTUUID
#شامل <uuid/uuid.h>
#اگر_ختم

استعمال نامخلا llvm;

/// Attempt to read the lock file with the given name, if it exists.
///
/// \param LockFileName The name of the lock file to read.
///
/// \returns The process ID of the process that owns this lock file
std::optional<تالا_File_Manager::OwnedByAnother>
تالا_File_Manager::read_تالا_File(StringRef تالا_File_Name) {
  // Read the owning host and PID out of the lock file. If it appears that the
  // owning process is dead, the lock file is invalid.
  ErrorOr<std::unique_ptr<حافظہ_بفر>> MBOrErr =
      حافظہ_بفر::getFile(تالا_File_Name);
  اگر (!MBOrErr) {
    sys::fs::remove(تالا_File_Name);
    واپس std::nullopt;
  }
  حافظہ_بفر &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.get_بفر(), " ");
  PIDStr = PIDStr.substr(PIDStr.ڈھونڈو_پہلا_not_کا(' '));
  صحیح PID;
  اگر (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    اگر (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      واپس Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(تالا_File_Name);
  واپس std::nullopt;
}

ساکن std::error_رمز getHostID(Small_سمتیہ_Impl<حرف> &HostID) {
  HostID.clear();

#اگر USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  ساخت timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  اگر (gethostuuid(uuid, &wait) != 0)
    واپس errno_As_Error_رمز();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.انتہا());

#وگرنہ_اگر LLVM_ON_UNIX
  حرف HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.انتہا());

#ورنہ
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.انتہا());
#اگر_ختم

  واپس std::error_رمز();
}

منطقی تالا_File_Manager::processStillExecuting(StringRef HostID, صحیح PID) {
#اگر LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  اگر (getHostID(StoredHostID))
    واپس سچ; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  اگر (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    واپس جھوٹ;
#اگر_ختم

  واپس سچ;
}

نامخلا {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
جماعت Remove_Unique_تالا_File_On_Signal {
  StringRef Filename;
  منطقی RemoveImmediately;
عوامی:
  Remove_Unique_تالا_File_On_Signal(StringRef Name)
  : Filename(Name), RemoveImmediately(سچ) {
    sys::RemoveFileOnSignal(Filename, خالی_اشاریہ);
  }

  ~Remove_Unique_تالا_File_On_Signal() {
    اگر (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      واپس;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  خالی تالا_Acquired() { RemoveImmediately = جھوٹ; }
};

} // end anonymous namespace

تالا_File_Manager::تالا_File_Manager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<منطقی> تالا_File_Manager::try_تالا() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  اگر (std::error_رمز EC = sys::fs::make_absolute(AbsoluteFileName))
    واپس createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  تالا_File_Name = AbsoluteFileName;
  تالا_File_Name += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  اگر (خودکار تالا_File_Owner = read_تالا_File(تالا_File_Name)) {
    Owner = std::move(*تالا_File_Owner);
    واپس جھوٹ;
  }

  // Create a lock file that is unique to this instance.
  صحیح Unique_تالا_File_ID;
  {
    SmallString<128> Unique_تالا_File_نمونہ = تالا_File_Name;
    Unique_تالا_File_نمونہ += "-%%%%%%%%";
    SmallString<128> Unique_راستہ;
    std::error_رمز EC = sys::fs::createUniqueFile(
        Unique_تالا_File_نمونہ, Unique_تالا_File_ID, Unique_راستہ);
    اگر (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::راستہ_2::والد_راستہ(Unique_تالا_File_نمونہ);
      اگر (!Dir.خالی_خانے()) {
        اگر (std::error_رمز DirEC = sys::fs::create_directories(Dir))
          واپس createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(Unique_تالا_File_نمونہ, Unique_تالا_File_ID,
                                     Unique_راستہ);
    }

    اگر (EC)
      واپس createStringError(EC,
                               "failed to create unique file " + Unique_راستہ);

    Unique_تالا_File_Name = Unique_راستہ;
  }

  // Clean up the unique file on signal or scope exit.
  Remove_Unique_تالا_File_On_Signal RemoveUniqueFile(Unique_تالا_File_Name);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    اگر (خودکار EC = getHostID(HostID))
      واپس createStringError(EC, "failed to get host id");

    raw_fd_ostream برآمد(Unique_تالا_File_ID, /*shouldClose=*/سچ);
    برآمد << HostID << ' ' << sys::Process::getProcessId();
    برآمد.close();

    اگر (برآمد.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(برآمد.error(),
                                    "failed to write to " + Unique_تالا_File_Name);
      // Don't call report_fatal_error.
      برآمد.clear_error();
      واپس std::move(Err);
    }
  }

  جب (سچ) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_رمز EC =
        sys::fs::create_link(Unique_تالا_File_Name, تالا_File_Name);
    اگر (!EC) {
      RemoveUniqueFile.تالا_Acquired();
      Owner = OwnedByUs{};
      واپس سچ;
    }

    اگر (EC != errc::file_exists)
      واپس createStringError(EC, "failed to create link " + تالا_File_Name +
                                       " to " + Unique_تالا_File_Name);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    اگر (خودکار تالا_File_Owner = read_تالا_File(تالا_File_Name)) {
      Owner = std::move(*تالا_File_Owner);
      واپس جھوٹ;
    }

    اگر (!sys::fs::exists(تالا_File_Name)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      جاری;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    اگر ((EC = sys::fs::remove(تالا_File_Name)))
      واپس createStringError(EC, "failed to remove lockfile " +
                                       Unique_تالا_File_Name);
  }
}

تالا_File_Manager::~تالا_File_Manager() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  اگر (!std::holds_alternative<OwnedByUs>(Owner))
    واپس;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::remove(تالا_File_Name);
  sys::fs::remove(Unique_تالا_File_Name);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(Unique_تالا_File_Name);
}

Wait_For_Unlock_نتیجہ
تالا_File_Manager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  خودکار *تالا_File_Owner = std::get_if<OwnedByAnother>(&Owner);
  أكد(تالا_File_Owner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  استعمال نامخلا std::chrono_literals;
  قوت_نمائی_Backoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  جب (Backoff.wait_For_اگلا_Attempt()) {
    // FIXME: implement event-based waiting
    اگر (sys::fs::access(تالا_File_Name.c_str(), sys::fs::Access_منوال::Exist) ==
        errc::no_such_file_or_directory)
      واپس Wait_For_Unlock_نتیجہ::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    اگر (!processStillExecuting(تالا_File_Owner->OwnerHostName,
                               تالا_File_Owner->OwnerPID))
      واپس Wait_For_Unlock_نتیجہ::OwnerDied;
  }

  // Give up.
  واپس Wait_For_Unlock_نتیجہ::Timeout;
}

std::error_رمز تالا_File_Manager::unsafeUnlock() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  واپس sys::fs::remove(تالا_File_Name);
}
