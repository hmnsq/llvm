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
#پایان_اگر

استفاده نطاق llvm;

Expected<FileCache> llvm::localCache(ثابت Twine &CacheNameRef,
                                     ثابت Twine &موقت_File_پیشوند_Ref,
                                     ثابت Twine &Cache_Directory_مسیر_Ref,
                                     جمع_کن_میانگیر_Fn جمع_کن_میانگیر, منطقي CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, موقت_File_پیشوند, Cache_Directory_مسیر;
  CacheNameRef.مقصد_وکتور(CacheName);
  موقت_File_پیشوند_Ref.مقصد_وکتور(موقت_File_پیشوند);
  Cache_Directory_مسیر_Ref.مقصد_وکتور(Cache_Directory_مسیر);

  خودکار Func = [=](بیعلامت Task, StringRef کلید,
                  ثابت Twine &ModuleName) -> Expected<جمع_کن_جریان_Fn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> مدخل_مسیر;
    sys::مسیر_2::append(مدخل_مسیر, Cache_Directory_مسیر, "llvmcache-" + کلید);
    // First, see if we have a cache hit.
    SmallString<64> نتیجه_مسیر;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(مدخل_مسیر), sys::fs::از_Update_Atime, &نتیجه_مسیر);
    std::error_کد EC;
    إذا (FDOrErr) {
      ErrorOr<std::unique_ptr<حافظه_میانگیر>> MBOrErr =
          حافظه_میانگیر::getOpenFile(*FDOrErr, مدخل_مسیر,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/خطأ);
      sys::fs::closeFile(*FDOrErr);
      إذا (MBOrErr) {
        جمع_کن_میانگیر(Task, ModuleName, std::move(*MBOrErr));
        أرجع جمع_کن_جریان_Fn();
      }
      EC = MBOrErr.getError();
    } وإلا {
      EC = error_مقصد_Error_کد(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    إذا (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      أرجع createStringError(EC, Twine("Failed to open cache file ") +
                                       مدخل_مسیر + ": " + EC.پیام_2() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    بنية Cache_جریان : Cached_File_جریان {
      جمع_کن_میانگیر_Fn جمع_کن_میانگیر;
      sys::fs::موقت_File موقت_File;
      std::string ModuleName;
      بیعلامت Task;

      Cache_جریان(std::unique_ptr<raw_pwrite_جریان> OS, جمع_کن_میانگیر_Fn جمع_کن_میانگیر,
                  sys::fs::موقت_File موقت_File, std::string مدخل_مسیر,
                  std::string ModuleName, بیعلامت Task)
          : Cached_File_جریان(std::move(OS), std::move(مدخل_مسیر)),
            جمع_کن_میانگیر(std::move(جمع_کن_میانگیر)), موقت_File(std::move(موقت_File)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = Cached_File_جریان::commit();
        إذا (E)
          أرجع E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<حافظه_میانگیر>> MBOrErr =
            حافظه_میانگیر::getOpenFile(
                sys::fs::convertFDToNativeFile(موقت_File.FD), Object_مسیر_Name,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/خطأ);
        إذا (!MBOrErr) {
          std::error_کد EC = MBOrErr.getError();
          أرجع createStringError(EC, Twine("Failed to open new cache file ") +
                                           موقت_File.TmpName + ": " +
                                           EC.پیام_2() + "\n");
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
        E = موقت_File.keep(Object_مسیر_Name);
        E = handleErrors(std::move(E), [&](ثابت ECError &E) -> Error {
          std::error_کد EC = E.convert_مقصد_Error_کد();
          إذا (EC != errc::permission_denied)
            أرجع createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        موقت_File.TmpName + " to " + Object_مسیر_Name + ": " +
                        EC.پیام_2() + "\n");

          خودکار MBCopy = حافظه_میانگیر::get_Mem_میانگیر_Copy((*MBOrErr)->get_میانگیر(),
                                                       Object_مسیر_Name);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(موقت_File.discard());

          أرجع Error::success();
        });

        إذا (E)
          أرجع E;

        جمع_کن_میانگیر(Task, ModuleName, std::move(*MBOrErr));
        أرجع Error::success();
      }

      Error abort() override {
        Error E1 = Cached_File_جریان::abort();
        Error E2 = موقت_File.discard();
        أرجع joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    بنية Move_File_مقصد_Cache : Cached_File_جریان {
      جمع_کن_میانگیر_Fn جمع_کن_میانگیر;
      std::string ModuleName;
      size_t Task;
      StringRef File_مسیر;

      Move_File_مقصد_Cache(جمع_کن_میانگیر_Fn جمع_کن_میانگیر, std::string مدخل_مسیر,
                      std::string ModuleID, size_t Task)
          : Cached_File_جریان({}, std::move(مدخل_مسیر)),
            جمع_کن_میانگیر(std::move(جمع_کن_میانگیر)), ModuleName(ModuleID), Task(Task) {}
      مجازی ~Move_File_مقصد_Cache() = افتراضي;

      مجازی Error commit(std::unique_ptr<حافظه_میانگیر> MemBuf) override {
        Error E = Cached_File_جریان::commit();
        إذا (E)
          أرجع E;

        File_مسیر = MemBuf->get_میانگیر_Identifier();
        أكد(!File_مسیر.خالی() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_کد EC = sys::fs::rename(File_مسیر, Object_مسیر_Name);
#اگر_تعریف_شده _WIN32
        إذا (EC ==
            std::error_کد(ERROR_NOT_SAME_DEVICE, std::system_category()))
#وإلا
        إذا (EC == std::make_error_کد(std::errc::cross_device_link))
#پایان_اگر
          EC = sys::fs::copy_file(File_مسیر, Object_مسیر_Name);
        إذا (EC)
          أرجع createStringError(EC, Twine("Failed to rename or copy file ") +
                                           File_مسیر + " to " + Object_مسیر_Name +
                                           ": " + EC.پیام_2() + "\n");

        جمع_کن_میانگیر(Task, ModuleName, std::move(MemBuf));

        أرجع Error::success();
      }
    };

    أرجع [=](size_t Task, ثابت Twine &ModuleName)
               -> Expected<std::unique_ptr<Cached_File_جریان>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      إذا (std::error_کد EC = sys::fs::create_directories(
              Cache_Directory_مسیر, /*IgnoreExisting=*/درست))
        أرجع createStringError(EC, Twine("can't create cache directory ") +
                                         Cache_Directory_مسیر + ": " +
                                         EC.پیام_2());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      إذا (CacheRename) {
        أرجع std::make_unique<Move_File_مقصد_Cache>(
            جمع_کن_میانگیر, std::string(مدخل_مسیر.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> موقت_Filename_Model;
      sys::مسیر_2::append(موقت_Filename_Model, Cache_Directory_مسیر,
                        موقت_File_پیشوند + "-%%%%%%.tmp.o");
      Expected<sys::fs::موقت_File> موقت = sys::fs::موقت_File::create(
          موقت_Filename_Model, sys::fs::owner_read | sys::fs::owner_write);
      إذا (!موقت)
        أرجع createStringError(errc::io_error,
                                 مقصد_String(موقت.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      أرجع std::make_unique<Cache_جریان>(
          std::make_unique<raw_fd_ostream>(موقت->FD, /* ShouldClose */ خطأ),
          جمع_کن_میانگیر, std::move(*موقت), std::string(مدخل_مسیر), ModuleName.str(),
          Task);
    };
  };
  أرجع FileCache(Func, Cache_Directory_مسیر_Ref.str());
}
