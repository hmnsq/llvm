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

#अन्तर्भाव "llvm/Support/CachePruning.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/WithColor.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "cache-pruning"

#अन्तर्भाव <set>
#अन्तर्भाव <system_error>

using नामाकाश llvm;

नामाकाश {
संरचना FileInfo {
  sys::TimePoint<> Time;
  uint64_t Size;
  std::string Path;

  /// Used to determine which files to prune first. Also used to determine
  /// set membership, so must take into account all fields.
  तार्किक चालक<(स्थिरांक FileInfo &Other) स्थिरांक {
    परत std::tie(Time, Other.Size, Path) <
           std::tie(Other.Time, Size, Other.Path);
  }
};
} // anonymous namespace

/// Write a new timestamp file with the given path. This is used for the pruning
/// interval option.
स्थिर शून्य writeTimestampFile(StringRef TimestampFile) {
  std::error_code EC;
  raw_fd_ostream Out(TimestampFile.str(), EC, sys::fs::OF_None);
}

स्थिर Expected<std::chrono::seconds> parseDuration(StringRef Duration) {
  जर (Duration.empty())
    परत make_error<StringError>("Duration must not be empty",
                                   inconvertibleErrorCode());

  StringRef अंक_Str = Duration.slice(0, Duration.size()-1);
  uint64_t अंक;
  जर (अंक_Str.getAsInteger(0, अंक))
    परत make_error<StringError>("'" + अंक_Str + "' not an integer",
                                   inconvertibleErrorCode());

  निवड (Duration.back()) {
  स्थिती 's':
    परत std::chrono::seconds(अंक);
  स्थिती 'm':
    परत std::chrono::minutes(अंक);
  स्थिती 'h':
    परत std::chrono::hours(अंक);
  पूर्वनिर्धारित:
    परत make_error<StringError>("'" + Duration +
                                       "' must end with one of 's', 'm' or 'h'",
                                   inconvertibleErrorCode());
  }
}

Expected<CachePruningPolicy>
llvm::parseCachePruningPolicy(StringRef PolicyStr) {
  CachePruningPolicy Policy;
  std::pair<StringRef, StringRef> P = {"", PolicyStr};
  पर्यंत (!P.second.empty()) {
    P = P.second.split(':');

    StringRef किल्ली, मूल्य;
    std::tie(किल्ली, मूल्य) = P.first.split('=');
    जर (किल्ली == "prune_interval") {
      स्वतः DurationOrErr = parseDuration(मूल्य);
      जर (!DurationOrErr)
        परत DurationOrErr.takeError();
      Policy.Interval = *DurationOrErr;
    } अन्यथा जर (किल्ली == "prune_after") {
      स्वतः DurationOrErr = parseDuration(मूल्य);
      जर (!DurationOrErr)
        परत DurationOrErr.takeError();
      Policy.Expiration = *DurationOrErr;
    } अन्यथा जर (किल्ली == "cache_size") {
      जर (मूल्य.back() != '%')
        परत make_error<StringError>("'" + मूल्य + "' must be a percentage",
                                       inconvertibleErrorCode());
      StringRef SizeStr = मूल्य.drop_back();
      uint64_t Size;
      जर (SizeStr.getAsInteger(0, Size))
        परत make_error<StringError>("'" + SizeStr + "' not an integer",
                                       inconvertibleErrorCode());
      जर (Size > 100)
        परत make_error<StringError>("'" + SizeStr +
                                           "' must be between 0 and 100",
                                       inconvertibleErrorCode());
      Policy.MaxSizePercentageOfAvailableSpace = Size;
    } अन्यथा जर (किल्ली == "cache_size_bytes") {
      uint64_t Mult = 1;
      निवड (छोटे(मूल्य.back())) {
      स्थिती 'k':
        Mult = 1024;
        मूल्य = मूल्य.drop_back();
        थांबा;
      स्थिती 'm':
        Mult = 1024 * 1024;
        मूल्य = मूल्य.drop_back();
        थांबा;
      स्थिती 'g':
        Mult = 1024 * 1024 * 1024;
        मूल्य = मूल्य.drop_back();
        थांबा;
      }
      uint64_t Size;
      जर (मूल्य.getAsInteger(0, Size))
        परत make_error<StringError>("'" + मूल्य + "' not an integer",
                                       inconvertibleErrorCode());
      Policy.MaxSizeBytes = Size * Mult;
    } अन्यथा जर (किल्ली == "cache_size_files") {
      जर (मूल्य.getAsInteger(0, Policy.MaxSizeFiles))
        परत make_error<StringError>("'" + मूल्य + "' not an integer",
                                       inconvertibleErrorCode());
    } अन्यथा {
      परत make_error<StringError>("Unknown key: '" + किल्ली + "'",
                                     inconvertibleErrorCode());
    }
  }

  परत Policy;
}

