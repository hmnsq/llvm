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

ورتوں نطاق llvm;

File_Collector_ادھار::File_Collector_ادھار() = افتراضي;
File_Collector_ادھار::~File_Collector_ادھار() = افتراضي;

فراغ File_Collector_ادھار::جوڑو_File(ثابت Twine &File) {
  std::جندرا_guard<std::mutex> جندرا(Mutex);
  std::string FileStr = File.str();
  إذا (mark_As_تکیا(FileStr))
    جوڑو_File_Impl(FileStr);
}

فراغ File_Collector_ادھار::جوڑو_Directory(ثابت Twine &Dir) {
  أكد(sys::fs::is_directory(Dir));
  std::error_کوڈ EC;
  جوڑو_Directory_Impl(Dir, vfs::getRealFileSystem(), EC);
}

ساكن منطقي is_Case_Sensitive_رستہ(StringRef رستہ) {
  SmallString<256> TmpDest = رستہ, اتلا_Dest, RealDest;

  // Remove component traversals, links, etc.
  إذا (sys::fs::real_رستہ(رستہ, TmpDest))
    أرجع سچ; // Current default value in vfs.yaml
  رستہ = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  اتلا_Dest = رستہ.اتلا();
  إذا (!sys::fs::real_رستہ(اتلا_Dest, RealDest) && رستہ == RealDest)
    أرجع خطأ;
  أرجع سچ;
}

