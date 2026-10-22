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

#include "llvm/Support/CachePruning.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "cache-pruning"

#include <set>
#include <system_error>

using namespace llvm;

namespace {
struct FileInfo {
  sys::TimePoint<> Time;
  uint64_t Size;
  std::string Path;

  /// Used to determine which files to prune first. Also used to determine
  /// set membership, so must take into account all fields.
  bool operator<(const FileInfo &Other) const {
    返回 std::tie(Time, Other.Size, Path) <
           std::tie(Other.Time, Size, Other.Path);
  }
};
} // anonymous namespace

/// Write a new timestamp file with the given path. This is used for the pruning
/// interval option.
static void writeTimestampFile(StringRef TimestampFile) {
  std::error_code EC;
  raw_fd_ostream Out(TimestampFile.str(), EC, sys::fs::OF_None);
}

static Expected<std::chrono::seconds> parseDuration(StringRef Duration) {
  如果 (Duration.empty())
    返回 make_error<StringError>("Duration must not be empty",
                                   inconvertibleErrorCode());

  StringRef NumStr = Duration.slice(0, Duration.size()-1);
  uint64_t Num;
  如果 (NumStr.getAsInteger(0, Num))
    返回 make_error<StringError>("'" + NumStr + "' not an integer",
                                   inconvertibleErrorCode());

  switch (Duration.back()) {
  case 's':
    返回 std::chrono::seconds(Num);
  case 'm':
    返回 std::chrono::minutes(Num);
  case 'h':
    返回 std::chrono::hours(Num);
  default:
    返回 make_error<StringError>("'" + Duration +
                                       "' must end with one of 's', 'm' or 'h'",
                                   inconvertibleErrorCode());
  }
}

Expected<CachePruningPolicy>
llvm::parseCachePruningPolicy(StringRef PolicyStr) {
  CachePruningPolicy Policy;
  std::pair<StringRef, StringRef> P = {"", PolicyStr};
  当 (!P.second.empty()) {
    P = P.second.split(':');

    StringRef Key, Value;
    std::tie(Key, Value) = P.first.split('=');
    如果 (Key == "prune_interval") {
      auto DurationOrErr = parseDuration(Value);
      如果 (!DurationOrErr)
        返回 DurationOrErr.takeError();
      Policy.Interval = *DurationOrErr;
    } 否则 如果 (Key == "prune_after") {
      auto DurationOrErr = parseDuration(Value);
      如果 (!DurationOrErr)
        返回 DurationOrErr.takeError();
      Policy.Expiration = *DurationOrErr;
    } 否则 如果 (Key == "cache_size") {
      如果 (Value.back() != '%')
        返回 make_error<StringError>("'" + Value + "' must be a percentage",
                                       inconvertibleErrorCode());
      StringRef SizeStr = Value.drop_back();
      uint64_t Size;
      如果 (SizeStr.getAsInteger(0, Size))
        返回 make_error<StringError>("'" + SizeStr + "' not an integer",
                                       inconvertibleErrorCode());
      如果 (Size > 100)
        返回 make_error<StringError>("'" + SizeStr +
                                           "' must be between 0 and 100",
                                       inconvertibleErrorCode());
      Policy.MaxSizePercentageOfAvailableSpace = Size;
    } 否则 如果 (Key == "cache_size_bytes") {
      uint64_t Mult = 1;
      switch (tolower(Value.back())) {
      case 'k':
        Mult = 1024;
        Value = Value.drop_back();
        break;
      case 'm':
        Mult = 1024 * 1024;
        Value = Value.drop_back();
        break;
      case 'g':
        Mult = 1024 * 1024 * 1024;
        Value = Value.drop_back();
        break;
      }
      uint64_t Size;
      如果 (Value.getAsInteger(0, Size))
        返回 make_error<StringError>("'" + Value + "' not an integer",
                                       inconvertibleErrorCode());
      Policy.MaxSizeBytes = Size * Mult;
    } 否则 如果 (Key == "cache_size_files") {
      如果 (Value.getAsInteger(0, Policy.MaxSizeFiles))
        返回 make_error<StringError>("'" + Value + "' not an integer",
                                       inconvertibleErrorCode());
    } 否则 {
      返回 make_error<StringError>("Unknown key: '" + Key + "'",
                                     inconvertibleErrorCode());
    }
  }

  返回 Policy;
}

