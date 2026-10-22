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

#અન્તર્ભાવ "llvm/Support/CachePruning.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/WithColor.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "cache-pruning"

#અન્તર્ભાવ <set>
#અન્તર્ભાવ <system_error>

using નામાકાશ llvm;

નામાકાશ {
સંરચના FileInfo {
  sys::TimePoint<> Time;
  uint64_t Size;
  std::string Path;

  /// Used to determine which files to prune first. Also used to determine
  /// set membership, so must take into account all fields.
  તાર્કિક operator<(અચલ FileInfo &Other) અચલ {
    પાછા std::tie(Time, Other.Size, Path) <
           std::tie(Other.Time, Size, Other.Path);
  }
};
} // anonymous namespace

/// Write a new timestamp file with the given path. This is used for the pruning
/// interval option.
સ્થિર શૂન્ય writeTimestampFile(StringRef TimestampFile) {
  std::error_code EC;
  raw_fd_ostream Out(TimestampFile.str(), EC, sys::fs::OF_None);
}

સ્થિર Expected<std::chrono::seconds> parseDuration(StringRef Duration) {
  જો (Duration.empty())
    પાછા make_error<StringError>("Duration must not be empty",
                                   inconvertibleErrorCode());

  StringRef અંક_Str = Duration.slice(0, Duration.size()-1);
  uint64_t અંક;
  જો (અંક_Str.getAsInteger(0, અંક))
    પાછા make_error<StringError>("'" + અંક_Str + "' not an integer",
                                   inconvertibleErrorCode());

  પસંદગી (Duration.back()) {
  સ્થિતિ 's':
    પાછા std::chrono::seconds(અંક);
  સ્થિતિ 'm':
    પાછા std::chrono::minutes(અંક);
  સ્થિતિ 'h':
    પાછા std::chrono::hours(અંક);
  મૂળભૂત:
    પાછા make_error<StringError>("'" + Duration +
                                       "' must end with one of 's', 'm' or 'h'",
                                   inconvertibleErrorCode());
  }
}

Expected<CachePruningPolicy>
llvm::parseCachePruningPolicy(StringRef PolicyStr) {
  CachePruningPolicy Policy;
  std::pair<StringRef, StringRef> P = {"", PolicyStr};
  જ્યારે (!P.second.empty()) {
    P = P.second.split(':');

    StringRef ચાવી, કિંમત;
    std::tie(ચાવી, કિંમત) = P.first.split('=');
    જો (ચાવી == "prune_interval") {
      auto DurationOrErr = parseDuration(કિંમત);
      જો (!DurationOrErr)
        પાછા DurationOrErr.takeError();
      Policy.Interval = *DurationOrErr;
    } નહીંતર જો (ચાવી == "prune_after") {
      auto DurationOrErr = parseDuration(કિંમત);
      જો (!DurationOrErr)
        પાછા DurationOrErr.takeError();
      Policy.Expiration = *DurationOrErr;
    } નહીંતર જો (ચાવી == "cache_size") {
      જો (કિંમત.back() != '%')
        પાછા make_error<StringError>("'" + કિંમત + "' must be a percentage",
                                       inconvertibleErrorCode());
      StringRef SizeStr = કિંમત.drop_back();
      uint64_t Size;
      જો (SizeStr.getAsInteger(0, Size))
        પાછા make_error<StringError>("'" + SizeStr + "' not an integer",
                                       inconvertibleErrorCode());
      જો (Size > 100)
        પાછા make_error<StringError>("'" + SizeStr +
                                           "' must be between 0 and 100",
                                       inconvertibleErrorCode());
      Policy.MaxSizePercentageOfAvailableSpace = Size;
    } નહીંતર જો (ચાવી == "cache_size_bytes") {
      uint64_t Mult = 1;
      પસંદગી (tolower(કિંમત.back())) {
      સ્થિતિ 'k':
        Mult = 1024;
        કિંમત = કિંમત.drop_back();
        તોડો;
      સ્થિતિ 'm':
        Mult = 1024 * 1024;
        કિંમત = કિંમત.drop_back();
        તોડો;
      સ્થિતિ 'g':
        Mult = 1024 * 1024 * 1024;
        કિંમત = કિંમત.drop_back();
        તોડો;
      }
      uint64_t Size;
      જો (કિંમત.getAsInteger(0, Size))
        પાછા make_error<StringError>("'" + કિંમત + "' not an integer",
                                       inconvertibleErrorCode());
      Policy.MaxSizeBytes = Size * Mult;
    } નહીંતર જો (ચાવી == "cache_size_files") {
      જો (કિંમત.getAsInteger(0, Policy.MaxSizeFiles))
        પાછા make_error<StringError>("'" + કિંમત + "' not an integer",
                                       inconvertibleErrorCode());
    } નહીંતર {
      પાછા make_error<StringError>("Unknown key: '" + ચાવી + "'",
                                     inconvertibleErrorCode());
    }
  }

  પાછા Policy;
}

