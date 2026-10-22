//===- VirtualFileSystem.cpp - Virtual File System Layer ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the VirtualFileSystem interface.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/VirtualFileSystem.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/DenseMap.h"
#ضمن "llvm/ADT/IntrusiveRefCntPtr.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/StringSet.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/ADT/iterator_range.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/Casting.h"
#ضمن "llvm/Support/Chrono.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/ErrorOr.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/FileSystem/UniqueID.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/SMLoc.h"
#ضمن "llvm/Support/SourceMgr.h"
#ضمن "llvm/Support/YAMLParser.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <atomic>
#ضمن <cassert>
#ضمن <cstdint>
#ضمن <iterator>
#ضمن <limits>
#ضمن <map>
#ضمن <memory>
#ضمن <optional>
#ضمن <string>
#ضمن <system_error>
#ضمن <utility>
#ضمن <vector>

کارول نطاق llvm;
کارول نطاق llvm::vfs;

کارول llvm::sys::fs::file_حالت;
کارول llvm::sys::fs::file_t;
کارول llvm::sys::fs::file_type;
کارول llvm::sys::fs::perms;
کارول llvm::sys::fs::UniqueID;

حالت::حالت(ثابت file_حالت &حالت)
    : UID(حالت.getUniqueID()), MTime(حالت.get_وروستی_Modification_وخت()),
      User(حالت.getUser()), ډله(حالت.get_ډله()), اندازه(حالت.get_اندازه()),
      Type(حالت.type()), Perms(حالت.permissions()) {}

حالت::حالت(ثابت Twine &Name, UniqueID UID, sys::وخت_ټکی<> MTime,
               uint32_t User, uint32_t ډله, uint64_t اندازه, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), ډله(ډله),
      اندازه(اندازه), Type(Type), Perms(Perms) {}

حالت حالت::copy_With_New_اندازه(ثابت حالت &دننه_2, uint64_t New_اندازه) {
  أرجع حالت(دننه_2.getName(), دننه_2.getUniqueID(), دننه_2.get_وروستی_Modification_وخت(),
                دننه_2.getUser(), دننه_2.get_ډله(), New_اندازه, دننه_2.getType(),
                دننه_2.getPermissions());
}

حالت حالت::copyWithNewName(ثابت حالت &دننه_2, ثابت Twine &NewName) {
  أرجع حالت(NewName, دننه_2.getUniqueID(), دننه_2.get_وروستی_Modification_وخت(),
                دننه_2.getUser(), دننه_2.get_ډله(), دننه_2.get_اندازه(), دننه_2.getType(),
                دننه_2.getPermissions());
}

حالت حالت::copyWithNewName(ثابت file_حالت &دننه_2, ثابت Twine &NewName) {
  أرجع حالت(NewName, دننه_2.getUniqueID(), دننه_2.get_وروستی_Modification_وخت(),
                دننه_2.getUser(), دننه_2.get_ډله(), دننه_2.get_اندازه(), دننه_2.type(),
                دننه_2.permissions());
}

منطقي حالت::equivalent(ثابت حالت &Other) ثابت {
  أكد(is_حالت_معلوم() && Other.is_حالت_معلوم());
  أرجع getUniqueID() == Other.getUniqueID();
}

منطقي حالت::isDirectory() ثابت { أرجع Type == file_type::directory_file; }

منطقي حالت::isRegularFile() ثابت { أرجع Type == file_type::regular_file; }

