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

#ضمن "llvm/Support/Caching.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"

#إذا !defined(_MSC_VER) && !defined(__MINGW32__)
#ضمن <unistd.h>
#وإلا
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#ضمن <io.h>
#د_که_پای

کارول نطاق llvm;

Expected<FileCache> llvm::localCache(ثابت Twine &CacheNameRef,
                                     ثابت Twine &لنډمهاله_File_مختاړی_Ref,
                                     ثابت Twine &Cache_Directory_لار_Ref,
                                     جمع_کړه_بفر_Fn جمع_کړه_بفر, منطقي CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, لنډمهاله_File_مختاړی, Cache_Directory_لار;
  CacheNameRef.منزل_ویکټور(CacheName);
  لنډمهاله_File_مختاړی_Ref.منزل_ویکټور(لنډمهاله_File_مختاړی);
  Cache_Directory_لار_Ref.منزل_ویکټور(Cache_Directory_لار);

  خپلکاری Func = [=](بینښه Task, StringRef کلی,
                  ثابت Twine &ModuleName) -> Expected<جمع_کړه_بهیر_Fn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> ثبت_لار;
    sys::لار_2::append(ثبت_لار, Cache_Directory_لار, "llvmcache-" + کلی);
    // First, see if we have a cache hit.
    SmallString<64> پایله_لار;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(ثبت_لار), sys::fs::د_Update_Atime, &پایله_لار);
    std::error_کوډ EC;
    إذا (FDOrErr) {
      ErrorOr<std::unique_ptr<حافظه_بفر>> MBOrErr =
          حافظه_بفر::getOpenFile(*FDOrErr, ثبت_لار,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/خطأ);
      sys::fs::closeFile(*FDOrErr);
      إذا (MBOrErr) {
        جمع_کړه_بفر(Task, ModuleName, std::move(*MBOrErr));
        أرجع جمع_کړه_بهیر_Fn();
      }
      EC = MBOrErr.getError();
    } وإلا {
      EC = error_منزل_Error_کوډ(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    إذا (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      أرجع createStringError(EC, Twine("Failed to open cache file ") +
                                       ثبت_لار + ": " + EC.پیغام_2() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    بنية Cache_بهیر : Cached_File_بهیر {
      جمع_کړه_بفر_Fn جمع_کړه_بفر;
      sys::fs::لنډمهاله_File لنډمهاله_File;
      std::string ModuleName;
      بینښه Task;

      Cache_بهیر(std::unique_ptr<raw_pwrite_بهیر> OS, جمع_کړه_بفر_Fn جمع_کړه_بفر,
                  sys::fs::لنډمهاله_File لنډمهاله_File, std::string ثبت_لار,
                  std::string ModuleName, بینښه Task)
          : Cached_File_بهیر(std::move(OS), std::move(ثبت_لار)),
            جمع_کړه_بفر(std::move(جمع_کړه_بفر)), لنډمهاله_File(std::move(لنډمهاله_File)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = Cached_File_بهیر::commit();
        إذا (E)
          أرجع E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<حافظه_بفر>> MBOrErr =
            حافظه_بفر::getOpenFile(
                sys::fs::convertFDToNativeFile(لنډمهاله_File.FD), Object_لار_Name,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/خطأ);
        إذا (!MBOrErr) {
          std::error_کوډ EC = MBOrErr.getError();
          أرجع createStringError(EC, Twine("Failed to open new cache file ") +
                                           لنډمهاله_File.TmpName + ": " +
                                           EC.پیغام_2() + "\n");
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
        E = لنډمهاله_File.keep(Object_لار_Name);
        E = handleErrors(std::move(E), [&](ثابت ECError &E) -> Error {
          std::error_کوډ EC = E.convert_منزل_Error_کوډ();
          إذا (EC != errc::permission_denied)
            أرجع createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        لنډمهاله_File.TmpName + " to " + Object_لار_Name + ": " +
                        EC.پیغام_2() + "\n");

          خپلکاری MBCopy = حافظه_بفر::get_Mem_بفر_Copy((*MBOrErr)->get_بفر(),
                                                       Object_لار_Name);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(لنډمهاله_File.discard());

          أرجع Error::success();
        });

        إذا (E)
          أرجع E;

        جمع_کړه_بفر(Task, ModuleName, std::move(*MBOrErr));
        أرجع Error::success();
      }

      Error abort() override {
        Error E1 = Cached_File_بهیر::abort();
        Error E2 = لنډمهاله_File.discard();
        أرجع joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    بنية Move_File_منزل_Cache : Cached_File_بهیر {
      جمع_کړه_بفر_Fn جمع_کړه_بفر;
      std::string ModuleName;
      size_t Task;
      StringRef File_لار;

      Move_File_منزل_Cache(جمع_کړه_بفر_Fn جمع_کړه_بفر, std::string ثبت_لار,
                      std::string ModuleID, size_t Task)
          : Cached_File_بهیر({}, std::move(ثبت_لار)),
            جمع_کړه_بفر(std::move(جمع_کړه_بفر)), ModuleName(ModuleID), Task(Task) {}
      مجازي ~Move_File_منزل_Cache() = افتراضي;

      مجازي Error commit(std::unique_ptr<حافظه_بفر> MemBuf) override {
        Error E = Cached_File_بهیر::commit();
        إذا (E)
          أرجع E;

        File_لار = MemBuf->get_بفر_Identifier();
        أكد(!File_لار.تشه() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_کوډ EC = sys::fs::rename(File_لار, Object_لار_Name);
#که_تعریف_شوی _WIN32
        إذا (EC ==
            std::error_کوډ(ERROR_NOT_SAME_DEVICE, std::system_category()))
#وإلا
        إذا (EC == std::make_error_کوډ(std::errc::cross_device_link))
#د_که_پای
          EC = sys::fs::copy_file(File_لار, Object_لار_Name);
        إذا (EC)
          أرجع createStringError(EC, Twine("Failed to rename or copy file ") +
                                           File_لار + " to " + Object_لار_Name +
                                           ": " + EC.پیغام_2() + "\n");

        جمع_کړه_بفر(Task, ModuleName, std::move(MemBuf));

        أرجع Error::success();
      }
    };

    أرجع [=](size_t Task, ثابت Twine &ModuleName)
               -> Expected<std::unique_ptr<Cached_File_بهیر>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      إذا (std::error_کوډ EC = sys::fs::create_directories(
              Cache_Directory_لار, /*IgnoreExisting=*/ریښتیا))
        أرجع createStringError(EC, Twine("can't create cache directory ") +
                                         Cache_Directory_لار + ": " +
                                         EC.پیغام_2());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      إذا (CacheRename) {
        أرجع std::make_unique<Move_File_منزل_Cache>(
            جمع_کړه_بفر, std::string(ثبت_لار.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> لنډمهاله_Filename_Model;
      sys::لار_2::append(لنډمهاله_Filename_Model, Cache_Directory_لار,
                        لنډمهاله_File_مختاړی + "-%%%%%%.tmp.o");
      Expected<sys::fs::لنډمهاله_File> لنډمهاله = sys::fs::لنډمهاله_File::create(
          لنډمهاله_Filename_Model, sys::fs::owner_read | sys::fs::owner_write);
      إذا (!لنډمهاله)
        أرجع createStringError(errc::io_error,
                                 منزل_String(لنډمهاله.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      أرجع std::make_unique<Cache_بهیر>(
          std::make_unique<raw_fd_ostream>(لنډمهاله->FD, /* ShouldClose */ خطأ),
          جمع_کړه_بفر, std::move(*لنډمهاله), std::string(ثبت_لار), ModuleName.str(),
          Task);
    };
  };
  أرجع FileCache(Func, Cache_Directory_لار_Ref.str());
}
