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

#परिभाषय DEBUG_TYPE "cache-pruning"

#अन्तर्भाव <set>
#अन्तर्भाव <system_error>

उपयुज्य नामाकाश llvm;

नामाकाश {
संरचना FileInfo {
  sys::TimePoint<> Time;
  uint64_t आकार;
  std::string मार्गः;

  /// Used to determine which files to prune first. Also used to determine
  /// set membership, so must take into account all fields.
  तार्किक संक्रियकः<(ध्रुव FileInfo &Other) ध्रुव {
    प्रत्यागम std::tie(Time, Other.आकार, मार्गः) <
           std::tie(Other.Time, आकार, Other.मार्गः);
  }
};
} // anonymous namespace

/// Write a new timestamp file with the given path. This is used for the pruning
/// interval option.
स्थिर शून्य writeTimestampFile(StringRef TimestampFile) {
  std::error_संहिता EC;
  raw_fd_ostream बहिर्गमनम्(TimestampFile.str(), EC, sys::fs::सम्बन्धि_None);
}

स्थिर Expected<std::chrono::seconds> parseDuration(StringRef Duration) {
  यदि (Duration.रिक्तानि())
    प्रत्यागम make_error<StringError>("Duration must not be empty",
                                   inconvertible_Error_संहिता());

  StringRef अङ्कः_Str = Duration.खण्डिका(0, Duration.आकार_2()-1);
  uint64_t अङ्कः;
  यदि (अङ्कः_Str.getAsInteger(0, अङ्कः))
    प्रत्यागम make_error<StringError>("'" + अङ्कः_Str + "' not an integer",
                                   inconvertible_Error_संहिता());

  विकल्प (Duration.पृष्ठभागः()) {
  पक्ष 's':
    प्रत्यागम std::chrono::seconds(अङ्कः);
  पक्ष 'm':
    प्रत्यागम std::chrono::minutes(अङ्कः);
  पक्ष 'h':
    प्रत्यागम std::chrono::hours(अङ्कः);
  स्वभाव:
    प्रत्यागम make_error<StringError>("'" + Duration +
                                       "' must end with one of 's', 'm' or 'h'",
                                   inconvertible_Error_संहिता());
  }
}

Expected<CachePruningPolicy>
llvm::parseCachePruningPolicy(StringRef PolicyStr) {
  CachePruningPolicy Policy;
  std::pair<StringRef, StringRef> P = {"", PolicyStr};
  यावत् (!P.second.रिक्तानि()) {
    P = P.second.विभज(':');

    StringRef कुञ्चिका, मानम्;
    std::tie(कुञ्चिका, मानम्) = P.first.विभज('=');
    यदि (कुञ्चिका == "prune_interval") {
      स्वचालित DurationOrErr = parseDuration(मानम्);
      यदि (!DurationOrErr)
        प्रत्यागम DurationOrErr.takeError();
      Policy.Interval = *DurationOrErr;
    } अन्यथा यदि (कुञ्चिका == "prune_after") {
      स्वचालित DurationOrErr = parseDuration(मानम्);
      यदि (!DurationOrErr)
        प्रत्यागम DurationOrErr.takeError();
      Policy.Expiration = *DurationOrErr;
    } अन्यथा यदि (कुञ्चिका == "cache_size") {
      यदि (मानम्.पृष्ठभागः() != '%')
        प्रत्यागम make_error<StringError>("'" + मानम् + "' must be a percentage",
                                       inconvertible_Error_संहिता());
      StringRef आकार_Str = मानम्.drop_पृष्ठभागः();
      uint64_t आकार;
      यदि (आकार_Str.getAsInteger(0, आकार))
        प्रत्यागम make_error<StringError>("'" + आकार_Str + "' not an integer",
                                       inconvertible_Error_संहिता());
      यदि (आकार > 100)
        प्रत्यागम make_error<StringError>("'" + आकार_Str +
                                           "' must be between 0 and 100",
                                       inconvertible_Error_संहिता());
      Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space = आकार;
    } अन्यथा यदि (कुञ्चिका == "cache_size_bytes") {
      uint64_t Mult = 1;
      विकल्प (tolower(मानम्.पृष्ठभागः())) {
      पक्ष 'k':
        Mult = 1024;
        मानम् = मानम्.drop_पृष्ठभागः();
        भङ्ग;
      पक्ष 'm':
        Mult = 1024 * 1024;
        मानम् = मानम्.drop_पृष्ठभागः();
        भङ्ग;
      पक्ष 'g':
        Mult = 1024 * 1024 * 1024;
        मानम् = मानम्.drop_पृष्ठभागः();
        भङ्ग;
      }
      uint64_t आकार;
      यदि (मानम्.getAsInteger(0, आकार))
        प्रत्यागम make_error<StringError>("'" + मानम् + "' not an integer",
                                       inconvertible_Error_संहिता());
      Policy.Max_आकार_अष्टकानि = आकार * Mult;
    } अन्यथा यदि (कुञ्चिका == "cache_size_files") {
      यदि (मानम्.getAsInteger(0, Policy.Max_आकार_Files))
        प्रत्यागम make_error<StringError>("'" + मानम् + "' not an integer",
                                       inconvertible_Error_संहिता());
    } अन्यथा {
      प्रत्यागम make_error<StringError>("Unknown key: '" + कुञ्चिका + "'",
                                     inconvertible_Error_संहिता());
    }
  }

  प्रत्यागम Policy;
}

