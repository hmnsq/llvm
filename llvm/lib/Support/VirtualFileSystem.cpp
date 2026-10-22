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

استعمال نطاق llvm;
استعمال نطاق llvm::vfs;

استعمال llvm::sys::fs::file_حالت;
استعمال llvm::sys::fs::file_t;
استعمال llvm::sys::fs::file_type;
استعمال llvm::sys::fs::perms;
استعمال llvm::sys::fs::UniqueID;

حالت::حالت(ثابت file_حالت &حالت)
    : UID(حالت.getUniqueID()), MTime(حالت.get_آخري_Modification_وقت()),
      User(حالت.getUser()), گروهه(حالت.get_گروهه()), سائيز(حالت.get_سائيز()),
      Type(حالت.type()), Perms(حالت.permissions()) {}

حالت::حالت(ثابت Twine &Name, UniqueID UID, sys::وقت_نقطو<> MTime,
               uint32_t User, uint32_t گروهه, uint64_t سائيز, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), گروهه(گروهه),
      سائيز(سائيز), Type(Type), Perms(Perms) {}

حالت حالت::copy_With_New_سائيز(ثابت حالت &اندريون_2, uint64_t New_سائيز) {
  أرجع حالت(اندريون_2.getName(), اندريون_2.getUniqueID(), اندريون_2.get_آخري_Modification_وقت(),
                اندريون_2.getUser(), اندريون_2.get_گروهه(), New_سائيز, اندريون_2.getType(),
                اندريون_2.getPermissions());
}

حالت حالت::copyWithNewName(ثابت حالت &اندريون_2, ثابت Twine &NewName) {
  أرجع حالت(NewName, اندريون_2.getUniqueID(), اندريون_2.get_آخري_Modification_وقت(),
                اندريون_2.getUser(), اندريون_2.get_گروهه(), اندريون_2.get_سائيز(), اندريون_2.getType(),
                اندريون_2.getPermissions());
}

حالت حالت::copyWithNewName(ثابت file_حالت &اندريون_2, ثابت Twine &NewName) {
  أرجع حالت(NewName, اندريون_2.getUniqueID(), اندريون_2.get_آخري_Modification_وقت(),
                اندريون_2.getUser(), اندريون_2.get_گروهه(), اندريون_2.get_سائيز(), اندريون_2.type(),
                اندريون_2.permissions());
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
  أرجع is_حالت_معلوم() && Type != file_type::file_not_مليو;
}

File::~File() = افتراضي;

FileSystem::~FileSystem() = افتراضي;

ErrorOr<std::unique_ptr<يادگيري_بفر>>
FileSystem::get_بفر_For_File(ثابت llvm::Twine &Name, int64_t File_سائيز,
                             منطقي RequiresNullTerminator, منطقي IsVolatile,
                             منطقي Is_متن) {
  خودڪار F = Is_متن ? openFileForRead(Name) : open_File_For_Read_ٻٽو(Name);
  إذا (!F)
    أرجع F.getError();

  أرجع (*F)->get_بفر(Name, File_سائيز, RequiresNullTerminator, IsVolatile);
}

std::error_ڪوڊ FileSystem::makeAbsolute(Small_ويڪٽر_Impl<حرف> &رستو) ثابت {
  إذا (llvm::sys::رستو_2::is_absolute(رستو))
    أرجع {};

  خودڪار WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  sys::رستو_2::make_absolute(WorkingDir.get(), رستو);
  أرجع {};
}

std::error_ڪوڊ FileSystem::get_Real_رستو(ثابت Twine &رستو,
                                        Small_ويڪٽر_Impl<حرف> &Output) {
  أرجع errc::operation_not_permitted;
}

std::error_ڪوڊ FileSystem::isLocal(ثابت Twine &رستو, منطقي &نتيجو) {
  أرجع errc::operation_not_permitted;
}

منطقي FileSystem::exists(ثابت Twine &رستو) {
  خودڪار حالت = حالت_2(رستو);
  أرجع حالت && حالت->exists();
}

llvm::ErrorOr<منطقي> FileSystem::equivalent(ثابت Twine &A, ثابت Twine &B) {
  خودڪار حالت_A = حالت_2(A);
  إذا (!حالت_A)
    أرجع حالت_A.getError();
  خودڪار حالت_B = حالت_2(B);
  إذا (!حالت_B)
    أرجع حالت_B.getError();
  أرجع حالت_A->equivalent(*حالت_B);
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
فراغ FileSystem::dump() ثابت { print(dbgs(), PrintType::RecursiveContents); }
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
ساكن منطقي isTraversalComponent(StringRef Component) {
  أرجع Component == ".." || Component == ".";
}

ساكن منطقي رستو_Has_Traversal(StringRef رستو) {
  استعمال نطاق llvm::sys;

  لكل (StringRef Comp : llvm::make_range(رستو_2::begin(رستو), رستو_2::پڇاڙي(رستو)))
    إذا (isTraversalComponent(Comp))
      أرجع سچ;
  أرجع خطأ;
}
#جيڪڏهن_پورو

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

/// Wrapper around a raw file descriptor.
صنف RealFile : عام File {
  دوست صنف RealFileSystem;

  file_t FD;
  حالت S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_رستو_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::حالت_error, {}),
        RealName(New_Real_رستو_Name.str()) {
    أكد(FD.isValid() && "Invalid or inactive file descriptor");
  }

عام:
  ~RealFile() override;

  ErrorOr<حالت> حالت_2() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<يادگيري_بفر>> get_بفر(ثابت Twine &Name,
                                                   int64_t File_سائيز,
                                                   منطقي RequiresNullTerminator,
                                                   منطقي IsVolatile) override;
  std::error_ڪوڊ close() override;
  فراغ سيٽ_رستو(ثابت Twine &رستو) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<حالت> RealFile::حالت_2() {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot stat closed file");
  إذا (!S.is_حالت_معلوم()) {
    file_حالت Real_حالت;
    إذا (std::error_ڪوڊ EC = sys::fs::حالت_2(FD, Real_حالت))
      أرجع EC;
    S = حالت::copyWithNewName(Real_حالت, S.getName());
  }
  أرجع S;
}

ErrorOr<std::string> RealFile::getName() {
  أرجع RealName.خالي_جايون() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<يادگيري_بفر>>
RealFile::get_بفر(ثابت Twine &Name, int64_t File_سائيز,
                    منطقي RequiresNullTerminator, منطقي IsVolatile) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot get buffer for closed file");
  أرجع يادگيري_بفر::getOpenFile(FD, Name, File_سائيز, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_ڪوڊ RealFile::close() {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  std::error_ڪوڊ EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  أرجع EC;
}

فراغ RealFile::سيٽ_رستو(ثابت Twine &رستو) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  RealName = رستو.str();
  إذا (خودڪار حالت = حالت_2())
    S = حالت.get().copyWithNewName(حالت.get(), رستو);
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
      إذا (std::error_ڪوڊ EC = llvm::sys::fs::current_رستو(PWD))
        WD = std::move(EC);
      وإلا إذا (llvm::sys::fs::real_رستو(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      وإلا
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<حالت> حالت_2(ثابت Twine &رستو) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ثابت Twine &رستو) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ٻٽو(ثابت Twine &رستو) override;
  directory_iterator dir_begin(ثابت Twine &Dir, std::error_ڪوڊ &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override;
  std::error_ڪوڊ سيٽ_Current_Working_Directory(ثابت Twine &رستو) override;
  std::error_ڪوڊ isLocal(ثابت Twine &رستو, منطقي &نتيجو) override;
  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_ويڪٽر_Impl<std::string> &ٻاهريون) override {
    ٻاهريون.ڌڪو_پٺ(Dir.str());
  }
  std::error_ڪوڊ get_Real_رستو(ثابت Twine &رستو,
                              Small_ويڪٽر_Impl<حرف> &Output) override;

محمي:
  فراغ printImpl(raw_ostream &OS, PrintType Type,
                 بنانشان IndentLevel) ثابت override;

خاص:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_رستو(ثابت Twine &رستو, Small_ويڪٽر_Impl<حرف> &Storage) ثابت {
    إذا (!WD || !*WD)
      أرجع رستو;
    رستو.منزل_ويڪٽر(Storage);
    sys::رستو_2::make_absolute(WD->get().Resolved, Storage);
    أرجع Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ثابت Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_رستو(Name, Storage), Flags, &RealName);
    إذا (!FDOrErr)
      أرجع error_منزل_Error_ڪوڊ(FDOrErr.takeError());
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

ErrorOr<حالت> RealFileSystem::حالت_2(ثابت Twine &رستو) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_حالت Real_حالت;
  إذا (std::error_ڪوڊ EC =
          sys::fs::حالت_2(adjust_رستو(رستو, Storage), Real_حالت))
    أرجع EC;
  أرجع حالت::copyWithNewName(Real_حالت, رستو);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ثابت Twine &Name) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::جو_متن);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ٻٽو(ثابت Twine &Name) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::جو_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ثابت {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (WD && *WD)
    أرجع std::string(WD->get().Specified);
  إذا (WD)
    أرجع WD->getError();

  SmallString<128> Dir;
  إذا (std::error_ڪوڊ EC = llvm::sys::fs::current_رستو(Dir))
    أرجع EC;
  أرجع std::string(Dir);
}

std::error_ڪوڊ RealFileSystem::سيٽ_Current_Working_Directory(ثابت Twine &رستو) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (!WD)
    أرجع llvm::sys::fs::سيٽ_current_رستو(رستو);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_رستو(رستو, Storage).منزل_ويڪٽر(Absolute);
  منطقي IsDir;
  إذا (خودڪار Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    أرجع Err;
  إذا (!IsDir)
    أرجع std::make_error_ڪوڊ(std::errc::not_a_directory);
  إذا (خودڪار Err = llvm::sys::fs::real_رستو(Absolute, Resolved))
    أرجع Err;
  WD = WorkingDirectory{Absolute, Resolved};
  أرجع std::error_ڪوڊ();
}

std::error_ڪوڊ RealFileSystem::isLocal(ثابت Twine &رستو, منطقي &نتيجو) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::is_local(adjust_رستو(رستو, Storage), نتيجو);
}

std::error_ڪوڊ RealFileSystem::get_Real_رستو(ثابت Twine &رستو,
                                            Small_ويڪٽر_Impl<حرف> &Output) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::real_رستو(adjust_رستو(رستو, Storage), Output);
}

