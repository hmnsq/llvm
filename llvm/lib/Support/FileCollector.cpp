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

کارول نطاق llvm;

File_Collector_بنسټ::File_Collector_بنسټ() = افتراضي;
File_Collector_بنسټ::~File_Collector_بنسټ() = افتراضي;

فراغ File_Collector_بنسټ::جمع_کړه_File(ثابت Twine &File) {
  std::قلف_guard<std::mutex> قلف(Mutex);
  std::string FileStr = File.str();
  إذا (mark_As_کتل_شوی(FileStr))
    جمع_کړه_File_Impl(FileStr);
}

فراغ File_Collector_بنسټ::جمع_کړه_Directory(ثابت Twine &Dir) {
  أكد(sys::fs::is_directory(Dir));
  std::error_کوډ EC;
  جمع_کړه_Directory_Impl(Dir, vfs::getRealFileSystem(), EC);
}

ساكن منطقي is_Case_Sensitive_لار(StringRef لار) {
  SmallString<256> TmpDest = لار, پاسنی_Dest, RealDest;

  // Remove component traversals, links, etc.
  إذا (sys::fs::real_لار(لار, TmpDest))
    أرجع ریښتیا; // Current default value in vfs.yaml
  لار = TmpDest;

  // Change path to all upper case and ask for its real path, if the latter
  // exists and is equal to path, it's not case sensitive. Default to case
  // sensitive in the absence of real_path, since this is the YAMLVFSWriter
  // default.
  پاسنی_Dest = لار.پاسنی();
  إذا (!sys::fs::real_لار(پاسنی_Dest, RealDest) && لار == RealDest)
    أرجع خطأ;
  أرجع ریښتیا;
}

FileCollector::FileCollector(std::string ریښه_2, std::string Overlay_ریښه,
                             IntrusiveRefCntPtr<vfs::FileSystem> VFS)
    : ریښه_2(ریښه_2), Overlay_ریښه(Overlay_ریښه), Canonicalizer(std::move(VFS)) {
  أكد(sys::لار_2::is_absolute(ریښه_2) && "Root not absolute");
  أكد(sys::لار_2::is_absolute(Overlay_ریښه) && "OverlayRoot not absolute");
}

فراغ FileCollector::لار_Canonicalizer::update_With_Real_لار(
    Small_ویکټور_Impl<حرف> &لار) {
  StringRef Src_لار(لار.begin(), لار.اندازه_2());
  StringRef Filename = sys::لار_2::filename(Src_لار);
  StringRef Directory = sys::لار_2::پلار_لار(Src_لار);

  // Use real_path to fix any symbolic link component present in the directory
  // part of the path, caching the search because computing the real path is
  // expensive.
  SmallString<256> Real_لار;
  خپلکاری DirWithSymlink = CachedDirs.پیدا_کړه(Directory);
  إذا (DirWithSymlink == CachedDirs.پای_ټکی()) {
    // FIXME: What if there is nothing on disk?
    إذا (VFS->get_Real_لار(Directory, Real_لار))
      أرجع;
    CachedDirs[Directory] = std::string(Real_لار);
  } وإلا {
    Real_لار = DirWithSymlink->دوهم_2;
  }

  // Finish recreating the path by appending the original filename, since we
  // don't need to resolve symlinks in the filename.
  //
  // FIXME: If we can cope with this, maybe we can cope without calling
  // getRealPath() at all when there's no ".." component.
  sys::لار_2::append(Real_لار, Filename);

  // Swap to create the output.
  لار.بدلون(Real_لار);
}

/// Make Path absolute.
ساكن فراغ makeAbsolute(vfs::FileSystem &VFS, Small_ویکټور_Impl<حرف> &لار) {
  // We need an absolute src path to append to the root.
  VFS.makeAbsolute(لار);

  // Canonicalize src to a native path to avoid mixed separator styles.
  sys::لار_2::native(لار);

  // Remove redundant leading "./" pieces and consecutive separators.
  لار.erase(لار.begin(), sys::لار_2::remove_leading_dotslash(
                               StringRef(لار.begin(), لار.اندازه_2()))
                               .begin());
}

