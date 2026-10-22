//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/FileCollector.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Process.h"

using نطاق llvm;

FileCollectorBase::FileCollectorBase() = افتراضي;
FileCollectorBase::~FileCollectorBase() = افتراضي;

فراغ FileCollectorBase::addFile(ثابت Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  إذا (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

فراغ FileCollectorBase::addDirectory(ثابت Twine &Dir) {
  أكد(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

ساكن منطقي isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  إذا (sys::fs::real_path(Path, TmpDest))
    أرجع true; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  إذا (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    أرجع خطأ;
  أرجع true;
}

FileCollector::FileCollector(std::string ڕەگ_2, std::string Overlay_ڕەگ,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : ڕەگ_2(ڕەگ_2), Overlay_ڕەگ(Overlay_ڕەگ), Canonicalizer(std::move(VFS)) {
  أكد(sys::path::is_absolute(ڕەگ_2) && "Root not absolute");
  أكد(sys::path::is_absolute(Overlay_ڕەگ) && "OverlayRoot not absolute");
}

فراغ FileCollector::PathCanonicalizer::updateWithRealPath(
    SmallVectorImpl<حرف> &Path) {
  StringRef SrcPath(Path.begin(), Path.size());
  StringRef Filename = sys::path::filename(SrcPath);
  StringRef Directory = sys::path::parent_path(SrcPath);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> RealPath;
  auto DirWithSymlink = CachedDirs.find(Directory);
  إذا (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    إذا (VFS->getRealPath(Directory, RealPath))
      أرجع;
    CachedDirs[Directory] = std::string(RealPath);
  } وإلا {
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
ساكن فراغ makeAbsolute(vfs::FileSystem &VFS, SmallVectorImpl<حرف> &Path) {
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
  sys::path::remove_dots(Paths.VirtualPath, /*remove_dot_dot=*/true);

  أرجع Paths;
}

فراغ FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(ڕەگ_2);
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
  إذا (EC)
    أرجع It;
  addFile(Dir);
  لكل (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    إذا (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  إذا (EC)
    أرجع It;
  // Return a new iterator.
  أرجع FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
ساكن std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              ثابت sys::fs::file_status &Stat) {
  صحيح FD;

  إذا (auto EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    أرجع EC;

  إذا (auto EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    أرجع EC;

  إذا (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    أرجع EC;

  أرجع {};
}

std::error_code FileCollector::copyFiles(منطقي StopOnError) {
  auto Err = sys::fs::create_directories(ڕەگ_2, /*IgnoreExisting=*/true);
  إذا (Err) {
    أرجع Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  لكل (auto &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    إذا (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      إذا (StopOnError)
        أرجع EC;
      استمر;
    }

    // Continue if the file doesn't exist.
    إذا (Stat.type() == sys::fs::file_type::file_not_found)
      استمر;

    // Create directory tree.
    إذا (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/true)) {
      إذا (StopOnError)
        أرجع EC;
    }

    إذا (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      إذا (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/true)) {
        إذا (StopOnError)
          أرجع EC;
      }
      استمر;
    }

    // Copy file over.
    إذا (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      إذا (StopOnError)
        أرجع EC;
    }

    // Copy over permissions.
    إذا (auto perms = sys::fs::getPermissions(entry.VPath)) {
      إذا (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        إذا (StopOnError)
          أرجع EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  أرجع {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(Overlay_ڕەگ);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(Overlay_ڕەگ));
  VFSWriter.setUseExternalNames(خطأ);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  إذا (EC)
    أرجع EC;

  VFSWriter.اكتب(os);

  أرجع {};
}

نطاق llvm {

صنف FileCollectorFileSystem : عام vfs::FileSystem {
عام:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(ثابت Twine &Path) override {
    auto Result = FS->status(Path);
    إذا (Result && Result->exists())
      Collector->addFile(Path);
    أرجع Result;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ثابت Twine &Path) override {
    auto Result = FS->openFileForRead(Path);
    إذا (Result && *Result)
      Collector->addFile(Path);
    أرجع Result;
  }

  llvm::vfs::directory_iterator dir_begin(ثابت llvm::Twine &Dir,
                                          std::error_code &EC) override {
    أرجع Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(ثابت Twine &Path,
                              SmallVectorImpl<حرف> &Output) override {
    auto EC = FS->getRealPath(Path, Output);
    إذا (!EC) {
      Collector->addFile(Path);
      إذا (Output.size() > 0)
        Collector->addFile(Output);
    }
    أرجع EC;
  }

  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(ثابت Twine &Path, منطقي &Result) override {
    أرجع FS->isLocal(Path, Result);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override {
    أرجع FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(ثابت llvm::Twine &Path) override {
    أرجع FS->setCurrentWorkingDirectory(Path);
  }

خاص:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  أرجع makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
