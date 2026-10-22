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
  तार्किक चालक<(अचर FileInfo &Other) अचर {
    फर्काऊ std::tie(Time, Other.Size, Path) <
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
  यदि (Duration.empty())
    फर्काऊ make_error<StringError>("Duration must not be empty",
                                   inconvertibleErrorCode());

  StringRef अङ्क_Str = Duration.slice(0, Duration.size()-1);
  uint64_t अङ्क;
  यदि (अङ्क_Str.getAsInteger(0, अङ्क))
    फर्काऊ make_error<StringError>("'" + अङ्क_Str + "' not an integer",
                                   inconvertibleErrorCode());

  विकल्प (Duration.back()) {
  पक्ष 's':
    फर्काऊ std::chrono::seconds(अङ्क);
  पक्ष 'm':
    फर्काऊ std::chrono::minutes(अङ्क);
  पक्ष 'h':
    फर्काऊ std::chrono::hours(अङ्क);
  स्वभाव:
    फर्काऊ make_error<StringError>("'" + Duration +
                                       "' must end with one of 's', 'm' or 'h'",
                                   inconvertibleErrorCode());
  }
}

Expected<CachePruningPolicy>
llvm::parseCachePruningPolicy(StringRef PolicyStr) {
  CachePruningPolicy Policy;
  std::pair<StringRef, StringRef> P = {"", PolicyStr};
  जबसम्म (!P.second.empty()) {
    P = P.second.split(':');

    StringRef साँचो, मान;
    std::tie(साँचो, मान) = P.first.split('=');
    यदि (साँचो == "prune_interval") {
      स्वतः DurationOrErr = parseDuration(मान);
      यदि (!DurationOrErr)
        फर्काऊ DurationOrErr.takeError();
      Policy.Interval = *DurationOrErr;
    } अन्यथा यदि (साँचो == "prune_after") {
      स्वतः DurationOrErr = parseDuration(मान);
      यदि (!DurationOrErr)
        फर्काऊ DurationOrErr.takeError();
      Policy.Expiration = *DurationOrErr;
    } अन्यथा यदि (साँचो == "cache_size") {
      यदि (मान.back() != '%')
        फर्काऊ make_error<StringError>("'" + मान + "' must be a percentage",
                                       inconvertibleErrorCode());
      StringRef SizeStr = मान.drop_back();
      uint64_t Size;
      यदि (SizeStr.getAsInteger(0, Size))
        फर्काऊ make_error<StringError>("'" + SizeStr + "' not an integer",
                                       inconvertibleErrorCode());
      यदि (Size > 100)
        फर्काऊ make_error<StringError>("'" + SizeStr +
                                           "' must be between 0 and 100",
                                       inconvertibleErrorCode());
      Policy.MaxSizePercentageOfAvailableSpace = Size;
    } अन्यथा यदि (साँचो == "cache_size_bytes") {
      uint64_t Mult = 1;
      विकल्प (छोटे(मान.back())) {
      पक्ष 'k':
        Mult = 1024;
        मान = मान.drop_back();
        रोक;
      पक्ष 'm':
        Mult = 1024 * 1024;
        मान = मान.drop_back();
        रोक;
      पक्ष 'g':
        Mult = 1024 * 1024 * 1024;
        मान = मान.drop_back();
        रोक;
      }
      uint64_t Size;
      यदि (मान.getAsInteger(0, Size))
        फर्काऊ make_error<StringError>("'" + मान + "' not an integer",
                                       inconvertibleErrorCode());
      Policy.MaxSizeBytes = Size * Mult;
    } अन्यथा यदि (साँचो == "cache_size_files") {
      यदि (मान.getAsInteger(0, Policy.MaxSizeFiles))
        फर्काऊ make_error<StringError>("'" + मान + "' not an integer",
                                       inconvertibleErrorCode());
    } अन्यथा {
      फर्काऊ make_error<StringError>("Unknown key: '" + साँचो + "'",
                                     inconvertibleErrorCode());
    }
  }

  फर्काऊ Policy;
}

