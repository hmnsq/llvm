//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileCollector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

File_Collector_ਅਧਾਰ::File_Collector_ਅਧਾਰ() = ਮੂਲ;
File_Collector_ਅਧਾਰ::~File_Collector_ਅਧਾਰ() = ਮੂਲ;

ਖਾਲੀ File_Collector_ਅਧਾਰ::ਜੋੜੋ_File(ਅਚਲ Twine &File) {
  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ(Mutex);
  std::string FileStr = File.str();
  ਜੇ (mark_As_ਵੇਖਿਆ(FileStr))
    ਜੋੜੋ_File_Impl(FileStr);
}

ਖਾਲੀ File_Collector_ਅਧਾਰ::ਜੋੜੋ_Directory(ਅਚਲ Twine &Dir) {
  ਪ੍ਰਤਿਜ੍ਞਾ(sys::fs::is_directory(Dir));
  std::error_ਕੋਡ EC;
  ਜੋੜੋ_Directory_Impl(Dir, vfs::getRealFileSystem(), EC);
}

ਸਥਿਰ ਤਾਰਕਿਕ is_Case_Sensitive_ਰਸਤਾ(StringRef ਰਸਤਾ) {
  SmallString<256> TmpDest = ਰਸਤਾ, ਉੱਤਲਾ_Dest, RealDest;

  // Remove component traversals, links, etc.
  ਜੇ (sys::fs::real_ਰਸਤਾ(ਰਸਤਾ, TmpDest))
    ਵਾਪਸ ਸੱਚ; // Current default value in vfs.yaml
  ਰਸਤਾ = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  ਉੱਤਲਾ_Dest = ਰਸਤਾ.ਉੱਤਲਾ();
  ਜੇ (!sys::fs::real_ਰਸਤਾ(ਉੱਤਲਾ_Dest, RealDest) && ਰਸਤਾ == RealDest)
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ ਸੱਚ;
}

