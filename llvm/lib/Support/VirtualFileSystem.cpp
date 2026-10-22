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

ورتوں نطاق llvm;
ورتوں نطاق llvm::vfs;

ورتوں llvm::sys::fs::file_حالت;
ورتوں llvm::sys::fs::file_t;
ورتوں llvm::sys::fs::file_type;
ورتوں llvm::sys::fs::perms;
ورتوں llvm::sys::fs::UniqueID;

حالت::حالت(ثابت file_حالت &حالت)
    : UID(حالت.getUniqueID()), MTime(حالت.get_آخری_Modification_ویلا()),
      User(حالت.getUser()), گروہ(حالت.get_گروہ()), ناپ(حالت.get_ناپ()),
      Type(حالت.type()), Perms(حالت.permissions()) {}

حالت::حالت(ثابت Twine &Name, UniqueID UID, sys::ویلا_نقطہ<> MTime,
               uint32_t User, uint32_t گروہ, uint64_t ناپ, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), گروہ(گروہ),
      ناپ(ناپ), Type(Type), Perms(Perms) {}

حالت حالت::copy_With_New_ناپ(ثابت حالت &وچ_2, uint64_t New_ناپ) {
  أرجع حالت(وچ_2.getName(), وچ_2.getUniqueID(), وچ_2.get_آخری_Modification_ویلا(),
                وچ_2.getUser(), وچ_2.get_گروہ(), New_ناپ, وچ_2.getType(),
                وچ_2.getPermissions());
}

حالت حالت::copyWithNewName(ثابت حالت &وچ_2, ثابت Twine &NewName) {
  أرجع حالت(NewName, وچ_2.getUniqueID(), وچ_2.get_آخری_Modification_ویلا(),
                وچ_2.getUser(), وچ_2.get_گروہ(), وچ_2.get_ناپ(), وچ_2.getType(),
                وچ_2.getPermissions());
}

حالت حالت::copyWithNewName(ثابت file_حالت &وچ_2, ثابت Twine &NewName) {
  أرجع حالت(NewName, وچ_2.getUniqueID(), وچ_2.get_آخری_Modification_ویلا(),
                وچ_2.getUser(), وچ_2.get_گروہ(), وچ_2.get_ناپ(), وچ_2.type(),
                وچ_2.permissions());
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
  أرجع is_حالت_معلوم() && Type != file_type::file_not_لبھیا;
}

File::~File() = افتراضي;

FileSystem::~FileSystem() = افتراضي;

ErrorOr<std::unique_ptr<حافظہ_بفر>>
FileSystem::get_بفر_For_File(ثابت llvm::Twine &Name, int64_t File_ناپ,
                             منطقي RequiresNullTerminator, منطقي IsVolatile,
                             منطقي Is_متن) {
  خودکار F = Is_متن ? openFileForRead(Name) : open_File_For_Read_ثنائی(Name);
  إذا (!F)
    أرجع F.getError();

  أرجع (*F)->get_بفر(Name, File_ناپ, RequiresNullTerminator, IsVolatile);
}

std::error_کوڈ FileSystem::makeAbsolute(Small_سمتیہ_Impl<حرف> &رستہ) ثابت {
  إذا (llvm::sys::رستہ_2::is_absolute(رستہ))
    أرجع {};

  خودکار WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  sys::رستہ_2::make_absolute(WorkingDir.get(), رستہ);
  أرجع {};
}

std::error_کوڈ FileSystem::get_Real_رستہ(ثابت Twine &رستہ,
                                        Small_سمتیہ_Impl<حرف> &Output) {
  أرجع errc::operation_not_permitted;
}

std::error_کوڈ FileSystem::isLocal(ثابت Twine &رستہ, منطقي &نتیجہ) {
  أرجع errc::operation_not_permitted;
}

منطقي FileSystem::exists(ثابت Twine &رستہ) {
  خودکار حالت = حالت_2(رستہ);
  أرجع حالت && حالت->exists();
}

llvm::ErrorOr<منطقي> FileSystem::equivalent(ثابت Twine &A, ثابت Twine &B) {
  خودکار حالت_A = حالت_2(A);
  إذا (!حالت_A)
    أرجع حالت_A.getError();
  خودکار حالت_B = حالت_2(B);
  إذا (!حالت_B)
    أرجع حالت_B.getError();
  أرجع حالت_A->equivalent(*حالت_B);
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
فراغ FileSystem::dump() ثابت { print(dbgs(), PrintType::RecursiveContents); }
#جے_انت

#جے_بنا_تعریف NDEBUG
ساكن منطقي isTraversalComponent(StringRef Component) {
  أرجع Component == ".." || Component == ".";
}

ساكن منطقي رستہ_Has_Traversal(StringRef رستہ) {
  ورتوں نطاق llvm::sys;

  لكل (StringRef Comp : llvm::make_range(رستہ_2::begin(رستہ), رستہ_2::سرا(رستہ)))
    إذا (isTraversalComponent(Comp))
      أرجع سچ;
  أرجع خطأ;
}
#جے_انت

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

/// Wrapper around a raw file descriptor.
صنف RealFile : عام File {
  یار صنف RealFileSystem;

  file_t FD;
  حالت S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_رستہ_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::حالت_error, {}),
        RealName(New_Real_رستہ_Name.str()) {
    أكد(FD.isValid() && "Invalid or inactive file descriptor");
  }

عام:
  ~RealFile() override;

  ErrorOr<حالت> حالت_2() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<حافظہ_بفر>> get_بفر(ثابت Twine &Name,
                                                   int64_t File_ناپ,
                                                   منطقي RequiresNullTerminator,
                                                   منطقي IsVolatile) override;
  std::error_کوڈ close() override;
  فراغ سیٹ_رستہ(ثابت Twine &رستہ) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<حالت> RealFile::حالت_2() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot stat closed file");
  إذا (!S.is_حالت_معلوم()) {
    file_حالت Real_حالت;
    إذا (std::error_کوڈ EC = sys::fs::حالت_2(FD, Real_حالت))
      أرجع EC;
    S = حالت::copyWithNewName(Real_حالت, S.getName());
  }
  أرجع S;
}

ErrorOr<std::string> RealFile::getName() {
  أرجع RealName.خالی_تھاواں() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
RealFile::get_بفر(ثابت Twine &Name, int64_t File_ناپ,
                    منطقي RequiresNullTerminator, منطقي IsVolatile) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot get buffer for closed file");
  أرجع حافظہ_بفر::getOpenFile(FD, Name, File_ناپ, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_کوڈ RealFile::close() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  std::error_کوڈ EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  أرجع EC;
}

فراغ RealFile::سیٹ_رستہ(ثابت Twine &رستہ) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  RealName = رستہ.str();
  إذا (خودکار حالت = حالت_2())
    S = حالت.get().copyWithNewName(حالت.get(), رستہ);
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
  واضح RealFileSystem(منطقي LinkCWDToProcess) {
    إذا (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      إذا (std::error_کوڈ EC = llvm::sys::fs::current_رستہ(PWD))
        WD = std::move(EC);
      وإلا إذا (llvm::sys::fs::real_رستہ(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      وإلا
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<حالت> حالت_2(ثابت Twine &رستہ) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ثابت Twine &رستہ) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ثنائی(ثابت Twine &رستہ) override;
  directory_iterator dir_begin(ثابت Twine &Dir, std::error_کوڈ &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override;
  std::error_کوڈ سیٹ_Current_Working_Directory(ثابت Twine &رستہ) override;
  std::error_کوڈ isLocal(ثابت Twine &رستہ, منطقي &نتیجہ) override;
  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_سمتیہ_Impl<std::string> &باہرلا) override {
    باہرلا.ڈالو_پٹھ(Dir.str());
  }
  std::error_کوڈ get_Real_رستہ(ثابت Twine &رستہ,
                              Small_سمتیہ_Impl<حرف> &Output) override;

محمي:
  فراغ printImpl(raw_ostream &OS, PrintType Type,
                 بنانشانی IndentLevel) ثابت override;

خاص:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_رستہ(ثابت Twine &رستہ, Small_سمتیہ_Impl<حرف> &Storage) ثابت {
    إذا (!WD || !*WD)
      أرجع رستہ;
    رستہ.منزل_سمتیہ(Storage);
    sys::رستہ_2::make_absolute(WD->get().Resolved, Storage);
    أرجع Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ثابت Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_رستہ(Name, Storage), Flags, &RealName);
    إذا (!FDOrErr)
      أرجع error_منزل_Error_کوڈ(FDOrErr.takeError());
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

ErrorOr<حالت> RealFileSystem::حالت_2(ثابت Twine &رستہ) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_حالت Real_حالت;
  إذا (std::error_کوڈ EC =
          sys::fs::حالت_2(adjust_رستہ(رستہ, Storage), Real_حالت))
    أرجع EC;
  أرجع حالت::copyWithNewName(Real_حالت, رستہ);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ثابت Twine &Name) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::دا_متن);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ثنائی(ثابت Twine &Name) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::دا_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ثابت {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (WD && *WD)
    أرجع std::string(WD->get().Specified);
  إذا (WD)
    أرجع WD->getError();

  SmallString<128> Dir;
  إذا (std::error_کوڈ EC = llvm::sys::fs::current_رستہ(Dir))
    أرجع EC;
  أرجع std::string(Dir);
}

std::error_کوڈ RealFileSystem::سیٹ_Current_Working_Directory(ثابت Twine &رستہ) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (!WD)
    أرجع llvm::sys::fs::سیٹ_current_رستہ(رستہ);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_رستہ(رستہ, Storage).منزل_سمتیہ(Absolute);
  منطقي IsDir;
  إذا (خودکار Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    أرجع Err;
  إذا (!IsDir)
    أرجع std::make_error_کوڈ(std::errc::not_a_directory);
  إذا (خودکار Err = llvm::sys::fs::real_رستہ(Absolute, Resolved))
    أرجع Err;
  WD = WorkingDirectory{Absolute, Resolved};
  أرجع std::error_کوڈ();
}

std::error_کوڈ RealFileSystem::isLocal(ثابت Twine &رستہ, منطقي &نتیجہ) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::is_local(adjust_رستہ(رستہ, Storage), نتیجہ);
}

std::error_کوڈ RealFileSystem::get_Real_رستہ(ثابت Twine &رستہ,
                                            Small_سمتیہ_Impl<حرف> &Output) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::real_رستہ(adjust_رستہ(رستہ, Storage), Output);
}

فراغ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               بنانشانی IndentLevel) ثابت {
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
      makeIntrusiveRefCnt<RealFileSystem>(سچ);
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
  RealFSDirIter(ثابت Twine &رستہ, std::error_کوڈ &EC) {
    خودکار BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(رستہ, EC);
    إذا (Iter != sys::fs::directory_iterator())
      Current_اندراج = directory_اندراج(Iter->رستہ_2(), Iter->type());
  }

  std::error_کوڈ increment() override {
    خودکار BypassSandbox = sys::sandbox::scopedDisable();

    std::error_کوڈ EC;
    Iter.increment(EC);
    Current_اندراج = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_اندراج()
                       : directory_اندراج(Iter->رستہ_2(), Iter->type());
    أرجع EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ثابت Twine &Dir,
                                             std::error_کوڈ &EC) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  أرجع directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_رستہ(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> ادھار_FS) {
  FSList.ڈالو_پٹھ(std::move(ادھار_FS));
}

فراغ OverlayFileSystem::ڈالو_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ڈالو_پٹھ(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->سیٹ_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<حالت> OverlayFileSystem::حالت_2(ثابت Twine &رستہ) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_سرا(); I != E; ++I) {
    ErrorOr<حالت> حالت = (*I)->حالت_2(رستہ);
    إذا (حالت || حالت.getError() != llvm::errc::no_such_file_or_directory)
      أرجع حالت;
  }
  أرجع make_error_کوڈ(llvm::errc::no_such_file_or_directory);
}

