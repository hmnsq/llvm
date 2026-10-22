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

استفاده نطاق llvm;

File_Collector_پایه::File_Collector_پایه() = افتراضي;
File_Collector_پایه::~File_Collector_پایه() = افتراضي;

فراغ File_Collector_پایه::جمع_کن_File(ثابت Twine &File) {
  std::قفل_guard<std::mutex> قفل_2(Mutex);
  std::string FileStr = File.str();
  إذا (mark_As_دیده(FileStr))
    جمع_کن_File_Impl(FileStr);
}

فراغ File_Collector_پایه::جمع_کن_Directory(ثابت Twine &Dir) {
  أكد(sys::fs::is_directory(Dir));
  std::error_کد EC;
  جمع_کن_Directory_Impl(Dir, vfs::getRealFileSystem(), EC);
}

ساكن منطقي is_Case_Sensitive_مسیر(StringRef مسیر) {
  SmallString<256> TmpDest = مسیر, فوقانی_Dest, RealDest;

  // Remove component traversals, links, etc.
  إذا (sys::fs::real_مسیر(مسیر, TmpDest))
    أرجع درست; // Current default value in vfs.yaml
  مسیر = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  فوقانی_Dest = مسیر.فوقانی();
  إذا (!sys::fs::real_مسیر(فوقانی_Dest, RealDest) && مسیر == RealDest)
    أرجع خطأ;
  أرجع درست;
}