FileCollector::FileCollector(std::string ਜੜ੍ਹ_2, std::string Overlay_ਜੜ੍ਹ,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : ਜੜ੍ਹ_2(ਜੜ੍ਹ_2), Overlay_ਜੜ੍ਹ(Overlay_ਜੜ੍ਹ), Canonicalizer(std::move(VFS)) {
  ਪ੍ਰਤਿਜ੍ਞਾ(sys::ਰਸਤਾ_2::is_absolute(ਜੜ੍ਹ_2) && "Root not absolute");
  ਪ੍ਰਤਿਜ੍ਞਾ(sys::ਰਸਤਾ_2::is_absolute(Overlay_ਜੜ੍ਹ) && "OverlayRoot not absolute");
}

ਖਾਲੀ FileCollector::ਰਸਤਾ_Canonicalizer::update_With_Real_ਰਸਤਾ(
    Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ) {
  StringRef Src_ਰਸਤਾ(ਰਸਤਾ.begin(), ਰਸਤਾ.ਮਾਪ_2());
  StringRef Filename = sys::ਰਸਤਾ_2::filename(Src_ਰਸਤਾ);
  StringRef Directory = sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(Src_ਰਸਤਾ);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> Real_ਰਸਤਾ;
  ਸਵੈਚਾਲਿਤ DirWithSymlink = CachedDirs.ਲੱਭੋ(Directory);
  ਜੇ (DirWithSymlink == CachedDirs.ਸਿਰਾ()) {
    // FIXME: What if there is nothing on disk?
    ਜੇ (VFS->get_Real_ਰਸਤਾ(Directory, Real_ਰਸਤਾ))
      ਵਾਪਸ;
    CachedDirs[Directory] = std::string(Real_ਰਸਤਾ);
  } ਨਹੀਂਤਾਂ {
    Real_ਰਸਤਾ = DirWithSymlink->ਦੂਜਾ_2;
  }

  // Finish recreating the path by appending the original filename, since we
  // don't need to resolve symlinks in the filename.
  //
  // FIXME: If we can cope with this, maybe we can cope without calling
  // getRealPath() at all when there's no ".." component.
  sys::ਰਸਤਾ_2::append(Real_ਰਸਤਾ, Filename);

  // Swap to create the output.
  ਰਸਤਾ.ਅਦਲਾ_ਬਦਲੀ(Real_ਰਸਤਾ);
}

/// Make Path absolute.
ਸਥਿਰ ਖਾਲੀ makeAbsolute(vfs::FileSystem &VFS, Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ) {
  // We need an absolute src path to append to the root.
  VFS.makeAbsolute(ਰਸਤਾ);

  // Canonicalize src to a native path to avoid mixed separator styles.
  sys::ਰਸਤਾ_2::native(ਰਸਤਾ);

  // Remove redundant leading "./" pieces and consecutive separators.
  ਰਸਤਾ.erase(ਰਸਤਾ.begin(), sys::ਰਸਤਾ_2::remove_leading_dotslash(
                               StringRef(ਰਸਤਾ.begin(), ਰਸਤਾ.ਮਾਪ_2()))
                               .begin());
}

FileCollector::ਰਸਤਾ_Canonicalizer::ਰਸਤਾ_Storage
FileCollector::ਰਸਤਾ_Canonicalizer::canonicalize(StringRef Src_ਰਸਤਾ) {
  ਰਸਤਾ_Storage Paths;
  Paths.Virtual_ਰਸਤਾ = Src_ਰਸਤਾ;
  makeAbsolute(*VFS, Paths.Virtual_ਰਸਤਾ);

  // If a ".." component is present after a symlink component, remove_dots may
  // lead to the wrong real destination path. Let the source be canonicalized
  // like that but make sure we always use the real path for the destination.
  Paths.CopyFrom = Paths.Virtual_ਰਸਤਾ;
  update_With_Real_ਰਸਤਾ(Paths.CopyFrom);

  // Canonicalize the virtual path by removing "..", "." components.
  sys::ਰਸਤਾ_2::remove_dots(Paths.Virtual_ਰਸਤਾ, /*remove_dot_dot=*/ਸੱਚ);

  ਵਾਪਸ Paths;
}

ਖਾਲੀ FileCollector::ਜੋੜੋ_File_Impl(StringRef Src_ਰਸਤਾ) {
  ਰਸਤਾ_Canonicalizer::ਰਸਤਾ_Storage Paths = Canonicalizer.canonicalize(Src_ਰਸਤਾ);

  SmallString<256> Dst_ਰਸਤਾ = StringRef(ਜੜ੍ਹ_2);
  sys::ਰਸਤਾ_2::append(Dst_ਰਸਤਾ, sys::ਰਸਤਾ_2::relative_ਰਸਤਾ(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  ਜੋੜੋ_File_ਮੰਜ਼ਿਲ_Mapping(Paths.Virtual_ਰਸਤਾ, Dst_ਰਸਤਾ);
}

llvm::vfs::directory_iterator
FileCollector::ਜੋੜੋ_Directory_Impl(ਅਚਲ llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_ਕੋਡ &EC) {
  ਸਵੈਚਾਲਿਤ It = FS->dir_begin(Dir, EC);
  ਜੇ (EC)
    ਵਾਪਸ It;
  ਜੋੜੋ_File(Dir);
  ਲਈ (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    ਜੇ (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      ਜੋੜੋ_File(It->ਰਸਤਾ_2());
    }
  }
  ਜੇ (EC)
    ਵਾਪਸ It;
  // Return a new iterator.
  ਵਾਪਸ FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
ਸਥਿਰ std::error_ਕੋਡ
copy_Access_And_Modification_ਸਮਾਂ(StringRef Filename,
                              ਅਚਲ sys::fs::file_ਸਥਿਤੀ &Stat) {
  ਪੂਰਨ FD;

  ਜੇ (ਸਵੈਚਾਲਿਤ EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    ਵਾਪਸ EC;

  ਜੇ (ਸਵੈਚਾਲਿਤ EC = sys::fs::ਸੈੱਟ_ਆਖਰੀ_Access_And_Modification_ਸਮਾਂ(
          FD, Stat.get_ਆਖਰੀ_Accessed_ਸਮਾਂ(), Stat.get_ਆਖਰੀ_Modification_ਸਮਾਂ()))
    ਵਾਪਸ EC;

  ਜੇ (ਸਵੈਚਾਲਿਤ EC = sys::Process::SafelyCloseFileDescriptor(FD))
    ਵਾਪਸ EC;

  ਵਾਪਸ {};
}

std::error_ਕੋਡ FileCollector::copyFiles(ਤਾਰਕਿਕ StopOnError) {
  ਸਵੈਚਾਲਿਤ Err = sys::fs::create_directories(ਜੜ੍ਹ_2, /*IgnoreExisting=*/ਸੱਚ);
  ਜੇ (Err) {
    ਵਾਪਸ Err;
  }

  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ(Mutex);

  ਲਈ (ਸਵੈਚਾਲਿਤ &ਇੰਦਰਾਜ_2 : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_ਸਥਿਤੀ Stat;
    ਜੇ (std::error_ਕੋਡ EC = sys::fs::ਸਥਿਤੀ_2(ਇੰਦਰਾਜ_2.VPath, Stat)) {
      ਜੇ (StopOnError)
        ਵਾਪਸ EC;
      ਜਾਰੀ;
    }

    // Continue if the file doesn't exist.
    ਜੇ (Stat.type() == sys::fs::file_type::file_not_ਮਿਲਿਆ)
      ਜਾਰੀ;

    // Create directory tree.
    ਜੇ (std::error_ਕੋਡ EC =
            sys::fs::create_directories(sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(ਇੰਦਰਾਜ_2.RPath),
                                        /*IgnoreExisting=*/ਸੱਚ)) {
      ਜੇ (StopOnError)
        ਵਾਪਸ EC;
    }

    ਜੇ (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      ਜੇ (std::error_ਕੋਡ EC =
              sys::fs::create_directories(ਇੰਦਰਾਜ_2.RPath,
                                          /*IgnoreExisting=*/ਸੱਚ)) {
        ਜੇ (StopOnError)
          ਵਾਪਸ EC;
      }
      ਜਾਰੀ;
    }

    // Copy file over.
    ਜੇ (std::error_ਕੋਡ EC = sys::fs::copy_file(ਇੰਦਰਾਜ_2.VPath, ਇੰਦਰਾਜ_2.RPath)) {
      ਜੇ (StopOnError)
        ਵਾਪਸ EC;
    }

    // Copy over permissions.
    ਜੇ (ਸਵੈਚਾਲਿਤ perms = sys::fs::getPermissions(ਇੰਦਰਾਜ_2.VPath)) {
      ਜੇ (std::error_ਕੋਡ EC = sys::fs::ਸੈੱਟ_Permissions(ਇੰਦਰਾਜ_2.RPath, *perms)) {
        ਜੇ (StopOnError)
          ਵਾਪਸ EC;
      }
    }

    // Copy over modification time.
    copy_Access_And_Modification_ਸਮਾਂ(ਇੰਦਰਾਜ_2.RPath, Stat);
  }
  ਵਾਪਸ {};
}

std::error_ਕੋਡ FileCollector::writeMapping(StringRef MappingFile) {
  std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ(Mutex);

  VFSWriter.ਸੈੱਟ_Overlay_Dir(Overlay_ਜੜ੍ਹ);
  VFSWriter.ਸੈੱਟ_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ(is_Case_Sensitive_ਰਸਤਾ(Overlay_ਜੜ੍ਹ));
  VFSWriter.ਸੈੱਟ_Use_External_ਨਾਂ(ਝੂਠ);

  std::error_ਕੋਡ EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::ਦਾ_ਪਾਠ_With_CRLF);
  ਜੇ (EC)
    ਵਾਪਸ EC;

  VFSWriter.ਲਿਖ(os);

  ਵਾਪਸ {};
}

ਨਾਮਾਕਾਸ਼ llvm {

ਵਰਗ FileCollectorFileSystem : ਜਨਤਕ vfs::FileSystem {
ਜਨਤਕ:
  ਸਪਸ਼ਟ FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::ਸਥਿਤੀ> ਸਥਿਤੀ_2(ਅਚਲ Twine &ਰਸਤਾ) override {
    ਸਵੈਚਾਲਿਤ ਨਤੀਜਾ = FS->ਸਥਿਤੀ_2(ਰਸਤਾ);
    ਜੇ (ਨਤੀਜਾ && ਨਤੀਜਾ->exists())
      Collector->ਜੋੜੋ_File(ਰਸਤਾ);
    ਵਾਪਸ ਨਤੀਜਾ;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ਅਚਲ Twine &ਰਸਤਾ) override {
    ਸਵੈਚਾਲਿਤ ਨਤੀਜਾ = FS->openFileForRead(ਰਸਤਾ);
    ਜੇ (ਨਤੀਜਾ && *ਨਤੀਜਾ)
      Collector->ਜੋੜੋ_File(ਰਸਤਾ);
    ਵਾਪਸ ਨਤੀਜਾ;
  }

  llvm::vfs::directory_iterator dir_begin(ਅਚਲ llvm::Twine &Dir,
                                          std::error_ਕੋਡ &EC) override {
    ਵਾਪਸ Collector->ਜੋੜੋ_Directory_Impl(Dir, FS, EC);
  }

  std::error_ਕੋਡ get_Real_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ,
                              Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Output) override {
    ਸਵੈਚਾਲਿਤ EC = FS->get_Real_ਰਸਤਾ(ਰਸਤਾ, Output);
    ਜੇ (!EC) {
      Collector->ਜੋੜੋ_File(ਰਸਤਾ);
      ਜੇ (Output.ਮਾਪ_2() > 0)
        Collector->ਜੋੜੋ_File(Output);
    }
    ਵਾਪਸ EC;
  }

  ਖਾਲੀ
  getDirectoryContentRealSources(ਅਚਲ Twine &Dir,
                                 Small_ਸਦਿਸ਼_Impl<std::string> &ਬਾਹਰਲਾ) override {
    FS->getDirectoryContentRealSources(Dir, ਬਾਹਰਲਾ);
  }

  std::error_ਕੋਡ isLocal(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ &ਨਤੀਜਾ) override {
    ਵਾਪਸ FS->isLocal(ਰਸਤਾ, ਨਤੀਜਾ);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ਅਚਲ override {
    ਵਾਪਸ FS->getCurrentWorkingDirectory();
  }

  std::error_ਕੋਡ ਸੈੱਟ_Current_Working_Directory(ਅਚਲ llvm::Twine &ਰਸਤਾ) override {
    ਵਾਪਸ FS->ਸੈੱਟ_Current_Working_Directory(ਰਸਤਾ);
  }

ਨਿਜੀ:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> ਅਧਾਰ_FS,
                                  std::shared_ptr<FileCollector> Collector) {
  ਵਾਪਸ makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(ਅਧਾਰ_FS),
                                                      std::move(Collector));
}
