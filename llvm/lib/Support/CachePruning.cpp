//===-CachePruning.cpp - LLVM Cache Directory Pruning ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the pruning of a directory based on least recently used.
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/CachePruning.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/WithColor.h"
#समावेश "llvm/Support/raw_ostream.h"

#घोषणा DEBUG_TYPE "cache-pruning"

#समावेश <set>
#समावेश <system_error>

using namespace llvm;

namespace {
काष्ठा FileInfo {
  sys::TimePoint<> Time;
  uint64_t आकार;
  std::string Path;

  /// Used to determine which files to prune first. Also used to determine
  /// set membership, so must take into account all fields.
  bool चालक<(स्थिर FileInfo &Other) स्थिर {
    वापस std::tie(Time, Other.आकार, Path) <
           std::tie(Other.Time, आकार, Other.Path);
  }
};
} // anonymous namespace

/// Write a new timestamp file with the given path. This is used for the pruning
/// interval option.
अटल व्योम writeTimestampFile(StringRef TimestampFile) {
  std::error_code EC;
  raw_fd_ostream Out(TimestampFile.str(), EC, sys::fs::OF_None);
}

अटल Expected<std::chrono::seconds> parseDuration(StringRef Duration) {
  अगर (Duration.empty())
    वापस make_error<StringError>("Duration must not be empty",
                                   inconvertibleErrorCode());

  StringRef अंक_Str = Duration.slice(0, Duration.आकार_2()-1);
  uint64_t अंक;
  अगर (अंक_Str.getAsInteger(0, अंक))
    वापस make_error<StringError>("'" + अंक_Str + "' not an integer",
                                   inconvertibleErrorCode());

  चयन (Duration.back()) {
  हाल 's':
    वापस std::chrono::seconds(अंक);
  हाल 'm':
    वापस std::chrono::minutes(अंक);
  हाल 'h':
    वापस std::chrono::hours(अंक);
  शेष:
    वापस make_error<StringError>("'" + Duration +
                                       "' must end with one of 's', 'm' or 'h'",
                                   inconvertibleErrorCode());
  }
}

Expected<CachePruningPolicy>
llvm::parseCachePruningPolicy(StringRef PolicyStr) {
  CachePruningPolicy Policy;
  std::pair<StringRef, StringRef> P = {"", PolicyStr};
  जबतक (!P.second.empty()) {
    P = P.second.split(':');

    StringRef कुंजी, मूल्य;
    std::tie(कुंजी, मूल्य) = P.first.split('=');
    अगर (कुंजी == "prune_interval") {
      स्वतः DurationOrErr = parseDuration(मूल्य);
      अगर (!DurationOrErr)
        वापस DurationOrErr.takeError();
      Policy.Interval = *DurationOrErr;
    } अन्यथा अगर (कुंजी == "prune_after") {
      स्वतः DurationOrErr = parseDuration(मूल्य);
      अगर (!DurationOrErr)
        वापस DurationOrErr.takeError();
      Policy.Expiration = *DurationOrErr;
    } अन्यथा अगर (कुंजी == "cache_size") {
      अगर (मूल्य.back() != '%')
        वापस make_error<StringError>("'" + मूल्य + "' must be a percentage",
                                       inconvertibleErrorCode());
      StringRef आकार_Str = मूल्य.drop_back();
      uint64_t आकार;
      अगर (आकार_Str.getAsInteger(0, आकार))
        वापस make_error<StringError>("'" + आकार_Str + "' not an integer",
                                       inconvertibleErrorCode());
      अगर (आकार > 100)
        वापस make_error<StringError>("'" + आकार_Str +
                                           "' must be between 0 and 100",
                                       inconvertibleErrorCode());
      Policy.Max_आकार_Percentage_Of_Available_Space = आकार;
    } अन्यथा अगर (कुंजी == "cache_size_bytes") {
      uint64_t Mult = 1;
      चयन (छोटे(मूल्य.back())) {
      हाल 'k':
        Mult = 1024;
        मूल्य = मूल्य.drop_back();
        अवरोध;
      हाल 'm':
        Mult = 1024 * 1024;
        मूल्य = मूल्य.drop_back();
        अवरोध;
      हाल 'g':
        Mult = 1024 * 1024 * 1024;
        मूल्य = मूल्य.drop_back();
        अवरोध;
      }
      uint64_t आकार;
      अगर (मूल्य.getAsInteger(0, आकार))
        वापस make_error<StringError>("'" + मूल्य + "' not an integer",
                                       inconvertibleErrorCode());
      Policy.Max_आकार_Bytes = आकार * Mult;
    } अन्यथा अगर (कुंजी == "cache_size_files") {
      अगर (मूल्य.getAsInteger(0, Policy.Max_आकार_Files))
        वापस make_error<StringError>("'" + मूल्य + "' not an integer",
                                       inconvertibleErrorCode());
    } अन्यथा {
      वापस make_error<StringError>("Unknown key: '" + कुंजी + "'",
                                     inconvertibleErrorCode());
    }
  }

  वापस Policy;
}

