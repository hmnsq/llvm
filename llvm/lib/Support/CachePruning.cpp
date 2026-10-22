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

#அந்தர்பாவ "llvm/Support/CachePruning.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/WithColor.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "cache-pruning"

#அந்தர்பாவ <set>
#அந்தர்பாவ <system_error>

using நாமாகாஶ llvm;

நாமாகாஶ {
கட்டமைப்பு FileInfo {
  sys::TimePoint<> Time;
  uint64_t Size;
  std::string Path;

  /// Used to determine which files to prune first. Also used to determine
  /// set membership, so must take into account all fields.
  தருக்க operator<(மாறிலி FileInfo &Other) மாறிலி {
    திரும்பு std::tie(Time, Other.Size, Path) <
           std::tie(Other.Time, Size, Other.Path);
  }
};
} // anonymous namespace

/// Write a new timestamp file with the given path. This is used for the pruning
/// interval option.
நிலையான வெறுமை writeTimestampFile(StringRef TimestampFile) {
  std::error_code EC;
  raw_fd_ostream Out(TimestampFile.str(), EC, sys::fs::OF_None);
}

நிலையான Expected<std::chrono::seconds> parseDuration(StringRef Duration) {
  எனில் (Duration.empty())
    திரும்பு make_error<StringError>("Duration must not be empty",
                                   inconvertibleErrorCode());

  StringRef இலக்கம்_Str = Duration.slice(0, Duration.size()-1);
  uint64_t இலக்கம்;
  எனில் (இலக்கம்_Str.getAsInteger(0, இலக்கம்))
    திரும்பு make_error<StringError>("'" + இலக்கம்_Str + "' not an integer",
                                   inconvertibleErrorCode());

  தேர்வு (Duration.back()) {
  நிலை 's':
    திரும்பு std::chrono::seconds(இலக்கம்);
  நிலை 'm':
    திரும்பு std::chrono::minutes(இலக்கம்);
  நிலை 'h':
    திரும்பு std::chrono::hours(இலக்கம்);
  இயல்பு:
    திரும்பு make_error<StringError>("'" + Duration +
                                       "' must end with one of 's', 'm' or 'h'",
                                   inconvertibleErrorCode());
  }
}

Expected<CachePruningPolicy>
llvm::parseCachePruningPolicy(StringRef PolicyStr) {
  CachePruningPolicy Policy;
  std::pair<StringRef, StringRef> P = {"", PolicyStr};
  போது (!P.second.empty()) {
    P = P.second.split(':');

    StringRef சாவி, மதிப்பு;
    std::tie(சாவி, மதிப்பு) = P.first.split('=');
    எனில் (சாவி == "prune_interval") {
      auto DurationOrErr = parseDuration(மதிப்பு);
      எனில் (!DurationOrErr)
        திரும்பு DurationOrErr.takeError();
      Policy.Interval = *DurationOrErr;
    } இல்லையெனில் எனில் (சாவி == "prune_after") {
      auto DurationOrErr = parseDuration(மதிப்பு);
      எனில் (!DurationOrErr)
        திரும்பு DurationOrErr.takeError();
      Policy.Expiration = *DurationOrErr;
    } இல்லையெனில் எனில் (சாவி == "cache_size") {
      எனில் (மதிப்பு.back() != '%')
        திரும்பு make_error<StringError>("'" + மதிப்பு + "' must be a percentage",
                                       inconvertibleErrorCode());
      StringRef SizeStr = மதிப்பு.drop_back();
      uint64_t Size;
      எனில் (SizeStr.getAsInteger(0, Size))
        திரும்பு make_error<StringError>("'" + SizeStr + "' not an integer",
                                       inconvertibleErrorCode());
      எனில் (Size > 100)
        திரும்பு make_error<StringError>("'" + SizeStr +
                                           "' must be between 0 and 100",
                                       inconvertibleErrorCode());
      Policy.MaxSizePercentageOfAvailableSpace = Size;
    } இல்லையெனில் எனில் (சாவி == "cache_size_bytes") {
      uint64_t Mult = 1;
      தேர்வு (tolower(மதிப்பு.back())) {
      நிலை 'k':
        Mult = 1024;
        மதிப்பு = மதிப்பு.drop_back();
        நிறுத்து;
      நிலை 'm':
        Mult = 1024 * 1024;
        மதிப்பு = மதிப்பு.drop_back();
        நிறுத்து;
      நிலை 'g':
        Mult = 1024 * 1024 * 1024;
        மதிப்பு = மதிப்பு.drop_back();
        நிறுத்து;
      }
      uint64_t Size;
      எனில் (மதிப்பு.getAsInteger(0, Size))
        திரும்பு make_error<StringError>("'" + மதிப்பு + "' not an integer",
                                       inconvertibleErrorCode());
      Policy.MaxSizeBytes = Size * Mult;
    } இல்லையெனில் எனில் (சாவி == "cache_size_files") {
      எனில் (மதிப்பு.getAsInteger(0, Policy.MaxSizeFiles))
        திரும்பு make_error<StringError>("'" + மதிப்பு + "' not an integer",
                                       inconvertibleErrorCode());
    } இல்லையெனில் {
      திரும்பு make_error<StringError>("Unknown key: '" + சாவி + "'",
                                     inconvertibleErrorCode());
    }
  }

  திரும்பு Policy;
}

