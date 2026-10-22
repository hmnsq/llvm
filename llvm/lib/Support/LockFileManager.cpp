//===--- LockFileManager.cpp - File-level Locking Utility------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/LockFileManager.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Config/llvm-config.h" // for LLVM_ON_UNIX
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/ErrorOr.h"
#అన్తర్భావ "llvm/Support/ExponentialBackoff.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/IOSandbox.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/Signals.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cerrno>
#అన్తర్భావ <chrono>
#అన్తర్భావ <ctime>
#అన్తర్భావ <memory>
#అన్తర్భావ <system_error>
#అన్తర్భావ <tuple>

#నిర్వచితమైతే _WIN32
#అన్తర్భావ <windows.h>
#అయితే_ముగింపు
#ఒకవేళ LLVM_ON_UNIX
#అన్తర్భావ <unistd.h>
#అయితే_ముగింపు

#ఒకవేళ defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1050)
#నిర్వచించు USE_OSX_GETHOSTUUID 1
#లేకపోతే
#నిర్వచించు USE_OSX_GETHOSTUUID 0
#అయితే_ముగింపు

#ఒకవేళ USE_OSX_GETHOSTUUID
#అన్తర్భావ <uuid/uuid.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

/// Attempt to read the lock file with the given name, if it exists.
///
/// \param LockFileName The name of the lock file to read.
///
/// \returns The process ID of the process that owns this lock file
std::optional<తాళం_File_Manager::OwnedByAnother>
తాళం_File_Manager::read_తాళం_File(StringRef తాళం_File_Name) {
  // Read the owning host and PID out of the lock file. If it appears that the
  // owning process is dead, the lock file is invalid.
  ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> MBOrErr =
      జ్ఞాపకం_బఫరు::getFile(తాళం_File_Name);
  ఒకవేళ (!MBOrErr) {
    sys::fs::remove(తాళం_File_Name);
    తిరిగి std::nullopt;
  }
  జ్ఞాపకం_బఫరు &MB = *MBOrErr.get();

  StringRef Hostname;
  StringRef PIDStr;
  std::tie(Hostname, PIDStr) = getToken(MB.get_బఫరు(), " ");
  PIDStr = PIDStr.substr(PIDStr.కనుగొను_మొదటి_not_యొక్క(' '));
  పూర్ణాంకం PID;
  ఒకవేళ (!PIDStr.getAsInteger(10, PID)) {
    OwnedByAnother Owner;
    Owner.OwnerHostName = Hostname;
    Owner.OwnerPID = PID;
    ఒకవేళ (processStillExecuting(Owner.OwnerHostName, Owner.OwnerPID))
      తిరిగి Owner;
  }

  // Delete the lock file. It's invalid anyway.
  sys::fs::remove(తాళం_File_Name);
  తిరిగి std::nullopt;
}

స్థిర std::error_సంకేతం getHostID(Small_సదిశ_Impl<అక్షరం> &HostID) {
  HostID.clear();

#ఒకవేళ USE_OSX_GETHOSTUUID
  // On OS X, use the more stable hardware UUID instead of hostname.
  నిర్మాణం timespec wait = {1, 0}; // 1 second.
  uuid_t uuid;
  ఒకవేళ (gethostuuid(uuid, &wait) != 0)
    తిరిగి errno_As_Error_సంకేతం();

  uuid_string_t UUIDStr;
  uuid_unparse(uuid, UUIDStr);
  StringRef UUIDRef(UUIDStr);
  HostID.append(UUIDRef.begin(), UUIDRef.చివర());

#లేదావేళ LLVM_ON_UNIX
  అక్షరం HostName[256];
  HostName[255] = 0;
  HostName[0] = 0;
  gethostname(HostName, 255);
  StringRef HostNameRef(HostName);
  HostID.append(HostNameRef.begin(), HostNameRef.చివర());

#లేకపోతే
  StringRef Dummy("localhost");
  HostID.append(Dummy.begin(), Dummy.చివర());
#అయితే_ముగింపు

  తిరిగి std::error_సంకేతం();
}

తార్కిక తాళం_File_Manager::processStillExecuting(StringRef HostID, పూర్ణాంకం PID) {
#ఒకవేళ LLVM_ON_UNIX && !defined(__ANDROID__)
  SmallString<256> StoredHostID;
  ఒకవేళ (getHostID(StoredHostID))
    తిరిగి నిజం; // Conservatively assume it's executing on error.

  // Check whether the process is dead. If so, we're done.
  ఒకవేళ (StoredHostID == HostID && getsid(PID) == -1 && errno == ESRCH)
    తిరిగి అబద్ధం;
#అయితే_ముగింపు

  తిరిగి నిజం;
}

