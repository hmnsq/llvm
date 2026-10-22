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

استعمال نطاق llvm;

File_Collector_بنياد::File_Collector_بنياد() = افتراضي;
File_Collector_بنياد::~File_Collector_بنياد() = افتراضي;

فراغ File_Collector_بنياد::جوڙيو_File(ثابت Twine &File) {
  std::تالو_guard<std::mutex> تالو(Mutex);
  std::string FileStr = File.str();
  إذا (mark_As_ڏٺل(FileStr))
    جوڙيو_File_Impl(FileStr);
}

فراغ File_Collector_بنياد::جوڙيو_Directory(ثابت Twine &Dir) {
  أكد(sys::fs::is_directory(Dir));
  std::error_ڪوڊ EC;
  جوڙيو_Directory_Impl(Dir, vfs::getRealFileSystem(), EC);
}

ساكن منطقي is_Case_Sensitive_رستو(StringRef رستو) {
  SmallString<256> TmpDest = رستو, مٿانهون_Dest, RealDest;

  // Remove component traversals, links, etc.
  إذا (sys::fs::real_رستو(رستو, TmpDest))
    أرجع سچ; // Current default value in vfs.yaml
  رستو = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  مٿانهون_Dest = رستو.مٿانهون();
  إذا (!sys::fs::real_رستو(مٿانهون_Dest, RealDest) && رستو == RealDest)
    أرجع خطأ;
  أرجع سچ;
}

