//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/FileCollector.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/Process.h"

using નામાકાશ llvm;

FileCollectorBase::FileCollectorBase() = મૂળભૂત;
FileCollectorBase::~FileCollectorBase() = મૂળભૂત;

શૂન્ય FileCollectorBase::addFile(અચલ Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  જો (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

શૂન્ય FileCollectorBase::addDirectory(અચલ Twine &Dir) {
  પ્રતિજ્ઞા(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

સ્થિર તાર્કિક isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  જો (sys::fs::real_path(Path, TmpDest))
    પાછા સાચું; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  જો (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    પાછા ખોટું;
  પાછા સાચું;
}

FileCollector::FileCollector(std::string મૂળ_2, std::string Overlay_મૂળ,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : મૂળ_2(મૂળ_2), Overlay_મૂળ(Overlay_મૂળ), Canonicalizer(std::move(VFS)) {
  પ્રતિજ્ઞા(sys::path::is_absolute(મૂળ_2) && "Root not absolute");
  પ્રતિજ્ઞા(sys::path::is_absolute(Overlay_મૂળ) && "OverlayRoot not absolute");
}

શૂન્ય FileCollector::PathCanonicalizer::updateWithRealPath(
    SmallVectorImpl<અક્ષર> &Path) {
  StringRef SrcPath(Path.begin(), Path.size());
  StringRef Filename = sys::path::filename(SrcPath);
  StringRef Directory = sys::path::parent_path(SrcPath);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> RealPath;
  auto DirWithSymlink = CachedDirs.find(Directory);
  જો (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    જો (VFS->getRealPath(Directory, RealPath))
      પાછા;
    CachedDirs[Directory] = std::string(RealPath);
  } નહીંતર {
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
સ્થિર શૂન્ય makeAbsolute(vfs::FileSystem &VFS, SmallVectorImpl<અક્ષર> &Path) {
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
  sys::path::remove_dots(Paths.VirtualPath, /*remove_dot_dot=*/સાચું);

  પાછા Paths;
}

શૂન્ય FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(મૂળ_2);
  sys::path::append(DstPath, sys::path::relative_path(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  addFileToMapping(Paths.VirtualPath, DstPath);
}

llvm::vfs::directory_iterator
FileCollector::addDirectoryImpl(અચલ llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_code &EC) {
  auto It = FS->dir_begin(Dir, EC);
  જો (EC)
    પાછા It;
  addFile(Dir);
  માટે (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    જો (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  જો (EC)
    પાછા It;
  // Return a new iterator.
  પાછા FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
સ્થિર std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              અચલ sys::fs::file_status &Stat) {
  પૂર્ણાંક FD;

  જો (auto EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    પાછા EC;

  જો (auto EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    પાછા EC;

  જો (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    પાછા EC;

  પાછા {};
}

std::error_code FileCollector::copyFiles(તાર્કિક StopOnError) {
  auto Err = sys::fs::create_directories(મૂળ_2, /*IgnoreExisting=*/સાચું);
  જો (Err) {
    પાછા Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  માટે (auto &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    જો (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      જો (StopOnError)
        પાછા EC;
      ચાલુ;
    }

    // Continue if the file doesn't exist.
    જો (Stat.type() == sys::fs::file_type::file_not_found)
      ચાલુ;

    // Create directory tree.
    જો (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/સાચું)) {
      જો (StopOnError)
        પાછા EC;
    }

    જો (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      જો (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/સાચું)) {
        જો (StopOnError)
          પાછા EC;
      }
      ચાલુ;
    }

    // Copy file over.
    જો (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      જો (StopOnError)
        પાછા EC;
    }

    // Copy over permissions.
    જો (auto perms = sys::fs::getPermissions(entry.VPath)) {
      જો (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        જો (StopOnError)
          પાછા EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  પાછા {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(Overlay_મૂળ);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(Overlay_મૂળ));
  VFSWriter.setUseExternalNames(ખોટું);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  જો (EC)
    પાછા EC;

  VFSWriter.લિખ(os);

  પાછા {};
}

નામાકાશ llvm {

વર્ગ FileCollectorFileSystem : જાહેર vfs::FileSystem {
જાહેર:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(અચલ Twine &Path) override {
    auto Result = FS->status(Path);
    જો (Result && Result->exists())
      Collector->addFile(Path);
    પાછા Result;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(અચલ Twine &Path) override {
    auto Result = FS->openFileForRead(Path);
    જો (Result && *Result)
      Collector->addFile(Path);
    પાછા Result;
  }

  llvm::vfs::directory_iterator dir_begin(અચલ llvm::Twine &Dir,
                                          std::error_code &EC) override {
    પાછા Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(અચલ Twine &Path,
                              SmallVectorImpl<અક્ષર> &Output) override {
    auto EC = FS->getRealPath(Path, Output);
    જો (!EC) {
      Collector->addFile(Path);
      જો (Output.size() > 0)
        Collector->addFile(Output);
    }
    પાછા EC;
  }

  શૂન્ય
  getDirectoryContentRealSources(અચલ Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(અચલ Twine &Path, તાર્કિક &Result) override {
    પાછા FS->isLocal(Path, Result);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() અચલ override {
    પાછા FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(અચલ llvm::Twine &Path) override {
    પાછા FS->setCurrentWorkingDirectory(Path);
  }

ખાનગી:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  પાછા makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
