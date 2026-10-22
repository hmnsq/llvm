//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/FileCollector.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/Twine.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"

using നാമാകാശ llvm;

FileCollectorBase::FileCollectorBase() = സ്ഥിരസ്ഥിതി;
FileCollectorBase::~FileCollectorBase() = സ്ഥിരസ്ഥിതി;

ശൂന്യം FileCollectorBase::addFile(സ്ഥിരം Twine &File) {
  std::lock_guard<std::mutex> lock(Mutex);
  std::string FileStr = File.str();
  എങ്കിൽ (markAsSeen(FileStr))
    addFileImpl(FileStr);
}

ശൂന്യം FileCollectorBase::addDirectory(സ്ഥിരം Twine &Dir) {
  പ്രതിജ്ഞാ(sys::fs::is_directory(Dir));
  std::error_code EC;
  addDirectoryImpl(Dir, vfs::getRealFileSystem(), EC);
}

സ്ഥിതം തർക്കം isCaseSensitivePath(StringRef Path) {
  SmallString<256> TmpDest = Path, UpperDest, RealDest;

  // Remove component traversals, links, etc.
  എങ്കിൽ (sys::fs::real_path(Path, TmpDest))
    തിരികെ സത്യം; // Current default value in vfs.yaml
  Path = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  UpperDest = Path.upper();
  എങ്കിൽ (!sys::fs::real_path(UpperDest, RealDest) && Path == RealDest)
    തിരികെ അസത്യം;
  തിരികെ സത്യം;
}