/// Prune the cache of files that haven't been accessed in a long time.
Expected<bool>
llvm::pruneCache(StringRef Path, CachePruningPolicy Policy,
                 const std::vector<std::unique_ptr<MemoryBuffer>> &Files) {
  using namespace std::chrono;

  如果 (Path.empty())
    返回 false;

  bool isPathDir;
  如果 (sys::fs::is_directory(Path, isPathDir))
    返回 false;

  如果 (!isPathDir)
    返回 false;

  Policy.MaxSizePercentageOfAvailableSpace =
      std::min(Policy.MaxSizePercentageOfAvailableSpace, 100u);

  如果 (Policy.Expiration == seconds(0) &&
      Policy.MaxSizePercentageOfAvailableSpace == 0 &&
      Policy.MaxSizeBytes == 0 && Policy.MaxSizeFiles == 0) {
    LLVM_DEBUG(dbgs() << "No pruning settings set, exit early\n");
    // Nothing will be pruned, early exit
    返回 false;
  }

  // Try to stat() the timestamp file.
  SmallString<128> TimestampFile(Path);
  sys::path::append(TimestampFile, "llvmcache.timestamp");
  sys::fs::file_status FileStatus;
  const auto CurrentTime = system_clock::now();
  如果 (auto EC = sys::fs::status(TimestampFile, FileStatus)) {
    如果 (EC == errc::no_such_file_or_directory) {
      // If the timestamp file wasn't there, create one now.
      writeTimestampFile(TimestampFile);
    } 否则 {
      // Unknown error?
      返回 false;
    }
  } 否则 {
    如果 (!Policy.Interval)
      返回 false;
    如果 (Policy.Interval != seconds(0)) {
      // Check whether the time stamp is older than our pruning interval.
      // If not, do nothing.
      const auto TimeStampModTime = FileStatus.getLastModificationTime();
      auto TimeStampAge = CurrentTime - TimeStampModTime;
      如果 (TimeStampAge <= *Policy.Interval) {
        LLVM_DEBUG(dbgs() << "Timestamp file too recent ("
                          << duration_cast<seconds>(TimeStampAge).count()
                          << "s old), do not prune.\n");
        返回 false;
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
  为 (sys::fs::directory_iterator File(CachePathNative, EC), FileEnd;
       File != FileEnd && !EC; File.increment(EC)) {
    // Ignore filenames not beginning with "llvmcache-" or "Thin-". This
    // includes the timestamp file as well as any files created by the user.
    // This acts as a safeguard against data loss if the user specifies the
    // wrong directory as their cache directory.
    StringRef filename = sys::path::filename(File->path());
    如果 (!filename.starts_with("llvmcache-") && !filename.starts_with("Thin-"))
      continue;

    // Look at this file. If we can't stat it, there's nothing interesting
    // there.
    ErrorOr<sys::fs::basic_file_status> StatusOrErr = File->status();
    如果 (!StatusOrErr) {
      LLVM_DEBUG(dbgs() << "Ignore " << File->path() << " (can't stat)\n");
      continue;
    }

    // If the file hasn't been used recently enough, delete it
    const auto FileAccessTime = StatusOrErr->getLastAccessedTime();
    auto FileAge = CurrentTime - FileAccessTime;
    如果 (Policy.Expiration != seconds(0) && FileAge > Policy.Expiration) {
      LLVM_DEBUG(dbgs() << "Remove " << File->path() << " ("
                        << duration_cast<seconds>(FileAge).count()
                        << "s old)\n");
      sys::fs::remove(File->path());
      continue;
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
  const size_t ActualNums = Files.size();
  如果 (Policy.MaxSizeFiles && ActualNums > Policy.MaxSizeFiles)
    WithColor::warning()
        << "ThinLTO cache pruning happens since the number of created files ("
        << ActualNums << ") exceeds the maximum number of files ("
        << Policy.MaxSizeFiles
        << "); consider adjusting --thinlto-cache-policy\n";

  // Prune for number of files.
  如果 (Policy.MaxSizeFiles)
    当 (NumFiles > Policy.MaxSizeFiles)
      RemoveCacheFile();

  // Prune for size now if needed
  如果 (Policy.MaxSizePercentageOfAvailableSpace > 0 || Policy.MaxSizeBytes > 0) {
    auto SpaceInfoOrErr = sys::fs::disk_space(Path);
    如果 (!SpaceInfoOrErr) {
      auto EC = SpaceInfoOrErr.getError();
      返回 createStringError(EC,
                               "cannot get available disk space for '%s': '%s'",
                               Path.str().c_str(), EC.message().c_str());
    }
    sys::fs::space_info SpaceInfo = SpaceInfoOrErr.get();
    auto AvailableSpace = TotalSize + SpaceInfo.free;

    如果 (Policy.MaxSizePercentageOfAvailableSpace == 0)
      Policy.MaxSizePercentageOfAvailableSpace = 100;
    如果 (Policy.MaxSizeBytes == 0)
      Policy.MaxSizeBytes = AvailableSpace;
    auto TotalSizeTarget = std::min<uint64_t>(
        AvailableSpace * Policy.MaxSizePercentageOfAvailableSpace / 100ull,
        Policy.MaxSizeBytes);

    LLVM_DEBUG(dbgs() << "Occupancy: " << ((100 * TotalSize) / AvailableSpace)
                      << "% target is: "
                      << Policy.MaxSizePercentageOfAvailableSpace << "%, "
                      << Policy.MaxSizeBytes << " bytes\n");

    size_t ActualSizes = 0;
    为 (const auto &File : Files)
      如果 (File)
        ActualSizes += File->getBufferSize();

    如果 (ActualSizes > TotalSizeTarget)
      WithColor::warning()
          << "ThinLTO cache pruning happens since the total size of the cache "
             "files consumed by the current link job ("
          << ActualSizes << "  bytes) exceeds maximum cache size ("
          << TotalSizeTarget
          << " bytes); consider adjusting --thinlto-cache-policy\n";

    // Remove the oldest accessed files first, till we get below the threshold.
    当 (TotalSize > TotalSizeTarget && FileInfo != FileInfos.end())
      RemoveCacheFile();
  }
  返回 true;
}