/// Prune the cache of files that haven't been accessed in a long time.
Expected<તાર્કિક>
llvm::pruneCache(StringRef Path, CachePruningPolicy Policy,
                 અચલ std::vector<std::unique_ptr<MemoryBuffer>> &Files) {
  using નામાકાશ std::chrono;

  જો (Path.empty())
    પાછા ખોટું;

  તાર્કિક isPathDir;
  જો (sys::fs::is_directory(Path, isPathDir))
    પાછા ખોટું;

  જો (!isPathDir)
    પાછા ખોટું;

  Policy.MaxSizePercentageOfAvailableSpace =
      std::min(Policy.MaxSizePercentageOfAvailableSpace, 100u);

  જો (Policy.Expiration == seconds(0) &&
      Policy.MaxSizePercentageOfAvailableSpace == 0 &&
      Policy.MaxSizeBytes == 0 && Policy.MaxSizeFiles == 0) {
    LLVM_DEBUG(dbgs() << "No pruning settings set, exit early\n");
    // Nothing will be pruned, early exit
    પાછા ખોટું;
  }

  // Try to stat() the timestamp file.
  SmallString<128> TimestampFile(Path);
  sys::path::append(TimestampFile, "llvmcache.timestamp");
  sys::fs::file_status FileStatus;
  અચલ auto CurrentTime = system_clock::now();
  જો (auto EC = sys::fs::status(TimestampFile, FileStatus)) {
    જો (EC == errc::no_such_file_or_directory) {
      // If the timestamp file wasn't there, create one now.
      writeTimestampFile(TimestampFile);
    } નહીંતર {
      // Unknown error?
      પાછા ખોટું;
    }
  } નહીંતર {
    જો (!Policy.Interval)
      પાછા ખોટું;
    જો (Policy.Interval != seconds(0)) {
      // Check whether the time stamp is older than our pruning interval.
      // If not, do nothing.
      અચલ auto TimeStampModTime = FileStatus.getLastModificationTime();
      auto TimeStampAge = CurrentTime - TimeStampModTime;
      જો (TimeStampAge <= *Policy.Interval) {
        LLVM_DEBUG(dbgs() << "Timestamp file too recent ("
                          << duration_cast<seconds>(TimeStampAge).ગણતરી()
                          << "s old), do not prune.\n");
        પાછા ખોટું;
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
  માટે (sys::fs::directory_iterator File(CachePathNative, EC), FileEnd;
       File != FileEnd && !EC; File.increment(EC)) {
    // Ignore filenames not beginning with "llvmcache-" or "Thin-". This
    // includes the timestamp file as well as any files created by the user.
    // This acts as a safeguard against data loss if the user specifies the
    // wrong directory as their cache directory.
    StringRef filename = sys::path::filename(File->path());
    જો (!filename.starts_with("llvmcache-") && !filename.starts_with("Thin-"))
      ચાલુ;

    // Look at this file. If we can't stat it, there's nothing interesting
    // there.
    ErrorOr<sys::fs::basic_file_status> StatusOrErr = File->status();
    જો (!StatusOrErr) {
      LLVM_DEBUG(dbgs() << "Ignore " << File->path() << " (can't stat)\n");
      ચાલુ;
    }

    // If the file hasn't been used recently enough, delete it
    અચલ auto FileAccessTime = StatusOrErr->getLastAccessedTime();
    auto FileAge = CurrentTime - FileAccessTime;
    જો (Policy.Expiration != seconds(0) && FileAge > Policy.Expiration) {
      LLVM_DEBUG(dbgs() << "Remove " << File->path() << " ("
                        << duration_cast<seconds>(FileAge).ગણતરી()
                        << "s old)\n");
      sys::fs::remove(File->path());
      ચાલુ;
    }

    // Leave it here for now, but add it to the list of size-based pruning.
    TotalSize += StatusOrErr->getSize();
    FileInfos.ઉમેરો({FileAccessTime, StatusOrErr->getSize(), File->path()});
  }

  auto FileInfo = FileInfos.begin();
  size_t અંક_Files = FileInfos.size();

  auto RemoveCacheFile = [&]() {
    // Remove the file.
    sys::fs::remove(FileInfo->Path);
    // Update size
    TotalSize -= FileInfo->Size;
    અંક_Files--;
    LLVM_DEBUG(dbgs() << " - Remove " << FileInfo->Path << " (size "
                      << FileInfo->Size << "), new occupancy is " << TotalSize
                      << "%\n");
    ++FileInfo;
  };

  // files.size() is greater the number of inputs  by one. However, a timestamp
  // file is created and stored in the cache directory if --thinlto-cache-policy
  // option is used. Therefore, files.size() is used as ActualNums.
  અચલ size_t ActualNums = Files.size();
  જો (Policy.MaxSizeFiles && ActualNums > Policy.MaxSizeFiles)
    WithColor::warning()
        << "ThinLTO cache pruning happens since the number of created files ("
        << ActualNums << ") exceeds the maximum number of files ("
        << Policy.MaxSizeFiles
        << "); consider adjusting --thinlto-cache-policy\n";

  // Prune for number of files.
  જો (Policy.MaxSizeFiles)
    જ્યારે (અંક_Files > Policy.MaxSizeFiles)
      RemoveCacheFile();

  // Prune for size now if needed
  જો (Policy.MaxSizePercentageOfAvailableSpace > 0 || Policy.MaxSizeBytes > 0) {
    auto SpaceInfoOrErr = sys::fs::disk_space(Path);
    જો (!SpaceInfoOrErr) {
      auto EC = SpaceInfoOrErr.getError();
      પાછા createStringError(EC,
                               "cannot get available disk space for '%s': '%s'",
                               Path.str().c_str(), EC.message().c_str());
    }
    sys::fs::space_info SpaceInfo = SpaceInfoOrErr.get();
    auto AvailableSpace = TotalSize + SpaceInfo.મુક્ત;

    જો (Policy.MaxSizePercentageOfAvailableSpace == 0)
      Policy.MaxSizePercentageOfAvailableSpace = 100;
    જો (Policy.MaxSizeBytes == 0)
      Policy.MaxSizeBytes = AvailableSpace;
    auto TotalSizeTarget = std::min<uint64_t>(
        AvailableSpace * Policy.MaxSizePercentageOfAvailableSpace / 100ull,
        Policy.MaxSizeBytes);

    LLVM_DEBUG(dbgs() << "Occupancy: " << ((100 * TotalSize) / AvailableSpace)
                      << "% target is: "
                      << Policy.MaxSizePercentageOfAvailableSpace << "%, "
                      << Policy.MaxSizeBytes << " bytes\n");

    size_t ActualSizes = 0;
    માટે (અચલ auto &File : Files)
      જો (File)
        ActualSizes += File->getBufferSize();

    જો (ActualSizes > TotalSizeTarget)
      WithColor::warning()
          << "ThinLTO cache pruning happens since the total size of the cache "
             "files consumed by the current link job ("
          << ActualSizes << "  bytes) exceeds maximum cache size ("
          << TotalSizeTarget
          << " bytes); consider adjusting --thinlto-cache-policy\n";

    // Remove the oldest accessed files first, till we get below the threshold.
    જ્યારે (TotalSize > TotalSizeTarget && FileInfo != FileInfos.end())
      RemoveCacheFile();
  }
  પાછા સાચું;
}