FileCollector::FileCollector(std::string വേര്_2, std::string Overlay_വേര്,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : വേര്_2(വേര്_2), Overlay_വേര്(Overlay_വേര്), Canonicalizer(std::move(VFS)) {
  പ്രതിജ്ഞാ(sys::path::is_absolute(വേര്_2) && "Root not absolute");
  പ്രതിജ്ഞാ(sys::path::is_absolute(Overlay_വേര്) && "OverlayRoot not absolute");
}

ശൂന്യം FileCollector::PathCanonicalizer::updateWithRealPath(
    SmallVectorImpl<അക്ഷരം> &Path) {
  StringRef SrcPath(Path.begin(), Path.size());
  StringRef Filename = sys::path::filename(SrcPath);
  StringRef Directory = sys::path::parent_path(SrcPath);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> RealPath;
  auto DirWithSymlink = CachedDirs.find(Directory);
  എങ്കിൽ (DirWithSymlink == CachedDirs.end()) {
    // FIXME: What if there is nothing on disk?
    എങ്കിൽ (VFS->getRealPath(Directory, RealPath))
      തിരികെ;
    CachedDirs[Directory] = std::string(RealPath);
  } അല്ലെങ്കിൽ {
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
സ്ഥിതം ശൂന്യം makeAbsolute(vfs::FileSystem &VFS, SmallVectorImpl<അക്ഷരം> &Path) {
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
  sys::path::remove_dots(Paths.VirtualPath, /*remove_dot_dot=*/സത്യം);

  തിരികെ Paths;
}

ശൂന്യം FileCollector::addFileImpl(StringRef SrcPath) {
  PathCanonicalizer::PathStorage Paths = Canonicalizer.canonicalize(SrcPath);

  SmallString<256> DstPath = StringRef(വേര്_2);
  sys::path::append(DstPath, sys::path::relative_path(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  addFileToMapping(Paths.VirtualPath, DstPath);
}

llvm::vfs::directory_iterator
FileCollector::addDirectoryImpl(സ്ഥിരം llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_code &EC) {
  auto It = FS->dir_begin(Dir, EC);
  എങ്കിൽ (EC)
    തിരികെ It;
  addFile(Dir);
  വേണ്ടി (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    എങ്കിൽ (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      addFile(It->path());
    }
  }
  എങ്കിൽ (EC)
    തിരികെ It;
  // Return a new iterator.
  തിരികെ FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
സ്ഥിതം std::error_code
copyAccessAndModificationTime(StringRef Filename,
                              സ്ഥിരം sys::fs::file_status &Stat) {
  പൂർണ്ണസംഖ്യ FD;

  എങ്കിൽ (auto EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    തിരികെ EC;

  എങ്കിൽ (auto EC = sys::fs::setLastAccessAndModificationTime(
          FD, Stat.getLastAccessedTime(), Stat.getLastModificationTime()))
    തിരികെ EC;

  എങ്കിൽ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    തിരികെ EC;

  തിരികെ {};
}

std::error_code FileCollector::copyFiles(തർക്കം StopOnError) {
  auto Err = sys::fs::create_directories(വേര്_2, /*IgnoreExisting=*/സത്യം);
  എങ്കിൽ (Err) {
    തിരികെ Err;
  }

  std::lock_guard<std::mutex> lock(Mutex);

  വേണ്ടി (auto &entry : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_status Stat;
    എങ്കിൽ (std::error_code EC = sys::fs::status(entry.VPath, Stat)) {
      എങ്കിൽ (StopOnError)
        തിരികെ EC;
      തുടരുക;
    }

    // Continue if the file doesn't exist.
    എങ്കിൽ (Stat.type() == sys::fs::file_type::file_not_found)
      തുടരുക;

    // Create directory tree.
    എങ്കിൽ (std::error_code EC =
            sys::fs::create_directories(sys::path::parent_path(entry.RPath),
                                        /*IgnoreExisting=*/സത്യം)) {
      എങ്കിൽ (StopOnError)
        തിരികെ EC;
    }

    എങ്കിൽ (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      എങ്കിൽ (std::error_code EC =
              sys::fs::create_directories(entry.RPath,
                                          /*IgnoreExisting=*/സത്യം)) {
        എങ്കിൽ (StopOnError)
          തിരികെ EC;
      }
      തുടരുക;
    }

    // Copy file over.
    എങ്കിൽ (std::error_code EC = sys::fs::copy_file(entry.VPath, entry.RPath)) {
      എങ്കിൽ (StopOnError)
        തിരികെ EC;
    }

    // Copy over permissions.
    എങ്കിൽ (auto perms = sys::fs::getPermissions(entry.VPath)) {
      എങ്കിൽ (std::error_code EC = sys::fs::setPermissions(entry.RPath, *perms)) {
        എങ്കിൽ (StopOnError)
          തിരികെ EC;
      }
    }

    // Copy over modification time.
    copyAccessAndModificationTime(entry.RPath, Stat);
  }
  തിരികെ {};
}

std::error_code FileCollector::writeMapping(StringRef MappingFile) {
  std::lock_guard<std::mutex> lock(Mutex);

  VFSWriter.setOverlayDir(Overlay_വേര്);
  VFSWriter.setCaseSensitivity(isCaseSensitivePath(Overlay_വേര്));
  VFSWriter.setUseExternalNames(അസത്യം);

  std::error_code EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::OF_TextWithCRLF);
  എങ്കിൽ (EC)
    തിരികെ EC;

  VFSWriter.ലിഖ(os);

  തിരികെ {};
}

നാമാകാശ llvm {

വർഗ്ഗം FileCollectorFileSystem : പൊതു vfs::FileSystem {
പൊതു:
  explicit FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::Status> status(സ്ഥിരം Twine &Path) override {
    auto Result = FS->status(Path);
    എങ്കിൽ (Result && Result->exists())
      Collector->addFile(Path);
    തിരികെ Result;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(സ്ഥിരം Twine &Path) override {
    auto Result = FS->openFileForRead(Path);
    എങ്കിൽ (Result && *Result)
      Collector->addFile(Path);
    തിരികെ Result;
  }

  llvm::vfs::directory_iterator dir_begin(സ്ഥിരം llvm::Twine &Dir,
                                          std::error_code &EC) override {
    തിരികെ Collector->addDirectoryImpl(Dir, FS, EC);
  }

  std::error_code getRealPath(സ്ഥിരം Twine &Path,
                              SmallVectorImpl<അക്ഷരം> &Output) override {
    auto EC = FS->getRealPath(Path, Output);
    എങ്കിൽ (!EC) {
      Collector->addFile(Path);
      എങ്കിൽ (Output.size() > 0)
        Collector->addFile(Output);
    }
    തിരികെ EC;
  }

  ശൂന്യം
  getDirectoryContentRealSources(സ്ഥിരം Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    FS->getDirectoryContentRealSources(Dir, Out);
  }

  std::error_code isLocal(സ്ഥിരം Twine &Path, തർക്കം &Result) override {
    തിരികെ FS->isLocal(Path, Result);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() സ്ഥിരം override {
    തിരികെ FS->getCurrentWorkingDirectory();
  }

  std::error_code setCurrentWorkingDirectory(സ്ഥിരം llvm::Twine &Path) override {
    തിരികെ FS->setCurrentWorkingDirectory(Path);
  }

സ്വകാര്യ:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> BaseFS,
                                  std::shared_ptr<FileCollector> Collector) {
  തിരികെ makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(BaseFS),
                                                      std::move(Collector));
}
