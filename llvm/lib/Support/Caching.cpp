//===-Caching.cpp - LLVM Local File Cache ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the localCache function, which simplifies creating,
// adding to, and querying a local file system cache. localCache takes care of
// periodically pruning older files from the cache using a CachePruningPolicy.
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/Caching.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"

#यदि !defined(_MSC_VER) && !defined(__MINGW32__)
#अन्तर्भाव <unistd.h>
#अन्यथा
#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"
#अन्तर्भाव <io.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

Expected<FileCache> llvm::localCache(ध्रुव Twine &CacheNameRef,
                                     ध्रुव Twine &अस्थायी_File_उपसर्गः_Ref,
                                     ध्रुव Twine &Cache_Directory_मार्गः_Ref,
                                     सङ्कलय_बफर_Fn सङ्कलय_बफर, तार्किक CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, अस्थायी_File_उपसर्गः, Cache_Directory_मार्गः;
  CacheNameRef.गन्तव्यम्_सदिशम्(CacheName);
  अस्थायी_File_उपसर्गः_Ref.गन्तव्यम्_सदिशम्(अस्थायी_File_उपसर्गः);
  Cache_Directory_मार्गः_Ref.गन्तव्यम्_सदिशम्(Cache_Directory_मार्गः);

  स्वचालित Func = [=](अचिह्नित Task, StringRef कुञ्चिका,
                  ध्रुव Twine &ModuleName) -> Expected<सङ्कलय_प्रवाहः_Fn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> प्रविष्टिः_मार्गः;
    sys::मार्गः_2::append(प्रविष्टिः_मार्गः, Cache_Directory_मार्गः, "llvmcache-" + कुञ्चिका);
    // First, see if we have a cache hit.
    SmallString<64> परिणाम_मार्गः;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(प्रविष्टिः_मार्गः), sys::fs::सम्बन्धि_Update_Atime, &परिणाम_मार्गः);
    std::error_संहिता EC;
    यदि (FDOrErr) {
      ErrorOr<std::unique_ptr<स्मृतिः_बफर>> MBOrErr =
          स्मृतिः_बफर::getOpenFile(*FDOrErr, प्रविष्टिः_मार्गः,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/असत्य);
      sys::fs::closeFile(*FDOrErr);
      यदि (MBOrErr) {
        सङ्कलय_बफर(Task, ModuleName, std::move(*MBOrErr));
        प्रत्यागम सङ्कलय_प्रवाहः_Fn();
      }
      EC = MBOrErr.getError();
    } अन्यथा {
      EC = error_गन्तव्यम्_Error_संहिता(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    यदि (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      प्रत्यागम createStringError(EC, Twine("Failed to open cache file ") +
                                       प्रविष्टिः_मार्गः + ": " + EC.सन्देशः_2() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    संरचना Cache_प्रवाहः : Cached_File_प्रवाहः {
      सङ्कलय_बफर_Fn सङ्कलय_बफर;
      sys::fs::अस्थायी_File अस्थायी_File;
      std::string ModuleName;
      अचिह्नित Task;

      Cache_प्रवाहः(std::unique_ptr<raw_pwrite_प्रवाहः> OS, सङ्कलय_बफर_Fn सङ्कलय_बफर,
                  sys::fs::अस्थायी_File अस्थायी_File, std::string प्रविष्टिः_मार्गः,
                  std::string ModuleName, अचिह्नित Task)
          : Cached_File_प्रवाहः(std::move(OS), std::move(प्रविष्टिः_मार्गः)),
            सङ्कलय_बफर(std::move(सङ्कलय_बफर)), अस्थायी_File(std::move(अस्थायी_File)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = Cached_File_प्रवाहः::commit();
        यदि (E)
          प्रत्यागम E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<स्मृतिः_बफर>> MBOrErr =
            स्मृतिः_बफर::getOpenFile(
                sys::fs::convertFDToNativeFile(अस्थायी_File.FD), Object_मार्गः_Name,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/असत्य);
        यदि (!MBOrErr) {
          std::error_संहिता EC = MBOrErr.getError();
          प्रत्यागम createStringError(EC, Twine("Failed to open new cache file ") +
                                           अस्थायी_File.TmpName + ": " +
                                           EC.सन्देशः_2() + "\n");
        }

        // On POSIX systems, this will atomically replace the destination if
        // it already exists. We try to emulate this on Windows, but this may
        // fail with a permission denied error (for example, if the destination
        // is currently opened by another process that does not give us the
        // sharing permissions we need). Since the existing file should be
        // semantically equivalent to the one we are trying to write, we give
        // AddBuffer a copy of the bytes we wrote in that case. We do this
        // instead of just using the existing file, because the pruner might
        // delete the file before we get a chance to use it.
        E = अस्थायी_File.keep(Object_मार्गः_Name);
        E = handleErrors(std::move(E), [&](ध्रुव ECError &E) -> Error {
          std::error_संहिता EC = E.convert_गन्तव्यम्_Error_संहिता();
          यदि (EC != errc::permission_denied)
            प्रत्यागम createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        अस्थायी_File.TmpName + " to " + Object_मार्गः_Name + ": " +
                        EC.सन्देशः_2() + "\n");

          स्वचालित MBCopy = स्मृतिः_बफर::get_Mem_बफर_Copy((*MBOrErr)->get_बफर(),
                                                       Object_मार्गः_Name);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(अस्थायी_File.discard());

          प्रत्यागम Error::success();
        });

        यदि (E)
          प्रत्यागम E;

        सङ्कलय_बफर(Task, ModuleName, std::move(*MBOrErr));
        प्रत्यागम Error::success();
      }

      Error abort() override {
        Error E1 = Cached_File_प्रवाहः::abort();
        Error E2 = अस्थायी_File.discard();
        प्रत्यागम joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    संरचना Move_File_गन्तव्यम्_Cache : Cached_File_प्रवाहः {
      सङ्कलय_बफर_Fn सङ्कलय_बफर;
      std::string ModuleName;
      size_t Task;
      StringRef File_मार्गः;

      Move_File_गन्तव्यम्_Cache(सङ्कलय_बफर_Fn सङ्कलय_बफर, std::string प्रविष्टिः_मार्गः,
                      std::string ModuleID, size_t Task)
          : Cached_File_प्रवाहः({}, std::move(प्रविष्टिः_मार्गः)),
            सङ्कलय_बफर(std::move(सङ्कलय_बफर)), ModuleName(ModuleID), Task(Task) {}
      आभास ~Move_File_गन्तव्यम्_Cache() = स्वभाव;

      आभास Error commit(std::unique_ptr<स्मृतिः_बफर> MemBuf) override {
        Error E = Cached_File_प्रवाहः::commit();
        यदि (E)
          प्रत्यागम E;

        File_मार्गः = MemBuf->get_बफर_Identifier();
        प्रतिज्ञा(!File_मार्गः.रिक्तानि() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_संहिता EC = sys::fs::rename(File_मार्गः, Object_मार्गः_Name);
#यदि_परिभाषितम् _WIN32
        यदि (EC ==
            std::error_संहिता(ERROR_NOT_SAME_DEVICE, std::system_category()))
#अन्यथा
        यदि (EC == std::make_error_संहिता(std::errc::cross_device_link))
#यदि_अन्तः
          EC = sys::fs::copy_file(File_मार्गः, Object_मार्गः_Name);
        यदि (EC)
          प्रत्यागम createStringError(EC, Twine("Failed to rename or copy file ") +
                                           File_मार्गः + " to " + Object_मार्गः_Name +
                                           ": " + EC.सन्देशः_2() + "\n");

        सङ्कलय_बफर(Task, ModuleName, std::move(MemBuf));

        प्रत्यागम Error::success();
      }
    };

    प्रत्यागम [=](size_t Task, ध्रुव Twine &ModuleName)
               -> Expected<std::unique_ptr<Cached_File_प्रवाहः>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      यदि (std::error_संहिता EC = sys::fs::create_directories(
              Cache_Directory_मार्गः, /*IgnoreExisting=*/सत्य))
        प्रत्यागम createStringError(EC, Twine("can't create cache directory ") +
                                         Cache_Directory_मार्गः + ": " +
                                         EC.सन्देशः_2());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      यदि (CacheRename) {
        प्रत्यागम std::make_unique<Move_File_गन्तव्यम्_Cache>(
            सङ्कलय_बफर, std::string(प्रविष्टिः_मार्गः.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> अस्थायी_Filename_Model;
      sys::मार्गः_2::append(अस्थायी_Filename_Model, Cache_Directory_मार्गः,
                        अस्थायी_File_उपसर्गः + "-%%%%%%.tmp.o");
      Expected<sys::fs::अस्थायी_File> अस्थायी = sys::fs::अस्थायी_File::create(
          अस्थायी_Filename_Model, sys::fs::owner_read | sys::fs::owner_write);
      यदि (!अस्थायी)
        प्रत्यागम createStringError(errc::io_error,
                                 गन्तव्यम्_String(अस्थायी.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      प्रत्यागम std::make_unique<Cache_प्रवाहः>(
          std::make_unique<raw_fd_ostream>(अस्थायी->FD, /* ShouldClose */ असत्य),
          सङ्कलय_बफर, std::move(*अस्थायी), std::string(प्रविष्टिः_मार्गः), ModuleName.str(),
          Task);
    };
  };
  प्रत्यागम FileCache(Func, Cache_Directory_मार्गः_Ref.str());
}
