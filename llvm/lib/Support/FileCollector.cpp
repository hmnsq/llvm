//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/FileCollector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"

using namespace llvm;

FileCollectorBase::FileCollectorBase() = default;
FileCollectorBase::~FileCollectorBase() = default;

void FileCollectorBase::addFile(const Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  если (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

void FileCollectorBase::addDirectory(const Twine &Dir) {
  assert(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

static bool isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  если (sys::fs::real_path(Path, TmpDest))
    вернуть true; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  если (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    вернуть false;
  вернуть true;
}

FileCollector::FileCollector(std::string Root, std::string OverlayRoot,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : Root(Root), OverlayRoot(OverlayRoot), Canonicalizer(std::move(VFS)) {
  assert(sys::path::is_absolute(Root) && "Root not absolute");
  assert(sys::path::is_absolute(OverlayRoot) && "OverlayRoot not absolute");
}

void FileCollector::PathCanonicalizer::updateWithRealPath(
    SmallVectorImpl<char> &Path) {
  StringRef SrcPath(Path.begin(), Path.size());
  StringRef Filename = sys::path::filename(SrcPath);
  StringRef Directory = sys::path::parent_path(SrcPath);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> RealPath;
  auto DirWithSymlink = CachedDirs.find(Directory);
  если (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    если (VFS->getRealPath(Directory, RealPath))
      вернуть;
    CachedDirs[Directory] = std::string(RealPath);
  } иначе {
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
static void makeAbsolute(vfs::FileSystem &VFS, SmallVectorImpl<char> &Path) {
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

  вернуть Paths;
}

void FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(Root);
  sys::path::append(DstPath, sys::path::relative_path(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  addFileToMapping(Paths.VirtualPath, DstPath);
}

llvm::vfs::directory_iterator
FileCollector::addDirectoryImpl(const llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_code &EC) {
  auto It = FS->dir_begin(Dir, EC);
  если (EC)
    вернуть It;
  addFile(Dir);
  для (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    если (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  если (EC)
    вернуть It;
  // Return a new iterator.
  вернуть FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
static std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              const sys::fs::file_status &Stat) {
  целое FD;

  если (auto EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    вернуть EC;

  если (auto EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    вернуть EC;

  если (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    вернуть EC;

  вернуть {};
}

std::error_code FileCollector::copyFiles(bool StopOnError) {
  auto Err = sys::fs::create_directories(Root, /*IgnoreExisting=*/true);
  если (Err) {
    вернуть Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  для (auto &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    если (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      если (StopOnError)
        вернуть EC;
      continue;
    }

    // Continue if the file doesn't exist.
    если (Stat.type() == sys::fs::file_type::file_not_found)
      continue;

    // Create directory tree.
    если (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/true)) {
      если (StopOnError)
        вернуть EC;
    }

    если (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      если (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/true)) {
        если (StopOnError)
          вернуть EC;
      }
      continue;
    }

    // Copy file over.
    если (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      если (StopOnError)
        вернуть EC;
    }

    // Copy over permissions.
    если (auto perms = sys::fs::getPermissions(entry.VPath)) {
      если (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        если (StopOnError)
          вернуть EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  вернуть {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(OverlayRoot);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(OverlayRoot));
  VFSWriter.setUseExternalNames(false);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  если (EC)
    вернуть EC;

  VFSWriter.write(os);

  вернуть {};
}

namespace llvm {

class FileCollectorFileSystem : public vfs::FileSystem {
public:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(const Twine &Path) override {
    auto Result = FS->status(Path);
    если (Result && Result->exists())
      Collector->addFile(Path);
    вернуть Result;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(const Twine &Path) override {
    auto Result = FS->openFileForRead(Path);
    если (Result && *Result)
      Collector->addFile(Path);
    вернуть Result;
  }

  llvm::vfs::directory_iterator dir_begin(const llvm::Twine &Dir,
                                          std::error_code &EC) override {
    вернуть Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(const Twine &Path,
                              SmallVectorImpl<char> &Output) override {
    auto EC = FS->getRealPath(Path, Output);
    если (!EC) {
      Collector->addFile(Path);
      если (Output.size() > 0)
        Collector->addFile(Output);
    }
    вернуть EC;
  }

  void
  getDirectoryContentRealSources(const Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(const Twine &Path, bool &Result) override {
    вернуть FS->isLocal(Path, Result);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override {
    вернуть FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(const llvm::Twine &Path) override {
    вернуть FS->setCurrentWorkingDirectory(Path);
  }

private:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  вернуть makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
