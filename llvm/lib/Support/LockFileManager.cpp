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

#यदि_परिभाषितम् _WIN32
#अन्तर्भाव <windows.h>
#यदि_अन्तः
#यदि LLVM_ON_UNIX
#अन्तर्भाव <unistd.h>
#यदि_अन्तः

#यदि defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#परिभाषय USE_OSX_GETHOSTUUID 1
#अन्यथा
#परिभाषय USE_OSX_GETHOSTUUID 0
#यदि_अन्तः

#यदि USE_OSX_GETHOSTUUID
#अन्तर्भाव <uuid/uuid.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

/// Attempt to read the lock file with the given name, if it exists.
///
/// \param LockFileName The name of the lock file to read.
///
/// \returns The process ID of the process that owns this lock file
std::optional<तालकम्_File_Manager::OwnedByAnother>
तालकम्_File_Manager::read_तालकम्_File(StringRef तालकम्_File_Name) {
  // Read the owning host and PID out of the lock file. If it appears that the
  // owning process is dead, the lock file is invalid.
  ErrorOr<std::unique_ptr<स्मृतिः_बफर>> MBOrErr =
      स्मृतिः_बफर::getFile(तालकम्_File_Name);
  यदि (!MBOrErr) {
    sys::fs::remove(तालकम्_File_Name);
    प्रत्यागम std::nullopt;
  }
  स्मृतिः_बफर &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.get_बफर(), " ");
  PIDStr = PIDStr.substr(PIDStr.find_first_not_सम्बन्धि(' '));
  पूर्णाङ्क PID;
  यदि (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    यदि (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      प्रत्यागम Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(तालकम्_File_Name);
  प्रत्यागम std::nullopt;
}

स्थिर std::error_संहिता getHostID(Small_सदिशम्_Impl<वर्ण> &HostID) {
  HostID.clear();

#यदि USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  संरचना timespec प्रतीक्षस्व = {1, 0}; // 1 second.
  uuid_t uuid;
  यदि (gethostuuid(uuid, &प्रतीक्षस्व) != 0)
    प्रत्यागम errno_As_Error_संहिता();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.अन्तः());

#नोचेद्यदि LLVM_ON_UNIX
  वर्ण HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.अन्तः());

#अन्यथा
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.अन्तः());
#यदि_अन्तः

  प्रत्यागम std::error_संहिता();
}