నామాకాశ {

/// An RAII helper object ensure that the unique lock file is removed.
///
/// Ensures that if there is an error or a signal before we finish acquiring the
/// lock, the unique file will be removed. And if we successfully take the lock,
/// the signal handler is left in place so that signals while the lock is held
/// will remove the unique lock file. The caller should ensure there is a
/// matching call to sys::DontRemoveFileOnSignal when the lock is released.
తరగతి Remove_Unique_తాళం_File_On_Signal {
  StringRef Filename;
  తార్కిక RemoveImmediately;
బహిరంగ:
  Remove_Unique_తాళం_File_On_Signal(StringRef Name)
  : Filename(Name), RemoveImmediately(నిజం) {
    sys::RemoveFileOnSignal(Filename, శూన్యసూచిక);
  }

  ~Remove_Unique_తాళం_File_On_Signal() {
    ఒకవేళ (!RemoveImmediately) {
      // Leave the signal handler enabled. It will be removed when the lock is
      // released.
      తిరిగి;
    }
    sys::fs::remove(Filename);
    sys::DontRemoveFileOnSignal(Filename);
  }

  శూన్య తాళం_Acquired() { RemoveImmediately = అబద్ధం; }
};

} // end anonymous namespace

తాళం_File_Manager::తాళం_File_Manager(StringRef FileName)
    : FileName(FileName), Owner(OwnerUnknown{}) {}

Expected<తార్కిక> తాళం_File_Manager::try_తాళం() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ప్రతిజ్ఞా(std::holds_alternative<OwnerUnknown>(Owner) &&
         "lock has already been attempted");

  SmallString<128> AbsoluteFileName(FileName);
  ఒకవేళ (std::error_సంకేతం EC = sys::fs::make_absolute(AbsoluteFileName))
    తిరిగి createStringError(EC, "failed to obtain absolute path for " +
                                     AbsoluteFileName);
  తాళం_File_Name = AbsoluteFileName;
  తాళం_File_Name += ".lock";

  // If the lock file already exists, don't bother to try to create our own
  // lock file; it won't work anyway. Just figure out who owns this lock file.
  ఒకవేళ (స్వయంచాలక తాళం_File_Owner = read_తాళం_File(తాళం_File_Name)) {
    Owner = std::move(*తాళం_File_Owner);
    తిరిగి అబద్ధం;
  }

  // Create a lock file that is unique to this instance.
  పూర్ణాంకం Unique_తాళం_File_ID;
  {
    SmallString<128> Unique_తాళం_File_నమూనా = తాళం_File_Name;
    Unique_తాళం_File_నమూనా += "-%%%%%%%%";
    SmallString<128> Unique_మార్గం;
    std::error_సంకేతం EC = sys::fs::createUniqueFile(
        Unique_తాళం_File_నమూనా, Unique_తాళం_File_ID, Unique_మార్గం);
    ఒకవేళ (EC == errc::no_such_file_or_directory) {
      SmallString<128> Dir = sys::మార్గం_2::మాతృ_మార్గం(Unique_తాళం_File_నమూనా);
      ఒకవేళ (!Dir.ఖాళీలు()) {
        ఒకవేళ (std::error_సంకేతం DirEC = sys::fs::create_directories(Dir))
          తిరిగి createStringError(DirEC,
                                   "failed to create lock directory " + Dir);
      }

      // Retry creating lock file
      EC = sys::fs::createUniqueFile(Unique_తాళం_File_నమూనా, Unique_తాళం_File_ID,
                                     Unique_మార్గం);
    }

    ఒకవేళ (EC)
      తిరిగి createStringError(EC,
                               "failed to create unique file " + Unique_మార్గం);

    Unique_తాళం_File_Name = Unique_మార్గం;
  }

  // Clean up the unique file on signal or scope exit.
  Remove_Unique_తాళం_File_On_Signal RemoveUniqueFile(Unique_తాళం_File_Name);

  // Write our process ID to our unique lock file.
  {
    SmallString<256> HostID;
    ఒకవేళ (స్వయంచాలక EC = getHostID(HostID))
      తిరిగి createStringError(EC, "failed to get host id");

    raw_fd_ostream బహిర్గతం(Unique_తాళం_File_ID, /*shouldClose=*/నిజం);
    బహిర్గతం << HostID << ' ' << sys::Process::getProcessId();
    బహిర్గతం.close();

    ఒకవేళ (బహిర్గతం.has_error()) {
      // We failed to write out PID, so report the error and fail.
      Error Err = createStringError(బహిర్గతం.error(),
                                    "failed to write to " + Unique_తాళం_File_Name);
      // Don't call report_fatal_error.
      బహిర్గతం.clear_error();
      తిరిగి std::move(Err);
    }
  }

  అయితే (నిజం) {
    // Create a link from the lock file name. If this succeeds, we're done.
    std::error_సంకేతం EC =
        sys::fs::create_link(Unique_తాళం_File_Name, తాళం_File_Name);
    ఒకవేళ (!EC) {
      RemoveUniqueFile.తాళం_Acquired();
      Owner = OwnedByUs{};
      తిరిగి నిజం;
    }

    ఒకవేళ (EC != errc::file_exists)
      తిరిగి createStringError(EC, "failed to create link " + తాళం_File_Name +
                                       " to " + Unique_తాళం_File_Name);

    // Someone else managed to create the lock file first. Read the process ID
    // from the lock file.
    ఒకవేళ (స్వయంచాలక తాళం_File_Owner = read_తాళం_File(తాళం_File_Name)) {
      Owner = std::move(*తాళం_File_Owner);
      తిరిగి అబద్ధం;
    }

    ఒకవేళ (!sys::fs::exists(తాళం_File_Name)) {
      // The previous owner released the lock file before we could read it.
      // Try to get ownership again.
      కొనసాగు;
    }

    // There is a lock file that nobody owns; try to clean it up and get
    // ownership.
    ఒకవేళ ((EC = sys::fs::remove(తాళం_File_Name)))
      తిరిగి createStringError(EC, "failed to remove lockfile " +
                                       Unique_తాళం_File_Name);
  }
}