FileCollector::FileCollector(std::string پاڙ_2, std::string Overlay_پاڙ,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : پاڙ_2(پاڙ_2), Overlay_پاڙ(Overlay_پاڙ), Canonicalizer(std::move(VFS)) {
  أكد(sys::رستو_2::is_absolute(پاڙ_2) && "Root not absolute");
  أكد(sys::رستو_2::is_absolute(Overlay_پاڙ) && "OverlayRoot not absolute");
}

فراغ FileCollector::رستو_Canonicalizer::update_With_Real_رستو(
    Small_ويڪٽر_Impl<حرف> &رستو) {
  StringRef Src_رستو(رستو.begin(), رستو.سائيز_2());
  StringRef Filename = sys::رستو_2::filename(Src_رستو);
  StringRef Directory = sys::رستو_2::والد_رستو(Src_رستو);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> Real_رستو;
  خودڪار DirWithSymlink = CachedDirs.ڳوليو(Directory);
  إذا (DirWithSymlink == CachedDirs.پڇاڙي()) {
    // FIXME: What if there is nothing on disk?
    إذا (VFS->get_Real_رستو(Directory, Real_رستو))
      أرجع;
    CachedDirs[Directory] = std::string(Real_رستو);
  } وإلا {
    Real_رستو = DirWithSymlink->ٻيو_2;
  }

  // Finish recreating the path by appending the original filename, since we
  // don't need to resolve symlinks in the filename.
  //
  // FIXME: If we can cope with this, maybe we can cope without calling
  // getRealPath() at all when there's no ".." component.
  sys::رستو_2::append(Real_رستو, Filename);

  // Swap to create the output.
  رستو.مٽا_سٽا(Real_رستو);
}

/// Make Path absolute.
ساكن فراغ makeAbsolute(vfs::FileSystem &VFS, Small_ويڪٽر_Impl<حرف> &رستو) {
  // We need an absolute src path to append to the root.
  VFS.makeAbsolute(رستو);

  // Canonicalize src to a native path to avoid mixed separator styles.
  sys::رستو_2::native(رستو);

  // Remove redundant leading "./" pieces and consecutive separators.
  رستو.erase(رستو.begin(), sys::رستو_2::remove_leading_dotslash(
                               StringRef(رستو.begin(), رستو.سائيز_2()))
                               .begin());
}

FileCollector::رستو_Canonicalizer::رستو_Storage
FileCollector::رستو_Canonicalizer::canonicalize(StringRef Src_رستو) {
  رستو_Storage Paths;
  Paths.Virtual_رستو = Src_رستو;
  makeAbsolute(*VFS, Paths.Virtual_رستو);

  // If a ".." component is present after a symlink component, remove_dots may
  // lead to the wrong real destination path. Let the source be canonicalized
  // like that but make sure we always use the real path for the destination.
  Paths.CopyFrom = Paths.Virtual_رستو;
  update_With_Real_رستو(Paths.CopyFrom);

  // Canonicalize the virtual path by removing "..", "." components.
  sys::رستو_2::remove_dots(Paths.Virtual_رستو, /*remove_dot_dot=*/سچ);

  أرجع Paths;
}

فراغ FileCollector::جوڙيو_File_Impl(StringRef Src_رستو) {
  رستو_Canonicalizer::رستو_Storage Paths = Canonicalizer.canonicalize(Src_رستو);

  SmallString<256> Dst_رستو = StringRef(پاڙ_2);
  sys::رستو_2::append(Dst_رستو, sys::رستو_2::relative_رستو(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  جوڙيو_File_منزل_Mapping(Paths.Virtual_رستو, Dst_رستو);
}

llvm::vfs::directory_iterator
FileCollector::جوڙيو_Directory_Impl(ثابت llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_ڪوڊ &EC) {
  خودڪار It = FS->dir_begin(Dir, EC);
  إذا (EC)
    أرجع It;
  جوڙيو_File(Dir);
  لكل (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    إذا (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      جوڙيو_File(It->رستو_2());
    }
  }
  إذا (EC)
    أرجع It;
  // Return a new iterator.
  أرجع FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
ساكن std::error_ڪوڊ
copy_Access_And_Modification_وقت(StringRef Filename,
                              ثابت sys::fs::file_حالت &Stat) {
  صحيح FD;

  إذا (خودڪار EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    أرجع EC;

  إذا (خودڪار EC = sys::fs::سيٽ_آخري_Access_And_Modification_وقت(
          FD, Stat.get_آخري_Accessed_وقت(), Stat.get_آخري_Modification_وقت()))
    أرجع EC;

  إذا (خودڪار EC = sys::Process::SafelyCloseFileDescriptor(FD))
    أرجع EC;

  أرجع {};
}

std::error_ڪوڊ FileCollector::copyFiles(منطقي StopOnError) {
  خودڪار Err = sys::fs::create_directories(پاڙ_2, /*IgnoreExisting=*/سچ);
  إذا (Err) {
    أرجع Err;
  }

  std::تالو_guard<std::mutex> تالو(Mutex);

  لكل (خودڪار &داخلا_4 : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_حالت Stat;
    إذا (std::error_ڪوڊ EC = sys::fs::حالت_2(داخلا_4.VPath, Stat)) {
      إذا (StopOnError)
        أرجع EC;
      استمر;
    }

    // Continue if the file doesn't exist.
    إذا (Stat.type() == sys::fs::file_type::file_not_مليو)
      استمر;

    // Create directory tree.
    إذا (std::error_ڪوڊ EC =
            sys::fs::create_directories(sys::رستو_2::والد_رستو(داخلا_4.RPath),
                                        /*IgnoreExisting=*/سچ)) {
      إذا (StopOnError)
        أرجع EC;
    }

    إذا (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      إذا (std::error_ڪوڊ EC =
              sys::fs::create_directories(داخلا_4.RPath,
                                          /*IgnoreExisting=*/سچ)) {
        إذا (StopOnError)
          أرجع EC;
      }
      استمر;
    }

    // Copy file over.
    إذا (std::error_ڪوڊ EC = sys::fs::copy_file(داخلا_4.VPath, داخلا_4.RPath)) {
      إذا (StopOnError)
        أرجع EC;
    }

    // Copy over permissions.
    إذا (خودڪار perms = sys::fs::getPermissions(داخلا_4.VPath)) {
      إذا (std::error_ڪوڊ EC = sys::fs::سيٽ_Permissions(داخلا_4.RPath, *perms)) {
        إذا (StopOnError)
          أرجع EC;
      }
    }

    // Copy over modification time.
    copy_Access_And_Modification_وقت(داخلا_4.RPath, Stat);
  }
  أرجع {};
}

std::error_ڪوڊ FileCollector::writeMapping(StringRef MappingFile) {
  std::تالو_guard<std::mutex> تالو(Mutex);

  VFSWriter.سيٽ_Overlay_Dir(Overlay_پاڙ);
  VFSWriter.سيٽ_Case_حساسيت(is_Case_Sensitive_رستو(Overlay_پاڙ));
  VFSWriter.سيٽ_Use_External_نالا(خطأ);

  std::error_ڪوڊ EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::جو_متن_With_CRLF);
  إذا (EC)
    أرجع EC;

  VFSWriter.اكتب(os);

  أرجع {};
}

نطاق llvm {

صنف FileCollectorFileSystem : عام vfs::FileSystem {
عام:
  واضح FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::حالت> حالت_2(ثابت Twine &رستو) override {
    خودڪار نتيجو = FS->حالت_2(رستو);
    إذا (نتيجو && نتيجو->exists())
      Collector->جوڙيو_File(رستو);
    أرجع نتيجو;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ثابت Twine &رستو) override {
    خودڪار نتيجو = FS->openFileForRead(رستو);
    إذا (نتيجو && *نتيجو)
      Collector->جوڙيو_File(رستو);
    أرجع نتيجو;
  }

  llvm::vfs::directory_iterator dir_begin(ثابت llvm::Twine &Dir,
                                          std::error_ڪوڊ &EC) override {
    أرجع Collector->جوڙيو_Directory_Impl(Dir, FS, EC);
  }

  std::error_ڪوڊ get_Real_رستو(ثابت Twine &رستو,
                              Small_ويڪٽر_Impl<حرف> &Output) override {
    خودڪار EC = FS->get_Real_رستو(رستو, Output);
    إذا (!EC) {
      Collector->جوڙيو_File(رستو);
      إذا (Output.سائيز_2() > 0)
        Collector->جوڙيو_File(Output);
    }
    أرجع EC;
  }

  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_ويڪٽر_Impl<std::string> &ٻاهريون) override {
    FS->getDirectoryContentRealSources(Dir, ٻاهريون);
  }

  std::error_ڪوڊ isLocal(ثابت Twine &رستو, منطقي &نتيجو) override {
    أرجع FS->isLocal(رستو, نتيجو);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override {
    أرجع FS->getCurrentWorkingDirectory();
  }

  std::error_ڪوڊ سيٽ_Current_Working_Directory(ثابت llvm::Twine &رستو) override {
    أرجع FS->سيٽ_Current_Working_Directory(رستو);
  }

خاص:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> بنياد_FS,
                                  std::shared_ptr<FileCollector> Collector) {
  أرجع makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(بنياد_FS),
                                                      std::move(Collector));
}
