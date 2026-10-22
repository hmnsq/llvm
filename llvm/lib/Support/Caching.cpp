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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Caching.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"

#ਜੇ !defined(_MSC_VER) && !defined(__MINGW32__)
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Windows/WindowsSupport.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <io.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

Expected<FileCache> llvm::localCache(ਅਚਲ Twine &CacheNameRef,
                                     ਅਚਲ Twine &ਆਰਜ਼ੀ_File_ਅਗੇਤਰ_Ref,
                                     ਅਚਲ Twine &Cache_Directory_ਰਸਤਾ_Ref,
                                     ਜੋੜੋ_ਬਫਰ_Fn ਜੋੜੋ_ਬਫਰ, ਤਾਰਕਿਕ CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, ਆਰਜ਼ੀ_File_ਅਗੇਤਰ, Cache_Directory_ਰਸਤਾ;
  CacheNameRef.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(CacheName);
  ਆਰਜ਼ੀ_File_ਅਗੇਤਰ_Ref.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਆਰਜ਼ੀ_File_ਅਗੇਤਰ);
  Cache_Directory_ਰਸਤਾ_Ref.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(Cache_Directory_ਰਸਤਾ);

  ਸਵੈਚਾਲਿਤ Func = [=](ਬਿਨਾਨਿਸ਼ਾਨੀ Task, StringRef ਕੁੰਜੀ,
                  ਅਚਲ Twine &ModuleName) -> Expected<ਜੋੜੋ_ਧਾਰਾ_Fn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> ਇੰਦਰਾਜ_ਰਸਤਾ;
    sys::ਰਸਤਾ_2::append(ਇੰਦਰਾਜ_ਰਸਤਾ, Cache_Directory_ਰਸਤਾ, "llvmcache-" + ਕੁੰਜੀ);
    // First, see if we have a cache hit.
    SmallString<64> ਨਤੀਜਾ_ਰਸਤਾ;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(ਇੰਦਰਾਜ_ਰਸਤਾ), sys::fs::ਦਾ_Update_Atime, &ਨਤੀਜਾ_ਰਸਤਾ);
    std::error_ਕੋਡ EC;
    ਜੇ (FDOrErr) {
      ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> MBOrErr =
          ਸਮ੍ਰਿਤੀ_ਬਫਰ::getOpenFile(*FDOrErr, ਇੰਦਰਾਜ_ਰਸਤਾ,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/ਝੂਠ);
      sys::fs::closeFile(*FDOrErr);
      ਜੇ (MBOrErr) {
        ਜੋੜੋ_ਬਫਰ(Task, ModuleName, std::move(*MBOrErr));
        ਵਾਪਸ ਜੋੜੋ_ਧਾਰਾ_Fn();
      }
      EC = MBOrErr.getError();
    } ਨਹੀਂਤਾਂ {
      EC = error_ਮੰਜ਼ਿਲ_Error_ਕੋਡ(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    ਜੇ (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      ਵਾਪਸ createStringError(EC, Twine("Failed to open cache file ") +
                                       ਇੰਦਰਾਜ_ਰਸਤਾ + ": " + EC.ਸੁਨੇਹਾ_2() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    ਬਣਤਰ Cache_ਧਾਰਾ : Cached_File_ਧਾਰਾ {
      ਜੋੜੋ_ਬਫਰ_Fn ਜੋੜੋ_ਬਫਰ;
      sys::fs::ਆਰਜ਼ੀ_File ਆਰਜ਼ੀ_File;
      std::string ModuleName;
      ਬਿਨਾਨਿਸ਼ਾਨੀ Task;

      Cache_ਧਾਰਾ(std::unique_ptr<raw_pwrite_ਧਾਰਾ> OS, ਜੋੜੋ_ਬਫਰ_Fn ਜੋੜੋ_ਬਫਰ,
                  sys::fs::ਆਰਜ਼ੀ_File ਆਰਜ਼ੀ_File, std::string ਇੰਦਰਾਜ_ਰਸਤਾ,
                  std::string ModuleName, ਬਿਨਾਨਿਸ਼ਾਨੀ Task)
          : Cached_File_ਧਾਰਾ(std::move(OS), std::move(ਇੰਦਰਾਜ_ਰਸਤਾ)),
            ਜੋੜੋ_ਬਫਰ(std::move(ਜੋੜੋ_ਬਫਰ)), ਆਰਜ਼ੀ_File(std::move(ਆਰਜ਼ੀ_File)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = Cached_File_ਧਾਰਾ::commit();
        ਜੇ (E)
          ਵਾਪਸ E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> MBOrErr =
            ਸਮ੍ਰਿਤੀ_ਬਫਰ::getOpenFile(
                sys::fs::convertFDToNativeFile(ਆਰਜ਼ੀ_File.FD), Object_ਰਸਤਾ_Name,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/ਝੂਠ);
        ਜੇ (!MBOrErr) {
          std::error_ਕੋਡ EC = MBOrErr.getError();
          ਵਾਪਸ createStringError(EC, Twine("Failed to open new cache file ") +
                                           ਆਰਜ਼ੀ_File.TmpName + ": " +
                                           EC.ਸੁਨੇਹਾ_2() + "\n");
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
        E = ਆਰਜ਼ੀ_File.keep(Object_ਰਸਤਾ_Name);
        E = handleErrors(std::move(E), [&](ਅਚਲ ECError &E) -> Error {
          std::error_ਕੋਡ EC = E.convert_ਮੰਜ਼ਿਲ_Error_ਕੋਡ();
          ਜੇ (EC != errc::permission_denied)
            ਵਾਪਸ createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        ਆਰਜ਼ੀ_File.TmpName + " to " + Object_ਰਸਤਾ_Name + ": " +
                        EC.ਸੁਨੇਹਾ_2() + "\n");

          ਸਵੈਚਾਲਿਤ MBCopy = ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ_Copy((*MBOrErr)->get_ਬਫਰ(),
                                                       Object_ਰਸਤਾ_Name);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(ਆਰਜ਼ੀ_File.discard());

          ਵਾਪਸ Error::success();
        });

        ਜੇ (E)
          ਵਾਪਸ E;

        ਜੋੜੋ_ਬਫਰ(Task, ModuleName, std::move(*MBOrErr));
        ਵਾਪਸ Error::success();
      }

      Error abort() override {
        Error E1 = Cached_File_ਧਾਰਾ::abort();
        Error E2 = ਆਰਜ਼ੀ_File.discard();
        ਵਾਪਸ joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    ਬਣਤਰ Move_File_ਮੰਜ਼ਿਲ_Cache : Cached_File_ਧਾਰਾ {
      ਜੋੜੋ_ਬਫਰ_Fn ਜੋੜੋ_ਬਫਰ;
      std::string ModuleName;
      size_t Task;
      StringRef File_ਰਸਤਾ;

      Move_File_ਮੰਜ਼ਿਲ_Cache(ਜੋੜੋ_ਬਫਰ_Fn ਜੋੜੋ_ਬਫਰ, std::string ਇੰਦਰਾਜ_ਰਸਤਾ,
                      std::string ModuleID, size_t Task)
          : Cached_File_ਧਾਰਾ({}, std::move(ਇੰਦਰਾਜ_ਰਸਤਾ)),
            ਜੋੜੋ_ਬਫਰ(std::move(ਜੋੜੋ_ਬਫਰ)), ModuleName(ModuleID), Task(Task) {}
      ਆਭਾਸੀ ~Move_File_ਮੰਜ਼ਿਲ_Cache() = ਮੂਲ;

      ਆਭਾਸੀ Error commit(std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ> MemBuf) override {
        Error E = Cached_File_ਧਾਰਾ::commit();
        ਜੇ (E)
          ਵਾਪਸ E;

        File_ਰਸਤਾ = MemBuf->get_ਬਫਰ_Identifier();
        ਪ੍ਰਤਿਜ੍ਞਾ(!File_ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_ਕੋਡ EC = sys::fs::rename(File_ਰਸਤਾ, Object_ਰਸਤਾ_Name);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
        ਜੇ (EC ==
            std::error_ਕੋਡ(ERROR_NOT_SAME_DEVICE, std::system_category()))
#ਨਹੀਂਤਾਂ
        ਜੇ (EC == std::make_error_ਕੋਡ(std::errc::cross_device_link))
#ਜੇ_ਅੰਤ
          EC = sys::fs::copy_file(File_ਰਸਤਾ, Object_ਰਸਤਾ_Name);
        ਜੇ (EC)
          ਵਾਪਸ createStringError(EC, Twine("Failed to rename or copy file ") +
                                           File_ਰਸਤਾ + " to " + Object_ਰਸਤਾ_Name +
                                           ": " + EC.ਸੁਨੇਹਾ_2() + "\n");

        ਜੋੜੋ_ਬਫਰ(Task, ModuleName, std::move(MemBuf));

        ਵਾਪਸ Error::success();
      }
    };

    ਵਾਪਸ [=](size_t Task, ਅਚਲ Twine &ModuleName)
               -> Expected<std::unique_ptr<Cached_File_ਧਾਰਾ>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      ਜੇ (std::error_ਕੋਡ EC = sys::fs::create_directories(
              Cache_Directory_ਰਸਤਾ, /*IgnoreExisting=*/ਸੱਚ))
        ਵਾਪਸ createStringError(EC, Twine("can't create cache directory ") +
                                         Cache_Directory_ਰਸਤਾ + ": " +
                                         EC.ਸੁਨੇਹਾ_2());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      ਜੇ (CacheRename) {
        ਵਾਪਸ std::make_unique<Move_File_ਮੰਜ਼ਿਲ_Cache>(
            ਜੋੜੋ_ਬਫਰ, std::string(ਇੰਦਰਾਜ_ਰਸਤਾ.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> ਆਰਜ਼ੀ_Filename_Model;
      sys::ਰਸਤਾ_2::append(ਆਰਜ਼ੀ_Filename_Model, Cache_Directory_ਰਸਤਾ,
                        ਆਰਜ਼ੀ_File_ਅਗੇਤਰ + "-%%%%%%.tmp.o");
      Expected<sys::fs::ਆਰਜ਼ੀ_File> ਆਰਜ਼ੀ = sys::fs::ਆਰਜ਼ੀ_File::create(
          ਆਰਜ਼ੀ_Filename_Model, sys::fs::owner_read | sys::fs::owner_write);
      ਜੇ (!ਆਰਜ਼ੀ)
        ਵਾਪਸ createStringError(errc::io_error,
                                 ਮੰਜ਼ਿਲ_String(ਆਰਜ਼ੀ.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      ਵਾਪਸ std::make_unique<Cache_ਧਾਰਾ>(
          std::make_unique<raw_fd_ostream>(ਆਰਜ਼ੀ->FD, /* ShouldClose */ ਝੂਠ),
          ਜੋੜੋ_ਬਫਰ, std::move(*ਆਰਜ਼ੀ), std::string(ਇੰਦਰਾਜ_ਰਸਤਾ), ModuleName.str(),
          Task);
    };
  };
  ਵਾਪਸ FileCache(Func, Cache_Directory_ਰਸਤਾ_Ref.str());
}