منطقي حالت::isOther() ثابت {
  أرجع exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

منطقي حالت::isSymlink() ثابت { أرجع Type == file_type::symlink_file; }

منطقي حالت::is_حالت_معلوم() ثابت { أرجع Type != file_type::حالت_error; }

منطقي حالت::exists() ثابت {
  أرجع is_حالت_معلوم() && Type != file_type::file_not_وموندل;
}

File::~File() = افتراضي;

FileSystem::~FileSystem() = افتراضي;

ErrorOr<std::unique_ptr<حافظه_بفر>>
FileSystem::get_بفر_For_File(ثابت llvm::Twine &Name, int64_t File_اندازه,
                             منطقي RequiresNullTerminator, منطقي IsVolatile,
                             منطقي Is_متن) {
  خپلکاری F = Is_متن ? openFileForRead(Name) : open_File_For_Read_دوګونی(Name);
  إذا (!F)
    أرجع F.getError();

  أرجع (*F)->get_بفر(Name, File_اندازه, RequiresNullTerminator, IsVolatile);
}

std::error_کوډ FileSystem::makeAbsolute(Small_ویکټور_Impl<حرف> &لار) ثابت {
  إذا (llvm::sys::لار_2::is_absolute(لار))
    أرجع {};

  خپلکاری WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  sys::لار_2::make_absolute(WorkingDir.get(), لار);
  أرجع {};
}

std::error_کوډ FileSystem::get_Real_لار(ثابت Twine &لار,
                                        Small_ویکټور_Impl<حرف> &Output) {
  أرجع errc::operation_not_permitted;
}

std::error_کوډ FileSystem::isLocal(ثابت Twine &لار, منطقي &پایله) {
  أرجع errc::operation_not_permitted;
}

منطقي FileSystem::exists(ثابت Twine &لار) {
  خپلکاری حالت = حالت_2(لار);
  أرجع حالت && حالت->exists();
}

llvm::ErrorOr<منطقي> FileSystem::equivalent(ثابت Twine &A, ثابت Twine &B) {
  خپلکاری حالت_A = حالت_2(A);
  إذا (!حالت_A)
    أرجع حالت_A.getError();
  خپلکاری حالت_B = حالت_2(B);
  إذا (!حالت_B)
    أرجع حالت_B.getError();
  أرجع حالت_A->equivalent(*حالت_B);
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
فراغ FileSystem::dump() ثابت { print(dbgs(), PrintType::RecursiveContents); }
#د_که_پای

#که_نه_تعریف_شوی NDEBUG
ساكن منطقي isTraversalComponent(StringRef Component) {
  أرجع Component == ".." || Component == ".";
}

ساكن منطقي لار_Has_Traversal(StringRef لار) {
  کارول نطاق llvm::sys;

  لكل (StringRef Comp : llvm::make_range(لار_2::begin(لار), لار_2::پای_ټکی(لار)))
    إذا (isTraversalComponent(Comp))
      أرجع ریښتیا;
  أرجع خطأ;
}
#د_که_پای

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

/// Wrapper around a raw file descriptor.
صنف RealFile : عام File {
  ملګری صنف RealFileSystem;

  file_t FD;
  حالت S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_لار_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::حالت_error, {}),
        RealName(New_Real_لار_Name.str()) {
    أكد(FD.isValid() && "Invalid or inactive file descriptor");
  }

عام:
  ~RealFile() override;

  ErrorOr<حالت> حالت_2() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<حافظه_بفر>> get_بفر(ثابت Twine &Name,
                                                   int64_t File_اندازه,
                                                   منطقي RequiresNullTerminator,
                                                   منطقي IsVolatile) override;
  std::error_کوډ close() override;
  فراغ سېټ_لار(ثابت Twine &لار) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<حالت> RealFile::حالت_2() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot stat closed file");
  إذا (!S.is_حالت_معلوم()) {
    file_حالت Real_حالت;
    إذا (std::error_کوډ EC = sys::fs::حالت_2(FD, Real_حالت))
      أرجع EC;
    S = حالت::copyWithNewName(Real_حالت, S.getName());
  }
  أرجع S;
}

ErrorOr<std::string> RealFile::getName() {
  أرجع RealName.تشه() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<حافظه_بفر>>
RealFile::get_بفر(ثابت Twine &Name, int64_t File_اندازه,
                    منطقي RequiresNullTerminator, منطقي IsVolatile) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot get buffer for closed file");
  أرجع حافظه_بفر::getOpenFile(FD, Name, File_اندازه, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_کوډ RealFile::close() {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  std::error_کوډ EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  أرجع EC;
}

فراغ RealFile::سېټ_لار(ثابت Twine &لار) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  RealName = لار.str();
  إذا (خپلکاری حالت = حالت_2())
    S = حالت.get().copyWithNewName(حالت.get(), لار);
}

نطاق {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
صنف RealFileSystem : عام FileSystem {
عام:
  څرګند RealFileSystem(منطقي LinkCWDToProcess) {
    إذا (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      إذا (std::error_کوډ EC = llvm::sys::fs::current_لار(PWD))
        WD = std::move(EC);
      وإلا إذا (llvm::sys::fs::real_لار(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      وإلا
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<حالت> حالت_2(ثابت Twine &لار) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ثابت Twine &لار) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_دوګونی(ثابت Twine &لار) override;
  directory_iterator dir_begin(ثابت Twine &Dir, std::error_کوډ &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override;
  std::error_کوډ سېټ_Current_Working_Directory(ثابت Twine &لار) override;
  std::error_کوډ isLocal(ثابت Twine &لار, منطقي &پایله) override;
  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_ویکټور_Impl<std::string> &بهر) override {
    بهر.ټېل_شا(Dir.str());
  }
  std::error_کوډ get_Real_لار(ثابت Twine &لار,
                              Small_ویکټور_Impl<حرف> &Output) override;

محمي:
  فراغ printImpl(raw_ostream &OS, PrintType Type,
                 بینښه IndentLevel) ثابت override;

خاص:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_لار(ثابت Twine &لار, Small_ویکټور_Impl<حرف> &Storage) ثابت {
    إذا (!WD || !*WD)
      أرجع لار;
    لار.منزل_ویکټور(Storage);
    sys::لار_2::make_absolute(WD->get().Resolved, Storage);
    أرجع Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ثابت Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_لار(Name, Storage), Flags, &RealName);
    إذا (!FDOrErr)
      أرجع error_منزل_Error_کوډ(FDOrErr.takeError());
    أرجع std::unique_ptr<File>(
        جديد RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  بنية WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<حالت> RealFileSystem::حالت_2(ثابت Twine &لار) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_حالت Real_حالت;
  إذا (std::error_کوډ EC =
          sys::fs::حالت_2(adjust_لار(لار, Storage), Real_حالت))
    أرجع EC;
  أرجع حالت::copyWithNewName(Real_حالت, لار);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ثابت Twine &Name) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::د_متن);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_دوګونی(ثابت Twine &Name) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::د_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ثابت {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  إذا (WD && *WD)
    أرجع std::string(WD->get().Specified);
  إذا (WD)
    أرجع WD->getError();

  SmallString<128> Dir;
  إذا (std::error_کوډ EC = llvm::sys::fs::current_لار(Dir))
    أرجع EC;
  أرجع std::string(Dir);
}

std::error_کوډ RealFileSystem::سېټ_Current_Working_Directory(ثابت Twine &لار) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  إذا (!WD)
    أرجع llvm::sys::fs::سېټ_current_لار(لار);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_لار(لار, Storage).منزل_ویکټور(Absolute);
  منطقي IsDir;
  إذا (خپلکاری Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    أرجع Err;
  إذا (!IsDir)
    أرجع std::make_error_کوډ(std::errc::not_a_directory);
  إذا (خپلکاری Err = llvm::sys::fs::real_لار(Absolute, Resolved))
    أرجع Err;
  WD = WorkingDirectory{Absolute, Resolved};
  أرجع std::error_کوډ();
}

std::error_کوډ RealFileSystem::isLocal(ثابت Twine &لار, منطقي &پایله) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::is_local(adjust_لار(لار, Storage), پایله);
}

std::error_کوډ RealFileSystem::get_Real_لار(ثابت Twine &لار,
                                            Small_ویکټور_Impl<حرف> &Output) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::real_لار(adjust_لار(لار, Storage), Output);
}

فراغ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               بینښه IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  إذا (WD)
    OS << "own";
  وإلا
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  ساكن IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(ریښتیا);
  sys::sandbox::violationIfEnabled();

  أرجع FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  أرجع std::make_unique<RealFileSystem>(خطأ);
}

نطاق {

صنف RealFSDirIter : عام llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

عام:
  RealFSDirIter(ثابت Twine &لار, std::error_کوډ &EC) {
    خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(لار, EC);
    إذا (Iter != sys::fs::directory_iterator())
      Current_ثبت = directory_ثبت(Iter->لار_2(), Iter->type());
  }

  std::error_کوډ increment() override {
    خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

    std::error_کوډ EC;
    Iter.increment(EC);
    Current_ثبت = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_ثبت()
                       : directory_ثبت(Iter->لار_2(), Iter->type());
    أرجع EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ثابت Twine &Dir,
                                             std::error_کوډ &EC) {
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  أرجع directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_لار(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> بنسټ_FS) {
  FSList.ټېل_شا(std::move(بنسټ_FS));
}

فراغ OverlayFileSystem::ټېل_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ټېل_شا(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->سېټ_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<حالت> OverlayFileSystem::حالت_2(ثابت Twine &لار) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_پای_ټکی(); I != E; ++I) {
    ErrorOr<حالت> حالت = (*I)->حالت_2(لار);
    إذا (حالت || حالت.getError() != llvm::errc::no_such_file_or_directory)
      أرجع حالت;
  }
  أرجع make_error_کوډ(llvm::errc::no_such_file_or_directory);
}

منطقي OverlayFileSystem::exists(ثابت Twine &لار) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_پای_ټکی(); I != E; ++I) {
    إذا ((*I)->exists(لار))
      أرجع ریښتیا;
  }
  أرجع خطأ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ثابت llvm::Twine &لار) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_پای_ټکی(); I != E; ++I) {
    خپلکاری پایله = (*I)->openFileForRead(لار);
    إذا (پایله || پایله.getError() != llvm::errc::no_such_file_or_directory)
      أرجع پایله;
  }
  أرجع make_error_کوډ(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ثابت {
  // All file systems are synchronized, just take the first working directory.
  أرجع FSList.front()->getCurrentWorkingDirectory();
}

std::error_کوډ
OverlayFileSystem::سېټ_Current_Working_Directory(ثابت Twine &لار) {
  لكل (خپلکاری &FS : FSList)
    إذا (std::error_کوډ EC = FS->سېټ_Current_Working_Directory(لار))
      أرجع EC;
  أرجع {};
}

std::error_کوډ OverlayFileSystem::isLocal(ثابت Twine &لار, منطقي &پایله) {
  لكل (خپلکاری &FS : FSList)
    إذا (FS->exists(لار))
      أرجع FS->isLocal(لار, پایله);
  أرجع errc::no_such_file_or_directory;
}

std::error_کوډ OverlayFileSystem::get_Real_لار(ثابت Twine &لار,
                                               Small_ویکټور_Impl<حرف> &Output) {
  لكل (ثابت خپلکاری &FS : FSList)
    إذا (FS->exists(لار))
      أرجع FS->get_Real_لار(لار, Output);
  أرجع errc::no_such_file_or_directory;
}

فراغ OverlayFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_ویکټور_Impl<std::string> &بهر) {
  // All layers contribute.
  لكل (iterator I = overlays_begin(), E = overlays_پای_ټکی(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, بهر);
}

فراغ OverlayFileSystem::لیدنه_ماشوم_File_Systems(لیدنه_Callback_Ty Callback) {
  لكل (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->لیدنه_ماشوم_File_Systems(Callback);
  }
}

فراغ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  بینښه IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  إذا (Type == PrintType::Summary)
    أرجع;

  إذا (Type == PrintType::Contents)
    Type = PrintType::Summary;
  لكل (ثابت خپلکاری &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = افتراضي;

نطاق {

/// Combines and deduplicates directory entries across multiple file systems.
صنف CombiningDirIterImpl : عام llvm::vfs::detail::DirIterImpl {
  کارول FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_ویکټور<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_سېټ<> کتل_شوی_نومونه;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_کوډ incrementIter(منطقي Is_لومړی_وخت) {
    طالما (!IterList.تشه()) {
      CurrentDirIter = IterList.شا();
      IterList.راوباسه_شا();
      إذا (CurrentDirIter != directory_iterator())
        اكسر; //وموندل
    }

    إذا (Is_لومړی_وخت && CurrentDirIter == directory_iterator())
      أرجع errc::no_such_file_or_directory;
    أرجع {};
  }

  std::error_کوډ incrementDirIter(منطقي Is_لومړی_وخت) {
    أكد((Is_لومړی_وخت || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_کوډ EC;
    إذا (!Is_لومړی_وخت)
      CurrentDirIter.increment(EC);
    إذا (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(Is_لومړی_وخت);
    أرجع EC;
  }

  std::error_کوډ incrementImpl(منطقي Is_لومړی_وخت) {
    طالما (ریښتیا) {
      std::error_کوډ EC = incrementDirIter(Is_لومړی_وخت);
      إذا (EC || CurrentDirIter == directory_iterator()) {
        Current_ثبت = directory_ثبت();
        أرجع EC;
      }
      Current_ثبت = *CurrentDirIter;
      StringRef Name = llvm::sys::لار_2::filename(Current_ثبت.لار_2());
      إذا (کتل_شوی_نومونه.ورننباسه(Name).دوهم_2)
        أرجع EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

عام:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_کوډ &EC) {
    لكل (ثابت خپلکاری &FS : FileSystems) {
      std::error_کوډ FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      إذا (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        أرجع;
      }
      إذا (!FEC)
        IterList.ټېل_شا(Iter);
    }
    EC = incrementImpl(ریښتیا);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_کوډ &EC)
      : IterList(DirIters) {
    EC = incrementImpl(ریښتیا);
  }

  std::error_کوډ increment() override { أرجع incrementImpl(خطأ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ثابت Twine &Dir,
                                                std::error_کوډ &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  إذا (EC)
    أرجع {};
  أرجع Combined;
}

فراغ ProxyFileSystem::anchor() {}

نطاق llvm {
نطاق vfs {

نطاق detail {

تعداد دننه_حافظه_غوټه_ډول {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
صنف دننه_حافظه_غوټه {
  دننه_حافظه_غوټه_ډول ډول_2;
  std::string FileName;

عام:
  دننه_حافظه_غوټه(llvm::StringRef FileName, دننه_حافظه_غوټه_ډول ډول_2)
      : ډول_2(ډول_2), FileName(std::string(llvm::sys::لار_2::filename(FileName))) {
  }
  مجازي ~دننه_حافظه_غوټه() = افتراضي;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  مجازي حالت get_حالت(ثابت Twine &RequestedName) ثابت = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ثابت { أرجع FileName; }
  دننه_حافظه_غوټه_ډول get_ډول() ثابت { أرجع ډول_2; }
  مجازي std::string منزل_String(بینښه Indent) ثابت = 0;
};

صنف دننه_حافظه_File : عام دننه_حافظه_غوټه {
  حالت Stat;
  std::unique_ptr<llvm::حافظه_بفر> بفر;

عام:
  دننه_حافظه_File(حالت Stat, std::unique_ptr<llvm::حافظه_بفر> بفر)
      : دننه_حافظه_غوټه(Stat.getName(), IME_File), Stat(std::move(Stat)),
        بفر(std::move(بفر)) {}

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }
  llvm::حافظه_بفر *get_بفر() ثابت { أرجع بفر.get(); }

  std::string منزل_String(بینښه Indent) ثابت override {
    أرجع (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ساكن منطقي classof(ثابت دننه_حافظه_غوټه *N) {
    أرجع N->get_ډول() == IME_File;
  }
};

نطاق {

صنف دننه_حافظه_Hard_Link : عام دننه_حافظه_غوټه {
  ثابت دننه_حافظه_File &ResolvedFile;

عام:
  دننه_حافظه_Hard_Link(StringRef لار, ثابت دننه_حافظه_File &ResolvedFile)
      : دننه_حافظه_غوټه(لار, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ثابت دننه_حافظه_File &getResolvedFile() ثابت { أرجع ResolvedFile; }

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع ResolvedFile.get_حالت(RequestedName);
  }

  std::string منزل_String(بینښه Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.منزل_String(0);
  }

  ساكن منطقي classof(ثابت دننه_حافظه_غوټه *N) {
    أرجع N->get_ډول() == IME_HardLink;
  }
};

صنف دننه_حافظه_Symbolic_Link : عام دننه_حافظه_غوټه {
  std::string موخه_لار;
  حالت Stat;

عام:
  دننه_حافظه_Symbolic_Link(StringRef لار, StringRef موخه_لار, حالت Stat)
      : دننه_حافظه_غوټه(لار, IME_SymbolicLink), موخه_لار(std::move(موخه_لار)),
        Stat(Stat) {}

  std::string منزل_String(بینښه Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "SymbolicLink to -> " + موخه_لار;
  }

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_موخه_لار() ثابت { أرجع موخه_لار; }

  ساكن منطقي classof(ثابت دننه_حافظه_غوټه *N) {
    أرجع N->get_ډول() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
صنف دننه_حافظه_File_Adaptor : عام File {
  ثابت دننه_حافظه_File &غوټه;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

عام:
  څرګند دننه_حافظه_File_Adaptor(ثابت دننه_حافظه_File &غوټه,
                               std::string RequestedName)
      : غوټه(غوټه), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<حالت> حالت_2() override {
    أرجع غوټه.get_حالت(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::حافظه_بفر>>
  get_بفر(ثابت Twine &Name, int64_t File_اندازه, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    llvm::حافظه_بفر *Buf = غوټه.get_بفر();
    أرجع llvm::حافظه_بفر::get_Mem_بفر(
        Buf->get_بفر(), Buf->get_بفر_Identifier(), RequiresNullTerminator);
  }

  std::error_کوډ close() override { أرجع {}; }

  فراغ سېټ_لار(ثابت Twine &لار) override { RequestedName = لار.str(); }
};
} // namespace

صنف دننه_حافظه_Directory : عام دننه_حافظه_غوټه {
  حالت Stat;
  std::map<std::string, std::unique_ptr<دننه_حافظه_غوټه>, std::less<>> Entries;

عام:
  دننه_حافظه_Directory(حالت Stat)
      : دننه_حافظه_غوټه(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ثابت { أرجع Stat.getUniqueID(); }

  دننه_حافظه_غوټه *get_ماشوم(StringRef Name) ثابت {
    خپلکاری I = Entries.پیدا_کړه(Name);
    إذا (I != Entries.پای_ټکی())
      أرجع I->دوهم_2.get();
    أرجع تش_اشاره;
  }

  دننه_حافظه_غوټه *جمع_کړه_ماشوم(StringRef Name, std::unique_ptr<دننه_حافظه_غوټه> ماشوم) {
    أرجع Entries.emplace(Name, std::move(ماشوم)).لومړی_2->دوهم_2.get();
  }

  کارول const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ثابت { أرجع Entries.begin(); }
  const_iterator پای_ټکی() ثابت { أرجع Entries.پای_ټکی(); }

  std::string منزل_String(بینښه Indent) ثابت override {
    std::string پایله =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    لكل (ثابت خپلکاری &ثبت : Entries)
      پایله += ثبت.دوهم_2->منزل_String(Indent + 2);
    أرجع پایله;
  }

  ساكن منطقي classof(ثابت دننه_حافظه_غوټه *N) {
    أرجع N->get_ډول() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ساكن sys::fs::UniqueID getUniqueID(hash_کوډ Hash) {
  أرجع sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ساكن sys::fs::UniqueID getFileID(sys::fs::UniqueID پلار_2,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  أرجع getUniqueID(llvm::hash_combine(پلار_2.getFile(), Name, Contents));
}
ساكن sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID پلار_2,
                                        llvm::StringRef Name) {
  أرجع getUniqueID(llvm::hash_combine(پلار_2.getFile(), Name));
}

حالت detail::New_دننه_حافظه_غوټه_Info::make_حالت() ثابت {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, بفر ? بفر->get_بفر() : "");

  أرجع حالت(لار, UID, llvm::sys::منزل_وخت_ټکی(Modification_وخت), User,
                ډله, بفر ? بفر->get_بفر_اندازه() : 0, Type, Perms);
}

دننه_حافظه_File_System::دننه_حافظه_File_System(منطقي UseNormalizedPaths)
    : ریښه_2(جديد detail::دننه_حافظه_Directory(
          حالت("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::وخت_ټکی<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

دننه_حافظه_File_System::~دننه_حافظه_File_System() = افتراضي;

std::string دننه_حافظه_File_System::منزل_String() ثابت {
  أرجع ریښه_2->منزل_String(/*Indent=*/0);
}

منطقي دننه_حافظه_File_System::جمع_کړه_File(ثابت Twine &P, time_t Modification_وخت,
                                 std::unique_ptr<llvm::حافظه_بفر> بفر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> ډله,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_غوټه_Fn Make_غوټه) {
  SmallString<128> لار;
  P.منزل_ویکټور(لار);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کوډ EC = makeAbsolute(لار);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::لار_2::remove_dots(لار, /*remove_dot_dot=*/ریښتیا);

  إذا (لار.تشه())
    أرجع خطأ;

  detail::دننه_حافظه_Directory *Dir = ریښه_2.get();
  خپلکاری I = llvm::sys::لار_2::begin(لار), E = sys::لار_2::پای_ټکی(لار);
  ثابت خپلکاری ResolvedUser = User.ارزښت_or(0);
  ثابت خپلکاری Resolved_ډله = ډله.ارزښت_or(0);
  ثابت خپلکاری ResolvedType = Type.ارزښت_or(sys::fs::file_type::regular_file);
  ثابت خپلکاری ResolvedPerms = Perms.ارزښت_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ثابت خپلکاری NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  طالما (ریښتیا) {
    Name = *I;
    ++I;
    إذا (I == E)
      اكسر;
    detail::دننه_حافظه_غوټه *غوټه = Dir->get_ماشوم(Name);
    إذا (!غوټه) {
      // This isn't the last element, so we create a new directory.
      حالت Stat(
          StringRef(لار.str().begin(), Name.پای_ټکی() - لار.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::منزل_وخت_ټکی(Modification_وخت), ResolvedUser, Resolved_ډله,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::دننه_حافظه_Directory>(Dir->جمع_کړه_ماشوم(
          Name, std::make_unique<detail::دننه_حافظه_Directory>(std::move(Stat))));
      استمر;
    }
    // Creating file under another file.
    إذا (!isa<detail::دننه_حافظه_Directory>(غوټه))
      أرجع خطأ;
    Dir = cast<detail::دننه_حافظه_Directory>(غوټه);
  }
  detail::دننه_حافظه_غوټه *غوټه = Dir->get_ماشوم(Name);
  إذا (!غوټه) {
    Dir->جمع_کړه_ماشوم(Name,
                  Make_غوټه({Dir->getUniqueID(), لار, Name, Modification_وخت,
                            std::move(بفر), ResolvedUser, Resolved_ډله,
                            ResolvedType, ResolvedPerms}));
    أرجع ریښتیا;
  }
  إذا (isa<detail::دننه_حافظه_Directory>(غوټه))
    أرجع ResolvedType == sys::fs::file_type::directory_file;

  أكد((isa<detail::دننه_حافظه_File>(غوټه) ||
          isa<detail::دننه_حافظه_Hard_Link>(غوټه)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  إذا (خپلکاری *Link = dyn_cast<detail::دننه_حافظه_Hard_Link>(غوټه)) {
    أرجع Link->getResolvedFile().get_بفر()->get_بفر() ==
           بفر->get_بفر();
  }
  أرجع cast<detail::دننه_حافظه_File>(غوټه)->get_بفر()->get_بفر() ==
         بفر->get_بفر();
}

منطقي دننه_حافظه_File_System::جمع_کړه_File(ثابت Twine &P, time_t Modification_وخت,
                                 std::unique_ptr<llvm::حافظه_بفر> بفر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> ډله,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  أرجع جمع_کړه_File(P, Modification_وخت, std::move(بفر), User, ډله, Type,
                 Perms,
                 [](detail::New_دننه_حافظه_غوټه_Info NNI)
                     -> std::unique_ptr<detail::دننه_حافظه_غوټه> {
                   حالت Stat = NNI.make_حالت();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::دننه_حافظه_Directory>(Stat);
                   أرجع std::make_unique<detail::دننه_حافظه_File>(
                       Stat, std::move(NNI.بفر));
                 });
}

منطقي دننه_حافظه_File_System::جمع_کړه_File_No_Own(
    ثابت Twine &P, time_t Modification_وخت,
    ثابت llvm::حافظه_بفر_Ref &بفر, std::optional<uint32_t> User,
    std::optional<uint32_t> ډله, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  أرجع جمع_کړه_File(P, Modification_وخت, llvm::حافظه_بفر::get_Mem_بفر(بفر),
                 std::move(User), std::move(ډله), std::move(Type),
                 std::move(Perms),
                 [](detail::New_دننه_حافظه_غوټه_Info NNI)
                     -> std::unique_ptr<detail::دننه_حافظه_غوټه> {
                   حالت Stat = NNI.make_حالت();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::دننه_حافظه_Directory>(Stat);
                   أرجع std::make_unique<detail::دننه_حافظه_File>(
                       Stat, std::move(NNI.بفر));
                 });
}

detail::Named_غوټه_Or_Error
دننه_حافظه_File_System::lookup_غوټه(ثابت Twine &P, منطقي FollowFinalSymlink,
                               size_t Symlink_ژوروالی) ثابت {
  SmallString<128> لار;
  P.منزل_ویکټور(لار);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کوډ EC = makeAbsolute(لار);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::لار_2::remove_dots(لار, /*remove_dot_dot=*/ریښتیا);

  ثابت detail::دننه_حافظه_Directory *Dir = ریښه_2.get();
  إذا (لار.تشه())
    أرجع detail::Named_غوټه_Or_Error(لار, Dir);

  خپلکاری I = llvm::sys::لار_2::begin(لار), E = llvm::sys::لار_2::پای_ټکی(لار);
  طالما (ریښتیا) {
    detail::دننه_حافظه_غوټه *غوټه = Dir->get_ماشوم(*I);
    ++I;
    إذا (!غوټه)
      أرجع errc::no_such_file_or_directory;

    إذا (خپلکاری Symlink = dyn_cast<detail::دننه_حافظه_Symbolic_Link>(غوټه)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      إذا (I == E && !FollowFinalSymlink)
        أرجع detail::Named_غوټه_Or_Error(لار, Symlink);

      إذا (Symlink_ژوروالی > دننه_حافظه_File_System::Max_Symlink_ژوروالی)
        أرجع errc::no_such_file_or_directory;

      SmallString<128> موخه_لار = Symlink->get_موخه_لار();
      إذا (std::error_کوډ EC = makeAbsolute(موخه_لار))
        أرجع EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      خپلکاری موخه_2 =
          lookup_غوټه(موخه_لار, /*FollowFinalSymlink=*/ریښتیا, Symlink_ژوروالی + 1);
      إذا (!موخه_2 || I == E)
        أرجع موخه_2;

      إذا (!isa<detail::دننه_حافظه_Directory>(*موخه_2))
        أرجع errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::دننه_حافظه_Directory>(*موخه_2);
      استمر;
    }

    // Return the file if it's at the end of the path.
    إذا (خپلکاری File = dyn_cast<detail::دننه_حافظه_File>(غوټه)) {
      إذا (I == E)
        أرجع detail::Named_غوټه_Or_Error(لار, File);
      أرجع errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    إذا (خپلکاری File = dyn_cast<detail::دننه_حافظه_Hard_Link>(غوټه)) {
      إذا (I == E)
        أرجع detail::Named_غوټه_Or_Error(لار, &File->getResolvedFile());
      أرجع errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::دننه_حافظه_Directory>(غوټه);
    إذا (I == E)
      أرجع detail::Named_غوټه_Or_Error(لار, Dir);
  }
}

منطقي دننه_حافظه_File_System::جمع_کړه_Hard_Link(ثابت Twine &NewLink,
                                     ثابت Twine &موخه_2) {
  خپلکاری New_Link_غوټه = lookup_غوټه(NewLink, /*FollowFinalSymlink=*/خطأ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  خپلکاری موخه_غوټه = lookup_غوټه(موخه_2, /*FollowFinalSymlink=*/ریښتیا);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  إذا (!موخه_غوټه || New_Link_غوټه || !isa<detail::دننه_حافظه_File>(*موخه_غوټه))
    أرجع خطأ;
  أرجع جمع_کړه_File(NewLink, 0, تش_اشاره, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_دننه_حافظه_غوټه_Info NNI) {
                   أرجع std::make_unique<detail::دننه_حافظه_Hard_Link>(
                       NNI.لار.str(),
                       *cast<detail::دننه_حافظه_File>(*موخه_غوټه));
                 });
}

منطقي دننه_حافظه_File_System::جمع_کړه_Symbolic_Link(
    ثابت Twine &NewLink, ثابت Twine &موخه_2, time_t Modification_وخت,
    std::optional<uint32_t> User, std::optional<uint32_t> ډله,
    std::optional<llvm::sys::fs::perms> Perms) {
  خپلکاری New_Link_غوټه = lookup_غوټه(NewLink, /*FollowFinalSymlink=*/خطأ);
  إذا (New_Link_غوټه)
    أرجع خطأ;

  SmallString<128> NewLinkStr, موخه_Str;
  NewLink.منزل_ویکټور(NewLinkStr);
  موخه_2.منزل_ویکټور(موخه_Str);

  أرجع جمع_کړه_File(NewLinkStr, Modification_وخت, تش_اشاره, User, ډله,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_دننه_حافظه_غوټه_Info NNI) {
                   أرجع std::make_unique<detail::دننه_حافظه_Symbolic_Link>(
                       NewLinkStr, موخه_Str, NNI.make_حالت());
                 });
}

llvm::ErrorOr<حالت> دننه_حافظه_File_System::حالت_2(ثابت Twine &لار) {
  خپلکاری غوټه = lookup_غوټه(لار, /*FollowFinalSymlink=*/ریښتیا);
  إذا (غوټه)
    أرجع (*غوټه)->get_حالت(لار);
  أرجع غوټه.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
دننه_حافظه_File_System::openFileForRead(ثابت Twine &لار) {
  خپلکاری غوټه = lookup_غوټه(لار,/*FollowFinalSymlink=*/ریښتیا);
  إذا (!غوټه)
    أرجع غوټه.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  إذا (خپلکاری *F = dyn_cast<detail::دننه_حافظه_File>(*غوټه))
    أرجع std::unique_ptr<File>(
        جديد detail::دننه_حافظه_File_Adaptor(*F, لار.str()));

  // FIXME: errc::not_a_file?
  أرجع make_error_کوډ(llvm::errc::invalid_دلیل);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
صنف دننه_حافظه_File_System::DirIterator : عام llvm::vfs::detail::DirIterImpl {
  ثابت دننه_حافظه_File_System *FS;
  detail::دننه_حافظه_Directory::const_iterator I;
  detail::دننه_حافظه_Directory::const_iterator E;
  std::string RequestedDirName;

  فراغ سېټ_Current_ثبت() {
    إذا (I != E) {
      SmallString<256> لار(RequestedDirName);
      llvm::sys::لار_2::append(لار, I->دوهم_2->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل (I->دوهم_2->get_ډول()) {
      حالة detail::IME_File:
      حالة detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      حالة detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة detail::IME_SymbolicLink:
        إذا (خپلکاری Symlink_موخه =
                FS->lookup_غوټه(لار, /*FollowFinalSymlink=*/ریښتیا)) {
          لار = Symlink_موخه.getName();
          Type = (*Symlink_موخه)->get_حالت(لار).getType();
        }
        اكسر;
      }
      Current_ثبت = directory_ثبت(std::string(لار), Type);
    } وإلا {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_ثبت = directory_ثبت();
    }
  }

عام:
  DirIterator() = افتراضي;

  DirIterator(ثابت دننه_حافظه_File_System *FS,
              ثابت detail::دننه_حافظه_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.پای_ټکی()),
        RequestedDirName(std::move(RequestedDirName)) {
    سېټ_Current_ثبت();
  }

  std::error_کوډ increment() override {
    ++I;
    سېټ_Current_ثبت();
    أرجع {};
  }
};

directory_iterator دننه_حافظه_File_System::dir_begin(ثابت Twine &Dir,
                                                 std::error_کوډ &EC) {
  خپلکاری غوټه = lookup_غوټه(Dir, /*FollowFinalSymlink=*/ریښتیا);
  إذا (!غوټه) {
    EC = غوټه.getError();
    أرجع directory_iterator(std::make_shared<DirIterator>());
  }

  إذا (خپلکاری *Dir_غوټه = dyn_cast<detail::دننه_حافظه_Directory>(*غوټه))
    أرجع directory_iterator(
        std::make_shared<DirIterator>(هذا, *Dir_غوټه, Dir.str()));

  EC = make_error_کوډ(llvm::errc::not_a_directory);
  أرجع directory_iterator(std::make_shared<DirIterator>());
}

std::error_کوډ دننه_حافظه_File_System::سېټ_Current_Working_Directory(ثابت Twine &P) {
  SmallString<128> لار;
  P.منزل_ویکټور(لار);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کوډ EC = makeAbsolute(لار);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::لار_2::remove_dots(لار, /*remove_dot_dot=*/ریښتیا);

  إذا (!لار.تشه())
    WorkingDirectory = std::string(لار);
  أرجع {};
}

std::error_کوډ دننه_حافظه_File_System::get_Real_لار(ثابت Twine &لار,
                                                Small_ویکټور_Impl<حرف> &Output) {
  خپلکاری CWD = getCurrentWorkingDirectory();
  إذا (!CWD || CWD->تشه())
    أرجع errc::operation_not_permitted;
  لار.منزل_ویکټور(Output);
  إذا (خپلکاری EC = makeAbsolute(Output))
    أرجع EC;
  llvm::sys::لار_2::remove_dots(Output, /*remove_dot_dot=*/ریښتیا);
  أرجع {};
}

std::error_کوډ دننه_حافظه_File_System::isLocal(ثابت Twine &لار, منطقي &پایله) {
  پایله = خطأ;
  أرجع {};
}

فراغ دننه_حافظه_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   بینښه IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

ساكن llvm::sys::لار_2::Style getExistingStyle(llvm::StringRef لار) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::لار_2::Style style = llvm::sys::لار_2::Style::native;
  ثابت size_t n = لار.پیدا_کړه_لومړی_د("/\\");
  // Can't distinguish between posix and windows_slash here.
  إذا (n != static_cast<size_t>(-1))
    style = (لار[n] == '/') ? llvm::sys::لار_2::Style::posix
                             : llvm::sys::لار_2::Style::windows_backslash;
  أرجع style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ساكن llvm::SmallString<256> canonicalize(llvm::StringRef لار) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::لار_2::Style style = getExistingStyle(لار);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> پایله_2 =
      llvm::sys::لار_2::remove_leading_dotslash(لار, style);
  llvm::sys::لار_2::remove_dots(پایله_2, /*remove_dot_dot=*/ریښتیا, style);
  أرجع پایله_2;
}

/// Whether the error and entry specify a file/directory that was not found.
ساكن منطقي is_File_Not_وموندل(std::error_کوډ EC,
                           RedirectingFileSystem::ثبت *E = تش_اشاره) {
  إذا (E && !isa<RedirectingFileSystem::Directory_Remap_ثبت>(E))
    أرجع خطأ;
  أرجع EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  أكد(ExternalFS && "RedirectingFileSystem requires an external FS");
  إذا (خپلکاری ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
صنف llvm::vfs::RedirectingFSDirIterImpl
    : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::Directory_ثبت::iterator Current, پای_ټکی_2;

  std::error_کوډ incrementImpl(منطقي Is_لومړی_وخت) {
    أكد((Is_لومړی_وخت || Current != پای_ټکی_2) && "cannot iterate past end");
    إذا (!Is_لومړی_وخت)
      ++Current;
    إذا (Current != پای_ټکی_2) {
      SmallString<128> لار_Str(Dir);
      llvm::sys::لار_2::append(لار_Str, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل ((*Current)->get_ډول()) {
      حالة RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      حالة RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      }
      Current_ثبت = directory_ثبت(std::string(لار_Str), Type);
    } وإلا {
      Current_ثبت = directory_ثبت();
    }
    أرجع {};
  };

عام:
  RedirectingFSDirIterImpl(
      ثابت Twine &لار, RedirectingFileSystem::Directory_ثبت::iterator Begin,
      RedirectingFileSystem::Directory_ثبت::iterator پای_ټکی_2, std::error_کوډ &EC)
      : Dir(لار.str()), Current(Begin), پای_ټکی_2(پای_ټکی_2) {
    EC = incrementImpl(/*IsFirstTime=*/ریښتیا);
  }

  std::error_کوډ increment() override {
    أرجع incrementImpl(/*IsFirstTime=*/خطأ);
  }
};

نطاق {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
صنف RedirectingFSDirRemapIterImpl : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::لار_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

عام:
  RedirectingFSDirRemapIterImpl(std::string Dir_لار,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_لار)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    إذا (ExternalIter != llvm::vfs::directory_iterator())
      سېټ_Current_ثبت();
  }

  فراغ سېټ_Current_ثبت() {
    StringRef External_لار = ExternalIter->لار_2();
    llvm::sys::لار_2::Style ExternalStyle = getExistingStyle(External_لار);
    StringRef File = llvm::sys::لار_2::filename(External_لار, ExternalStyle);

    SmallString<128> New_لار(Dir);
    llvm::sys::لار_2::append(New_لار, DirStyle, File);

    Current_ثبت = directory_ثبت(std::string(New_لار), ExternalIter->type());
  }

  std::error_کوډ increment() override {
    std::error_کوډ EC;
    ExternalIter.increment(EC);
    إذا (!EC && ExternalIter != llvm::vfs::directory_iterator())
      سېټ_Current_ثبت();
    وإلا
      Current_ثبت = directory_ثبت();
    أرجع EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ثابت {
  أرجع WorkingDirectory;
}

std::error_کوډ
RedirectingFileSystem::سېټ_Current_Working_Directory(ثابت Twine &لار) {
  // Don't change the working directory if the path doesn't exist.
  إذا (!exists(لار))
    أرجع errc::no_such_file_or_directory;

  SmallString<128> Absolute_لار;
  لار.منزل_ویکټور(Absolute_لار);
  إذا (std::error_کوډ EC = makeAbsolute(Absolute_لار))
    أرجع EC;
  WorkingDirectory = std::string(Absolute_لار);
  أرجع {};
}

std::error_کوډ RedirectingFileSystem::isLocal(ثابت Twine &لار_3,
                                               منطقي &پایله) {
  SmallString<256> لار;
  لار_3.منزل_ویکټور(لار);

  إذا (makeAbsolute(لار))
    أرجع {};

  أرجع ExternalFS->isLocal(لار, پایله);
}

std::error_کوډ RedirectingFileSystem::makeAbsolute(Small_ویکټور_Impl<حرف> &لار) ثابت {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  إذا (llvm::sys::لار_2::is_absolute(لار, llvm::sys::لار_2::Style::posix) ||
      llvm::sys::لار_2::is_absolute(لار,
                                   llvm::sys::لار_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    أرجع {};

  خپلکاری WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  أرجع makeAbsolute(WorkingDir.get(), لار);
}

std::error_کوډ
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_ویکټور_Impl<حرف> &لار) ثابت {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  إذا (!WorkingDir.تشه() &&
      !sys::لار_2::is_absolute(WorkingDir, sys::لار_2::Style::posix) &&
      !sys::لار_2::is_absolute(WorkingDir,
                              sys::لار_2::Style::windows_backslash)) {
    أرجع std::error_کوډ();
  }
  sys::لار_2::Style style = sys::لار_2::Style::windows_backslash;
  إذا (sys::لار_2::is_absolute(WorkingDir, sys::لار_2::Style::posix)) {
    style = sys::لار_2::Style::posix;
  } وإلا {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    إذا (getExistingStyle(WorkingDir) != sys::لار_2::Style::windows_backslash)
      style = sys::لار_2::Style::windows_slash;
  }

  std::string پایله = std::string(WorkingDir);
  StringRef Dir(پایله);
  إذا (!Dir.پایونه_with(sys::لار_2::get_separator(style))) {
    پایله += sys::لار_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  پایله.append(لار.معلومات(), لار.اندازه_2());
  لار.ټاکل(پایله.begin(), پایله.پای_ټکی());

  أرجع {};
}

directory_iterator RedirectingFileSystem::dir_begin(ثابت Twine &Dir,
                                                    std::error_کوډ &EC) {
  SmallString<256> لار;
  Dir.منزل_ویکټور(لار);

  EC = makeAbsolute(لار);
  إذا (EC)
    أرجع {};

  ErrorOr<RedirectingFileSystem::Lookup_پایله> پایله = lookup_لار(لار);
  إذا (!پایله) {
    إذا (Redirection != Redirect_ډول::RedirectOnly &&
        is_File_Not_وموندل(پایله.getError()))
      أرجع ExternalFS->dir_begin(لار, EC);

    EC = پایله.getError();
    أرجع {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<حالت> S = حالت_2(لار, Dir, *پایله);
  إذا (!S) {
    إذا (Redirection != Redirect_ډول::RedirectOnly &&
        is_File_Not_وموندل(S.getError(), پایله->E))
      أرجع ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    أرجع {};
  }

  إذا (!S->isDirectory()) {
    EC = errc::not_a_directory;
    أرجع {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_کوډ RedirectEC;
  إذا (خپلکاری ExtRedirect = پایله->getExternalRedirect()) {
    خپلکاری RE = cast<RedirectingFileSystem::Remap_ثبت>(پایله->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    إذا (!RE->useExternalName(Use_External_نومونه)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(لار), RedirectIter));
    }
  } وإلا {
    خپلکاری DE = cast<Directory_ثبت>(پایله->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            لار, DE->contents_begin(), DE->contents_پای_ټکی(), RedirectEC));
  }

  إذا (RedirectEC) {
    إذا (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      أرجع {};
    }
    RedirectIter = {};
  }

  إذا (Redirection == Redirect_ډول::RedirectOnly) {
    EC = RedirectEC;
    أرجع RedirectIter;
  }

  std::error_کوډ ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(لار, ExternalEC);
  إذا (ExternalEC) {
    إذا (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      أرجع {};
    }
    ExternalIter = {};
  }

  Small_ویکټور<directory_iterator, 2> Iters;
  بدل (Redirection) {
  حالة Redirect_ډول::Fallthrough:
    Iters.ټېل_شا(ExternalIter);
    Iters.ټېل_شا(RedirectIter);
    اكسر;
  حالة Redirect_ډول::Fallback:
    Iters.ټېل_شا(RedirectIter);
    Iters.ټېل_شا(ExternalIter);
    اكسر;
  افتراضي:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  إذا (EC)
    أرجع {};
  أرجع Combined;
}

فراغ RedirectingFileSystem::سېټ_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ثابت {
  أرجع OverlayFileDir;
}

فراغ RedirectingFileSystem::سېټ_Fallthrough(منطقي Fallthrough) {
  إذا (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_ډول::Fallthrough;
  } وإلا {
    Redirection = RedirectingFileSystem::Redirect_ډول::RedirectOnly;
  }
}

فراغ RedirectingFileSystem::سېټ_Redirection(
    RedirectingFileSystem::Redirect_ډول ډول_2) {
  Redirection = ډول_2;
}

std::ویکټور<StringRef> RedirectingFileSystem::getRoots() ثابت {
  std::ویکټور<StringRef> R;
  R.reserve(Roots.اندازه_2());
  لكل (ثابت خپلکاری &ریښه_2 : Roots)
    R.ټېل_شا(ریښه_2->getName());
  أرجع R;
}

فراغ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      بینښه IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_نومونه ? "true" : "false") << ")\n";
  إذا (Type == PrintType::Summary)
    أرجع;

  لكل (ثابت خپلکاری &ریښه_2 : Roots)
    print_ثبت(OS, ریښه_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

فراغ RedirectingFileSystem::print_ثبت(raw_ostream &OS,
                                       RedirectingFileSystem::ثبت *E,
                                       بینښه IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  بدل (E->get_ډول()) {
  حالة EK_Directory: {
    خپلکاری *DE = cast<RedirectingFileSystem::Directory_ثبت>(E);

    OS << "\n";
    لكل (std::unique_ptr<ثبت> &Sub_ثبت :
         llvm::make_range(DE->contents_begin(), DE->contents_پای_ټکی()))
      print_ثبت(OS, Sub_ثبت.get(), IndentLevel + 1);
    اكسر;
  }
  حالة EK_DirectoryRemap:
  حالة EK_File: {
    خپلکاری *RE = cast<RedirectingFileSystem::Remap_ثبت>(E);
    OS << " -> '" << RE->get_External_Contents_لار() << "'";
    بدل (RE->getUseName()) {
    حالة NK_Not_سېټ:
      اكسر;
    حالة NK_External:
      OS << " (UseExternalName: true)";
      اكسر;
    حالة NK_Virtual:
      OS << " (UseExternalName: false)";
      اكسر;
    }
    OS << "\n";
    اكسر;
  }
  }
}

فراغ RedirectingFileSystem::لیدنه_ماشوم_File_Systems(لیدنه_Callback_Ty Callback) {
  إذا (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->لیدنه_ماشوم_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
صنف llvm::vfs::RedirectingFileSystemParser {
  yaml::بهیر &بهیر;

  فراغ error(yaml::غوټه *N, ثابت Twine &پیغام_4) { بهیر.printError(N, پیغام_4); }

  // false on error
  منطقي parseScalarString(yaml::غوټه *N, StringRef &پایله,
                         Small_ویکټور_Impl<حرف> &Storage) {
    ثابت خپلکاری *S = dyn_cast_if_present<yaml::Scalar_غوټه>(N);

    إذا (!S) {
      error(N, "expected string");
      أرجع خطأ;
    }
    پایله = S->get_ارزښت(Storage);
    أرجع ریښتیا;
  }

  // false on error
  منطقي parseScalarBool(yaml::غوټه *N, منطقي &پایله) {
    SmallString<5> Storage;
    StringRef ارزښت;
    إذا (!parseScalarString(N, ارزښت, Storage))
      أرجع خطأ;

    إذا (ارزښت.equals_insensitive("true") || ارزښت.equals_insensitive("on") ||
        ارزښت.equals_insensitive("yes") || ارزښت == "1") {
      پایله = ریښتیا;
      أرجع ریښتیا;
    } وإلا إذا (ارزښت.equals_insensitive("false") ||
               ارزښت.equals_insensitive("off") ||
               ارزښت.equals_insensitive("no") || ارزښت == "0") {
      پایله = خطأ;
      أرجع ریښتیا;
    }

    error(N, "expected boolean value");
    أرجع خطأ;
  }

  std::optional<RedirectingFileSystem::Redirect_ډول>
  parse_Redirect_ډول(yaml::غوټه *N) {
    SmallString<12> Storage;
    StringRef ارزښت;
    إذا (!parseScalarString(N, ارزښت, Storage))
      أرجع std::nullopt;

    إذا (ارزښت.equals_insensitive("fallthrough")) {
      أرجع RedirectingFileSystem::Redirect_ډول::Fallthrough;
    } وإلا إذا (ارزښت.equals_insensitive("fallback")) {
      أرجع RedirectingFileSystem::Redirect_ډول::Fallback;
    } وإلا إذا (ارزښت.equals_insensitive("redirect-only")) {
      أرجع RedirectingFileSystem::Redirect_ډول::RedirectOnly;
    }
    أرجع std::nullopt;
  }

  std::optional<RedirectingFileSystem::ریښه_Relative_ډول>
  parse_ریښه_Relative_ډول(yaml::غوټه *N) {
    SmallString<12> Storage;
    StringRef ارزښت;
    إذا (!parseScalarString(N, ارزښت, Storage))
      أرجع std::nullopt;
    إذا (ارزښت.equals_insensitive("cwd")) {
      أرجع RedirectingFileSystem::ریښه_Relative_ډول::CWD;
    } وإلا إذا (ارزښت.equals_insensitive("overlay-dir")) {
      أرجع RedirectingFileSystem::ریښه_Relative_ډول::OverlayDir;
    }
    أرجع std::nullopt;
  }

  بنية کلی_حالت {
    منطقي Required;
    منطقي کتل_شوی = خطأ;

    کلی_حالت(منطقي Required = خطأ) : Required(Required) {}
  };

  کارول کلی_حالت_Pair = std::pair<StringRef, کلی_حالت>;

  // false on error
  منطقي check_Duplicate_Or_Unknown_کلی(yaml::غوټه *کلی_غوټه, StringRef کلی,
                                  DenseMap<StringRef, کلی_حالت> &کلیګانې_2) {
    خپلکاری It = کلیګانې_2.پیدا_کړه(کلی);
    إذا (It == کلیګانې_2.پای_ټکی()) {
      error(کلی_غوټه, "unknown key");
      أرجع خطأ;
    }
    کلی_حالت &S = It->دوهم_2;
    إذا (S.کتل_شوی) {
      error(کلی_غوټه, Twine("duplicate key '") + کلی + "'");
      أرجع خطأ;
    }
    S.کتل_شوی = ریښتیا;
    أرجع ریښتیا;
  }

  // false on error
  منطقي check_Missing_کلیګانې(yaml::غوټه *Obj, DenseMap<StringRef, کلی_حالت> &کلیګانې_2) {
    لكل (ثابت خپلکاری &I : کلیګانې_2) {
      إذا (I.دوهم_2.Required && !I.دوهم_2.کتل_شوی) {
        error(Obj, Twine("missing key '") + I.لومړی_2 + "'");
        أرجع خطأ;
      }
    }
    أرجع ریښتیا;
  }

عام:
  ساكن RedirectingFileSystem::ثبت *
  lookup_Or_Create_ثبت(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::ثبت *پلار_ثبت = تش_اشاره) {
    إذا (!پلار_ثبت) { // Look for a existent root
      لكل (ثابت خپلکاری &ریښه_2 : FS->Roots) {
        إذا (Name == ریښه_2->getName()) {
          پلار_ثبت = ریښه_2.get();
          أرجع پلار_ثبت;
        }
      }
    } وإلا { // Advance to the next component
      خپلکاری *DE = dyn_cast<RedirectingFileSystem::Directory_ثبت>(پلار_ثبت);
      لكل (std::unique_ptr<RedirectingFileSystem::ثبت> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_پای_ټکی())) {
        خپلکاری *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_ثبت>(Content.get());
        إذا (DirContent && Name == Content->getName())
          أرجع DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::ثبت> E =
        std::make_unique<RedirectingFileSystem::Directory_ثبت>(
            Name, حالت("", get_بل_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    إذا (!پلار_ثبت) { // Add a new root to the overlay
      FS->Roots.ټېل_شا(std::move(E));
      پلار_ثبت = FS->Roots.شا().get();
      أرجع پلار_ثبت;
    }

    خپلکاری *DE = cast<RedirectingFileSystem::Directory_ثبت>(پلار_ثبت);
    DE->جمع_کړه_Content(std::move(E));
    أرجع DE->get_وروستی_Content();
  }

خاص:
  فراغ unique_Overlay_ونه(RedirectingFileSystem *FS,
                         RedirectingFileSystem::ثبت *SrcE,
                         RedirectingFileSystem::ثبت *New_پلار_E = تش_اشاره) {
    StringRef Name = SrcE->getName();
    بدل (SrcE->get_ډول()) {
    حالة RedirectingFileSystem::EK_Directory: {
      خپلکاری *DE = cast<RedirectingFileSystem::Directory_ثبت>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      إذا (!Name.تشه())
        New_پلار_E = lookup_Or_Create_ثبت(FS, Name, New_پلار_E);
      لكل (std::unique_ptr<RedirectingFileSystem::ثبت> &Sub_ثبت :
           llvm::make_range(DE->contents_begin(), DE->contents_پای_ټکی()))
        unique_Overlay_ونه(FS, Sub_ثبت.get(), New_پلار_E);
      اكسر;
    }
    حالة RedirectingFileSystem::EK_DirectoryRemap: {
      أكد(New_پلار_E && "Parent entry must exist");
      خپلکاری *DR = cast<RedirectingFileSystem::Directory_Remap_ثبت>(SrcE);
      خپلکاری *DE = cast<RedirectingFileSystem::Directory_ثبت>(New_پلار_E);
      DE->جمع_کړه_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_ثبت>(
              Name, DR->get_External_Contents_لار(), DR->getUseName()));
      اكسر;
    }
    حالة RedirectingFileSystem::EK_File: {
      أكد(New_پلار_E && "Parent entry must exist");
      خپلکاری *FE = cast<RedirectingFileSystem::File_ثبت>(SrcE);
      خپلکاری *DE = cast<RedirectingFileSystem::Directory_ثبت>(New_پلار_E);
      DE->جمع_کړه_Content(std::make_unique<RedirectingFileSystem::File_ثبت>(
          Name, FE->get_External_Contents_لار(), FE->getUseName()));
      اكسر;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::ثبت>
  parse_ثبت(yaml::غوټه *N, RedirectingFileSystem *FS, منطقي Is_ریښه_ثبت) {
    خپلکاری *M = dyn_cast<yaml::Mapping_غوټه>(N);
    إذا (!M) {
      error(N, "expected mapping node for file or directory entry");
      أرجع تش_اشاره;
    }

    کلی_حالت_Pair Fields[] = {
        کلی_حالت_Pair("name", ریښتیا),
        کلی_حالت_Pair("type", ریښتیا),
        کلی_حالت_Pair("contents", خطأ),
        کلی_حالت_Pair("external-contents", خطأ),
        کلی_حالت_Pair("use-external-name", خطأ),
    };

    DenseMap<StringRef, کلی_حالت> کلیګانې_2(std::begin(Fields), std::پای_ټکی(Fields));

    تعداد { CF_Not_سېټ, CF_List, CF_External } Contents_ډګر = CF_Not_سېټ;
    std::ویکټور<std::unique_ptr<RedirectingFileSystem::ثبت>>
        ثبت_Array_Contents;
    SmallString<256> External_Contents_لار;
    SmallString<256> Name;
    yaml::غوټه *Name_ارزښت_غوټه = تش_اشاره;
    خپلکاری UseExternalName = RedirectingFileSystem::NK_Not_سېټ;
    RedirectingFileSystem::ثبت_ډول ډول_2;

    لكل (خپلکاری &I : *M) {
      StringRef کلی;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> بفر;
      إذا (!parseScalarString(I.get_کلی(), کلی, بفر))
        أرجع تش_اشاره;

      إذا (!check_Duplicate_Or_Unknown_کلی(I.get_کلی(), کلی, کلیګانې_2))
        أرجع تش_اشاره;

      StringRef ارزښت;
      إذا (کلی == "name") {
        إذا (!parseScalarString(I.get_ارزښت(), ارزښت, بفر))
          أرجع تش_اشاره;

        Name_ارزښت_غوټه = I.get_ارزښت();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(ارزښت).str();
      } وإلا إذا (کلی == "type") {
        إذا (!parseScalarString(I.get_ارزښت(), ارزښت, بفر))
          أرجع تش_اشاره;
        إذا (ارزښت == "file")
          ډول_2 = RedirectingFileSystem::EK_File;
        وإلا إذا (ارزښت == "directory")
          ډول_2 = RedirectingFileSystem::EK_Directory;
        وإلا إذا (ارزښت == "directory-remap")
          ډول_2 = RedirectingFileSystem::EK_DirectoryRemap;
        وإلا {
          error(I.get_ارزښت(), "unknown value for 'type'");
          أرجع تش_اشاره;
        }
      } وإلا إذا (کلی == "contents") {
        إذا (Contents_ډګر != CF_Not_سېټ) {
          error(I.get_کلی(),
                "entry already has 'contents' or 'external-contents'");
          أرجع تش_اشاره;
        }
        Contents_ډګر = CF_List;
        خپلکاری *Contents = dyn_cast_if_present<yaml::Sequence_غوټه>(I.get_ارزښت());
        إذا (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_ارزښت(), "expected array");
          أرجع تش_اشاره;
        }

        لكل (خپلکاری &I : *Contents) {
          إذا (std::unique_ptr<RedirectingFileSystem::ثبت> E =
                  parse_ثبت(&I, FS, /*IsRootEntry*/ خطأ))
            ثبت_Array_Contents.ټېل_شا(std::move(E));
          وإلا
            أرجع تش_اشاره;
        }
      } وإلا إذا (کلی == "external-contents") {
        إذا (Contents_ډګر != CF_Not_سېټ) {
          error(I.get_کلی(),
                "entry already has 'contents' or 'external-contents'");
          أرجع تش_اشاره;
        }
        Contents_ډګر = CF_External;
        إذا (!parseScalarString(I.get_ارزښت(), ارزښت, بفر))
          أرجع تش_اشاره;

        SmallString<256> ډک_لار;
        إذا (FS->IsRelativeOverlay) {
          ډک_لار = FS->getOverlayFileDir();
          أكد(!ډک_لار.تشه() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_ډک_لار = ارزښت;
          إذا (FS->makeAbsolute(ډک_لار, Abs_ډک_لار)) {
            error(N, "failed to make 'external-contents' absolute");
            أرجع تش_اشاره;
          }
          ډک_لار = Abs_ډک_لار;
        } وإلا {
          ډک_لار = ارزښت;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        ډک_لار = canonicalize(ډک_لار);
        External_Contents_لار = ډک_لار.str();
      } وإلا إذا (کلی == "use-external-name") {
        منطقي Val;
        إذا (!parseScalarBool(I.get_ارزښت(), Val))
          أرجع تش_اشاره;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (بهیر.failed())
      أرجع تش_اشاره;

    // check for missing keys
    إذا (Contents_ډګر == CF_Not_سېټ) {
      error(N, "missing key 'contents' or 'external-contents'");
      أرجع تش_اشاره;
    }
    إذا (!check_Missing_کلیګانې(N, کلیګانې_2))
      أرجع تش_اشاره;

    // check invalid configuration
    إذا (ډول_2 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_سېټ) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      أرجع تش_اشاره;
    }

    إذا (ډول_2 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_ډګر == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      أرجع تش_اشاره;
    }

    sys::لار_2::Style لار_style = sys::لار_2::Style::native;
    إذا (Is_ریښه_ثبت) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      إذا (sys::لار_2::is_absolute(Name, sys::لار_2::Style::posix)) {
        لار_style = sys::لار_2::Style::posix;
      } وإلا إذا (sys::لار_2::is_absolute(Name,
                                        sys::لار_2::Style::windows_backslash)) {
        لار_style = sys::لار_2::Style::windows_backslash;
      } وإلا {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_کوډ EC;
        إذا (FS->ریښه_Relative ==
            RedirectingFileSystem::ریښه_Relative_ډول::OverlayDir) {
          StringRef ډک_لار = FS->getOverlayFileDir();
          أكد(!ډک_لار.تشه() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(ډک_لار, Name);
          Name = canonicalize(Name);
        } وإلا {
          EC = FS->makeAbsolute(Name);
        }
        إذا (EC) {
          أكد(Name_ارزښت_غوټه && "Name presence should be checked earlier");
          error(
              Name_ارزښت_غوټه,
              "entry with relative path at the root level is not discoverable");
          أرجع تش_اشاره;
        }
        لار_style = sys::لار_2::is_absolute(Name, sys::لار_2::Style::posix)
                         ? sys::لار_2::Style::posix
                         : sys::لار_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      إذا (لار_style == sys::لار_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::لار_2::Style::windows_backslash)
        لار_style = sys::لار_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t ریښه_لار_Len = sys::لار_2::ریښه_لار_2(Trimmed, لار_style).اندازه_2();
    طالما (Trimmed.اندازه_2() > ریښه_لار_Len &&
           sys::لار_2::is_separator(Trimmed.شا(), لار_style))
      Trimmed = Trimmed.ټوټه(0, Trimmed.اندازه_2() - 1);

    // Get the last component
    StringRef وروستی_Component = sys::لار_2::filename(Trimmed, لار_style);

    std::unique_ptr<RedirectingFileSystem::ثبت> پایله;
    بدل (ډول_2) {
    حالة RedirectingFileSystem::EK_File:
      پایله = std::make_unique<RedirectingFileSystem::File_ثبت>(
          وروستی_Component, std::move(External_Contents_لار), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_DirectoryRemap:
      پایله = std::make_unique<RedirectingFileSystem::Directory_Remap_ثبت>(
          وروستی_Component, std::move(External_Contents_لار), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_Directory:
      پایله = std::make_unique<RedirectingFileSystem::Directory_ثبت>(
          وروستی_Component, std::move(ثبت_Array_Contents),
          حالت("", get_بل_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      اكسر;
    }

    StringRef پلار_2 = sys::لار_2::پلار_لار(Trimmed, لار_style);
    إذا (پلار_2.تشه())
      أرجع پایله;

    // if 'name' contains multiple components, create implicit directory entries
    لكل (sys::لار_2::سرچپه_iterator I = sys::لار_2::rbegin(پلار_2, لار_style),
                                     E = sys::لار_2::rend(پلار_2);
         I != E; ++I) {
      std::ویکټور<std::unique_ptr<RedirectingFileSystem::ثبت>> Entries;
      Entries.ټېل_شا(std::move(پایله));
      پایله = std::make_unique<RedirectingFileSystem::Directory_ثبت>(
          *I, std::move(Entries),
          حالت("", get_بل_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    أرجع پایله;
  }

عام:
  RedirectingFileSystemParser(yaml::بهیر &S) : بهیر(S) {}

  // false on error
  منطقي parse(yaml::غوټه *ریښه_2, RedirectingFileSystem *FS) {
    خپلکاری *سر_3 = dyn_cast<yaml::Mapping_غوټه>(ریښه_2);
    إذا (!سر_3) {
      error(ریښه_2, "expected mapping node");
      أرجع خطأ;
    }

    کلی_حالت_Pair Fields[] = {
        کلی_حالت_Pair("version", ریښتیا),
        کلی_حالت_Pair("case-sensitive", خطأ),
        کلی_حالت_Pair("use-external-names", خطأ),
        کلی_حالت_Pair("root-relative", خطأ),
        کلی_حالت_Pair("overlay-relative", خطأ),
        کلی_حالت_Pair("fallthrough", خطأ),
        کلی_حالت_Pair("redirecting-with", خطأ),
        کلی_حالت_Pair("roots", ریښتیا),
    };

    DenseMap<StringRef, کلی_حالت> کلیګانې_2(std::begin(Fields), std::پای_ټکی(Fields));
    std::ویکټور<std::unique_ptr<RedirectingFileSystem::ثبت>> ریښه_Entries;

    // Parse configuration and 'roots'
    لكل (خپلکاری &I : *سر_3) {
      SmallString<10> کلی_بفر;
      StringRef کلی;
      إذا (!parseScalarString(I.get_کلی(), کلی, کلی_بفر))
        أرجع خطأ;

      إذا (!check_Duplicate_Or_Unknown_کلی(I.get_کلی(), کلی, کلیګانې_2))
        أرجع خطأ;

      إذا (کلی == "roots") {
        خپلکاری *Roots = dyn_cast_if_present<yaml::Sequence_غوټه>(I.get_ارزښت());
        إذا (!Roots) {
          error(I.get_ارزښت(), "expected array");
          أرجع خطأ;
        }

        لكل (خپلکاری &I : *Roots) {
          إذا (std::unique_ptr<RedirectingFileSystem::ثبت> E =
                  parse_ثبت(&I, FS, /*IsRootEntry*/ ریښتیا))
            ریښه_Entries.ټېل_شا(std::move(E));
          وإلا
            أرجع خطأ;
        }
      } وإلا إذا (کلی == "version") {
        StringRef نسخه_String;
        SmallString<4> Storage;
        إذا (!parseScalarString(I.get_ارزښت(), نسخه_String, Storage))
          أرجع خطأ;
        صحيح نسخه;
        إذا (نسخه_String.getAsInteger<صحيح>(10, نسخه)) {
          error(I.get_ارزښت(), "expected integer");
          أرجع خطأ;
        }
        إذا (نسخه < 0) {
          error(I.get_ارزښت(), "invalid version number");
          أرجع خطأ;
        }
        إذا (نسخه != 0) {
          error(I.get_ارزښت(), "version mismatch, expected 0");
          أرجع خطأ;
        }
      } وإلا إذا (کلی == "case-sensitive") {
        إذا (!parseScalarBool(I.get_ارزښت(), FS->CaseSensitive))
          أرجع خطأ;
      } وإلا إذا (کلی == "overlay-relative") {
        إذا (!parseScalarBool(I.get_ارزښت(), FS->IsRelativeOverlay))
          أرجع خطأ;
      } وإلا إذا (کلی == "use-external-names") {
        إذا (!parseScalarBool(I.get_ارزښت(), FS->Use_External_نومونه))
          أرجع خطأ;
      } وإلا إذا (کلی == "fallthrough") {
        إذا (کلیګانې_2["redirecting-with"].کتل_شوی) {
          error(I.get_ارزښت(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        منطقي ShouldFallthrough = خطأ;
        إذا (!parseScalarBool(I.get_ارزښت(), ShouldFallthrough))
          أرجع خطأ;

        إذا (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::Redirect_ډول::Fallthrough;
        } وإلا {
          FS->Redirection = RedirectingFileSystem::Redirect_ډول::RedirectOnly;
        }
      } وإلا إذا (کلی == "redirecting-with") {
        إذا (کلیګانې_2["fallthrough"].کتل_شوی) {
          error(I.get_ارزښت(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        إذا (خپلکاری ډول_2 = parse_Redirect_ډول(I.get_ارزښت())) {
          FS->Redirection = *ډول_2;
        } وإلا {
          error(I.get_ارزښت(), "expected valid redirect kind");
          أرجع خطأ;
        }
      } وإلا إذا (کلی == "root-relative") {
        إذا (خپلکاری ډول_2 = parse_ریښه_Relative_ډول(I.get_ارزښت())) {
          FS->ریښه_Relative = *ډول_2;
        } وإلا {
          error(I.get_ارزښت(), "expected valid root-relative kind");
          أرجع خطأ;
        }
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (بهیر.failed())
      أرجع خطأ;

    إذا (!check_Missing_کلیګانې(سر_3, کلیګانې_2))
      أرجع خطأ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    لكل (خپلکاری &E : ریښه_Entries)
      unique_Overlay_ونه(FS, E.get());

    أرجع ریښتیا;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<حافظه_بفر> بفر,
                              سرچینه_Mgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_لار, فراغ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  سرچینه_Mgr SM;
  yaml::بهیر بهیر(بفر->get_Mem_بفر_Ref(), SM);

  SM.سېټ_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = بهیر.begin();
  yaml::غوټه *ریښه_2 = DI->get_ریښه();
  إذا (DI == بهیر.پای_ټکی() || !ریښه_2) {
    SM.Print_پیغام(SMLoc(), سرچینه_Mgr::DK_Error, "expected root node");
    أرجع تش_اشاره;
  }

  RedirectingFileSystemParser P(بهیر);

  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));

  إذا (!YAMLFile_لار.تشه()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::لار_2::پلار_لار(YAMLFile_لار);
    std::error_کوډ EC = FS->makeAbsolute(OverlayAbsDir);
    أكد(!EC && "Overlay dir final path must be absolute");
    (فراغ)EC;
    FS->سېټ_Overlay_File_Dir(OverlayAbsDir);
  }

  إذا (!P.parse(ریښه_2, FS.get()))
    أرجع تش_اشاره;

  أرجع FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    منطقي Use_External_نومونه, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));
  FS->Use_External_نومونه = Use_External_نومونه;

  StringMap<RedirectingFileSystem::ثبت *> Entries;

  لكل (خپلکاری &Mapping : llvm::سرچپه(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.لومړی_2);
    SmallString<128> منزل = StringRef(Mapping.دوهم_2);
    {
      خپلکاری EC = ExternalFS->makeAbsolute(From);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::ثبت *&منزل_ثبت = Entries[From];
    إذا (منزل_ثبت)
      استمر;

    // Add parent directories.
    RedirectingFileSystem::ثبت *پلار_2 = تش_اشاره;
    StringRef FromDirectory = llvm::sys::لار_2::پلار_لار(From);
    لكل (خپلکاری I = llvm::sys::لار_2::begin(FromDirectory),
              E = llvm::sys::لار_2::پای_ټکی(FromDirectory);
         I != E; ++I) {
      پلار_2 = RedirectingFileSystemParser::lookup_Or_Create_ثبت(FS.get(), *I,
                                                                پلار_2);
    }
    أكد(پلار_2 && "File without a directory?");
    {
      خپلکاری EC = ExternalFS->makeAbsolute(منزل);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Add the file.
    خپلکاری NewFile = std::make_unique<RedirectingFileSystem::File_ثبت>(
        llvm::sys::لار_2::filename(From), منزل,
        Use_External_نومونه ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    منزل_ثبت = NewFile.get();
    cast<RedirectingFileSystem::Directory_ثبت>(پلار_2)->جمع_کړه_Content(
        std::move(NewFile));
  }

  أرجع FS;
}

RedirectingFileSystem::Lookup_پایله::Lookup_پایله(
    ثبت *E, sys::لار_2::const_iterator پیل, sys::لار_2::const_iterator پای_ټکی_2)
    : E(E) {
  أكد(E != تش_اشاره);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  إذا (خپلکاری *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_ثبت>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_لار());
    sys::لار_2::append(Redirect, پیل, پای_ټکی_2,
                      getExistingStyle(DRE->get_External_Contents_لار()));
    ExternalRedirect = std::string(Redirect);
  }
}

فراغ RedirectingFileSystem::Lookup_پایله::get_لار(
    llvm::Small_ویکټور_Impl<حرف> &پایله) ثابت {
  پایله.clear();
  لكل (ثبت *پلار_2 : Parents)
    llvm::sys::لار_2::append(پایله, پلار_2->getName());
  llvm::sys::لار_2::append(پایله, E->getName());
}

std::error_کوډ RedirectingFileSystem::makeCanonicalForLookup(
    Small_ویکټور_Impl<حرف> &لار) ثابت {
  إذا (std::error_کوډ EC = makeAbsolute(لار))
    أرجع EC;

  llvm::SmallString<256> Canonical_لار =
      canonicalize(StringRef(لار.معلومات(), لار.اندازه_2()));
  إذا (Canonical_لار.تشه())
    أرجع make_error_کوډ(llvm::errc::invalid_دلیل);

  لار.ټاکل(Canonical_لار.begin(), Canonical_لار.پای_ټکی());
  أرجع {};
}

ErrorOr<RedirectingFileSystem::Lookup_پایله>
RedirectingFileSystem::lookup_لار(StringRef لار) ثابت {
  llvm::SmallString<128> Canonical_لار(لار);
  إذا (std::error_کوډ EC = makeCanonicalForLookup(Canonical_لار))
    أرجع EC;

  // RedirectOnly means the VFS is always used.
  إذا (Usage_Tracking_فعال && Redirection == Redirect_ډول::RedirectOnly)
    Has_Been_کارول_شوی = ریښتیا;

  sys::لار_2::const_iterator پیل = sys::لار_2::begin(Canonical_لار);
  sys::لار_2::const_iterator پای_ټکی_2 = sys::لار_2::پای_ټکی(Canonical_لار);
  llvm::Small_ویکټور<ثبت *, 32> Entries;
  لكل (ثابت خپلکاری &ریښه_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_پایله> پایله =
        lookup_لار_Impl(پیل, پای_ټکی_2, ریښه_2.get(), Entries);
    إذا (Usage_Tracking_فعال && پایله && isa<Remap_ثبت>(پایله->E))
      Has_Been_کارول_شوی = ریښتیا;
    إذا (پایله) {
      پایله->Parents = std::move(Entries);
      أرجع پایله;
    }

    إذا (پایله.getError() != llvm::errc::no_such_file_or_directory)
      أرجع پایله;
  }
  أرجع make_error_کوډ(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_پایله>
RedirectingFileSystem::lookup_لار_Impl(
    sys::لار_2::const_iterator پیل, sys::لار_2::const_iterator پای_ټکی_2,
    RedirectingFileSystem::ثبت *From,
    llvm::Small_ویکټور_Impl<ثبت *> &Entries) ثابت {
  أكد(!isTraversalComponent(*پیل) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  إذا (!FromName.تشه()) {
    إذا (!لار_Component_Matches(*پیل, FromName))
      أرجع make_error_کوډ(llvm::errc::no_such_file_or_directory);

    ++پیل;

    إذا (پیل == پای_ټکی_2) {
      // Match!
      أرجع Lookup_پایله(From, پیل, پای_ټکی_2);
    }
  }

  إذا (isa<RedirectingFileSystem::File_ثبت>(From))
    أرجع make_error_کوډ(llvm::errc::not_a_directory);

  إذا (isa<RedirectingFileSystem::Directory_Remap_ثبت>(From))
    أرجع Lookup_پایله(From, پیل, پای_ټکی_2);

  خپلکاری *DE = cast<RedirectingFileSystem::Directory_ثبت>(From);
  لكل (ثابت std::unique_ptr<RedirectingFileSystem::ثبت> &Dir_ثبت :
       llvm::make_range(DE->contents_begin(), DE->contents_پای_ټکی())) {
    Entries.ټېل_شا(From);
    ErrorOr<RedirectingFileSystem::Lookup_پایله> پایله =
        lookup_لار_Impl(پیل, پای_ټکی_2, Dir_ثبت.get(), Entries);
    إذا (پایله || پایله.getError() != llvm::errc::no_such_file_or_directory)
      أرجع پایله;
    Entries.راوباسه_شا();
  }

  أرجع make_error_کوډ(llvm::errc::no_such_file_or_directory);
}

ساكن حالت get_Redirected_File_حالت(ثابت Twine &Original_لار,
                                      منطقي Use_External_نومونه,
                                      حالت External_حالت) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  إذا (External_حالت.ExposesExternalVFSPath)
    أرجع External_حالت;

  حالت S = External_حالت;
  إذا (!Use_External_نومونه)
    S = حالت::copyWithNewName(S, Original_لار);
  وإلا
    S.ExposesExternalVFSPath = ریښتیا;
  أرجع S;
}

ErrorOr<حالت> RedirectingFileSystem::حالت_2(
    ثابت Twine &Lookup_لار, ثابت Twine &Original_لار,
    ثابت RedirectingFileSystem::Lookup_پایله &پایله) {
  إذا (std::optional<StringRef> ExtRedirect = پایله.getExternalRedirect()) {
    SmallString<256> Remapped_لار((*ExtRedirect).str());
    إذا (std::error_کوډ EC = makeAbsolute(Remapped_لار))
      أرجع EC;

    ErrorOr<حالت> S = ExternalFS->حالت_2(Remapped_لار);
    إذا (!S)
      أرجع S;
    S = حالت::copyWithNewName(*S, *ExtRedirect);
    خپلکاری *RE = cast<RedirectingFileSystem::Remap_ثبت>(پایله.E);
    أرجع get_Redirected_File_حالت(Original_لار,
                                   RE->useExternalName(Use_External_نومونه), *S);
  }

  خپلکاری *DE = cast<RedirectingFileSystem::Directory_ثبت>(پایله.E);
  أرجع حالت::copyWithNewName(DE->get_حالت(), Lookup_لار);
}

ErrorOr<حالت>
RedirectingFileSystem::get_External_حالت(ثابت Twine &Lookup_لار,
                                         ثابت Twine &Original_لار) ثابت {
  خپلکاری پایله = ExternalFS->حالت_2(Lookup_لار);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  إذا (!پایله || پایله->ExposesExternalVFSPath)
    أرجع پایله;
  أرجع حالت::copyWithNewName(پایله.get(), Original_لار);
}

ErrorOr<حالت> RedirectingFileSystem::حالت_2(ثابت Twine &Original_لار) {
  SmallString<256> لار;
  Original_لار.منزل_ویکټور(لار);

  إذا (std::error_کوډ EC = makeAbsolute(لار))
    أرجع EC;

  إذا (Redirection == Redirect_ډول::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<حالت> S = get_External_حالت(لار, Original_لار);
    إذا (S)
      أرجع S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_پایله> پایله = lookup_لار(لار);
  إذا (!پایله) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_ډول::Fallthrough &&
        is_File_Not_وموندل(پایله.getError()))
      أرجع get_External_حالت(لار, Original_لار);
    أرجع پایله.getError();
  }

  ErrorOr<حالت> S = حالت_2(لار, Original_لار, *پایله);
  إذا (!S && Redirection == Redirect_ډول::Fallthrough &&
      is_File_Not_وموندل(S.getError(), پایله->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع get_External_حالت(لار, Original_لار);
  }

  أرجع S;
}

منطقي RedirectingFileSystem::exists(ثابت Twine &Original_لار) {
  SmallString<256> لار;
  Original_لار.منزل_ویکټور(لار);

  إذا (makeAbsolute(لار))
    أرجع خطأ;

  إذا (Redirection == Redirect_ډول::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    إذا (ExternalFS->exists(لار))
      أرجع ریښتیا;
  }

  ErrorOr<RedirectingFileSystem::Lookup_پایله> پایله = lookup_لار(لار);
  إذا (!پایله) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_ډول::Fallthrough &&
        is_File_Not_وموندل(پایله.getError()))
      أرجع ExternalFS->exists(لار);
    أرجع خطأ;
  }

  std::optional<StringRef> ExtRedirect = پایله->getExternalRedirect();
  إذا (!ExtRedirect) {
    أكد(isa<RedirectingFileSystem::Directory_ثبت>(پایله->E));
    أرجع ریښتیا;
  }

  SmallString<256> Remapped_لار((*ExtRedirect).str());
  إذا (makeAbsolute(Remapped_لار))
    أرجع خطأ;

  إذا (ExternalFS->exists(Remapped_لار))
    أرجع ریښتیا;

  إذا (Redirection == Redirect_ډول::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع ExternalFS->exists(لار);
  }

  أرجع خطأ;
}

نطاق {

/// Provide a file wrapper with an overriden status.
صنف File_With_Fixed_حالت : عام File {
  std::unique_ptr<File> InnerFile;
  حالت S;

عام:
  File_With_Fixed_حالت(std::unique_ptr<File> InnerFile, حالت S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<حالت> حالت_2() override { أرجع S; }
  ErrorOr<std::unique_ptr<llvm::حافظه_بفر>>

  get_بفر(ثابت Twine &Name, int64_t File_اندازه, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    أرجع InnerFile->get_بفر(Name, File_اندازه, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_کوډ close() override { أرجع InnerFile->close(); }

  فراغ سېټ_لار(ثابت Twine &لار) override { S = S.copyWithNewName(S, لار); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_لار(ErrorOr<std::unique_ptr<File>> پایله, ثابت Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  إذا (!پایله || (*پایله)->حالت_2()->ExposesExternalVFSPath)
    أرجع پایله;

  ErrorOr<std::unique_ptr<File>> F = std::move(*پایله);
  خپلکاری Name = F->get()->getName();
  إذا (Name && Name.get() != P.str())
    F->get()->سېټ_لار(P);
  أرجع F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ثابت Twine &Original_لار) {
  SmallString<256> لار;
  Original_لار.منزل_ویکټور(لار);

  إذا (std::error_کوډ EC = makeAbsolute(لار))
    أرجع EC;

  إذا (Redirection == Redirect_ډول::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    خپلکاری F = File::get_With_لار(ExternalFS->openFileForRead(لار), Original_لار);
    إذا (F)
      أرجع F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_پایله> پایله = lookup_لار(لار);
  إذا (!پایله) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_ډول::Fallthrough &&
        is_File_Not_وموندل(پایله.getError()))
      أرجع File::get_With_لار(ExternalFS->openFileForRead(لار), Original_لار);
    أرجع پایله.getError();
  }

  إذا (!پایله->getExternalRedirect()) // FIXME: errc::not_a_file?
    أرجع make_error_کوډ(llvm::errc::invalid_دلیل);

  StringRef ExtRedirect = *پایله->getExternalRedirect();
  SmallString<256> Remapped_لار(ExtRedirect.str());
  إذا (std::error_کوډ EC = makeAbsolute(Remapped_لار))
    أرجع EC;

  خپلکاری *RE = cast<RedirectingFileSystem::Remap_ثبت>(پایله->E);

  خپلکاری ExternalFile =
      File::get_With_لار(ExternalFS->openFileForRead(Remapped_لار), ExtRedirect);
  إذا (!ExternalFile) {
    إذا (Redirection == Redirect_ډول::Fallthrough &&
        is_File_Not_وموندل(ExternalFile.getError(), پایله->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع File::get_With_لار(ExternalFS->openFileForRead(لار), Original_لار);
    }
    أرجع ExternalFile;
  }

  خپلکاری External_حالت = (*ExternalFile)->حالت_2();
  إذا (!External_حالت)
    أرجع External_حالت.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  حالت S = get_Redirected_File_حالت(
      Original_لار, RE->useExternalName(Use_External_نومونه), *External_حالت);
  أرجع std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_حالت>(std::move(*ExternalFile), S));
}

std::error_کوډ
RedirectingFileSystem::get_Real_لار(ثابت Twine &Original_لار,
                                   Small_ویکټور_Impl<حرف> &Output) {
  SmallString<256> لار;
  Original_لار.منزل_ویکټور(لار);

  إذا (std::error_کوډ EC = makeAbsolute(لار))
    أرجع EC;

  إذا (Redirection == Redirect_ډول::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_کوډ EC = ExternalFS->get_Real_لار(لار, Output);
    إذا (!EC)
      أرجع EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_پایله> پایله = lookup_لار(لار);
  إذا (!پایله) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_ډول::Fallthrough &&
        is_File_Not_وموندل(پایله.getError()))
      أرجع ExternalFS->get_Real_لار(لار, Output);
    أرجع پایله.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  إذا (خپلکاری ExtRedirect = پایله->getExternalRedirect()) {
    خپلکاری P = ExternalFS->get_Real_لار(*ExtRedirect, Output);
    إذا (P && Redirection == Redirect_ډول::Fallthrough &&
        is_File_Not_وموندل(P, پایله->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع ExternalFS->get_Real_لار(لار, Output);
    }
    أرجع P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  إذا (Redirection == Redirect_ډول::Fallthrough) {
    پایله->get_لار(Output);
    أرجع {};
  }
  أرجع llvm::errc::invalid_دلیل;
}

فراغ RedirectingFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_ویکټور_Impl<std::string> &بهر) {
  SmallString<256> لار;
  Dir.منزل_ویکټور(لار);

  إذا (makeAbsolute(لار))
    أرجع;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ثابت منطقي ConsultsExternalFS = Redirection != Redirect_ډول::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_پایله> پایله = lookup_لار(لار);
  إذا (!پایله) {
    // dir_begin() delegates entirely to ExternalFS.
    إذا (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(لار, بهر);
    أرجع;
  }

  بدل (پایله->E->get_ډول()) {
  حالة EK_File:
    أرجع;
  حالة EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    اكسر;
  حالة EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_لار(*پایله->getExternalRedirect());
    إذا (!makeAbsolute(Remapped_لار))
      ExternalFS->getDirectoryContentRealSources(Remapped_لار, بهر);
    اكسر;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  إذا (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(لار, بهر);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<حافظه_بفر> بفر,
                    سرچینه_Mgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_لار, فراغ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  أرجع RedirectingFileSystem::create(std::move(بفر), DiagHandler,
                                       YAMLFile_لار, DiagContext,
                                       std::move(ExternalFS));
}

ساكن فراغ getVFSEntries(RedirectingFileSystem::ثبت *SrcE,
                          Small_ویکټور_Impl<StringRef> &لار,
                          Small_ویکټور_Impl<YAMLVFSEntry> &Entries) {
  خپلکاری ډول_2 = SrcE->get_ډول();
  إذا (ډول_2 == RedirectingFileSystem::EK_Directory) {
    خپلکاری *DE = dyn_cast<RedirectingFileSystem::Directory_ثبت>(SrcE);
    أكد(DE && "Must be a directory");
    لكل (std::unique_ptr<RedirectingFileSystem::ثبت> &Sub_ثبت :
         llvm::make_range(DE->contents_begin(), DE->contents_پای_ټکی())) {
      لار.ټېل_شا(Sub_ثبت->getName());
      getVFSEntries(Sub_ثبت.get(), لار, Entries);
      لار.راوباسه_شا();
    }
    أرجع;
  }

  إذا (ډول_2 == RedirectingFileSystem::EK_DirectoryRemap) {
    خپلکاری *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_ثبت>(SrcE);
    أكد(DR && "Must be a directory remap");
    SmallString<128> VPath;
    لكل (خپلکاری &Comp : لار)
      llvm::sys::لار_2::append(VPath, Comp);
    Entries.ټېل_شا(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_لار()));
    أرجع;
  }

  أكد(ډول_2 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  خپلکاری *FE = dyn_cast<RedirectingFileSystem::File_ثبت>(SrcE);
  أكد(FE && "Must be a file");
  SmallString<128> VPath;
  لكل (خپلکاری &Comp : لار)
    llvm::sys::لار_2::append(VPath, Comp);
  Entries.ټېل_شا(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_لار()));
}

فراغ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_ویکټور_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_پایله> ریښه_پایله = VFS.lookup_لار("/");
  إذا (!ریښه_پایله)
    أرجع;
  Small_ویکټور<StringRef, 8> Components;
  Components.ټېل_شا("/");
  getVFSEntries(ریښه_پایله->E, Components, CollectedEntries);
}

UniqueID vfs::get_بل_Virtual_Unique_ID() {
  ساكن std::atomic<بینښه> UID;
  بینښه ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  أرجع UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

فراغ YAMLVFSWriter::جمع_کړه_ثبت(StringRef Virtual_لار, StringRef Real_لار,
                             منطقي IsDirectory) {
  أكد(sys::لار_2::is_absolute(Virtual_لار) && "virtual path not absolute");
  أكد(sys::لار_2::is_absolute(Real_لار) && "real path not absolute");
  أكد(!لار_Has_Traversal(Virtual_لار) && "path traversal is not supported");
  Mappings.emplace_شا(Virtual_لار, Real_لار, IsDirectory);
}

فراغ YAMLVFSWriter::جمع_کړه_File_Mapping(StringRef Virtual_لار, StringRef Real_لار) {
  جمع_کړه_ثبت(Virtual_لار, Real_لار, /*IsDirectory=*/خطأ);
}

فراغ YAMLVFSWriter::جمع_کړه_Directory_Mapping(StringRef Virtual_لار,
                                        StringRef Real_لار) {
  جمع_کړه_ثبت(Virtual_لار, Real_لار, /*IsDirectory=*/ریښتیا);
}

نطاق {

صنف JSONWriter {
  llvm::raw_ostream &OS;
  Small_ویکټور<StringRef, 16> Dir_ډېری;

  بینښه getDirIndent() { أرجع 4 * Dir_ډېری.اندازه_2(); }
  بینښه getFileIndent() { أرجع 4 * (Dir_ډېری.اندازه_2() + 1); }
  منطقي contained_دننه(StringRef پلار_2, StringRef لار);
  StringRef containedPart(StringRef پلار_2, StringRef لار);
  فراغ پیل_Directory(StringRef لار);
  فراغ پای_ټکی_Directory();
  فراغ write_ثبت(StringRef VPath, StringRef RPath);

عام:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  فراغ اكتب(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<منطقي> Use_External_نومونه,
             std::optional<منطقي> IsCaseSensitive,
             std::optional<منطقي> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

منطقي JSONWriter::contained_دننه(StringRef پلار_2, StringRef لار) {
  کارول نطاق llvm::sys;

  // Compare each path component.
  خپلکاری IParent = لار_2::begin(پلار_2), EParent = لار_2::پای_ټکی(پلار_2);
  لكل (خپلکاری IChild = لار_2::begin(لار), EChild = لار_2::پای_ټکی(لار);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    إذا (*IParent != *IChild)
      أرجع خطأ;
  }
  // Have we exhausted the parent path?
  أرجع IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef پلار_2, StringRef لار) {
  أكد(!پلار_2.تشه());
  أكد(contained_دننه(پلار_2, لار));
  أرجع لار.substr(پلار_2.اندازه_2() + 1);
}

فراغ JSONWriter::پیل_Directory(StringRef لار) {
  StringRef Name =
      Dir_ډېری.تشه() ? لار : containedPart(Dir_ډېری.شا(), لار);
  Dir_ډېری.ټېل_شا(لار);
  بینښه Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

فراغ JSONWriter::پای_ټکی_Directory() {
  بینښه Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ډېری.راوباسه_شا();
}

فراغ JSONWriter::write_ثبت(StringRef VPath, StringRef RPath) {
  بینښه Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

فراغ JSONWriter::اكتب(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<منطقي> Use_External_نومونه,
                       std::optional<منطقي> IsCaseSensitive,
                       std::optional<منطقي> IsOverlayRelative,
                       StringRef OverlayDir) {
  کارول نطاق llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  إذا (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  إذا (Use_External_نومونه)
    OS << "  'use-external-names': '" << (*Use_External_نومونه ? "true" : "false")
       << "',\n";
  منطقي UseOverlayRelative = خطأ;
  إذا (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  إذا (!Entries.تشه()) {
    ثابت YAMLVFSEntry &ثبت = Entries.front();

    پیل_Directory(
      ثبت.IsDirectory ? ثبت.VPath : لار_2::پلار_لار(ثبت.VPath)
    );

    StringRef RPath = ثبت.RPath;
    إذا (UseOverlayRelative) {
      أكد(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.اندازه_2());
    }

    منطقي Is_Current_Dir_تشه = ریښتیا;
    إذا (!ثبت.IsDirectory) {
      write_ثبت(لار_2::filename(ثبت.VPath), RPath);
      Is_Current_Dir_تشه = خطأ;
    }

    لكل (ثابت خپلکاری &ثبت : Entries.ټوټه(1)) {
      StringRef Dir =
          ثبت.IsDirectory ? ثبت.VPath : لار_2::پلار_لار(ثبت.VPath);
      إذا (Dir == Dir_ډېری.شا()) {
        إذا (!Is_Current_Dir_تشه) {
          OS << ",\n";
        }
      } وإلا {
        منطقي Is_Dir_Popped_From_ډېری = خطأ;
        طالما (!Dir_ډېری.تشه() && !contained_دننه(Dir_ډېری.شا(), Dir)) {
          OS << "\n";
          پای_ټکی_Directory();
          Is_Dir_Popped_From_ډېری = ریښتیا;
        }
        إذا (Is_Dir_Popped_From_ډېری || !Is_Current_Dir_تشه) {
          OS << ",\n";
        }
        پیل_Directory(Dir);
        Is_Current_Dir_تشه = ریښتیا;
      }
      StringRef RPath = ثبت.RPath;
      إذا (UseOverlayRelative) {
        أكد(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.اندازه_2());
      }
      إذا (!ثبت.IsDirectory) {
        write_ثبت(لار_2::filename(ثبت.VPath), RPath);
        Is_Current_Dir_تشه = خطأ;
      }
    }

    طالما (!Dir_ډېری.تشه()) {
      OS << "\n";
      پای_ټکی_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

فراغ YAMLVFSWriter::اكتب(llvm::raw_ostream &OS) {
  llvm::ترتیبول(Mappings, [](ثابت YAMLVFSEntry &LHS, ثابت YAMLVFSEntry &RHS) {
    أرجع LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).اكتب(Mappings, Use_External_نومونه, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ثابت Twine &لار, std::error_کوډ &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(لار, EC);
  إذا (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ډېری.ټېل_شا(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_کوډ &EC) {
  أكد(FS && State && !State->ډېری.تشه() && "incrementing past end");
  أكد(!State->ډېری.شا()->لار_2().تشه() && "non-canonical end iterator");
  vfs::directory_iterator پای_ټکی_2;

  إذا (State->Has_No_ټېل_Request)
    State->Has_No_ټېل_Request = خطأ;
  وإلا {
    إذا (State->ډېری.شا()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ډېری.شا()->لار_2(), EC);
      إذا (I != پای_ټکی_2) {
        State->ډېری.ټېل_شا(I);
        أرجع *هذا;
      }
    }
  }

  طالما (!State->ډېری.تشه() && State->ډېری.شا().increment(EC) == پای_ټکی_2)
    State->ډېری.راوباسه_شا();

  إذا (State->ډېری.تشه())
    State.reset(); // end iterator

  أرجع *هذا;
}

ثابت حرف FileSystem::ID = 0;
ثابت حرف OverlayFileSystem::ID = 0;
ثابت حرف ProxyFileSystem::ID = 0;
ثابت حرف دننه_حافظه_File_System::ID = 0;
ثابت حرف RedirectingFileSystem::ID = 0;

بینښه ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_شمېر(
    ثابت FileSystem *FS) {
  أرجع FS->Use_شمېر();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