తాళం_File_Manager::~తాళం_File_Manager() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ఒకవేళ (!std::holds_alternative<OwnedByUs>(Owner))
    తిరిగి;

  // Since we own the lock, remove the lock file and our own unique lock file.
  sys::fs::remove(తాళం_File_Name);
  sys::fs::remove(Unique_తాళం_File_Name);
  // The unique file is now gone, so remove it from the signal handler. This
  // matches a sys::RemoveFileOnSignal() in LockFileManager().
  sys::DontRemoveFileOnSignal(Unique_తాళం_File_Name);
}

Wait_For_Unlock_ఫలితం
తాళం_File_Manager::waitForUnlockFor(std::chrono::seconds MaxSeconds) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  స్వయంచాలక *తాళం_File_Owner = std::get_if<OwnedByAnother>(&Owner);
  ప్రతిజ్ఞా(తాళం_File_Owner &&
         "waiting for lock to be unlocked without knowing the owner");

  // Since we don't yet have an event-based method to wait for the lock file,
  // use randomized exponential backoff, similar to Ethernet collision
  // algorithm. This improves performance on machines with high core counts
  // when the file lock is heavily contended by multiple clang processes
  ఉపయోగించి నామాకాశ std::chrono_literals;
  ఘాతాంక_Backoff Backoff(MaxSeconds, 10ms, 500ms);

  // Wait first as this is only called when the lock is known to be held.
  అయితే (Backoff.wait_For_తదుపరి_Attempt()) {
    // FIXME: implement event-based waiting
    ఒకవేళ (sys::fs::access(తాళం_File_Name.c_str(), sys::fs::Access_బహుళకం::Exist) ==
        errc::no_such_file_or_directory)
      తిరిగి Wait_For_Unlock_ఫలితం::Success;

    // If the process owning the lock died without cleaning up, just bail out.
    ఒకవేళ (!processStillExecuting(తాళం_File_Owner->OwnerHostName,
                               తాళం_File_Owner->OwnerPID))
      తిరిగి Wait_For_Unlock_ఫలితం::OwnerDied;
  }

  // Give up.
  తిరిగి Wait_For_Unlock_ఫలితం::Timeout;
}

std::error_సంకేతం తాళం_File_Manager::unsafeUnlock() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  తిరిగి sys::fs::remove(తాళం_File_Name);
}
