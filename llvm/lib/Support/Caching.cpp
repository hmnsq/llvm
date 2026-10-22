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
#جيڪڏهن_پورو

استعمال نطاق llvm;

Expected<FileCache> llvm::localCache(ثابت Twine &CacheNameRef,
                                     ثابت Twine &عارضي_File_اڳياڙي_Ref,
                                     ثابت Twine &Cache_Directory_رستو_Ref,
                                     جوڙيو_بفر_Fn جوڙيو_بفر, منطقي CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, عارضي_File_اڳياڙي, Cache_Directory_رستو;
  CacheNameRef.منزل_ويڪٽر(CacheName);
  عارضي_File_اڳياڙي_Ref.منزل_ويڪٽر(عارضي_File_اڳياڙي);
  Cache_Directory_رستو_Ref.منزل_ويڪٽر(Cache_Directory_رستو);

  خودڪار Func = [=](بنانشان Task, StringRef ڪنجي,
                  ثابت Twine &ModuleName) -> Expected<جوڙيو_وهڪرو_Fn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> داخلا_رستو;
    sys::رستو_2::append(داخلا_رستو, Cache_Directory_رستو, "llvmcache-" + ڪنجي);
    // First, see if we have a cache hit.
    SmallString<64> نتيجو_رستو;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(داخلا_رستو), sys::fs::جو_Update_Atime, &نتيجو_رستو);
    std::error_ڪوڊ EC;
    إذا (FDOrErr) {
      ErrorOr<std::unique_ptr<يادگيري_بفر>> MBOrErr =
          يادگيري_بفر::getOpenFile(*FDOrErr, داخلا_رستو,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/خطأ);
      sys::fs::closeFile(*FDOrErr);
      إذا (MBOrErr) {
        جوڙيو_بفر(Task, ModuleName, std::move(*MBOrErr));
        أرجع جوڙيو_وهڪرو_Fn();
      }
      EC = MBOrErr.getError();
    } وإلا {
      EC = error_منزل_Error_ڪوڊ(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    إذا (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      أرجع createStringError(EC, Twine("Failed to open cache file ") +
                                       داخلا_رستو + ": " + EC.پيغام() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    بنية Cache_وهڪرو : Cached_File_وهڪرو {
      جوڙيو_بفر_Fn جوڙيو_بفر;
      sys::fs::عارضي_File عارضي_File;
      std::string ModuleName;
      بنانشان Task;

      Cache_وهڪرو(std::unique_ptr<raw_pwrite_وهڪرو> OS, جوڙيو_بفر_Fn جوڙيو_بفر,
                  sys::fs::عارضي_File عارضي_File, std::string داخلا_رستو,
                  std::string ModuleName, بنانشان Task)
          : Cached_File_وهڪرو(std::move(OS), std::move(داخلا_رستو)),
            جوڙيو_بفر(std::move(جوڙيو_بفر)), عارضي_File(std::move(عارضي_File)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = Cached_File_وهڪرو::commit();
        إذا (E)
          أرجع E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<يادگيري_بفر>> MBOrErr =
            يادگيري_بفر::getOpenFile(
                sys::fs::convertFDToNativeFile(عارضي_File.FD), Object_رستو_Name,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/خطأ);
        إذا (!MBOrErr) {
          std::error_ڪوڊ EC = MBOrErr.getError();
          أرجع createStringError(EC, Twine("Failed to open new cache file ") +
                                           عارضي_File.TmpName + ": " +
                                           EC.پيغام() + "\n");
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
        E = عارضي_File.keep(Object_رستو_Name);
        E = handleErrors(std::move(E), [&](ثابت ECError &E) -> Error {
          std::error_ڪوڊ EC = E.convert_منزل_Error_ڪوڊ();
          إذا (EC != errc::permission_denied)
            أرجع createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        عارضي_File.TmpName + " to " + Object_رستو_Name + ": " +
                        EC.پيغام() + "\n");

          خودڪار MBCopy = يادگيري_بفر::get_Mem_بفر_Copy((*MBOrErr)->get_بفر(),
                                                       Object_رستو_Name);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(عارضي_File.discard());

          أرجع Error::success();
        });

        إذا (E)
          أرجع E;

        جوڙيو_بفر(Task, ModuleName, std::move(*MBOrErr));
        أرجع Error::success();
      }

      Error abort() override {
        Error E1 = Cached_File_وهڪرو::abort();
        Error E2 = عارضي_File.discard();
        أرجع joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    بنية Move_File_منزل_Cache : Cached_File_وهڪرو {
      جوڙيو_بفر_Fn جوڙيو_بفر;
      std::string ModuleName;
      size_t Task;
      StringRef File_رستو;

      Move_File_منزل_Cache(جوڙيو_بفر_Fn جوڙيو_بفر, std::string داخلا_رستو,
                      std::string ModuleID, size_t Task)
          : Cached_File_وهڪرو({}, std::move(داخلا_رستو)),
            جوڙيو_بفر(std::move(جوڙيو_بفر)), ModuleName(ModuleID), Task(Task) {}
      مجازي ~Move_File_منزل_Cache() = افتراضي;

      مجازي Error commit(std::unique_ptr<يادگيري_بفر> MemBuf) override {
        Error E = Cached_File_وهڪرو::commit();
        إذا (E)
          أرجع E;

        File_رستو = MemBuf->get_بفر_Identifier();
        أكد(!File_رستو.خالي_جايون() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_ڪوڊ EC = sys::fs::rename(File_رستو, Object_رستو_Name);
#جيڪڏهن_وصف_ٿيل _WIN32
        إذا (EC ==
            std::error_ڪوڊ(ERROR_NOT_SAME_DEVICE, std::system_category()))
#وإلا
        إذا (EC == std::make_error_ڪوڊ(std::errc::cross_device_link))
#جيڪڏهن_پورو
          EC = sys::fs::copy_file(File_رستو, Object_رستو_Name);
        إذا (EC)
          أرجع createStringError(EC, Twine("Failed to rename or copy file ") +
                                           File_رستو + " to " + Object_رستو_Name +
                                           ": " + EC.پيغام() + "\n");

        جوڙيو_بفر(Task, ModuleName, std::move(MemBuf));

        أرجع Error::success();
      }
    };

    أرجع [=](size_t Task, ثابت Twine &ModuleName)
               -> Expected<std::unique_ptr<Cached_File_وهڪرو>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      إذا (std::error_ڪوڊ EC = sys::fs::create_directories(
              Cache_Directory_رستو, /*IgnoreExisting=*/سچ))
        أرجع createStringError(EC, Twine("can't create cache directory ") +
                                         Cache_Directory_رستو + ": " +
                                         EC.پيغام());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      إذا (CacheRename) {
        أرجع std::make_unique<Move_File_منزل_Cache>(
            جوڙيو_بفر, std::string(داخلا_رستو.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> عارضي_Filename_Model;
      sys::رستو_2::append(عارضي_Filename_Model, Cache_Directory_رستو,
                        عارضي_File_اڳياڙي + "-%%%%%%.tmp.o");
      Expected<sys::fs::عارضي_File> عارضي = sys::fs::عارضي_File::create(
          عارضي_Filename_Model, sys::fs::owner_read | sys::fs::owner_write);
      إذا (!عارضي)
        أرجع createStringError(errc::io_error,
                                 منزل_String(عارضي.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      أرجع std::make_unique<Cache_وهڪرو>(
          std::make_unique<raw_fd_ostream>(عارضي->FD, /* ShouldClose */ خطأ),
          جوڙيو_بفر, std::move(*عارضي), std::string(داخلا_رستو), ModuleName.str(),
          Task);
    };
  };
  أرجع FileCache(Func, Cache_Directory_رستو_Ref.str());
}
