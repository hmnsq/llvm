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

استفاده نطاق llvm;
استفاده نطاق llvm::vfs;

استفاده llvm::sys::fs::file_وضعیت;
استفاده llvm::sys::fs::file_t;
استفاده llvm::sys::fs::file_type;
استفاده llvm::sys::fs::perms;
استفاده llvm::sys::fs::UniqueID;

وضعیت::وضعیت(ثابت file_وضعیت &وضعیت)
    : UID(وضعیت.getUniqueID()), MTime(وضعیت.get_آخرین_Modification_زمان()),
      User(وضعیت.getUser()), گروه(وضعیت.get_گروه()), اندازه(وضعیت.get_اندازه()),
      Type(وضعیت.type()), Perms(وضعیت.permissions()) {}

وضعیت::وضعیت(ثابت Twine &Name, UniqueID UID, sys::زمان_نقطه<> MTime,
               uint32_t User, uint32_t گروه, uint64_t اندازه, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), گروه(گروه),
      اندازه(اندازه), Type(Type), Perms(Perms) {}

وضعیت وضعیت::copy_With_New_اندازه(ثابت وضعیت &درون_2, uint64_t New_اندازه) {
  أرجع وضعیت(درون_2.getName(), درون_2.getUniqueID(), درون_2.get_آخرین_Modification_زمان(),
                درون_2.getUser(), درون_2.get_گروه(), New_اندازه, درون_2.getType(),
                درون_2.getPermissions());
}

وضعیت وضعیت::copyWithNewName(ثابت وضعیت &درون_2, ثابت Twine &NewName) {
  أرجع وضعیت(NewName, درون_2.getUniqueID(), درون_2.get_آخرین_Modification_زمان(),
                درون_2.getUser(), درون_2.get_گروه(), درون_2.get_اندازه(), درون_2.getType(),
                درون_2.getPermissions());
}

وضعیت وضعیت::copyWithNewName(ثابت file_وضعیت &درون_2, ثابت Twine &NewName) {
  أرجع وضعیت(NewName, درون_2.getUniqueID(), درون_2.get_آخرین_Modification_زمان(),
                درون_2.getUser(), درون_2.get_گروه(), درون_2.get_اندازه(), درون_2.type(),
                درون_2.permissions());
}

منطقي وضعیت::equivalent(ثابت وضعیت &Other) ثابت {
  أكد(is_وضعیت_معلوم() && Other.is_وضعیت_معلوم());
  أرجع getUniqueID() == Other.getUniqueID();
}

منطقي وضعیت::isDirectory() ثابت { أرجع Type == file_type::directory_file; }

منطقي وضعیت::isRegularFile() ثابت { أرجع Type == file_type::regular_file; }

منطقي وضعیت::isOther() ثابت {
  أرجع exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

منطقي وضعیت::isSymlink() ثابت { أرجع Type == file_type::symlink_file; }

منطقي وضعیت::is_وضعیت_معلوم() ثابت { أرجع Type != file_type::وضعیت_error; }

منطقي وضعیت::exists() ثابت {
  أرجع is_وضعیت_معلوم() && Type != file_type::file_not_یافت;
}

File::~File() = افتراضي;

FileSystem::~FileSystem() = افتراضي;

ErrorOr<std::unique_ptr<حافظه_میانگیر>>
FileSystem::get_میانگیر_For_File(ثابت llvm::Twine &Name, int64_t File_اندازه,
                             منطقي RequiresNullTerminator, منطقي IsVolatile,
                             منطقي Is_متن) {
  خودکار F = Is_متن ? openFileForRead(Name) : open_File_For_Read_دودویی(Name);
  إذا (!F)
    أرجع F.getError();

  أرجع (*F)->get_میانگیر(Name, File_اندازه, RequiresNullTerminator, IsVolatile);
}

std::error_کد FileSystem::makeAbsolute(Small_وکتور_Impl<حرف> &مسیر) ثابت {
  إذا (llvm::sys::مسیر_2::is_absolute(مسیر))
    أرجع {};

  خودکار WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  sys::مسیر_2::make_absolute(WorkingDir.get(), مسیر);
  أرجع {};
}

std::error_کد FileSystem::get_Real_مسیر(ثابت Twine &مسیر,
                                        Small_وکتور_Impl<حرف> &Output) {
  أرجع errc::operation_not_permitted;
}

std::error_کد FileSystem::isLocal(ثابت Twine &مسیر, منطقي &نتیجه) {
  أرجع errc::operation_not_permitted;
}

منطقي FileSystem::exists(ثابت Twine &مسیر) {
  خودکار وضعیت = وضعیت_2(مسیر);
  أرجع وضعیت && وضعیت->exists();
}

llvm::ErrorOr<منطقي> FileSystem::equivalent(ثابت Twine &A, ثابت Twine &B) {
  خودکار وضعیت_A = وضعیت_2(A);
  إذا (!وضعیت_A)
    أرجع وضعیت_A.getError();
  خودکار وضعیت_B = وضعیت_2(B);
  إذا (!وضعیت_B)
    أرجع وضعیت_B.getError();
  أرجع وضعیت_A->equivalent(*وضعیت_B);
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
فراغ FileSystem::dump() ثابت { print(dbgs(), PrintType::RecursiveContents); }
#پایان_اگر

#اگر_تعریف_نشده NDEBUG
ساكن منطقي isTraversalComponent(StringRef Component) {
  أرجع Component == ".." || Component == ".";
}

ساكن منطقي مسیر_Has_Traversal(StringRef مسیر) {
  استفاده نطاق llvm::sys;

  لكل (StringRef Comp : llvm::make_range(مسیر_2::begin(مسیر), مسیر_2::انتها(مسیر)))
    إذا (isTraversalComponent(Comp))
      أرجع درست;
  أرجع خطأ;
}
#پایان_اگر

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

/// Wrapper around a raw file descriptor.
صنف RealFile : عام File {
  دوست صنف RealFileSystem;

  file_t FD;
  وضعیت S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_مسیر_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::وضعیت_error, {}),
        RealName(New_Real_مسیر_Name.str()) {
    أكد(FD.isValid() && "Invalid or inactive file descriptor");
  }

عام:
  ~RealFile() override;

  ErrorOr<وضعیت> وضعیت_2() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<حافظه_میانگیر>> get_میانگیر(ثابت Twine &Name,
                                                   int64_t File_اندازه,
                                                   منطقي RequiresNullTerminator,
                                                   منطقي IsVolatile) override;
  std::error_کد close() override;
  فراغ مجموعه_مسیر(ثابت Twine &مسیر) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<وضعیت> RealFile::وضعیت_2() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot stat closed file");
  إذا (!S.is_وضعیت_معلوم()) {
    file_وضعیت Real_وضعیت;
    إذا (std::error_کد EC = sys::fs::وضعیت_2(FD, Real_وضعیت))
      أرجع EC;
    S = وضعیت::copyWithNewName(Real_وضعیت, S.getName());
  }
  أرجع S;
}

ErrorOr<std::string> RealFile::getName() {
  أرجع RealName.خالی() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<حافظه_میانگیر>>
RealFile::get_میانگیر(ثابت Twine &Name, int64_t File_اندازه,
                    منطقي RequiresNullTerminator, منطقي IsVolatile) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot get buffer for closed file");
  أرجع حافظه_میانگیر::getOpenFile(FD, Name, File_اندازه, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_کد RealFile::close() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  std::error_کد EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  أرجع EC;
}

فراغ RealFile::مجموعه_مسیر(ثابت Twine &مسیر) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  RealName = مسیر.str();
  إذا (خودکار وضعیت = وضعیت_2())
    S = وضعیت.get().copyWithNewName(وضعیت.get(), مسیر);
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
  صریح RealFileSystem(منطقي LinkCWDToProcess) {
    إذا (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      إذا (std::error_کد EC = llvm::sys::fs::current_مسیر(PWD))
        WD = std::move(EC);
      وإلا إذا (llvm::sys::fs::real_مسیر(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      وإلا
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<وضعیت> وضعیت_2(ثابت Twine &مسیر) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ثابت Twine &مسیر) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_دودویی(ثابت Twine &مسیر) override;
  directory_iterator dir_begin(ثابت Twine &Dir, std::error_کد &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override;
  std::error_کد مجموعه_Current_Working_Directory(ثابت Twine &مسیر) override;
  std::error_کد isLocal(ثابت Twine &مسیر, منطقي &نتیجه) override;
  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_وکتور_Impl<std::string> &بیرون) override {
    بیرون.افزودن_پشت(Dir.str());
  }
  std::error_کد get_Real_مسیر(ثابت Twine &مسیر,
                              Small_وکتور_Impl<حرف> &Output) override;

محمي:
  فراغ printImpl(raw_ostream &OS, PrintType Type,
                 بیعلامت IndentLevel) ثابت override;

خاص:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_مسیر(ثابت Twine &مسیر, Small_وکتور_Impl<حرف> &Storage) ثابت {
    إذا (!WD || !*WD)
      أرجع مسیر;
    مسیر.مقصد_وکتور(Storage);
    sys::مسیر_2::make_absolute(WD->get().Resolved, Storage);
    أرجع Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ثابت Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_مسیر(Name, Storage), Flags, &RealName);
    إذا (!FDOrErr)
      أرجع error_مقصد_Error_کد(FDOrErr.takeError());
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

ErrorOr<وضعیت> RealFileSystem::وضعیت_2(ثابت Twine &مسیر) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_وضعیت Real_وضعیت;
  إذا (std::error_کد EC =
          sys::fs::وضعیت_2(adjust_مسیر(مسیر, Storage), Real_وضعیت))
    أرجع EC;
  أرجع وضعیت::copyWithNewName(Real_وضعیت, مسیر);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ثابت Twine &Name) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::از_متن);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_دودویی(ثابت Twine &Name) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::از_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ثابت {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (WD && *WD)
    أرجع std::string(WD->get().Specified);
  إذا (WD)
    أرجع WD->getError();

  SmallString<128> Dir;
  إذا (std::error_کد EC = llvm::sys::fs::current_مسیر(Dir))
    أرجع EC;
  أرجع std::string(Dir);
}

std::error_کد RealFileSystem::مجموعه_Current_Working_Directory(ثابت Twine &مسیر) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (!WD)
    أرجع llvm::sys::fs::مجموعه_current_مسیر(مسیر);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_مسیر(مسیر, Storage).مقصد_وکتور(Absolute);
  منطقي IsDir;
  إذا (خودکار Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    أرجع Err;
  إذا (!IsDir)
    أرجع std::make_error_کد(std::errc::not_a_directory);
  إذا (خودکار Err = llvm::sys::fs::real_مسیر(Absolute, Resolved))
    أرجع Err;
  WD = WorkingDirectory{Absolute, Resolved};
  أرجع std::error_کد();
}

std::error_کد RealFileSystem::isLocal(ثابت Twine &مسیر, منطقي &نتیجه) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::is_local(adjust_مسیر(مسیر, Storage), نتیجه);
}

std::error_کد RealFileSystem::get_Real_مسیر(ثابت Twine &مسیر,
                                            Small_وکتور_Impl<حرف> &Output) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::real_مسیر(adjust_مسیر(مسیر, Storage), Output);
}

فراغ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               بیعلامت IndentLevel) ثابت {
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
      makeIntrusiveRefCnt<RealFileSystem>(درست);
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
  RealFSDirIter(ثابت Twine &مسیر, std::error_کد &EC) {
    خودکار BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(مسیر, EC);
    إذا (Iter != sys::fs::directory_iterator())
      Current_مدخل = directory_مدخل(Iter->مسیر_2(), Iter->type());
  }

  std::error_کد increment() override {
    خودکار BypassSandbox = sys::sandbox::scopedDisable();

    std::error_کد EC;
    Iter.increment(EC);
    Current_مدخل = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_مدخل()
                       : directory_مدخل(Iter->مسیر_2(), Iter->type());
    أرجع EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ثابت Twine &Dir,
                                             std::error_کد &EC) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  أرجع directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_مسیر(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> پایه_FS) {
  FSList.افزودن_پشت(std::move(پایه_FS));
}

فراغ OverlayFileSystem::افزودن_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.افزودن_پشت(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->مجموعه_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<وضعیت> OverlayFileSystem::وضعیت_2(ثابت Twine &مسیر) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_انتها(); I != E; ++I) {
    ErrorOr<وضعیت> وضعیت = (*I)->وضعیت_2(مسیر);
    إذا (وضعیت || وضعیت.getError() != llvm::errc::no_such_file_or_directory)
      أرجع وضعیت;
  }
  أرجع make_error_کد(llvm::errc::no_such_file_or_directory);
}