/// Prune the cache of files that haven't been accessed in a long time.
Expected<तार्किक>
llvm::pruneCache(StringRef Path, CachePruningPolicy Policy,
                 अचर std::vector<std::unique_ptr<MemoryBuffer>> &Files) {
  using नामाकाश std::chrono;

  यदि (Path.empty())
    फर्काऊ असत्य;

  तार्किक isPathDir;
  यदि (sys::fs::is_directory(Path, isPathDir))
    फर्काऊ असत्य;

  यदि (!isPathDir)
    फर्काऊ असत्य;

  Policy.MaxSizePercentageOfAvailableSpace =
      std::min(Policy.MaxSizePercentageOfAvailableSpace, 100u);

  यदि (Policy.Expiration == seconds(0) &&
      Policy.MaxSizePercentageOfAvailableSpace == 0 &&
      Policy.MaxSizeBytes == 0 && Policy.MaxSizeFiles == 0) {
    LLVM_DEBUG(dbgs() << "No pruning settings set, exit early\n");
    // Nothing will be pruned, early exit
    फर्काऊ असत्य;
  }

  // Try to stat() the timestamp file.
  SmallString<128> TimestampFile(Path);
  sys::path::append(TimestampFile, "llvmcache.timestamp");
  sys::fs::file_status FileStatus;
  अचर स्वतः CurrentTime = system_clock::now();
  यदि (स्वतः EC = sys::fs::status(TimestampFile, FileStatus)) {
    यदि (EC == errc::no_such_file_or_directory) {
      // If the timestamp file wasn't there, create one now.
      writeTimestampFile(TimestampFile);
    } अन्यथा {
      // Unknown error?
      फर्काऊ असत्य;
    }
  } अन्यथा {
    यदि (!Policy.Interval)
      फर्काऊ असत्य;
    यदि (Policy.Interval != seconds(0)) {
      // Check whether the time stamp is older than our pruning interval.
      // If not, do nothing.
      अचर स्वतः TimeStampModTime = FileStatus.getLastModificationTime();
      स्वतः TimeStampAge = CurrentTime - TimeStampModTime;
      यदि (TimeStampAge <= *Policy.Interval) {
        LLVM_DEBUG(dbgs() << "Timestamp file too recent ("
                          << duration_cast<seconds>(TimeStampAge).गन्ती()
                          << "s old), do not prune.\n");
        फर्काऊ असत्य;
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
  लागि (sys::fs::directory_iterator File(CachePathNative, EC), FileEnd;
       File != FileEnd && !EC; File.increment(EC)) {
    // Ignore filenames not beginning with "llvmcache-" or "Thin-". This
    // includes the timestamp file as well as any files created by the user.
    // This acts as a safeguard against data loss if the user specifies the
    // wrong directory as their cache directory.
    StringRef filename = sys::path::filename(File->path());
    यदि (!filename.starts_with("llvmcache-") && !filename.starts_with("Thin-"))
      जारी;

    // Look at this file. If we can't stat it, there's nothing interesting
    // there.
    ErrorOr<sys::fs::basic_file_status> StatusOrErr = File->status();
    यदि (!StatusOrErr) {
      LLVM_DEBUG(dbgs() << "Ignore " << File->path() << " (can't stat)\n");
      जारी;
    }

    // If the file hasn't been used recently enough, delete it
    अचर स्वतः FileAccessTime = StatusOrErr->getLastAccessedTime();
    स्वतः FileAge = CurrentTime - FileAccessTime;
    यदि (Policy.Expiration != seconds(0) && FileAge > Policy.Expiration) {
      LLVM_DEBUG(dbgs() << "Remove " << File->path() << " ("
                        << duration_cast<seconds>(FileAge).गन्ती()
                        << "s old)\n");
      sys::fs::हटाओ(File->path());
      जारी;
    }

    // Leave it here for now, but add it to the list of size-based pruning.
    TotalSize += StatusOrErr->getSize();
    FileInfos.घुसाउ({FileAccessTime, StatusOrErr->getSize(), File->path()});
  }

  स्वतः FileInfo = FileInfos.begin();
  माप_प्रकार अङ्क_Files = FileInfos.size();

  स्वतः RemoveCacheFile = [&]() {
    // Remove the file.
    sys::fs::हटाओ(FileInfo->Path);
    // Update size
    TotalSize -= FileInfo->Size;
    अङ्क_Files--;
    LLVM_DEBUG(dbgs() << " - Remove " << FileInfo->Path << " (size "
                      << FileInfo->Size << "), new occupancy is " << TotalSize
                      << "%\n");
    ++FileInfo;
  };

  // files.size() is greater the number of inputs  by one. However, a timestamp
  // file is created and stored in the cache directory if --thinlto-cache-policy
  // option is used. Therefore, files.size() is used as ActualNums.
  अचर माप_प्रकार ActualNums = Files.size();
  यदि (Policy.MaxSizeFiles && ActualNums > Policy.MaxSizeFiles)
    WithColor::warning()
        << "ThinLTO cache pruning happens since the number of created files ("
        << ActualNums << ") exceeds the maximum number of files ("
        << Policy.MaxSizeFiles
        << "); consider adjusting --thinlto-cache-policy\n";

  // Prune for number of files.
  यदि (Policy.MaxSizeFiles)
    जबसम्म (अङ्क_Files > Policy.MaxSizeFiles)
      RemoveCacheFile();

  // Prune for size now if needed
  यदि (Policy.MaxSizePercentageOfAvailableSpace > 0 || Policy.MaxSizeBytes > 0) {
    स्वतः SpaceInfoOrErr = sys::fs::disk_space(Path);
    यदि (!SpaceInfoOrErr) {
      स्वतः EC = SpaceInfoOrErr.getError();
      फर्काऊ createStringError(EC,
                               "cannot get available disk space for '%s': '%s'",
                               Path.str().c_str(), EC.message().c_str());
    }
    sys::fs::space_info SpaceInfo = SpaceInfoOrErr.get();
    स्वतः AvailableSpace = TotalSize + SpaceInfo.मुक्त;

    यदि (Policy.MaxSizePercentageOfAvailableSpace == 0)
      Policy.MaxSizePercentageOfAvailableSpace = 100;
    यदि (Policy.MaxSizeBytes == 0)
      Policy.MaxSizeBytes = AvailableSpace;
    स्वतः TotalSizeTarget = std::min<uint64_t>(
        AvailableSpace * Policy.MaxSizePercentageOfAvailableSpace / 100ull,
        Policy.MaxSizeBytes);

    LLVM_DEBUG(dbgs() << "Occupancy: " << ((100 * TotalSize) / AvailableSpace)
                      << "% target is: "
                      << Policy.MaxSizePercentageOfAvailableSpace << "%, "
                      << Policy.MaxSizeBytes << " bytes\n");

    माप_प्रकार ActualSizes = 0;
    लागि (अचर स्वतः &File : Files)
      यदि (File)
        ActualSizes += File->getBufferSize();

    यदि (ActualSizes > TotalSizeTarget)
      WithColor::warning()
          << "ThinLTO cache pruning happens since the total size of the cache "
             "files consumed by the current link job ("
          << ActualSizes << "  bytes) exceeds maximum cache size ("
          << TotalSizeTarget
          << " bytes); consider adjusting --thinlto-cache-policy\n";

    // Remove the oldest accessed files first, till we get below the threshold.
    जबसम्म (TotalSize > TotalSizeTarget && FileInfo != FileInfos.end())
      RemoveCacheFile();
  }
  फर्काऊ सत्य;
}
