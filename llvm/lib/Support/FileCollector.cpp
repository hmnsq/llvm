//===-- FileCollector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/FileCollector.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Process.h"

استعمال نامخلا llvm;

File_Collector_بنیاد::File_Collector_بنیاد() = طےشدہ;
File_Collector_بنیاد::~File_Collector_بنیاد() = طےشدہ;

خالی File_Collector_بنیاد::جوڑو_File(ثابت Twine &File) {
  std::تالا_guard<std::mutex> تالا_2(Mutex);
  std::string FileStr = File.str();
  اگر (mark_As_نظر_آیا(FileStr))
    جوڑو_File_Impl(FileStr);
}

خالی File_Collector_بنیاد::جوڑو_Directory(ثابت Twine &Dir) {
  أكد(sys::fs::is_directory(Dir));
  std::error_رمز EC;
  جوڑو_Directory_Impl(Dir, vfs::getRealFileSystem(), EC);
}

ساکن منطقی is_Case_Sensitive_راستہ(StringRef راستہ) {
  SmallString<256> TmpDest = راستہ, بالائی_Dest, RealDest;

  // Remove component traversals, links, etc.
  اگر (sys::fs::real_راستہ(راستہ, TmpDest))
    واپس سچ; // Current default value in vfs.yaml
  راستہ = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  بالائی_Dest = راستہ.بالائی();
  اگر (!sys::fs::real_راستہ(بالائی_Dest, RealDest) && راستہ == RealDest)
    واپس جھوٹ;
  واپس سچ;
}

