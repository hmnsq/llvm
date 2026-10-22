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

#ضمن "llvm/Support/CachePruning.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/WithColor.h"
#ضمن "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "cache-pruning"

#ضمن <set>
#ضمن <system_error>

using نطاق llvm;

نطاق {
بنية FileInfo {
  sys::TimePoint<> Time;
  uint64_t Size;
  std::string Path;

  /// Used to determine which files to prune first. Also used to determine
  /// set membership, so must take into account all fields.
  منطقي operator<(ثابت FileInfo &Other) ثابت {
    החזר std::tie(Time, Other.Size, Path) <
           std::tie(Other.Time, Size, Other.Path);
  }
};
} // anonymous namespace

/// Write a new timestamp file with the given path. This is used for the pruning
/// interval option.
ساكن فراغ writeTimestampFile(StringRef TimestampFile) {
  std::error_code EC;
  raw_fd_ostream Out(TimestampFile.str(), EC, sys::fs::OF_None);
}

ساكن Expected<std::chrono::seconds> parseDuration(StringRef Duration) {
  אם (Duration.empty())
    החזר make_error<StringError>("Duration must not be empty",
                                   inconvertibleErrorCode());

  StringRef NumStr = Duration.slice(0, Duration.size()-1);
  uint64_t Num;
  אם (NumStr.getAsInteger(0, Num))
    החזר make_error<StringError>("'" + NumStr + "' not an integer",
                                   inconvertibleErrorCode());

  بدل (Duration.back()) {
  حالة 's':
    החזר std::chrono::seconds(Num);
  حالة 'm':
    החזר std::chrono::minutes(Num);
  حالة 'h':
    החזר std::chrono::hours(Num);
  افتراضي:
    החזר make_error<StringError>("'" + Duration +
                                       "' must end with one of 's', 'm' or 'h'",
                                   inconvertibleErrorCode());
  }
}

Expected<CachePruningPolicy>
llvm::parseCachePruningPolicy(StringRef PolicyStr) {
  CachePruningPolicy Policy;
  std::pair<StringRef, StringRef> P = {"", PolicyStr};
  כלעוד (!P.second.empty()) {
    P = P.second.split(':');

    StringRef Key, Value;
    std::tie(Key, Value) = P.first.split('=');
    אם (Key == "prune_interval") {
      auto DurationOrErr = parseDuration(Value);
      אם (!DurationOrErr)
        החזר DurationOrErr.takeError();
      Policy.Interval = *DurationOrErr;
    } אחרת אם (Key == "prune_after") {
      auto DurationOrErr = parseDuration(Value);
      אם (!DurationOrErr)
        החזר DurationOrErr.takeError();
      Policy.Expiration = *DurationOrErr;
    } אחרת אם (Key == "cache_size") {
      אם (Value.back() != '%')
        החזר make_error<StringError>("'" + Value + "' must be a percentage",
                                       inconvertibleErrorCode());
      StringRef SizeStr = Value.drop_back();
      uint64_t Size;
      אם (SizeStr.getAsInteger(0, Size))
        החזר make_error<StringError>("'" + SizeStr + "' not an integer",
                                       inconvertibleErrorCode());
      אם (Size > 100)
        החזר make_error<StringError>("'" + SizeStr +
                                           "' must be between 0 and 100",
                                       inconvertibleErrorCode());
      Policy.MaxSizePercentageOfAvailableSpace = Size;
    } אחרת אם (Key == "cache_size_bytes") {
      uint64_t Mult = 1;
      بدل (tolower(Value.back())) {
      حالة 'k':
        Mult = 1024;
        Value = Value.drop_back();
        اكسر;
      حالة 'm':
        Mult = 1024 * 1024;
        Value = Value.drop_back();
        اكسر;
      حالة 'g':
        Mult = 1024 * 1024 * 1024;
        Value = Value.drop_back();
        اكسر;
      }
      uint64_t Size;
      אם (Value.getAsInteger(0, Size))
        החזר make_error<StringError>("'" + Value + "' not an integer",
                                       inconvertibleErrorCode());
      Policy.MaxSizeBytes = Size * Mult;
    } אחרת אם (Key == "cache_size_files") {
      אם (Value.getAsInteger(0, Policy.MaxSizeFiles))
        החזר make_error<StringError>("'" + Value + "' not an integer",
                                       inconvertibleErrorCode());
    } אחרת {
      החזר make_error<StringError>("Unknown key: '" + Key + "'",
                                     inconvertibleErrorCode());
    }
  }

  החזר Policy;
}