FileCollector::لار_Canonicalizer::لار_Storage
FileCollector::لار_Canonicalizer::canonicalize(StringRef Src_لار) {
  لار_Storage Paths;
  Paths.Virtual_لار = Src_لار;
  makeAbsolute(*VFS, Paths.Virtual_لار);

  // If a ".." component is present after a symlink component, remove_dots may
  // lead to the wrong real destination path. Let the source be canonicalized
  // like that but make sure we always use the real path for the destination.
  Paths.CopyFrom = Paths.Virtual_لار;
  update_With_Real_لار(Paths.CopyFrom);

  // Canonicalize the virtual path by removing "..", "." components.
  sys::لار_2::remove_dots(Paths.Virtual_لار, /*remove_dot_dot=*/ریښتیا);

  أرجع Paths;
}

فراغ FileCollector::جمع_کړه_File_Impl(StringRef Src_لار) {
  لار_Canonicalizer::لار_Storage Paths = Canonicalizer.canonicalize(Src_لار);

  SmallString<256> Dst_لار = StringRef(ریښه_2);
  sys::لار_2::append(Dst_لار, sys::لار_2::relative_لار(Paths.CopyFrom));

  // Always map a canonical src path to its real path into the YAML, by doing
  // this we map different virtual src paths to the same entry in the VFS
  // overlay, which is a way to emulate symlink inside the VFS; this is also
  // needed for correctness, not doing that can lead to module redefinition
  // errors.
  جمع_کړه_File_منزل_Mapping(Paths.Virtual_لار, Dst_لار);
}

llvm::vfs::directory_iterator
FileCollector::جمع_کړه_Directory_Impl(ثابت llvm::Twine &Dir,
                                IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                std::error_کوډ &EC) {
  خپلکاری It = FS->dir_begin(Dir, EC);
  إذا (EC)
    أرجع It;
  جمع_کړه_File(Dir);
  لكل (; !EC && It != llvm::vfs::directory_iterator(); It.increment(EC)) {
    إذا (It->type() == sys::fs::file_type::regular_file ||
        It->type() == sys::fs::file_type::directory_file ||
        It->type() == sys::fs::file_type::symlink_file) {
      جمع_کړه_File(It->لار_2());
    }
  }
  إذا (EC)
    أرجع It;
  // Return a new iterator.
  أرجع FS->dir_begin(Dir, EC);
}

/// Set the access and modification time for the given file from the given
/// status object.
ساكن std::error_کوډ
copy_Access_And_Modification_وخت(StringRef Filename,
                              ثابت sys::fs::file_حالت &Stat) {
  صحيح FD;

  إذا (خپلکاری EC =
          sys::fs::openFileForWrite(Filename, FD, sys::fs::CD_OpenExisting))
    أرجع EC;

  إذا (خپلکاری EC = sys::fs::سېټ_وروستی_Access_And_Modification_وخت(
          FD, Stat.get_وروستی_Accessed_وخت(), Stat.get_وروستی_Modification_وخت()))
    أرجع EC;

  إذا (خپلکاری EC = sys::Process::SafelyCloseFileDescriptor(FD))
    أرجع EC;

  أرجع {};
}

std::error_کوډ FileCollector::copyFiles(منطقي StopOnError) {
  خپلکاری Err = sys::fs::create_directories(ریښه_2, /*IgnoreExisting=*/ریښتیا);
  إذا (Err) {
    أرجع Err;
  }

  std::قلف_guard<std::mutex> قلف(Mutex);

  لكل (خپلکاری &ثبت_2 : VFSWriter.getMappings()) {
    // Get the status of the original file/directory.
    sys::fs::file_حالت Stat;
    إذا (std::error_کوډ EC = sys::fs::حالت_2(ثبت_2.VPath, Stat)) {
      إذا (StopOnError)
        أرجع EC;
      استمر;
    }

    // Continue if the file doesn't exist.
    إذا (Stat.type() == sys::fs::file_type::file_not_وموندل)
      استمر;

    // Create directory tree.
    إذا (std::error_کوډ EC =
            sys::fs::create_directories(sys::لار_2::پلار_لار(ثبت_2.RPath),
                                        /*IgnoreExisting=*/ریښتیا)) {
      إذا (StopOnError)
        أرجع EC;
    }

    إذا (Stat.type() == sys::fs::file_type::directory_file) {
      // Construct a directory when it's just a directory entry.
      إذا (std::error_کوډ EC =
              sys::fs::create_directories(ثبت_2.RPath,
                                          /*IgnoreExisting=*/ریښتیا)) {
        إذا (StopOnError)
          أرجع EC;
      }
      استمر;
    }

    // Copy file over.
    إذا (std::error_کوډ EC = sys::fs::copy_file(ثبت_2.VPath, ثبت_2.RPath)) {
      إذا (StopOnError)
        أرجع EC;
    }

    // Copy over permissions.
    إذا (خپلکاری perms = sys::fs::getPermissions(ثبت_2.VPath)) {
      إذا (std::error_کوډ EC = sys::fs::سېټ_Permissions(ثبت_2.RPath, *perms)) {
        إذا (StopOnError)
          أرجع EC;
      }
    }

    // Copy over modification time.
    copy_Access_And_Modification_وخت(ثبت_2.RPath, Stat);
  }
  أرجع {};
}