FileCollector::FileCollector(std::string جڑ_2, std::string Overlay_جڑ,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : جڑ_2(جڑ_2), Overlay_جڑ(Overlay_جڑ), Canonicalizer(std::move(VFS)) {
  أكد(sys::راستہ_2::is_absolute(جڑ_2) && "Root not absolute");
  أكد(sys::راستہ_2::is_absolute(Overlay_جڑ) && "OverlayRoot not absolute");
}

خالی FileCollector::راستہ_Canonicalizer::update_With_Real_راستہ(
    Small_سمتیہ_Impl<حرف> &راستہ) {
  StringRef Src_راستہ(راستہ.begin(), راستہ.سائز_2());
  StringRef Filename = sys::راستہ_2::filename(Src_راستہ);
  StringRef Directory = sys::راستہ_2::والد_راستہ(Src_راستہ);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> Real_راستہ;
  خودکار DirWithSymlink = CachedDirs.ڈھونڈو(Directory);
  اگر (DirWithSymlink == CachedDirs.انتہا()) {
    // FIXME: What if there is nothing on disk?
    اگر (VFS->get_Real_راستہ(Directory, Real_راستہ))
      واپس;
    CachedDirs[Directory] = std::string(Real_راستہ);
  } ورنہ {
    Real_راستہ = DirWithSymlink->دوسرا_2;
  }

  // Finish recreating the path by appending the original filename, since we
  // don't need to resolve symlinks in the filename.
  //
  // FIXME: If we can cope with this, maybe we can cope without calling
  // getRealPath() at all when there's no ".." component.
  sys::راستہ_2::append(Real_راستہ, Filename);

  // Swap to create the output.
  راستہ.تبادلہ(Real_راستہ);
}

/// Make Path absolute.
ساکن خالی makeAbsolute(vfs::FileSystem &VFS, Small_سمتیہ_Impl<حرف> &راستہ) {
  // We need an absolute src path to append to the root.
  VFS.makeAbsolute(راستہ);

  // Canonicalize src to a native path to avoid mixed separator styles.
  sys::راستہ_2::native(راستہ);

  // Remove redundant leading "./" pieces and consecutive separators.
  راستہ.erase(راستہ.begin(), sys::راستہ_2::remove_leading_dotslash(
                               StringRef(راستہ.begin(), راستہ.سائز_2()))
                               .begin());
}

FileCollector::راستہ_Canonicalizer::راستہ_Storage
FileCollector::راستہ_Canonicalizer::canonicalize(StringRef Src_راستہ) {
  راستہ_Storage Paths;
  Paths.Virtual_راستہ = Src_راستہ;
  makeAbsolute(*VFS, Paths.Virtual_راستہ);

  // If a ".." component is present after a symlink component, remove_dots may
  // lead to the wrong real destination path. Let the source be canonicalized
  // like that but make sure we always use the real path for the destination.
  Paths.CopyFrom = Paths.Virtual_راستہ;
  update_With_Real_راستہ(Paths.CopyFrom);

  // Canonicalize the virtual path by removing "..", "." components.
  sys::راستہ_2::remove_dots(Paths.Virtual_راستہ, /*remove_dot_dot=*/سچ);

  واپس Paths;
}

خالی FileCollector::جوڑو_File_Impl(StringRef Src_راستہ) {
  راستہ_Canonicalizer::راستہ_Storage Paths = Canonicalizer.canonicalize(Src_راستہ);

  SmallString<256> Dst_راستہ = StringRef(جڑ_2);
  sys::راستہ_2::append(Dst_راستہ, sys::راستہ_2::relative_راستہ(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  جوڑو_File_مقصد_Mapping(Paths.Virtual_راستہ, Dst_راستہ);
}

llvm::vfs::directory_iterator
FileCollector::جوڑو_Directory_Impl(ثابت llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_رمز &EC) {
  خودکار It = FS->dir_begin(Dir, EC);
  اگر (EC)
    واپس It;
  جوڑو_File(Dir);
  برائے (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    اگر (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      جوڑو_File(It->راستہ_2());
    }
  }
  اگر (EC)
    واپس It;
  // Return a new iterator.
  واپس FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
ساکن std::error_رمز
copy_Access_And_Modification_وقت(StringRef Filename,
                              ثابت sys::fs::file_حالت &Stat) {
  صحیح FD;

  اگر (خودکار EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    واپس EC;

  اگر (خودکار EC = sys::fs::سیٹ_آخری_Access_And_Modification_وقت(
          FD, Stat.get_آخری_Accessed_وقت(), Stat.get_آخری_Modification_وقت()))
    واپس EC;

  اگر (خودکار EC = sys::Process::SafelyCloseFileDescriptor(FD))
    واپس EC;

  واپس {};
}

std::error_رمز FileCollector::copyFiles(منطقی StopOnError) {
  خودکار Err = sys::fs::create_directories(جڑ_2, /*IgnoreExisting=*/سچ);
  اگر (Err) {
    واپس Err;
  }

  std::تالا_guard<std::mutex> تالا_2(Mutex);

  برائے (خودکار &اندراج_3 : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_حالت Stat;
    اگر (std::error_رمز EC = sys::fs::حالت_3(اندراج_3.VPath, Stat)) {
      اگر (StopOnError)
        واپس EC;
      جاری;
    }

    // Continue if the file doesn't exist.
    اگر (Stat.type() == sys::fs::file_type::file_not_ملا)
      جاری;

    // Create directory tree.
    اگر (std::error_رمز EC =
            sys::fs::create_directories(sys::راستہ_2::والد_راستہ(اندراج_3.RPath),
                                        /*IgnoreExisting=*/سچ)) {
      اگر (StopOnError)
        واپس EC;
    }

    اگر (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      اگر (std::error_رمز EC =
              sys::fs::create_directories(اندراج_3.RPath,
                                          /*IgnoreExisting=*/سچ)) {
        اگر (StopOnError)
          واپس EC;
      }
      جاری;
    }

    // Copy file over.
    اگر (std::error_رمز EC = sys::fs::copy_file(اندراج_3.VPath, اندراج_3.RPath)) {
      اگر (StopOnError)
        واپس EC;
    }

    // Copy over permissions.
    اگر (خودکار perms = sys::fs::getPermissions(اندراج_3.VPath)) {
      اگر (std::error_رمز EC = sys::fs::سیٹ_Permissions(اندراج_3.RPath, *perms)) {
        اگر (StopOnError)
          واپس EC;
      }
    }

    // Copy over modification time.
    copy_Access_And_Modification_وقت(اندراج_3.RPath, Stat);
  }
  واپس {};
}

std::error_رمز FileCollector::writeMapping(StringRef MappingFile) {
  std::تالا_guard<std::mutex> تالا_2(Mutex);

  VFSWriter.سیٹ_Overlay_Dir(Overlay_جڑ);
  VFSWriter.سیٹ_Case_حساسیت(is_Case_Sensitive_راستہ(Overlay_جڑ));
  VFSWriter.سیٹ_Use_External_نام(جھوٹ);

  std::error_رمز EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::کا_متن_With_CRLF);
  اگر (EC)
    واپس EC;

  VFSWriter.اكتب(os);

  واپس {};
}

نامخلا llvm {

جماعت FileCollectorFileSystem : عوامی vfs::FileSystem {
عوامی:
  صریح FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::حالت_2> حالت_3(ثابت Twine &راستہ) override {
    خودکار نتیجہ = FS->حالت_3(راستہ);
    اگر (نتیجہ && نتیجہ->exists())
      Collector->جوڑو_File(راستہ);
    واپس نتیجہ;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ثابت Twine &راستہ) override {
    خودکار نتیجہ = FS->openFileForRead(راستہ);
    اگر (نتیجہ && *نتیجہ)
      Collector->جوڑو_File(راستہ);
    واپس نتیجہ;
  }

  llvm::vfs::directory_iterator dir_begin(ثابت llvm::Twine &Dir,
                                          std::error_رمز &EC) override {
    واپس Collector->جوڑو_Directory_Impl(Dir, FS, EC);
  }

  std::error_رمز get_Real_راستہ(ثابت Twine &راستہ,
                              Small_سمتیہ_Impl<حرف> &Output) override {
    خودکار EC = FS->get_Real_راستہ(راستہ, Output);
    اگر (!EC) {
      Collector->جوڑو_File(راستہ);
      اگر (Output.سائز_2() > 0)
        Collector->جوڑو_File(Output);
    }
    واپس EC;
  }

  خالی
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_سمتیہ_Impl<std::string> &برآمد) override {
    FS->getDirectoryContentRealSources(Dir, برآمد);
  }

  std::error_رمز isLocal(ثابت Twine &راستہ, منطقی &نتیجہ) override {
    واپس FS->isLocal(راستہ, نتیجہ);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override {
    واپس FS->getCurrentWorkingDirectory();
  }

  std::error_رمز سیٹ_Current_Working_Directory(ثابت llvm::Twine &راستہ) override {
    واپس FS->سیٹ_Current_Working_Directory(راستہ);
  }

نجی:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> بنیاد_FS,
                                  std::shared_ptr<FileCollector> Collector) {
  واپس makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(بنیاد_FS),
                                                      std::move(Collector));
}