/// Prune the cache of files that haven't been accessed in a long time.
Expected<तार्किक>
llvm::pruneCache(StringRef मार्गः, CachePruningPolicy Policy,
                 ध्रुव std::सदिशम्<std::unique_ptr<स्मृतिः_बफर>> &Files) {
  उपयुज्य नामाकाश std::chrono;

  यदि (मार्गः.रिक्तानि())
    प्रत्यागम असत्य;

  तार्किक is_मार्गः_Dir;
  यदि (sys::fs::is_directory(मार्गः, is_मार्गः_Dir))
    प्रत्यागम असत्य;

  यदि (!is_मार्गः_Dir)
    प्रत्यागम असत्य;

  Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space =
      std::min(Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space, 100u);

  यदि (Policy.Expiration == seconds(0) &&
      Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space == 0 &&
      Policy.Max_आकार_अष्टकानि == 0 && Policy.Max_आकार_Files == 0) {
    LLVM_DEBUG(dbgs() << "No pruning settings set, exit early\n");
    // Nothing will be pruned, early exit
    प्रत्यागम असत्य;
  }

  // Try to stat() the timestamp file.
  SmallString<128> TimestampFile(मार्गः);
  sys::मार्गः_2::append(TimestampFile, "llvmcache.timestamp");
  sys::fs::file_स्थितिः File_स्थितिः;
  ध्रुव स्वचालित CurrentTime = system_clock::now();
  यदि (स्वचालित EC = sys::fs::स्थितिः_2(TimestampFile, File_स्थितिः)) {
    यदि (EC == errc::no_such_file_or_directory) {
      // If the timestamp file wasn't there, create one now.
      writeTimestampFile(TimestampFile);
    } अन्यथा {
      // Unknown error?
      प्रत्यागम असत्य;
    }
  } अन्यथा {
    यदि (!Policy.Interval)
      प्रत्यागम असत्य;
    यदि (Policy.Interval != seconds(0)) {
      // Check whether the time stamp is older than our pruning interval.
      // If not, do nothing.
      ध्रुव स्वचालित TimeStampModTime = File_स्थितिः.get_चरमः_Modification_Time();
      स्वचालित TimeStampAge = CurrentTime - TimeStampModTime;
      यदि (TimeStampAge <= *Policy.Interval) {
        LLVM_DEBUG(dbgs() << "Timestamp file too recent ("
                          << duration_cast<seconds>(TimeStampAge).गणनम्()
                          << "s old), do not prune.\n");
        प्रत्यागम असत्य;
      }
    }
    // Write a new timestamp file so that nobody else attempts to prune.
    // There is a benign race condition here, if two processes happen to
    // notice at the same time that the timestamp is out-of-date.
    writeTimestampFile(TimestampFile);
  }

  // Keep track of files to delete to get below the size limit.
  // Order by time of last use so that recently used files are preserved.
  std::समुच्चयः<FileInfo> FileInfos;
  uint64_t योग_आकार = 0;

  // Walk the entire directory cache, looking for unused files.
  std::error_संहिता EC;
  SmallString<128> Cache_मार्गः_Native;
  sys::मार्गः_2::native(मार्गः, Cache_मार्गः_Native);
  // Walk all of the files within this directory.
  प्रति (sys::fs::directory_iterator File(Cache_मार्गः_Native, EC), File_अन्तः;
       File != File_अन्तः && !EC; File.increment(EC)) {
    // Ignore filenames not beginning with "llvmcache-" or "Thin-". This
    // includes the timestamp file as well as any files created by the user.
    // This acts as a safeguard against data loss if the user specifies the
    // wrong directory as their cache directory.
    StringRef filename = sys::मार्गः_2::filename(File->मार्गः_2());
    यदि (!filename.starts_with("llvmcache-") && !filename.starts_with("Thin-"))
      अनुवर्त;

    // Look at this file. If we can't stat it, there's nothing interesting
    // there.
    ErrorOr<sys::fs::basic_file_स्थितिः> स्थितिः_Or_Err = File->स्थितिः_2();
    यदि (!स्थितिः_Or_Err) {
      LLVM_DEBUG(dbgs() << "Ignore " << File->मार्गः_2() << " (can't stat)\n");
      अनुवर्त;
    }

    // If the file hasn't been used recently enough, delete it
    ध्रुव स्वचालित FileAccessTime = स्थितिः_Or_Err->get_चरमः_Accessed_Time();
    स्वचालित FileAge = CurrentTime - FileAccessTime;
    यदि (Policy.Expiration != seconds(0) && FileAge > Policy.Expiration) {
      LLVM_DEBUG(dbgs() << "Remove " << File->मार्गः_2() << " ("
                        << duration_cast<seconds>(FileAge).गणनम्()
                        << "s old)\n");
      sys::fs::remove(File->मार्गः_2());
      अनुवर्त;
    }

    // Leave it here for now, but add it to the list of size-based pruning.
    योग_आकार += स्थितिः_Or_Err->get_आकार();
    FileInfos.निवेशय({FileAccessTime, स्थितिः_Or_Err->get_आकार(), File->मार्गः_2()});
  }

  स्वचालित FileInfo = FileInfos.begin();
  size_t अङ्कः_Files = FileInfos.आकार_2();

  स्वचालित RemoveCacheFile = [&]() {
    // Remove the file.
    sys::fs::remove(FileInfo->मार्गः);
    // Update size
    योग_आकार -= FileInfo->आकार;
    अङ्कः_Files--;
    LLVM_DEBUG(dbgs() << " - Remove " << FileInfo->मार्गः << " (size "
                      << FileInfo->आकार << "), new occupancy is " << योग_आकार
                      << "%\n");
    ++FileInfo;
  };

  // files.size() is greater the number of inputs  by one. However, a timestamp
  // file is created and stored in the cache directory if --thinlto-cache-policy
  // option is used. Therefore, files.size() is used as ActualNums.
  ध्रुव size_t ActualNums = Files.आकार_2();
  यदि (Policy.Max_आकार_Files && ActualNums > Policy.Max_आकार_Files)
    WithColor::warning()
        << "ThinLTO cache pruning happens since the number of created files ("
        << ActualNums << ") exceeds the maximum number of files ("
        << Policy.Max_आकार_Files
        << "); consider adjusting --thinlto-cache-policy\n";

  // Prune for number of files.
  यदि (Policy.Max_आकार_Files)
    यावत् (अङ्कः_Files > Policy.Max_आकार_Files)
      RemoveCacheFile();

  // Prune for size now if needed
  यदि (Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space > 0 || Policy.Max_आकार_अष्टकानि > 0) {
    स्वचालित SpaceInfoOrErr = sys::fs::disk_space(मार्गः);
    यदि (!SpaceInfoOrErr) {
      स्वचालित EC = SpaceInfoOrErr.getError();
      प्रत्यागम createStringError(EC,
                               "cannot get available disk space for '%s': '%s'",
                               मार्गः.str().c_str(), EC.सन्देशः_2().c_str());
    }
    sys::fs::space_info SpaceInfo = SpaceInfoOrErr.get();
    स्वचालित उपलब्धम्_Space = योग_आकार + SpaceInfo.मुक्त;

    यदि (Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space == 0)
      Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space = 100;
    यदि (Policy.Max_आकार_अष्टकानि == 0)
      Policy.Max_आकार_अष्टकानि = उपलब्धम्_Space;
    स्वचालित योग_आकार_Target = std::min<uint64_t>(
        उपलब्धम्_Space * Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space / 100ull,
        Policy.Max_आकार_अष्टकानि);

    LLVM_DEBUG(dbgs() << "Occupancy: " << ((100 * योग_आकार) / उपलब्धम्_Space)
                      << "% target is: "
                      << Policy.Max_आकार_Percentage_सम्बन्धि_उपलब्धम्_Space << "%, "
                      << Policy.Max_आकार_अष्टकानि << " bytes\n");

    size_t ActualSizes = 0;
    प्रति (ध्रुव स्वचालित &File : Files)
      यदि (File)
        ActualSizes += File->get_बफर_आकार();

    यदि (ActualSizes > योग_आकार_Target)
      WithColor::warning()
          << "ThinLTO cache pruning happens since the total size of the cache "
             "files consumed by the current link job ("
          << ActualSizes << "  bytes) exceeds maximum cache size ("
          << योग_आकार_Target
          << " bytes); consider adjusting --thinlto-cache-policy\n";

    // Remove the oldest accessed files first, till we get below the threshold.
    यावत् (योग_आकार > योग_आकार_Target && FileInfo != FileInfos.अन्तः())
      RemoveCacheFile();
  }
  प्रत्यागम सत्य;
}