منطقي OverlayFileSystem::exists(ثابت Twine &رستہ) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_سرا(); I != E; ++I) {
    إذا ((*I)->exists(رستہ))
      أرجع سچ;
  }
  أرجع خطأ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ثابت llvm::Twine &رستہ) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_سرا(); I != E; ++I) {
    خودکار نتیجہ = (*I)->openFileForRead(رستہ);
    إذا (نتیجہ || نتیجہ.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتیجہ;
  }
  أرجع make_error_کوڈ(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ثابت {
  // All file systems are synchronized, just take the first working directory.
  أرجع FSList.front()->getCurrentWorkingDirectory();
}

std::error_کوڈ
OverlayFileSystem::سیٹ_Current_Working_Directory(ثابت Twine &رستہ) {
  لكل (خودکار &FS : FSList)
    إذا (std::error_کوڈ EC = FS->سیٹ_Current_Working_Directory(رستہ))
      أرجع EC;
  أرجع {};
}

std::error_کوڈ OverlayFileSystem::isLocal(ثابت Twine &رستہ, منطقي &نتیجہ) {
  لكل (خودکار &FS : FSList)
    إذا (FS->exists(رستہ))
      أرجع FS->isLocal(رستہ, نتیجہ);
  أرجع errc::no_such_file_or_directory;
}

std::error_کوڈ OverlayFileSystem::get_Real_رستہ(ثابت Twine &رستہ,
                                               Small_سمتیہ_Impl<حرف> &Output) {
  لكل (ثابت خودکار &FS : FSList)
    إذا (FS->exists(رستہ))
      أرجع FS->get_Real_رستہ(رستہ, Output);
  أرجع errc::no_such_file_or_directory;
}

فراغ OverlayFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_سمتیہ_Impl<std::string> &باہرلا) {
  // All layers contribute.
  لكل (iterator I = overlays_begin(), E = overlays_سرا(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, باہرلا);
}

فراغ OverlayFileSystem::ویکھو_بچہ_File_Systems(ویکھو_Callback_Ty Callback) {
  لكل (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->ویکھو_بچہ_File_Systems(Callback);
  }
}

فراغ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  بنانشانی IndentLevel) ثابت {
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
  ورتوں FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_سمتیہ<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_سیٹ<> تکیا_ناں;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_کوڈ incrementIter(منطقي Is_پہلا_ویلا) {
    طالما (!IterList.خالی_تھاواں()) {
      CurrentDirIter = IterList.پٹھ();
      IterList.نکالو_پٹھ();
      إذا (CurrentDirIter != directory_iterator())
        اكسر; //لبھیا
    }

    إذا (Is_پہلا_ویلا && CurrentDirIter == directory_iterator())
      أرجع errc::no_such_file_or_directory;
    أرجع {};
  }

  std::error_کوڈ incrementDirIter(منطقي Is_پہلا_ویلا) {
    أكد((Is_پہلا_ویلا || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_کوڈ EC;
    إذا (!Is_پہلا_ویلا)
      CurrentDirIter.increment(EC);
    إذا (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(Is_پہلا_ویلا);
    أرجع EC;
  }

  std::error_کوڈ incrementImpl(منطقي Is_پہلا_ویلا) {
    طالما (سچ) {
      std::error_کوڈ EC = incrementDirIter(Is_پہلا_ویلا);
      إذا (EC || CurrentDirIter == directory_iterator()) {
        Current_اندراج = directory_اندراج();
        أرجع EC;
      }
      Current_اندراج = *CurrentDirIter;
      StringRef Name = llvm::sys::رستہ_2::filename(Current_اندراج.رستہ_2());
      إذا (تکیا_ناں.داخل_کرو(Name).دوجا_2)
        أرجع EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

عام:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_کوڈ &EC) {
    لكل (ثابت خودکار &FS : FileSystems) {
      std::error_کوڈ FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      إذا (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        أرجع;
      }
      إذا (!FEC)
        IterList.ڈالو_پٹھ(Iter);
    }
    EC = incrementImpl(سچ);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_کوڈ &EC)
      : IterList(DirIters) {
    EC = incrementImpl(سچ);
  }

  std::error_کوڈ increment() override { أرجع incrementImpl(خطأ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ثابت Twine &Dir,
                                                std::error_کوڈ &EC) {
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

تعداد وچ_حافظہ_گرہ_قسم {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
صنف وچ_حافظہ_گرہ {
  وچ_حافظہ_گرہ_قسم قسم_2;
  std::string FileName;

عام:
  وچ_حافظہ_گرہ(llvm::StringRef FileName, وچ_حافظہ_گرہ_قسم قسم_2)
      : قسم_2(قسم_2), FileName(std::string(llvm::sys::رستہ_2::filename(FileName))) {
  }
  مجازی ~وچ_حافظہ_گرہ() = افتراضي;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  مجازی حالت get_حالت(ثابت Twine &RequestedName) ثابت = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ثابت { أرجع FileName; }
  وچ_حافظہ_گرہ_قسم get_قسم() ثابت { أرجع قسم_2; }
  مجازی std::string منزل_String(بنانشانی Indent) ثابت = 0;
};

صنف وچ_حافظہ_File : عام وچ_حافظہ_گرہ {
  حالت Stat;
  std::unique_ptr<llvm::حافظہ_بفر> بفر;

عام:
  وچ_حافظہ_File(حالت Stat, std::unique_ptr<llvm::حافظہ_بفر> بفر)
      : وچ_حافظہ_گرہ(Stat.getName(), IME_File), Stat(std::move(Stat)),
        بفر(std::move(بفر)) {}

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }
  llvm::حافظہ_بفر *get_بفر() ثابت { أرجع بفر.get(); }

  std::string منزل_String(بنانشانی Indent) ثابت override {
    أرجع (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ساكن منطقي classof(ثابت وچ_حافظہ_گرہ *N) {
    أرجع N->get_قسم() == IME_File;
  }
};

نطاق {

صنف وچ_حافظہ_Hard_Link : عام وچ_حافظہ_گرہ {
  ثابت وچ_حافظہ_File &ResolvedFile;

عام:
  وچ_حافظہ_Hard_Link(StringRef رستہ, ثابت وچ_حافظہ_File &ResolvedFile)
      : وچ_حافظہ_گرہ(رستہ, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ثابت وچ_حافظہ_File &getResolvedFile() ثابت { أرجع ResolvedFile; }

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع ResolvedFile.get_حالت(RequestedName);
  }

  std::string منزل_String(بنانشانی Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.منزل_String(0);
  }

  ساكن منطقي classof(ثابت وچ_حافظہ_گرہ *N) {
    أرجع N->get_قسم() == IME_HardLink;
  }
};

صنف وچ_حافظہ_Symbolic_Link : عام وچ_حافظہ_گرہ {
  std::string نشانہ_رستہ;
  حالت Stat;

عام:
  وچ_حافظہ_Symbolic_Link(StringRef رستہ, StringRef نشانہ_رستہ, حالت Stat)
      : وچ_حافظہ_گرہ(رستہ, IME_SymbolicLink), نشانہ_رستہ(std::move(نشانہ_رستہ)),
        Stat(Stat) {}

  std::string منزل_String(بنانشانی Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "SymbolicLink to -> " + نشانہ_رستہ;
  }

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_نشانہ_رستہ() ثابت { أرجع نشانہ_رستہ; }

  ساكن منطقي classof(ثابت وچ_حافظہ_گرہ *N) {
    أرجع N->get_قسم() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
صنف وچ_حافظہ_File_Adaptor : عام File {
  ثابت وچ_حافظہ_File &گرہ;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

عام:
  واضح وچ_حافظہ_File_Adaptor(ثابت وچ_حافظہ_File &گرہ,
                               std::string RequestedName)
      : گرہ(گرہ), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<حالت> حالت_2() override {
    أرجع گرہ.get_حالت(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::حافظہ_بفر>>
  get_بفر(ثابت Twine &Name, int64_t File_ناپ, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    llvm::حافظہ_بفر *Buf = گرہ.get_بفر();
    أرجع llvm::حافظہ_بفر::get_Mem_بفر(
        Buf->get_بفر(), Buf->get_بفر_Identifier(), RequiresNullTerminator);
  }

  std::error_کوڈ close() override { أرجع {}; }

  فراغ سیٹ_رستہ(ثابت Twine &رستہ) override { RequestedName = رستہ.str(); }
};
} // namespace

صنف وچ_حافظہ_Directory : عام وچ_حافظہ_گرہ {
  حالت Stat;
  std::map<std::string, std::unique_ptr<وچ_حافظہ_گرہ>, std::less<>> Entries;

عام:
  وچ_حافظہ_Directory(حالت Stat)
      : وچ_حافظہ_گرہ(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ثابت { أرجع Stat.getUniqueID(); }

  وچ_حافظہ_گرہ *get_بچہ(StringRef Name) ثابت {
    خودکار I = Entries.لبھو(Name);
    إذا (I != Entries.سرا())
      أرجع I->دوجا_2.get();
    أرجع خالی_سوچک;
  }

  وچ_حافظہ_گرہ *جوڑو_بچہ(StringRef Name, std::unique_ptr<وچ_حافظہ_گرہ> بچہ) {
    أرجع Entries.emplace(Name, std::move(بچہ)).پہلا_2->دوجا_2.get();
  }

  ورتوں const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ثابت { أرجع Entries.begin(); }
  const_iterator سرا() ثابت { أرجع Entries.سرا(); }

  std::string منزل_String(بنانشانی Indent) ثابت override {
    std::string نتیجہ =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    لكل (ثابت خودکار &اندراج : Entries)
      نتیجہ += اندراج.دوجا_2->منزل_String(Indent + 2);
    أرجع نتیجہ;
  }

  ساكن منطقي classof(ثابت وچ_حافظہ_گرہ *N) {
    أرجع N->get_قسم() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ساكن sys::fs::UniqueID getUniqueID(hash_کوڈ Hash) {
  أرجع sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ساكن sys::fs::UniqueID getFileID(sys::fs::UniqueID ماپا_2,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  أرجع getUniqueID(llvm::hash_combine(ماپا_2.getFile(), Name, Contents));
}
ساكن sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID ماپا_2,
                                        llvm::StringRef Name) {
  أرجع getUniqueID(llvm::hash_combine(ماپا_2.getFile(), Name));
}

حالت detail::New_وچ_حافظہ_گرہ_Info::make_حالت() ثابت {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, بفر ? بفر->get_بفر() : "");

  أرجع حالت(رستہ, UID, llvm::sys::منزل_ویلا_نقطہ(Modification_ویلا), User,
                گروہ, بفر ? بفر->get_بفر_ناپ() : 0, Type, Perms);
}

وچ_حافظہ_File_System::وچ_حافظہ_File_System(منطقي UseNormalizedPaths)
    : جڑ_2(جديد detail::وچ_حافظہ_Directory(
          حالت("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::ویلا_نقطہ<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

وچ_حافظہ_File_System::~وچ_حافظہ_File_System() = افتراضي;

std::string وچ_حافظہ_File_System::منزل_String() ثابت {
  أرجع جڑ_2->منزل_String(/*Indent=*/0);
}

منطقي وچ_حافظہ_File_System::جوڑو_File(ثابت Twine &P, time_t Modification_ویلا,
                                 std::unique_ptr<llvm::حافظہ_بفر> بفر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> گروہ,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_گرہ_Fn Make_گرہ) {
  SmallString<128> رستہ;
  P.منزل_سمتیہ(رستہ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کوڈ EC = makeAbsolute(رستہ);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::رستہ_2::remove_dots(رستہ, /*remove_dot_dot=*/سچ);

  إذا (رستہ.خالی_تھاواں())
    أرجع خطأ;

  detail::وچ_حافظہ_Directory *Dir = جڑ_2.get();
  خودکار I = llvm::sys::رستہ_2::begin(رستہ), E = sys::رستہ_2::سرا(رستہ);
  ثابت خودکار ResolvedUser = User.قدر_or(0);
  ثابت خودکار Resolved_گروہ = گروہ.قدر_or(0);
  ثابت خودکار ResolvedType = Type.قدر_or(sys::fs::file_type::regular_file);
  ثابت خودکار ResolvedPerms = Perms.قدر_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ثابت خودکار NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  طالما (سچ) {
    Name = *I;
    ++I;
    إذا (I == E)
      اكسر;
    detail::وچ_حافظہ_گرہ *گرہ = Dir->get_بچہ(Name);
    إذا (!گرہ) {
      // This isn't the last element, so we create a new directory.
      حالت Stat(
          StringRef(رستہ.str().begin(), Name.سرا() - رستہ.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::منزل_ویلا_نقطہ(Modification_ویلا), ResolvedUser, Resolved_گروہ,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::وچ_حافظہ_Directory>(Dir->جوڑو_بچہ(
          Name, std::make_unique<detail::وچ_حافظہ_Directory>(std::move(Stat))));
      استمر;
    }
    // Creating file under another file.
    إذا (!isa<detail::وچ_حافظہ_Directory>(گرہ))
      أرجع خطأ;
    Dir = cast<detail::وچ_حافظہ_Directory>(گرہ);
  }
  detail::وچ_حافظہ_گرہ *گرہ = Dir->get_بچہ(Name);
  إذا (!گرہ) {
    Dir->جوڑو_بچہ(Name,
                  Make_گرہ({Dir->getUniqueID(), رستہ, Name, Modification_ویلا,
                            std::move(بفر), ResolvedUser, Resolved_گروہ,
                            ResolvedType, ResolvedPerms}));
    أرجع سچ;
  }
  إذا (isa<detail::وچ_حافظہ_Directory>(گرہ))
    أرجع ResolvedType == sys::fs::file_type::directory_file;

  أكد((isa<detail::وچ_حافظہ_File>(گرہ) ||
          isa<detail::وچ_حافظہ_Hard_Link>(گرہ)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  إذا (خودکار *Link = dyn_cast<detail::وچ_حافظہ_Hard_Link>(گرہ)) {
    أرجع Link->getResolvedFile().get_بفر()->get_بفر() ==
           بفر->get_بفر();
  }
  أرجع cast<detail::وچ_حافظہ_File>(گرہ)->get_بفر()->get_بفر() ==
         بفر->get_بفر();
}

منطقي وچ_حافظہ_File_System::جوڑو_File(ثابت Twine &P, time_t Modification_ویلا,
                                 std::unique_ptr<llvm::حافظہ_بفر> بفر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> گروہ,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  أرجع جوڑو_File(P, Modification_ویلا, std::move(بفر), User, گروہ, Type,
                 Perms,
                 [](detail::New_وچ_حافظہ_گرہ_Info NNI)
                     -> std::unique_ptr<detail::وچ_حافظہ_گرہ> {
                   حالت Stat = NNI.make_حالت();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::وچ_حافظہ_Directory>(Stat);
                   أرجع std::make_unique<detail::وچ_حافظہ_File>(
                       Stat, std::move(NNI.بفر));
                 });
}

منطقي وچ_حافظہ_File_System::جوڑو_File_No_Own(
    ثابت Twine &P, time_t Modification_ویلا,
    ثابت llvm::حافظہ_بفر_Ref &بفر, std::optional<uint32_t> User,
    std::optional<uint32_t> گروہ, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  أرجع جوڑو_File(P, Modification_ویلا, llvm::حافظہ_بفر::get_Mem_بفر(بفر),
                 std::move(User), std::move(گروہ), std::move(Type),
                 std::move(Perms),
                 [](detail::New_وچ_حافظہ_گرہ_Info NNI)
                     -> std::unique_ptr<detail::وچ_حافظہ_گرہ> {
                   حالت Stat = NNI.make_حالت();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::وچ_حافظہ_Directory>(Stat);
                   أرجع std::make_unique<detail::وچ_حافظہ_File>(
                       Stat, std::move(NNI.بفر));
                 });
}

detail::Named_گرہ_Or_Error
وچ_حافظہ_File_System::lookup_گرہ(ثابت Twine &P, منطقي FollowFinalSymlink,
                               size_t Symlink_ڈونگھائی) ثابت {
  SmallString<128> رستہ;
  P.منزل_سمتیہ(رستہ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کوڈ EC = makeAbsolute(رستہ);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::رستہ_2::remove_dots(رستہ, /*remove_dot_dot=*/سچ);

  ثابت detail::وچ_حافظہ_Directory *Dir = جڑ_2.get();
  إذا (رستہ.خالی_تھاواں())
    أرجع detail::Named_گرہ_Or_Error(رستہ, Dir);

  خودکار I = llvm::sys::رستہ_2::begin(رستہ), E = llvm::sys::رستہ_2::سرا(رستہ);
  طالما (سچ) {
    detail::وچ_حافظہ_گرہ *گرہ = Dir->get_بچہ(*I);
    ++I;
    إذا (!گرہ)
      أرجع errc::no_such_file_or_directory;

    إذا (خودکار Symlink = dyn_cast<detail::وچ_حافظہ_Symbolic_Link>(گرہ)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      إذا (I == E && !FollowFinalSymlink)
        أرجع detail::Named_گرہ_Or_Error(رستہ, Symlink);

      إذا (Symlink_ڈونگھائی > وچ_حافظہ_File_System::Max_Symlink_ڈونگھائی)
        أرجع errc::no_such_file_or_directory;

      SmallString<128> نشانہ_رستہ = Symlink->get_نشانہ_رستہ();
      إذا (std::error_کوڈ EC = makeAbsolute(نشانہ_رستہ))
        أرجع EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      خودکار نشانہ_2 =
          lookup_گرہ(نشانہ_رستہ, /*FollowFinalSymlink=*/سچ, Symlink_ڈونگھائی + 1);
      إذا (!نشانہ_2 || I == E)
        أرجع نشانہ_2;

      إذا (!isa<detail::وچ_حافظہ_Directory>(*نشانہ_2))
        أرجع errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::وچ_حافظہ_Directory>(*نشانہ_2);
      استمر;
    }

    // Return the file if it's at the end of the path.
    إذا (خودکار File = dyn_cast<detail::وچ_حافظہ_File>(گرہ)) {
      إذا (I == E)
        أرجع detail::Named_گرہ_Or_Error(رستہ, File);
      أرجع errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    إذا (خودکار File = dyn_cast<detail::وچ_حافظہ_Hard_Link>(گرہ)) {
      إذا (I == E)
        أرجع detail::Named_گرہ_Or_Error(رستہ, &File->getResolvedFile());
      أرجع errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::وچ_حافظہ_Directory>(گرہ);
    إذا (I == E)
      أرجع detail::Named_گرہ_Or_Error(رستہ, Dir);
  }
}

منطقي وچ_حافظہ_File_System::جوڑو_Hard_Link(ثابت Twine &NewLink,
                                     ثابت Twine &نشانہ_2) {
  خودکار New_Link_گرہ = lookup_گرہ(NewLink, /*FollowFinalSymlink=*/خطأ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  خودکار نشانہ_گرہ = lookup_گرہ(نشانہ_2, /*FollowFinalSymlink=*/سچ);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  إذا (!نشانہ_گرہ || New_Link_گرہ || !isa<detail::وچ_حافظہ_File>(*نشانہ_گرہ))
    أرجع خطأ;
  أرجع جوڑو_File(NewLink, 0, خالی_سوچک, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_وچ_حافظہ_گرہ_Info NNI) {
                   أرجع std::make_unique<detail::وچ_حافظہ_Hard_Link>(
                       NNI.رستہ.str(),
                       *cast<detail::وچ_حافظہ_File>(*نشانہ_گرہ));
                 });
}

منطقي وچ_حافظہ_File_System::جوڑو_Symbolic_Link(
    ثابت Twine &NewLink, ثابت Twine &نشانہ_2, time_t Modification_ویلا,
    std::optional<uint32_t> User, std::optional<uint32_t> گروہ,
    std::optional<llvm::sys::fs::perms> Perms) {
  خودکار New_Link_گرہ = lookup_گرہ(NewLink, /*FollowFinalSymlink=*/خطأ);
  إذا (New_Link_گرہ)
    أرجع خطأ;

  SmallString<128> NewLinkStr, نشانہ_Str;
  NewLink.منزل_سمتیہ(NewLinkStr);
  نشانہ_2.منزل_سمتیہ(نشانہ_Str);

  أرجع جوڑو_File(NewLinkStr, Modification_ویلا, خالی_سوچک, User, گروہ,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_وچ_حافظہ_گرہ_Info NNI) {
                   أرجع std::make_unique<detail::وچ_حافظہ_Symbolic_Link>(
                       NewLinkStr, نشانہ_Str, NNI.make_حالت());
                 });
}

llvm::ErrorOr<حالت> وچ_حافظہ_File_System::حالت_2(ثابت Twine &رستہ) {
  خودکار گرہ = lookup_گرہ(رستہ, /*FollowFinalSymlink=*/سچ);
  إذا (گرہ)
    أرجع (*گرہ)->get_حالت(رستہ);
  أرجع گرہ.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
وچ_حافظہ_File_System::openFileForRead(ثابت Twine &رستہ) {
  خودکار گرہ = lookup_گرہ(رستہ,/*FollowFinalSymlink=*/سچ);
  إذا (!گرہ)
    أرجع گرہ.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  إذا (خودکار *F = dyn_cast<detail::وچ_حافظہ_File>(*گرہ))
    أرجع std::unique_ptr<File>(
        جديد detail::وچ_حافظہ_File_Adaptor(*F, رستہ.str()));

  // FIXME: errc::not_a_file?
  أرجع make_error_کوڈ(llvm::errc::invalid_دلیل);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
صنف وچ_حافظہ_File_System::DirIterator : عام llvm::vfs::detail::DirIterImpl {
  ثابت وچ_حافظہ_File_System *FS;
  detail::وچ_حافظہ_Directory::const_iterator I;
  detail::وچ_حافظہ_Directory::const_iterator E;
  std::string RequestedDirName;

  فراغ سیٹ_Current_اندراج() {
    إذا (I != E) {
      SmallString<256> رستہ(RequestedDirName);
      llvm::sys::رستہ_2::append(رستہ, I->دوجا_2->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل (I->دوجا_2->get_قسم()) {
      حالة detail::IME_File:
      حالة detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      حالة detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة detail::IME_SymbolicLink:
        إذا (خودکار Symlink_نشانہ =
                FS->lookup_گرہ(رستہ, /*FollowFinalSymlink=*/سچ)) {
          رستہ = Symlink_نشانہ.getName();
          Type = (*Symlink_نشانہ)->get_حالت(رستہ).getType();
        }
        اكسر;
      }
      Current_اندراج = directory_اندراج(std::string(رستہ), Type);
    } وإلا {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_اندراج = directory_اندراج();
    }
  }

عام:
  DirIterator() = افتراضي;

  DirIterator(ثابت وچ_حافظہ_File_System *FS,
              ثابت detail::وچ_حافظہ_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.سرا()),
        RequestedDirName(std::move(RequestedDirName)) {
    سیٹ_Current_اندراج();
  }

  std::error_کوڈ increment() override {
    ++I;
    سیٹ_Current_اندراج();
    أرجع {};
  }
};

directory_iterator وچ_حافظہ_File_System::dir_begin(ثابت Twine &Dir,
                                                 std::error_کوڈ &EC) {
  خودکار گرہ = lookup_گرہ(Dir, /*FollowFinalSymlink=*/سچ);
  إذا (!گرہ) {
    EC = گرہ.getError();
    أرجع directory_iterator(std::make_shared<DirIterator>());
  }

  إذا (خودکار *Dir_گرہ = dyn_cast<detail::وچ_حافظہ_Directory>(*گرہ))
    أرجع directory_iterator(
        std::make_shared<DirIterator>(هذا, *Dir_گرہ, Dir.str()));

  EC = make_error_کوڈ(llvm::errc::not_a_directory);
  أرجع directory_iterator(std::make_shared<DirIterator>());
}

std::error_کوڈ وچ_حافظہ_File_System::سیٹ_Current_Working_Directory(ثابت Twine &P) {
  SmallString<128> رستہ;
  P.منزل_سمتیہ(رستہ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_کوڈ EC = makeAbsolute(رستہ);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::رستہ_2::remove_dots(رستہ, /*remove_dot_dot=*/سچ);

  إذا (!رستہ.خالی_تھاواں())
    WorkingDirectory = std::string(رستہ);
  أرجع {};
}

std::error_کوڈ وچ_حافظہ_File_System::get_Real_رستہ(ثابت Twine &رستہ,
                                                Small_سمتیہ_Impl<حرف> &Output) {
  خودکار CWD = getCurrentWorkingDirectory();
  إذا (!CWD || CWD->خالی_تھاواں())
    أرجع errc::operation_not_permitted;
  رستہ.منزل_سمتیہ(Output);
  إذا (خودکار EC = makeAbsolute(Output))
    أرجع EC;
  llvm::sys::رستہ_2::remove_dots(Output, /*remove_dot_dot=*/سچ);
  أرجع {};
}

std::error_کوڈ وچ_حافظہ_File_System::isLocal(ثابت Twine &رستہ, منطقي &نتیجہ) {
  نتیجہ = خطأ;
  أرجع {};
}

فراغ وچ_حافظہ_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   بنانشانی IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

ساكن llvm::sys::رستہ_2::Style getExistingStyle(llvm::StringRef رستہ) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::رستہ_2::Style style = llvm::sys::رستہ_2::Style::native;
  ثابت size_t n = رستہ.لبھو_پہلا_دا("/\\");
  // Can't distinguish between posix and windows_slash here.
  إذا (n != static_cast<size_t>(-1))
    style = (رستہ[n] == '/') ? llvm::sys::رستہ_2::Style::posix
                             : llvm::sys::رستہ_2::Style::windows_backslash;
  أرجع style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ساكن llvm::SmallString<256> canonicalize(llvm::StringRef رستہ) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::رستہ_2::Style style = getExistingStyle(رستہ);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> نتیجہ_2 =
      llvm::sys::رستہ_2::remove_leading_dotslash(رستہ, style);
  llvm::sys::رستہ_2::remove_dots(نتیجہ_2, /*remove_dot_dot=*/سچ, style);
  أرجع نتیجہ_2;
}

/// Whether the error and entry specify a file/directory that was not found.
ساكن منطقي is_File_Not_لبھیا(std::error_کوڈ EC,
                           RedirectingFileSystem::اندراج *E = خالی_سوچک) {
  إذا (E && !isa<RedirectingFileSystem::Directory_Remap_اندراج>(E))
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
  RedirectingFileSystem::Directory_اندراج::iterator Current, سرا_2;

  std::error_کوڈ incrementImpl(منطقي Is_پہلا_ویلا) {
    أكد((Is_پہلا_ویلا || Current != سرا_2) && "cannot iterate past end");
    إذا (!Is_پہلا_ویلا)
      ++Current;
    إذا (Current != سرا_2) {
      SmallString<128> رستہ_Str(Dir);
      llvm::sys::رستہ_2::append(رستہ_Str, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل ((*Current)->get_قسم()) {
      حالة RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      حالة RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      }
      Current_اندراج = directory_اندراج(std::string(رستہ_Str), Type);
    } وإلا {
      Current_اندراج = directory_اندراج();
    }
    أرجع {};
  };

عام:
  RedirectingFSDirIterImpl(
      ثابت Twine &رستہ, RedirectingFileSystem::Directory_اندراج::iterator Begin,
      RedirectingFileSystem::Directory_اندراج::iterator سرا_2, std::error_کوڈ &EC)
      : Dir(رستہ.str()), Current(Begin), سرا_2(سرا_2) {
    EC = incrementImpl(/*IsFirstTime=*/سچ);
  }

  std::error_کوڈ increment() override {
    أرجع incrementImpl(/*IsFirstTime=*/خطأ);
  }
};

نطاق {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
صنف RedirectingFSDirRemapIterImpl : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::رستہ_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

عام:
  RedirectingFSDirRemapIterImpl(std::string Dir_رستہ,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_رستہ)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    إذا (ExternalIter != llvm::vfs::directory_iterator())
      سیٹ_Current_اندراج();
  }

  فراغ سیٹ_Current_اندراج() {
    StringRef External_رستہ = ExternalIter->رستہ_2();
    llvm::sys::رستہ_2::Style ExternalStyle = getExistingStyle(External_رستہ);
    StringRef File = llvm::sys::رستہ_2::filename(External_رستہ, ExternalStyle);

    SmallString<128> New_رستہ(Dir);
    llvm::sys::رستہ_2::append(New_رستہ, DirStyle, File);

    Current_اندراج = directory_اندراج(std::string(New_رستہ), ExternalIter->type());
  }

  std::error_کوڈ increment() override {
    std::error_کوڈ EC;
    ExternalIter.increment(EC);
    إذا (!EC && ExternalIter != llvm::vfs::directory_iterator())
      سیٹ_Current_اندراج();
    وإلا
      Current_اندراج = directory_اندراج();
    أرجع EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ثابت {
  أرجع WorkingDirectory;
}

std::error_کوڈ
RedirectingFileSystem::سیٹ_Current_Working_Directory(ثابت Twine &رستہ) {
  // Don't change the working directory if the path doesn't exist.
  إذا (!exists(رستہ))
    أرجع errc::no_such_file_or_directory;

  SmallString<128> Absolute_رستہ;
  رستہ.منزل_سمتیہ(Absolute_رستہ);
  إذا (std::error_کوڈ EC = makeAbsolute(Absolute_رستہ))
    أرجع EC;
  WorkingDirectory = std::string(Absolute_رستہ);
  أرجع {};
}

std::error_کوڈ RedirectingFileSystem::isLocal(ثابت Twine &رستہ_3,
                                               منطقي &نتیجہ) {
  SmallString<256> رستہ;
  رستہ_3.منزل_سمتیہ(رستہ);

  إذا (makeAbsolute(رستہ))
    أرجع {};

  أرجع ExternalFS->isLocal(رستہ, نتیجہ);
}

std::error_کوڈ RedirectingFileSystem::makeAbsolute(Small_سمتیہ_Impl<حرف> &رستہ) ثابت {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  إذا (llvm::sys::رستہ_2::is_absolute(رستہ, llvm::sys::رستہ_2::Style::posix) ||
      llvm::sys::رستہ_2::is_absolute(رستہ,
                                   llvm::sys::رستہ_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    أرجع {};

  خودکار WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  أرجع makeAbsolute(WorkingDir.get(), رستہ);
}

std::error_کوڈ
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_سمتیہ_Impl<حرف> &رستہ) ثابت {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  إذا (!WorkingDir.خالی_تھاواں() &&
      !sys::رستہ_2::is_absolute(WorkingDir, sys::رستہ_2::Style::posix) &&
      !sys::رستہ_2::is_absolute(WorkingDir,
                              sys::رستہ_2::Style::windows_backslash)) {
    أرجع std::error_کوڈ();
  }
  sys::رستہ_2::Style style = sys::رستہ_2::Style::windows_backslash;
  إذا (sys::رستہ_2::is_absolute(WorkingDir, sys::رستہ_2::Style::posix)) {
    style = sys::رستہ_2::Style::posix;
  } وإلا {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    إذا (getExistingStyle(WorkingDir) != sys::رستہ_2::Style::windows_backslash)
      style = sys::رستہ_2::Style::windows_slash;
  }

  std::string نتیجہ = std::string(WorkingDir);
  StringRef Dir(نتیجہ);
  إذا (!Dir.سرے_with(sys::رستہ_2::get_separator(style))) {
    نتیجہ += sys::رستہ_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  نتیجہ.append(رستہ.انکڑے(), رستہ.ناپ_2());
  رستہ.سونپو(نتیجہ.begin(), نتیجہ.سرا());

  أرجع {};
}

directory_iterator RedirectingFileSystem::dir_begin(ثابت Twine &Dir,
                                                    std::error_کوڈ &EC) {
  SmallString<256> رستہ;
  Dir.منزل_سمتیہ(رستہ);

  EC = makeAbsolute(رستہ);
  إذا (EC)
    أرجع {};

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_رستہ(رستہ);
  إذا (!نتیجہ) {
    إذا (Redirection != Redirect_قسم::RedirectOnly &&
        is_File_Not_لبھیا(نتیجہ.getError()))
      أرجع ExternalFS->dir_begin(رستہ, EC);

    EC = نتیجہ.getError();
    أرجع {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<حالت> S = حالت_2(رستہ, Dir, *نتیجہ);
  إذا (!S) {
    إذا (Redirection != Redirect_قسم::RedirectOnly &&
        is_File_Not_لبھیا(S.getError(), نتیجہ->E))
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
  std::error_کوڈ RedirectEC;
  إذا (خودکار ExtRedirect = نتیجہ->getExternalRedirect()) {
    خودکار RE = cast<RedirectingFileSystem::Remap_اندراج>(نتیجہ->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    إذا (!RE->useExternalName(Use_External_ناں)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(رستہ), RedirectIter));
    }
  } وإلا {
    خودکار DE = cast<Directory_اندراج>(نتیجہ->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            رستہ, DE->contents_begin(), DE->contents_سرا(), RedirectEC));
  }

  إذا (RedirectEC) {
    إذا (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      أرجع {};
    }
    RedirectIter = {};
  }

  إذا (Redirection == Redirect_قسم::RedirectOnly) {
    EC = RedirectEC;
    أرجع RedirectIter;
  }

  std::error_کوڈ ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(رستہ, ExternalEC);
  إذا (ExternalEC) {
    إذا (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      أرجع {};
    }
    ExternalIter = {};
  }

  Small_سمتیہ<directory_iterator, 2> Iters;
  بدل (Redirection) {
  حالة Redirect_قسم::Fallthrough:
    Iters.ڈالو_پٹھ(ExternalIter);
    Iters.ڈالو_پٹھ(RedirectIter);
    اكسر;
  حالة Redirect_قسم::Fallback:
    Iters.ڈالو_پٹھ(RedirectIter);
    Iters.ڈالو_پٹھ(ExternalIter);
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

فراغ RedirectingFileSystem::سیٹ_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ثابت {
  أرجع OverlayFileDir;
}

فراغ RedirectingFileSystem::سیٹ_Fallthrough(منطقي Fallthrough) {
  إذا (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_قسم::Fallthrough;
  } وإلا {
    Redirection = RedirectingFileSystem::Redirect_قسم::RedirectOnly;
  }
}

فراغ RedirectingFileSystem::سیٹ_Redirection(
    RedirectingFileSystem::Redirect_قسم قسم_2) {
  Redirection = قسم_2;
}

std::سمتیہ<StringRef> RedirectingFileSystem::getRoots() ثابت {
  std::سمتیہ<StringRef> R;
  R.reserve(Roots.ناپ_2());
  لكل (ثابت خودکار &جڑ_2 : Roots)
    R.ڈالو_پٹھ(جڑ_2->getName());
  أرجع R;
}

فراغ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      بنانشانی IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_ناں ? "true" : "false") << ")\n";
  إذا (Type == PrintType::Summary)
    أرجع;

  لكل (ثابت خودکار &جڑ_2 : Roots)
    print_اندراج(OS, جڑ_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

فراغ RedirectingFileSystem::print_اندراج(raw_ostream &OS,
                                       RedirectingFileSystem::اندراج *E,
                                       بنانشانی IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  بدل (E->get_قسم()) {
  حالة EK_Directory: {
    خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(E);

    OS << "\n";
    لكل (std::unique_ptr<اندراج> &Sub_اندراج :
         llvm::make_range(DE->contents_begin(), DE->contents_سرا()))
      print_اندراج(OS, Sub_اندراج.get(), IndentLevel + 1);
    اكسر;
  }
  حالة EK_DirectoryRemap:
  حالة EK_File: {
    خودکار *RE = cast<RedirectingFileSystem::Remap_اندراج>(E);
    OS << " -> '" << RE->get_External_Contents_رستہ() << "'";
    بدل (RE->getUseName()) {
    حالة NK_Not_سیٹ:
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

فراغ RedirectingFileSystem::ویکھو_بچہ_File_Systems(ویکھو_Callback_Ty Callback) {
  إذا (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->ویکھو_بچہ_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
صنف llvm::vfs::RedirectingFileSystemParser {
  yaml::دھارا &دھارا;

  فراغ error(yaml::گرہ *N, ثابت Twine &پیغام_2) { دھارا.printError(N, پیغام_2); }

  // false on error
  منطقي parseScalarString(yaml::گرہ *N, StringRef &نتیجہ,
                         Small_سمتیہ_Impl<حرف> &Storage) {
    ثابت خودکار *S = dyn_cast_if_present<yaml::Scalar_گرہ>(N);

    إذا (!S) {
      error(N, "expected string");
      أرجع خطأ;
    }
    نتیجہ = S->get_قدر(Storage);
    أرجع سچ;
  }

  // false on error
  منطقي parseScalarBool(yaml::گرہ *N, منطقي &نتیجہ) {
    SmallString<5> Storage;
    StringRef قدر;
    إذا (!parseScalarString(N, قدر, Storage))
      أرجع خطأ;

    إذا (قدر.equals_insensitive("true") || قدر.equals_insensitive("on") ||
        قدر.equals_insensitive("yes") || قدر == "1") {
      نتیجہ = سچ;
      أرجع سچ;
    } وإلا إذا (قدر.equals_insensitive("false") ||
               قدر.equals_insensitive("off") ||
               قدر.equals_insensitive("no") || قدر == "0") {
      نتیجہ = خطأ;
      أرجع سچ;
    }

    error(N, "expected boolean value");
    أرجع خطأ;
  }

  std::optional<RedirectingFileSystem::Redirect_قسم>
  parse_Redirect_قسم(yaml::گرہ *N) {
    SmallString<12> Storage;
    StringRef قدر;
    إذا (!parseScalarString(N, قدر, Storage))
      أرجع std::nullopt;

    إذا (قدر.equals_insensitive("fallthrough")) {
      أرجع RedirectingFileSystem::Redirect_قسم::Fallthrough;
    } وإلا إذا (قدر.equals_insensitive("fallback")) {
      أرجع RedirectingFileSystem::Redirect_قسم::Fallback;
    } وإلا إذا (قدر.equals_insensitive("redirect-only")) {
      أرجع RedirectingFileSystem::Redirect_قسم::RedirectOnly;
    }
    أرجع std::nullopt;
  }

  std::optional<RedirectingFileSystem::جڑ_Relative_قسم>
  parse_جڑ_Relative_قسم(yaml::گرہ *N) {
    SmallString<12> Storage;
    StringRef قدر;
    إذا (!parseScalarString(N, قدر, Storage))
      أرجع std::nullopt;
    إذا (قدر.equals_insensitive("cwd")) {
      أرجع RedirectingFileSystem::جڑ_Relative_قسم::CWD;
    } وإلا إذا (قدر.equals_insensitive("overlay-dir")) {
      أرجع RedirectingFileSystem::جڑ_Relative_قسم::OverlayDir;
    }
    أرجع std::nullopt;
  }

  بنية کلید_حالت {
    منطقي Required;
    منطقي تکیا = خطأ;

    کلید_حالت(منطقي Required = خطأ) : Required(Required) {}
  };

  ورتوں کلید_حالت_Pair = std::pair<StringRef, کلید_حالت>;

  // false on error
  منطقي check_Duplicate_Or_Unknown_کلید(yaml::گرہ *کلید_گرہ, StringRef کلید,
                                  DenseMap<StringRef, کلید_حالت> &کلیدیں_2) {
    خودکار It = کلیدیں_2.لبھو(کلید);
    إذا (It == کلیدیں_2.سرا()) {
      error(کلید_گرہ, "unknown key");
      أرجع خطأ;
    }
    کلید_حالت &S = It->دوجا_2;
    إذا (S.تکیا) {
      error(کلید_گرہ, Twine("duplicate key '") + کلید + "'");
      أرجع خطأ;
    }
    S.تکیا = سچ;
    أرجع سچ;
  }

  // false on error
  منطقي check_Missing_کلیدیں(yaml::گرہ *Obj, DenseMap<StringRef, کلید_حالت> &کلیدیں_2) {
    لكل (ثابت خودکار &I : کلیدیں_2) {
      إذا (I.دوجا_2.Required && !I.دوجا_2.تکیا) {
        error(Obj, Twine("missing key '") + I.پہلا_2 + "'");
        أرجع خطأ;
      }
    }
    أرجع سچ;
  }

عام:
  ساكن RedirectingFileSystem::اندراج *
  lookup_Or_Create_اندراج(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::اندراج *ماپا_اندراج = خالی_سوچک) {
    إذا (!ماپا_اندراج) { // Look for a existent root
      لكل (ثابت خودکار &جڑ_2 : FS->Roots) {
        إذا (Name == جڑ_2->getName()) {
          ماپا_اندراج = جڑ_2.get();
          أرجع ماپا_اندراج;
        }
      }
    } وإلا { // Advance to the next component
      خودکار *DE = dyn_cast<RedirectingFileSystem::Directory_اندراج>(ماپا_اندراج);
      لكل (std::unique_ptr<RedirectingFileSystem::اندراج> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_سرا())) {
        خودکار *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_اندراج>(Content.get());
        إذا (DirContent && Name == Content->getName())
          أرجع DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::اندراج> E =
        std::make_unique<RedirectingFileSystem::Directory_اندراج>(
            Name, حالت("", get_اگلا_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    إذا (!ماپا_اندراج) { // Add a new root to the overlay
      FS->Roots.ڈالو_پٹھ(std::move(E));
      ماپا_اندراج = FS->Roots.پٹھ().get();
      أرجع ماپا_اندراج;
    }

    خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(ماپا_اندراج);
    DE->جوڑو_Content(std::move(E));
    أرجع DE->get_آخری_Content();
  }

خاص:
  فراغ unique_Overlay_درخت(RedirectingFileSystem *FS,
                         RedirectingFileSystem::اندراج *SrcE,
                         RedirectingFileSystem::اندراج *New_ماپا_E = خالی_سوچک) {
    StringRef Name = SrcE->getName();
    بدل (SrcE->get_قسم()) {
    حالة RedirectingFileSystem::EK_Directory: {
      خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      إذا (!Name.خالی_تھاواں())
        New_ماپا_E = lookup_Or_Create_اندراج(FS, Name, New_ماپا_E);
      لكل (std::unique_ptr<RedirectingFileSystem::اندراج> &Sub_اندراج :
           llvm::make_range(DE->contents_begin(), DE->contents_سرا()))
        unique_Overlay_درخت(FS, Sub_اندراج.get(), New_ماپا_E);
      اكسر;
    }
    حالة RedirectingFileSystem::EK_DirectoryRemap: {
      أكد(New_ماپا_E && "Parent entry must exist");
      خودکار *DR = cast<RedirectingFileSystem::Directory_Remap_اندراج>(SrcE);
      خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(New_ماپا_E);
      DE->جوڑو_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_اندراج>(
              Name, DR->get_External_Contents_رستہ(), DR->getUseName()));
      اكسر;
    }
    حالة RedirectingFileSystem::EK_File: {
      أكد(New_ماپا_E && "Parent entry must exist");
      خودکار *FE = cast<RedirectingFileSystem::File_اندراج>(SrcE);
      خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(New_ماپا_E);
      DE->جوڑو_Content(std::make_unique<RedirectingFileSystem::File_اندراج>(
          Name, FE->get_External_Contents_رستہ(), FE->getUseName()));
      اكسر;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::اندراج>
  parse_اندراج(yaml::گرہ *N, RedirectingFileSystem *FS, منطقي Is_جڑ_اندراج) {
    خودکار *M = dyn_cast<yaml::Mapping_گرہ>(N);
    إذا (!M) {
      error(N, "expected mapping node for file or directory entry");
      أرجع خالی_سوچک;
    }

    کلید_حالت_Pair Fields[] = {
        کلید_حالت_Pair("name", سچ),
        کلید_حالت_Pair("type", سچ),
        کلید_حالت_Pair("contents", خطأ),
        کلید_حالت_Pair("external-contents", خطأ),
        کلید_حالت_Pair("use-external-name", خطأ),
    };

    DenseMap<StringRef, کلید_حالت> کلیدیں_2(std::begin(Fields), std::سرا(Fields));

    تعداد { CF_Not_سیٹ, CF_List, CF_External } Contents_میدان = CF_Not_سیٹ;
    std::سمتیہ<std::unique_ptr<RedirectingFileSystem::اندراج>>
        اندراج_Array_Contents;
    SmallString<256> External_Contents_رستہ;
    SmallString<256> Name;
    yaml::گرہ *Name_قدر_گرہ = خالی_سوچک;
    خودکار UseExternalName = RedirectingFileSystem::NK_Not_سیٹ;
    RedirectingFileSystem::اندراج_قسم قسم_2;

    لكل (خودکار &I : *M) {
      StringRef کلید;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> بفر;
      إذا (!parseScalarString(I.get_کلید(), کلید, بفر))
        أرجع خالی_سوچک;

      إذا (!check_Duplicate_Or_Unknown_کلید(I.get_کلید(), کلید, کلیدیں_2))
        أرجع خالی_سوچک;

      StringRef قدر;
      إذا (کلید == "name") {
        إذا (!parseScalarString(I.get_قدر(), قدر, بفر))
          أرجع خالی_سوچک;

        Name_قدر_گرہ = I.get_قدر();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(قدر).str();
      } وإلا إذا (کلید == "type") {
        إذا (!parseScalarString(I.get_قدر(), قدر, بفر))
          أرجع خالی_سوچک;
        إذا (قدر == "file")
          قسم_2 = RedirectingFileSystem::EK_File;
        وإلا إذا (قدر == "directory")
          قسم_2 = RedirectingFileSystem::EK_Directory;
        وإلا إذا (قدر == "directory-remap")
          قسم_2 = RedirectingFileSystem::EK_DirectoryRemap;
        وإلا {
          error(I.get_قدر(), "unknown value for 'type'");
          أرجع خالی_سوچک;
        }
      } وإلا إذا (کلید == "contents") {
        إذا (Contents_میدان != CF_Not_سیٹ) {
          error(I.get_کلید(),
                "entry already has 'contents' or 'external-contents'");
          أرجع خالی_سوچک;
        }
        Contents_میدان = CF_List;
        خودکار *Contents = dyn_cast_if_present<yaml::Sequence_گرہ>(I.get_قدر());
        إذا (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_قدر(), "expected array");
          أرجع خالی_سوچک;
        }

        لكل (خودکار &I : *Contents) {
          إذا (std::unique_ptr<RedirectingFileSystem::اندراج> E =
                  parse_اندراج(&I, FS, /*IsRootEntry*/ خطأ))
            اندراج_Array_Contents.ڈالو_پٹھ(std::move(E));
          وإلا
            أرجع خالی_سوچک;
        }
      } وإلا إذا (کلید == "external-contents") {
        إذا (Contents_میدان != CF_Not_سیٹ) {
          error(I.get_کلید(),
                "entry already has 'contents' or 'external-contents'");
          أرجع خالی_سوچک;
        }
        Contents_میدان = CF_External;
        إذا (!parseScalarString(I.get_قدر(), قدر, بفر))
          أرجع خالی_سوچک;

        SmallString<256> بھریا_رستہ;
        إذا (FS->IsRelativeOverlay) {
          بھریا_رستہ = FS->getOverlayFileDir();
          أكد(!بھریا_رستہ.خالی_تھاواں() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_بھریا_رستہ = قدر;
          إذا (FS->makeAbsolute(بھریا_رستہ, Abs_بھریا_رستہ)) {
            error(N, "failed to make 'external-contents' absolute");
            أرجع خالی_سوچک;
          }
          بھریا_رستہ = Abs_بھریا_رستہ;
        } وإلا {
          بھریا_رستہ = قدر;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        بھریا_رستہ = canonicalize(بھریا_رستہ);
        External_Contents_رستہ = بھریا_رستہ.str();
      } وإلا إذا (کلید == "use-external-name") {
        منطقي Val;
        إذا (!parseScalarBool(I.get_قدر(), Val))
          أرجع خالی_سوچک;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (دھارا.failed())
      أرجع خالی_سوچک;

    // check for missing keys
    إذا (Contents_میدان == CF_Not_سیٹ) {
      error(N, "missing key 'contents' or 'external-contents'");
      أرجع خالی_سوچک;
    }
    إذا (!check_Missing_کلیدیں(N, کلیدیں_2))
      أرجع خالی_سوچک;

    // check invalid configuration
    إذا (قسم_2 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_سیٹ) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      أرجع خالی_سوچک;
    }

    إذا (قسم_2 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_میدان == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      أرجع خالی_سوچک;
    }

    sys::رستہ_2::Style رستہ_style = sys::رستہ_2::Style::native;
    إذا (Is_جڑ_اندراج) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      إذا (sys::رستہ_2::is_absolute(Name, sys::رستہ_2::Style::posix)) {
        رستہ_style = sys::رستہ_2::Style::posix;
      } وإلا إذا (sys::رستہ_2::is_absolute(Name,
                                        sys::رستہ_2::Style::windows_backslash)) {
        رستہ_style = sys::رستہ_2::Style::windows_backslash;
      } وإلا {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_کوڈ EC;
        إذا (FS->جڑ_Relative ==
            RedirectingFileSystem::جڑ_Relative_قسم::OverlayDir) {
          StringRef بھریا_رستہ = FS->getOverlayFileDir();
          أكد(!بھریا_رستہ.خالی_تھاواں() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(بھریا_رستہ, Name);
          Name = canonicalize(Name);
        } وإلا {
          EC = FS->makeAbsolute(Name);
        }
        إذا (EC) {
          أكد(Name_قدر_گرہ && "Name presence should be checked earlier");
          error(
              Name_قدر_گرہ,
              "entry with relative path at the root level is not discoverable");
          أرجع خالی_سوچک;
        }
        رستہ_style = sys::رستہ_2::is_absolute(Name, sys::رستہ_2::Style::posix)
                         ? sys::رستہ_2::Style::posix
                         : sys::رستہ_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      إذا (رستہ_style == sys::رستہ_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::رستہ_2::Style::windows_backslash)
        رستہ_style = sys::رستہ_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t جڑ_رستہ_Len = sys::رستہ_2::جڑ_رستہ_2(Trimmed, رستہ_style).ناپ_2();
    طالما (Trimmed.ناپ_2() > جڑ_رستہ_Len &&
           sys::رستہ_2::is_separator(Trimmed.پٹھ(), رستہ_style))
      Trimmed = Trimmed.ٹکڑا(0, Trimmed.ناپ_2() - 1);

    // Get the last component
    StringRef آخری_Component = sys::رستہ_2::filename(Trimmed, رستہ_style);

    std::unique_ptr<RedirectingFileSystem::اندراج> نتیجہ;
    بدل (قسم_2) {
    حالة RedirectingFileSystem::EK_File:
      نتیجہ = std::make_unique<RedirectingFileSystem::File_اندراج>(
          آخری_Component, std::move(External_Contents_رستہ), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_DirectoryRemap:
      نتیجہ = std::make_unique<RedirectingFileSystem::Directory_Remap_اندراج>(
          آخری_Component, std::move(External_Contents_رستہ), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_Directory:
      نتیجہ = std::make_unique<RedirectingFileSystem::Directory_اندراج>(
          آخری_Component, std::move(اندراج_Array_Contents),
          حالت("", get_اگلا_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      اكسر;
    }

    StringRef ماپا_2 = sys::رستہ_2::ماپا_رستہ(Trimmed, رستہ_style);
    إذا (ماپا_2.خالی_تھاواں())
      أرجع نتیجہ;

    // if 'name' contains multiple components, create implicit directory entries
    لكل (sys::رستہ_2::الٹ_iterator I = sys::رستہ_2::rbegin(ماپا_2, رستہ_style),
                                     E = sys::رستہ_2::rend(ماپا_2);
         I != E; ++I) {
      std::سمتیہ<std::unique_ptr<RedirectingFileSystem::اندراج>> Entries;
      Entries.ڈالو_پٹھ(std::move(نتیجہ));
      نتیجہ = std::make_unique<RedirectingFileSystem::Directory_اندراج>(
          *I, std::move(Entries),
          حالت("", get_اگلا_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    أرجع نتیجہ;
  }

عام:
  RedirectingFileSystemParser(yaml::دھارا &S) : دھارا(S) {}

  // false on error
  منطقي parse(yaml::گرہ *جڑ_2, RedirectingFileSystem *FS) {
    خودکار *چوٹی = dyn_cast<yaml::Mapping_گرہ>(جڑ_2);
    إذا (!چوٹی) {
      error(جڑ_2, "expected mapping node");
      أرجع خطأ;
    }

    کلید_حالت_Pair Fields[] = {
        کلید_حالت_Pair("version", سچ),
        کلید_حالت_Pair("case-sensitive", خطأ),
        کلید_حالت_Pair("use-external-names", خطأ),
        کلید_حالت_Pair("root-relative", خطأ),
        کلید_حالت_Pair("overlay-relative", خطأ),
        کلید_حالت_Pair("fallthrough", خطأ),
        کلید_حالت_Pair("redirecting-with", خطأ),
        کلید_حالت_Pair("roots", سچ),
    };

    DenseMap<StringRef, کلید_حالت> کلیدیں_2(std::begin(Fields), std::سرا(Fields));
    std::سمتیہ<std::unique_ptr<RedirectingFileSystem::اندراج>> جڑ_Entries;

    // Parse configuration and 'roots'
    لكل (خودکار &I : *چوٹی) {
      SmallString<10> کلید_بفر;
      StringRef کلید;
      إذا (!parseScalarString(I.get_کلید(), کلید, کلید_بفر))
        أرجع خطأ;

      إذا (!check_Duplicate_Or_Unknown_کلید(I.get_کلید(), کلید, کلیدیں_2))
        أرجع خطأ;

      إذا (کلید == "roots") {
        خودکار *Roots = dyn_cast_if_present<yaml::Sequence_گرہ>(I.get_قدر());
        إذا (!Roots) {
          error(I.get_قدر(), "expected array");
          أرجع خطأ;
        }

        لكل (خودکار &I : *Roots) {
          إذا (std::unique_ptr<RedirectingFileSystem::اندراج> E =
                  parse_اندراج(&I, FS, /*IsRootEntry*/ سچ))
            جڑ_Entries.ڈالو_پٹھ(std::move(E));
          وإلا
            أرجع خطأ;
        }
      } وإلا إذا (کلید == "version") {
        StringRef نسخہ_String;
        SmallString<4> Storage;
        إذا (!parseScalarString(I.get_قدر(), نسخہ_String, Storage))
          أرجع خطأ;
        صحيح نسخہ;
        إذا (نسخہ_String.getAsInteger<صحيح>(10, نسخہ)) {
          error(I.get_قدر(), "expected integer");
          أرجع خطأ;
        }
        إذا (نسخہ < 0) {
          error(I.get_قدر(), "invalid version number");
          أرجع خطأ;
        }
        إذا (نسخہ != 0) {
          error(I.get_قدر(), "version mismatch, expected 0");
          أرجع خطأ;
        }
      } وإلا إذا (کلید == "case-sensitive") {
        إذا (!parseScalarBool(I.get_قدر(), FS->CaseSensitive))
          أرجع خطأ;
      } وإلا إذا (کلید == "overlay-relative") {
        إذا (!parseScalarBool(I.get_قدر(), FS->IsRelativeOverlay))
          أرجع خطأ;
      } وإلا إذا (کلید == "use-external-names") {
        إذا (!parseScalarBool(I.get_قدر(), FS->Use_External_ناں))
          أرجع خطأ;
      } وإلا إذا (کلید == "fallthrough") {
        إذا (کلیدیں_2["redirecting-with"].تکیا) {
          error(I.get_قدر(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        منطقي ShouldFallthrough = خطأ;
        إذا (!parseScalarBool(I.get_قدر(), ShouldFallthrough))
          أرجع خطأ;

        إذا (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::Redirect_قسم::Fallthrough;
        } وإلا {
          FS->Redirection = RedirectingFileSystem::Redirect_قسم::RedirectOnly;
        }
      } وإلا إذا (کلید == "redirecting-with") {
        إذا (کلیدیں_2["fallthrough"].تکیا) {
          error(I.get_قدر(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        إذا (خودکار قسم_2 = parse_Redirect_قسم(I.get_قدر())) {
          FS->Redirection = *قسم_2;
        } وإلا {
          error(I.get_قدر(), "expected valid redirect kind");
          أرجع خطأ;
        }
      } وإلا إذا (کلید == "root-relative") {
        إذا (خودکار قسم_2 = parse_جڑ_Relative_قسم(I.get_قدر())) {
          FS->جڑ_Relative = *قسم_2;
        } وإلا {
          error(I.get_قدر(), "expected valid root-relative kind");
          أرجع خطأ;
        }
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (دھارا.failed())
      أرجع خطأ;

    إذا (!check_Missing_کلیدیں(چوٹی, کلیدیں_2))
      أرجع خطأ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    لكل (خودکار &E : جڑ_Entries)
      unique_Overlay_درخت(FS, E.get());

    أرجع سچ;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<حافظہ_بفر> بفر,
                              سروت_Mgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_رستہ, فراغ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  سروت_Mgr SM;
  yaml::دھارا دھارا(بفر->get_Mem_بفر_Ref(), SM);

  SM.سیٹ_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = دھارا.begin();
  yaml::گرہ *جڑ_2 = DI->get_جڑ();
  إذا (DI == دھارا.سرا() || !جڑ_2) {
    SM.Print_سنیہا(SMLoc(), سروت_Mgr::DK_Error, "expected root node");
    أرجع خالی_سوچک;
  }

  RedirectingFileSystemParser P(دھارا);

  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));

  إذا (!YAMLFile_رستہ.خالی_تھاواں()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::رستہ_2::ماپا_رستہ(YAMLFile_رستہ);
    std::error_کوڈ EC = FS->makeAbsolute(OverlayAbsDir);
    أكد(!EC && "Overlay dir final path must be absolute");
    (فراغ)EC;
    FS->سیٹ_Overlay_File_Dir(OverlayAbsDir);
  }

  إذا (!P.parse(جڑ_2, FS.get()))
    أرجع خالی_سوچک;

  أرجع FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    منطقي Use_External_ناں, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));
  FS->Use_External_ناں = Use_External_ناں;

  StringMap<RedirectingFileSystem::اندراج *> Entries;

  لكل (خودکار &Mapping : llvm::الٹ(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.پہلا_2);
    SmallString<128> منزل = StringRef(Mapping.دوجا_2);
    {
      خودکار EC = ExternalFS->makeAbsolute(From);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::اندراج *&منزل_اندراج = Entries[From];
    إذا (منزل_اندراج)
      استمر;

    // Add parent directories.
    RedirectingFileSystem::اندراج *ماپا_2 = خالی_سوچک;
    StringRef FromDirectory = llvm::sys::رستہ_2::ماپا_رستہ(From);
    لكل (خودکار I = llvm::sys::رستہ_2::begin(FromDirectory),
              E = llvm::sys::رستہ_2::سرا(FromDirectory);
         I != E; ++I) {
      ماپا_2 = RedirectingFileSystemParser::lookup_Or_Create_اندراج(FS.get(), *I,
                                                                ماپا_2);
    }
    أكد(ماپا_2 && "File without a directory?");
    {
      خودکار EC = ExternalFS->makeAbsolute(منزل);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Add the file.
    خودکار NewFile = std::make_unique<RedirectingFileSystem::File_اندراج>(
        llvm::sys::رستہ_2::filename(From), منزل,
        Use_External_ناں ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    منزل_اندراج = NewFile.get();
    cast<RedirectingFileSystem::Directory_اندراج>(ماپا_2)->جوڑو_Content(
        std::move(NewFile));
  }

  أرجع FS;
}

RedirectingFileSystem::Lookup_نتیجہ::Lookup_نتیجہ(
    اندراج *E, sys::رستہ_2::const_iterator شروع, sys::رستہ_2::const_iterator سرا_2)
    : E(E) {
  أكد(E != خالی_سوچک);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  إذا (خودکار *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_اندراج>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_رستہ());
    sys::رستہ_2::append(Redirect, شروع, سرا_2,
                      getExistingStyle(DRE->get_External_Contents_رستہ()));
    ExternalRedirect = std::string(Redirect);
  }
}

فراغ RedirectingFileSystem::Lookup_نتیجہ::get_رستہ(
    llvm::Small_سمتیہ_Impl<حرف> &نتیجہ) ثابت {
  نتیجہ.clear();
  لكل (اندراج *ماپا_2 : Parents)
    llvm::sys::رستہ_2::append(نتیجہ, ماپا_2->getName());
  llvm::sys::رستہ_2::append(نتیجہ, E->getName());
}

std::error_کوڈ RedirectingFileSystem::makeCanonicalForLookup(
    Small_سمتیہ_Impl<حرف> &رستہ) ثابت {
  إذا (std::error_کوڈ EC = makeAbsolute(رستہ))
    أرجع EC;

  llvm::SmallString<256> Canonical_رستہ =
      canonicalize(StringRef(رستہ.انکڑے(), رستہ.ناپ_2()));
  إذا (Canonical_رستہ.خالی_تھاواں())
    أرجع make_error_کوڈ(llvm::errc::invalid_دلیل);

  رستہ.سونپو(Canonical_رستہ.begin(), Canonical_رستہ.سرا());
  أرجع {};
}

ErrorOr<RedirectingFileSystem::Lookup_نتیجہ>
RedirectingFileSystem::lookup_رستہ(StringRef رستہ) ثابت {
  llvm::SmallString<128> Canonical_رستہ(رستہ);
  إذا (std::error_کوڈ EC = makeCanonicalForLookup(Canonical_رستہ))
    أرجع EC;

  // RedirectOnly means the VFS is always used.
  إذا (Usage_Tracking_سرگرم && Redirection == Redirect_قسم::RedirectOnly)
    Has_Been_ورتیا = سچ;

  sys::رستہ_2::const_iterator شروع = sys::رستہ_2::begin(Canonical_رستہ);
  sys::رستہ_2::const_iterator سرا_2 = sys::رستہ_2::سرا(Canonical_رستہ);
  llvm::Small_سمتیہ<اندراج *, 32> Entries;
  لكل (ثابت خودکار &جڑ_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ =
        lookup_رستہ_Impl(شروع, سرا_2, جڑ_2.get(), Entries);
    إذا (Usage_Tracking_سرگرم && نتیجہ && isa<Remap_اندراج>(نتیجہ->E))
      Has_Been_ورتیا = سچ;
    إذا (نتیجہ) {
      نتیجہ->Parents = std::move(Entries);
      أرجع نتیجہ;
    }

    إذا (نتیجہ.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتیجہ;
  }
  أرجع make_error_کوڈ(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_نتیجہ>
RedirectingFileSystem::lookup_رستہ_Impl(
    sys::رستہ_2::const_iterator شروع, sys::رستہ_2::const_iterator سرا_2,
    RedirectingFileSystem::اندراج *From,
    llvm::Small_سمتیہ_Impl<اندراج *> &Entries) ثابت {
  أكد(!isTraversalComponent(*شروع) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  إذا (!FromName.خالی_تھاواں()) {
    إذا (!رستہ_Component_Matches(*شروع, FromName))
      أرجع make_error_کوڈ(llvm::errc::no_such_file_or_directory);

    ++شروع;

    إذا (شروع == سرا_2) {
      // Match!
      أرجع Lookup_نتیجہ(From, شروع, سرا_2);
    }
  }

  إذا (isa<RedirectingFileSystem::File_اندراج>(From))
    أرجع make_error_کوڈ(llvm::errc::not_a_directory);

  إذا (isa<RedirectingFileSystem::Directory_Remap_اندراج>(From))
    أرجع Lookup_نتیجہ(From, شروع, سرا_2);

  خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(From);
  لكل (ثابت std::unique_ptr<RedirectingFileSystem::اندراج> &Dir_اندراج :
       llvm::make_range(DE->contents_begin(), DE->contents_سرا())) {
    Entries.ڈالو_پٹھ(From);
    ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ =
        lookup_رستہ_Impl(شروع, سرا_2, Dir_اندراج.get(), Entries);
    إذا (نتیجہ || نتیجہ.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتیجہ;
    Entries.نکالو_پٹھ();
  }

  أرجع make_error_کوڈ(llvm::errc::no_such_file_or_directory);
}

ساكن حالت get_Redirected_File_حالت(ثابت Twine &Original_رستہ,
                                      منطقي Use_External_ناں,
                                      حالت External_حالت) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  إذا (External_حالت.ExposesExternalVFSPath)
    أرجع External_حالت;

  حالت S = External_حالت;
  إذا (!Use_External_ناں)
    S = حالت::copyWithNewName(S, Original_رستہ);
  وإلا
    S.ExposesExternalVFSPath = سچ;
  أرجع S;
}

ErrorOr<حالت> RedirectingFileSystem::حالت_2(
    ثابت Twine &Lookup_رستہ, ثابت Twine &Original_رستہ,
    ثابت RedirectingFileSystem::Lookup_نتیجہ &نتیجہ) {
  إذا (std::optional<StringRef> ExtRedirect = نتیجہ.getExternalRedirect()) {
    SmallString<256> Remapped_رستہ((*ExtRedirect).str());
    إذا (std::error_کوڈ EC = makeAbsolute(Remapped_رستہ))
      أرجع EC;

    ErrorOr<حالت> S = ExternalFS->حالت_2(Remapped_رستہ);
    إذا (!S)
      أرجع S;
    S = حالت::copyWithNewName(*S, *ExtRedirect);
    خودکار *RE = cast<RedirectingFileSystem::Remap_اندراج>(نتیجہ.E);
    أرجع get_Redirected_File_حالت(Original_رستہ,
                                   RE->useExternalName(Use_External_ناں), *S);
  }

  خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(نتیجہ.E);
  أرجع حالت::copyWithNewName(DE->get_حالت(), Lookup_رستہ);
}

ErrorOr<حالت>
RedirectingFileSystem::get_External_حالت(ثابت Twine &Lookup_رستہ,
                                         ثابت Twine &Original_رستہ) ثابت {
  خودکار نتیجہ = ExternalFS->حالت_2(Lookup_رستہ);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  إذا (!نتیجہ || نتیجہ->ExposesExternalVFSPath)
    أرجع نتیجہ;
  أرجع حالت::copyWithNewName(نتیجہ.get(), Original_رستہ);
}

ErrorOr<حالت> RedirectingFileSystem::حالت_2(ثابت Twine &Original_رستہ) {
  SmallString<256> رستہ;
  Original_رستہ.منزل_سمتیہ(رستہ);

  إذا (std::error_کوڈ EC = makeAbsolute(رستہ))
    أرجع EC;

  إذا (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<حالت> S = get_External_حالت(رستہ, Original_رستہ);
    إذا (S)
      أرجع S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_رستہ(رستہ);
  إذا (!نتیجہ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_لبھیا(نتیجہ.getError()))
      أرجع get_External_حالت(رستہ, Original_رستہ);
    أرجع نتیجہ.getError();
  }

  ErrorOr<حالت> S = حالت_2(رستہ, Original_رستہ, *نتیجہ);
  إذا (!S && Redirection == Redirect_قسم::Fallthrough &&
      is_File_Not_لبھیا(S.getError(), نتیجہ->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع get_External_حالت(رستہ, Original_رستہ);
  }

  أرجع S;
}

منطقي RedirectingFileSystem::exists(ثابت Twine &Original_رستہ) {
  SmallString<256> رستہ;
  Original_رستہ.منزل_سمتیہ(رستہ);

  إذا (makeAbsolute(رستہ))
    أرجع خطأ;

  إذا (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    إذا (ExternalFS->exists(رستہ))
      أرجع سچ;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_رستہ(رستہ);
  إذا (!نتیجہ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_لبھیا(نتیجہ.getError()))
      أرجع ExternalFS->exists(رستہ);
    أرجع خطأ;
  }

  std::optional<StringRef> ExtRedirect = نتیجہ->getExternalRedirect();
  إذا (!ExtRedirect) {
    أكد(isa<RedirectingFileSystem::Directory_اندراج>(نتیجہ->E));
    أرجع سچ;
  }

  SmallString<256> Remapped_رستہ((*ExtRedirect).str());
  إذا (makeAbsolute(Remapped_رستہ))
    أرجع خطأ;

  إذا (ExternalFS->exists(Remapped_رستہ))
    أرجع سچ;

  إذا (Redirection == Redirect_قسم::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع ExternalFS->exists(رستہ);
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
  ErrorOr<std::unique_ptr<llvm::حافظہ_بفر>>

  get_بفر(ثابت Twine &Name, int64_t File_ناپ, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    أرجع InnerFile->get_بفر(Name, File_ناپ, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_کوڈ close() override { أرجع InnerFile->close(); }

  فراغ سیٹ_رستہ(ثابت Twine &رستہ) override { S = S.copyWithNewName(S, رستہ); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_رستہ(ErrorOr<std::unique_ptr<File>> نتیجہ, ثابت Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  إذا (!نتیجہ || (*نتیجہ)->حالت_2()->ExposesExternalVFSPath)
    أرجع نتیجہ;

  ErrorOr<std::unique_ptr<File>> F = std::move(*نتیجہ);
  خودکار Name = F->get()->getName();
  إذا (Name && Name.get() != P.str())
    F->get()->سیٹ_رستہ(P);
  أرجع F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ثابت Twine &Original_رستہ) {
  SmallString<256> رستہ;
  Original_رستہ.منزل_سمتیہ(رستہ);

  إذا (std::error_کوڈ EC = makeAbsolute(رستہ))
    أرجع EC;

  إذا (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    خودکار F = File::get_With_رستہ(ExternalFS->openFileForRead(رستہ), Original_رستہ);
    إذا (F)
      أرجع F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_رستہ(رستہ);
  إذا (!نتیجہ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_لبھیا(نتیجہ.getError()))
      أرجع File::get_With_رستہ(ExternalFS->openFileForRead(رستہ), Original_رستہ);
    أرجع نتیجہ.getError();
  }

  إذا (!نتیجہ->getExternalRedirect()) // FIXME: errc::not_a_file?
    أرجع make_error_کوڈ(llvm::errc::invalid_دلیل);

  StringRef ExtRedirect = *نتیجہ->getExternalRedirect();
  SmallString<256> Remapped_رستہ(ExtRedirect.str());
  إذا (std::error_کوڈ EC = makeAbsolute(Remapped_رستہ))
    أرجع EC;

  خودکار *RE = cast<RedirectingFileSystem::Remap_اندراج>(نتیجہ->E);

  خودکار ExternalFile =
      File::get_With_رستہ(ExternalFS->openFileForRead(Remapped_رستہ), ExtRedirect);
  إذا (!ExternalFile) {
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_لبھیا(ExternalFile.getError(), نتیجہ->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع File::get_With_رستہ(ExternalFS->openFileForRead(رستہ), Original_رستہ);
    }
    أرجع ExternalFile;
  }

  خودکار External_حالت = (*ExternalFile)->حالت_2();
  إذا (!External_حالت)
    أرجع External_حالت.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  حالت S = get_Redirected_File_حالت(
      Original_رستہ, RE->useExternalName(Use_External_ناں), *External_حالت);
  أرجع std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_حالت>(std::move(*ExternalFile), S));
}

std::error_کوڈ
RedirectingFileSystem::get_Real_رستہ(ثابت Twine &Original_رستہ,
                                   Small_سمتیہ_Impl<حرف> &Output) {
  SmallString<256> رستہ;
  Original_رستہ.منزل_سمتیہ(رستہ);

  إذا (std::error_کوڈ EC = makeAbsolute(رستہ))
    أرجع EC;

  إذا (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_کوڈ EC = ExternalFS->get_Real_رستہ(رستہ, Output);
    إذا (!EC)
      أرجع EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_رستہ(رستہ);
  إذا (!نتیجہ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_لبھیا(نتیجہ.getError()))
      أرجع ExternalFS->get_Real_رستہ(رستہ, Output);
    أرجع نتیجہ.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  إذا (خودکار ExtRedirect = نتیجہ->getExternalRedirect()) {
    خودکار P = ExternalFS->get_Real_رستہ(*ExtRedirect, Output);
    إذا (P && Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_لبھیا(P, نتیجہ->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع ExternalFS->get_Real_رستہ(رستہ, Output);
    }
    أرجع P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  إذا (Redirection == Redirect_قسم::Fallthrough) {
    نتیجہ->get_رستہ(Output);
    أرجع {};
  }
  أرجع llvm::errc::invalid_دلیل;
}

فراغ RedirectingFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_سمتیہ_Impl<std::string> &باہرلا) {
  SmallString<256> رستہ;
  Dir.منزل_سمتیہ(رستہ);

  إذا (makeAbsolute(رستہ))
    أرجع;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ثابت منطقي ConsultsExternalFS = Redirection != Redirect_قسم::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_رستہ(رستہ);
  إذا (!نتیجہ) {
    // dir_begin() delegates entirely to ExternalFS.
    إذا (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(رستہ, باہرلا);
    أرجع;
  }

  بدل (نتیجہ->E->get_قسم()) {
  حالة EK_File:
    أرجع;
  حالة EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    اكسر;
  حالة EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_رستہ(*نتیجہ->getExternalRedirect());
    إذا (!makeAbsolute(Remapped_رستہ))
      ExternalFS->getDirectoryContentRealSources(Remapped_رستہ, باہرلا);
    اكسر;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  إذا (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(رستہ, باہرلا);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<حافظہ_بفر> بفر,
                    سروت_Mgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_رستہ, فراغ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  أرجع RedirectingFileSystem::create(std::move(بفر), DiagHandler,
                                       YAMLFile_رستہ, DiagContext,
                                       std::move(ExternalFS));
}

ساكن فراغ getVFSEntries(RedirectingFileSystem::اندراج *SrcE,
                          Small_سمتیہ_Impl<StringRef> &رستہ,
                          Small_سمتیہ_Impl<YAMLVFSEntry> &Entries) {
  خودکار قسم_2 = SrcE->get_قسم();
  إذا (قسم_2 == RedirectingFileSystem::EK_Directory) {
    خودکار *DE = dyn_cast<RedirectingFileSystem::Directory_اندراج>(SrcE);
    أكد(DE && "Must be a directory");
    لكل (std::unique_ptr<RedirectingFileSystem::اندراج> &Sub_اندراج :
         llvm::make_range(DE->contents_begin(), DE->contents_سرا())) {
      رستہ.ڈالو_پٹھ(Sub_اندراج->getName());
      getVFSEntries(Sub_اندراج.get(), رستہ, Entries);
      رستہ.نکالو_پٹھ();
    }
    أرجع;
  }

  إذا (قسم_2 == RedirectingFileSystem::EK_DirectoryRemap) {
    خودکار *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_اندراج>(SrcE);
    أكد(DR && "Must be a directory remap");
    SmallString<128> VPath;
    لكل (خودکار &Comp : رستہ)
      llvm::sys::رستہ_2::append(VPath, Comp);
    Entries.ڈالو_پٹھ(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_رستہ()));
    أرجع;
  }

  أكد(قسم_2 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  خودکار *FE = dyn_cast<RedirectingFileSystem::File_اندراج>(SrcE);
  أكد(FE && "Must be a file");
  SmallString<128> VPath;
  لكل (خودکار &Comp : رستہ)
    llvm::sys::رستہ_2::append(VPath, Comp);
  Entries.ڈالو_پٹھ(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_رستہ()));
}

فراغ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_سمتیہ_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> جڑ_نتیجہ = VFS.lookup_رستہ("/");
  إذا (!جڑ_نتیجہ)
    أرجع;
  Small_سمتیہ<StringRef, 8> Components;
  Components.ڈالو_پٹھ("/");
  getVFSEntries(جڑ_نتیجہ->E, Components, CollectedEntries);
}

UniqueID vfs::get_اگلا_Virtual_Unique_ID() {
  ساكن std::atomic<بنانشانی> UID;
  بنانشانی ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  أرجع UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

فراغ YAMLVFSWriter::جوڑو_اندراج(StringRef Virtual_رستہ, StringRef Real_رستہ,
                             منطقي IsDirectory) {
  أكد(sys::رستہ_2::is_absolute(Virtual_رستہ) && "virtual path not absolute");
  أكد(sys::رستہ_2::is_absolute(Real_رستہ) && "real path not absolute");
  أكد(!رستہ_Has_Traversal(Virtual_رستہ) && "path traversal is not supported");
  Mappings.emplace_پٹھ(Virtual_رستہ, Real_رستہ, IsDirectory);
}

فراغ YAMLVFSWriter::جوڑو_File_Mapping(StringRef Virtual_رستہ, StringRef Real_رستہ) {
  جوڑو_اندراج(Virtual_رستہ, Real_رستہ, /*IsDirectory=*/خطأ);
}

فراغ YAMLVFSWriter::جوڑو_Directory_Mapping(StringRef Virtual_رستہ,
                                        StringRef Real_رستہ) {
  جوڑو_اندراج(Virtual_رستہ, Real_رستہ, /*IsDirectory=*/سچ);
}

نطاق {

صنف JSONWriter {
  llvm::raw_ostream &OS;
  Small_سمتیہ<StringRef, 16> Dir_ڈھیر;

  بنانشانی getDirIndent() { أرجع 4 * Dir_ڈھیر.ناپ_2(); }
  بنانشانی getFileIndent() { أرجع 4 * (Dir_ڈھیر.ناپ_2() + 1); }
  منطقي contained_وچ(StringRef ماپا_2, StringRef رستہ);
  StringRef containedPart(StringRef ماپا_2, StringRef رستہ);
  فراغ شروع_Directory(StringRef رستہ);
  فراغ سرا_Directory();
  فراغ write_اندراج(StringRef VPath, StringRef RPath);

عام:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  فراغ اكتب(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<منطقي> Use_External_ناں,
             std::optional<منطقي> IsCaseSensitive,
             std::optional<منطقي> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

منطقي JSONWriter::contained_وچ(StringRef ماپا_2, StringRef رستہ) {
  ورتوں نطاق llvm::sys;

  // Compare each path component.
  خودکار IParent = رستہ_2::begin(ماپا_2), EParent = رستہ_2::سرا(ماپا_2);
  لكل (خودکار IChild = رستہ_2::begin(رستہ), EChild = رستہ_2::سرا(رستہ);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    إذا (*IParent != *IChild)
      أرجع خطأ;
  }
  // Have we exhausted the parent path?
  أرجع IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef ماپا_2, StringRef رستہ) {
  أكد(!ماپا_2.خالی_تھاواں());
  أكد(contained_وچ(ماپا_2, رستہ));
  أرجع رستہ.substr(ماپا_2.ناپ_2() + 1);
}

فراغ JSONWriter::شروع_Directory(StringRef رستہ) {
  StringRef Name =
      Dir_ڈھیر.خالی_تھاواں() ? رستہ : containedPart(Dir_ڈھیر.پٹھ(), رستہ);
  Dir_ڈھیر.ڈالو_پٹھ(رستہ);
  بنانشانی Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

فراغ JSONWriter::سرا_Directory() {
  بنانشانی Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ڈھیر.نکالو_پٹھ();
}

فراغ JSONWriter::write_اندراج(StringRef VPath, StringRef RPath) {
  بنانشانی Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

فراغ JSONWriter::اكتب(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<منطقي> Use_External_ناں,
                       std::optional<منطقي> IsCaseSensitive,
                       std::optional<منطقي> IsOverlayRelative,
                       StringRef OverlayDir) {
  ورتوں نطاق llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  إذا (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  إذا (Use_External_ناں)
    OS << "  'use-external-names': '" << (*Use_External_ناں ? "true" : "false")
       << "',\n";
  منطقي UseOverlayRelative = خطأ;
  إذا (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  إذا (!Entries.خالی_تھاواں()) {
    ثابت YAMLVFSEntry &اندراج = Entries.front();

    شروع_Directory(
      اندراج.IsDirectory ? اندراج.VPath : رستہ_2::ماپا_رستہ(اندراج.VPath)
    );

    StringRef RPath = اندراج.RPath;
    إذا (UseOverlayRelative) {
      أكد(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.ناپ_2());
    }

    منطقي Is_Current_Dir_خالی_تھاواں = سچ;
    إذا (!اندراج.IsDirectory) {
      write_اندراج(رستہ_2::filename(اندراج.VPath), RPath);
      Is_Current_Dir_خالی_تھاواں = خطأ;
    }

    لكل (ثابت خودکار &اندراج : Entries.ٹکڑا(1)) {
      StringRef Dir =
          اندراج.IsDirectory ? اندراج.VPath : رستہ_2::ماپا_رستہ(اندراج.VPath);
      إذا (Dir == Dir_ڈھیر.پٹھ()) {
        إذا (!Is_Current_Dir_خالی_تھاواں) {
          OS << ",\n";
        }
      } وإلا {
        منطقي Is_Dir_Popped_From_ڈھیر = خطأ;
        طالما (!Dir_ڈھیر.خالی_تھاواں() && !contained_وچ(Dir_ڈھیر.پٹھ(), Dir)) {
          OS << "\n";
          سرا_Directory();
          Is_Dir_Popped_From_ڈھیر = سچ;
        }
        إذا (Is_Dir_Popped_From_ڈھیر || !Is_Current_Dir_خالی_تھاواں) {
          OS << ",\n";
        }
        شروع_Directory(Dir);
        Is_Current_Dir_خالی_تھاواں = سچ;
      }
      StringRef RPath = اندراج.RPath;
      إذا (UseOverlayRelative) {
        أكد(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.ناپ_2());
      }
      إذا (!اندراج.IsDirectory) {
        write_اندراج(رستہ_2::filename(اندراج.VPath), RPath);
        Is_Current_Dir_خالی_تھاواں = خطأ;
      }
    }

    طالما (!Dir_ڈھیر.خالی_تھاواں()) {
      OS << "\n";
      سرا_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

فراغ YAMLVFSWriter::اكتب(llvm::raw_ostream &OS) {
  llvm::ترتیب_دیو(Mappings, [](ثابت YAMLVFSEntry &LHS, ثابت YAMLVFSEntry &RHS) {
    أرجع LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).اكتب(Mappings, Use_External_ناں, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ثابت Twine &رستہ, std::error_کوڈ &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(رستہ, EC);
  إذا (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ڈھیر.ڈالو_پٹھ(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_کوڈ &EC) {
  أكد(FS && State && !State->ڈھیر.خالی_تھاواں() && "incrementing past end");
  أكد(!State->ڈھیر.پٹھ()->رستہ_2().خالی_تھاواں() && "non-canonical end iterator");
  vfs::directory_iterator سرا_2;

  إذا (State->Has_No_ڈالو_Request)
    State->Has_No_ڈالو_Request = خطأ;
  وإلا {
    إذا (State->ڈھیر.پٹھ()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ڈھیر.پٹھ()->رستہ_2(), EC);
      إذا (I != سرا_2) {
        State->ڈھیر.ڈالو_پٹھ(I);
        أرجع *هذا;
      }
    }
  }

  طالما (!State->ڈھیر.خالی_تھاواں() && State->ڈھیر.پٹھ().increment(EC) == سرا_2)
    State->ڈھیر.نکالو_پٹھ();

  إذا (State->ڈھیر.خالی_تھاواں())
    State.reset(); // end iterator

  أرجع *هذا;
}

ثابت حرف FileSystem::ID = 0;
ثابت حرف OverlayFileSystem::ID = 0;
ثابت حرف ProxyFileSystem::ID = 0;
ثابت حرف وچ_حافظہ_File_System::ID = 0;
ثابت حرف RedirectingFileSystem::ID = 0;

بنانشانی ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_گنتی(
    ثابت FileSystem *FS) {
  أرجع FS->Use_گنتی();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
