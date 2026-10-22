//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/FileCollector.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Process.h"

using فضاینام llvm;

FileCollectorBase::FileCollectorBase() = پیشفرض;
FileCollectorBase::~FileCollectorBase() = پیشفرض;

تهی FileCollectorBase::addFile(ثابت Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  اگر (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

تهی FileCollectorBase::addDirectory(ثابت Twine &Dir) {
  أكد(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

ایستا منطقی isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  اگر (sys::fs::real_path(Path, TmpDest))
    بازگشت درست; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  اگر (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    بازگشت نادرست;
  بازگشت درست;
}

FileCollector::FileCollector(std::string ریشه_2, std::string Overlay_ریشه,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : ریشه_2(ریشه_2), Overlay_ریشه(Overlay_ریشه), Canonicalizer(std::move(VFS)) {
  أكد(sys::path::is_absolute(ریشه_2) && "Root not absolute");
  أكد(sys::path::is_absolute(Overlay_ریشه) && "OverlayRoot not absolute");
}

تهی FileCollector::PathCanonicalizer::updateWithRealPath(
    SmallVectorImpl<نویسه> &Path) {
  StringRef SrcPath(Path.begin(), Path.size());
  StringRef Filename = sys::path::filename(SrcPath);
  StringRef Directory = sys::path::parent_path(SrcPath);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> RealPath;
  auto DirWithSymlink = CachedDirs.find(Directory);
  اگر (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    اگر (VFS->getRealPath(Directory, RealPath))
      بازگشت;
    CachedDirs[Directory] = std::string(RealPath);
  } وگرنه {
    RealPath = DirWithSymlink->second;
  }

  // Finish recreating the path by appending the original filename, since we
  // don't need to resolve symlinks in the filename.
  //
  // FIXME: If we can cope with this, maybe we can cope without calling
  // getRealPath() at all when there's no ".." component.
  sys::path::append(RealPath, Filename);

  // Swap to create the output.
  Path.swap(RealPath);
}

/// Make Path absolute.
ایستا تهی makeAbsolute(vfs::FileSystem &VFS, SmallVectorImpl<نویسه> &Path) {
  // We need an absolute src path to append to the root.
  VFS.makeAbsolute(Path);

  // Canonicalize src to a native path to avoid mixed separator styles.
  sys::path::native(Path);

  // Remove redundant leading "./" pieces and consecutive separators.
  Path.erase(Path.begin(), sys::path::remove_leading_dotslash(
                               StringRef(Path.begin(), Path.size()))
                               .begin());
}

FileCollector::PathCanonicalizer::PathStorage
FileCollector::PathCanonicalizer::canonicalize(StringRef SrcPath) {
  PathStorage Paths;
  Paths.VirtualPath = SrcPath;
  makeAbsolute(*VFS, Paths.VirtualPath);

  // If a ".." component is present after a symlink component, remove_dots may
  // lead to the wrong real destination path. Let the source be canonicalized
  // like that but make sure we always use the real path for the destination.
  Paths.CopyFrom = Paths.VirtualPath;
  updateWithRealPath(Paths.CopyFrom);

  // Canonicalize the virtual path by removing "..", "." components.
  sys::path::remove_dots(Paths.VirtualPath, /*remove_dot_dot=*/درست);

  بازگشت Paths;
}

تهی FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(ریشه_2);
  sys::path::append(DstPath, sys::path::relative_path(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  addFileToMapping(Paths.VirtualPath, DstPath);
}

llvm::vfs::directory_iterator
FileCollector::addDirectoryImpl(ثابت llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_code &EC) {
  auto It = FS->dir_begin(Dir, EC);
  اگر (EC)
    بازگشت It;
  addFile(Dir);
  برای (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    اگر (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  اگر (EC)
    بازگشت It;
  // Return a new iterator.
  بازگشت FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
ایستا std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              ثابت sys::fs::file_status &Stat) {
  صحیح FD;

  اگر (auto EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    بازگشت EC;

  اگر (auto EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    بازگشت EC;

  اگر (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    بازگشت EC;

  بازگشت {};
}

std::error_code FileCollector::copyFiles(منطقی StopOnError) {
  auto Err = sys::fs::create_directories(ریشه_2, /*IgnoreExisting=*/درست);
  اگر (Err) {
    بازگشت Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  برای (auto &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    اگر (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      اگر (StopOnError)
        بازگشت EC;
      ادامه;
    }

    // Continue if the file doesn't exist.
    اگر (Stat.type() == sys::fs::file_type::file_not_found)
      ادامه;

    // Create directory tree.
    اگر (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/درست)) {
      اگر (StopOnError)
        بازگشت EC;
    }

    اگر (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      اگر (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/درست)) {
        اگر (StopOnError)
          بازگشت EC;
      }
      ادامه;
    }

    // Copy file over.
    اگر (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      اگر (StopOnError)
        بازگشت EC;
    }

    // Copy over permissions.
    اگر (auto perms = sys::fs::getPermissions(entry.VPath)) {
      اگر (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        اگر (StopOnError)
          بازگشت EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  بازگشت {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(Overlay_ریشه);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(Overlay_ریشه));
  VFSWriter.setUseExternalNames(نادرست);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  اگر (EC)
    بازگشت EC;

  VFSWriter.اكتب(os);

  بازگشت {};
}

فضاینام llvm {

رده FileCollectorFileSystem : عمومی vfs::FileSystem {
عمومی:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(ثابت Twine &Path) override {
    auto Result = FS->status(Path);
    اگر (Result && Result->exists())
      Collector->addFile(Path);
    بازگشت Result;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ثابت Twine &Path) override {
    auto Result = FS->openFileForRead(Path);
    اگر (Result && *Result)
      Collector->addFile(Path);
    بازگشت Result;
  }

  llvm::vfs::directory_iterator dir_begin(ثابت llvm::Twine &Dir,
                                          std::error_code &EC) override {
    بازگشت Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(ثابت Twine &Path,
                              SmallVectorImpl<نویسه> &Output) override {
    auto EC = FS->getRealPath(Path, Output);
    اگر (!EC) {
      Collector->addFile(Path);
      اگر (Output.size() > 0)
        Collector->addFile(Output);
    }
    بازگشت EC;
  }

  تهی
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(ثابت Twine &Path, منطقی &Result) override {
    بازگشت FS->isLocal(Path, Result);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override {
    بازگشت FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(ثابت llvm::Twine &Path) override {
    بازگشت FS->setCurrentWorkingDirectory(Path);
  }

خصوصی:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  بازگشت makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