فراغ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               بنانشان IndentLevel) ثابت {
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
  RealFSDirIter(ثابت Twine &رستو, std::error_ڪوڊ &EC) {
    خودڪار BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(رستو, EC);
    إذا (Iter != sys::fs::directory_iterator())
      Current_داخلا = directory_داخلا(Iter->رستو_2(), Iter->type());
  }

  std::error_ڪوڊ increment() override {
    خودڪار BypassSandbox = sys::sandbox::scopedDisable();

    std::error_ڪوڊ EC;
    Iter.increment(EC);
    Current_داخلا = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_داخلا()
                       : directory_داخلا(Iter->رستو_2(), Iter->type());
    أرجع EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ثابت Twine &Dir,
                                             std::error_ڪوڊ &EC) {
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  أرجع directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_رستو(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> بنياد_FS) {
  FSList.ڌڪو_پٺ(std::move(بنياد_FS));
}

فراغ OverlayFileSystem::ڌڪو_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ڌڪو_پٺ(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->سيٽ_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<حالت> OverlayFileSystem::حالت_2(ثابت Twine &رستو) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_پڇاڙي(); I != E; ++I) {
    ErrorOr<حالت> حالت = (*I)->حالت_2(رستو);
    إذا (حالت || حالت.getError() != llvm::errc::no_such_file_or_directory)
      أرجع حالت;
  }
  أرجع make_error_ڪوڊ(llvm::errc::no_such_file_or_directory);
}

منطقي OverlayFileSystem::exists(ثابت Twine &رستو) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_پڇاڙي(); I != E; ++I) {
    إذا ((*I)->exists(رستو))
      أرجع سچ;
  }
  أرجع خطأ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ثابت llvm::Twine &رستو) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_پڇاڙي(); I != E; ++I) {
    خودڪار نتيجو = (*I)->openFileForRead(رستو);
    إذا (نتيجو || نتيجو.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتيجو;
  }
  أرجع make_error_ڪوڊ(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ثابت {
  // All file systems are synchronized, just take the first working directory.
  أرجع FSList.front()->getCurrentWorkingDirectory();
}

std::error_ڪوڊ
OverlayFileSystem::سيٽ_Current_Working_Directory(ثابت Twine &رستو) {
  لكل (خودڪار &FS : FSList)
    إذا (std::error_ڪوڊ EC = FS->سيٽ_Current_Working_Directory(رستو))
      أرجع EC;
  أرجع {};
}

std::error_ڪوڊ OverlayFileSystem::isLocal(ثابت Twine &رستو, منطقي &نتيجو) {
  لكل (خودڪار &FS : FSList)
    إذا (FS->exists(رستو))
      أرجع FS->isLocal(رستو, نتيجو);
  أرجع errc::no_such_file_or_directory;
}

std::error_ڪوڊ OverlayFileSystem::get_Real_رستو(ثابت Twine &رستو,
                                               Small_ويڪٽر_Impl<حرف> &Output) {
  لكل (ثابت خودڪار &FS : FSList)
    إذا (FS->exists(رستو))
      أرجع FS->get_Real_رستو(رستو, Output);
  أرجع errc::no_such_file_or_directory;
}

فراغ OverlayFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_ويڪٽر_Impl<std::string> &ٻاهريون) {
  // All layers contribute.
  لكل (iterator I = overlays_begin(), E = overlays_پڇاڙي(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, ٻاهريون);
}

فراغ OverlayFileSystem::گهمو_ٻار_File_Systems(گهمو_Callback_Ty Callback) {
  لكل (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->گهمو_ٻار_File_Systems(Callback);
  }
}

فراغ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  بنانشان IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  إذا (Type == PrintType::Summary)
    أرجع;

  إذا (Type == PrintType::Contents)
    Type = PrintType::Summary;
  لكل (ثابت خودڪار &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = افتراضي;

نطاق {

/// Combines and deduplicates directory entries across multiple file systems.
صنف CombiningDirIterImpl : عام llvm::vfs::detail::DirIterImpl {
  استعمال FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_ويڪٽر<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_سيٽ<> ڏٺل_نالا;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_ڪوڊ incrementIter(منطقي Is_پهريون_وقت) {
    طالما (!IterList.خالي_جايون()) {
      CurrentDirIter = IterList.پٺ();
      IterList.ڪڍو_پٺ();
      إذا (CurrentDirIter != directory_iterator())
        اكسر; //مليو
    }

    إذا (Is_پهريون_وقت && CurrentDirIter == directory_iterator())
      أرجع errc::no_such_file_or_directory;
    أرجع {};
  }

  std::error_ڪوڊ incrementDirIter(منطقي Is_پهريون_وقت) {
    أكد((Is_پهريون_وقت || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_ڪوڊ EC;
    إذا (!Is_پهريون_وقت)
      CurrentDirIter.increment(EC);
    إذا (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(Is_پهريون_وقت);
    أرجع EC;
  }

  std::error_ڪوڊ incrementImpl(منطقي Is_پهريون_وقت) {
    طالما (سچ) {
      std::error_ڪوڊ EC = incrementDirIter(Is_پهريون_وقت);
      إذا (EC || CurrentDirIter == directory_iterator()) {
        Current_داخلا = directory_داخلا();
        أرجع EC;
      }
      Current_داخلا = *CurrentDirIter;
      StringRef Name = llvm::sys::رستو_2::filename(Current_داخلا.رستو_2());
      إذا (ڏٺل_نالا.وجهو(Name).ٻيو_2)
        أرجع EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

عام:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_ڪوڊ &EC) {
    لكل (ثابت خودڪار &FS : FileSystems) {
      std::error_ڪوڊ FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      إذا (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        أرجع;
      }
      إذا (!FEC)
        IterList.ڌڪو_پٺ(Iter);
    }
    EC = incrementImpl(سچ);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_ڪوڊ &EC)
      : IterList(DirIters) {
    EC = incrementImpl(سچ);
  }

  std::error_ڪوڊ increment() override { أرجع incrementImpl(خطأ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ثابت Twine &Dir,
                                                std::error_ڪوڊ &EC) {
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

تعداد اندريون_يادگيري_ڳنڍ_قسم {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
صنف اندريون_يادگيري_ڳنڍ {
  اندريون_يادگيري_ڳنڍ_قسم قسم_2;
  std::string FileName;

عام:
  اندريون_يادگيري_ڳنڍ(llvm::StringRef FileName, اندريون_يادگيري_ڳنڍ_قسم قسم_2)
      : قسم_2(قسم_2), FileName(std::string(llvm::sys::رستو_2::filename(FileName))) {
  }
  مجازي ~اندريون_يادگيري_ڳنڍ() = افتراضي;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  مجازي حالت get_حالت(ثابت Twine &RequestedName) ثابت = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ثابت { أرجع FileName; }
  اندريون_يادگيري_ڳنڍ_قسم get_قسم() ثابت { أرجع قسم_2; }
  مجازي std::string منزل_String(بنانشان Indent) ثابت = 0;
};

صنف اندريون_يادگيري_File : عام اندريون_يادگيري_ڳنڍ {
  حالت Stat;
  std::unique_ptr<llvm::يادگيري_بفر> بفر;

عام:
  اندريون_يادگيري_File(حالت Stat, std::unique_ptr<llvm::يادگيري_بفر> بفر)
      : اندريون_يادگيري_ڳنڍ(Stat.getName(), IME_File), Stat(std::move(Stat)),
        بفر(std::move(بفر)) {}

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }
  llvm::يادگيري_بفر *get_بفر() ثابت { أرجع بفر.get(); }

  std::string منزل_String(بنانشان Indent) ثابت override {
    أرجع (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ساكن منطقي classof(ثابت اندريون_يادگيري_ڳنڍ *N) {
    أرجع N->get_قسم() == IME_File;
  }
};

نطاق {

صنف اندريون_يادگيري_Hard_Link : عام اندريون_يادگيري_ڳنڍ {
  ثابت اندريون_يادگيري_File &ResolvedFile;

عام:
  اندريون_يادگيري_Hard_Link(StringRef رستو, ثابت اندريون_يادگيري_File &ResolvedFile)
      : اندريون_يادگيري_ڳنڍ(رستو, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ثابت اندريون_يادگيري_File &getResolvedFile() ثابت { أرجع ResolvedFile; }

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع ResolvedFile.get_حالت(RequestedName);
  }

  std::string منزل_String(بنانشان Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.منزل_String(0);
  }

  ساكن منطقي classof(ثابت اندريون_يادگيري_ڳنڍ *N) {
    أرجع N->get_قسم() == IME_HardLink;
  }
};

صنف اندريون_يادگيري_Symbolic_Link : عام اندريون_يادگيري_ڳنڍ {
  std::string هدف_رستو;
  حالت Stat;

عام:
  اندريون_يادگيري_Symbolic_Link(StringRef رستو, StringRef هدف_رستو, حالت Stat)
      : اندريون_يادگيري_ڳنڍ(رستو, IME_SymbolicLink), هدف_رستو(std::move(هدف_رستو)),
        Stat(Stat) {}

  std::string منزل_String(بنانشان Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "SymbolicLink to -> " + هدف_رستو;
  }

  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_هدف_رستو() ثابت { أرجع هدف_رستو; }

  ساكن منطقي classof(ثابت اندريون_يادگيري_ڳنڍ *N) {
    أرجع N->get_قسم() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
صنف اندريون_يادگيري_File_Adaptor : عام File {
  ثابت اندريون_يادگيري_File &ڳنڍ;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

عام:
  واضح اندريون_يادگيري_File_Adaptor(ثابت اندريون_يادگيري_File &ڳنڍ,
                               std::string RequestedName)
      : ڳنڍ(ڳنڍ), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<حالت> حالت_2() override {
    أرجع ڳنڍ.get_حالت(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::يادگيري_بفر>>
  get_بفر(ثابت Twine &Name, int64_t File_سائيز, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    llvm::يادگيري_بفر *Buf = ڳنڍ.get_بفر();
    أرجع llvm::يادگيري_بفر::get_Mem_بفر(
        Buf->get_بفر(), Buf->get_بفر_Identifier(), RequiresNullTerminator);
  }

  std::error_ڪوڊ close() override { أرجع {}; }

  فراغ سيٽ_رستو(ثابت Twine &رستو) override { RequestedName = رستو.str(); }
};
} // namespace

صنف اندريون_يادگيري_Directory : عام اندريون_يادگيري_ڳنڍ {
  حالت Stat;
  std::map<std::string, std::unique_ptr<اندريون_يادگيري_ڳنڍ>, std::less<>> Entries;

عام:
  اندريون_يادگيري_Directory(حالت Stat)
      : اندريون_يادگيري_ڳنڍ(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  حالت get_حالت(ثابت Twine &RequestedName) ثابت override {
    أرجع حالت::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ثابت { أرجع Stat.getUniqueID(); }

  اندريون_يادگيري_ڳنڍ *get_ٻار(StringRef Name) ثابت {
    خودڪار I = Entries.ڳوليو(Name);
    إذا (I != Entries.پڇاڙي())
      أرجع I->ٻيو_2.get();
    أرجع خالي_اشارو;
  }

  اندريون_يادگيري_ڳنڍ *جوڙيو_ٻار(StringRef Name, std::unique_ptr<اندريون_يادگيري_ڳنڍ> ٻار) {
    أرجع Entries.emplace(Name, std::move(ٻار)).پهريون_2->ٻيو_2.get();
  }

  استعمال const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ثابت { أرجع Entries.begin(); }
  const_iterator پڇاڙي() ثابت { أرجع Entries.پڇاڙي(); }

  std::string منزل_String(بنانشان Indent) ثابت override {
    std::string نتيجو =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    لكل (ثابت خودڪار &داخلا_3 : Entries)
      نتيجو += داخلا_3.ٻيو_2->منزل_String(Indent + 2);
    أرجع نتيجو;
  }

  ساكن منطقي classof(ثابت اندريون_يادگيري_ڳنڍ *N) {
    أرجع N->get_قسم() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ساكن sys::fs::UniqueID getUniqueID(hash_ڪوڊ Hash) {
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

حالت detail::New_اندريون_يادگيري_ڳنڍ_Info::make_حالت() ثابت {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, بفر ? بفر->get_بفر() : "");

  أرجع حالت(رستو, UID, llvm::sys::منزل_وقت_نقطو(Modification_وقت), User,
                گروهه, بفر ? بفر->get_بفر_سائيز() : 0, Type, Perms);
}

اندريون_يادگيري_File_System::اندريون_يادگيري_File_System(منطقي UseNormalizedPaths)
    : پاڙ_2(جديد detail::اندريون_يادگيري_Directory(
          حالت("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::وقت_نقطو<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

اندريون_يادگيري_File_System::~اندريون_يادگيري_File_System() = افتراضي;

std::string اندريون_يادگيري_File_System::منزل_String() ثابت {
  أرجع پاڙ_2->منزل_String(/*Indent=*/0);
}

منطقي اندريون_يادگيري_File_System::جوڙيو_File(ثابت Twine &P, time_t Modification_وقت,
                                 std::unique_ptr<llvm::يادگيري_بفر> بفر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> گروهه,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_ڳنڍ_Fn Make_ڳنڍ) {
  SmallString<128> رستو;
  P.منزل_ويڪٽر(رستو);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ڪوڊ EC = makeAbsolute(رستو);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::رستو_2::remove_dots(رستو, /*remove_dot_dot=*/سچ);

  إذا (رستو.خالي_جايون())
    أرجع خطأ;

  detail::اندريون_يادگيري_Directory *Dir = پاڙ_2.get();
  خودڪار I = llvm::sys::رستو_2::begin(رستو), E = sys::رستو_2::پڇاڙي(رستو);
  ثابت خودڪار ResolvedUser = User.قدر_or(0);
  ثابت خودڪار Resolved_گروهه = گروهه.قدر_or(0);
  ثابت خودڪار ResolvedType = Type.قدر_or(sys::fs::file_type::regular_file);
  ثابت خودڪار ResolvedPerms = Perms.قدر_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ثابت خودڪار NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  طالما (سچ) {
    Name = *I;
    ++I;
    إذا (I == E)
      اكسر;
    detail::اندريون_يادگيري_ڳنڍ *ڳنڍ = Dir->get_ٻار(Name);
    إذا (!ڳنڍ) {
      // This isn't the last element, so we create a new directory.
      حالت Stat(
          StringRef(رستو.str().begin(), Name.پڇاڙي() - رستو.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::منزل_وقت_نقطو(Modification_وقت), ResolvedUser, Resolved_گروهه,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::اندريون_يادگيري_Directory>(Dir->جوڙيو_ٻار(
          Name, std::make_unique<detail::اندريون_يادگيري_Directory>(std::move(Stat))));
      استمر;
    }
    // Creating file under another file.
    إذا (!isa<detail::اندريون_يادگيري_Directory>(ڳنڍ))
      أرجع خطأ;
    Dir = cast<detail::اندريون_يادگيري_Directory>(ڳنڍ);
  }
  detail::اندريون_يادگيري_ڳنڍ *ڳنڍ = Dir->get_ٻار(Name);
  إذا (!ڳنڍ) {
    Dir->جوڙيو_ٻار(Name,
                  Make_ڳنڍ({Dir->getUniqueID(), رستو, Name, Modification_وقت,
                            std::move(بفر), ResolvedUser, Resolved_گروهه,
                            ResolvedType, ResolvedPerms}));
    أرجع سچ;
  }
  إذا (isa<detail::اندريون_يادگيري_Directory>(ڳنڍ))
    أرجع ResolvedType == sys::fs::file_type::directory_file;

  أكد((isa<detail::اندريون_يادگيري_File>(ڳنڍ) ||
          isa<detail::اندريون_يادگيري_Hard_Link>(ڳنڍ)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  إذا (خودڪار *Link = dyn_cast<detail::اندريون_يادگيري_Hard_Link>(ڳنڍ)) {
    أرجع Link->getResolvedFile().get_بفر()->get_بفر() ==
           بفر->get_بفر();
  }
  أرجع cast<detail::اندريون_يادگيري_File>(ڳنڍ)->get_بفر()->get_بفر() ==
         بفر->get_بفر();
}

منطقي اندريون_يادگيري_File_System::جوڙيو_File(ثابت Twine &P, time_t Modification_وقت,
                                 std::unique_ptr<llvm::يادگيري_بفر> بفر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> گروهه,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  أرجع جوڙيو_File(P, Modification_وقت, std::move(بفر), User, گروهه, Type,
                 Perms,
                 [](detail::New_اندريون_يادگيري_ڳنڍ_Info NNI)
                     -> std::unique_ptr<detail::اندريون_يادگيري_ڳنڍ> {
                   حالت Stat = NNI.make_حالت();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::اندريون_يادگيري_Directory>(Stat);
                   أرجع std::make_unique<detail::اندريون_يادگيري_File>(
                       Stat, std::move(NNI.بفر));
                 });
}

منطقي اندريون_يادگيري_File_System::جوڙيو_File_No_Own(
    ثابت Twine &P, time_t Modification_وقت,
    ثابت llvm::يادگيري_بفر_Ref &بفر, std::optional<uint32_t> User,
    std::optional<uint32_t> گروهه, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  أرجع جوڙيو_File(P, Modification_وقت, llvm::يادگيري_بفر::get_Mem_بفر(بفر),
                 std::move(User), std::move(گروهه), std::move(Type),
                 std::move(Perms),
                 [](detail::New_اندريون_يادگيري_ڳنڍ_Info NNI)
                     -> std::unique_ptr<detail::اندريون_يادگيري_ڳنڍ> {
                   حالت Stat = NNI.make_حالت();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::اندريون_يادگيري_Directory>(Stat);
                   أرجع std::make_unique<detail::اندريون_يادگيري_File>(
                       Stat, std::move(NNI.بفر));
                 });
}

detail::Named_ڳنڍ_Or_Error
اندريون_يادگيري_File_System::lookup_ڳنڍ(ثابت Twine &P, منطقي FollowFinalSymlink,
                               size_t Symlink_اونهائي) ثابت {
  SmallString<128> رستو;
  P.منزل_ويڪٽر(رستو);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ڪوڊ EC = makeAbsolute(رستو);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::رستو_2::remove_dots(رستو, /*remove_dot_dot=*/سچ);

  ثابت detail::اندريون_يادگيري_Directory *Dir = پاڙ_2.get();
  إذا (رستو.خالي_جايون())
    أرجع detail::Named_ڳنڍ_Or_Error(رستو, Dir);

  خودڪار I = llvm::sys::رستو_2::begin(رستو), E = llvm::sys::رستو_2::پڇاڙي(رستو);
  طالما (سچ) {
    detail::اندريون_يادگيري_ڳنڍ *ڳنڍ = Dir->get_ٻار(*I);
    ++I;
    إذا (!ڳنڍ)
      أرجع errc::no_such_file_or_directory;

    إذا (خودڪار Symlink = dyn_cast<detail::اندريون_يادگيري_Symbolic_Link>(ڳنڍ)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      إذا (I == E && !FollowFinalSymlink)
        أرجع detail::Named_ڳنڍ_Or_Error(رستو, Symlink);

      إذا (Symlink_اونهائي > اندريون_يادگيري_File_System::Max_Symlink_اونهائي)
        أرجع errc::no_such_file_or_directory;

      SmallString<128> هدف_رستو = Symlink->get_هدف_رستو();
      إذا (std::error_ڪوڊ EC = makeAbsolute(هدف_رستو))
        أرجع EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      خودڪار هدف_2 =
          lookup_ڳنڍ(هدف_رستو, /*FollowFinalSymlink=*/سچ, Symlink_اونهائي + 1);
      إذا (!هدف_2 || I == E)
        أرجع هدف_2;

      إذا (!isa<detail::اندريون_يادگيري_Directory>(*هدف_2))
        أرجع errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::اندريون_يادگيري_Directory>(*هدف_2);
      استمر;
    }

    // Return the file if it's at the end of the path.
    إذا (خودڪار File = dyn_cast<detail::اندريون_يادگيري_File>(ڳنڍ)) {
      إذا (I == E)
        أرجع detail::Named_ڳنڍ_Or_Error(رستو, File);
      أرجع errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    إذا (خودڪار File = dyn_cast<detail::اندريون_يادگيري_Hard_Link>(ڳنڍ)) {
      إذا (I == E)
        أرجع detail::Named_ڳنڍ_Or_Error(رستو, &File->getResolvedFile());
      أرجع errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::اندريون_يادگيري_Directory>(ڳنڍ);
    إذا (I == E)
      أرجع detail::Named_ڳنڍ_Or_Error(رستو, Dir);
  }
}

منطقي اندريون_يادگيري_File_System::جوڙيو_Hard_Link(ثابت Twine &NewLink,
                                     ثابت Twine &هدف_2) {
  خودڪار New_Link_ڳنڍ = lookup_ڳنڍ(NewLink, /*FollowFinalSymlink=*/خطأ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  خودڪار هدف_ڳنڍ = lookup_ڳنڍ(هدف_2, /*FollowFinalSymlink=*/سچ);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  إذا (!هدف_ڳنڍ || New_Link_ڳنڍ || !isa<detail::اندريون_يادگيري_File>(*هدف_ڳنڍ))
    أرجع خطأ;
  أرجع جوڙيو_File(NewLink, 0, خالي_اشارو, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_اندريون_يادگيري_ڳنڍ_Info NNI) {
                   أرجع std::make_unique<detail::اندريون_يادگيري_Hard_Link>(
                       NNI.رستو.str(),
                       *cast<detail::اندريون_يادگيري_File>(*هدف_ڳنڍ));
                 });
}

منطقي اندريون_يادگيري_File_System::جوڙيو_Symbolic_Link(
    ثابت Twine &NewLink, ثابت Twine &هدف_2, time_t Modification_وقت,
    std::optional<uint32_t> User, std::optional<uint32_t> گروهه,
    std::optional<llvm::sys::fs::perms> Perms) {
  خودڪار New_Link_ڳنڍ = lookup_ڳنڍ(NewLink, /*FollowFinalSymlink=*/خطأ);
  إذا (New_Link_ڳنڍ)
    أرجع خطأ;

  SmallString<128> NewLinkStr, هدف_Str;
  NewLink.منزل_ويڪٽر(NewLinkStr);
  هدف_2.منزل_ويڪٽر(هدف_Str);

  أرجع جوڙيو_File(NewLinkStr, Modification_وقت, خالي_اشارو, User, گروهه,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_اندريون_يادگيري_ڳنڍ_Info NNI) {
                   أرجع std::make_unique<detail::اندريون_يادگيري_Symbolic_Link>(
                       NewLinkStr, هدف_Str, NNI.make_حالت());
                 });
}

llvm::ErrorOr<حالت> اندريون_يادگيري_File_System::حالت_2(ثابت Twine &رستو) {
  خودڪار ڳنڍ = lookup_ڳنڍ(رستو, /*FollowFinalSymlink=*/سچ);
  إذا (ڳنڍ)
    أرجع (*ڳنڍ)->get_حالت(رستو);
  أرجع ڳنڍ.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
اندريون_يادگيري_File_System::openFileForRead(ثابت Twine &رستو) {
  خودڪار ڳنڍ = lookup_ڳنڍ(رستو,/*FollowFinalSymlink=*/سچ);
  إذا (!ڳنڍ)
    أرجع ڳنڍ.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  إذا (خودڪار *F = dyn_cast<detail::اندريون_يادگيري_File>(*ڳنڍ))
    أرجع std::unique_ptr<File>(
        جديد detail::اندريون_يادگيري_File_Adaptor(*F, رستو.str()));

  // FIXME: errc::not_a_file?
  أرجع make_error_ڪوڊ(llvm::errc::invalid_دليل);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
صنف اندريون_يادگيري_File_System::DirIterator : عام llvm::vfs::detail::DirIterImpl {
  ثابت اندريون_يادگيري_File_System *FS;
  detail::اندريون_يادگيري_Directory::const_iterator I;
  detail::اندريون_يادگيري_Directory::const_iterator E;
  std::string RequestedDirName;

  فراغ سيٽ_Current_داخلا() {
    إذا (I != E) {
      SmallString<256> رستو(RequestedDirName);
      llvm::sys::رستو_2::append(رستو, I->ٻيو_2->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل (I->ٻيو_2->get_قسم()) {
      حالة detail::IME_File:
      حالة detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      حالة detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة detail::IME_SymbolicLink:
        إذا (خودڪار Symlink_هدف =
                FS->lookup_ڳنڍ(رستو, /*FollowFinalSymlink=*/سچ)) {
          رستو = Symlink_هدف.getName();
          Type = (*Symlink_هدف)->get_حالت(رستو).getType();
        }
        اكسر;
      }
      Current_داخلا = directory_داخلا(std::string(رستو), Type);
    } وإلا {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_داخلا = directory_داخلا();
    }
  }

عام:
  DirIterator() = افتراضي;

  DirIterator(ثابت اندريون_يادگيري_File_System *FS,
              ثابت detail::اندريون_يادگيري_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.پڇاڙي()),
        RequestedDirName(std::move(RequestedDirName)) {
    سيٽ_Current_داخلا();
  }

  std::error_ڪوڊ increment() override {
    ++I;
    سيٽ_Current_داخلا();
    أرجع {};
  }
};

directory_iterator اندريون_يادگيري_File_System::dir_begin(ثابت Twine &Dir,
                                                 std::error_ڪوڊ &EC) {
  خودڪار ڳنڍ = lookup_ڳنڍ(Dir, /*FollowFinalSymlink=*/سچ);
  إذا (!ڳنڍ) {
    EC = ڳنڍ.getError();
    أرجع directory_iterator(std::make_shared<DirIterator>());
  }

  إذا (خودڪار *Dir_ڳنڍ = dyn_cast<detail::اندريون_يادگيري_Directory>(*ڳنڍ))
    أرجع directory_iterator(
        std::make_shared<DirIterator>(هذا, *Dir_ڳنڍ, Dir.str()));

  EC = make_error_ڪوڊ(llvm::errc::not_a_directory);
  أرجع directory_iterator(std::make_shared<DirIterator>());
}

std::error_ڪوڊ اندريون_يادگيري_File_System::سيٽ_Current_Working_Directory(ثابت Twine &P) {
  SmallString<128> رستو;
  P.منزل_ويڪٽر(رستو);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ڪوڊ EC = makeAbsolute(رستو);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::رستو_2::remove_dots(رستو, /*remove_dot_dot=*/سچ);

  إذا (!رستو.خالي_جايون())
    WorkingDirectory = std::string(رستو);
  أرجع {};
}

std::error_ڪوڊ اندريون_يادگيري_File_System::get_Real_رستو(ثابت Twine &رستو,
                                                Small_ويڪٽر_Impl<حرف> &Output) {
  خودڪار CWD = getCurrentWorkingDirectory();
  إذا (!CWD || CWD->خالي_جايون())
    أرجع errc::operation_not_permitted;
  رستو.منزل_ويڪٽر(Output);
  إذا (خودڪار EC = makeAbsolute(Output))
    أرجع EC;
  llvm::sys::رستو_2::remove_dots(Output, /*remove_dot_dot=*/سچ);
  أرجع {};
}

std::error_ڪوڊ اندريون_يادگيري_File_System::isLocal(ثابت Twine &رستو, منطقي &نتيجو) {
  نتيجو = خطأ;
  أرجع {};
}

فراغ اندريون_يادگيري_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   بنانشان IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

ساكن llvm::sys::رستو_2::Style getExistingStyle(llvm::StringRef رستو) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::رستو_2::Style style = llvm::sys::رستو_2::Style::native;
  ثابت size_t n = رستو.ڳوليو_پهريون_جو("/\\");
  // Can't distinguish between posix and windows_slash here.
  إذا (n != static_cast<size_t>(-1))
    style = (رستو[n] == '/') ? llvm::sys::رستو_2::Style::posix
                             : llvm::sys::رستو_2::Style::windows_backslash;
  أرجع style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ساكن llvm::SmallString<256> canonicalize(llvm::StringRef رستو) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::رستو_2::Style style = getExistingStyle(رستو);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> نتيجو_2 =
      llvm::sys::رستو_2::remove_leading_dotslash(رستو, style);
  llvm::sys::رستو_2::remove_dots(نتيجو_2, /*remove_dot_dot=*/سچ, style);
  أرجع نتيجو_2;
}

/// Whether the error and entry specify a file/directory that was not found.
ساكن منطقي is_File_Not_مليو(std::error_ڪوڊ EC,
                           RedirectingFileSystem::داخلا_3 *E = خالي_اشارو) {
  إذا (E && !isa<RedirectingFileSystem::Directory_Remap_داخلا>(E))
    أرجع خطأ;
  أرجع EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  أكد(ExternalFS && "RedirectingFileSystem requires an external FS");
  إذا (خودڪار ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
صنف llvm::vfs::RedirectingFSDirIterImpl
    : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::Directory_داخلا::iterator Current, پڇاڙي_2;

  std::error_ڪوڊ incrementImpl(منطقي Is_پهريون_وقت) {
    أكد((Is_پهريون_وقت || Current != پڇاڙي_2) && "cannot iterate past end");
    إذا (!Is_پهريون_وقت)
      ++Current;
    إذا (Current != پڇاڙي_2) {
      SmallString<128> رستو_Str(Dir);
      llvm::sys::رستو_2::append(رستو_Str, (*Current)->getName());
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
      Current_داخلا = directory_داخلا(std::string(رستو_Str), Type);
    } وإلا {
      Current_داخلا = directory_داخلا();
    }
    أرجع {};
  };

عام:
  RedirectingFSDirIterImpl(
      ثابت Twine &رستو, RedirectingFileSystem::Directory_داخلا::iterator Begin,
      RedirectingFileSystem::Directory_داخلا::iterator پڇاڙي_2, std::error_ڪوڊ &EC)
      : Dir(رستو.str()), Current(Begin), پڇاڙي_2(پڇاڙي_2) {
    EC = incrementImpl(/*IsFirstTime=*/سچ);
  }

  std::error_ڪوڊ increment() override {
    أرجع incrementImpl(/*IsFirstTime=*/خطأ);
  }
};

نطاق {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
صنف RedirectingFSDirRemapIterImpl : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::رستو_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

عام:
  RedirectingFSDirRemapIterImpl(std::string Dir_رستو,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_رستو)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    إذا (ExternalIter != llvm::vfs::directory_iterator())
      سيٽ_Current_داخلا();
  }

  فراغ سيٽ_Current_داخلا() {
    StringRef External_رستو = ExternalIter->رستو_2();
    llvm::sys::رستو_2::Style ExternalStyle = getExistingStyle(External_رستو);
    StringRef File = llvm::sys::رستو_2::filename(External_رستو, ExternalStyle);

    SmallString<128> New_رستو(Dir);
    llvm::sys::رستو_2::append(New_رستو, DirStyle, File);

    Current_داخلا = directory_داخلا(std::string(New_رستو), ExternalIter->type());
  }

  std::error_ڪوڊ increment() override {
    std::error_ڪوڊ EC;
    ExternalIter.increment(EC);
    إذا (!EC && ExternalIter != llvm::vfs::directory_iterator())
      سيٽ_Current_داخلا();
    وإلا
      Current_داخلا = directory_داخلا();
    أرجع EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ثابت {
  أرجع WorkingDirectory;
}

std::error_ڪوڊ
RedirectingFileSystem::سيٽ_Current_Working_Directory(ثابت Twine &رستو) {
  // Don't change the working directory if the path doesn't exist.
  إذا (!exists(رستو))
    أرجع errc::no_such_file_or_directory;

  SmallString<128> Absolute_رستو;
  رستو.منزل_ويڪٽر(Absolute_رستو);
  إذا (std::error_ڪوڊ EC = makeAbsolute(Absolute_رستو))
    أرجع EC;
  WorkingDirectory = std::string(Absolute_رستو);
  أرجع {};
}

std::error_ڪوڊ RedirectingFileSystem::isLocal(ثابت Twine &رستو_3,
                                               منطقي &نتيجو) {
  SmallString<256> رستو;
  رستو_3.منزل_ويڪٽر(رستو);

  إذا (makeAbsolute(رستو))
    أرجع {};

  أرجع ExternalFS->isLocal(رستو, نتيجو);
}

std::error_ڪوڊ RedirectingFileSystem::makeAbsolute(Small_ويڪٽر_Impl<حرف> &رستو) ثابت {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  إذا (llvm::sys::رستو_2::is_absolute(رستو, llvm::sys::رستو_2::Style::posix) ||
      llvm::sys::رستو_2::is_absolute(رستو,
                                   llvm::sys::رستو_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    أرجع {};

  خودڪار WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  أرجع makeAbsolute(WorkingDir.get(), رستو);
}

std::error_ڪوڊ
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_ويڪٽر_Impl<حرف> &رستو) ثابت {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  إذا (!WorkingDir.خالي_جايون() &&
      !sys::رستو_2::is_absolute(WorkingDir, sys::رستو_2::Style::posix) &&
      !sys::رستو_2::is_absolute(WorkingDir,
                              sys::رستو_2::Style::windows_backslash)) {
    أرجع std::error_ڪوڊ();
  }
  sys::رستو_2::Style style = sys::رستو_2::Style::windows_backslash;
  إذا (sys::رستو_2::is_absolute(WorkingDir, sys::رستو_2::Style::posix)) {
    style = sys::رستو_2::Style::posix;
  } وإلا {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    إذا (getExistingStyle(WorkingDir) != sys::رستو_2::Style::windows_backslash)
      style = sys::رستو_2::Style::windows_slash;
  }

  std::string نتيجو = std::string(WorkingDir);
  StringRef Dir(نتيجو);
  إذا (!Dir.پڇاڙيون_with(sys::رستو_2::get_separator(style))) {
    نتيجو += sys::رستو_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  نتيجو.append(رستو.معلومات(), رستو.سائيز_2());
  رستو.سونپيو(نتيجو.begin(), نتيجو.پڇاڙي());

  أرجع {};
}

directory_iterator RedirectingFileSystem::dir_begin(ثابت Twine &Dir,
                                                    std::error_ڪوڊ &EC) {
  SmallString<256> رستو;
  Dir.منزل_ويڪٽر(رستو);

  EC = makeAbsolute(رستو);
  إذا (EC)
    أرجع {};

  ErrorOr<RedirectingFileSystem::Lookup_نتيجو> نتيجو = lookup_رستو(رستو);
  إذا (!نتيجو) {
    إذا (Redirection != Redirect_قسم::RedirectOnly &&
        is_File_Not_مليو(نتيجو.getError()))
      أرجع ExternalFS->dir_begin(رستو, EC);

    EC = نتيجو.getError();
    أرجع {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<حالت> S = حالت_2(رستو, Dir, *نتيجو);
  إذا (!S) {
    إذا (Redirection != Redirect_قسم::RedirectOnly &&
        is_File_Not_مليو(S.getError(), نتيجو->E))
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
  std::error_ڪوڊ RedirectEC;
  إذا (خودڪار ExtRedirect = نتيجو->getExternalRedirect()) {
    خودڪار RE = cast<RedirectingFileSystem::Remap_داخلا>(نتيجو->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    إذا (!RE->useExternalName(Use_External_نالا)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(رستو), RedirectIter));
    }
  } وإلا {
    خودڪار DE = cast<Directory_داخلا>(نتيجو->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            رستو, DE->contents_begin(), DE->contents_پڇاڙي(), RedirectEC));
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

  std::error_ڪوڊ ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(رستو, ExternalEC);
  إذا (ExternalEC) {
    إذا (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      أرجع {};
    }
    ExternalIter = {};
  }

  Small_ويڪٽر<directory_iterator, 2> Iters;
  بدل (Redirection) {
  حالة Redirect_قسم::Fallthrough:
    Iters.ڌڪو_پٺ(ExternalIter);
    Iters.ڌڪو_پٺ(RedirectIter);
    اكسر;
  حالة Redirect_قسم::Fallback:
    Iters.ڌڪو_پٺ(RedirectIter);
    Iters.ڌڪو_پٺ(ExternalIter);
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

فراغ RedirectingFileSystem::سيٽ_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ثابت {
  أرجع OverlayFileDir;
}

فراغ RedirectingFileSystem::سيٽ_Fallthrough(منطقي Fallthrough) {
  إذا (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_قسم::Fallthrough;
  } وإلا {
    Redirection = RedirectingFileSystem::Redirect_قسم::RedirectOnly;
  }
}

فراغ RedirectingFileSystem::سيٽ_Redirection(
    RedirectingFileSystem::Redirect_قسم قسم_2) {
  Redirection = قسم_2;
}

std::ويڪٽر<StringRef> RedirectingFileSystem::getRoots() ثابت {
  std::ويڪٽر<StringRef> R;
  R.reserve(Roots.سائيز_2());
  لكل (ثابت خودڪار &پاڙ_2 : Roots)
    R.ڌڪو_پٺ(پاڙ_2->getName());
  أرجع R;
}

فراغ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      بنانشان IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_نالا ? "true" : "false") << ")\n";
  إذا (Type == PrintType::Summary)
    أرجع;

  لكل (ثابت خودڪار &پاڙ_2 : Roots)
    print_داخلا(OS, پاڙ_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

فراغ RedirectingFileSystem::print_داخلا(raw_ostream &OS,
                                       RedirectingFileSystem::داخلا_3 *E,
                                       بنانشان IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  بدل (E->get_قسم()) {
  حالة EK_Directory: {
    خودڪار *DE = cast<RedirectingFileSystem::Directory_داخلا>(E);

    OS << "\n";
    لكل (std::unique_ptr<داخلا_3> &Sub_داخلا :
         llvm::make_range(DE->contents_begin(), DE->contents_پڇاڙي()))
      print_داخلا(OS, Sub_داخلا.get(), IndentLevel + 1);
    اكسر;
  }
  حالة EK_DirectoryRemap:
  حالة EK_File: {
    خودڪار *RE = cast<RedirectingFileSystem::Remap_داخلا>(E);
    OS << " -> '" << RE->get_External_Contents_رستو() << "'";
    بدل (RE->getUseName()) {
    حالة NK_Not_سيٽ:
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

فراغ RedirectingFileSystem::گهمو_ٻار_File_Systems(گهمو_Callback_Ty Callback) {
  إذا (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->گهمو_ٻار_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
صنف llvm::vfs::RedirectingFileSystemParser {
  yaml::وهڪرو &وهڪرو;

  فراغ error(yaml::ڳنڍ *N, ثابت Twine &نياپو_2) { وهڪرو.printError(N, نياپو_2); }

  // false on error
  منطقي parseScalarString(yaml::ڳنڍ *N, StringRef &نتيجو,
                         Small_ويڪٽر_Impl<حرف> &Storage) {
    ثابت خودڪار *S = dyn_cast_if_present<yaml::Scalar_ڳنڍ>(N);

    إذا (!S) {
      error(N, "expected string");
      أرجع خطأ;
    }
    نتيجو = S->get_قدر(Storage);
    أرجع سچ;
  }

  // false on error
  منطقي parseScalarBool(yaml::ڳنڍ *N, منطقي &نتيجو) {
    SmallString<5> Storage;
    StringRef قدر;
    إذا (!parseScalarString(N, قدر, Storage))
      أرجع خطأ;

    إذا (قدر.equals_insensitive("true") || قدر.equals_insensitive("on") ||
        قدر.equals_insensitive("yes") || قدر == "1") {
      نتيجو = سچ;
      أرجع سچ;
    } وإلا إذا (قدر.equals_insensitive("false") ||
               قدر.equals_insensitive("off") ||
               قدر.equals_insensitive("no") || قدر == "0") {
      نتيجو = خطأ;
      أرجع سچ;
    }

    error(N, "expected boolean value");
    أرجع خطأ;
  }

  std::optional<RedirectingFileSystem::Redirect_قسم>
  parse_Redirect_قسم(yaml::ڳنڍ *N) {
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

  std::optional<RedirectingFileSystem::پاڙ_Relative_قسم>
  parse_پاڙ_Relative_قسم(yaml::ڳنڍ *N) {
    SmallString<12> Storage;
    StringRef قدر;
    إذا (!parseScalarString(N, قدر, Storage))
      أرجع std::nullopt;
    إذا (قدر.equals_insensitive("cwd")) {
      أرجع RedirectingFileSystem::پاڙ_Relative_قسم::CWD;
    } وإلا إذا (قدر.equals_insensitive("overlay-dir")) {
      أرجع RedirectingFileSystem::پاڙ_Relative_قسم::OverlayDir;
    }
    أرجع std::nullopt;
  }

  بنية ڪنجي_حالت {
    منطقي Required;
    منطقي ڏٺل = خطأ;

    ڪنجي_حالت(منطقي Required = خطأ) : Required(Required) {}
  };

  استعمال ڪنجي_حالت_Pair = std::pair<StringRef, ڪنجي_حالت>;

  // false on error
  منطقي check_Duplicate_Or_Unknown_ڪنجي(yaml::ڳنڍ *ڪنجي_ڳنڍ, StringRef ڪنجي,
                                  DenseMap<StringRef, ڪنجي_حالت> &ڪنجيون_2) {
    خودڪار It = ڪنجيون_2.ڳوليو(ڪنجي);
    إذا (It == ڪنجيون_2.پڇاڙي()) {
      error(ڪنجي_ڳنڍ, "unknown key");
      أرجع خطأ;
    }
    ڪنجي_حالت &S = It->ٻيو_2;
    إذا (S.ڏٺل) {
      error(ڪنجي_ڳنڍ, Twine("duplicate key '") + ڪنجي + "'");
      أرجع خطأ;
    }
    S.ڏٺل = سچ;
    أرجع سچ;
  }

  // false on error
  منطقي check_Missing_ڪنجيون(yaml::ڳنڍ *Obj, DenseMap<StringRef, ڪنجي_حالت> &ڪنجيون_2) {
    لكل (ثابت خودڪار &I : ڪنجيون_2) {
      إذا (I.ٻيو_2.Required && !I.ٻيو_2.ڏٺل) {
        error(Obj, Twine("missing key '") + I.پهريون_2 + "'");
        أرجع خطأ;
      }
    }
    أرجع سچ;
  }

عام:
  ساكن RedirectingFileSystem::داخلا_3 *
  lookup_Or_Create_داخلا(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::داخلا_3 *والد_داخلا = خالي_اشارو) {
    إذا (!والد_داخلا) { // Look for a existent root
      لكل (ثابت خودڪار &پاڙ_2 : FS->Roots) {
        إذا (Name == پاڙ_2->getName()) {
          والد_داخلا = پاڙ_2.get();
          أرجع والد_داخلا;
        }
      }
    } وإلا { // Advance to the next component
      خودڪار *DE = dyn_cast<RedirectingFileSystem::Directory_داخلا>(والد_داخلا);
      لكل (std::unique_ptr<RedirectingFileSystem::داخلا_3> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_پڇاڙي())) {
        خودڪار *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_داخلا>(Content.get());
        إذا (DirContent && Name == Content->getName())
          أرجع DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::داخلا_3> E =
        std::make_unique<RedirectingFileSystem::Directory_داخلا>(
            Name, حالت("", get_ايندڙ_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    إذا (!والد_داخلا) { // Add a new root to the overlay
      FS->Roots.ڌڪو_پٺ(std::move(E));
      والد_داخلا = FS->Roots.پٺ().get();
      أرجع والد_داخلا;
    }

    خودڪار *DE = cast<RedirectingFileSystem::Directory_داخلا>(والد_داخلا);
    DE->جوڙيو_Content(std::move(E));
    أرجع DE->get_آخري_Content();
  }

خاص:
  فراغ unique_Overlay_وڻ(RedirectingFileSystem *FS,
                         RedirectingFileSystem::داخلا_3 *SrcE,
                         RedirectingFileSystem::داخلا_3 *New_والد_E = خالي_اشارو) {
    StringRef Name = SrcE->getName();
    بدل (SrcE->get_قسم()) {
    حالة RedirectingFileSystem::EK_Directory: {
      خودڪار *DE = cast<RedirectingFileSystem::Directory_داخلا>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      إذا (!Name.خالي_جايون())
        New_والد_E = lookup_Or_Create_داخلا(FS, Name, New_والد_E);
      لكل (std::unique_ptr<RedirectingFileSystem::داخلا_3> &Sub_داخلا :
           llvm::make_range(DE->contents_begin(), DE->contents_پڇاڙي()))
        unique_Overlay_وڻ(FS, Sub_داخلا.get(), New_والد_E);
      اكسر;
    }
    حالة RedirectingFileSystem::EK_DirectoryRemap: {
      أكد(New_والد_E && "Parent entry must exist");
      خودڪار *DR = cast<RedirectingFileSystem::Directory_Remap_داخلا>(SrcE);
      خودڪار *DE = cast<RedirectingFileSystem::Directory_داخلا>(New_والد_E);
      DE->جوڙيو_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_داخلا>(
              Name, DR->get_External_Contents_رستو(), DR->getUseName()));
      اكسر;
    }
    حالة RedirectingFileSystem::EK_File: {
      أكد(New_والد_E && "Parent entry must exist");
      خودڪار *FE = cast<RedirectingFileSystem::File_داخلا>(SrcE);
      خودڪار *DE = cast<RedirectingFileSystem::Directory_داخلا>(New_والد_E);
      DE->جوڙيو_Content(std::make_unique<RedirectingFileSystem::File_داخلا>(
          Name, FE->get_External_Contents_رستو(), FE->getUseName()));
      اكسر;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::داخلا_3>
  parse_داخلا(yaml::ڳنڍ *N, RedirectingFileSystem *FS, منطقي Is_پاڙ_داخلا) {
    خودڪار *M = dyn_cast<yaml::Mapping_ڳنڍ>(N);
    إذا (!M) {
      error(N, "expected mapping node for file or directory entry");
      أرجع خالي_اشارو;
    }

    ڪنجي_حالت_Pair Fields[] = {
        ڪنجي_حالت_Pair("name", سچ),
        ڪنجي_حالت_Pair("type", سچ),
        ڪنجي_حالت_Pair("contents", خطأ),
        ڪنجي_حالت_Pair("external-contents", خطأ),
        ڪنجي_حالت_Pair("use-external-name", خطأ),
    };

    DenseMap<StringRef, ڪنجي_حالت> ڪنجيون_2(std::begin(Fields), std::پڇاڙي(Fields));

    تعداد { CF_Not_سيٽ, CF_List, CF_External } Contents_ميدان = CF_Not_سيٽ;
    std::ويڪٽر<std::unique_ptr<RedirectingFileSystem::داخلا_3>>
        داخلا_Array_Contents;
    SmallString<256> External_Contents_رستو;
    SmallString<256> Name;
    yaml::ڳنڍ *Name_قدر_ڳنڍ = خالي_اشارو;
    خودڪار UseExternalName = RedirectingFileSystem::NK_Not_سيٽ;
    RedirectingFileSystem::داخلا_قسم قسم_2;

    لكل (خودڪار &I : *M) {
      StringRef ڪنجي;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> بفر;
      إذا (!parseScalarString(I.get_ڪنجي(), ڪنجي, بفر))
        أرجع خالي_اشارو;

      إذا (!check_Duplicate_Or_Unknown_ڪنجي(I.get_ڪنجي(), ڪنجي, ڪنجيون_2))
        أرجع خالي_اشارو;

      StringRef قدر;
      إذا (ڪنجي == "name") {
        إذا (!parseScalarString(I.get_قدر(), قدر, بفر))
          أرجع خالي_اشارو;

        Name_قدر_ڳنڍ = I.get_قدر();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(قدر).str();
      } وإلا إذا (ڪنجي == "type") {
        إذا (!parseScalarString(I.get_قدر(), قدر, بفر))
          أرجع خالي_اشارو;
        إذا (قدر == "file")
          قسم_2 = RedirectingFileSystem::EK_File;
        وإلا إذا (قدر == "directory")
          قسم_2 = RedirectingFileSystem::EK_Directory;
        وإلا إذا (قدر == "directory-remap")
          قسم_2 = RedirectingFileSystem::EK_DirectoryRemap;
        وإلا {
          error(I.get_قدر(), "unknown value for 'type'");
          أرجع خالي_اشارو;
        }
      } وإلا إذا (ڪنجي == "contents") {
        إذا (Contents_ميدان != CF_Not_سيٽ) {
          error(I.get_ڪنجي(),
                "entry already has 'contents' or 'external-contents'");
          أرجع خالي_اشارو;
        }
        Contents_ميدان = CF_List;
        خودڪار *Contents = dyn_cast_if_present<yaml::Sequence_ڳنڍ>(I.get_قدر());
        إذا (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_قدر(), "expected array");
          أرجع خالي_اشارو;
        }

        لكل (خودڪار &I : *Contents) {
          إذا (std::unique_ptr<RedirectingFileSystem::داخلا_3> E =
                  parse_داخلا(&I, FS, /*IsRootEntry*/ خطأ))
            داخلا_Array_Contents.ڌڪو_پٺ(std::move(E));
          وإلا
            أرجع خالي_اشارو;
        }
      } وإلا إذا (ڪنجي == "external-contents") {
        إذا (Contents_ميدان != CF_Not_سيٽ) {
          error(I.get_ڪنجي(),
                "entry already has 'contents' or 'external-contents'");
          أرجع خالي_اشارو;
        }
        Contents_ميدان = CF_External;
        إذا (!parseScalarString(I.get_قدر(), قدر, بفر))
          أرجع خالي_اشارو;

        SmallString<256> ڀريل_رستو;
        إذا (FS->IsRelativeOverlay) {
          ڀريل_رستو = FS->getOverlayFileDir();
          أكد(!ڀريل_رستو.خالي_جايون() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_ڀريل_رستو = قدر;
          إذا (FS->makeAbsolute(ڀريل_رستو, Abs_ڀريل_رستو)) {
            error(N, "failed to make 'external-contents' absolute");
            أرجع خالي_اشارو;
          }
          ڀريل_رستو = Abs_ڀريل_رستو;
        } وإلا {
          ڀريل_رستو = قدر;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        ڀريل_رستو = canonicalize(ڀريل_رستو);
        External_Contents_رستو = ڀريل_رستو.str();
      } وإلا إذا (ڪنجي == "use-external-name") {
        منطقي Val;
        إذا (!parseScalarBool(I.get_قدر(), Val))
          أرجع خالي_اشارو;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (وهڪرو.failed())
      أرجع خالي_اشارو;

    // check for missing keys
    إذا (Contents_ميدان == CF_Not_سيٽ) {
      error(N, "missing key 'contents' or 'external-contents'");
      أرجع خالي_اشارو;
    }
    إذا (!check_Missing_ڪنجيون(N, ڪنجيون_2))
      أرجع خالي_اشارو;

    // check invalid configuration
    إذا (قسم_2 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_سيٽ) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      أرجع خالي_اشارو;
    }

    إذا (قسم_2 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_ميدان == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      أرجع خالي_اشارو;
    }

    sys::رستو_2::Style رستو_style = sys::رستو_2::Style::native;
    إذا (Is_پاڙ_داخلا) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      إذا (sys::رستو_2::is_absolute(Name, sys::رستو_2::Style::posix)) {
        رستو_style = sys::رستو_2::Style::posix;
      } وإلا إذا (sys::رستو_2::is_absolute(Name,
                                        sys::رستو_2::Style::windows_backslash)) {
        رستو_style = sys::رستو_2::Style::windows_backslash;
      } وإلا {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_ڪوڊ EC;
        إذا (FS->پاڙ_Relative ==
            RedirectingFileSystem::پاڙ_Relative_قسم::OverlayDir) {
          StringRef ڀريل_رستو = FS->getOverlayFileDir();
          أكد(!ڀريل_رستو.خالي_جايون() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(ڀريل_رستو, Name);
          Name = canonicalize(Name);
        } وإلا {
          EC = FS->makeAbsolute(Name);
        }
        إذا (EC) {
          أكد(Name_قدر_ڳنڍ && "Name presence should be checked earlier");
          error(
              Name_قدر_ڳنڍ,
              "entry with relative path at the root level is not discoverable");
          أرجع خالي_اشارو;
        }
        رستو_style = sys::رستو_2::is_absolute(Name, sys::رستو_2::Style::posix)
                         ? sys::رستو_2::Style::posix
                         : sys::رستو_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      إذا (رستو_style == sys::رستو_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::رستو_2::Style::windows_backslash)
        رستو_style = sys::رستو_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t پاڙ_رستو_Len = sys::رستو_2::پاڙ_رستو_2(Trimmed, رستو_style).سائيز_2();
    طالما (Trimmed.سائيز_2() > پاڙ_رستو_Len &&
           sys::رستو_2::is_separator(Trimmed.پٺ(), رستو_style))
      Trimmed = Trimmed.ٽڪرو(0, Trimmed.سائيز_2() - 1);

    // Get the last component
    StringRef آخري_Component = sys::رستو_2::filename(Trimmed, رستو_style);

    std::unique_ptr<RedirectingFileSystem::داخلا_3> نتيجو;
    بدل (قسم_2) {
    حالة RedirectingFileSystem::EK_File:
      نتيجو = std::make_unique<RedirectingFileSystem::File_داخلا>(
          آخري_Component, std::move(External_Contents_رستو), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_DirectoryRemap:
      نتيجو = std::make_unique<RedirectingFileSystem::Directory_Remap_داخلا>(
          آخري_Component, std::move(External_Contents_رستو), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_Directory:
      نتيجو = std::make_unique<RedirectingFileSystem::Directory_داخلا>(
          آخري_Component, std::move(داخلا_Array_Contents),
          حالت("", get_ايندڙ_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      اكسر;
    }

    StringRef والد_2 = sys::رستو_2::والد_رستو(Trimmed, رستو_style);
    إذا (والد_2.خالي_جايون())
      أرجع نتيجو;

    // if 'name' contains multiple components, create implicit directory entries
    لكل (sys::رستو_2::ابتو_iterator I = sys::رستو_2::rbegin(والد_2, رستو_style),
                                     E = sys::رستو_2::rend(والد_2);
         I != E; ++I) {
      std::ويڪٽر<std::unique_ptr<RedirectingFileSystem::داخلا_3>> Entries;
      Entries.ڌڪو_پٺ(std::move(نتيجو));
      نتيجو = std::make_unique<RedirectingFileSystem::Directory_داخلا>(
          *I, std::move(Entries),
          حالت("", get_ايندڙ_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    أرجع نتيجو;
  }

عام:
  RedirectingFileSystemParser(yaml::وهڪرو &S) : وهڪرو(S) {}

  // false on error
  منطقي parse(yaml::ڳنڍ *پاڙ_2, RedirectingFileSystem *FS) {
    خودڪار *چوٽي = dyn_cast<yaml::Mapping_ڳنڍ>(پاڙ_2);
    إذا (!چوٽي) {
      error(پاڙ_2, "expected mapping node");
      أرجع خطأ;
    }

    ڪنجي_حالت_Pair Fields[] = {
        ڪنجي_حالت_Pair("version", سچ),
        ڪنجي_حالت_Pair("case-sensitive", خطأ),
        ڪنجي_حالت_Pair("use-external-names", خطأ),
        ڪنجي_حالت_Pair("root-relative", خطأ),
        ڪنجي_حالت_Pair("overlay-relative", خطأ),
        ڪنجي_حالت_Pair("fallthrough", خطأ),
        ڪنجي_حالت_Pair("redirecting-with", خطأ),
        ڪنجي_حالت_Pair("roots", سچ),
    };

    DenseMap<StringRef, ڪنجي_حالت> ڪنجيون_2(std::begin(Fields), std::پڇاڙي(Fields));
    std::ويڪٽر<std::unique_ptr<RedirectingFileSystem::داخلا_3>> پاڙ_Entries;

    // Parse configuration and 'roots'
    لكل (خودڪار &I : *چوٽي) {
      SmallString<10> ڪنجي_بفر;
      StringRef ڪنجي;
      إذا (!parseScalarString(I.get_ڪنجي(), ڪنجي, ڪنجي_بفر))
        أرجع خطأ;

      إذا (!check_Duplicate_Or_Unknown_ڪنجي(I.get_ڪنجي(), ڪنجي, ڪنجيون_2))
        أرجع خطأ;

      إذا (ڪنجي == "roots") {
        خودڪار *Roots = dyn_cast_if_present<yaml::Sequence_ڳنڍ>(I.get_قدر());
        إذا (!Roots) {
          error(I.get_قدر(), "expected array");
          أرجع خطأ;
        }

        لكل (خودڪار &I : *Roots) {
          إذا (std::unique_ptr<RedirectingFileSystem::داخلا_3> E =
                  parse_داخلا(&I, FS, /*IsRootEntry*/ سچ))
            پاڙ_Entries.ڌڪو_پٺ(std::move(E));
          وإلا
            أرجع خطأ;
        }
      } وإلا إذا (ڪنجي == "version") {
        StringRef نسخو_String;
        SmallString<4> Storage;
        إذا (!parseScalarString(I.get_قدر(), نسخو_String, Storage))
          أرجع خطأ;
        صحيح نسخو;
        إذا (نسخو_String.getAsInteger<صحيح>(10, نسخو)) {
          error(I.get_قدر(), "expected integer");
          أرجع خطأ;
        }
        إذا (نسخو < 0) {
          error(I.get_قدر(), "invalid version number");
          أرجع خطأ;
        }
        إذا (نسخو != 0) {
          error(I.get_قدر(), "version mismatch, expected 0");
          أرجع خطأ;
        }
      } وإلا إذا (ڪنجي == "case-sensitive") {
        إذا (!parseScalarBool(I.get_قدر(), FS->CaseSensitive))
          أرجع خطأ;
      } وإلا إذا (ڪنجي == "overlay-relative") {
        إذا (!parseScalarBool(I.get_قدر(), FS->IsRelativeOverlay))
          أرجع خطأ;
      } وإلا إذا (ڪنجي == "use-external-names") {
        إذا (!parseScalarBool(I.get_قدر(), FS->Use_External_نالا))
          أرجع خطأ;
      } وإلا إذا (ڪنجي == "fallthrough") {
        إذا (ڪنجيون_2["redirecting-with"].ڏٺل) {
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
      } وإلا إذا (ڪنجي == "redirecting-with") {
        إذا (ڪنجيون_2["fallthrough"].ڏٺل) {
          error(I.get_قدر(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        إذا (خودڪار قسم_2 = parse_Redirect_قسم(I.get_قدر())) {
          FS->Redirection = *قسم_2;
        } وإلا {
          error(I.get_قدر(), "expected valid redirect kind");
          أرجع خطأ;
        }
      } وإلا إذا (ڪنجي == "root-relative") {
        إذا (خودڪار قسم_2 = parse_پاڙ_Relative_قسم(I.get_قدر())) {
          FS->پاڙ_Relative = *قسم_2;
        } وإلا {
          error(I.get_قدر(), "expected valid root-relative kind");
          أرجع خطأ;
        }
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (وهڪرو.failed())
      أرجع خطأ;

    إذا (!check_Missing_ڪنجيون(چوٽي, ڪنجيون_2))
      أرجع خطأ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    لكل (خودڪار &E : پاڙ_Entries)
      unique_Overlay_وڻ(FS, E.get());

    أرجع سچ;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<يادگيري_بفر> بفر,
                              ذريعو_Mgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_رستو, فراغ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ذريعو_Mgr SM;
  yaml::وهڪرو وهڪرو(بفر->get_Mem_بفر_Ref(), SM);

  SM.سيٽ_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = وهڪرو.begin();
  yaml::ڳنڍ *پاڙ_2 = DI->get_پاڙ();
  إذا (DI == وهڪرو.پڇاڙي() || !پاڙ_2) {
    SM.Print_پيغام(SMLoc(), ذريعو_Mgr::DK_Error, "expected root node");
    أرجع خالي_اشارو;
  }

  RedirectingFileSystemParser P(وهڪرو);

  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));

  إذا (!YAMLFile_رستو.خالي_جايون()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::رستو_2::والد_رستو(YAMLFile_رستو);
    std::error_ڪوڊ EC = FS->makeAbsolute(OverlayAbsDir);
    أكد(!EC && "Overlay dir final path must be absolute");
    (فراغ)EC;
    FS->سيٽ_Overlay_File_Dir(OverlayAbsDir);
  }

  إذا (!P.parse(پاڙ_2, FS.get()))
    أرجع خالي_اشارو;

  أرجع FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    منطقي Use_External_نالا, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));
  FS->Use_External_نالا = Use_External_نالا;

  StringMap<RedirectingFileSystem::داخلا_3 *> Entries;

  لكل (خودڪار &Mapping : llvm::ابتو(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.پهريون_2);
    SmallString<128> منزل = StringRef(Mapping.ٻيو_2);
    {
      خودڪار EC = ExternalFS->makeAbsolute(From);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::داخلا_3 *&منزل_داخلا = Entries[From];
    إذا (منزل_داخلا)
      استمر;

    // Add parent directories.
    RedirectingFileSystem::داخلا_3 *والد_2 = خالي_اشارو;
    StringRef FromDirectory = llvm::sys::رستو_2::والد_رستو(From);
    لكل (خودڪار I = llvm::sys::رستو_2::begin(FromDirectory),
              E = llvm::sys::رستو_2::پڇاڙي(FromDirectory);
         I != E; ++I) {
      والد_2 = RedirectingFileSystemParser::lookup_Or_Create_داخلا(FS.get(), *I,
                                                                والد_2);
    }
    أكد(والد_2 && "File without a directory?");
    {
      خودڪار EC = ExternalFS->makeAbsolute(منزل);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Add the file.
    خودڪار NewFile = std::make_unique<RedirectingFileSystem::File_داخلا>(
        llvm::sys::رستو_2::filename(From), منزل,
        Use_External_نالا ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    منزل_داخلا = NewFile.get();
    cast<RedirectingFileSystem::Directory_داخلا>(والد_2)->جوڙيو_Content(
        std::move(NewFile));
  }

  أرجع FS;
}

RedirectingFileSystem::Lookup_نتيجو::Lookup_نتيجو(
    داخلا_3 *E, sys::رستو_2::const_iterator شروعات, sys::رستو_2::const_iterator پڇاڙي_2)
    : E(E) {
  أكد(E != خالي_اشارو);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  إذا (خودڪار *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_داخلا>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_رستو());
    sys::رستو_2::append(Redirect, شروعات, پڇاڙي_2,
                      getExistingStyle(DRE->get_External_Contents_رستو()));
    ExternalRedirect = std::string(Redirect);
  }
}

فراغ RedirectingFileSystem::Lookup_نتيجو::get_رستو(
    llvm::Small_ويڪٽر_Impl<حرف> &نتيجو) ثابت {
  نتيجو.clear();
  لكل (داخلا_3 *والد_2 : Parents)
    llvm::sys::رستو_2::append(نتيجو, والد_2->getName());
  llvm::sys::رستو_2::append(نتيجو, E->getName());
}

std::error_ڪوڊ RedirectingFileSystem::makeCanonicalForLookup(
    Small_ويڪٽر_Impl<حرف> &رستو) ثابت {
  إذا (std::error_ڪوڊ EC = makeAbsolute(رستو))
    أرجع EC;

  llvm::SmallString<256> Canonical_رستو =
      canonicalize(StringRef(رستو.معلومات(), رستو.سائيز_2()));
  إذا (Canonical_رستو.خالي_جايون())
    أرجع make_error_ڪوڊ(llvm::errc::invalid_دليل);

  رستو.سونپيو(Canonical_رستو.begin(), Canonical_رستو.پڇاڙي());
  أرجع {};
}

ErrorOr<RedirectingFileSystem::Lookup_نتيجو>
RedirectingFileSystem::lookup_رستو(StringRef رستو) ثابت {
  llvm::SmallString<128> Canonical_رستو(رستو);
  إذا (std::error_ڪوڊ EC = makeCanonicalForLookup(Canonical_رستو))
    أرجع EC;

  // RedirectOnly means the VFS is always used.
  إذا (Usage_Tracking_سرگرم && Redirection == Redirect_قسم::RedirectOnly)
    Has_Been_استعمال_ٿيل = سچ;

  sys::رستو_2::const_iterator شروعات = sys::رستو_2::begin(Canonical_رستو);
  sys::رستو_2::const_iterator پڇاڙي_2 = sys::رستو_2::پڇاڙي(Canonical_رستو);
  llvm::Small_ويڪٽر<داخلا_3 *, 32> Entries;
  لكل (ثابت خودڪار &پاڙ_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_نتيجو> نتيجو =
        lookup_رستو_Impl(شروعات, پڇاڙي_2, پاڙ_2.get(), Entries);
    إذا (Usage_Tracking_سرگرم && نتيجو && isa<Remap_داخلا>(نتيجو->E))
      Has_Been_استعمال_ٿيل = سچ;
    إذا (نتيجو) {
      نتيجو->Parents = std::move(Entries);
      أرجع نتيجو;
    }

    إذا (نتيجو.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتيجو;
  }
  أرجع make_error_ڪوڊ(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_نتيجو>
RedirectingFileSystem::lookup_رستو_Impl(
    sys::رستو_2::const_iterator شروعات, sys::رستو_2::const_iterator پڇاڙي_2,
    RedirectingFileSystem::داخلا_3 *From,
    llvm::Small_ويڪٽر_Impl<داخلا_3 *> &Entries) ثابت {
  أكد(!isTraversalComponent(*شروعات) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  إذا (!FromName.خالي_جايون()) {
    إذا (!رستو_Component_Matches(*شروعات, FromName))
      أرجع make_error_ڪوڊ(llvm::errc::no_such_file_or_directory);

    ++شروعات;

    إذا (شروعات == پڇاڙي_2) {
      // Match!
      أرجع Lookup_نتيجو(From, شروعات, پڇاڙي_2);
    }
  }

  إذا (isa<RedirectingFileSystem::File_داخلا>(From))
    أرجع make_error_ڪوڊ(llvm::errc::not_a_directory);

  إذا (isa<RedirectingFileSystem::Directory_Remap_داخلا>(From))
    أرجع Lookup_نتيجو(From, شروعات, پڇاڙي_2);

  خودڪار *DE = cast<RedirectingFileSystem::Directory_داخلا>(From);
  لكل (ثابت std::unique_ptr<RedirectingFileSystem::داخلا_3> &Dir_داخلا :
       llvm::make_range(DE->contents_begin(), DE->contents_پڇاڙي())) {
    Entries.ڌڪو_پٺ(From);
    ErrorOr<RedirectingFileSystem::Lookup_نتيجو> نتيجو =
        lookup_رستو_Impl(شروعات, پڇاڙي_2, Dir_داخلا.get(), Entries);
    إذا (نتيجو || نتيجو.getError() != llvm::errc::no_such_file_or_directory)
      أرجع نتيجو;
    Entries.ڪڍو_پٺ();
  }

  أرجع make_error_ڪوڊ(llvm::errc::no_such_file_or_directory);
}

ساكن حالت get_Redirected_File_حالت(ثابت Twine &Original_رستو,
                                      منطقي Use_External_نالا,
                                      حالت External_حالت) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  إذا (External_حالت.ExposesExternalVFSPath)
    أرجع External_حالت;

  حالت S = External_حالت;
  إذا (!Use_External_نالا)
    S = حالت::copyWithNewName(S, Original_رستو);
  وإلا
    S.ExposesExternalVFSPath = سچ;
  أرجع S;
}

ErrorOr<حالت> RedirectingFileSystem::حالت_2(
    ثابت Twine &Lookup_رستو, ثابت Twine &Original_رستو,
    ثابت RedirectingFileSystem::Lookup_نتيجو &نتيجو) {
  إذا (std::optional<StringRef> ExtRedirect = نتيجو.getExternalRedirect()) {
    SmallString<256> Remapped_رستو((*ExtRedirect).str());
    إذا (std::error_ڪوڊ EC = makeAbsolute(Remapped_رستو))
      أرجع EC;

    ErrorOr<حالت> S = ExternalFS->حالت_2(Remapped_رستو);
    إذا (!S)
      أرجع S;
    S = حالت::copyWithNewName(*S, *ExtRedirect);
    خودڪار *RE = cast<RedirectingFileSystem::Remap_داخلا>(نتيجو.E);
    أرجع get_Redirected_File_حالت(Original_رستو,
                                   RE->useExternalName(Use_External_نالا), *S);
  }

  خودڪار *DE = cast<RedirectingFileSystem::Directory_داخلا>(نتيجو.E);
  أرجع حالت::copyWithNewName(DE->get_حالت(), Lookup_رستو);
}

ErrorOr<حالت>
RedirectingFileSystem::get_External_حالت(ثابت Twine &Lookup_رستو,
                                         ثابت Twine &Original_رستو) ثابت {
  خودڪار نتيجو = ExternalFS->حالت_2(Lookup_رستو);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  إذا (!نتيجو || نتيجو->ExposesExternalVFSPath)
    أرجع نتيجو;
  أرجع حالت::copyWithNewName(نتيجو.get(), Original_رستو);
}

ErrorOr<حالت> RedirectingFileSystem::حالت_2(ثابت Twine &Original_رستو) {
  SmallString<256> رستو;
  Original_رستو.منزل_ويڪٽر(رستو);

  إذا (std::error_ڪوڊ EC = makeAbsolute(رستو))
    أرجع EC;

  إذا (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<حالت> S = get_External_حالت(رستو, Original_رستو);
    إذا (S)
      أرجع S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتيجو> نتيجو = lookup_رستو(رستو);
  إذا (!نتيجو) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_مليو(نتيجو.getError()))
      أرجع get_External_حالت(رستو, Original_رستو);
    أرجع نتيجو.getError();
  }

  ErrorOr<حالت> S = حالت_2(رستو, Original_رستو, *نتيجو);
  إذا (!S && Redirection == Redirect_قسم::Fallthrough &&
      is_File_Not_مليو(S.getError(), نتيجو->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع get_External_حالت(رستو, Original_رستو);
  }

  أرجع S;
}

منطقي RedirectingFileSystem::exists(ثابت Twine &Original_رستو) {
  SmallString<256> رستو;
  Original_رستو.منزل_ويڪٽر(رستو);

  إذا (makeAbsolute(رستو))
    أرجع خطأ;

  إذا (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    إذا (ExternalFS->exists(رستو))
      أرجع سچ;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتيجو> نتيجو = lookup_رستو(رستو);
  إذا (!نتيجو) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_مليو(نتيجو.getError()))
      أرجع ExternalFS->exists(رستو);
    أرجع خطأ;
  }

  std::optional<StringRef> ExtRedirect = نتيجو->getExternalRedirect();
  إذا (!ExtRedirect) {
    أكد(isa<RedirectingFileSystem::Directory_داخلا>(نتيجو->E));
    أرجع سچ;
  }

  SmallString<256> Remapped_رستو((*ExtRedirect).str());
  إذا (makeAbsolute(Remapped_رستو))
    أرجع خطأ;

  إذا (ExternalFS->exists(Remapped_رستو))
    أرجع سچ;

  إذا (Redirection == Redirect_قسم::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع ExternalFS->exists(رستو);
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
  ErrorOr<std::unique_ptr<llvm::يادگيري_بفر>>

  get_بفر(ثابت Twine &Name, int64_t File_سائيز, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    أرجع InnerFile->get_بفر(Name, File_سائيز, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_ڪوڊ close() override { أرجع InnerFile->close(); }

  فراغ سيٽ_رستو(ثابت Twine &رستو) override { S = S.copyWithNewName(S, رستو); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_رستو(ErrorOr<std::unique_ptr<File>> نتيجو, ثابت Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  إذا (!نتيجو || (*نتيجو)->حالت_2()->ExposesExternalVFSPath)
    أرجع نتيجو;

  ErrorOr<std::unique_ptr<File>> F = std::move(*نتيجو);
  خودڪار Name = F->get()->getName();
  إذا (Name && Name.get() != P.str())
    F->get()->سيٽ_رستو(P);
  أرجع F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ثابت Twine &Original_رستو) {
  SmallString<256> رستو;
  Original_رستو.منزل_ويڪٽر(رستو);

  إذا (std::error_ڪوڊ EC = makeAbsolute(رستو))
    أرجع EC;

  إذا (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    خودڪار F = File::get_With_رستو(ExternalFS->openFileForRead(رستو), Original_رستو);
    إذا (F)
      أرجع F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتيجو> نتيجو = lookup_رستو(رستو);
  إذا (!نتيجو) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_مليو(نتيجو.getError()))
      أرجع File::get_With_رستو(ExternalFS->openFileForRead(رستو), Original_رستو);
    أرجع نتيجو.getError();
  }

  إذا (!نتيجو->getExternalRedirect()) // FIXME: errc::not_a_file?
    أرجع make_error_ڪوڊ(llvm::errc::invalid_دليل);

  StringRef ExtRedirect = *نتيجو->getExternalRedirect();
  SmallString<256> Remapped_رستو(ExtRedirect.str());
  إذا (std::error_ڪوڊ EC = makeAbsolute(Remapped_رستو))
    أرجع EC;

  خودڪار *RE = cast<RedirectingFileSystem::Remap_داخلا>(نتيجو->E);

  خودڪار ExternalFile =
      File::get_With_رستو(ExternalFS->openFileForRead(Remapped_رستو), ExtRedirect);
  إذا (!ExternalFile) {
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_مليو(ExternalFile.getError(), نتيجو->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع File::get_With_رستو(ExternalFS->openFileForRead(رستو), Original_رستو);
    }
    أرجع ExternalFile;
  }

  خودڪار External_حالت = (*ExternalFile)->حالت_2();
  إذا (!External_حالت)
    أرجع External_حالت.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  حالت S = get_Redirected_File_حالت(
      Original_رستو, RE->useExternalName(Use_External_نالا), *External_حالت);
  أرجع std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_حالت>(std::move(*ExternalFile), S));
}

std::error_ڪوڊ
RedirectingFileSystem::get_Real_رستو(ثابت Twine &Original_رستو,
                                   Small_ويڪٽر_Impl<حرف> &Output) {
  SmallString<256> رستو;
  Original_رستو.منزل_ويڪٽر(رستو);

  إذا (std::error_ڪوڊ EC = makeAbsolute(رستو))
    أرجع EC;

  إذا (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_ڪوڊ EC = ExternalFS->get_Real_رستو(رستو, Output);
    إذا (!EC)
      أرجع EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتيجو> نتيجو = lookup_رستو(رستو);
  إذا (!نتيجو) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_مليو(نتيجو.getError()))
      أرجع ExternalFS->get_Real_رستو(رستو, Output);
    أرجع نتيجو.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  إذا (خودڪار ExtRedirect = نتيجو->getExternalRedirect()) {
    خودڪار P = ExternalFS->get_Real_رستو(*ExtRedirect, Output);
    إذا (P && Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_مليو(P, نتيجو->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع ExternalFS->get_Real_رستو(رستو, Output);
    }
    أرجع P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  إذا (Redirection == Redirect_قسم::Fallthrough) {
    نتيجو->get_رستو(Output);
    أرجع {};
  }
  أرجع llvm::errc::invalid_دليل;
}

فراغ RedirectingFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_ويڪٽر_Impl<std::string> &ٻاهريون) {
  SmallString<256> رستو;
  Dir.منزل_ويڪٽر(رستو);

  إذا (makeAbsolute(رستو))
    أرجع;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ثابت منطقي ConsultsExternalFS = Redirection != Redirect_قسم::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_نتيجو> نتيجو = lookup_رستو(رستو);
  إذا (!نتيجو) {
    // dir_begin() delegates entirely to ExternalFS.
    إذا (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(رستو, ٻاهريون);
    أرجع;
  }

  بدل (نتيجو->E->get_قسم()) {
  حالة EK_File:
    أرجع;
  حالة EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    اكسر;
  حالة EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_رستو(*نتيجو->getExternalRedirect());
    إذا (!makeAbsolute(Remapped_رستو))
      ExternalFS->getDirectoryContentRealSources(Remapped_رستو, ٻاهريون);
    اكسر;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  إذا (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(رستو, ٻاهريون);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<يادگيري_بفر> بفر,
                    ذريعو_Mgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_رستو, فراغ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  أرجع RedirectingFileSystem::create(std::move(بفر), DiagHandler,
                                       YAMLFile_رستو, DiagContext,
                                       std::move(ExternalFS));
}

ساكن فراغ getVFSEntries(RedirectingFileSystem::داخلا_3 *SrcE,
                          Small_ويڪٽر_Impl<StringRef> &رستو,
                          Small_ويڪٽر_Impl<YAMLVFSEntry> &Entries) {
  خودڪار قسم_2 = SrcE->get_قسم();
  إذا (قسم_2 == RedirectingFileSystem::EK_Directory) {
    خودڪار *DE = dyn_cast<RedirectingFileSystem::Directory_داخلا>(SrcE);
    أكد(DE && "Must be a directory");
    لكل (std::unique_ptr<RedirectingFileSystem::داخلا_3> &Sub_داخلا :
         llvm::make_range(DE->contents_begin(), DE->contents_پڇاڙي())) {
      رستو.ڌڪو_پٺ(Sub_داخلا->getName());
      getVFSEntries(Sub_داخلا.get(), رستو, Entries);
      رستو.ڪڍو_پٺ();
    }
    أرجع;
  }

  إذا (قسم_2 == RedirectingFileSystem::EK_DirectoryRemap) {
    خودڪار *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_داخلا>(SrcE);
    أكد(DR && "Must be a directory remap");
    SmallString<128> VPath;
    لكل (خودڪار &Comp : رستو)
      llvm::sys::رستو_2::append(VPath, Comp);
    Entries.ڌڪو_پٺ(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_رستو()));
    أرجع;
  }

  أكد(قسم_2 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  خودڪار *FE = dyn_cast<RedirectingFileSystem::File_داخلا>(SrcE);
  أكد(FE && "Must be a file");
  SmallString<128> VPath;
  لكل (خودڪار &Comp : رستو)
    llvm::sys::رستو_2::append(VPath, Comp);
  Entries.ڌڪو_پٺ(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_رستو()));
}

فراغ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_ويڪٽر_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_نتيجو> پاڙ_نتيجو = VFS.lookup_رستو("/");
  إذا (!پاڙ_نتيجو)
    أرجع;
  Small_ويڪٽر<StringRef, 8> Components;
  Components.ڌڪو_پٺ("/");
  getVFSEntries(پاڙ_نتيجو->E, Components, CollectedEntries);
}

UniqueID vfs::get_ايندڙ_Virtual_Unique_ID() {
  ساكن std::atomic<بنانشان> UID;
  بنانشان ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  أرجع UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

فراغ YAMLVFSWriter::جوڙيو_داخلا(StringRef Virtual_رستو, StringRef Real_رستو,
                             منطقي IsDirectory) {
  أكد(sys::رستو_2::is_absolute(Virtual_رستو) && "virtual path not absolute");
  أكد(sys::رستو_2::is_absolute(Real_رستو) && "real path not absolute");
  أكد(!رستو_Has_Traversal(Virtual_رستو) && "path traversal is not supported");
  Mappings.emplace_پٺ(Virtual_رستو, Real_رستو, IsDirectory);
}

فراغ YAMLVFSWriter::جوڙيو_File_Mapping(StringRef Virtual_رستو, StringRef Real_رستو) {
  جوڙيو_داخلا(Virtual_رستو, Real_رستو, /*IsDirectory=*/خطأ);
}

فراغ YAMLVFSWriter::جوڙيو_Directory_Mapping(StringRef Virtual_رستو,
                                        StringRef Real_رستو) {
  جوڙيو_داخلا(Virtual_رستو, Real_رستو, /*IsDirectory=*/سچ);
}

نطاق {

صنف JSONWriter {
  llvm::raw_ostream &OS;
  Small_ويڪٽر<StringRef, 16> Dir_ڍير;

  بنانشان getDirIndent() { أرجع 4 * Dir_ڍير.سائيز_2(); }
  بنانشان getFileIndent() { أرجع 4 * (Dir_ڍير.سائيز_2() + 1); }
  منطقي contained_اندريون(StringRef والد_2, StringRef رستو);
  StringRef containedPart(StringRef والد_2, StringRef رستو);
  فراغ شروعات_Directory(StringRef رستو);
  فراغ پڇاڙي_Directory();
  فراغ write_داخلا(StringRef VPath, StringRef RPath);

عام:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  فراغ اكتب(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<منطقي> Use_External_نالا,
             std::optional<منطقي> IsCaseSensitive,
             std::optional<منطقي> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

منطقي JSONWriter::contained_اندريون(StringRef والد_2, StringRef رستو) {
  استعمال نطاق llvm::sys;

  // Compare each path component.
  خودڪار IParent = رستو_2::begin(والد_2), EParent = رستو_2::پڇاڙي(والد_2);
  لكل (خودڪار IChild = رستو_2::begin(رستو), EChild = رستو_2::پڇاڙي(رستو);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    إذا (*IParent != *IChild)
      أرجع خطأ;
  }
  // Have we exhausted the parent path?
  أرجع IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef والد_2, StringRef رستو) {
  أكد(!والد_2.خالي_جايون());
  أكد(contained_اندريون(والد_2, رستو));
  أرجع رستو.substr(والد_2.سائيز_2() + 1);
}

فراغ JSONWriter::شروعات_Directory(StringRef رستو) {
  StringRef Name =
      Dir_ڍير.خالي_جايون() ? رستو : containedPart(Dir_ڍير.پٺ(), رستو);
  Dir_ڍير.ڌڪو_پٺ(رستو);
  بنانشان Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

فراغ JSONWriter::پڇاڙي_Directory() {
  بنانشان Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ڍير.ڪڍو_پٺ();
}

فراغ JSONWriter::write_داخلا(StringRef VPath, StringRef RPath) {
  بنانشان Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

فراغ JSONWriter::اكتب(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<منطقي> Use_External_نالا,
                       std::optional<منطقي> IsCaseSensitive,
                       std::optional<منطقي> IsOverlayRelative,
                       StringRef OverlayDir) {
  استعمال نطاق llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  إذا (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  إذا (Use_External_نالا)
    OS << "  'use-external-names': '" << (*Use_External_نالا ? "true" : "false")
       << "',\n";
  منطقي UseOverlayRelative = خطأ;
  إذا (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  إذا (!Entries.خالي_جايون()) {
    ثابت YAMLVFSEntry &داخلا_3 = Entries.front();

    شروعات_Directory(
      داخلا_3.IsDirectory ? داخلا_3.VPath : رستو_2::والد_رستو(داخلا_3.VPath)
    );

    StringRef RPath = داخلا_3.RPath;
    إذا (UseOverlayRelative) {
      أكد(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.سائيز_2());
    }

    منطقي Is_Current_Dir_خالي_جايون = سچ;
    إذا (!داخلا_3.IsDirectory) {
      write_داخلا(رستو_2::filename(داخلا_3.VPath), RPath);
      Is_Current_Dir_خالي_جايون = خطأ;
    }

    لكل (ثابت خودڪار &داخلا_3 : Entries.ٽڪرو(1)) {
      StringRef Dir =
          داخلا_3.IsDirectory ? داخلا_3.VPath : رستو_2::والد_رستو(داخلا_3.VPath);
      إذا (Dir == Dir_ڍير.پٺ()) {
        إذا (!Is_Current_Dir_خالي_جايون) {
          OS << ",\n";
        }
      } وإلا {
        منطقي Is_Dir_Popped_From_ڍير = خطأ;
        طالما (!Dir_ڍير.خالي_جايون() && !contained_اندريون(Dir_ڍير.پٺ(), Dir)) {
          OS << "\n";
          پڇاڙي_Directory();
          Is_Dir_Popped_From_ڍير = سچ;
        }
        إذا (Is_Dir_Popped_From_ڍير || !Is_Current_Dir_خالي_جايون) {
          OS << ",\n";
        }
        شروعات_Directory(Dir);
        Is_Current_Dir_خالي_جايون = سچ;
      }
      StringRef RPath = داخلا_3.RPath;
      إذا (UseOverlayRelative) {
        أكد(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.سائيز_2());
      }
      إذا (!داخلا_3.IsDirectory) {
        write_داخلا(رستو_2::filename(داخلا_3.VPath), RPath);
        Is_Current_Dir_خالي_جايون = خطأ;
      }
    }

    طالما (!Dir_ڍير.خالي_جايون()) {
      OS << "\n";
      پڇاڙي_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

فراغ YAMLVFSWriter::اكتب(llvm::raw_ostream &OS) {
  llvm::ترتيب_ڏيو(Mappings, [](ثابت YAMLVFSEntry &LHS, ثابت YAMLVFSEntry &RHS) {
    أرجع LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).اكتب(Mappings, Use_External_نالا, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ثابت Twine &رستو, std::error_ڪوڊ &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(رستو, EC);
  إذا (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ڍير.ڌڪو_پٺ(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_ڪوڊ &EC) {
  أكد(FS && State && !State->ڍير.خالي_جايون() && "incrementing past end");
  أكد(!State->ڍير.پٺ()->رستو_2().خالي_جايون() && "non-canonical end iterator");
  vfs::directory_iterator پڇاڙي_2;

  إذا (State->Has_No_ڌڪو_Request)
    State->Has_No_ڌڪو_Request = خطأ;
  وإلا {
    إذا (State->ڍير.پٺ()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ڍير.پٺ()->رستو_2(), EC);
      إذا (I != پڇاڙي_2) {
        State->ڍير.ڌڪو_پٺ(I);
        أرجع *هذا;
      }
    }
  }

  طالما (!State->ڍير.خالي_جايون() && State->ڍير.پٺ().increment(EC) == پڇاڙي_2)
    State->ڍير.ڪڍو_پٺ();

  إذا (State->ڍير.خالي_جايون())
    State.reset(); // end iterator

  أرجع *هذا;
}

ثابت حرف FileSystem::ID = 0;
ثابت حرف OverlayFileSystem::ID = 0;
ثابت حرف ProxyFileSystem::ID = 0;
ثابت حرف اندريون_يادگيري_File_System::ID = 0;
ثابت حرف RedirectingFileSystem::ID = 0;

بنانشان ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_ڳڻپ(
    ثابت FileSystem *FS) {
  أرجع FS->Use_ڳڻپ();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