FileCollector::FileCollector(std::string جڑ_2, std::string Overlay_جڑ,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : جڑ_2(جڑ_2), Overlay_جڑ(Overlay_جڑ), Canonicalizer(std::move(VFS)) {
  أكد(sys::رستہ_2::is_absolute(جڑ_2) && "Root not absolute");
  أكد(sys::رستہ_2::is_absolute(Overlay_جڑ) && "OverlayRoot not absolute");
}

فراغ FileCollector::رستہ_Canonicalizer::update_With_Real_رستہ(
    Small_سمتیہ_Impl<حرف> &رستہ) {
  StringRef Src_رستہ(رستہ.begin(), رستہ.ناپ_2());
  StringRef Filename = sys::رستہ_2::filename(Src_رستہ);
  StringRef Directory = sys::رستہ_2::ماپا_رستہ(Src_رستہ);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> Real_رستہ;
  خودکار DirWithSymlink = CachedDirs.لبھو(Directory);
  إذا (DirWithSymlink == CachedDirs.سرا()) {
    // FIXME: What if there is nothing on disk?
    إذا (VFS->get_Real_رستہ(Directory, Real_رستہ))
      أرجع;
    CachedDirs[Directory] = std::string(Real_رستہ);
  } وإلا {
    Real_رستہ = DirWithSymlink->دوجا_2;
  }

  // Finish recreating the path by appending the original filename, since we
  // don't need to resolve symlinks in the filename.
  //
  // FIXME: If we can cope with this, maybe we can cope without calling
  // getRealPath() at all when there's no ".." component.
  sys::رستہ_2::append(Real_رستہ, Filename);

  // Swap to create the output.
  رستہ.وٹاندرا(Real_رستہ);
}

/// Make Path absolute.
ساكن فراغ makeAbsolute(vfs::FileSystem &VFS, Small_سمتیہ_Impl<حرف> &رستہ) {
  // We need an absolute src path to append to the root.
  VFS.makeAbsolute(رستہ);

  // Canonicalize src to a native path to avoid mixed separator styles.
  sys::رستہ_2::native(رستہ);

  // Remove redundant leading "./" pieces and consecutive separators.
  رستہ.erase(رستہ.begin(), sys::رستہ_2::remove_leading_dotslash(
                               StringRef(رستہ.begin(), رستہ.ناپ_2()))
                               .begin());
}

FileCollector::رستہ_Canonicalizer::رستہ_Storage
FileCollector::رستہ_Canonicalizer::canonicalize(StringRef Src_رستہ) {
  رستہ_Storage Paths;
  Paths.Virtual_رستہ = Src_رستہ;
  makeAbsolute(*VFS, Paths.Virtual_رستہ);

  // If a ".." component is present after a symlink component, remove_dots may
  // lead to the wrong real destination path. Let the source be canonicalized
  // like that but make sure we always use the real path for the destination.
  Paths.CopyFrom = Paths.Virtual_رستہ;
  update_With_Real_رستہ(Paths.CopyFrom);

  // Canonicalize the virtual path by removing "..", "." components.
  sys::رستہ_2::remove_dots(Paths.Virtual_رستہ, /*remove_dot_dot=*/سچ);

  أرجع Paths;
}

فراغ FileCollector::جوڑو_File_Impl(StringRef Src_رستہ) {
  رستہ_Canonicalizer::رستہ_Storage Paths = Canonicalizer.canonicalize(Src_رستہ);

  SmallString<256> Dst_رستہ = StringRef(جڑ_2);
  sys::رستہ_2::append(Dst_رستہ, sys::رستہ_2::relative_رستہ(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  جوڑو_File_منزل_Mapping(Paths.Virtual_رستہ, Dst_رستہ);
}

llvm::vfs::directory_iterator
FileCollector::جوڑو_Directory_Impl(ثابت llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_کوڈ &EC) {
  خودکار It = FS->dir_begin(Dir, EC);
  إذا (EC)
    أرجع It;
  جوڑو_File(Dir);
  لكل (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    إذا (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      جوڑو_File(It->رستہ_2());
    }
  }
  إذا (EC)
    أرجع It;
  // Return a new iterator.
  أرجع FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
ساكن std::error_کوڈ
copy_Access_And_Modification_ویلا(StringRef Filename,
                              ثابت sys::fs::file_حالت &Stat) {
  صحيح FD;

  إذا (خودکار EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    أرجع EC;

  إذا (خودکار EC = sys::fs::سیٹ_آخری_Access_And_Modification_ویلا(
          FD, Stat.get_آخری_Accessed_ویلا(), Stat.get_آخری_Modification_ویلا()))
    أرجع EC;

  إذا (خودکار EC = sys::Process::SafelyCloseFileDescriptor(FD))
    أرجع EC;

  أرجع {};
}

std::error_کوڈ FileCollector::copyFiles(منطقي StopOnError) {
  خودکار Err = sys::fs::create_directories(جڑ_2, /*IgnoreExisting=*/سچ);
  إذا (Err) {
    أرجع Err;
  }

  std::جندرا_guard<std::mutex> جندرا(Mutex);

  لكل (خودکار &اندراج_2 : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_حالت Stat;
    إذا (std::error_کوڈ EC = sys::fs::حالت_2(اندراج_2.VPath, Stat)) {
      إذا (StopOnError)
        أرجع EC;
      استمر;
    }

    // Continue if the file doesn't exist.
    إذا (Stat.type() == sys::fs::file_type::file_not_لبھیا)
      استمر;

    // Create directory tree.
    إذا (std::error_کوڈ EC =
            sys::fs::create_directories(sys::رستہ_2::ماپا_رستہ(اندراج_2.RPath),
                                        /*IgnoreExisting=*/سچ)) {
      إذا (StopOnError)
        أرجع EC;
    }

    إذا (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      إذا (std::error_کوڈ EC =
              sys::fs::create_directories(اندراج_2.RPath,
                                          /*IgnoreExisting=*/سچ)) {
        إذا (StopOnError)
          أرجع EC;
      }
      استمر;
    }

    // Copy file over.
    إذا (std::error_کوڈ EC = sys::fs::copy_file(اندراج_2.VPath, اندراج_2.RPath)) {
      إذا (StopOnError)
        أرجع EC;
    }

    // Copy over permissions.
    إذا (خودکار perms = sys::fs::getPermissions(اندراج_2.VPath)) {
      إذا (std::error_کوڈ EC = sys::fs::سیٹ_Permissions(اندراج_2.RPath, *perms)) {
        إذا (StopOnError)
          أرجع EC;
      }
    }

    // Copy over modification time.
    copy_Access_And_Modification_ویلا(اندراج_2.RPath, Stat);
  }
  أرجع {};
}

std::error_کوڈ FileCollector::writeMapping(StringRef MappingFile) {
  std::جندرا_guard<std::mutex> جندرا(Mutex);

  VFSWriter.سیٹ_Overlay_Dir(Overlay_جڑ);
  VFSWriter.سیٹ_Case_حساسیت(is_Case_Sensitive_رستہ(Overlay_جڑ));
  VFSWriter.سیٹ_Use_External_ناں(خطأ);

  std::error_کوڈ EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::دا_متن_With_CRLF);
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

  llvm::ErrorOr<llvm::vfs::حالت> حالت_2(ثابت Twine &رستہ) override {
    خودکار نتیجہ = FS->حالت_2(رستہ);
    إذا (نتیجہ && نتیجہ->exists())
      Collector->جوڑو_File(رستہ);
    أرجع نتیجہ;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ثابت Twine &رستہ) override {
    خودکار نتیجہ = FS->openFileForRead(رستہ);
    إذا (نتیجہ && *نتیجہ)
      Collector->جوڑو_File(رستہ);
    أرجع نتیجہ;
  }

  llvm::vfs::directory_iterator dir_begin(ثابت llvm::Twine &Dir,
                                          std::error_کوڈ &EC) override {
    أرجع Collector->جوڑو_Directory_Impl(Dir, FS, EC);
  }

  std::error_کوڈ get_Real_رستہ(ثابت Twine &رستہ,
                              Small_سمتیہ_Impl<حرف> &Output) override {
    خودکار EC = FS->get_Real_رستہ(رستہ, Output);
    إذا (!EC) {
      Collector->جوڑو_File(رستہ);
      إذا (Output.ناپ_2() > 0)
        Collector->جوڑو_File(Output);
    }
    أرجع EC;
  }

  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_سمتیہ_Impl<std::string> &باہرلا) override {
    FS->getDirectoryContentRealSources(Dir, باہرلا);
  }

  std::error_کوڈ isLocal(ثابت Twine &رستہ, منطقي &نتیجہ) override {
    أرجع FS->isLocal(رستہ, نتیجہ);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override {
    أرجع FS->getCurrentWorkingDirectory();
  }

  std::error_کوڈ سیٹ_Current_Working_Directory(ثابت llvm::Twine &رستہ) override {
    أرجع FS->سیٹ_Current_Working_Directory(رستہ);
  }

خاص:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> ادھار_FS,
                                  std::shared_ptr<FileCollector> Collector) {
  أرجع makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(ادھار_FS),
                                                      std::move(Collector));
}