FileCollector::FileCollector(std::string ریشه_2, std::string Overlay_ریشه,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : ریشه_2(ریشه_2), Overlay_ریشه(Overlay_ریشه), Canonicalizer(std::move(VFS)) {
  أكد(sys::مسیر_2::is_absolute(ریشه_2) && "Root not absolute");
  أكد(sys::مسیر_2::is_absolute(Overlay_ریشه) && "OverlayRoot not absolute");
}

فراغ FileCollector::مسیر_Canonicalizer::update_With_Real_مسیر(
    Small_وکتور_Impl<حرف> &مسیر) {
  StringRef Src_مسیر(مسیر.begin(), مسیر.اندازه_2());
  StringRef Filename = sys::مسیر_2::filename(Src_مسیر);
  StringRef Directory = sys::مسیر_2::والد_مسیر(Src_مسیر);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> Real_مسیر;
  خودکار DirWithSymlink = CachedDirs.پیدا_کن(Directory);
  إذا (DirWithSymlink == CachedDirs.انتها()) {
    // FIXME: What if there is nothing on disk?
    إذا (VFS->get_Real_مسیر(Directory, Real_مسیر))
      أرجع;
    CachedDirs[Directory] = std::string(Real_مسیر);
  } وإلا {
    Real_مسیر = DirWithSymlink->دومی_2;
  }

  // Finish recreating the path by appending the original filename, since we
  // don't need to resolve symlinks in the filename.
  //
  // FIXME: If we can cope with this, maybe we can cope without calling
  // getRealPath() at all when there's no ".." component.
  sys::مسیر_2::append(Real_مسیر, Filename);

  // Swap to create the output.
  مسیر.جابجا(Real_مسیر);
}

/// Make Path absolute.
ساكن فراغ makeAbsolute(vfs::FileSystem &VFS, Small_وکتور_Impl<حرف> &مسیر) {
  // We need an absolute src path to append to the root.
  VFS.makeAbsolute(مسیر);

  // Canonicalize src to a native path to avoid mixed separator styles.
  sys::مسیر_2::native(مسیر);

  // Remove redundant leading "./" pieces and consecutive separators.
  مسیر.erase(مسیر.begin(), sys::مسیر_2::remove_leading_dotslash(
                               StringRef(مسیر.begin(), مسیر.اندازه_2()))
                               .begin());
}

FileCollector::مسیر_Canonicalizer::مسیر_Storage
FileCollector::مسیر_Canonicalizer::canonicalize(StringRef Src_مسیر) {
  مسیر_Storage Paths;
  Paths.Virtual_مسیر = Src_مسیر;
  makeAbsolute(*VFS, Paths.Virtual_مسیر);

  // If a ".." component is present after a symlink component, remove_dots may
  // lead to the wrong real destination path. Let the source be canonicalized
  // like that but make sure we always use the real path for the destination.
  Paths.CopyFrom = Paths.Virtual_مسیر;
  update_With_Real_مسیر(Paths.CopyFrom);

  // Canonicalize the virtual path by removing "..", "." components.
  sys::مسیر_2::remove_dots(Paths.Virtual_مسیر, /*remove_dot_dot=*/درست);

  أرجع Paths;
}

فراغ FileCollector::جمع_کن_File_Impl(StringRef Src_مسیر) {
  مسیر_Canonicalizer::مسیر_Storage Paths = Canonicalizer.canonicalize(Src_مسیر);

  SmallString<256> Dst_مسیر = StringRef(ریشه_2);
  sys::مسیر_2::append(Dst_مسیر, sys::مسیر_2::relative_مسیر(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  جمع_کن_File_مقصد_Mapping(Paths.Virtual_مسیر, Dst_مسیر);
}

llvm::vfs::directory_iterator
FileCollector::جمع_کن_Directory_Impl(ثابت llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_کد &EC) {
  خودکار It = FS->dir_begin(Dir, EC);
  إذا (EC)
    أرجع It;
  جمع_کن_File(Dir);
  لكل (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    إذا (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      جمع_کن_File(It->مسیر_2());
    }
  }
  إذا (EC)
    أرجع It;
  // Return a new iterator.
  أرجع FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
ساكن std::error_کد
copy_Access_And_Modification_زمان(StringRef Filename,
                              ثابت sys::fs::file_وضعیت &Stat) {
  صحيح FD;

  إذا (خودکار EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    أرجع EC;

  إذا (خودکار EC = sys::fs::مجموعه_آخرین_Access_And_Modification_زمان(
          FD, Stat.get_آخرین_Accessed_زمان(), Stat.get_آخرین_Modification_زمان()))
    أرجع EC;

  إذا (خودکار EC = sys::Process::SafelyCloseFileDescriptor(FD))
    أرجع EC;

  أرجع {};
}

std::error_کد FileCollector::copyFiles(منطقي StopOnError) {
  خودکار Err = sys::fs::create_directories(ریشه_2, /*IgnoreExisting=*/درست);
  إذا (Err) {
    أرجع Err;
  }

  std::قفل_guard<std::mutex> قفل_2(Mutex);

  لكل (خودکار &مدخل_2 : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_وضعیت Stat;
    إذا (std::error_کد EC = sys::fs::وضعیت_2(مدخل_2.VPath, Stat)) {
      إذا (StopOnError)
        أرجع EC;
      استمر;
    }

    // Continue if the file doesn't exist.
    إذا (Stat.type() == sys::fs::file_type::file_not_یافت)
      استمر;

    // Create directory tree.
    إذا (std::error_کد EC =
            sys::fs::create_directories(sys::مسیر_2::والد_مسیر(مدخل_2.RPath),
                                        /*IgnoreExisting=*/درست)) {
      إذا (StopOnError)
        أرجع EC;
    }

    إذا (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      إذا (std::error_کد EC =
              sys::fs::create_directories(مدخل_2.RPath,
                                          /*IgnoreExisting=*/درست)) {
        إذا (StopOnError)
          أرجع EC;
      }
      استمر;
    }

    // Copy file over.
    إذا (std::error_کد EC = sys::fs::copy_file(مدخل_2.VPath, مدخل_2.RPath)) {
      إذا (StopOnError)
        أرجع EC;
    }

    // Copy over permissions.
    إذا (خودکار perms = sys::fs::getPermissions(مدخل_2.VPath)) {
      إذا (std::error_کد EC = sys::fs::مجموعه_Permissions(مدخل_2.RPath, *perms)) {
        إذا (StopOnError)
          أرجع EC;
      }
    }

    // Copy over modification time.
    copy_Access_And_Modification_زمان(مدخل_2.RPath, Stat);
  }
  أرجع {};
}

std::error_کد FileCollector::writeMapping(StringRef MappingFile) {
  std::قفل_guard<std::mutex> قفل_2(Mutex);

  VFSWriter.مجموعه_Overlay_Dir(Overlay_ریشه);
  VFSWriter.مجموعه_Case_حساسیت(is_Case_Sensitive_مسیر(Overlay_ریشه));
  VFSWriter.مجموعه_Use_External_نامها(خطأ);

  std::error_کد EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::از_متن_With_CRLF);
  إذا (EC)
    أرجع EC;

  VFSWriter.اكتب(os);

  أرجع {};
}

نطاق llvm {

صنف FileCollectorFileSystem : عام vfs::FileSystem {
عام:
  صریح FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::وضعیت> وضعیت_2(ثابت Twine &مسیر) override {
    خودکار نتیجه = FS->وضعیت_2(مسیر);
    إذا (نتیجه && نتیجه->exists())
      Collector->جمع_کن_File(مسیر);
    أرجع نتیجه;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ثابت Twine &مسیر) override {
    خودکار نتیجه = FS->openFileForRead(مسیر);
    إذا (نتیجه && *نتیجه)
      Collector->جمع_کن_File(مسیر);
    أرجع نتیجه;
  }

  llvm::vfs::directory_iterator dir_begin(ثابت llvm::Twine &Dir,
                                          std::error_کد &EC) override {
    أرجع Collector->جمع_کن_Directory_Impl(Dir, FS, EC);
  }

  std::error_کد get_Real_مسیر(ثابت Twine &مسیر,
                              Small_وکتور_Impl<حرف> &Output) override {
    خودکار EC = FS->get_Real_مسیر(مسیر, Output);
    إذا (!EC) {
      Collector->جمع_کن_File(مسیر);
      إذا (Output.اندازه_2() > 0)
        Collector->جمع_کن_File(Output);
    }
    أرجع EC;
  }

  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_وکتور_Impl<std::string> &بیرون) override {
    FS->getDirectoryContentRealSources(Dir, بیرون);
  }

  std::error_کد isLocal(ثابت Twine &مسیر, منطقي &نتیجه) override {
    أرجع FS->isLocal(مسیر, نتیجه);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override {
    أرجع FS->getCurrentWorkingDirectory();
  }

  std::error_کد مجموعه_Current_Working_Directory(ثابت llvm::Twine &مسیر) override {
    أرجع FS->مجموعه_Current_Working_Directory(مسیر);
  }

خاص:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> پایه_FS,
                                  std::shared_ptr<FileCollector> Collector) {
  أرجع makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(پایه_FS),
                                                      std::move(Collector));
}
