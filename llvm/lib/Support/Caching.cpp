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
#وگرنه
#شامل "llvm/Support/Windows/WindowsSupport.h"
#شامل <io.h>
#endif

using فضاینام llvm;

Expected<FileCache> llvm::localCache(ثابت Twine &CacheNameRef,
                                     ثابت Twine &TempFilePrefixRef,
                                     ثابت Twine &CacheDirectoryPathRef,
                                     AddBufferFn AddBuffer, منطقی CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, TempFilePrefix, CacheDirectoryPath;
  CacheNameRef.toVector(CacheName);
  TempFilePrefixRef.toVector(TempFilePrefix);
  CacheDirectoryPathRef.toVector(CacheDirectoryPath);

  auto Func = [=](unsigned Task, StringRef کلید,
                  ثابت Twine &ModuleName) -> Expected<AddStreamFn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> EntryPath;
    sys::path::append(EntryPath, CacheDirectoryPath, "llvmcache-" + کلید);
    // First, see if we have a cache hit.
    SmallString<64> ResultPath;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(EntryPath), sys::fs::OF_UpdateAtime, &ResultPath);
    std::error_code EC;
    اگر (FDOrErr) {
      ErrorOr<std::unique_ptr<MemoryBuffer>> MBOrErr =
          MemoryBuffer::getOpenFile(*FDOrErr, EntryPath,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/نادرست);
      sys::fs::closeFile(*FDOrErr);
      اگر (MBOrErr) {
        AddBuffer(Task, ModuleName, std::move(*MBOrErr));
        بازگشت AddStreamFn();
      }
      EC = MBOrErr.getError();
    } وگرنه {
      EC = errorToErrorCode(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    اگر (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      بازگشت createStringError(EC, Twine("Failed to open cache file ") +
                                       EntryPath + ": " + EC.message() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    ساختار CacheStream : CachedFileStream {
      AddBufferFn AddBuffer;
      sys::fs::TempFile TempFile;
      std::string ModuleName;
      unsigned Task;

      CacheStream(std::unique_ptr<raw_pwrite_stream> OS, AddBufferFn AddBuffer,
                  sys::fs::TempFile TempFile, std::string EntryPath,
                  std::string ModuleName, unsigned Task)
          : CachedFileStream(std::move(OS), std::move(EntryPath)),
            AddBuffer(std::move(AddBuffer)), TempFile(std::move(TempFile)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = CachedFileStream::commit();
        اگر (E)
          بازگشت E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<MemoryBuffer>> MBOrErr =
            MemoryBuffer::getOpenFile(
                sys::fs::convertFDToNativeFile(TempFile.FD), ObjectPathName,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/نادرست);
        اگر (!MBOrErr) {
          std::error_code EC = MBOrErr.getError();
          بازگشت createStringError(EC, Twine("Failed to open new cache file ") +
                                           TempFile.TmpName + ": " +
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
        E = TempFile.keep(ObjectPathName);
        E = handleErrors(std::move(E), [&](ثابت ECError &E) -> Error {
          std::error_code EC = E.convertToErrorCode();
          اگر (EC != errc::permission_denied)
            بازگشت createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        TempFile.TmpName + " to " + ObjectPathName + ": " +
                        EC.message() + "\n");

          auto MBCopy = MemoryBuffer::getMemBufferCopy((*MBOrErr)->getBuffer(),
                                                       ObjectPathName);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(TempFile.discard());

          بازگشت Error::success();
        });

        اگر (E)
          بازگشت E;

        AddBuffer(Task, ModuleName, std::move(*MBOrErr));
        بازگشت Error::success();
      }

      Error abort() override {
        Error E1 = CachedFileStream::abort();
        Error E2 = TempFile.discard();
        بازگشت joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    ساختار MoveFileToCache : CachedFileStream {
      AddBufferFn AddBuffer;
      std::string ModuleName;
      size_t Task;
      StringRef FilePath;

      MoveFileToCache(AddBufferFn AddBuffer, std::string EntryPath,
                      std::string ModuleID, size_t Task)
          : CachedFileStream({}, std::move(EntryPath)),
            AddBuffer(std::move(AddBuffer)), ModuleName(ModuleID), Task(Task) {}
      مجازی ~MoveFileToCache() = پیشفرض;

      مجازی Error commit(std::unique_ptr<MemoryBuffer> MemBuf) override {
        Error E = CachedFileStream::commit();
        اگر (E)
          بازگشت E;

        FilePath = MemBuf->getBufferIdentifier();
        أكد(!FilePath.empty() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_code EC = sys::fs::rename(FilePath, ObjectPathName);
#ifdef _WIN32
        اگر (EC ==
            std::error_code(ERROR_NOT_SAME_DEVICE, std::system_category()))
#وگرنه
        اگر (EC == std::make_error_code(std::errc::cross_device_link))
#endif
          EC = sys::fs::copy_file(FilePath, ObjectPathName);
        اگر (EC)
          بازگشت createStringError(EC, Twine("Failed to rename or copy file ") +
                                           FilePath + " to " + ObjectPathName +
                                           ": " + EC.message() + "\n");

        AddBuffer(Task, ModuleName, std::move(MemBuf));

        بازگشت Error::success();
      }
    };

    بازگشت [=](size_t Task, ثابت Twine &ModuleName)
               -> Expected<std::unique_ptr<CachedFileStream>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      اگر (std::error_code EC = sys::fs::create_directories(
              CacheDirectoryPath, /*IgnoreExisting=*/درست))
        بازگشت createStringError(EC, Twine("can't create cache directory ") +
                                         CacheDirectoryPath + ": " +
                                         EC.message());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      اگر (CacheRename) {
        بازگشت std::make_unique<MoveFileToCache>(
            AddBuffer, std::string(EntryPath.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> TempFilenameModel;
      sys::path::append(TempFilenameModel, CacheDirectoryPath,
                        TempFilePrefix + "-%%%%%%.tmp.o");
      Expected<sys::fs::TempFile> Temp = sys::fs::TempFile::create(
          TempFilenameModel, sys::fs::owner_read | sys::fs::owner_write);
      اگر (!Temp)
        بازگشت createStringError(errc::io_error,
                                 toString(Temp.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      بازگشت std::make_unique<CacheStream>(
          std::make_unique<raw_fd_ostream>(Temp->FD, /* ShouldClose */ نادرست),
          AddBuffer, std::move(*Temp), std::string(EntryPath), ModuleName.str(),
          Task);
    };
  };
  بازگشت FileCache(Func, CacheDirectoryPathRef.str());
}
