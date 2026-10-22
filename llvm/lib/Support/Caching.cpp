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

#అన్తర్భావ "llvm/Support/Caching.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"

#ఒకవేళ !defined(_MSC_VER) && !defined(__MINGW32__)
#అన్తర్భావ <unistd.h>
#లేకపోతే
#అన్తర్భావ "llvm/Support/Windows/WindowsSupport.h"
#అన్తర్భావ <io.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

Expected<FileCache> llvm::localCache(స్థిరరాశి Twine &CacheNameRef,
                                     స్థిరరాశి Twine &తాత్కాలిక_File_ఉపసర్గ_Ref,
                                     స్థిరరాశి Twine &Cache_Directory_మార్గం_Ref,
                                     కూడు_బఫరు_Fn కూడు_బఫరు, తార్కిక CacheRename) {

  // Create local copies which are safely captured-by-copy in lambdas
  SmallString<64> CacheName, తాత్కాలిక_File_ఉపసర్గ, Cache_Directory_మార్గం;
  CacheNameRef.గమ్యం_సదిశ(CacheName);
  తాత్కాలిక_File_ఉపసర్గ_Ref.గమ్యం_సదిశ(తాత్కాలిక_File_ఉపసర్గ);
  Cache_Directory_మార్గం_Ref.గమ్యం_సదిశ(Cache_Directory_మార్గం);

  స్వయంచాలక Func = [=](గుర్తులేని Task, StringRef కీలకం,
                  స్థిరరాశి Twine &ModuleName) -> Expected<కూడు_ప్రవాహం_Fn> {
    // This choice of file name allows the cache to be pruned (see pruneCache()
    // in include/llvm/Support/CachePruning.h).
    SmallString<64> నమోదు_మార్గం;
    sys::మార్గం_2::append(నమోదు_మార్గం, Cache_Directory_మార్గం, "llvmcache-" + కీలకం);
    // First, see if we have a cache hit.
    SmallString<64> ఫలితం_మార్గం;
    Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(
        Twine(నమోదు_మార్గం), sys::fs::యొక్క_Update_Atime, &ఫలితం_మార్గం);
    std::error_సంకేతం EC;
    ఒకవేళ (FDOrErr) {
      ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> MBOrErr =
          జ్ఞాపకం_బఫరు::getOpenFile(*FDOrErr, నమోదు_మార్గం,
                                    /*FileSize=*/-1,
                                    /*RequiresNullTerminator=*/అబద్ధం);
      sys::fs::closeFile(*FDOrErr);
      ఒకవేళ (MBOrErr) {
        కూడు_బఫరు(Task, ModuleName, std::move(*MBOrErr));
        తిరిగి కూడు_ప్రవాహం_Fn();
      }
      EC = MBOrErr.getError();
    } లేకపోతే {
      EC = error_గమ్యం_Error_సంకేతం(FDOrErr.takeError());
    }

    // On Windows we can fail to open a cache file with a permission denied
    // error. This generally means that another process has requested to delete
    // the file while it is still open, but it could also mean that another
    // process has opened the file without the sharing permissions we need.
    // Since the file is probably being deleted we handle it in the same way as
    // if the file did not exist at all.
    ఒకవేళ (EC != errc::no_such_file_or_directory && EC != errc::permission_denied)
      తిరిగి createStringError(EC, Twine("Failed to open cache file ") +
                                       నమోదు_మార్గం + ": " + EC.సందేశం_2() + "\n");

    // This file stream is responsible for commiting the resulting file to the
    // cache and calling AddBuffer to add it to the link.
    నిర్మాణం Cache_ప్రవాహం : Cached_File_ప్రవాహం {
      కూడు_బఫరు_Fn కూడు_బఫరు;
      sys::fs::తాత్కాలిక_File తాత్కాలిక_File;
      std::string ModuleName;
      గుర్తులేని Task;

      Cache_ప్రవాహం(std::unique_ptr<raw_pwrite_ప్రవాహం> OS, కూడు_బఫరు_Fn కూడు_బఫరు,
                  sys::fs::తాత్కాలిక_File తాత్కాలిక_File, std::string నమోదు_మార్గం,
                  std::string ModuleName, గుర్తులేని Task)
          : Cached_File_ప్రవాహం(std::move(OS), std::move(నమోదు_మార్గం)),
            కూడు_బఫరు(std::move(కూడు_బఫరు)), తాత్కాలిక_File(std::move(తాత్కాలిక_File)),
            ModuleName(ModuleName), Task(Task) {}

      Error commit() override {
        Error E = Cached_File_ప్రవాహం::commit();
        ఒకవేళ (E)
          తిరిగి E;

        // Make sure the stream is closed before committing it.
        OS.reset();

        // Open the file first to avoid racing with a cache pruner.
        ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> MBOrErr =
            జ్ఞాపకం_బఫరు::getOpenFile(
                sys::fs::convertFDToNativeFile(తాత్కాలిక_File.FD), Object_మార్గం_Name,
                /*FileSize=*/-1, /*RequiresNullTerminator=*/అబద్ధం);
        ఒకవేళ (!MBOrErr) {
          std::error_సంకేతం EC = MBOrErr.getError();
          తిరిగి createStringError(EC, Twine("Failed to open new cache file ") +
                                           తాత్కాలిక_File.TmpName + ": " +
                                           EC.సందేశం_2() + "\n");
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
        E = తాత్కాలిక_File.keep(Object_మార్గం_Name);
        E = handleErrors(std::move(E), [&](స్థిరరాశి ECError &E) -> Error {
          std::error_సంకేతం EC = E.convert_గమ్యం_Error_సంకేతం();
          ఒకవేళ (EC != errc::permission_denied)
            తిరిగి createStringError(
                EC, Twine("Failed to rename temporary file ") +
                        తాత్కాలిక_File.TmpName + " to " + Object_మార్గం_Name + ": " +
                        EC.సందేశం_2() + "\n");

          స్వయంచాలక MBCopy = జ్ఞాపకం_బఫరు::get_Mem_బఫరు_Copy((*MBOrErr)->get_బఫరు(),
                                                       Object_మార్గం_Name);
          MBOrErr = std::move(MBCopy);

          // FIXME: should we consume the discard error?
          consumeError(తాత్కాలిక_File.discard());

          తిరిగి Error::success();
        });

        ఒకవేళ (E)
          తిరిగి E;

        కూడు_బఫరు(Task, ModuleName, std::move(*MBOrErr));
        తిరిగి Error::success();
      }

      Error abort() override {
        Error E1 = Cached_File_ప్రవాహం::abort();
        Error E2 = తాత్కాలిక_File.discard();
        తిరిగి joinErrors(std::move(E1), std::move(E2));
      }
    };

    // This class is responsible for renaming/moving existing file into a
    // cache directory. The path for an input file is passed through a string
    // stream.
    నిర్మాణం Move_File_గమ్యం_Cache : Cached_File_ప్రవాహం {
      కూడు_బఫరు_Fn కూడు_బఫరు;
      std::string ModuleName;
      size_t Task;
      StringRef File_మార్గం;

      Move_File_గమ్యం_Cache(కూడు_బఫరు_Fn కూడు_బఫరు, std::string నమోదు_మార్గం,
                      std::string ModuleID, size_t Task)
          : Cached_File_ప్రవాహం({}, std::move(నమోదు_మార్గం)),
            కూడు_బఫరు(std::move(కూడు_బఫరు)), ModuleName(ModuleID), Task(Task) {}
      మాయా ~Move_File_గమ్యం_Cache() = అప్రమేయ;

      మాయా Error commit(std::unique_ptr<జ్ఞాపకం_బఫరు> MemBuf) override {
        Error E = Cached_File_ప్రవాహం::commit();
        ఒకవేళ (E)
          తిరిగి E;

        File_మార్గం = MemBuf->get_బఫరు_Identifier();
        ప్రతిజ్ఞా(!File_మార్గం.ఖాళీలు() && "File path is empty.");

        // Rename/move native object file into cache directory, if they are
        // located the same device/logical drive, otherwise we use a copy.
        std::error_సంకేతం EC = sys::fs::rename(File_మార్గం, Object_మార్గం_Name);
#నిర్వచితమైతే _WIN32
        ఒకవేళ (EC ==
            std::error_సంకేతం(ERROR_NOT_SAME_DEVICE, std::system_category()))
#లేకపోతే
        ఒకవేళ (EC == std::make_error_సంకేతం(std::errc::cross_device_link))
#అయితే_ముగింపు
          EC = sys::fs::copy_file(File_మార్గం, Object_మార్గం_Name);
        ఒకవేళ (EC)
          తిరిగి createStringError(EC, Twine("Failed to rename or copy file ") +
                                           File_మార్గం + " to " + Object_మార్గం_Name +
                                           ": " + EC.సందేశం_2() + "\n");

        కూడు_బఫరు(Task, ModuleName, std::move(MemBuf));

        తిరిగి Error::success();
      }
    };

    తిరిగి [=](size_t Task, స్థిరరాశి Twine &ModuleName)
               -> Expected<std::unique_ptr<Cached_File_ప్రవాహం>> {
      // Create the cache directory if not already done. Doing this lazily
      // ensures the filesystem isn't mutated until the cache is.
      ఒకవేళ (std::error_సంకేతం EC = sys::fs::create_directories(
              Cache_Directory_మార్గం, /*IgnoreExisting=*/నిజం))
        తిరిగి createStringError(EC, Twine("can't create cache directory ") +
                                         Cache_Directory_మార్గం + ": " +
                                         EC.సందేశం_2());
      // MoveFileToChache class will rename/move the file into the cache on
      // destruction.
      ఒకవేళ (CacheRename) {
        తిరిగి std::make_unique<Move_File_గమ్యం_Cache>(
            కూడు_బఫరు, std::string(నమోదు_మార్గం.str()), ModuleName.str(), Task);
      }

      // Write to a temporary to avoid race condition
      SmallString<64> తాత్కాలిక_Filename_Model;
      sys::మార్గం_2::append(తాత్కాలిక_Filename_Model, Cache_Directory_మార్గం,
                        తాత్కాలిక_File_ఉపసర్గ + "-%%%%%%.tmp.o");
      Expected<sys::fs::తాత్కాలిక_File> తాత్కాలిక = sys::fs::తాత్కాలిక_File::create(
          తాత్కాలిక_Filename_Model, sys::fs::owner_read | sys::fs::owner_write);
      ఒకవేళ (!తాత్కాలిక)
        తిరిగి createStringError(errc::io_error,
                                 గమ్యం_String(తాత్కాలిక.takeError()) + ": " + CacheName +
                                     ": Can't get a temporary file");

      // This CacheStream will move the temporary file into the cache when done.
      తిరిగి std::make_unique<Cache_ప్రవాహం>(
          std::make_unique<raw_fd_ostream>(తాత్కాలిక->FD, /* ShouldClose */ అబద్ధం),
          కూడు_బఫరు, std::move(*తాత్కాలిక), std::string(నమోదు_మార్గం), ModuleName.str(),
          Task);
    };
  };
  తిరిగి FileCache(Func, Cache_Directory_మార్గం_Ref.str());
}