/// Prune the cache of files that haven't been accessed in a long time.
Expected<தருக்க>
llvm::pruneCache(StringRef Path, CachePruningPolicy Policy,
                 மாறிலி std::vector<std::unique_ptr<MemoryBuffer>> &Files) {
  using நாமாகாஶ std::chrono;

  எனில் (Path.empty())
    திரும்பு பொய்;

  தருக்க isPathDir;
  எனில் (sys::fs::is_directory(Path, isPathDir))
    திரும்பு பொய்;

  எனில் (!isPathDir)
    திரும்பு பொய்;

  Policy.MaxSizePercentageOfAvailableSpace =
      std::min(Policy.MaxSizePercentageOfAvailableSpace, 100u);

  எனில் (Policy.Expiration == seconds(0) &&
      Policy.MaxSizePercentageOfAvailableSpace == 0 &&
      Policy.MaxSizeBytes == 0 && Policy.MaxSizeFiles == 0) {
    LLVM_DEBUG(dbgs() << "No pruning settings set, exit early\n");
    // Nothing will be pruned, early exit
    திரும்பு பொய்;
  }

  // Try to stat() the timestamp file.
  SmallString<128> TimestampFile(Path);
  sys::path::append(TimestampFile, "llvmcache.timestamp");
  sys::fs::file_status FileStatus;
  மாறிலி auto CurrentTime = system_clock::now();
  எனில் (auto EC = sys::fs::status(TimestampFile, FileStatus)) {
    எனில் (EC == errc::no_such_file_or_directory) {
      // If the timestamp file wasn't there, create one now.
      writeTimestampFile(TimestampFile);
    } இல்லையெனில் {
      // Unknown error?
      திரும்பு பொய்;
    }
  } இல்லையெனில் {
    எனில் (!Policy.Interval)
      திரும்பு பொய்;
    எனில் (Policy.Interval != seconds(0)) {
      // Check whether the time stamp is older than our pruning interval.
      // If not, do nothing.
      மாறிலி auto TimeStampModTime = FileStatus.getLastModificationTime();
      auto TimeStampAge = CurrentTime - TimeStampModTime;
      எனில் (TimeStampAge <= *Policy.Interval) {
        LLVM_DEBUG(dbgs() << "Timestamp file too recent ("
                          << duration_cast<seconds>(TimeStampAge).மொத்தம்()
                          << "s old), do not prune.\n");
        திரும்பு பொய்;
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
  க்கு (sys::fs::directory_iterator File(CachePathNative, EC), FileEnd;
       File != FileEnd && !EC; File.increment(EC)) {
    // Ignore filenames not beginning with "llvmcache-" or "Thin-". This
    // includes the timestamp file as well as any files created by the user.
    // This acts as a safeguard against data loss if the user specifies the
    // wrong directory as their cache directory.
    StringRef filename = sys::path::filename(File->path());
    எனில் (!filename.starts_with("llvmcache-") && !filename.starts_with("Thin-"))
      தொடர்;

    // Look at this file. If we can't stat it, there's nothing interesting
    // there.
    ErrorOr<sys::fs::basic_file_status> StatusOrErr = File->status();
    எனில் (!StatusOrErr) {
      LLVM_DEBUG(dbgs() << "Ignore " << File->path() << " (can't stat)\n");
      தொடர்;
    }

    // If the file hasn't been used recently enough, delete it
    மாறிலி auto FileAccessTime = StatusOrErr->getLastAccessedTime();
    auto FileAge = CurrentTime - FileAccessTime;
    எனில் (Policy.Expiration != seconds(0) && FileAge > Policy.Expiration) {
      LLVM_DEBUG(dbgs() << "Remove " << File->path() << " ("
                        << duration_cast<seconds>(FileAge).மொத்தம்()
                        << "s old)\n");
      sys::fs::remove(File->path());
      தொடர்;
    }

    // Leave it here for now, but add it to the list of size-based pruning.
    TotalSize += StatusOrErr->getSize();
    FileInfos.செருகு({FileAccessTime, StatusOrErr->getSize(), File->path()});
  }

  auto FileInfo = FileInfos.begin();
  size_t இலக்கம்_Files = FileInfos.size();

  auto RemoveCacheFile = [&]() {
    // Remove the file.
    sys::fs::remove(FileInfo->Path);
    // Update size
    TotalSize -= FileInfo->Size;
    இலக்கம்_Files--;
    LLVM_DEBUG(dbgs() << " - Remove " << FileInfo->Path << " (size "
                      << FileInfo->Size << "), new occupancy is " << TotalSize
                      << "%\n");
    ++FileInfo;
  };

  // files.size() is greater the number of inputs  by one. However, a timestamp
  // file is created and stored in the cache directory if --thinlto-cache-policy
  // option is used. Therefore, files.size() is used as ActualNums.
  மாறிலி size_t ActualNums = Files.size();
  எனில் (Policy.MaxSizeFiles && ActualNums > Policy.MaxSizeFiles)
    WithColor::warning()
        << "ThinLTO cache pruning happens since the number of created files ("
        << ActualNums << ") exceeds the maximum number of files ("
        << Policy.MaxSizeFiles
        << "); consider adjusting --thinlto-cache-policy\n";

  // Prune for number of files.
  எனில் (Policy.MaxSizeFiles)
    போது (இலக்கம்_Files > Policy.MaxSizeFiles)
      RemoveCacheFile();

  // Prune for size now if needed
  எனில் (Policy.MaxSizePercentageOfAvailableSpace > 0 || Policy.MaxSizeBytes > 0) {
    auto SpaceInfoOrErr = sys::fs::disk_space(Path);
    எனில் (!SpaceInfoOrErr) {
      auto EC = SpaceInfoOrErr.getError();
      திரும்பு createStringError(EC,
                               "cannot get available disk space for '%s': '%s'",
                               Path.str().c_str(), EC.message().c_str());
    }
    sys::fs::space_info SpaceInfo = SpaceInfoOrErr.get();
    auto AvailableSpace = TotalSize + SpaceInfo.விடுவி;

    எனில் (Policy.MaxSizePercentageOfAvailableSpace == 0)
      Policy.MaxSizePercentageOfAvailableSpace = 100;
    எனில் (Policy.MaxSizeBytes == 0)
      Policy.MaxSizeBytes = AvailableSpace;
    auto TotalSizeTarget = std::min<uint64_t>(
        AvailableSpace * Policy.MaxSizePercentageOfAvailableSpace / 100ull,
        Policy.MaxSizeBytes);

    LLVM_DEBUG(dbgs() << "Occupancy: " << ((100 * TotalSize) / AvailableSpace)
                      << "% target is: "
                      << Policy.MaxSizePercentageOfAvailableSpace << "%, "
                      << Policy.MaxSizeBytes << " bytes\n");

    size_t ActualSizes = 0;
    க்கு (மாறிலி auto &File : Files)
      எனில் (File)
        ActualSizes += File->getBufferSize();

    எனில் (ActualSizes > TotalSizeTarget)
      WithColor::warning()
          << "ThinLTO cache pruning happens since the total size of the cache "
             "files consumed by the current link job ("
          << ActualSizes << "  bytes) exceeds maximum cache size ("
          << TotalSizeTarget
          << " bytes); consider adjusting --thinlto-cache-policy\n";

    // Remove the oldest accessed files first, till we get below the threshold.
    போது (TotalSize > TotalSizeTarget && FileInfo != FileInfos.end())
      RemoveCacheFile();
  }
  திரும்பு உண்மை;
}
