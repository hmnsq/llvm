//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/FileCollector.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Process.h"

using नामाकाश llvm;

FileCollectorBase::FileCollectorBase() = पूर्वनिर्धारित;
FileCollectorBase::~FileCollectorBase() = पूर्वनिर्धारित;

शून्य FileCollectorBase::addFile(स्थिरांक Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  जर (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

शून्य FileCollectorBase::addDirectory(स्थिरांक Twine &Dir) {
  प्रतिज्ञा(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

स्थिर तार्किक isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  जर (sys::fs::real_path(Path, TmpDest))
    परत सत्य; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  जर (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    परत असत्य;
  परत सत्य;
}

FileCollector::FileCollector(std::string मूळ_2, std::string Overlay_मूळ,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : मूळ_2(मूळ_2), Overlay_मूळ(Overlay_मूळ), Canonicalizer(std::move(VFS)) {
  प्रतिज्ञा(sys::path::is_absolute(मूळ_2) && "Root not absolute");
  प्रतिज्ञा(sys::path::is_absolute(Overlay_मूळ) && "OverlayRoot not absolute");
}

शून्य FileCollector::PathCanonicalizer::updateWithRealPath(
    SmallVectorImpl<अक्षर> &Path) {
  StringRef SrcPath(Path.begin(), Path.size());
  StringRef Filename = sys::path::filename(SrcPath);
  StringRef Directory = sys::path::parent_path(SrcPath);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> RealPath;
  स्वतः DirWithSymlink = CachedDirs.find(Directory);
  जर (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    जर (VFS->getRealPath(Directory, RealPath))
      परत;
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
स्थिर शून्य makeAbsolute(vfs::FileSystem &VFS, SmallVectorImpl<अक्षर> &Path) {
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
  sys::path::remove_dots(Paths.VirtualPath, /*remove_dot_dot=*/सत्य);

  परत Paths;
}

शून्य FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(मूळ_2);
  sys::path::append(DstPath, sys::path::relative_path(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  addFileToMapping(Paths.VirtualPath, DstPath);
}

llvm::vfs::directory_iterator
FileCollector::addDirectoryImpl(स्थिरांक llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_code &EC) {
  स्वतः It = FS->dir_begin(Dir, EC);
  जर (EC)
    परत It;
  addFile(Dir);
  साठी (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    जर (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  जर (EC)
    परत It;
  // Return a new iterator.
  परत FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
स्थिर std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              स्थिरांक sys::fs::file_status &Stat) {
  पूर्णांक FD;

  जर (स्वतः EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    परत EC;

  जर (स्वतः EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    परत EC;

  जर (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
    परत EC;

  परत {};
}

std::error_code FileCollector::copyFiles(तार्किक StopOnError) {
  स्वतः Err = sys::fs::create_directories(मूळ_2, /*IgnoreExisting=*/सत्य);
  जर (Err) {
    परत Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  साठी (स्वतः &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    जर (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      जर (StopOnError)
        परत EC;
      सुरूठेवा;
    }

    // Continue if the file doesn't exist.
    जर (Stat.type() == sys::fs::file_type::file_not_found)
      सुरूठेवा;

    // Create directory tree.
    जर (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/सत्य)) {
      जर (StopOnError)
        परत EC;
    }

    जर (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      जर (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/सत्य)) {
        जर (StopOnError)
          परत EC;
      }
      सुरूठेवा;
    }

    // Copy file over.
    जर (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      जर (StopOnError)
        परत EC;
    }

    // Copy over permissions.
    जर (स्वतः perms = sys::fs::getPermissions(entry.VPath)) {
      जर (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        जर (StopOnError)
          परत EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  परत {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(Overlay_मूळ);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(Overlay_मूळ));
  VFSWriter.setUseExternalNames(असत्य);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  जर (EC)
    परत EC;

  VFSWriter.लिख(os);

  परत {};
}

नामाकाश llvm {

वर्ग FileCollectorFileSystem : सार्वजनिक vfs::FileSystem {
सार्वजनिक:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(स्थिरांक Twine &Path) override {
    स्वतः Result = FS->status(Path);
    जर (Result && Result->exists())
      Collector->addFile(Path);
    परत Result;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(स्थिरांक Twine &Path) override {
    स्वतः Result = FS->openFileForRead(Path);
    जर (Result && *Result)
      Collector->addFile(Path);
    परत Result;
  }

  llvm::vfs::directory_iterator dir_begin(स्थिरांक llvm::Twine &Dir,
                                          std::error_code &EC) override {
    परत Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(स्थिरांक Twine &Path,
                              SmallVectorImpl<अक्षर> &Output) override {
    स्वतः EC = FS->getRealPath(Path, Output);
    जर (!EC) {
      Collector->addFile(Path);
      जर (Output.size() > 0)
        Collector->addFile(Output);
    }
    परत EC;
  }

  शून्य
  getDirectoryContentRealSources(स्थिरांक Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(स्थिरांक Twine &Path, तार्किक &Result) override {
    परत FS->isLocal(Path, Result);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() स्थिरांक override {
    परत FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(स्थिरांक llvm::Twine &Path) override {
    परत FS->setCurrentWorkingDirectory(Path);
  }

खाजगी:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  परत makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
