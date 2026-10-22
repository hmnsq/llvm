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

#شامل "llvm/Support/Caching.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"

#اگر !defined(_MSC_VER) && !defined(__MINGW32__)
#شامل <unistd.h>
#ورنہ
#شامل "llvm/Support/Windows/WindowsSupport.h"
#شامل <io.h>
#اگر_ختم

استعمال نامخلا llvm;

Expected<FileCache> llvm::localCache(ثابت Twine &CacheNameRef,
                                     ثابت Twine &عارضی_File_سابقہ_Ref,
                                     ثابت Twine &Cache_Directory_راستہ_Ref,
                                     جوڑو_بفر_Fn جوڑو_بفر, منطقی CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, عارضی_File_سابقہ, Cache_Directory_راستہ;
  CacheNameRef.مقصد_سمتیہ(CacheName);
  عارضی_File_سابقہ_Ref.مقصد_سمتیہ(عارضی_File_سابقہ);
  Cache_Directory_راستہ_Ref.مقصد_سمتیہ(Cache_Directory_راستہ);

  خودکار Func = [=](غیرعلامتی Task, StringRef کلید,
                  ثابت Twine &ModuleName) -> Expected<جوڑو_دھارا_Fn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> اندراج_راستہ;
    sys::راستہ_2::append(اندراج_راستہ, Cache_Directory_راستہ, "llvmcache-" + کلید);
    // First, see if we have a cache hit.
    SmallString<64> نتیجہ_راستہ;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(اندراج_راستہ), sys::fs::کا_Update_Atime, &نتیجہ_راستہ);
    std::error_رمز EC;
    اگر (FDOrErr) {
      ErrorOr<std::unique_ptr<حافظہ_بفر>> MBOrErr =
          حافظہ_بفر::getOpenFile(*FDOrErr, اندراج_راستہ,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/جھوٹ);
      sys::fs::closeFile(*FDOrErr);
      اگر (MBOrErr) {
        جوڑو_بفر(Task, ModuleName, std::move(*MBOrErr));
        واپس جوڑو_دھارا_Fn();
      }
      EC = MBOrErr.getError();
    } ورنہ {
      EC = error_مقصد_Error_رمز(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    اگر (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      واپس createStringError(EC, Twine("Failed to open cache file ") +
                                       اندراج_راستہ + ": " + EC.پیغام_2() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    ساخت Cache_دھارا : Cached_File_دھارا {
      جوڑو_بفر_Fn جوڑو_بفر;
      sys::fs::عارضی_File عارضی_File;
      std::string ModuleName;
      غیرعلامتی Task;

      Cache_دھارا(std::unique_ptr<raw_pwrite_دھارا> OS, جوڑو_بفر_Fn جوڑو_بفر,
                  sys::fs::عارضی_File عارضی_File, std::string اندراج_راستہ,
                  std::string ModuleName, غیرعلامتی Task)
          : Cached_File_دھارا(std::move(OS), std::move(اندراج_راستہ)),
            جوڑو_بفر(std::move(جوڑو_بفر)), عارضی_File(std::move(عارضی_File)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = Cached_File_دھارا::commit();
        اگر (E)
          واپس E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<حافظہ_بفر>> MBOrErr =
            حافظہ_بفر::getOpenFile(
                sys::fs::convertFDToNativeFile(عارضی_File.FD), Object_راستہ_Name,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/جھوٹ);
        اگر (!MBOrErr) {
          std::error_رمز EC = MBOrErr.getError();
          واپس createStringError(EC, Twine("Failed to open new cache file ") +
                                           عارضی_File.TmpName + ": " +
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
        E = عارضی_File.keep(Object_راستہ_Name);
        E = handleErrors(std::move(E), [&](ثابت ECError &E) -> Error {
          std::error_رمز EC = E.convert_مقصد_Error_رمز();
          اگر (EC != errc::permission_denied)
            واپس createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        عارضی_File.TmpName + " to " + Object_راستہ_Name + ": " +
                        EC.پیغام_2() + "\n");

          خودکار MBCopy = حافظہ_بفر::get_Mem_بفر_Copy((*MBOrErr)->get_بفر(),
                                                       Object_راستہ_Name);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(عارضی_File.discard());

          واپس Error::success();
        });

        اگر (E)
          واپس E;

        جوڑو_بفر(Task, ModuleName, std::move(*MBOrErr));
        واپس Error::success();
      }

      Error abort() override {
        Error E1 = Cached_File_دھارا::abort();
        Error E2 = عارضی_File.discard();
        واپس joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    ساخت Move_File_مقصد_Cache : Cached_File_دھارا {
      جوڑو_بفر_Fn جوڑو_بفر;
      std::string ModuleName;
      size_t Task;
      StringRef File_راستہ;

      Move_File_مقصد_Cache(جوڑو_بفر_Fn جوڑو_بفر, std::string اندراج_راستہ,
                      std::string ModuleID, size_t Task)
          : Cached_File_دھارا({}, std::move(اندراج_راستہ)),
            جوڑو_بفر(std::move(جوڑو_بفر)), ModuleName(ModuleID), Task(Task) {}
      مجازی ~Move_File_مقصد_Cache() = طےشدہ;

      مجازی Error commit(std::unique_ptr<حافظہ_بفر> MemBuf) override {
        Error E = Cached_File_دھارا::commit();
        اگر (E)
          واپس E;

        File_راستہ = MemBuf->get_بفر_Identifier();
        أكد(!File_راستہ.خالی_خانے() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_رمز EC = sys::fs::rename(File_راستہ, Object_راستہ_Name);
#اگر_تعریف_شدہ _WIN32
        اگر (EC ==
            std::error_رمز(ERROR_NOT_SAME_DEVICE, std::system_category()))
#ورنہ
        اگر (EC == std::make_error_رمز(std::errc::cross_device_link))
#اگر_ختم
          EC = sys::fs::copy_file(File_راستہ, Object_راستہ_Name);
        اگر (EC)
          واپس createStringError(EC, Twine("Failed to rename or copy file ") +
                                           File_راستہ + " to " + Object_راستہ_Name +
                                           ": " + EC.پیغام_2() + "\n");

        جوڑو_بفر(Task, ModuleName, std::move(MemBuf));

        واپس Error::success();
      }
    };

    واپس [=](size_t Task, ثابت Twine &ModuleName)
               -> Expected<std::unique_ptr<Cached_File_دھارا>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      اگر (std::error_رمز EC = sys::fs::create_directories(
              Cache_Directory_راستہ, /*IgnoreExisting=*/سچ))
        واپس createStringError(EC, Twine("can't create cache directory ") +
                                         Cache_Directory_راستہ + ": " +
                                         EC.پیغام_2());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      اگر (CacheRename) {
        واپس std::make_unique<Move_File_مقصد_Cache>(
            جوڑو_بفر, std::string(اندراج_راستہ.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> عارضی_Filename_Model;
      sys::راستہ_2::append(عارضی_Filename_Model, Cache_Directory_راستہ,
                        عارضی_File_سابقہ + "-%%%%%%.tmp.o");
      Expected<sys::fs::عارضی_File> عارضی = sys::fs::عارضی_File::create(
          عارضی_Filename_Model, sys::fs::owner_read | sys::fs::owner_write);
      اگر (!عارضی)
        واپس createStringError(errc::io_error,
                                 مقصد_String(عارضی.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      واپس std::make_unique<Cache_دھارا>(
          std::make_unique<raw_fd_ostream>(عارضی->FD, /* ShouldClose */ جھوٹ),
          جوڑو_بفر, std::move(*عارضی), std::string(اندراج_راستہ), ModuleName.str(),
          Task);
    };
  };
  واپس FileCache(Func, Cache_Directory_راستہ_Ref.str());
}