std::error_کوډ FileCollector::writeMapping(StringRef MappingFile) {
  std::قلف_guard<std::mutex> قلف(Mutex);

  VFSWriter.سېټ_Overlay_Dir(Overlay_ریښه);
  VFSWriter.سېټ_Case_حساسیت(is_Case_Sensitive_لار(Overlay_ریښه));
  VFSWriter.سېټ_Use_External_نومونه(خطأ);

  std::error_کوډ EC;
  raw_fd_ostream os(MappingFile, EC, sys::fs::د_متن_With_CRLF);
  إذا (EC)
    أرجع EC;

  VFSWriter.اكتب(os);

  أرجع {};
}

نطاق llvm {

صنف FileCollectorFileSystem : عام vfs::FileSystem {
عام:
  څرګند FileCollectorFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS,
                                   std::shared_ptr<FileCollector> Collector)
      : FS(std::move(FS)), Collector(std::move(Collector)) {}

  llvm::ErrorOr<llvm::vfs::حالت> حالت_2(ثابت Twine &لار) override {
    خپلکاری پایله = FS->حالت_2(لار);
    إذا (پایله && پایله->exists())
      Collector->جمع_کړه_File(لار);
    أرجع پایله;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(ثابت Twine &لار) override {
    خپلکاری پایله = FS->openFileForRead(لار);
    إذا (پایله && *پایله)
      Collector->جمع_کړه_File(لار);
    أرجع پایله;
  }

  llvm::vfs::directory_iterator dir_begin(ثابت llvm::Twine &Dir,
                                          std::error_کوډ &EC) override {
    أرجع Collector->جمع_کړه_Directory_Impl(Dir, FS, EC);
  }

  std::error_کوډ get_Real_لار(ثابت Twine &لار,
                              Small_ویکټور_Impl<حرف> &Output) override {
    خپلکاری EC = FS->get_Real_لار(لار, Output);
    إذا (!EC) {
      Collector->جمع_کړه_File(لار);
      إذا (Output.اندازه_2() > 0)
        Collector->جمع_کړه_File(Output);
    }
    أرجع EC;
  }

  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_ویکټور_Impl<std::string> &بهر) override {
    FS->getDirectoryContentRealSources(Dir, بهر);
  }

  std::error_کوډ isLocal(ثابت Twine &لار, منطقي &پایله) override {
    أرجع FS->isLocal(لار, پایله);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override {
    أرجع FS->getCurrentWorkingDirectory();
  }

  std::error_کوډ سېټ_Current_Working_Directory(ثابت llvm::Twine &لار) override {
    أرجع FS->سېټ_Current_Working_Directory(لار);
  }

خاص:
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  std::shared_ptr<FileCollector> Collector;
};

} // namespace llvm

IntrusiveRefCntPtr<vfs::FileSystem>
FileCollector::createCollectorVFS(IntrusiveRefCntPtr<vfs::FileSystem> بنسټ_FS,
                                  std::shared_ptr<FileCollector> Collector) {
  أرجع makeIntrusiveRefCnt<FileCollectorFileSystem>(std::move(بنسټ_FS),
                                                      std::move(Collector));
}
