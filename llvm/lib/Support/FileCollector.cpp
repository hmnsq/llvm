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

शून्य FileCollectorBase::addFile(ध्रुव Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  सचे (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

शून्य FileCollectorBase::addDirectory(ध्रुव Twine &Dir) {
  प्रतिज्ञा(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

स्थिर तार्किक isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  सचे (sys::fs::real_path(Path, TmpDest))
    पटिनिवत्त सच्च; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  सचे (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    पटिनिवत्त मिच्छा;
  पटिनिवत्त सच्च;
}

FileCollector::FileCollector(std::string मूलम्_2, std::string Overlay_मूलम्,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : मूलम्_2(मूलम्_2), Overlay_मूलम्(Overlay_मूलम्), Canonicalizer(std::move(VFS)) {
  प्रतिज्ञा(sys::path::is_absolute(मूलम्_2) && "Root not absolute");
  प्रतिज्ञा(sys::path::is_absolute(Overlay_मूलम्) && "OverlayRoot not absolute");
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
  auto DirWithSymlink = CachedDirs.find(Directory);
  सचे (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    सचे (VFS->getRealPath(Directory, RealPath))
      पटिनिवत्त;
    CachedDirs[Directory] = std::string(RealPath);
  } अञ्ञथा {
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
  sys::path::remove_dots(Paths.VirtualPath, /*remove_dot_dot=*/सच्च);

  पटिनिवत्त Paths;
}

शून्य FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(मूलम्_2);
  sys::path::append(DstPath, sys::path::relative_path(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  addFileToMapping(Paths.VirtualPath, DstPath);
}

llvm::vfs::directory_iterator
FileCollector::addDirectoryImpl(ध्रुव llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_code &EC) {
  auto It = FS->dir_begin(Dir, EC);
  सचे (EC)
    पटिनिवत्त It;
  addFile(Dir);
  अत्थाय (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    सचे (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  सचे (EC)
    पटिनिवत्त It;
  // Return a new iterator.
  पटिनिवत्त FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
स्थिर std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              ध्रुव sys::fs::file_status &Stat) {
  पूर्णाङ्क FD;

  सचे (auto EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    पटिनिवत्त EC;

  सचे (auto EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    पटिनिवत्त EC;

  सचे (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    पटिनिवत्त EC;

  पटिनिवत्त {};
}

std::error_code FileCollector::copyFiles(तार्किक StopOnError) {
  auto Err = sys::fs::create_directories(मूलम्_2, /*IgnoreExisting=*/सच्च);
  सचे (Err) {
    पटिनिवत्त Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  अत्थाय (auto &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    सचे (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      सचे (StopOnError)
        पटिनिवत्त EC;
      अनुवर्त;
    }

    // Continue if the file doesn't exist.
    सचे (Stat.type() == sys::fs::file_type::file_not_found)
      अनुवर्त;

    // Create directory tree.
    सचे (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/सच्च)) {
      सचे (StopOnError)
        पटिनिवत्त EC;
    }

    सचे (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      सचे (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/सच्च)) {
        सचे (StopOnError)
          पटिनिवत्त EC;
      }
      अनुवर्त;
    }

    // Copy file over.
    सचे (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      सचे (StopOnError)
        पटिनिवत्त EC;
    }

    // Copy over permissions.
    सचे (auto perms = sys::fs::getPermissions(entry.VPath)) {
      सचे (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        सचे (StopOnError)
          पटिनिवत्त EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  पटिनिवत्त {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(Overlay_मूलम्);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(Overlay_मूलम्));
  VFSWriter.setUseExternalNames(मिच्छा);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  सचे (EC)
    पटिनिवत्त EC;

  VFSWriter.लिख(os);

  पटिनिवत्त {};
}

नामाकाश llvm {

वग्ग FileCollectorFileSystem : सार्वजनिक vfs::FileSystem {
सार्वजनिक:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(ध्रुव Twine &Path) override {
    auto Result = FS->status(Path);
    सचे (Result && Result->exists())
      Collector->addFile(Path);
    पटिनिवत्त Result;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ध्रुव Twine &Path) override {
    auto Result = FS->openFileForRead(Path);
    सचे (Result && *Result)
      Collector->addFile(Path);
    पटिनिवत्त Result;
  }

  llvm::vfs::directory_iterator dir_begin(ध्रुव llvm::Twine &Dir,
                                          std::error_code &EC) override {
    पटिनिवत्त Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(ध्रुव Twine &Path,
                              SmallVectorImpl<वर्ण> &Output) override {
    auto EC = FS->getRealPath(Path, Output);
    सचे (!EC) {
      Collector->addFile(Path);
      सचे (Output.size() > 0)
        Collector->addFile(Output);
    }
    पटिनिवत्त EC;
  }

  शून्य
  getDirectoryContentRealSources(ध्रुव Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(ध्रुव Twine &Path, तार्किक &Result) override {
    पटिनिवत्त FS->isLocal(Path, Result);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ध्रुव override {
    पटिनिवत्त FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(ध्रुव llvm::Twine &Path) override {
    पटिनिवत्त FS->setCurrentWorkingDirectory(Path);
  }

गुप्त:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  पटिनिवत्त makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