/// Prune the cache of files that haven't been accessed in a long time.
Expected<तार्किक>
llvm::pruneCache(StringRef Path, CachePruningPolicy Policy,
                 स्थिरांक std::vector<std::unique_ptr<MemoryBuffer>> &Files) {
  using नामाकाश std::chrono;

  जर (Path.empty())
    परत असत्य;

  तार्किक isPathDir;
  जर (sys::fs::is_directory(Path, isPathDir))
    परत असत्य;

  जर (!isPathDir)
    परत असत्य;

  Policy.MaxSizePercentageOfAvailableSpace =
      std::min(Policy.MaxSizePercentageOfAvailableSpace, 100u);

  जर (Policy.Expiration == seconds(0) &&
      Policy.MaxSizePercentageOfAvailableSpace == 0 &&
      Policy.MaxSizeBytes == 0 && Policy.MaxSizeFiles == 0) {
    LLVM_DEBUG(dbgs() << "No pruning settings set, exit early\n");
    // Nothing will be pruned, early exit
    परत असत्य;
  }

  // Try to stat() the timestamp file.
  SmallString<128> TimestampFile(Path);
  sys::path::append(TimestampFile, "llvmcache.timestamp");
  sys::fs::file_status FileStatus;
  स्थिरांक स्वतः CurrentTime = system_clock::now();
  जर (स्वतः EC = sys::fs::status(TimestampFile, FileStatus)) {
    जर (EC == errc::no_such_file_or_directory) {
      // If the timestamp file wasn't there, create one now.
      writeTimestampFile(TimestampFile);
    } अन्यथा {
      // Unknown error?
      परत असत्य;
    }
  } अन्यथा {
    जर (!Policy.Interval)
      परत असत्य;
    जर (Policy.Interval != seconds(0)) {
      // Check whether the time stamp is older than our pruning interval.
      // If not, do nothing.
      स्थिरांक स्वतः TimeStampModTime = FileStatus.getLastModificationTime();
      स्वतः TimeStampAge = CurrentTime - TimeStampModTime;
      जर (TimeStampAge <= *Policy.Interval) {
        LLVM_DEBUG(dbgs() << "Timestamp file too recent ("
                          << duration_cast<seconds>(TimeStampAge).मोजणी()
                          << "s old), do not prune.\n");
        परत असत्य;
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
  uint64_t TotalSize = 0;

  // Walk the entire directory cache, looking for unused files.
  std::error_code EC;
  SmallString<128> CachePathNative;
  sys::path::native(Path, CachePathNative);
  // Walk all of the files within this directory.
  साठी (sys::fs::directory_iterator File(CachePathNative, EC), FileEnd;
       File != FileEnd && !EC; File.increment(EC)) {
    // Ignore filenames not beginning with "llvmcache-" or "Thin-". This
    // includes the timestamp file as well as any files created by the user.
    // This acts as a safeguard against data loss if the user specifies the
    // wrong directory as their cache directory.
    StringRef filename = sys::path::filename(File->path());
    जर (!filename.starts_with("llvmcache-") && !filename.starts_with("Thin-"))
      सुरूठेवा;

    // Look at this file. If we can't stat it, there's nothing interesting
    // there.
    ErrorOr<sys::fs::basic_file_status> StatusOrErr = File->status();
    जर (!StatusOrErr) {
      LLVM_DEBUG(dbgs() << "Ignore " << File->path() << " (can't stat)\n");
      सुरूठेवा;
    }

    // If the file hasn't been used recently enough, delete it
    स्थिरांक स्वतः FileAccessTime = StatusOrErr->getLastAccessedTime();
    स्वतः FileAge = CurrentTime - FileAccessTime;
    जर (Policy.Expiration != seconds(0) && FileAge > Policy.Expiration) {
      LLVM_DEBUG(dbgs() << "Remove " << File->path() << " ("
                        << duration_cast<seconds>(FileAge).मोजणी()
                        << "s old)\n");
      sys::fs::हटाओ(File->path());
      सुरूठेवा;
    }

    // Leave it here for now, but add it to the list of size-based pruning.
    TotalSize += StatusOrErr->getSize();
    FileInfos.घाला({FileAccessTime, StatusOrErr->getSize(), File->path()});
  }

  स्वतः FileInfo = FileInfos.begin();
  माप_प्रकार अंक_Files = FileInfos.size();

  स्वतः RemoveCacheFile = [&]() {
    // Remove the file.
    sys::fs::हटाओ(FileInfo->Path);
    // Update size
    TotalSize -= FileInfo->Size;
    अंक_Files--;
    LLVM_DEBUG(dbgs() << " - Remove " << FileInfo->Path << " (size "
                      << FileInfo->Size << "), new occupancy is " << TotalSize
                      << "%\n");
    ++FileInfo;
  };

  // files.size() is greater the number of inputs  by one. However, a timestamp
  // file is created and stored in the cache directory if --thinlto-cache-policy
  // option is used. Therefore, files.size() is used as ActualNums.
  स्थिरांक माप_प्रकार ActualNums = Files.size();
  जर (Policy.MaxSizeFiles && ActualNums > Policy.MaxSizeFiles)
    WithColor::warning()
        << "ThinLTO cache pruning happens since the number of created files ("
        << ActualNums << ") exceeds the maximum number of files ("
        << Policy.MaxSizeFiles
        << "); consider adjusting --thinlto-cache-policy\n";

  // Prune for number of files.
  जर (Policy.MaxSizeFiles)
    पर्यंत (अंक_Files > Policy.MaxSizeFiles)
      RemoveCacheFile();

  // Prune for size now if needed
  जर (Policy.MaxSizePercentageOfAvailableSpace > 0 || Policy.MaxSizeBytes > 0) {
    स्वतः SpaceInfoOrErr = sys::fs::disk_space(Path);
    जर (!SpaceInfoOrErr) {
      स्वतः EC = SpaceInfoOrErr.getError();
      परत createStringError(EC,
                               "cannot get available disk space for '%s': '%s'",
                               Path.str().c_str(), EC.message().c_str());
    }
    sys::fs::space_info SpaceInfo = SpaceInfoOrErr.get();
    स्वतः AvailableSpace = TotalSize + SpaceInfo.मुक्त;

    जर (Policy.MaxSizePercentageOfAvailableSpace == 0)
      Policy.MaxSizePercentageOfAvailableSpace = 100;
    जर (Policy.MaxSizeBytes == 0)
      Policy.MaxSizeBytes = AvailableSpace;
    स्वतः TotalSizeTarget = std::min<uint64_t>(
        AvailableSpace * Policy.MaxSizePercentageOfAvailableSpace / 100ull,
        Policy.MaxSizeBytes);

    LLVM_DEBUG(dbgs() << "Occupancy: " << ((100 * TotalSize) / AvailableSpace)
                      << "% target is: "
                      << Policy.MaxSizePercentageOfAvailableSpace << "%, "
                      << Policy.MaxSizeBytes << " bytes\n");

    माप_प्रकार ActualSizes = 0;
    साठी (स्थिरांक स्वतः &File : Files)
      जर (File)
        ActualSizes += File->getBufferSize();

    जर (ActualSizes > TotalSizeTarget)
      WithColor::warning()
          << "ThinLTO cache pruning happens since the total size of the cache "
             "files consumed by the current link job ("
          << ActualSizes << "  bytes) exceeds maximum cache size ("
          << TotalSizeTarget
          << " bytes); consider adjusting --thinlto-cache-policy\n";

    // Remove the oldest accessed files first, till we get below the threshold.
    पर्यंत (TotalSize > TotalSizeTarget && FileInfo != FileInfos.end())
      RemoveCacheFile();
  }
  परत सत्य;
}