/// Prune the cache of files that haven't been accessed in a long time.
Expected<منطقي>
llvm::pruneCache(StringRef Path, CachePruningPolicy Policy,
                 ثابت std::vector<std::unique_ptr<MemoryBuffer>> &Files) {
  using نطاق std::chrono;

  אם (Path.empty())
    החזר שקר;

  منطقي isPathDir;
  אם (sys::fs::is_directory(Path, isPathDir))
    החזר שקר;

  אם (!isPathDir)
    החזר שקר;

  Policy.MaxSizePercentageOfAvailableSpace =
      std::min(Policy.MaxSizePercentageOfAvailableSpace, 100u);

  אם (Policy.Expiration == seconds(0) &&
      Policy.MaxSizePercentageOfAvailableSpace == 0 &&
      Policy.MaxSizeBytes == 0 && Policy.MaxSizeFiles == 0) {
    LLVM_DEBUG(dbgs() << "No pruning settings set, exit early\n");
    // Nothing will be pruned, early exit
    החזר שקר;
  }

  // Try to stat() the timestamp file.
  SmallString<128> TimestampFile(Path);
  sys::path::append(TimestampFile, "llvmcache.timestamp");
  sys::fs::file_status FileStatus;
  ثابت auto CurrentTime = system_clock::now();
  אם (auto EC = sys::fs::status(TimestampFile, FileStatus)) {
    אם (EC == errc::no_such_file_or_directory) {
      // If the timestamp file wasn't there, create one now.
      writeTimestampFile(TimestampFile);
    } אחרת {
      // Unknown error?
      החזר שקר;
    }
  } אחרת {
    אם (!Policy.Interval)
      החזר שקר;
    אם (Policy.Interval != seconds(0)) {
      // Check whether the time stamp is older than our pruning interval.
      // If not, do nothing.
      ثابت auto TimeStampModTime = FileStatus.getLastModificationTime();
      auto TimeStampAge = CurrentTime - TimeStampModTime;
      אם (TimeStampAge <= *Policy.Interval) {
        LLVM_DEBUG(dbgs() << "Timestamp file too recent ("
                          << duration_cast<seconds>(TimeStampAge).count()
                          << "s old), do not prune.\n");
        החזר שקר;
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
  עבור (sys::fs::directory_iterator File(CachePathNative, EC), FileEnd;
       File != FileEnd && !EC; File.increment(EC)) {
    // Ignore filenames not beginning with "llvmcache-" or "Thin-". This
    // includes the timestamp file as well as any files created by the user.
    // This acts as a safeguard against data loss if the user specifies the
    // wrong directory as their cache directory.
    StringRef filename = sys::path::filename(File->path());
    אם (!filename.starts_with("llvmcache-") && !filename.starts_with("Thin-"))
      استمر;

    // Look at this file. If we can't stat it, there's nothing interesting
    // there.
    ErrorOr<sys::fs::basic_file_status> StatusOrErr = File->status();
    אם (!StatusOrErr) {
      LLVM_DEBUG(dbgs() << "Ignore " << File->path() << " (can't stat)\n");
      استمر;
    }

    // If the file hasn't been used recently enough, delete it
    ثابت auto FileAccessTime = StatusOrErr->getLastAccessedTime();
    auto FileAge = CurrentTime - FileAccessTime;
    אם (Policy.Expiration != seconds(0) && FileAge > Policy.Expiration) {
      LLVM_DEBUG(dbgs() << "Remove " << File->path() << " ("
                        << duration_cast<seconds>(FileAge).count()
                        << "s old)\n");
      sys::fs::remove(File->path());
      استمر;
    }

    // Leave it here for now, but add it to the list of size-based pruning.
    TotalSize += StatusOrErr->getSize();
    FileInfos.insert({FileAccessTime, StatusOrErr->getSize(), File->path()});
  }

  auto FileInfo = FileInfos.begin();
  size_t NumFiles = FileInfos.size();

  auto RemoveCacheFile = [&]() {
    // Remove the file.
    sys::fs::remove(FileInfo->Path);
    // Update size
    TotalSize -= FileInfo->Size;
    NumFiles--;
    LLVM_DEBUG(dbgs() << " - Remove " << FileInfo->Path << " (size "
                      << FileInfo->Size << "), new occupancy is " << TotalSize
                      << "%\n");
    ++FileInfo;
  };

  // files.size() is greater the number of inputs  by one. However, a timestamp
  // file is created and stored in the cache directory if --thinlto-cache-policy
  // option is used. Therefore, files.size() is used as ActualNums.
  ثابت size_t ActualNums = Files.size();
  אם (Policy.MaxSizeFiles && ActualNums > Policy.MaxSizeFiles)
    WithColor::warning()
        << "ThinLTO cache pruning happens since the number of created files ("
        << ActualNums << ") exceeds the maximum number of files ("
        << Policy.MaxSizeFiles
        << "); consider adjusting --thinlto-cache-policy\n";

  // Prune for number of files.
  אם (Policy.MaxSizeFiles)
    כלעוד (NumFiles > Policy.MaxSizeFiles)
      RemoveCacheFile();

  // Prune for size now if needed
  אם (Policy.MaxSizePercentageOfAvailableSpace > 0 || Policy.MaxSizeBytes > 0) {
    auto SpaceInfoOrErr = sys::fs::disk_space(Path);
    אם (!SpaceInfoOrErr) {
      auto EC = SpaceInfoOrErr.getError();
      החזר createStringError(EC,
                               "cannot get available disk space for '%s': '%s'",
                               Path.str().c_str(), EC.message().c_str());
    }
    sys::fs::space_info SpaceInfo = SpaceInfoOrErr.get();
    auto AvailableSpace = TotalSize + SpaceInfo.حرر;

    אם (Policy.MaxSizePercentageOfAvailableSpace == 0)
      Policy.MaxSizePercentageOfAvailableSpace = 100;
    אם (Policy.MaxSizeBytes == 0)
      Policy.MaxSizeBytes = AvailableSpace;
    auto TotalSizeTarget = std::min<uint64_t>(
        AvailableSpace * Policy.MaxSizePercentageOfAvailableSpace / 100ull,
        Policy.MaxSizeBytes);

    LLVM_DEBUG(dbgs() << "Occupancy: " << ((100 * TotalSize) / AvailableSpace)
                      << "% target is: "
                      << Policy.MaxSizePercentageOfAvailableSpace << "%, "
                      << Policy.MaxSizeBytes << " bytes\n");

    size_t ActualSizes = 0;
    עבור (ثابت auto &File : Files)
      אם (File)
        ActualSizes += File->getBufferSize();

    אם (ActualSizes > TotalSizeTarget)
      WithColor::warning()
          << "ThinLTO cache pruning happens since the total size of the cache "
             "files consumed by the current link job ("
          << ActualSizes << "  bytes) exceeds maximum cache size ("
          << TotalSizeTarget
          << " bytes); consider adjusting --thinlto-cache-policy\n";

    // Remove the oldest accessed files first, till we get below the threshold.
    כלעוד (TotalSize > TotalSizeTarget && FileInfo != FileInfos.end())
      RemoveCacheFile();
  }
  החזר אמת;
}
