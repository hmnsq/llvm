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

FileCollectorBase::FileCollectorBase() = स्वभाव;
FileCollectorBase::~FileCollectorBase() = स्वभाव;

शून्य FileCollectorBase::addFile(अचर Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  यदि (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

शून्य FileCollectorBase::addDirectory(अचर Twine &Dir) {
  प्रतिज्ञा(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

स्थिर तार्किक isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  यदि (sys::fs::real_path(Path, TmpDest))
    फर्काऊ सत्य; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  यदि (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    फर्काऊ असत्य;
  फर्काऊ सत्य;
}

FileCollector::FileCollector(std::string जरा_2, std::string Overlay_जरा,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : जरा_2(जरा_2), Overlay_जरा(Overlay_जरा), Canonicalizer(std::move(VFS)) {
  प्रतिज्ञा(sys::path::is_absolute(जरा_2) && "Root not absolute");
  प्रतिज्ञा(sys::path::is_absolute(Overlay_जरा) && "OverlayRoot not absolute");
}

शून्य FileCollector::PathCanonicalizer::updateWithRealPath(
    SmallVectorImpl<वर्ण> &Path) {
  StringRef SrcPath(Path.begin(), Path.size());
  StringRef Filename = sys::path::filename(SrcPath);
  StringRef Directory = sys::path::parent_path(SrcPath);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> RealPath;
  स्वतः DirWithSymlink = CachedDirs.find(Directory);
  यदि (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    यदि (VFS->getRealPath(Directory, RealPath))
      फर्काऊ;
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
स्थिर शून्य makeAbsolute(vfs::FileSystem &VFS, SmallVectorImpl<वर्ण> &Path) {
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

  फर्काऊ Paths;
}

शून्य FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(जरा_2);
  sys::path::append(DstPath, sys::path::relative_path(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  addFileToMapping(Paths.VirtualPath, DstPath);
}

llvm::vfs::directory_iterator
FileCollector::addDirectoryImpl(अचर llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_code &EC) {
  स्वतः It = FS->dir_begin(Dir, EC);
  यदि (EC)
    फर्काऊ It;
  addFile(Dir);
  लागि (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    यदि (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  यदि (EC)
    फर्काऊ It;
  // Return a new iterator.
  फर्काऊ FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
स्थिर std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              अचर sys::fs::file_status &Stat) {
  पूर्णाङ्क FD;

  यदि (स्वतः EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    फर्काऊ EC;

  यदि (स्वतः EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    फर्काऊ EC;

  यदि (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
    फर्काऊ EC;

  फर्काऊ {};
}

std::error_code FileCollector::copyFiles(तार्किक StopOnError) {
  स्वतः Err = sys::fs::create_directories(जरा_2, /*IgnoreExisting=*/सत्य);
  यदि (Err) {
    फर्काऊ Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  लागि (स्वतः &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    यदि (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      यदि (StopOnError)
        फर्काऊ EC;
      जारी;
    }

    // Continue if the file doesn't exist.
    यदि (Stat.type() == sys::fs::file_type::file_not_found)
      जारी;

    // Create directory tree.
    यदि (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/सत्य)) {
      यदि (StopOnError)
        फर्काऊ EC;
    }

    यदि (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      यदि (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/सत्य)) {
        यदि (StopOnError)
          फर्काऊ EC;
      }
      जारी;
    }

    // Copy file over.
    यदि (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      यदि (StopOnError)
        फर्काऊ EC;
    }

    // Copy over permissions.
    यदि (स्वतः perms = sys::fs::getPermissions(entry.VPath)) {
      यदि (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        यदि (StopOnError)
          फर्काऊ EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  फर्काऊ {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(Overlay_जरा);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(Overlay_जरा));
  VFSWriter.setUseExternalNames(असत्य);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  यदि (EC)
    फर्काऊ EC;

  VFSWriter.लिख(os);

  फर्काऊ {};
}

नामाकाश llvm {

वर्ग FileCollectorFileSystem : सार्वजनिक vfs::FileSystem {
सार्वजनिक:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(अचर Twine &Path) override {
    स्वतः Result = FS->status(Path);
    यदि (Result && Result->exists())
      Collector->addFile(Path);
    फर्काऊ Result;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(अचर Twine &Path) override {
    स्वतः Result = FS->openFileForRead(Path);
    यदि (Result && *Result)
      Collector->addFile(Path);
    फर्काऊ Result;
  }

  llvm::vfs::directory_iterator dir_begin(अचर llvm::Twine &Dir,
                                          std::error_code &EC) override {
    फर्काऊ Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(अचर Twine &Path,
                              SmallVectorImpl<वर्ण> &Output) override {
    स्वतः EC = FS->getRealPath(Path, Output);
    यदि (!EC) {
      Collector->addFile(Path);
      यदि (Output.size() > 0)
        Collector->addFile(Output);
    }
    फर्काऊ EC;
  }

  शून्य
  getDirectoryContentRealSources(अचर Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(अचर Twine &Path, तार्किक &Result) override {
    फर्काऊ FS->isLocal(Path, Result);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() अचर override {
    फर्काऊ FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(अचर llvm::Twine &Path) override {
    फर्काऊ FS->setCurrentWorkingDirectory(Path);
  }

गुप्त:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  फर्काऊ makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