/// Prune the cache of files that haven't been accessed in a long time.
Expected<bool>
llvm::pruneCache(StringRef Path, CachePruningPolicy Policy,
                 स्थिर std::vector<std::unique_ptr<Memory_बफर>> &Files) {
  using namespace std::chrono;

  अगर (Path.empty())
    वापस false;

  bool isPathDir;
  अगर (sys::fs::is_directory(Path, isPathDir))
    वापस false;

  अगर (!isPathDir)
    वापस false;

  Policy.Max_आकार_Percentage_Of_Available_Space =
      std::min(Policy.Max_आकार_Percentage_Of_Available_Space, 100u);

  अगर (Policy.Expiration == seconds(0) &&
      Policy.Max_आकार_Percentage_Of_Available_Space == 0 &&
      Policy.Max_आकार_Bytes == 0 && Policy.Max_आकार_Files == 0) {
    LLVM_DEBUG(dbgs() << "No pruning settings set, exit early\n");
    // Nothing will be pruned, early exit
    वापस false;
  }

  // Try to stat() the timestamp file.
  SmallString<128> TimestampFile(Path);
  sys::path::append(TimestampFile, "llvmcache.timestamp");
  sys::fs::file_status FileStatus;
  स्थिर स्वतः CurrentTime = system_clock::now();
  अगर (स्वतः EC = sys::fs::status(TimestampFile, FileStatus)) {
    अगर (EC == errc::no_such_file_or_directory) {
      // If the timestamp file wasn't there, create one now.
      writeTimestampFile(TimestampFile);
    } अन्यथा {
      // Unknown error?
      वापस false;
    }
  } अन्यथा {
    अगर (!Policy.Interval)
      वापस false;
    अगर (Policy.Interval != seconds(0)) {
      // Check whether the time stamp is older than our pruning interval.
      // If not, do nothing.
      स्थिर स्वतः TimeStampModTime = FileStatus.getLastModificationTime();
      स्वतः TimeStampAge = CurrentTime - TimeStampModTime;
      अगर (TimeStampAge <= *Policy.Interval) {
        LLVM_DEBUG(dbgs() << "Timestamp file too recent ("
                          << duration_cast<seconds>(TimeStampAge).गिनती()
                          << "s old), do not prune.\n");
        वापस false;
      }
    }
    // Write a new timestamp file so that nobody else attempts to prune.
    // There is a benign race condition here, if two processes happen to
    // notice at the same time that the timestamp is out-of-date.
    writeTimestampFile(TimestampFile);
  }

  // Keep track of files to delete to get below the size limit.
  // Order by time of last use so that recently used files are preserved.
  std::set<FileInfo> FileInfos;
  uint64_t योग_आकार = 0;

  // Walk the entire directory cache, looking for unused files.
  std::error_code EC;
  SmallString<128> CachePathNative;
  sys::path::native(Path, CachePathNative);
  // Walk all of the files within this directory.
  क्रम (sys::fs::directory_iterator File(CachePathNative, EC), FileEnd;
       File != FileEnd && !EC; File.increment(EC)) {
    // Ignore filenames not beginning with "llvmcache-" or "Thin-". This
    // includes the timestamp file as well as any files created by the user.
    // This acts as a safeguard against data loss if the user specifies the
    // wrong directory as their cache directory.
    StringRef filename = sys::path::filename(File->path());
    अगर (!filename.starts_with("llvmcache-") && !filename.starts_with("Thin-"))
      जारी;

    // Look at this file. If we can't stat it, there's nothing interesting
    // there.
    ErrorOr<sys::fs::basic_file_status> StatusOrErr = File->status();
    अगर (!StatusOrErr) {
      LLVM_DEBUG(dbgs() << "Ignore " << File->path() << " (can't stat)\n");
      जारी;
    }

    // If the file hasn't been used recently enough, delete it
    स्थिर स्वतः FileAccessTime = StatusOrErr->getLastAccessedTime();
    स्वतः FileAge = CurrentTime - FileAccessTime;
    अगर (Policy.Expiration != seconds(0) && FileAge > Policy.Expiration) {
      LLVM_DEBUG(dbgs() << "Remove " << File->path() << " ("
                        << duration_cast<seconds>(FileAge).गिनती()
                        << "s old)\n");
      sys::fs::हटाओ(File->path());
      जारी;
    }

    // Leave it here for now, but add it to the list of size-based pruning.
    योग_आकार += StatusOrErr->get_आकार();
    FileInfos.जोड़ो({FileAccessTime, StatusOrErr->get_आकार(), File->path()});
  }

  स्वतः FileInfo = FileInfos.begin();
  माप_प्रकार अंक_Files = FileInfos.आकार_2();

  स्वतः RemoveCacheFile = [&]() {
    // Remove the file.
    sys::fs::हटाओ(FileInfo->Path);
    // Update size
    योग_आकार -= FileInfo->आकार;
    अंक_Files--;
    LLVM_DEBUG(dbgs() << " - Remove " << FileInfo->Path << " (size "
                      << FileInfo->आकार << "), new occupancy is " << योग_आकार
                      << "%\n");
    ++FileInfo;
  };

  // files.size() is greater the number of inputs  by one. However, a timestamp
  // file is created and stored in the cache directory if --thinlto-cache-policy
  // option is used. Therefore, files.size() is used as ActualNums.
  स्थिर माप_प्रकार ActualNums = Files.आकार_2();
  अगर (Policy.Max_आकार_Files && ActualNums > Policy.Max_आकार_Files)
    WithColor::warning()
        << "ThinLTO cache pruning happens since the number of created files ("
        << ActualNums << ") exceeds the maximum number of files ("
        << Policy.Max_आकार_Files
        << "); consider adjusting --thinlto-cache-policy\n";

  // Prune for number of files.
  अगर (Policy.Max_आकार_Files)
    जबतक (अंक_Files > Policy.Max_आकार_Files)
      RemoveCacheFile();

  // Prune for size now if needed
  अगर (Policy.Max_आकार_Percentage_Of_Available_Space > 0 || Policy.Max_आकार_Bytes > 0) {
    स्वतः SpaceInfoOrErr = sys::fs::disk_space(Path);
    अगर (!SpaceInfoOrErr) {
      स्वतः EC = SpaceInfoOrErr.getError();
      वापस createStringError(EC,
                               "cannot get available disk space for '%s': '%s'",
                               Path.str().c_str(), EC.message().c_str());
    }
    sys::fs::space_info SpaceInfo = SpaceInfoOrErr.लो();
    स्वतः AvailableSpace = योग_आकार + SpaceInfo.मुक्त;

    अगर (Policy.Max_आकार_Percentage_Of_Available_Space == 0)
      Policy.Max_आकार_Percentage_Of_Available_Space = 100;
    अगर (Policy.Max_आकार_Bytes == 0)
      Policy.Max_आकार_Bytes = AvailableSpace;
    स्वतः योग_आकार_Target = std::min<uint64_t>(
        AvailableSpace * Policy.Max_आकार_Percentage_Of_Available_Space / 100ull,
        Policy.Max_आकार_Bytes);

    LLVM_DEBUG(dbgs() << "Occupancy: " << ((100 * योग_आकार) / AvailableSpace)
                      << "% target is: "
                      << Policy.Max_आकार_Percentage_Of_Available_Space << "%, "
                      << Policy.Max_आकार_Bytes << " bytes\n");

    माप_प्रकार ActualSizes = 0;
    क्रम (स्थिर स्वतः &File : Files)
      अगर (File)
        ActualSizes += File->get_बफर_आकार();

    अगर (ActualSizes > योग_आकार_Target)
      WithColor::warning()
          << "ThinLTO cache pruning happens since the total size of the cache "
             "files consumed by the current link job ("
          << ActualSizes << "  bytes) exceeds maximum cache size ("
          << योग_आकार_Target
          << " bytes); consider adjusting --thinlto-cache-policy\n";

    // Remove the oldest accessed files first, till we get below the threshold.
    जबतक (योग_आकार > योग_आकार_Target && FileInfo != FileInfos.end())
      RemoveCacheFile();
  }
  वापस true;
}