منطقي OverlayFileSystem::exists(ثابت Twine &مسیر) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_انتها(); I != E; ++I) {
    إذا ((*I)->exists(مسیر))
      أرجع درست;
  }
  أرجع خطأ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ثابت llvm::Twine &مسیر) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_انتها(); I != E; ++I) {
    خودکار نتیجه = (*I)->openFileForRead(مسیر);
    إذا (نتیجه || نتیجه.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتیجه;
  }
  أرجع make_error_کد(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ثابت {
  // All file systems are synchronized, just take the first working directory.
  أرجع FSList.front()->getCurrentWorkingDirectory();
}

std::error_کد
OverlayFileSystem::مجموعه_Current_Working_Directory(ثابت Twine &مسیر) {
  لكل (خودکار &FS : FSList)
    إذا (std::error_کد EC = FS->مجموعه_Current_Working_Directory(مسیر))
      أرجع EC;
  أرجع {};
}

std::error_کد OverlayFileSystem::isLocal(ثابت Twine &مسیر, منطقي &نتیجه) {
  لكل (خودکار &FS : FSList)
    إذا (FS->exists(مسیر))
      أرجع FS->isLocal(مسیر, نتیجه);
  أرجع errc::no_such_file_or_directory;
}

std::error_کد OverlayFileSystem::get_Real_مسیر(ثابت Twine &مسیر,
                                               Small_وکتور_Impl<حرف> &Output) {
  لكل (ثابت خودکار &FS : FSList)
    إذا (FS->exists(مسیر))
      أرجع FS->get_Real_مسیر(مسیر, Output);
  أرجع errc::no_such_file_or_directory;
}

فراغ OverlayFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_وکتور_Impl<std::string> &بیرون) {
  // All layers contribute.
  لكل (iterator I = overlays_begin(), E = overlays_انتها(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, بیرون);
}

فراغ OverlayFileSystem::بازدید_فرزند_File_Systems(بازدید_Callback_Ty Callback) {
  لكل (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->بازدید_فرزند_File_Systems(Callback);
  }
}

فراغ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  بیعلامت IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  إذا (Type == PrintType::Summary)
    أرجع;

  إذا (Type == PrintType::Contents)
    Type = PrintType::Summary;
  لكل (ثابت خودکار &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = افتراضي;

نطاق {

/// Combines and deduplicates directory entries across multiple file systems.
صنف CombiningDirIterImpl : عام llvm::vfs::detail::DirIterImpl {
  استفاده FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_وکتور<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_مجموعه<> دیده_نامها;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_کد incrementIter(منطقي Is_اولی_زمان) {
    طالما (!IterList.خالی()) {
      CurrentDirIter = IterList.پشت();
      IterList.برداشت_پشت();
      إذا (CurrentDirIter != directory_iterator())
        اكسر; //یافت
    }

    إذا (Is_اولی_زمان && CurrentDirIter == directory_iterator())
      أرجع errc::no_such_file_or_directory;
    أرجع {};
  }

  std::error_کد incrementDirIter(منطقي Is_اولی_زمان) {
    أكد((Is_اولی_زمان || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_کد EC;
    إذا (!Is_اولی_زمان)
      CurrentDirIter.increment(EC);
    إذا (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(Is_اولی_زمان);
    أرجع EC;
  }

  std::error_کد incrementImpl(منطقي Is_اولی_زمان) {
    طالما (درست) {
      std::error_کد EC = incrementDirIter(Is_اولی_زمان);
      إذا (EC || CurrentDirIter == directory_iterator()) {
        Current_مدخل = directory_مدخل();
        أرجع EC;
      }
      Current_مدخل = *CurrentDirIter;
      StringRef Name = llvm::sys::مسیر_2::filename(Current_مدخل.مسیر_2());
      إذا (دیده_نامها.درج(Name).دومی_2)
        أرجع EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

عام:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_کد &EC) {
    لكل (ثابت خودکار &FS : FileSystems) {
      std::error_کد FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      إذا (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        أرجع;
      }
      إذا (!FEC)
        IterList.افزودن_پشت(Iter);
    }
    EC = incrementImpl(درست);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_کد &EC)
      : IterList(DirIters) {
    EC = incrementImpl(درست);
  }

  std::error_کد increment() override { أرجع incrementImpl(خطأ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ثابت Twine &Dir,
                                                std::error_کد &EC) {
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

تعداد درون_حافظه_گره_نوع {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
صنف درون_حافظه_گره {
  درون_حافظه_گره_نوع نوع_3;
  std::string FileName;

عام:
  درون_حافظه_گره(llvm::StringRef FileName, درون_حافظه_گره_نوع نوع_3)
      : نوع_3(نوع_3), FileName(std::string(llvm::sys::مسیر_2::filename(FileName))) {
  }
  مجازی ~درون_حافظه_گره() = افتراضي;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  مجازی وضعیت get_وضعیت(ثابت Twine &RequestedName) ثابت = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ثابت { أرجع FileName; }
  درون_حافظه_گره_نوع get_نوع() ثابت { أرجع نوع_3; }
  مجازی std::string مقصد_String(بیعلامت Indent) ثابت = 0;
};

صنف درون_حافظه_File : عام درون_حافظه_گره {
  وضعیت Stat;
  std::unique_ptr<llvm::حافظه_میانگیر> میانگیر;

عام:
  درون_حافظه_File(وضعیت Stat, std::unique_ptr<llvm::حافظه_میانگیر> میانگیر)
      : درون_حافظه_گره(Stat.getName(), IME_File), Stat(std::move(Stat)),
        میانگیر(std::move(میانگیر)) {}

  وضعیت get_وضعیت(ثابت Twine &RequestedName) ثابت override {
    أرجع وضعیت::copyWithNewName(Stat, RequestedName);
  }
  llvm::حافظه_میانگیر *get_میانگیر() ثابت { أرجع میانگیر.get(); }

  std::string مقصد_String(بیعلامت Indent) ثابت override {
    أرجع (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ساكن منطقي classof(ثابت درون_حافظه_گره *N) {
    أرجع N->get_نوع() == IME_File;
  }
};

نطاق {

صنف درون_حافظه_Hard_Link : عام درون_حافظه_گره {
  ثابت درون_حافظه_File &ResolvedFile;

عام:
  درون_حافظه_Hard_Link(StringRef مسیر, ثابت درون_حافظه_File &ResolvedFile)
      : درون_حافظه_گره(مسیر, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ثابت درون_حافظه_File &getResolvedFile() ثابت { أرجع ResolvedFile; }

  وضعیت get_وضعیت(ثابت Twine &RequestedName) ثابت override {
    أرجع ResolvedFile.get_وضعیت(RequestedName);
  }

  std::string مقصد_String(بیعلامت Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.مقصد_String(0);
  }

  ساكن منطقي classof(ثابت درون_حافظه_گره *N) {
    أرجع N->get_نوع() == IME_HardLink;
  }
};

صنف درون_حافظه_Symbolic_Link : عام درون_حافظه_گره {
  std::string هدف_مسیر;
  وضعیت Stat;

عام:
  درون_حافظه_Symbolic_Link(StringRef مسیر, StringRef هدف_مسیر, وضعیت Stat)
      : درون_حافظه_گره(مسیر, IME_SymbolicLink), هدف_مسیر(std::move(هدف_مسیر)),
        Stat(Stat) {}

  std::string مقصد_String(بیعلامت Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "SymbolicLink to -> " + هدف_مسیر;
  }

  وضعیت get_وضعیت(ثابت Twine &RequestedName) ثابت override {
    أرجع وضعیت::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_هدف_مسیر() ثابت { أرجع هدف_مسیر; }

  ساكن منطقي classof(ثابت درون_حافظه_گره *N) {
    أرجع N->get_نوع() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
صنف درون_حافظه_File_Adaptor : عام File {
  ثابت درون_حافظه_File &گره;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

عام:
  صریح درون_حافظه_File_Adaptor(ثابت درون_حافظه_File &گره,
                               std::string RequestedName)
      : گره(گره), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<وضعیت> وضعیت_2() override {
    أرجع گره.get_وضعیت(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::حافظه_میانگیر>>
  get_میانگیر(ثابت Twine &Name, int64_t File_اندازه, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    llvm::حافظه_میانگیر *Buf = گره.get_میانگیر();
    أرجع llvm::حافظه_میانگیر::get_Mem_میانگیر(
        Buf->get_میانگیر(), Buf->get_میانگیر_Identifier(), RequiresNullTerminator);
  }

  std::error_کد close() override { أرجع {}; }

  فراغ مجموعه_مسیر(ثابت Twine &مسیر) override { RequestedName = مسیر.str(); }
};
} // namespace

صنف درون_حافظه_Directory : عام درون_حافظه_گره {
  وضعیت Stat;
  std::map<std::string, std::unique_ptr<درون_حافظه_گره>, std::less<>> Entries;

عام:
  درون_حافظه_Directory(وضعیت Stat)
      : درون_حافظه_گره(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  وضعیت get_وضعیت(ثابت Twine &RequestedName) ثابت override {
    أرجع وضعیت::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ثابت { أرجع Stat.getUniqueID(); }

  درون_حافظه_گره *get_فرزند(StringRef Name) ثابت {
    خودکار I = Entries.پیدا_کن(Name);
    إذا (I != Entries.انتها())
      أرجع I->دومی_2.get();
    أرجع اشاره_تهی;
  }

  درون_حافظه_گره *جمع_کن_فرزند(StringRef Name, std::unique_ptr<درون_حافظه_گره> فرزند) {
    أرجع Entries.emplace(Name, std::move(فرزند)).اولی_2->دومی_2.get();
  }

  استفاده const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ثابت { أرجع Entries.begin(); }
  const_iterator انتها() ثابت { أرجع Entries.انتها(); }

  std::string مقصد_String(بیعلامت Indent) ثابت override {
    std::string نتیجه =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    لكل (ثابت خودکار &مدخل : Entries)
      نتیجه += مدخل.دومی_2->مقصد_String(Indent + 2);
    أرجع نتیجه;
  }

  ساكن منطقي classof(ثابت درون_حافظه_گره *N) {
    أرجع N->get_نوع() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ساكن sys::fs::UniqueID getUniqueID(hash_کد Hash) {
  أرجع sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ساكن sys::fs::UniqueID getFileID(sys::fs::UniqueID والد_2,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  أرجع getUniqueID(llvm::hash_combine(والد_2.getFile(), Name, Contents));
}
ساكن sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID والد_2,
                                        llvm::StringRef Name) {
  أرجع getUniqueID(llvm::hash_combine(والد_2.getFile(), Name));
}

وضعیت detail::New_درون_حافظه_گره_Info::make_وضعیت() ثابت {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, میانگیر ? میانگیر->get_میانگیر() : "");

  أرجع وضعیت(مسیر, UID, llvm::sys::مقصد_زمان_نقطه(Modification_زمان), User,
                گروه, میانگیر ? میانگیر->get_میانگیر_اندازه() : 0, Type, Perms);
}

درون_حافظه_File_System::درون_حافظه_File_System(منطقي UseNormalizedPaths)
    : ریشه_2(جديد detail::درون_حافظه_Directory(
          وضعیت("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::زمان_نقطه<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

درون_حافظه_File_System::~درون_حافظه_File_System() = افتراضي;

std::string درون_حافظه_File_System::مقصد_String() ثابت {
  أرجع ریشه_2->مقصد_String(/*Indent=*/0);
}

منطقي درون_حافظه_File_System::جمع_کن_File(ثابت Twine &P, time_t Modification_زمان,
                                 std::unique_ptr<llvm::حافظه_میانگیر> میانگیر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> گروه,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_گره_Fn Make_گره) {
  SmallString<128> مسیر;
  P.مقصد_وکتور(مسیر);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کد EC = makeAbsolute(مسیر);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::مسیر_2::remove_dots(مسیر, /*remove_dot_dot=*/درست);

  إذا (مسیر.خالی())
    أرجع خطأ;

  detail::درون_حافظه_Directory *Dir = ریشه_2.get();
  خودکار I = llvm::sys::مسیر_2::begin(مسیر), E = sys::مسیر_2::انتها(مسیر);
  ثابت خودکار ResolvedUser = User.مقدار_or(0);
  ثابت خودکار Resolved_گروه = گروه.مقدار_or(0);
  ثابت خودکار ResolvedType = Type.مقدار_or(sys::fs::file_type::regular_file);
  ثابت خودکار ResolvedPerms = Perms.مقدار_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ثابت خودکار NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  طالما (درست) {
    Name = *I;
    ++I;
    إذا (I == E)
      اكسر;
    detail::درون_حافظه_گره *گره = Dir->get_فرزند(Name);
    إذا (!گره) {
      // This isn't the last element, so we create a new directory.
      وضعیت Stat(
          StringRef(مسیر.str().begin(), Name.انتها() - مسیر.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::مقصد_زمان_نقطه(Modification_زمان), ResolvedUser, Resolved_گروه,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::درون_حافظه_Directory>(Dir->جمع_کن_فرزند(
          Name, std::make_unique<detail::درون_حافظه_Directory>(std::move(Stat))));
      استمر;
    }
    // Creating file under another file.
    إذا (!isa<detail::درون_حافظه_Directory>(گره))
      أرجع خطأ;
    Dir = cast<detail::درون_حافظه_Directory>(گره);
  }
  detail::درون_حافظه_گره *گره = Dir->get_فرزند(Name);
  إذا (!گره) {
    Dir->جمع_کن_فرزند(Name,
                  Make_گره({Dir->getUniqueID(), مسیر, Name, Modification_زمان,
                            std::move(میانگیر), ResolvedUser, Resolved_گروه,
                            ResolvedType, ResolvedPerms}));
    أرجع درست;
  }
  إذا (isa<detail::درون_حافظه_Directory>(گره))
    أرجع ResolvedType == sys::fs::file_type::directory_file;

  أكد((isa<detail::درون_حافظه_File>(گره) ||
          isa<detail::درون_حافظه_Hard_Link>(گره)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  إذا (خودکار *Link = dyn_cast<detail::درون_حافظه_Hard_Link>(گره)) {
    أرجع Link->getResolvedFile().get_میانگیر()->get_میانگیر() ==
           میانگیر->get_میانگیر();
  }
  أرجع cast<detail::درون_حافظه_File>(گره)->get_میانگیر()->get_میانگیر() ==
         میانگیر->get_میانگیر();
}

منطقي درون_حافظه_File_System::جمع_کن_File(ثابت Twine &P, time_t Modification_زمان,
                                 std::unique_ptr<llvm::حافظه_میانگیر> میانگیر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> گروه,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  أرجع جمع_کن_File(P, Modification_زمان, std::move(میانگیر), User, گروه, Type,
                 Perms,
                 [](detail::New_درون_حافظه_گره_Info NNI)
                     -> std::unique_ptr<detail::درون_حافظه_گره> {
                   وضعیت Stat = NNI.make_وضعیت();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::درون_حافظه_Directory>(Stat);
                   أرجع std::make_unique<detail::درون_حافظه_File>(
                       Stat, std::move(NNI.میانگیر));
                 });
}

منطقي درون_حافظه_File_System::جمع_کن_File_No_Own(
    ثابت Twine &P, time_t Modification_زمان,
    ثابت llvm::حافظه_میانگیر_Ref &میانگیر, std::optional<uint32_t> User,
    std::optional<uint32_t> گروه, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  أرجع جمع_کن_File(P, Modification_زمان, llvm::حافظه_میانگیر::get_Mem_میانگیر(میانگیر),
                 std::move(User), std::move(گروه), std::move(Type),
                 std::move(Perms),
                 [](detail::New_درون_حافظه_گره_Info NNI)
                     -> std::unique_ptr<detail::درون_حافظه_گره> {
                   وضعیت Stat = NNI.make_وضعیت();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::درون_حافظه_Directory>(Stat);
                   أرجع std::make_unique<detail::درون_حافظه_File>(
                       Stat, std::move(NNI.میانگیر));
                 });
}

detail::Named_گره_Or_Error
درون_حافظه_File_System::lookup_گره(ثابت Twine &P, منطقي FollowFinalSymlink,
                               size_t Symlink_عمق) ثابت {
  SmallString<128> مسیر;
  P.مقصد_وکتور(مسیر);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کد EC = makeAbsolute(مسیر);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::مسیر_2::remove_dots(مسیر, /*remove_dot_dot=*/درست);

  ثابت detail::درون_حافظه_Directory *Dir = ریشه_2.get();
  إذا (مسیر.خالی())
    أرجع detail::Named_گره_Or_Error(مسیر, Dir);

  خودکار I = llvm::sys::مسیر_2::begin(مسیر), E = llvm::sys::مسیر_2::انتها(مسیر);
  طالما (درست) {
    detail::درون_حافظه_گره *گره = Dir->get_فرزند(*I);
    ++I;
    إذا (!گره)
      أرجع errc::no_such_file_or_directory;

    إذا (خودکار Symlink = dyn_cast<detail::درون_حافظه_Symbolic_Link>(گره)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      إذا (I == E && !FollowFinalSymlink)
        أرجع detail::Named_گره_Or_Error(مسیر, Symlink);

      إذا (Symlink_عمق > درون_حافظه_File_System::Max_Symlink_عمق)
        أرجع errc::no_such_file_or_directory;

      SmallString<128> هدف_مسیر = Symlink->get_هدف_مسیر();
      إذا (std::error_کد EC = makeAbsolute(هدف_مسیر))
        أرجع EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      خودکار هدف_2 =
          lookup_گره(هدف_مسیر, /*FollowFinalSymlink=*/درست, Symlink_عمق + 1);
      إذا (!هدف_2 || I == E)
        أرجع هدف_2;

      إذا (!isa<detail::درون_حافظه_Directory>(*هدف_2))
        أرجع errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::درون_حافظه_Directory>(*هدف_2);
      استمر;
    }

    // Return the file if it's at the end of the path.
    إذا (خودکار File = dyn_cast<detail::درون_حافظه_File>(گره)) {
      إذا (I == E)
        أرجع detail::Named_گره_Or_Error(مسیر, File);
      أرجع errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    إذا (خودکار File = dyn_cast<detail::درون_حافظه_Hard_Link>(گره)) {
      إذا (I == E)
        أرجع detail::Named_گره_Or_Error(مسیر, &File->getResolvedFile());
      أرجع errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::درون_حافظه_Directory>(گره);
    إذا (I == E)
      أرجع detail::Named_گره_Or_Error(مسیر, Dir);
  }
}

منطقي درون_حافظه_File_System::جمع_کن_Hard_Link(ثابت Twine &NewLink,
                                     ثابت Twine &هدف_2) {
  خودکار New_Link_گره = lookup_گره(NewLink, /*FollowFinalSymlink=*/خطأ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  خودکار هدف_گره = lookup_گره(هدف_2, /*FollowFinalSymlink=*/درست);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  إذا (!هدف_گره || New_Link_گره || !isa<detail::درون_حافظه_File>(*هدف_گره))
    أرجع خطأ;
  أرجع جمع_کن_File(NewLink, 0, اشاره_تهی, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_درون_حافظه_گره_Info NNI) {
                   أرجع std::make_unique<detail::درون_حافظه_Hard_Link>(
                       NNI.مسیر.str(),
                       *cast<detail::درون_حافظه_File>(*هدف_گره));
                 });
}

منطقي درون_حافظه_File_System::جمع_کن_Symbolic_Link(
    ثابت Twine &NewLink, ثابت Twine &هدف_2, time_t Modification_زمان,
    std::optional<uint32_t> User, std::optional<uint32_t> گروه,
    std::optional<llvm::sys::fs::perms> Perms) {
  خودکار New_Link_گره = lookup_گره(NewLink, /*FollowFinalSymlink=*/خطأ);
  إذا (New_Link_گره)
    أرجع خطأ;

  SmallString<128> NewLinkStr, هدف_Str;
  NewLink.مقصد_وکتور(NewLinkStr);
  هدف_2.مقصد_وکتور(هدف_Str);

  أرجع جمع_کن_File(NewLinkStr, Modification_زمان, اشاره_تهی, User, گروه,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_درون_حافظه_گره_Info NNI) {
                   أرجع std::make_unique<detail::درون_حافظه_Symbolic_Link>(
                       NewLinkStr, هدف_Str, NNI.make_وضعیت());
                 });
}

llvm::ErrorOr<وضعیت> درون_حافظه_File_System::وضعیت_2(ثابت Twine &مسیر) {
  خودکار گره = lookup_گره(مسیر, /*FollowFinalSymlink=*/درست);
  إذا (گره)
    أرجع (*گره)->get_وضعیت(مسیر);
  أرجع گره.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
درون_حافظه_File_System::openFileForRead(ثابت Twine &مسیر) {
  خودکار گره = lookup_گره(مسیر,/*FollowFinalSymlink=*/درست);
  إذا (!گره)
    أرجع گره.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  إذا (خودکار *F = dyn_cast<detail::درون_حافظه_File>(*گره))
    أرجع std::unique_ptr<File>(
        جديد detail::درون_حافظه_File_Adaptor(*F, مسیر.str()));

  // FIXME: errc::not_a_file?
  أرجع make_error_کد(llvm::errc::invalid_آرگومان);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
صنف درون_حافظه_File_System::DirIterator : عام llvm::vfs::detail::DirIterImpl {
  ثابت درون_حافظه_File_System *FS;
  detail::درون_حافظه_Directory::const_iterator I;
  detail::درون_حافظه_Directory::const_iterator E;
  std::string RequestedDirName;

  فراغ مجموعه_Current_مدخل() {
    إذا (I != E) {
      SmallString<256> مسیر(RequestedDirName);
      llvm::sys::مسیر_2::append(مسیر, I->دومی_2->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل (I->دومی_2->get_نوع()) {
      حالة detail::IME_File:
      حالة detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      حالة detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة detail::IME_SymbolicLink:
        إذا (خودکار Symlink_هدف =
                FS->lookup_گره(مسیر, /*FollowFinalSymlink=*/درست)) {
          مسیر = Symlink_هدف.getName();
          Type = (*Symlink_هدف)->get_وضعیت(مسیر).getType();
        }
        اكسر;
      }
      Current_مدخل = directory_مدخل(std::string(مسیر), Type);
    } وإلا {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_مدخل = directory_مدخل();
    }
  }

عام:
  DirIterator() = افتراضي;

  DirIterator(ثابت درون_حافظه_File_System *FS,
              ثابت detail::درون_حافظه_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.انتها()),
        RequestedDirName(std::move(RequestedDirName)) {
    مجموعه_Current_مدخل();
  }

  std::error_کد increment() override {
    ++I;
    مجموعه_Current_مدخل();
    أرجع {};
  }
};

directory_iterator درون_حافظه_File_System::dir_begin(ثابت Twine &Dir,
                                                 std::error_کد &EC) {
  خودکار گره = lookup_گره(Dir, /*FollowFinalSymlink=*/درست);
  إذا (!گره) {
    EC = گره.getError();
    أرجع directory_iterator(std::make_shared<DirIterator>());
  }

  إذا (خودکار *Dir_گره = dyn_cast<detail::درون_حافظه_Directory>(*گره))
    أرجع directory_iterator(
        std::make_shared<DirIterator>(هذا, *Dir_گره, Dir.str()));

  EC = make_error_کد(llvm::errc::not_a_directory);
  أرجع directory_iterator(std::make_shared<DirIterator>());
}

std::error_کد درون_حافظه_File_System::مجموعه_Current_Working_Directory(ثابت Twine &P) {
  SmallString<128> مسیر;
  P.مقصد_وکتور(مسیر);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کد EC = makeAbsolute(مسیر);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::مسیر_2::remove_dots(مسیر, /*remove_dot_dot=*/درست);

  إذا (!مسیر.خالی())
    WorkingDirectory = std::string(مسیر);
  أرجع {};
}

std::error_کد درون_حافظه_File_System::get_Real_مسیر(ثابت Twine &مسیر,
                                                Small_وکتور_Impl<حرف> &Output) {
  خودکار CWD = getCurrentWorkingDirectory();
  إذا (!CWD || CWD->خالی())
    أرجع errc::operation_not_permitted;
  مسیر.مقصد_وکتور(Output);
  إذا (خودکار EC = makeAbsolute(Output))
    أرجع EC;
  llvm::sys::مسیر_2::remove_dots(Output, /*remove_dot_dot=*/درست);
  أرجع {};
}

std::error_کد درون_حافظه_File_System::isLocal(ثابت Twine &مسیر, منطقي &نتیجه) {
  نتیجه = خطأ;
  أرجع {};
}

فراغ درون_حافظه_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   بیعلامت IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

ساكن llvm::sys::مسیر_2::Style getExistingStyle(llvm::StringRef مسیر) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::مسیر_2::Style style = llvm::sys::مسیر_2::Style::native;
  ثابت size_t n = مسیر.پیدا_کن_اولی_از("/\\");
  // Can't distinguish between posix and windows_slash here.
  إذا (n != static_cast<size_t>(-1))
    style = (مسیر[n] == '/') ? llvm::sys::مسیر_2::Style::posix
                             : llvm::sys::مسیر_2::Style::windows_backslash;
  أرجع style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ساكن llvm::SmallString<256> canonicalize(llvm::StringRef مسیر) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::مسیر_2::Style style = getExistingStyle(مسیر);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> نتیجه_2 =
      llvm::sys::مسیر_2::remove_leading_dotslash(مسیر, style);
  llvm::sys::مسیر_2::remove_dots(نتیجه_2, /*remove_dot_dot=*/درست, style);
  أرجع نتیجه_2;
}

/// Whether the error and entry specify a file/directory that was not found.
ساكن منطقي is_File_Not_یافت(std::error_کد EC,
                           RedirectingFileSystem::مدخل *E = اشاره_تهی) {
  إذا (E && !isa<RedirectingFileSystem::Directory_Remap_مدخل>(E))
    أرجع خطأ;
  أرجع EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  أكد(ExternalFS && "RedirectingFileSystem requires an external FS");
  إذا (خودکار ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
صنف llvm::vfs::RedirectingFSDirIterImpl
    : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::Directory_مدخل::iterator Current, انتها_2;

  std::error_کد incrementImpl(منطقي Is_اولی_زمان) {
    أكد((Is_اولی_زمان || Current != انتها_2) && "cannot iterate past end");
    إذا (!Is_اولی_زمان)
      ++Current;
    إذا (Current != انتها_2) {
      SmallString<128> مسیر_Str(Dir);
      llvm::sys::مسیر_2::append(مسیر_Str, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل ((*Current)->get_نوع()) {
      حالة RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      حالة RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      }
      Current_مدخل = directory_مدخل(std::string(مسیر_Str), Type);
    } وإلا {
      Current_مدخل = directory_مدخل();
    }
    أرجع {};
  };

عام:
  RedirectingFSDirIterImpl(
      ثابت Twine &مسیر, RedirectingFileSystem::Directory_مدخل::iterator Begin,
      RedirectingFileSystem::Directory_مدخل::iterator انتها_2, std::error_کد &EC)
      : Dir(مسیر.str()), Current(Begin), انتها_2(انتها_2) {
    EC = incrementImpl(/*IsFirstTime=*/درست);
  }

  std::error_کد increment() override {
    أرجع incrementImpl(/*IsFirstTime=*/خطأ);
  }
};

نطاق {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
صنف RedirectingFSDirRemapIterImpl : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::مسیر_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

عام:
  RedirectingFSDirRemapIterImpl(std::string Dir_مسیر,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_مسیر)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    إذا (ExternalIter != llvm::vfs::directory_iterator())
      مجموعه_Current_مدخل();
  }

  فراغ مجموعه_Current_مدخل() {
    StringRef External_مسیر = ExternalIter->مسیر_2();
    llvm::sys::مسیر_2::Style ExternalStyle = getExistingStyle(External_مسیر);
    StringRef File = llvm::sys::مسیر_2::filename(External_مسیر, ExternalStyle);

    SmallString<128> New_مسیر(Dir);
    llvm::sys::مسیر_2::append(New_مسیر, DirStyle, File);

    Current_مدخل = directory_مدخل(std::string(New_مسیر), ExternalIter->type());
  }

  std::error_کد increment() override {
    std::error_کد EC;
    ExternalIter.increment(EC);
    إذا (!EC && ExternalIter != llvm::vfs::directory_iterator())
      مجموعه_Current_مدخل();
    وإلا
      Current_مدخل = directory_مدخل();
    أرجع EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ثابت {
  أرجع WorkingDirectory;
}

std::error_کد
RedirectingFileSystem::مجموعه_Current_Working_Directory(ثابت Twine &مسیر) {
  // Don't change the working directory if the path doesn't exist.
  إذا (!exists(مسیر))
    أرجع errc::no_such_file_or_directory;

  SmallString<128> Absolute_مسیر;
  مسیر.مقصد_وکتور(Absolute_مسیر);
  إذا (std::error_کد EC = makeAbsolute(Absolute_مسیر))
    أرجع EC;
  WorkingDirectory = std::string(Absolute_مسیر);
  أرجع {};
}

std::error_کد RedirectingFileSystem::isLocal(ثابت Twine &مسیر_3,
                                               منطقي &نتیجه) {
  SmallString<256> مسیر;
  مسیر_3.مقصد_وکتور(مسیر);

  إذا (makeAbsolute(مسیر))
    أرجع {};

  أرجع ExternalFS->isLocal(مسیر, نتیجه);
}

std::error_کد RedirectingFileSystem::makeAbsolute(Small_وکتور_Impl<حرف> &مسیر) ثابت {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  إذا (llvm::sys::مسیر_2::is_absolute(مسیر, llvm::sys::مسیر_2::Style::posix) ||
      llvm::sys::مسیر_2::is_absolute(مسیر,
                                   llvm::sys::مسیر_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    أرجع {};

  خودکار WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  أرجع makeAbsolute(WorkingDir.get(), مسیر);
}

std::error_کد
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_وکتور_Impl<حرف> &مسیر) ثابت {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  إذا (!WorkingDir.خالی() &&
      !sys::مسیر_2::is_absolute(WorkingDir, sys::مسیر_2::Style::posix) &&
      !sys::مسیر_2::is_absolute(WorkingDir,
                              sys::مسیر_2::Style::windows_backslash)) {
    أرجع std::error_کد();
  }
  sys::مسیر_2::Style style = sys::مسیر_2::Style::windows_backslash;
  إذا (sys::مسیر_2::is_absolute(WorkingDir, sys::مسیر_2::Style::posix)) {
    style = sys::مسیر_2::Style::posix;
  } وإلا {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    إذا (getExistingStyle(WorkingDir) != sys::مسیر_2::Style::windows_backslash)
      style = sys::مسیر_2::Style::windows_slash;
  }

  std::string نتیجه = std::string(WorkingDir);
  StringRef Dir(نتیجه);
  إذا (!Dir.دوسر_with(sys::مسیر_2::get_separator(style))) {
    نتیجه += sys::مسیر_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  نتیجه.append(مسیر.داده(), مسیر.اندازه_2());
  مسیر.تخصیص_بده(نتیجه.begin(), نتیجه.انتها());

  أرجع {};
}

directory_iterator RedirectingFileSystem::dir_begin(ثابت Twine &Dir,
                                                    std::error_کد &EC) {
  SmallString<256> مسیر;
  Dir.مقصد_وکتور(مسیر);

  EC = makeAbsolute(مسیر);
  إذا (EC)
    أرجع {};

  ErrorOr<RedirectingFileSystem::Lookup_نتیجه> نتیجه = lookup_مسیر(مسیر);
  إذا (!نتیجه) {
    إذا (Redirection != Redirect_نوع::RedirectOnly &&
        is_File_Not_یافت(نتیجه.getError()))
      أرجع ExternalFS->dir_begin(مسیر, EC);

    EC = نتیجه.getError();
    أرجع {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<وضعیت> S = وضعیت_2(مسیر, Dir, *نتیجه);
  إذا (!S) {
    إذا (Redirection != Redirect_نوع::RedirectOnly &&
        is_File_Not_یافت(S.getError(), نتیجه->E))
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
  std::error_کد RedirectEC;
  إذا (خودکار ExtRedirect = نتیجه->getExternalRedirect()) {
    خودکار RE = cast<RedirectingFileSystem::Remap_مدخل>(نتیجه->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    إذا (!RE->useExternalName(Use_External_نامها)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(مسیر), RedirectIter));
    }
  } وإلا {
    خودکار DE = cast<Directory_مدخل>(نتیجه->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            مسیر, DE->contents_begin(), DE->contents_انتها(), RedirectEC));
  }

  إذا (RedirectEC) {
    إذا (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      أرجع {};
    }
    RedirectIter = {};
  }

  إذا (Redirection == Redirect_نوع::RedirectOnly) {
    EC = RedirectEC;
    أرجع RedirectIter;
  }

  std::error_کد ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(مسیر, ExternalEC);
  إذا (ExternalEC) {
    إذا (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      أرجع {};
    }
    ExternalIter = {};
  }

  Small_وکتور<directory_iterator, 2> Iters;
  بدل (Redirection) {
  حالة Redirect_نوع::Fallthrough:
    Iters.افزودن_پشت(ExternalIter);
    Iters.افزودن_پشت(RedirectIter);
    اكسر;
  حالة Redirect_نوع::Fallback:
    Iters.افزودن_پشت(RedirectIter);
    Iters.افزودن_پشت(ExternalIter);
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

فراغ RedirectingFileSystem::مجموعه_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ثابت {
  أرجع OverlayFileDir;
}

فراغ RedirectingFileSystem::مجموعه_Fallthrough(منطقي Fallthrough) {
  إذا (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_نوع::Fallthrough;
  } وإلا {
    Redirection = RedirectingFileSystem::Redirect_نوع::RedirectOnly;
  }
}

فراغ RedirectingFileSystem::مجموعه_Redirection(
    RedirectingFileSystem::Redirect_نوع نوع_3) {
  Redirection = نوع_3;
}

std::وکتور<StringRef> RedirectingFileSystem::getRoots() ثابت {
  std::وکتور<StringRef> R;
  R.reserve(Roots.اندازه_2());
  لكل (ثابت خودکار &ریشه_2 : Roots)
    R.افزودن_پشت(ریشه_2->getName());
  أرجع R;
}

فراغ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      بیعلامت IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_نامها ? "true" : "false") << ")\n";
  إذا (Type == PrintType::Summary)
    أرجع;

  لكل (ثابت خودکار &ریشه_2 : Roots)
    print_مدخل(OS, ریشه_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

فراغ RedirectingFileSystem::print_مدخل(raw_ostream &OS,
                                       RedirectingFileSystem::مدخل *E,
                                       بیعلامت IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  بدل (E->get_نوع()) {
  حالة EK_Directory: {
    خودکار *DE = cast<RedirectingFileSystem::Directory_مدخل>(E);

    OS << "\n";
    لكل (std::unique_ptr<مدخل> &Sub_مدخل :
         llvm::make_range(DE->contents_begin(), DE->contents_انتها()))
      print_مدخل(OS, Sub_مدخل.get(), IndentLevel + 1);
    اكسر;
  }
  حالة EK_DirectoryRemap:
  حالة EK_File: {
    خودکار *RE = cast<RedirectingFileSystem::Remap_مدخل>(E);
    OS << " -> '" << RE->get_External_Contents_مسیر() << "'";
    بدل (RE->getUseName()) {
    حالة NK_Not_مجموعه:
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

فراغ RedirectingFileSystem::بازدید_فرزند_File_Systems(بازدید_Callback_Ty Callback) {
  إذا (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->بازدید_فرزند_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
صنف llvm::vfs::RedirectingFileSystemParser {
  yaml::جریان &جریان;

  فراغ error(yaml::گره *N, ثابت Twine &پیام_4) { جریان.printError(N, پیام_4); }

  // false on error
  منطقي parseScalarString(yaml::گره *N, StringRef &نتیجه,
                         Small_وکتور_Impl<حرف> &Storage) {
    ثابت خودکار *S = dyn_cast_if_present<yaml::Scalar_گره>(N);

    إذا (!S) {
      error(N, "expected string");
      أرجع خطأ;
    }
    نتیجه = S->get_مقدار(Storage);
    أرجع درست;
  }

  // false on error
  منطقي parseScalarBool(yaml::گره *N, منطقي &نتیجه) {
    SmallString<5> Storage;
    StringRef مقدار;
    إذا (!parseScalarString(N, مقدار, Storage))
      أرجع خطأ;

    إذا (مقدار.equals_insensitive("true") || مقدار.equals_insensitive("on") ||
        مقدار.equals_insensitive("yes") || مقدار == "1") {
      نتیجه = درست;
      أرجع درست;
    } وإلا إذا (مقدار.equals_insensitive("false") ||
               مقدار.equals_insensitive("off") ||
               مقدار.equals_insensitive("no") || مقدار == "0") {
      نتیجه = خطأ;
      أرجع درست;
    }

    error(N, "expected boolean value");
    أرجع خطأ;
  }

  std::optional<RedirectingFileSystem::Redirect_نوع>
  parse_Redirect_نوع(yaml::گره *N) {
    SmallString<12> Storage;
    StringRef مقدار;
    إذا (!parseScalarString(N, مقدار, Storage))
      أرجع std::nullopt;

    إذا (مقدار.equals_insensitive("fallthrough")) {
      أرجع RedirectingFileSystem::Redirect_نوع::Fallthrough;
    } وإلا إذا (مقدار.equals_insensitive("fallback")) {
      أرجع RedirectingFileSystem::Redirect_نوع::Fallback;
    } وإلا إذا (مقدار.equals_insensitive("redirect-only")) {
      أرجع RedirectingFileSystem::Redirect_نوع::RedirectOnly;
    }
    أرجع std::nullopt;
  }

  std::optional<RedirectingFileSystem::ریشه_Relative_نوع>
  parse_ریشه_Relative_نوع(yaml::گره *N) {
    SmallString<12> Storage;
    StringRef مقدار;
    إذا (!parseScalarString(N, مقدار, Storage))
      أرجع std::nullopt;
    إذا (مقدار.equals_insensitive("cwd")) {
      أرجع RedirectingFileSystem::ریشه_Relative_نوع::CWD;
    } وإلا إذا (مقدار.equals_insensitive("overlay-dir")) {
      أرجع RedirectingFileSystem::ریشه_Relative_نوع::OverlayDir;
    }
    أرجع std::nullopt;
  }

  بنية کلید_وضعیت {
    منطقي Required;
    منطقي دیده = خطأ;

    کلید_وضعیت(منطقي Required = خطأ) : Required(Required) {}
  };

  استفاده کلید_وضعیت_Pair = std::pair<StringRef, کلید_وضعیت>;

  // false on error
  منطقي check_Duplicate_Or_Unknown_کلید(yaml::گره *کلید_گره, StringRef کلید,
                                  DenseMap<StringRef, کلید_وضعیت> &کلیدها_2) {
    خودکار It = کلیدها_2.پیدا_کن(کلید);
    إذا (It == کلیدها_2.انتها()) {
      error(کلید_گره, "unknown key");
      أرجع خطأ;
    }
    کلید_وضعیت &S = It->دومی_2;
    إذا (S.دیده) {
      error(کلید_گره, Twine("duplicate key '") + کلید + "'");
      أرجع خطأ;
    }
    S.دیده = درست;
    أرجع درست;
  }

  // false on error
  منطقي check_Missing_کلیدها(yaml::گره *Obj, DenseMap<StringRef, کلید_وضعیت> &کلیدها_2) {
    لكل (ثابت خودکار &I : کلیدها_2) {
      إذا (I.دومی_2.Required && !I.دومی_2.دیده) {
        error(Obj, Twine("missing key '") + I.اولی_2 + "'");
        أرجع خطأ;
      }
    }
    أرجع درست;
  }

عام:
  ساكن RedirectingFileSystem::مدخل *
  lookup_Or_Create_مدخل(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::مدخل *والد_مدخل = اشاره_تهی) {
    إذا (!والد_مدخل) { // Look for a existent root
      لكل (ثابت خودکار &ریشه_2 : FS->Roots) {
        إذا (Name == ریشه_2->getName()) {
          والد_مدخل = ریشه_2.get();
          أرجع والد_مدخل;
        }
      }
    } وإلا { // Advance to the next component
      خودکار *DE = dyn_cast<RedirectingFileSystem::Directory_مدخل>(والد_مدخل);
      لكل (std::unique_ptr<RedirectingFileSystem::مدخل> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_انتها())) {
        خودکار *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_مدخل>(Content.get());
        إذا (DirContent && Name == Content->getName())
          أرجع DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::مدخل> E =
        std::make_unique<RedirectingFileSystem::Directory_مدخل>(
            Name, وضعیت("", get_بعدی_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    إذا (!والد_مدخل) { // Add a new root to the overlay
      FS->Roots.افزودن_پشت(std::move(E));
      والد_مدخل = FS->Roots.پشت().get();
      أرجع والد_مدخل;
    }

    خودکار *DE = cast<RedirectingFileSystem::Directory_مدخل>(والد_مدخل);
    DE->جمع_کن_Content(std::move(E));
    أرجع DE->get_آخرین_Content();
  }

خاص:
  فراغ unique_Overlay_درخت(RedirectingFileSystem *FS,
                         RedirectingFileSystem::مدخل *SrcE,
                         RedirectingFileSystem::مدخل *New_والد_E = اشاره_تهی) {
    StringRef Name = SrcE->getName();
    بدل (SrcE->get_نوع()) {
    حالة RedirectingFileSystem::EK_Directory: {
      خودکار *DE = cast<RedirectingFileSystem::Directory_مدخل>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      إذا (!Name.خالی())
        New_والد_E = lookup_Or_Create_مدخل(FS, Name, New_والد_E);
      لكل (std::unique_ptr<RedirectingFileSystem::مدخل> &Sub_مدخل :
           llvm::make_range(DE->contents_begin(), DE->contents_انتها()))
        unique_Overlay_درخت(FS, Sub_مدخل.get(), New_والد_E);
      اكسر;
    }
    حالة RedirectingFileSystem::EK_DirectoryRemap: {
      أكد(New_والد_E && "Parent entry must exist");
      خودکار *DR = cast<RedirectingFileSystem::Directory_Remap_مدخل>(SrcE);
      خودکار *DE = cast<RedirectingFileSystem::Directory_مدخل>(New_والد_E);
      DE->جمع_کن_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_مدخل>(
              Name, DR->get_External_Contents_مسیر(), DR->getUseName()));
      اكسر;
    }
    حالة RedirectingFileSystem::EK_File: {
      أكد(New_والد_E && "Parent entry must exist");
      خودکار *FE = cast<RedirectingFileSystem::File_مدخل>(SrcE);
      خودکار *DE = cast<RedirectingFileSystem::Directory_مدخل>(New_والد_E);
      DE->جمع_کن_Content(std::make_unique<RedirectingFileSystem::File_مدخل>(
          Name, FE->get_External_Contents_مسیر(), FE->getUseName()));
      اكسر;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::مدخل>
  parse_مدخل(yaml::گره *N, RedirectingFileSystem *FS, منطقي Is_ریشه_مدخل) {
    خودکار *M = dyn_cast<yaml::Mapping_گره>(N);
    إذا (!M) {
      error(N, "expected mapping node for file or directory entry");
      أرجع اشاره_تهی;
    }

    کلید_وضعیت_Pair Fields[] = {
        کلید_وضعیت_Pair("name", درست),
        کلید_وضعیت_Pair("type", درست),
        کلید_وضعیت_Pair("contents", خطأ),
        کلید_وضعیت_Pair("external-contents", خطأ),
        کلید_وضعیت_Pair("use-external-name", خطأ),
    };

    DenseMap<StringRef, کلید_وضعیت> کلیدها_2(std::begin(Fields), std::انتها(Fields));

    تعداد { CF_Not_مجموعه, CF_List, CF_External } Contents_فیلد = CF_Not_مجموعه;
    std::وکتور<std::unique_ptr<RedirectingFileSystem::مدخل>>
        مدخل_Array_Contents;
    SmallString<256> External_Contents_مسیر;
    SmallString<256> Name;
    yaml::گره *Name_مقدار_گره = اشاره_تهی;
    خودکار UseExternalName = RedirectingFileSystem::NK_Not_مجموعه;
    RedirectingFileSystem::مدخل_نوع نوع_3;

    لكل (خودکار &I : *M) {
      StringRef کلید;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> میانگیر;
      إذا (!parseScalarString(I.get_کلید(), کلید, میانگیر))
        أرجع اشاره_تهی;

      إذا (!check_Duplicate_Or_Unknown_کلید(I.get_کلید(), کلید, کلیدها_2))
        أرجع اشاره_تهی;

      StringRef مقدار;
      إذا (کلید == "name") {
        إذا (!parseScalarString(I.get_مقدار(), مقدار, میانگیر))
          أرجع اشاره_تهی;

        Name_مقدار_گره = I.get_مقدار();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(مقدار).str();
      } وإلا إذا (کلید == "type") {
        إذا (!parseScalarString(I.get_مقدار(), مقدار, میانگیر))
          أرجع اشاره_تهی;
        إذا (مقدار == "file")
          نوع_3 = RedirectingFileSystem::EK_File;
        وإلا إذا (مقدار == "directory")
          نوع_3 = RedirectingFileSystem::EK_Directory;
        وإلا إذا (مقدار == "directory-remap")
          نوع_3 = RedirectingFileSystem::EK_DirectoryRemap;
        وإلا {
          error(I.get_مقدار(), "unknown value for 'type'");
          أرجع اشاره_تهی;
        }
      } وإلا إذا (کلید == "contents") {
        إذا (Contents_فیلد != CF_Not_مجموعه) {
          error(I.get_کلید(),
                "entry already has 'contents' or 'external-contents'");
          أرجع اشاره_تهی;
        }
        Contents_فیلد = CF_List;
        خودکار *Contents = dyn_cast_if_present<yaml::Sequence_گره>(I.get_مقدار());
        إذا (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_مقدار(), "expected array");
          أرجع اشاره_تهی;
        }

        لكل (خودکار &I : *Contents) {
          إذا (std::unique_ptr<RedirectingFileSystem::مدخل> E =
                  parse_مدخل(&I, FS, /*IsRootEntry*/ خطأ))
            مدخل_Array_Contents.افزودن_پشت(std::move(E));
          وإلا
            أرجع اشاره_تهی;
        }
      } وإلا إذا (کلید == "external-contents") {
        إذا (Contents_فیلد != CF_Not_مجموعه) {
          error(I.get_کلید(),
                "entry already has 'contents' or 'external-contents'");
          أرجع اشاره_تهی;
        }
        Contents_فیلد = CF_External;
        إذا (!parseScalarString(I.get_مقدار(), مقدار, میانگیر))
          أرجع اشاره_تهی;

        SmallString<256> پر_مسیر;
        إذا (FS->IsRelativeOverlay) {
          پر_مسیر = FS->getOverlayFileDir();
          أكد(!پر_مسیر.خالی() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_پر_مسیر = مقدار;
          إذا (FS->makeAbsolute(پر_مسیر, Abs_پر_مسیر)) {
            error(N, "failed to make 'external-contents' absolute");
            أرجع اشاره_تهی;
          }
          پر_مسیر = Abs_پر_مسیر;
        } وإلا {
          پر_مسیر = مقدار;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        پر_مسیر = canonicalize(پر_مسیر);
        External_Contents_مسیر = پر_مسیر.str();
      } وإلا إذا (کلید == "use-external-name") {
        منطقي Val;
        إذا (!parseScalarBool(I.get_مقدار(), Val))
          أرجع اشاره_تهی;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (جریان.failed())
      أرجع اشاره_تهی;

    // check for missing keys
    إذا (Contents_فیلد == CF_Not_مجموعه) {
      error(N, "missing key 'contents' or 'external-contents'");
      أرجع اشاره_تهی;
    }
    إذا (!check_Missing_کلیدها(N, کلیدها_2))
      أرجع اشاره_تهی;

    // check invalid configuration
    إذا (نوع_3 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_مجموعه) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      أرجع اشاره_تهی;
    }

    إذا (نوع_3 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_فیلد == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      أرجع اشاره_تهی;
    }

    sys::مسیر_2::Style مسیر_style = sys::مسیر_2::Style::native;
    إذا (Is_ریشه_مدخل) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      إذا (sys::مسیر_2::is_absolute(Name, sys::مسیر_2::Style::posix)) {
        مسیر_style = sys::مسیر_2::Style::posix;
      } وإلا إذا (sys::مسیر_2::is_absolute(Name,
                                        sys::مسیر_2::Style::windows_backslash)) {
        مسیر_style = sys::مسیر_2::Style::windows_backslash;
      } وإلا {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_کد EC;
        إذا (FS->ریشه_Relative ==
            RedirectingFileSystem::ریشه_Relative_نوع::OverlayDir) {
          StringRef پر_مسیر = FS->getOverlayFileDir();
          أكد(!پر_مسیر.خالی() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(پر_مسیر, Name);
          Name = canonicalize(Name);
        } وإلا {
          EC = FS->makeAbsolute(Name);
        }
        إذا (EC) {
          أكد(Name_مقدار_گره && "Name presence should be checked earlier");
          error(
              Name_مقدار_گره,
              "entry with relative path at the root level is not discoverable");
          أرجع اشاره_تهی;
        }
        مسیر_style = sys::مسیر_2::is_absolute(Name, sys::مسیر_2::Style::posix)
                         ? sys::مسیر_2::Style::posix
                         : sys::مسیر_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      إذا (مسیر_style == sys::مسیر_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::مسیر_2::Style::windows_backslash)
        مسیر_style = sys::مسیر_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t ریشه_مسیر_Len = sys::مسیر_2::ریشه_مسیر_2(Trimmed, مسیر_style).اندازه_2();
    طالما (Trimmed.اندازه_2() > ریشه_مسیر_Len &&
           sys::مسیر_2::is_separator(Trimmed.پشت(), مسیر_style))
      Trimmed = Trimmed.برش(0, Trimmed.اندازه_2() - 1);

    // Get the last component
    StringRef آخرین_Component = sys::مسیر_2::filename(Trimmed, مسیر_style);

    std::unique_ptr<RedirectingFileSystem::مدخل> نتیجه;
    بدل (نوع_3) {
    حالة RedirectingFileSystem::EK_File:
      نتیجه = std::make_unique<RedirectingFileSystem::File_مدخل>(
          آخرین_Component, std::move(External_Contents_مسیر), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_DirectoryRemap:
      نتیجه = std::make_unique<RedirectingFileSystem::Directory_Remap_مدخل>(
          آخرین_Component, std::move(External_Contents_مسیر), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_Directory:
      نتیجه = std::make_unique<RedirectingFileSystem::Directory_مدخل>(
          آخرین_Component, std::move(مدخل_Array_Contents),
          وضعیت("", get_بعدی_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      اكسر;
    }

    StringRef والد_2 = sys::مسیر_2::والد_مسیر(Trimmed, مسیر_style);
    إذا (والد_2.خالی())
      أرجع نتیجه;

    // if 'name' contains multiple components, create implicit directory entries
    لكل (sys::مسیر_2::معکوس_iterator I = sys::مسیر_2::rbegin(والد_2, مسیر_style),
                                     E = sys::مسیر_2::rend(والد_2);
         I != E; ++I) {
      std::وکتور<std::unique_ptr<RedirectingFileSystem::مدخل>> Entries;
      Entries.افزودن_پشت(std::move(نتیجه));
      نتیجه = std::make_unique<RedirectingFileSystem::Directory_مدخل>(
          *I, std::move(Entries),
          وضعیت("", get_بعدی_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    أرجع نتیجه;
  }

عام:
  RedirectingFileSystemParser(yaml::جریان &S) : جریان(S) {}

  // false on error
  منطقي parse(yaml::گره *ریشه_2, RedirectingFileSystem *FS) {
    خودکار *بالاترین = dyn_cast<yaml::Mapping_گره>(ریشه_2);
    إذا (!بالاترین) {
      error(ریشه_2, "expected mapping node");
      أرجع خطأ;
    }

    کلید_وضعیت_Pair Fields[] = {
        کلید_وضعیت_Pair("version", درست),
        کلید_وضعیت_Pair("case-sensitive", خطأ),
        کلید_وضعیت_Pair("use-external-names", خطأ),
        کلید_وضعیت_Pair("root-relative", خطأ),
        کلید_وضعیت_Pair("overlay-relative", خطأ),
        کلید_وضعیت_Pair("fallthrough", خطأ),
        کلید_وضعیت_Pair("redirecting-with", خطأ),
        کلید_وضعیت_Pair("roots", درست),
    };

    DenseMap<StringRef, کلید_وضعیت> کلیدها_2(std::begin(Fields), std::انتها(Fields));
    std::وکتور<std::unique_ptr<RedirectingFileSystem::مدخل>> ریشه_Entries;

    // Parse configuration and 'roots'
    لكل (خودکار &I : *بالاترین) {
      SmallString<10> کلید_میانگیر;
      StringRef کلید;
      إذا (!parseScalarString(I.get_کلید(), کلید, کلید_میانگیر))
        أرجع خطأ;

      إذا (!check_Duplicate_Or_Unknown_کلید(I.get_کلید(), کلید, کلیدها_2))
        أرجع خطأ;

      إذا (کلید == "roots") {
        خودکار *Roots = dyn_cast_if_present<yaml::Sequence_گره>(I.get_مقدار());
        إذا (!Roots) {
          error(I.get_مقدار(), "expected array");
          أرجع خطأ;
        }

        لكل (خودکار &I : *Roots) {
          إذا (std::unique_ptr<RedirectingFileSystem::مدخل> E =
                  parse_مدخل(&I, FS, /*IsRootEntry*/ درست))
            ریشه_Entries.افزودن_پشت(std::move(E));
          وإلا
            أرجع خطأ;
        }
      } وإلا إذا (کلید == "version") {
        StringRef نسخه_String;
        SmallString<4> Storage;
        إذا (!parseScalarString(I.get_مقدار(), نسخه_String, Storage))
          أرجع خطأ;
        صحيح نسخه;
        إذا (نسخه_String.getAsInteger<صحيح>(10, نسخه)) {
          error(I.get_مقدار(), "expected integer");
          أرجع خطأ;
        }
        إذا (نسخه < 0) {
          error(I.get_مقدار(), "invalid version number");
          أرجع خطأ;
        }
        إذا (نسخه != 0) {
          error(I.get_مقدار(), "version mismatch, expected 0");
          أرجع خطأ;
        }
      } وإلا إذا (کلید == "case-sensitive") {
        إذا (!parseScalarBool(I.get_مقدار(), FS->CaseSensitive))
          أرجع خطأ;
      } وإلا إذا (کلید == "overlay-relative") {
        إذا (!parseScalarBool(I.get_مقدار(), FS->IsRelativeOverlay))
          أرجع خطأ;
      } وإلا إذا (کلید == "use-external-names") {
        إذا (!parseScalarBool(I.get_مقدار(), FS->Use_External_نامها))
          أرجع خطأ;
      } وإلا إذا (کلید == "fallthrough") {
        إذا (کلیدها_2["redirecting-with"].دیده) {
          error(I.get_مقدار(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        منطقي ShouldFallthrough = خطأ;
        إذا (!parseScalarBool(I.get_مقدار(), ShouldFallthrough))
          أرجع خطأ;

        إذا (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::Redirect_نوع::Fallthrough;
        } وإلا {
          FS->Redirection = RedirectingFileSystem::Redirect_نوع::RedirectOnly;
        }
      } وإلا إذا (کلید == "redirecting-with") {
        إذا (کلیدها_2["fallthrough"].دیده) {
          error(I.get_مقدار(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        إذا (خودکار نوع_3 = parse_Redirect_نوع(I.get_مقدار())) {
          FS->Redirection = *نوع_3;
        } وإلا {
          error(I.get_مقدار(), "expected valid redirect kind");
          أرجع خطأ;
        }
      } وإلا إذا (کلید == "root-relative") {
        إذا (خودکار نوع_3 = parse_ریشه_Relative_نوع(I.get_مقدار())) {
          FS->ریشه_Relative = *نوع_3;
        } وإلا {
          error(I.get_مقدار(), "expected valid root-relative kind");
          أرجع خطأ;
        }
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (جریان.failed())
      أرجع خطأ;

    إذا (!check_Missing_کلیدها(بالاترین, کلیدها_2))
      أرجع خطأ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    لكل (خودکار &E : ریشه_Entries)
      unique_Overlay_درخت(FS, E.get());

    أرجع درست;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<حافظه_میانگیر> میانگیر,
                              منبع_Mgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_مسیر, فراغ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  منبع_Mgr SM;
  yaml::جریان جریان(میانگیر->get_Mem_میانگیر_Ref(), SM);

  SM.مجموعه_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = جریان.begin();
  yaml::گره *ریشه_2 = DI->get_ریشه();
  إذا (DI == جریان.انتها() || !ریشه_2) {
    SM.Print_پیام(SMLoc(), منبع_Mgr::DK_Error, "expected root node");
    أرجع اشاره_تهی;
  }

  RedirectingFileSystemParser P(جریان);

  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));

  إذا (!YAMLFile_مسیر.خالی()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::مسیر_2::والد_مسیر(YAMLFile_مسیر);
    std::error_کد EC = FS->makeAbsolute(OverlayAbsDir);
    أكد(!EC && "Overlay dir final path must be absolute");
    (فراغ)EC;
    FS->مجموعه_Overlay_File_Dir(OverlayAbsDir);
  }

  إذا (!P.parse(ریشه_2, FS.get()))
    أرجع اشاره_تهی;

  أرجع FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    منطقي Use_External_نامها, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));
  FS->Use_External_نامها = Use_External_نامها;

  StringMap<RedirectingFileSystem::مدخل *> Entries;

  لكل (خودکار &Mapping : llvm::معکوس(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.اولی_2);
    SmallString<128> مقصد = StringRef(Mapping.دومی_2);
    {
      خودکار EC = ExternalFS->makeAbsolute(From);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::مدخل *&مقصد_مدخل = Entries[From];
    إذا (مقصد_مدخل)
      استمر;

    // Add parent directories.
    RedirectingFileSystem::مدخل *والد_2 = اشاره_تهی;
    StringRef FromDirectory = llvm::sys::مسیر_2::والد_مسیر(From);
    لكل (خودکار I = llvm::sys::مسیر_2::begin(FromDirectory),
              E = llvm::sys::مسیر_2::انتها(FromDirectory);
         I != E; ++I) {
      والد_2 = RedirectingFileSystemParser::lookup_Or_Create_مدخل(FS.get(), *I,
                                                                والد_2);
    }
    أكد(والد_2 && "File without a directory?");
    {
      خودکار EC = ExternalFS->makeAbsolute(مقصد);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Add the file.
    خودکار NewFile = std::make_unique<RedirectingFileSystem::File_مدخل>(
        llvm::sys::مسیر_2::filename(From), مقصد,
        Use_External_نامها ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    مقصد_مدخل = NewFile.get();
    cast<RedirectingFileSystem::Directory_مدخل>(والد_2)->جمع_کن_Content(
        std::move(NewFile));
  }

  أرجع FS;
}

RedirectingFileSystem::Lookup_نتیجه::Lookup_نتیجه(
    مدخل *E, sys::مسیر_2::const_iterator شروع, sys::مسیر_2::const_iterator انتها_2)
    : E(E) {
  أكد(E != اشاره_تهی);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  إذا (خودکار *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_مدخل>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_مسیر());
    sys::مسیر_2::append(Redirect, شروع, انتها_2,
                      getExistingStyle(DRE->get_External_Contents_مسیر()));
    ExternalRedirect = std::string(Redirect);
  }
}

فراغ RedirectingFileSystem::Lookup_نتیجه::get_مسیر(
    llvm::Small_وکتور_Impl<حرف> &نتیجه) ثابت {
  نتیجه.clear();
  لكل (مدخل *والد_2 : Parents)
    llvm::sys::مسیر_2::append(نتیجه, والد_2->getName());
  llvm::sys::مسیر_2::append(نتیجه, E->getName());
}

std::error_کد RedirectingFileSystem::makeCanonicalForLookup(
    Small_وکتور_Impl<حرف> &مسیر) ثابت {
  إذا (std::error_کد EC = makeAbsolute(مسیر))
    أرجع EC;

  llvm::SmallString<256> Canonical_مسیر =
      canonicalize(StringRef(مسیر.داده(), مسیر.اندازه_2()));
  إذا (Canonical_مسیر.خالی())
    أرجع make_error_کد(llvm::errc::invalid_آرگومان);

  مسیر.تخصیص_بده(Canonical_مسیر.begin(), Canonical_مسیر.انتها());
  أرجع {};
}

ErrorOr<RedirectingFileSystem::Lookup_نتیجه>
RedirectingFileSystem::lookup_مسیر(StringRef مسیر) ثابت {
  llvm::SmallString<128> Canonical_مسیر(مسیر);
  إذا (std::error_کد EC = makeCanonicalForLookup(Canonical_مسیر))
    أرجع EC;

  // RedirectOnly means the VFS is always used.
  إذا (Usage_Tracking_فعال && Redirection == Redirect_نوع::RedirectOnly)
    Has_Been_استفاده_شده = درست;

  sys::مسیر_2::const_iterator شروع = sys::مسیر_2::begin(Canonical_مسیر);
  sys::مسیر_2::const_iterator انتها_2 = sys::مسیر_2::انتها(Canonical_مسیر);
  llvm::Small_وکتور<مدخل *, 32> Entries;
  لكل (ثابت خودکار &ریشه_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_نتیجه> نتیجه =
        lookup_مسیر_Impl(شروع, انتها_2, ریشه_2.get(), Entries);
    إذا (Usage_Tracking_فعال && نتیجه && isa<Remap_مدخل>(نتیجه->E))
      Has_Been_استفاده_شده = درست;
    إذا (نتیجه) {
      نتیجه->Parents = std::move(Entries);
      أرجع نتیجه;
    }

    إذا (نتیجه.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتیجه;
  }
  أرجع make_error_کد(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_نتیجه>
RedirectingFileSystem::lookup_مسیر_Impl(
    sys::مسیر_2::const_iterator شروع, sys::مسیر_2::const_iterator انتها_2,
    RedirectingFileSystem::مدخل *From,
    llvm::Small_وکتور_Impl<مدخل *> &Entries) ثابت {
  أكد(!isTraversalComponent(*شروع) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  إذا (!FromName.خالی()) {
    إذا (!مسیر_Component_Matches(*شروع, FromName))
      أرجع make_error_کد(llvm::errc::no_such_file_or_directory);

    ++شروع;

    إذا (شروع == انتها_2) {
      // Match!
      أرجع Lookup_نتیجه(From, شروع, انتها_2);
    }
  }

  إذا (isa<RedirectingFileSystem::File_مدخل>(From))
    أرجع make_error_کد(llvm::errc::not_a_directory);

  إذا (isa<RedirectingFileSystem::Directory_Remap_مدخل>(From))
    أرجع Lookup_نتیجه(From, شروع, انتها_2);

  خودکار *DE = cast<RedirectingFileSystem::Directory_مدخل>(From);
  لكل (ثابت std::unique_ptr<RedirectingFileSystem::مدخل> &Dir_مدخل :
       llvm::make_range(DE->contents_begin(), DE->contents_انتها())) {
    Entries.افزودن_پشت(From);
    ErrorOr<RedirectingFileSystem::Lookup_نتیجه> نتیجه =
        lookup_مسیر_Impl(شروع, انتها_2, Dir_مدخل.get(), Entries);
    إذا (نتیجه || نتیجه.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتیجه;
    Entries.برداشت_پشت();
  }

  أرجع make_error_کد(llvm::errc::no_such_file_or_directory);
}

ساكن وضعیت get_Redirected_File_وضعیت(ثابت Twine &Original_مسیر,
                                      منطقي Use_External_نامها,
                                      وضعیت External_وضعیت) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  إذا (External_وضعیت.ExposesExternalVFSPath)
    أرجع External_وضعیت;

  وضعیت S = External_وضعیت;
  إذا (!Use_External_نامها)
    S = وضعیت::copyWithNewName(S, Original_مسیر);
  وإلا
    S.ExposesExternalVFSPath = درست;
  أرجع S;
}

ErrorOr<وضعیت> RedirectingFileSystem::وضعیت_2(
    ثابت Twine &Lookup_مسیر, ثابت Twine &Original_مسیر,
    ثابت RedirectingFileSystem::Lookup_نتیجه &نتیجه) {
  إذا (std::optional<StringRef> ExtRedirect = نتیجه.getExternalRedirect()) {
    SmallString<256> Remapped_مسیر((*ExtRedirect).str());
    إذا (std::error_کد EC = makeAbsolute(Remapped_مسیر))
      أرجع EC;

    ErrorOr<وضعیت> S = ExternalFS->وضعیت_2(Remapped_مسیر);
    إذا (!S)
      أرجع S;
    S = وضعیت::copyWithNewName(*S, *ExtRedirect);
    خودکار *RE = cast<RedirectingFileSystem::Remap_مدخل>(نتیجه.E);
    أرجع get_Redirected_File_وضعیت(Original_مسیر,
                                   RE->useExternalName(Use_External_نامها), *S);
  }

  خودکار *DE = cast<RedirectingFileSystem::Directory_مدخل>(نتیجه.E);
  أرجع وضعیت::copyWithNewName(DE->get_وضعیت(), Lookup_مسیر);
}

ErrorOr<وضعیت>
RedirectingFileSystem::get_External_وضعیت(ثابت Twine &Lookup_مسیر,
                                         ثابت Twine &Original_مسیر) ثابت {
  خودکار نتیجه = ExternalFS->وضعیت_2(Lookup_مسیر);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  إذا (!نتیجه || نتیجه->ExposesExternalVFSPath)
    أرجع نتیجه;
  أرجع وضعیت::copyWithNewName(نتیجه.get(), Original_مسیر);
}

ErrorOr<وضعیت> RedirectingFileSystem::وضعیت_2(ثابت Twine &Original_مسیر) {
  SmallString<256> مسیر;
  Original_مسیر.مقصد_وکتور(مسیر);

  إذا (std::error_کد EC = makeAbsolute(مسیر))
    أرجع EC;

  إذا (Redirection == Redirect_نوع::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<وضعیت> S = get_External_وضعیت(مسیر, Original_مسیر);
    إذا (S)
      أرجع S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجه> نتیجه = lookup_مسیر(مسیر);
  إذا (!نتیجه) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_نوع::Fallthrough &&
        is_File_Not_یافت(نتیجه.getError()))
      أرجع get_External_وضعیت(مسیر, Original_مسیر);
    أرجع نتیجه.getError();
  }

  ErrorOr<وضعیت> S = وضعیت_2(مسیر, Original_مسیر, *نتیجه);
  إذا (!S && Redirection == Redirect_نوع::Fallthrough &&
      is_File_Not_یافت(S.getError(), نتیجه->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع get_External_وضعیت(مسیر, Original_مسیر);
  }

  أرجع S;
}

منطقي RedirectingFileSystem::exists(ثابت Twine &Original_مسیر) {
  SmallString<256> مسیر;
  Original_مسیر.مقصد_وکتور(مسیر);

  إذا (makeAbsolute(مسیر))
    أرجع خطأ;

  إذا (Redirection == Redirect_نوع::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    إذا (ExternalFS->exists(مسیر))
      أرجع درست;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجه> نتیجه = lookup_مسیر(مسیر);
  إذا (!نتیجه) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_نوع::Fallthrough &&
        is_File_Not_یافت(نتیجه.getError()))
      أرجع ExternalFS->exists(مسیر);
    أرجع خطأ;
  }

  std::optional<StringRef> ExtRedirect = نتیجه->getExternalRedirect();
  إذا (!ExtRedirect) {
    أكد(isa<RedirectingFileSystem::Directory_مدخل>(نتیجه->E));
    أرجع درست;
  }

  SmallString<256> Remapped_مسیر((*ExtRedirect).str());
  إذا (makeAbsolute(Remapped_مسیر))
    أرجع خطأ;

  إذا (ExternalFS->exists(Remapped_مسیر))
    أرجع درست;

  إذا (Redirection == Redirect_نوع::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع ExternalFS->exists(مسیر);
  }

  أرجع خطأ;
}

نطاق {

/// Provide a file wrapper with an overriden status.
صنف File_With_Fixed_وضعیت : عام File {
  std::unique_ptr<File> InnerFile;
  وضعیت S;

عام:
  File_With_Fixed_وضعیت(std::unique_ptr<File> InnerFile, وضعیت S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<وضعیت> وضعیت_2() override { أرجع S; }
  ErrorOr<std::unique_ptr<llvm::حافظه_میانگیر>>

  get_میانگیر(ثابت Twine &Name, int64_t File_اندازه, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    أرجع InnerFile->get_میانگیر(Name, File_اندازه, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_کد close() override { أرجع InnerFile->close(); }

  فراغ مجموعه_مسیر(ثابت Twine &مسیر) override { S = S.copyWithNewName(S, مسیر); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_مسیر(ErrorOr<std::unique_ptr<File>> نتیجه, ثابت Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  إذا (!نتیجه || (*نتیجه)->وضعیت_2()->ExposesExternalVFSPath)
    أرجع نتیجه;

  ErrorOr<std::unique_ptr<File>> F = std::move(*نتیجه);
  خودکار Name = F->get()->getName();
  إذا (Name && Name.get() != P.str())
    F->get()->مجموعه_مسیر(P);
  أرجع F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ثابت Twine &Original_مسیر) {
  SmallString<256> مسیر;
  Original_مسیر.مقصد_وکتور(مسیر);

  إذا (std::error_کد EC = makeAbsolute(مسیر))
    أرجع EC;

  إذا (Redirection == Redirect_نوع::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    خودکار F = File::get_With_مسیر(ExternalFS->openFileForRead(مسیر), Original_مسیر);
    إذا (F)
      أرجع F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجه> نتیجه = lookup_مسیر(مسیر);
  إذا (!نتیجه) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_نوع::Fallthrough &&
        is_File_Not_یافت(نتیجه.getError()))
      أرجع File::get_With_مسیر(ExternalFS->openFileForRead(مسیر), Original_مسیر);
    أرجع نتیجه.getError();
  }

  إذا (!نتیجه->getExternalRedirect()) // FIXME: errc::not_a_file?
    أرجع make_error_کد(llvm::errc::invalid_آرگومان);

  StringRef ExtRedirect = *نتیجه->getExternalRedirect();
  SmallString<256> Remapped_مسیر(ExtRedirect.str());
  إذا (std::error_کد EC = makeAbsolute(Remapped_مسیر))
    أرجع EC;

  خودکار *RE = cast<RedirectingFileSystem::Remap_مدخل>(نتیجه->E);

  خودکار ExternalFile =
      File::get_With_مسیر(ExternalFS->openFileForRead(Remapped_مسیر), ExtRedirect);
  إذا (!ExternalFile) {
    إذا (Redirection == Redirect_نوع::Fallthrough &&
        is_File_Not_یافت(ExternalFile.getError(), نتیجه->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع File::get_With_مسیر(ExternalFS->openFileForRead(مسیر), Original_مسیر);
    }
    أرجع ExternalFile;
  }

  خودکار External_وضعیت = (*ExternalFile)->وضعیت_2();
  إذا (!External_وضعیت)
    أرجع External_وضعیت.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  وضعیت S = get_Redirected_File_وضعیت(
      Original_مسیر, RE->useExternalName(Use_External_نامها), *External_وضعیت);
  أرجع std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_وضعیت>(std::move(*ExternalFile), S));
}

std::error_کد
RedirectingFileSystem::get_Real_مسیر(ثابت Twine &Original_مسیر,
                                   Small_وکتور_Impl<حرف> &Output) {
  SmallString<256> مسیر;
  Original_مسیر.مقصد_وکتور(مسیر);

  إذا (std::error_کد EC = makeAbsolute(مسیر))
    أرجع EC;

  إذا (Redirection == Redirect_نوع::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_کد EC = ExternalFS->get_Real_مسیر(مسیر, Output);
    إذا (!EC)
      أرجع EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجه> نتیجه = lookup_مسیر(مسیر);
  إذا (!نتیجه) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_نوع::Fallthrough &&
        is_File_Not_یافت(نتیجه.getError()))
      أرجع ExternalFS->get_Real_مسیر(مسیر, Output);
    أرجع نتیجه.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  إذا (خودکار ExtRedirect = نتیجه->getExternalRedirect()) {
    خودکار P = ExternalFS->get_Real_مسیر(*ExtRedirect, Output);
    إذا (P && Redirection == Redirect_نوع::Fallthrough &&
        is_File_Not_یافت(P, نتیجه->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع ExternalFS->get_Real_مسیر(مسیر, Output);
    }
    أرجع P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  إذا (Redirection == Redirect_نوع::Fallthrough) {
    نتیجه->get_مسیر(Output);
    أرجع {};
  }
  أرجع llvm::errc::invalid_آرگومان;
}

فراغ RedirectingFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_وکتور_Impl<std::string> &بیرون) {
  SmallString<256> مسیر;
  Dir.مقصد_وکتور(مسیر);

  إذا (makeAbsolute(مسیر))
    أرجع;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ثابت منطقي ConsultsExternalFS = Redirection != Redirect_نوع::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_نتیجه> نتیجه = lookup_مسیر(مسیر);
  إذا (!نتیجه) {
    // dir_begin() delegates entirely to ExternalFS.
    إذا (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(مسیر, بیرون);
    أرجع;
  }

  بدل (نتیجه->E->get_نوع()) {
  حالة EK_File:
    أرجع;
  حالة EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    اكسر;
  حالة EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_مسیر(*نتیجه->getExternalRedirect());
    إذا (!makeAbsolute(Remapped_مسیر))
      ExternalFS->getDirectoryContentRealSources(Remapped_مسیر, بیرون);
    اكسر;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  إذا (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(مسیر, بیرون);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<حافظه_میانگیر> میانگیر,
                    منبع_Mgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_مسیر, فراغ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  أرجع RedirectingFileSystem::create(std::move(میانگیر), DiagHandler,
                                       YAMLFile_مسیر, DiagContext,
                                       std::move(ExternalFS));
}

ساكن فراغ getVFSEntries(RedirectingFileSystem::مدخل *SrcE,
                          Small_وکتور_Impl<StringRef> &مسیر,
                          Small_وکتور_Impl<YAMLVFSEntry> &Entries) {
  خودکار نوع_3 = SrcE->get_نوع();
  إذا (نوع_3 == RedirectingFileSystem::EK_Directory) {
    خودکار *DE = dyn_cast<RedirectingFileSystem::Directory_مدخل>(SrcE);
    أكد(DE && "Must be a directory");
    لكل (std::unique_ptr<RedirectingFileSystem::مدخل> &Sub_مدخل :
         llvm::make_range(DE->contents_begin(), DE->contents_انتها())) {
      مسیر.افزودن_پشت(Sub_مدخل->getName());
      getVFSEntries(Sub_مدخل.get(), مسیر, Entries);
      مسیر.برداشت_پشت();
    }
    أرجع;
  }

  إذا (نوع_3 == RedirectingFileSystem::EK_DirectoryRemap) {
    خودکار *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_مدخل>(SrcE);
    أكد(DR && "Must be a directory remap");
    SmallString<128> VPath;
    لكل (خودکار &Comp : مسیر)
      llvm::sys::مسیر_2::append(VPath, Comp);
    Entries.افزودن_پشت(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_مسیر()));
    أرجع;
  }

  أكد(نوع_3 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  خودکار *FE = dyn_cast<RedirectingFileSystem::File_مدخل>(SrcE);
  أكد(FE && "Must be a file");
  SmallString<128> VPath;
  لكل (خودکار &Comp : مسیر)
    llvm::sys::مسیر_2::append(VPath, Comp);
  Entries.افزودن_پشت(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_مسیر()));
}

فراغ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_وکتور_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_نتیجه> ریشه_نتیجه = VFS.lookup_مسیر("/");
  إذا (!ریشه_نتیجه)
    أرجع;
  Small_وکتور<StringRef, 8> Components;
  Components.افزودن_پشت("/");
  getVFSEntries(ریشه_نتیجه->E, Components, CollectedEntries);
}

UniqueID vfs::get_بعدی_Virtual_Unique_ID() {
  ساكن std::atomic<بیعلامت> UID;
  بیعلامت ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  أرجع UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

فراغ YAMLVFSWriter::جمع_کن_مدخل(StringRef Virtual_مسیر, StringRef Real_مسیر,
                             منطقي IsDirectory) {
  أكد(sys::مسیر_2::is_absolute(Virtual_مسیر) && "virtual path not absolute");
  أكد(sys::مسیر_2::is_absolute(Real_مسیر) && "real path not absolute");
  أكد(!مسیر_Has_Traversal(Virtual_مسیر) && "path traversal is not supported");
  Mappings.emplace_پشت(Virtual_مسیر, Real_مسیر, IsDirectory);
}

فراغ YAMLVFSWriter::جمع_کن_File_Mapping(StringRef Virtual_مسیر, StringRef Real_مسیر) {
  جمع_کن_مدخل(Virtual_مسیر, Real_مسیر, /*IsDirectory=*/خطأ);
}

فراغ YAMLVFSWriter::جمع_کن_Directory_Mapping(StringRef Virtual_مسیر,
                                        StringRef Real_مسیر) {
  جمع_کن_مدخل(Virtual_مسیر, Real_مسیر, /*IsDirectory=*/درست);
}

نطاق {

صنف JSONWriter {
  llvm::raw_ostream &OS;
  Small_وکتور<StringRef, 16> Dir_پشته;

  بیعلامت getDirIndent() { أرجع 4 * Dir_پشته.اندازه_2(); }
  بیعلامت getFileIndent() { أرجع 4 * (Dir_پشته.اندازه_2() + 1); }
  منطقي contained_درون(StringRef والد_2, StringRef مسیر);
  StringRef containedPart(StringRef والد_2, StringRef مسیر);
  فراغ شروع_Directory(StringRef مسیر);
  فراغ انتها_Directory();
  فراغ write_مدخل(StringRef VPath, StringRef RPath);

عام:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  فراغ اكتب(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<منطقي> Use_External_نامها,
             std::optional<منطقي> IsCaseSensitive,
             std::optional<منطقي> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

منطقي JSONWriter::contained_درون(StringRef والد_2, StringRef مسیر) {
  استفاده نطاق llvm::sys;

  // Compare each path component.
  خودکار IParent = مسیر_2::begin(والد_2), EParent = مسیر_2::انتها(والد_2);
  لكل (خودکار IChild = مسیر_2::begin(مسیر), EChild = مسیر_2::انتها(مسیر);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    إذا (*IParent != *IChild)
      أرجع خطأ;
  }
  // Have we exhausted the parent path?
  أرجع IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef والد_2, StringRef مسیر) {
  أكد(!والد_2.خالی());
  أكد(contained_درون(والد_2, مسیر));
  أرجع مسیر.substr(والد_2.اندازه_2() + 1);
}

فراغ JSONWriter::شروع_Directory(StringRef مسیر) {
  StringRef Name =
      Dir_پشته.خالی() ? مسیر : containedPart(Dir_پشته.پشت(), مسیر);
  Dir_پشته.افزودن_پشت(مسیر);
  بیعلامت Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

فراغ JSONWriter::انتها_Directory() {
  بیعلامت Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_پشته.برداشت_پشت();
}

فراغ JSONWriter::write_مدخل(StringRef VPath, StringRef RPath) {
  بیعلامت Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

فراغ JSONWriter::اكتب(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<منطقي> Use_External_نامها,
                       std::optional<منطقي> IsCaseSensitive,
                       std::optional<منطقي> IsOverlayRelative,
                       StringRef OverlayDir) {
  استفاده نطاق llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  إذا (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  إذا (Use_External_نامها)
    OS << "  'use-external-names': '" << (*Use_External_نامها ? "true" : "false")
       << "',\n";
  منطقي UseOverlayRelative = خطأ;
  إذا (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  إذا (!Entries.خالی()) {
    ثابت YAMLVFSEntry &مدخل = Entries.front();

    شروع_Directory(
      مدخل.IsDirectory ? مدخل.VPath : مسیر_2::والد_مسیر(مدخل.VPath)
    );

    StringRef RPath = مدخل.RPath;
    إذا (UseOverlayRelative) {
      أكد(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.اندازه_2());
    }

    منطقي Is_Current_Dir_خالی = درست;
    إذا (!مدخل.IsDirectory) {
      write_مدخل(مسیر_2::filename(مدخل.VPath), RPath);
      Is_Current_Dir_خالی = خطأ;
    }

    لكل (ثابت خودکار &مدخل : Entries.برش(1)) {
      StringRef Dir =
          مدخل.IsDirectory ? مدخل.VPath : مسیر_2::والد_مسیر(مدخل.VPath);
      إذا (Dir == Dir_پشته.پشت()) {
        إذا (!Is_Current_Dir_خالی) {
          OS << ",\n";
        }
      } وإلا {
        منطقي Is_Dir_Popped_From_پشته = خطأ;
        طالما (!Dir_پشته.خالی() && !contained_درون(Dir_پشته.پشت(), Dir)) {
          OS << "\n";
          انتها_Directory();
          Is_Dir_Popped_From_پشته = درست;
        }
        إذا (Is_Dir_Popped_From_پشته || !Is_Current_Dir_خالی) {
          OS << ",\n";
        }
        شروع_Directory(Dir);
        Is_Current_Dir_خالی = درست;
      }
      StringRef RPath = مدخل.RPath;
      إذا (UseOverlayRelative) {
        أكد(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.اندازه_2());
      }
      إذا (!مدخل.IsDirectory) {
        write_مدخل(مسیر_2::filename(مدخل.VPath), RPath);
        Is_Current_Dir_خالی = خطأ;
      }
    }

    طالما (!Dir_پشته.خالی()) {
      OS << "\n";
      انتها_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

فراغ YAMLVFSWriter::اكتب(llvm::raw_ostream &OS) {
  llvm::مرتب_سازی(Mappings, [](ثابت YAMLVFSEntry &LHS, ثابت YAMLVFSEntry &RHS) {
    أرجع LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).اكتب(Mappings, Use_External_نامها, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ثابت Twine &مسیر, std::error_کد &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(مسیر, EC);
  إذا (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->پشته.افزودن_پشت(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_کد &EC) {
  أكد(FS && State && !State->پشته.خالی() && "incrementing past end");
  أكد(!State->پشته.پشت()->مسیر_2().خالی() && "non-canonical end iterator");
  vfs::directory_iterator انتها_2;

  إذا (State->Has_No_افزودن_Request)
    State->Has_No_افزودن_Request = خطأ;
  وإلا {
    إذا (State->پشته.پشت()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->پشته.پشت()->مسیر_2(), EC);
      إذا (I != انتها_2) {
        State->پشته.افزودن_پشت(I);
        أرجع *هذا;
      }
    }
  }

  طالما (!State->پشته.خالی() && State->پشته.پشت().increment(EC) == انتها_2)
    State->پشته.برداشت_پشت();

  إذا (State->پشته.خالی())
    State.reset(); // end iterator

  أرجع *هذا;
}

ثابت حرف FileSystem::ID = 0;
ثابت حرف OverlayFileSystem::ID = 0;
ثابت حرف ProxyFileSystem::ID = 0;
ثابت حرف درون_حافظه_File_System::ID = 0;
ثابت حرف RedirectingFileSystem::ID = 0;

بیعلامت ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_شمار(
    ثابت FileSystem *FS) {
  أرجع FS->Use_شمار();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