तार्किक तालकम्_File_Manager::processStillExecuting(StringRef HostID, पूर्णाङ्क PID) {
#यदि LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  यदि (getHostID(StoredHostID))
    प्रत्यागम सत्य; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  यदि (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    प्रत्यागम असत्य;
#यदि_अन्तः

  प्रत्यागम सत्य;
}

नामाकाश {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
वर्ग Remove_Unique_तालकम्_File_On_Signal {
  StringRef Filename;
  तार्किक RemoveImmediately;
सार्वजनिक:
  Remove_Unique_तालकम्_File_On_Signal(StringRef Name)
  : Filename(Name), RemoveImmediately(सत्य) {
    sys::RemoveFileOnSignal(Filename, शून्यसूचकः);
  }

  ~Remove_Unique_तालकम्_File_On_Signal() {
    यदि (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      प्रत्यागम;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  शून्य तालकम्_Acquired() { RemoveImmediately = असत्य; }
};

} // end anonymous namespace

तालकम्_File_Manager::तालकम्_File_Manager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<तार्किक> तालकम्_File_Manager::try_तालकम्() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  यदि (std::error_संहिता EC = sys::fs::make_absolute(AbsoluteFileName))
    प्रत्यागम createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  तालकम्_File_Name = AbsoluteFileName;
  तालकम्_File_Name += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  यदि (स्वचालित तालकम्_File_Owner = read_तालकम्_File(तालकम्_File_Name)) {
    Owner = std::move(*तालकम्_File_Owner);
    प्रत्यागम असत्य;
  }

  // Create a lock file that is unique to this instance.
  पूर्णाङ्क Unique_तालकम्_File_ID;
  {
    SmallString<128> Unique_तालकम्_File_आदर्शः = तालकम्_File_Name;
    Unique_तालकम्_File_आदर्शः += "-%%%%%%%%";
    SmallString<128> Unique_मार्गः;
    std::error_संहिता EC = sys::fs::createUniqueFile(
        Unique_तालकम्_File_आदर्शः, Unique_तालकम्_File_ID, Unique_मार्गः);
    यदि (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::मार्गः_2::parent_मार्गः(Unique_तालकम्_File_आदर्शः);
      यदि (!Dir.रिक्तानि()) {
        यदि (std::error_संहिता DirEC = sys::fs::create_directories(Dir))
          प्रत्यागम createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(Unique_तालकम्_File_आदर्शः, Unique_तालकम्_File_ID,
                                     Unique_मार्गः);
    }

    यदि (EC)
      प्रत्यागम createStringError(EC,
                               "failed to create unique file " + Unique_मार्गः);

    Unique_तालकम्_File_Name = Unique_मार्गः;
  }

  // Clean up the unique file on signal or scope exit.
  Remove_Unique_तालकम्_File_On_Signal RemoveUniqueFile(Unique_तालकम्_File_Name);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    यदि (स्वचालित EC = getHostID(HostID))
      प्रत्यागम createStringError(EC, "failed to get host id");

    raw_fd_ostream बहिर्गमनम्(Unique_तालकम्_File_ID, /*shouldClose=*/सत्य);
    बहिर्गमनम् << HostID << ' ' << sys::Process::getProcessId();
    बहिर्गमनम्.close();

    यदि (बहिर्गमनम्.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(बहिर्गमनम्.error(),
                                    "failed to write to " + Unique_तालकम्_File_Name);
      // Don't call report_fatal_error.
      बहिर्गमनम्.clear_error();
      प्रत्यागम std::move(Err);
    }
  }

  यावत् (सत्य) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_संहिता EC =
        sys::fs::create_link(Unique_तालकम्_File_Name, तालकम्_File_Name);
    यदि (!EC) {
      RemoveUniqueFile.तालकम्_Acquired();
      Owner = OwnedByUs{};
      प्रत्यागम सत्य;
    }

    यदि (EC != errc::file_exists)
      प्रत्यागम createStringError(EC, "failed to create link " + तालकम्_File_Name +
                                       " to " + Unique_तालकम्_File_Name);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    यदि (स्वचालित तालकम्_File_Owner = read_तालकम्_File(तालकम्_File_Name)) {
      Owner = std::move(*तालकम्_File_Owner);
      प्रत्यागम असत्य;
    }

    यदि (!sys::fs::exists(तालकम्_File_Name)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      अनुवर्त;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    यदि ((EC = sys::fs::remove(तालकम्_File_Name)))
      प्रत्यागम createStringError(EC, "failed to remove lockfile " +
                                       Unique_तालकम्_File_Name);
  }
}

तालकम्_File_Manager::~तालकम्_File_Manager() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  यदि (!std::holds_alternative<OwnedByUs>(Owner))
    प्रत्यागम;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::remove(तालकम्_File_Name);
  sys::fs::remove(Unique_तालकम्_File_Name);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(Unique_तालकम्_File_Name);
}

Wait_For_Unlock_परिणाम
तालकम्_File_Manager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  स्वचालित *तालकम्_File_Owner = std::get_if<OwnedByAnother>(&Owner);
  प्रतिज्ञा(तालकम्_File_Owner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  उपयुज्य नामाकाश std::chrono_literals;
  घातीयम्_Backoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  यावत् (Backoff.wait_For_अग्र_Attempt()) {
    // FIXME: implement event-based waiting
    यदि (sys::fs::access(तालकम्_File_Name.c_str(), sys::fs::Access_बहुलकम्::Exist) ==
        errc::no_such_file_or_directory)
      प्रत्यागम Wait_For_Unlock_परिणाम::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    यदि (!processStillExecuting(तालकम्_File_Owner->OwnerHostName,
                               तालकम्_File_Owner->OwnerPID))
      प्रत्यागम Wait_For_Unlock_परिणाम::OwnerDied;
  }

  // Give up.
  प्रत्यागम Wait_For_Unlock_परिणाम::Timeout;
}

std::error_संहिता तालकम्_File_Manager::unsafeUnlock() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  प्रत्यागम sys::fs::remove(तालकम्_File_Name);
}
