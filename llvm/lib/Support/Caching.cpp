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

#समावेश "llvm/Support/Caching.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"

#अगर !defined(_MSC_VER) && !defined(__MINGW32__)
#समावेश <unistd.h>
#अन्यथा
#समावेश "llvm/Support/Windows/WindowsSupport.h"
#समावेश <io.h>
#पूर्ण_अगर

using namespace llvm;

Expected<FileCache> llvm::localCache(स्थिर Twine &CacheNameRef,
                                     स्थिर Twine &अस्थायी_File_Prefix_Ref,
                                     स्थिर Twine &CacheDirectoryPathRef,
                                     Add_बफर_Fn Add_बफर, bool CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, अस्थायी_File_Prefix, CacheDirectoryPath;
  CacheNameRef.toVector(CacheName);
  अस्थायी_File_Prefix_Ref.toVector(अस्थायी_File_Prefix);
  CacheDirectoryPathRef.toVector(CacheDirectoryPath);

  स्वतः Func = [=](अचिन्हित Task, StringRef कुंजी,
                  स्थिर Twine &ModuleName) -> Expected<AddStreamFn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> EntryPath;
    sys::path::append(EntryPath, CacheDirectoryPath, "llvmcache-" + कुंजी);
    // First, see if we have a cache hit.
    SmallString<64> परिणाम_Path;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(EntryPath), sys::fs::OF_UpdateAtime, &परिणाम_Path);
    std::error_code EC;
    अगर (FDOrErr) {
      ErrorOr<std::unique_ptr<Memory_बफर>> MBOrErr =
          Memory_बफर::getOpenFile(*FDOrErr, EntryPath,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/false);
      sys::fs::closeFile(*FDOrErr);
      अगर (MBOrErr) {
        Add_बफर(Task, ModuleName, std::move(*MBOrErr));
        वापस AddStreamFn();
      }
      EC = MBOrErr.getError();
    } अन्यथा {
      EC = errorToErrorCode(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    अगर (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      वापस createStringError(EC, Twine("Failed to open cache file ") +
                                       EntryPath + ": " + EC.message() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    काष्ठा CacheStream : CachedFileStream {
      Add_बफर_Fn Add_बफर;
      sys::fs::अस्थायी_File अस्थायी_File;
      std::string ModuleName;
      अचिन्हित Task;

      CacheStream(std::unique_ptr<raw_pwrite_stream> OS, Add_बफर_Fn Add_बफर,
                  sys::fs::अस्थायी_File अस्थायी_File, std::string EntryPath,
                  std::string ModuleName, अचिन्हित Task)
          : CachedFileStream(std::move(OS), std::move(EntryPath)),
            Add_बफर(std::move(Add_बफर)), अस्थायी_File(std::move(अस्थायी_File)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = CachedFileStream::commit();
        अगर (E)
          वापस E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<Memory_बफर>> MBOrErr =
            Memory_बफर::getOpenFile(
                sys::fs::convertFDToNativeFile(अस्थायी_File.FD), ObjectPathName,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/false);
        अगर (!MBOrErr) {
          std::error_code EC = MBOrErr.getError();
          वापस createStringError(EC, Twine("Failed to open new cache file ") +
                                           अस्थायी_File.TmpName + ": " +
                                           EC.message() + "\n");
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
        E = अस्थायी_File.keep(ObjectPathName);
        E = handleErrors(std::move(E), [&](स्थिर ECError &E) -> Error {
          std::error_code EC = E.convertToErrorCode();
          अगर (EC != errc::permission_denied)
            वापस createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        अस्थायी_File.TmpName + " to " + ObjectPathName + ": " +
                        EC.message() + "\n");

          स्वतः MBCopy = Memory_बफर::get_Mem_बफर_Copy((*MBOrErr)->get_बफर(),
                                                       ObjectPathName);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(अस्थायी_File.discard());

          वापस Error::success();
        });

        अगर (E)
          वापस E;

        Add_बफर(Task, ModuleName, std::move(*MBOrErr));
        वापस Error::success();
      }

      Error पात() override {
        Error E1 = CachedFileStream::पात();
        Error E2 = अस्थायी_File.discard();
        वापस joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    काष्ठा MoveFileToCache : CachedFileStream {
      Add_बफर_Fn Add_बफर;
      std::string ModuleName;
      माप_प्रकार Task;
      StringRef FilePath;

      MoveFileToCache(Add_बफर_Fn Add_बफर, std::string EntryPath,
                      std::string ModuleID, माप_प्रकार Task)
          : CachedFileStream({}, std::move(EntryPath)),
            Add_बफर(std::move(Add_बफर)), ModuleName(ModuleID), Task(Task) {}
      भव ~MoveFileToCache() = शेष;

      भव Error commit(std::unique_ptr<Memory_बफर> MemBuf) override {
        Error E = CachedFileStream::commit();
        अगर (E)
          वापस E;

        FilePath = MemBuf->get_बफर_Identifier();
        निश्चित(!FilePath.empty() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_code EC = sys::fs::नाम(FilePath, ObjectPathName);
#अगर_घोषित _WIN32
        अगर (EC ==
            std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category()))
#अन्यथा
        अगर (EC == std::make_error_code(std::errc::cross_device_link))
#पूर्ण_अगर
          EC = sys::fs::copy_file(FilePath, ObjectPathName);
        अगर (EC)
          वापस createStringError(EC, Twine("Failed to rename or copy file ") +
                                           FilePath + " to " + ObjectPathName +
                                           ": " + EC.message() + "\n");

        Add_बफर(Task, ModuleName, std::move(MemBuf));

        वापस Error::success();
      }
    };

    वापस [=](माप_प्रकार Task, स्थिर Twine &ModuleName)
               -> Expected<std::unique_ptr<CachedFileStream>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      अगर (std::error_code EC = sys::fs::create_directories(
              CacheDirectoryPath, /*IgnoreExisting=*/true))
        वापस createStringError(EC, Twine("can't create cache directory ") +
                                         CacheDirectoryPath + ": " +
                                         EC.message());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      अगर (CacheRename) {
        वापस std::make_unique<MoveFileToCache>(
            Add_बफर, std::string(EntryPath.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> अस्थायी_Filename_Model;
      sys::path::append(अस्थायी_Filename_Model, CacheDirectoryPath,
                        अस्थायी_File_Prefix + "-%%%%%%.tmp.o");
      Expected<sys::fs::अस्थायी_File> अस्थायी = sys::fs::अस्थायी_File::create(
          अस्थायी_Filename_Model, sys::fs::owner_read | sys::fs::owner_write);
      अगर (!अस्थायी)
        वापस createStringError(errc::io_error,
                                 toString(अस्थायी.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      वापस std::make_unique<CacheStream>(
          std::make_unique<raw_fd_ostream>(अस्थायी->FD, /* ShouldClose */ false),
          Add_बफर, std::move(*अस्थायी), std::string(EntryPath), ModuleName.str(),
          Task);
    };
  };
  वापस FileCache(Func, CacheDirectoryPathRef.str());
}
