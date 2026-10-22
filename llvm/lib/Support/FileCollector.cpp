//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/FileCollector.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/Twine.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/Process.h"

using namespace llvm;

FileCollectorBase::FileCollectorBase() = शेष;
FileCollectorBase::~FileCollectorBase() = शेष;

व्योम FileCollectorBase::addFile(स्थिर Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  अगर (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

व्योम FileCollectorBase::addDirectory(स्थिर Twine &Dir) {
  निश्चित(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

अटल bool isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  अगर (sys::fs::real_path(Path, TmpDest))
    वापस true; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  अगर (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    वापस false;
  वापस true;
}

FileCollector::FileCollector(std::string मूल_2, std::string Overlay_मूल,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : मूल_2(मूल_2), Overlay_मूल(Overlay_मूल), Canonicalizer(std::move(VFS)) {
  निश्चित(sys::path::is_absolute(मूल_2) && "Root not absolute");
  निश्चित(sys::path::is_absolute(Overlay_मूल) && "OverlayRoot not absolute");
}

व्योम FileCollector::PathCanonicalizer::updateWithRealPath(
    SmallVectorImpl<अक्षर> &Path) {
  StringRef SrcPath(Path.begin(), Path.आकार_2());
  StringRef Filename = sys::path::filename(SrcPath);
  StringRef Directory = sys::path::parent_path(SrcPath);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> RealPath;
  स्वतः DirWithSymlink = CachedDirs.find(Directory);
  अगर (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    अगर (VFS->getRealPath(Directory, RealPath))
      वापस;
    CachedDirs[Directory] = std::string(RealPath);
  } अन्यथा {
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
अटल व्योम makeAbsolute(vfs::FileSystem &VFS, SmallVectorImpl<अक्षर> &Path) {
  // We need an absolute src path to append to the root.
  VFS.makeAbsolute(Path);

  // Canonicalize src to a native path to avoid mixed separator styles.
  sys::path::native(Path);

  // Remove redundant leading "./" pieces and consecutive separators.
  Path.erase(Path.begin(), sys::path::remove_leading_dotslash(
                               StringRef(Path.begin(), Path.आकार_2()))
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

  वापस Paths;
}

व्योम FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(मूल_2);
  sys::path::append(DstPath, sys::path::relative_path(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  addFileToMapping(Paths.VirtualPath, DstPath);
}

llvm::vfs::directory_iterator
FileCollector::addDirectoryImpl(स्थिर llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_code &EC) {
  स्वतः It = FS->dir_begin(Dir, EC);
  अगर (EC)
    वापस It;
  addFile(Dir);
  क्रम (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    अगर (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  अगर (EC)
    वापस It;
  // Return a new iterator.
  वापस FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
अटल std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              स्थिर sys::fs::file_status &Stat) {
  पूर्णांक FD;

  अगर (स्वतः EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    वापस EC;

  अगर (स्वतः EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    वापस EC;

  अगर (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
    वापस EC;

  वापस {};
}

std::error_code FileCollector::copyFiles(bool StopOnError) {
  स्वतः Err = sys::fs::create_directories(मूल_2, /*IgnoreExisting=*/true);
  अगर (Err) {
    वापस Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  क्रम (स्वतः &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    अगर (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      अगर (StopOnError)
        वापस EC;
      जारी;
    }

    // Continue if the file doesn't exist.
    अगर (Stat.type() == sys::fs::file_type::file_not_found)
      जारी;

    // Create directory tree.
    अगर (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/true)) {
      अगर (StopOnError)
        वापस EC;
    }

    अगर (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      अगर (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/true)) {
        अगर (StopOnError)
          वापस EC;
      }
      जारी;
    }

    // Copy file over.
    अगर (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      अगर (StopOnError)
        वापस EC;
    }

    // Copy over permissions.
    अगर (स्वतः perms = sys::fs::getPermissions(entry.VPath)) {
      अगर (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        अगर (StopOnError)
          वापस EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  वापस {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(Overlay_मूल);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(Overlay_मूल));
  VFSWriter.setUseExternalNames(false);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  अगर (EC)
    वापस EC;

  VFSWriter.ग_लिखो(os);

  वापस {};
}

namespace llvm {

श्रेणी FileCollectorFileSystem : खुला vfs::FileSystem {
खुला:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(स्थिर Twine &Path) override {
    स्वतः परिणाम = FS->status(Path);
    अगर (परिणाम && परिणाम->exists())
      Collector->addFile(Path);
    वापस परिणाम;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(स्थिर Twine &Path) override {
    स्वतः परिणाम = FS->openFileForRead(Path);
    अगर (परिणाम && *परिणाम)
      Collector->addFile(Path);
    वापस परिणाम;
  }

  llvm::vfs::directory_iterator dir_begin(स्थिर llvm::Twine &Dir,
                                          std::error_code &EC) override {
    वापस Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(स्थिर Twine &Path,
                              SmallVectorImpl<अक्षर> &Output) override {
    स्वतः EC = FS->getRealPath(Path, Output);
    अगर (!EC) {
      Collector->addFile(Path);
      अगर (Output.आकार_2() > 0)
        Collector->addFile(Output);
    }
    वापस EC;
  }

  व्योम
  getDirectoryContentRealSources(स्थिर Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(स्थिर Twine &Path, bool &परिणाम) override {
    वापस FS->isLocal(Path, परिणाम);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() स्थिर override {
    वापस FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(स्थिर llvm::Twine &Path) override {
    वापस FS->setCurrentWorkingDirectory(Path);
  }

निजी:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  वापस makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
