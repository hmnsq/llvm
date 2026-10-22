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

#شامل "llvm/Support/VirtualFileSystem.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/DenseMap.h"
#شامل "llvm/ADT/IntrusiveRefCntPtr.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/StringSet.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/ADT/iterator_range.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/Casting.h"
#شامل "llvm/Support/Chrono.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/ErrorOr.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/FileSystem/UniqueID.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/SMLoc.h"
#شامل "llvm/Support/SourceMgr.h"
#شامل "llvm/Support/YAMLParser.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <atomic>
#شامل <cassert>
#شامل <cstdint>
#شامل <iterator>
#شامل <limits>
#شامل <map>
#شامل <memory>
#شامل <optional>
#شامل <string>
#شامل <system_error>
#شامل <utility>
#شامل <vector>

استعمال نامخلا llvm;
استعمال نامخلا llvm::vfs;

استعمال llvm::sys::fs::file_حالت;
استعمال llvm::sys::fs::file_t;
استعمال llvm::sys::fs::file_type;
استعمال llvm::sys::fs::perms;
استعمال llvm::sys::fs::UniqueID;

حالت_2::حالت_2(ثابت file_حالت &حالت_2)
    : UID(حالت_2.getUniqueID()), MTime(حالت_2.get_آخری_Modification_وقت()),
      User(حالت_2.getUser()), گروہ(حالت_2.get_گروہ()), سائز(حالت_2.get_سائز()),
      Type(حالت_2.type()), Perms(حالت_2.permissions()) {}

حالت_2::حالت_2(ثابت Twine &Name, UniqueID UID, sys::وقت_نقطہ<> MTime,
               uint32_t User, uint32_t گروہ, uint64_t سائز, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), گروہ(گروہ),
      سائز(سائز), Type(Type), Perms(Perms) {}

حالت_2 حالت_2::copy_With_New_سائز(ثابت حالت_2 &اندر_2, uint64_t New_سائز) {
  واپس حالت_2(اندر_2.getName(), اندر_2.getUniqueID(), اندر_2.get_آخری_Modification_وقت(),
                اندر_2.getUser(), اندر_2.get_گروہ(), New_سائز, اندر_2.getType(),
                اندر_2.getPermissions());
}

حالت_2 حالت_2::copyWithNewName(ثابت حالت_2 &اندر_2, ثابت Twine &NewName) {
  واپس حالت_2(NewName, اندر_2.getUniqueID(), اندر_2.get_آخری_Modification_وقت(),
                اندر_2.getUser(), اندر_2.get_گروہ(), اندر_2.get_سائز(), اندر_2.getType(),
                اندر_2.getPermissions());
}

حالت_2 حالت_2::copyWithNewName(ثابت file_حالت &اندر_2, ثابت Twine &NewName) {
  واپس حالت_2(NewName, اندر_2.getUniqueID(), اندر_2.get_آخری_Modification_وقت(),
                اندر_2.getUser(), اندر_2.get_گروہ(), اندر_2.get_سائز(), اندر_2.type(),
                اندر_2.permissions());
}

منطقی حالت_2::equivalent(ثابت حالت_2 &Other) ثابت {
  أكد(is_حالت_معلوم() && Other.is_حالت_معلوم());
  واپس getUniqueID() == Other.getUniqueID();
}

منطقی حالت_2::isDirectory() ثابت { واپس Type == file_type::directory_file; }

منطقی حالت_2::isRegularFile() ثابت { واپس Type == file_type::regular_file; }

منطقی حالت_2::isOther() ثابت {
  واپس exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

منطقی حالت_2::isSymlink() ثابت { واپس Type == file_type::symlink_file; }

منطقی حالت_2::is_حالت_معلوم() ثابت { واپس Type != file_type::حالت_error; }

منطقی حالت_2::exists() ثابت {
  واپس is_حالت_معلوم() && Type != file_type::file_not_ملا;
}

File::~File() = طےشدہ;

FileSystem::~FileSystem() = طےشدہ;

ErrorOr<std::unique_ptr<حافظہ_بفر>>
FileSystem::get_بفر_For_File(ثابت llvm::Twine &Name, int64_t File_سائز,
                             منطقی RequiresNullTerminator, منطقی IsVolatile,
                             منطقی Is_متن) {
  خودکار F = Is_متن ? openFileForRead(Name) : open_File_For_Read_ثنائی(Name);
  اگر (!F)
    واپس F.getError();

  واپس (*F)->get_بفر(Name, File_سائز, RequiresNullTerminator, IsVolatile);
}

std::error_رمز FileSystem::makeAbsolute(Small_سمتیہ_Impl<حرف> &راستہ) ثابت {
  اگر (llvm::sys::راستہ_2::is_absolute(راستہ))
    واپس {};

  خودکار WorkingDir = getCurrentWorkingDirectory();
  اگر (!WorkingDir)
    واپس WorkingDir.getError();

  sys::راستہ_2::make_absolute(WorkingDir.get(), راستہ);
  واپس {};
}

std::error_رمز FileSystem::get_Real_راستہ(ثابت Twine &راستہ,
                                        Small_سمتیہ_Impl<حرف> &Output) {
  واپس errc::operation_not_permitted;
}

std::error_رمز FileSystem::isLocal(ثابت Twine &راستہ, منطقی &نتیجہ) {
  واپس errc::operation_not_permitted;
}

منطقی FileSystem::exists(ثابت Twine &راستہ) {
  خودکار حالت_2 = حالت_3(راستہ);
  واپس حالت_2 && حالت_2->exists();
}

llvm::ErrorOr<منطقی> FileSystem::equivalent(ثابت Twine &A, ثابت Twine &B) {
  خودکار حالت_A = حالت_3(A);
  اگر (!حالت_A)
    واپس حالت_A.getError();
  خودکار حالت_B = حالت_3(B);
  اگر (!حالت_B)
    واپس حالت_B.getError();
  واپس حالت_A->equivalent(*حالت_B);
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
خالی FileSystem::dump() ثابت { print(dbgs(), PrintType::RecursiveContents); }
#اگر_ختم

#اگر_غیر_تعریف_شدہ NDEBUG
ساکن منطقی isTraversalComponent(StringRef Component) {
  واپس Component == ".." || Component == ".";
}

ساکن منطقی راستہ_Has_Traversal(StringRef راستہ) {
  استعمال نامخلا llvm::sys;

  برائے (StringRef Comp : llvm::make_range(راستہ_2::begin(راستہ), راستہ_2::انتہا(راستہ)))
    اگر (isTraversalComponent(Comp))
      واپس سچ;
  واپس جھوٹ;
}
#اگر_ختم

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

نامخلا {

/// Wrapper around a raw file descriptor.
جماعت RealFile : عوامی File {
  دوست جماعت RealFileSystem;

  file_t FD;
  حالت_2 S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_راستہ_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::حالت_error, {}),
        RealName(New_Real_راستہ_Name.str()) {
    أكد(FD.isValid() && "Invalid or inactive file descriptor");
  }

عوامی:
  ~RealFile() override;

  ErrorOr<حالت_2> حالت_3() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<حافظہ_بفر>> get_بفر(ثابت Twine &Name,
                                                   int64_t File_سائز,
                                                   منطقی RequiresNullTerminator,
                                                   منطقی IsVolatile) override;
  std::error_رمز close() override;
  خالی سیٹ_راستہ(ثابت Twine &راستہ) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<حالت_2> RealFile::حالت_3() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot stat closed file");
  اگر (!S.is_حالت_معلوم()) {
    file_حالت Real_حالت;
    اگر (std::error_رمز EC = sys::fs::حالت_3(FD, Real_حالت))
      واپس EC;
    S = حالت_2::copyWithNewName(Real_حالت, S.getName());
  }
  واپس S;
}

ErrorOr<std::string> RealFile::getName() {
  واپس RealName.خالی_خانے() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<حافظہ_بفر>>
RealFile::get_بفر(ثابت Twine &Name, int64_t File_سائز,
                    منطقی RequiresNullTerminator, منطقی IsVolatile) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot get buffer for closed file");
  واپس حافظہ_بفر::getOpenFile(FD, Name, File_سائز, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_رمز RealFile::close() {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  std::error_رمز EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  واپس EC;
}

خالی RealFile::سیٹ_راستہ(ثابت Twine &راستہ) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  RealName = راستہ.str();
  اگر (خودکار حالت_2 = حالت_3())
    S = حالت_2.get().copyWithNewName(حالت_2.get(), راستہ);
}

نامخلا {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
جماعت RealFileSystem : عوامی FileSystem {
عوامی:
  صریح RealFileSystem(منطقی LinkCWDToProcess) {
    اگر (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      اگر (std::error_رمز EC = llvm::sys::fs::current_راستہ(PWD))
        WD = std::move(EC);
      ورنہ اگر (llvm::sys::fs::real_راستہ(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      ورنہ
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<حالت_2> حالت_3(ثابت Twine &راستہ) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ثابت Twine &راستہ) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ثنائی(ثابت Twine &راستہ) override;
  directory_iterator dir_begin(ثابت Twine &Dir, std::error_رمز &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override;
  std::error_رمز سیٹ_Current_Working_Directory(ثابت Twine &راستہ) override;
  std::error_رمز isLocal(ثابت Twine &راستہ, منطقی &نتیجہ) override;
  خالی
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 Small_سمتیہ_Impl<std::string> &برآمد) override {
    برآمد.ڈالو_عقب(Dir.str());
  }
  std::error_رمز get_Real_راستہ(ثابت Twine &راستہ,
                              Small_سمتیہ_Impl<حرف> &Output) override;

محفوظ:
  خالی printImpl(raw_ostream &OS, PrintType Type,
                 غیرعلامتی IndentLevel) ثابت override;

نجی:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_راستہ(ثابت Twine &راستہ, Small_سمتیہ_Impl<حرف> &Storage) ثابت {
    اگر (!WD || !*WD)
      واپس راستہ;
    راستہ.مقصد_سمتیہ(Storage);
    sys::راستہ_2::make_absolute(WD->get().Resolved, Storage);
    واپس Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ثابت Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_راستہ(Name, Storage), Flags, &RealName);
    اگر (!FDOrErr)
      واپس error_مقصد_Error_رمز(FDOrErr.takeError());
    واپس std::unique_ptr<File>(
        نیا RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  ساخت WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<حالت_2> RealFileSystem::حالت_3(ثابت Twine &راستہ) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_حالت Real_حالت;
  اگر (std::error_رمز EC =
          sys::fs::حالت_3(adjust_راستہ(راستہ, Storage), Real_حالت))
    واپس EC;
  واپس حالت_2::copyWithNewName(Real_حالت, راستہ);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ثابت Twine &Name) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  واپس openFileForReadWithFlags(Name, sys::fs::کا_متن);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ثنائی(ثابت Twine &Name) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  واپس openFileForReadWithFlags(Name, sys::fs::کا_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ثابت {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  اگر (WD && *WD)
    واپس std::string(WD->get().Specified);
  اگر (WD)
    واپس WD->getError();

  SmallString<128> Dir;
  اگر (std::error_رمز EC = llvm::sys::fs::current_راستہ(Dir))
    واپس EC;
  واپس std::string(Dir);
}

std::error_رمز RealFileSystem::سیٹ_Current_Working_Directory(ثابت Twine &راستہ) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  اگر (!WD)
    واپس llvm::sys::fs::سیٹ_current_راستہ(راستہ);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_راستہ(راستہ, Storage).مقصد_سمتیہ(Absolute);
  منطقی IsDir;
  اگر (خودکار Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    واپس Err;
  اگر (!IsDir)
    واپس std::make_error_رمز(std::errc::not_a_directory);
  اگر (خودکار Err = llvm::sys::fs::real_راستہ(Absolute, Resolved))
    واپس Err;
  WD = WorkingDirectory{Absolute, Resolved};
  واپس std::error_رمز();
}

std::error_رمز RealFileSystem::isLocal(ثابت Twine &راستہ, منطقی &نتیجہ) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  واپس llvm::sys::fs::is_local(adjust_راستہ(راستہ, Storage), نتیجہ);
}

std::error_رمز RealFileSystem::get_Real_راستہ(ثابت Twine &راستہ,
                                            Small_سمتیہ_Impl<حرف> &Output) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  واپس llvm::sys::fs::real_راستہ(adjust_راستہ(راستہ, Storage), Output);
}

خالی RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               غیرعلامتی IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  اگر (WD)
    OS << "own";
  ورنہ
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  ساکن IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(سچ);
  sys::sandbox::violationIfEnabled();

  واپس FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  واپس std::make_unique<RealFileSystem>(جھوٹ);
}

نامخلا {

جماعت RealFSDirIter : عوامی llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

عوامی:
  RealFSDirIter(ثابت Twine &راستہ, std::error_رمز &EC) {
    خودکار BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(راستہ, EC);
    اگر (Iter != sys::fs::directory_iterator())
      Current_اندراج = directory_اندراج(Iter->راستہ_2(), Iter->type());
  }

  std::error_رمز increment() override {
    خودکار BypassSandbox = sys::sandbox::scopedDisable();

    std::error_رمز EC;
    Iter.increment(EC);
    Current_اندراج = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_اندراج()
                       : directory_اندراج(Iter->راستہ_2(), Iter->type());
    واپس EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ثابت Twine &Dir,
                                             std::error_رمز &EC) {
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  واپس directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_راستہ(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> بنیاد_FS) {
  FSList.ڈالو_عقب(std::move(بنیاد_FS));
}

خالی OverlayFileSystem::ڈالو_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ڈالو_عقب(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->سیٹ_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<حالت_2> OverlayFileSystem::حالت_3(ثابت Twine &راستہ) {
  // FIXME: handle symlinks that cross file systems
  برائے (iterator I = overlays_begin(), E = overlays_انتہا(); I != E; ++I) {
    ErrorOr<حالت_2> حالت_2 = (*I)->حالت_3(راستہ);
    اگر (حالت_2 || حالت_2.getError() != llvm::errc::no_such_file_or_directory)
      واپس حالت_2;
  }
  واپس make_error_رمز(llvm::errc::no_such_file_or_directory);
}

منطقی OverlayFileSystem::exists(ثابت Twine &راستہ) {
  // FIXME: handle symlinks that cross file systems
  برائے (iterator I = overlays_begin(), E = overlays_انتہا(); I != E; ++I) {
    اگر ((*I)->exists(راستہ))
      واپس سچ;
  }
  واپس جھوٹ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ثابت llvm::Twine &راستہ) {
  // FIXME: handle symlinks that cross file systems
  برائے (iterator I = overlays_begin(), E = overlays_انتہا(); I != E; ++I) {
    خودکار نتیجہ = (*I)->openFileForRead(راستہ);
    اگر (نتیجہ || نتیجہ.getError() != llvm::errc::no_such_file_or_directory)
      واپس نتیجہ;
  }
  واپس make_error_رمز(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ثابت {
  // All file systems are synchronized, just take the first working directory.
  واپس FSList.front()->getCurrentWorkingDirectory();
}

std::error_رمز
OverlayFileSystem::سیٹ_Current_Working_Directory(ثابت Twine &راستہ) {
  برائے (خودکار &FS : FSList)
    اگر (std::error_رمز EC = FS->سیٹ_Current_Working_Directory(راستہ))
      واپس EC;
  واپس {};
}

std::error_رمز OverlayFileSystem::isLocal(ثابت Twine &راستہ, منطقی &نتیجہ) {
  برائے (خودکار &FS : FSList)
    اگر (FS->exists(راستہ))
      واپس FS->isLocal(راستہ, نتیجہ);
  واپس errc::no_such_file_or_directory;
}

std::error_رمز OverlayFileSystem::get_Real_راستہ(ثابت Twine &راستہ,
                                               Small_سمتیہ_Impl<حرف> &Output) {
  برائے (ثابت خودکار &FS : FSList)
    اگر (FS->exists(راستہ))
      واپس FS->get_Real_راستہ(راستہ, Output);
  واپس errc::no_such_file_or_directory;
}

خالی OverlayFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_سمتیہ_Impl<std::string> &برآمد) {
  // All layers contribute.
  برائے (iterator I = overlays_begin(), E = overlays_انتہا(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, برآمد);
}

خالی OverlayFileSystem::دورہ_بچہ_File_Systems(دورہ_Callback_Ty Callback) {
  برائے (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->دورہ_بچہ_File_Systems(Callback);
  }
}

خالی OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  غیرعلامتی IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  اگر (Type == PrintType::Summary)
    واپس;

  اگر (Type == PrintType::Contents)
    Type = PrintType::Summary;
  برائے (ثابت خودکار &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = طےشدہ;

نامخلا {

/// Combines and deduplicates directory entries across multiple file systems.
جماعت CombiningDirIterImpl : عوامی llvm::vfs::detail::DirIterImpl {
  استعمال FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_سمتیہ<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_سیٹ<> نظر_آیا_نام;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_رمز incrementIter(منطقی Is_پہلا_وقت) {
    جب (!IterList.خالی_خانے()) {
      CurrentDirIter = IterList.عقب();
      IterList.نکالو_عقب();
      اگر (CurrentDirIter != directory_iterator())
        توڑو; //ملا
    }

    اگر (Is_پہلا_وقت && CurrentDirIter == directory_iterator())
      واپس errc::no_such_file_or_directory;
    واپس {};
  }

  std::error_رمز incrementDirIter(منطقی Is_پہلا_وقت) {
    أكد((Is_پہلا_وقت || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_رمز EC;
    اگر (!Is_پہلا_وقت)
      CurrentDirIter.increment(EC);
    اگر (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(Is_پہلا_وقت);
    واپس EC;
  }

  std::error_رمز incrementImpl(منطقی Is_پہلا_وقت) {
    جب (سچ) {
      std::error_رمز EC = incrementDirIter(Is_پہلا_وقت);
      اگر (EC || CurrentDirIter == directory_iterator()) {
        Current_اندراج = directory_اندراج();
        واپس EC;
      }
      Current_اندراج = *CurrentDirIter;
      StringRef Name = llvm::sys::راستہ_2::filename(Current_اندراج.راستہ_2());
      اگر (نظر_آیا_نام.داخل_کرو(Name).دوسرا_2)
        واپس EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

عوامی:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_رمز &EC) {
    برائے (ثابت خودکار &FS : FileSystems) {
      std::error_رمز FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      اگر (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        واپس;
      }
      اگر (!FEC)
        IterList.ڈالو_عقب(Iter);
    }
    EC = incrementImpl(سچ);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_رمز &EC)
      : IterList(DirIters) {
    EC = incrementImpl(سچ);
  }

  std::error_رمز increment() override { واپس incrementImpl(جھوٹ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ثابت Twine &Dir,
                                                std::error_رمز &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  اگر (EC)
    واپس {};
  واپس Combined;
}

خالی ProxyFileSystem::anchor() {}

نامخلا llvm {
نامخلا vfs {

نامخلا detail {

شمار اندر_حافظہ_گرہ_قسم {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
جماعت اندر_حافظہ_گرہ {
  اندر_حافظہ_گرہ_قسم قسم_3;
  std::string FileName;

عوامی:
  اندر_حافظہ_گرہ(llvm::StringRef FileName, اندر_حافظہ_گرہ_قسم قسم_3)
      : قسم_3(قسم_3), FileName(std::string(llvm::sys::راستہ_2::filename(FileName))) {
  }
  مجازی ~اندر_حافظہ_گرہ() = طےشدہ;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  مجازی حالت_2 get_حالت(ثابت Twine &RequestedName) ثابت = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ثابت { واپس FileName; }
  اندر_حافظہ_گرہ_قسم get_قسم() ثابت { واپس قسم_3; }
  مجازی std::string مقصد_String(غیرعلامتی Indent) ثابت = 0;
};

جماعت اندر_حافظہ_File : عوامی اندر_حافظہ_گرہ {
  حالت_2 Stat;
  std::unique_ptr<llvm::حافظہ_بفر> بفر;

عوامی:
  اندر_حافظہ_File(حالت_2 Stat, std::unique_ptr<llvm::حافظہ_بفر> بفر)
      : اندر_حافظہ_گرہ(Stat.getName(), IME_File), Stat(std::move(Stat)),
        بفر(std::move(بفر)) {}

  حالت_2 get_حالت(ثابت Twine &RequestedName) ثابت override {
    واپس حالت_2::copyWithNewName(Stat, RequestedName);
  }
  llvm::حافظہ_بفر *get_بفر() ثابت { واپس بفر.get(); }

  std::string مقصد_String(غیرعلامتی Indent) ثابت override {
    واپس (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ساکن منطقی classof(ثابت اندر_حافظہ_گرہ *N) {
    واپس N->get_قسم() == IME_File;
  }
};

نامخلا {

جماعت اندر_حافظہ_Hard_Link : عوامی اندر_حافظہ_گرہ {
  ثابت اندر_حافظہ_File &ResolvedFile;

عوامی:
  اندر_حافظہ_Hard_Link(StringRef راستہ, ثابت اندر_حافظہ_File &ResolvedFile)
      : اندر_حافظہ_گرہ(راستہ, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ثابت اندر_حافظہ_File &getResolvedFile() ثابت { واپس ResolvedFile; }

  حالت_2 get_حالت(ثابت Twine &RequestedName) ثابت override {
    واپس ResolvedFile.get_حالت(RequestedName);
  }

  std::string مقصد_String(غیرعلامتی Indent) ثابت override {
    واپس std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.مقصد_String(0);
  }

  ساکن منطقی classof(ثابت اندر_حافظہ_گرہ *N) {
    واپس N->get_قسم() == IME_HardLink;
  }
};

جماعت اندر_حافظہ_Symbolic_Link : عوامی اندر_حافظہ_گرہ {
  std::string ہدف_راستہ;
  حالت_2 Stat;

عوامی:
  اندر_حافظہ_Symbolic_Link(StringRef راستہ, StringRef ہدف_راستہ, حالت_2 Stat)
      : اندر_حافظہ_گرہ(راستہ, IME_SymbolicLink), ہدف_راستہ(std::move(ہدف_راستہ)),
        Stat(Stat) {}

  std::string مقصد_String(غیرعلامتی Indent) ثابت override {
    واپس std::string(Indent, ' ') + "SymbolicLink to -> " + ہدف_راستہ;
  }

  حالت_2 get_حالت(ثابت Twine &RequestedName) ثابت override {
    واپس حالت_2::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_ہدف_راستہ() ثابت { واپس ہدف_راستہ; }

  ساکن منطقی classof(ثابت اندر_حافظہ_گرہ *N) {
    واپس N->get_قسم() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
جماعت اندر_حافظہ_File_Adaptor : عوامی File {
  ثابت اندر_حافظہ_File &گرہ;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

عوامی:
  صریح اندر_حافظہ_File_Adaptor(ثابت اندر_حافظہ_File &گرہ,
                               std::string RequestedName)
      : گرہ(گرہ), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<حالت_2> حالت_3() override {
    واپس گرہ.get_حالت(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::حافظہ_بفر>>
  get_بفر(ثابت Twine &Name, int64_t File_سائز, منطقی RequiresNullTerminator,
            منطقی IsVolatile) override {
    llvm::حافظہ_بفر *Buf = گرہ.get_بفر();
    واپس llvm::حافظہ_بفر::get_Mem_بفر(
        Buf->get_بفر(), Buf->get_بفر_Identifier(), RequiresNullTerminator);
  }

  std::error_رمز close() override { واپس {}; }

  خالی سیٹ_راستہ(ثابت Twine &راستہ) override { RequestedName = راستہ.str(); }
};
} // namespace

جماعت اندر_حافظہ_Directory : عوامی اندر_حافظہ_گرہ {
  حالت_2 Stat;
  std::map<std::string, std::unique_ptr<اندر_حافظہ_گرہ>, std::less<>> Entries;

عوامی:
  اندر_حافظہ_Directory(حالت_2 Stat)
      : اندر_حافظہ_گرہ(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  حالت_2 get_حالت(ثابت Twine &RequestedName) ثابت override {
    واپس حالت_2::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ثابت { واپس Stat.getUniqueID(); }

  اندر_حافظہ_گرہ *get_بچہ(StringRef Name) ثابت {
    خودکار I = Entries.ڈھونڈو(Name);
    اگر (I != Entries.انتہا())
      واپس I->دوسرا_2.get();
    واپس خالی_اشاریہ;
  }

  اندر_حافظہ_گرہ *جوڑو_بچہ(StringRef Name, std::unique_ptr<اندر_حافظہ_گرہ> بچہ) {
    واپس Entries.emplace(Name, std::move(بچہ)).پہلا_2->دوسرا_2.get();
  }

  استعمال const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ثابت { واپس Entries.begin(); }
  const_iterator انتہا() ثابت { واپس Entries.انتہا(); }

  std::string مقصد_String(غیرعلامتی Indent) ثابت override {
    std::string نتیجہ =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    برائے (ثابت خودکار &اندراج_2 : Entries)
      نتیجہ += اندراج_2.دوسرا_2->مقصد_String(Indent + 2);
    واپس نتیجہ;
  }

  ساکن منطقی classof(ثابت اندر_حافظہ_گرہ *N) {
    واپس N->get_قسم() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ساکن sys::fs::UniqueID getUniqueID(hash_رمز Hash) {
  واپس sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ساکن sys::fs::UniqueID getFileID(sys::fs::UniqueID والد_2,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  واپس getUniqueID(llvm::hash_combine(والد_2.getFile(), Name, Contents));
}
ساکن sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID والد_2,
                                        llvm::StringRef Name) {
  واپس getUniqueID(llvm::hash_combine(والد_2.getFile(), Name));
}

حالت_2 detail::New_اندر_حافظہ_گرہ_Info::make_حالت() ثابت {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, بفر ? بفر->get_بفر() : "");

  واپس حالت_2(راستہ, UID, llvm::sys::مقصد_وقت_نقطہ(Modification_وقت), User,
                گروہ, بفر ? بفر->get_بفر_سائز() : 0, Type, Perms);
}

اندر_حافظہ_File_System::اندر_حافظہ_File_System(منطقی UseNormalizedPaths)
    : جڑ_2(نیا detail::اندر_حافظہ_Directory(
          حالت_2("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::وقت_نقطہ<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

اندر_حافظہ_File_System::~اندر_حافظہ_File_System() = طےشدہ;

std::string اندر_حافظہ_File_System::مقصد_String() ثابت {
  واپس جڑ_2->مقصد_String(/*Indent=*/0);
}

منطقی اندر_حافظہ_File_System::جوڑو_File(ثابت Twine &P, time_t Modification_وقت,
                                 std::unique_ptr<llvm::حافظہ_بفر> بفر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> گروہ,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_گرہ_Fn Make_گرہ) {
  SmallString<128> راستہ;
  P.مقصد_سمتیہ(راستہ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_رمز EC = makeAbsolute(راستہ);
  أكد(!EC);
  (خالی)EC;

  اگر (useNormalizedPaths())
    llvm::sys::راستہ_2::remove_dots(راستہ, /*remove_dot_dot=*/سچ);

  اگر (راستہ.خالی_خانے())
    واپس جھوٹ;

  detail::اندر_حافظہ_Directory *Dir = جڑ_2.get();
  خودکار I = llvm::sys::راستہ_2::begin(راستہ), E = sys::راستہ_2::انتہا(راستہ);
  ثابت خودکار ResolvedUser = User.قدر_or(0);
  ثابت خودکار Resolved_گروہ = گروہ.قدر_or(0);
  ثابت خودکار ResolvedType = Type.قدر_or(sys::fs::file_type::regular_file);
  ثابت خودکار ResolvedPerms = Perms.قدر_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ثابت خودکار NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  جب (سچ) {
    Name = *I;
    ++I;
    اگر (I == E)
      توڑو;
    detail::اندر_حافظہ_گرہ *گرہ = Dir->get_بچہ(Name);
    اگر (!گرہ) {
      // This isn't the last element, so we create a new directory.
      حالت_2 Stat(
          StringRef(راستہ.str().begin(), Name.انتہا() - راستہ.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::مقصد_وقت_نقطہ(Modification_وقت), ResolvedUser, Resolved_گروہ,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::اندر_حافظہ_Directory>(Dir->جوڑو_بچہ(
          Name, std::make_unique<detail::اندر_حافظہ_Directory>(std::move(Stat))));
      جاری;
    }
    // Creating file under another file.
    اگر (!isa<detail::اندر_حافظہ_Directory>(گرہ))
      واپس جھوٹ;
    Dir = cast<detail::اندر_حافظہ_Directory>(گرہ);
  }
  detail::اندر_حافظہ_گرہ *گرہ = Dir->get_بچہ(Name);
  اگر (!گرہ) {
    Dir->جوڑو_بچہ(Name,
                  Make_گرہ({Dir->getUniqueID(), راستہ, Name, Modification_وقت,
                            std::move(بفر), ResolvedUser, Resolved_گروہ,
                            ResolvedType, ResolvedPerms}));
    واپس سچ;
  }
  اگر (isa<detail::اندر_حافظہ_Directory>(گرہ))
    واپس ResolvedType == sys::fs::file_type::directory_file;

  أكد((isa<detail::اندر_حافظہ_File>(گرہ) ||
          isa<detail::اندر_حافظہ_Hard_Link>(گرہ)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  اگر (خودکار *Link = dyn_cast<detail::اندر_حافظہ_Hard_Link>(گرہ)) {
    واپس Link->getResolvedFile().get_بفر()->get_بفر() ==
           بفر->get_بفر();
  }
  واپس cast<detail::اندر_حافظہ_File>(گرہ)->get_بفر()->get_بفر() ==
         بفر->get_بفر();
}

منطقی اندر_حافظہ_File_System::جوڑو_File(ثابت Twine &P, time_t Modification_وقت,
                                 std::unique_ptr<llvm::حافظہ_بفر> بفر,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> گروہ,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  واپس جوڑو_File(P, Modification_وقت, std::move(بفر), User, گروہ, Type,
                 Perms,
                 [](detail::New_اندر_حافظہ_گرہ_Info NNI)
                     -> std::unique_ptr<detail::اندر_حافظہ_گرہ> {
                   حالت_2 Stat = NNI.make_حالت();
                   اگر (Stat.getType() == sys::fs::file_type::directory_file)
                     واپس std::make_unique<detail::اندر_حافظہ_Directory>(Stat);
                   واپس std::make_unique<detail::اندر_حافظہ_File>(
                       Stat, std::move(NNI.بفر));
                 });
}

منطقی اندر_حافظہ_File_System::جوڑو_File_No_Own(
    ثابت Twine &P, time_t Modification_وقت,
    ثابت llvm::حافظہ_بفر_Ref &بفر, std::optional<uint32_t> User,
    std::optional<uint32_t> گروہ, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  واپس جوڑو_File(P, Modification_وقت, llvm::حافظہ_بفر::get_Mem_بفر(بفر),
                 std::move(User), std::move(گروہ), std::move(Type),
                 std::move(Perms),
                 [](detail::New_اندر_حافظہ_گرہ_Info NNI)
                     -> std::unique_ptr<detail::اندر_حافظہ_گرہ> {
                   حالت_2 Stat = NNI.make_حالت();
                   اگر (Stat.getType() == sys::fs::file_type::directory_file)
                     واپس std::make_unique<detail::اندر_حافظہ_Directory>(Stat);
                   واپس std::make_unique<detail::اندر_حافظہ_File>(
                       Stat, std::move(NNI.بفر));
                 });
}

detail::Named_گرہ_Or_Error
اندر_حافظہ_File_System::lookup_گرہ(ثابت Twine &P, منطقی FollowFinalSymlink,
                               size_t Symlink_گہرائی) ثابت {
  SmallString<128> راستہ;
  P.مقصد_سمتیہ(راستہ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_رمز EC = makeAbsolute(راستہ);
  أكد(!EC);
  (خالی)EC;

  اگر (useNormalizedPaths())
    llvm::sys::راستہ_2::remove_dots(راستہ, /*remove_dot_dot=*/سچ);

  ثابت detail::اندر_حافظہ_Directory *Dir = جڑ_2.get();
  اگر (راستہ.خالی_خانے())
    واپس detail::Named_گرہ_Or_Error(راستہ, Dir);

  خودکار I = llvm::sys::راستہ_2::begin(راستہ), E = llvm::sys::راستہ_2::انتہا(راستہ);
  جب (سچ) {
    detail::اندر_حافظہ_گرہ *گرہ = Dir->get_بچہ(*I);
    ++I;
    اگر (!گرہ)
      واپس errc::no_such_file_or_directory;

    اگر (خودکار Symlink = dyn_cast<detail::اندر_حافظہ_Symbolic_Link>(گرہ)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      اگر (I == E && !FollowFinalSymlink)
        واپس detail::Named_گرہ_Or_Error(راستہ, Symlink);

      اگر (Symlink_گہرائی > اندر_حافظہ_File_System::Max_Symlink_گہرائی)
        واپس errc::no_such_file_or_directory;

      SmallString<128> ہدف_راستہ = Symlink->get_ہدف_راستہ();
      اگر (std::error_رمز EC = makeAbsolute(ہدف_راستہ))
        واپس EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      خودکار ہدف_2 =
          lookup_گرہ(ہدف_راستہ, /*FollowFinalSymlink=*/سچ, Symlink_گہرائی + 1);
      اگر (!ہدف_2 || I == E)
        واپس ہدف_2;

      اگر (!isa<detail::اندر_حافظہ_Directory>(*ہدف_2))
        واپس errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::اندر_حافظہ_Directory>(*ہدف_2);
      جاری;
    }

    // Return the file if it's at the end of the path.
    اگر (خودکار File = dyn_cast<detail::اندر_حافظہ_File>(گرہ)) {
      اگر (I == E)
        واپس detail::Named_گرہ_Or_Error(راستہ, File);
      واپس errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    اگر (خودکار File = dyn_cast<detail::اندر_حافظہ_Hard_Link>(گرہ)) {
      اگر (I == E)
        واپس detail::Named_گرہ_Or_Error(راستہ, &File->getResolvedFile());
      واپس errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::اندر_حافظہ_Directory>(گرہ);
    اگر (I == E)
      واپس detail::Named_گرہ_Or_Error(راستہ, Dir);
  }
}

منطقی اندر_حافظہ_File_System::جوڑو_Hard_Link(ثابت Twine &NewLink,
                                     ثابت Twine &ہدف_2) {
  خودکار New_Link_گرہ = lookup_گرہ(NewLink, /*FollowFinalSymlink=*/جھوٹ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  خودکار ہدف_گرہ = lookup_گرہ(ہدف_2, /*FollowFinalSymlink=*/سچ);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  اگر (!ہدف_گرہ || New_Link_گرہ || !isa<detail::اندر_حافظہ_File>(*ہدف_گرہ))
    واپس جھوٹ;
  واپس جوڑو_File(NewLink, 0, خالی_اشاریہ, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_اندر_حافظہ_گرہ_Info NNI) {
                   واپس std::make_unique<detail::اندر_حافظہ_Hard_Link>(
                       NNI.راستہ.str(),
                       *cast<detail::اندر_حافظہ_File>(*ہدف_گرہ));
                 });
}

منطقی اندر_حافظہ_File_System::جوڑو_Symbolic_Link(
    ثابت Twine &NewLink, ثابت Twine &ہدف_2, time_t Modification_وقت,
    std::optional<uint32_t> User, std::optional<uint32_t> گروہ,
    std::optional<llvm::sys::fs::perms> Perms) {
  خودکار New_Link_گرہ = lookup_گرہ(NewLink, /*FollowFinalSymlink=*/جھوٹ);
  اگر (New_Link_گرہ)
    واپس جھوٹ;

  SmallString<128> NewLinkStr, ہدف_Str;
  NewLink.مقصد_سمتیہ(NewLinkStr);
  ہدف_2.مقصد_سمتیہ(ہدف_Str);

  واپس جوڑو_File(NewLinkStr, Modification_وقت, خالی_اشاریہ, User, گروہ,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_اندر_حافظہ_گرہ_Info NNI) {
                   واپس std::make_unique<detail::اندر_حافظہ_Symbolic_Link>(
                       NewLinkStr, ہدف_Str, NNI.make_حالت());
                 });
}

llvm::ErrorOr<حالت_2> اندر_حافظہ_File_System::حالت_3(ثابت Twine &راستہ) {
  خودکار گرہ = lookup_گرہ(راستہ, /*FollowFinalSymlink=*/سچ);
  اگر (گرہ)
    واپس (*گرہ)->get_حالت(راستہ);
  واپس گرہ.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
اندر_حافظہ_File_System::openFileForRead(ثابت Twine &راستہ) {
  خودکار گرہ = lookup_گرہ(راستہ,/*FollowFinalSymlink=*/سچ);
  اگر (!گرہ)
    واپس گرہ.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  اگر (خودکار *F = dyn_cast<detail::اندر_حافظہ_File>(*گرہ))
    واپس std::unique_ptr<File>(
        نیا detail::اندر_حافظہ_File_Adaptor(*F, راستہ.str()));

  // FIXME: errc::not_a_file?
  واپس make_error_رمز(llvm::errc::invalid_دلیل);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
جماعت اندر_حافظہ_File_System::DirIterator : عوامی llvm::vfs::detail::DirIterImpl {
  ثابت اندر_حافظہ_File_System *FS;
  detail::اندر_حافظہ_Directory::const_iterator I;
  detail::اندر_حافظہ_Directory::const_iterator E;
  std::string RequestedDirName;

  خالی سیٹ_Current_اندراج() {
    اگر (I != E) {
      SmallString<256> راستہ(RequestedDirName);
      llvm::sys::راستہ_2::append(راستہ, I->دوسرا_2->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      انتخاب (I->دوسرا_2->get_قسم()) {
      حالت detail::IME_File:
      حالت detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        توڑو;
      حالت detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        توڑو;
      حالت detail::IME_SymbolicLink:
        اگر (خودکار Symlink_ہدف =
                FS->lookup_گرہ(راستہ, /*FollowFinalSymlink=*/سچ)) {
          راستہ = Symlink_ہدف.getName();
          Type = (*Symlink_ہدف)->get_حالت(راستہ).getType();
        }
        توڑو;
      }
      Current_اندراج = directory_اندراج(std::string(راستہ), Type);
    } ورنہ {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_اندراج = directory_اندراج();
    }
  }

عوامی:
  DirIterator() = طےشدہ;

  DirIterator(ثابت اندر_حافظہ_File_System *FS,
              ثابت detail::اندر_حافظہ_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.انتہا()),
        RequestedDirName(std::move(RequestedDirName)) {
    سیٹ_Current_اندراج();
  }

  std::error_رمز increment() override {
    ++I;
    سیٹ_Current_اندراج();
    واپس {};
  }
};

directory_iterator اندر_حافظہ_File_System::dir_begin(ثابت Twine &Dir,
                                                 std::error_رمز &EC) {
  خودکار گرہ = lookup_گرہ(Dir, /*FollowFinalSymlink=*/سچ);
  اگر (!گرہ) {
    EC = گرہ.getError();
    واپس directory_iterator(std::make_shared<DirIterator>());
  }

  اگر (خودکار *Dir_گرہ = dyn_cast<detail::اندر_حافظہ_Directory>(*گرہ))
    واپس directory_iterator(
        std::make_shared<DirIterator>(یہ, *Dir_گرہ, Dir.str()));

  EC = make_error_رمز(llvm::errc::not_a_directory);
  واپس directory_iterator(std::make_shared<DirIterator>());
}

std::error_رمز اندر_حافظہ_File_System::سیٹ_Current_Working_Directory(ثابت Twine &P) {
  SmallString<128> راستہ;
  P.مقصد_سمتیہ(راستہ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_رمز EC = makeAbsolute(راستہ);
  أكد(!EC);
  (خالی)EC;

  اگر (useNormalizedPaths())
    llvm::sys::راستہ_2::remove_dots(راستہ, /*remove_dot_dot=*/سچ);

  اگر (!راستہ.خالی_خانے())
    WorkingDirectory = std::string(راستہ);
  واپس {};
}

std::error_رمز اندر_حافظہ_File_System::get_Real_راستہ(ثابت Twine &راستہ,
                                                Small_سمتیہ_Impl<حرف> &Output) {
  خودکار CWD = getCurrentWorkingDirectory();
  اگر (!CWD || CWD->خالی_خانے())
    واپس errc::operation_not_permitted;
  راستہ.مقصد_سمتیہ(Output);
  اگر (خودکار EC = makeAbsolute(Output))
    واپس EC;
  llvm::sys::راستہ_2::remove_dots(Output, /*remove_dot_dot=*/سچ);
  واپس {};
}

std::error_رمز اندر_حافظہ_File_System::isLocal(ثابت Twine &راستہ, منطقی &نتیجہ) {
  نتیجہ = جھوٹ;
  واپس {};
}

خالی اندر_حافظہ_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   غیرعلامتی IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

نامخلا {

ساکن llvm::sys::راستہ_2::Style getExistingStyle(llvm::StringRef راستہ) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::راستہ_2::Style style = llvm::sys::راستہ_2::Style::native;
  ثابت size_t n = راستہ.ڈھونڈو_پہلا_کا("/\\");
  // Can't distinguish between posix and windows_slash here.
  اگر (n != static_cast<size_t>(-1))
    style = (راستہ[n] == '/') ? llvm::sys::راستہ_2::Style::posix
                             : llvm::sys::راستہ_2::Style::windows_backslash;
  واپس style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ساکن llvm::SmallString<256> canonicalize(llvm::StringRef راستہ) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::راستہ_2::Style style = getExistingStyle(راستہ);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> نتیجہ_2 =
      llvm::sys::راستہ_2::remove_leading_dotslash(راستہ, style);
  llvm::sys::راستہ_2::remove_dots(نتیجہ_2, /*remove_dot_dot=*/سچ, style);
  واپس نتیجہ_2;
}

/// Whether the error and entry specify a file/directory that was not found.
ساکن منطقی is_File_Not_ملا(std::error_رمز EC,
                           RedirectingFileSystem::اندراج_2 *E = خالی_اشاریہ) {
  اگر (E && !isa<RedirectingFileSystem::Directory_Remap_اندراج>(E))
    واپس جھوٹ;
  واپس EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  أكد(ExternalFS && "RedirectingFileSystem requires an external FS");
  اگر (خودکار ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
جماعت llvm::vfs::RedirectingFSDirIterImpl
    : عوامی llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::Directory_اندراج::iterator Current, انتہا_2;

  std::error_رمز incrementImpl(منطقی Is_پہلا_وقت) {
    أكد((Is_پہلا_وقت || Current != انتہا_2) && "cannot iterate past end");
    اگر (!Is_پہلا_وقت)
      ++Current;
    اگر (Current != انتہا_2) {
      SmallString<128> راستہ_Str(Dir);
      llvm::sys::راستہ_2::append(راستہ_Str, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      انتخاب ((*Current)->get_قسم()) {
      حالت RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      حالت RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        توڑو;
      حالت RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        توڑو;
      }
      Current_اندراج = directory_اندراج(std::string(راستہ_Str), Type);
    } ورنہ {
      Current_اندراج = directory_اندراج();
    }
    واپس {};
  };

عوامی:
  RedirectingFSDirIterImpl(
      ثابت Twine &راستہ, RedirectingFileSystem::Directory_اندراج::iterator Begin,
      RedirectingFileSystem::Directory_اندراج::iterator انتہا_2, std::error_رمز &EC)
      : Dir(راستہ.str()), Current(Begin), انتہا_2(انتہا_2) {
    EC = incrementImpl(/*IsFirstTime=*/سچ);
  }

  std::error_رمز increment() override {
    واپس incrementImpl(/*IsFirstTime=*/جھوٹ);
  }
};

نامخلا {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
جماعت RedirectingFSDirRemapIterImpl : عوامی llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::راستہ_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

عوامی:
  RedirectingFSDirRemapIterImpl(std::string Dir_راستہ,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_راستہ)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    اگر (ExternalIter != llvm::vfs::directory_iterator())
      سیٹ_Current_اندراج();
  }

  خالی سیٹ_Current_اندراج() {
    StringRef External_راستہ = ExternalIter->راستہ_2();
    llvm::sys::راستہ_2::Style ExternalStyle = getExistingStyle(External_راستہ);
    StringRef File = llvm::sys::راستہ_2::filename(External_راستہ, ExternalStyle);

    SmallString<128> New_راستہ(Dir);
    llvm::sys::راستہ_2::append(New_راستہ, DirStyle, File);

    Current_اندراج = directory_اندراج(std::string(New_راستہ), ExternalIter->type());
  }

  std::error_رمز increment() override {
    std::error_رمز EC;
    ExternalIter.increment(EC);
    اگر (!EC && ExternalIter != llvm::vfs::directory_iterator())
      سیٹ_Current_اندراج();
    ورنہ
      Current_اندراج = directory_اندراج();
    واپس EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ثابت {
  واپس WorkingDirectory;
}

std::error_رمز
RedirectingFileSystem::سیٹ_Current_Working_Directory(ثابت Twine &راستہ) {
  // Don't change the working directory if the path doesn't exist.
  اگر (!exists(راستہ))
    واپس errc::no_such_file_or_directory;

  SmallString<128> Absolute_راستہ;
  راستہ.مقصد_سمتیہ(Absolute_راستہ);
  اگر (std::error_رمز EC = makeAbsolute(Absolute_راستہ))
    واپس EC;
  WorkingDirectory = std::string(Absolute_راستہ);
  واپس {};
}

std::error_رمز RedirectingFileSystem::isLocal(ثابت Twine &راستہ_3,
                                               منطقی &نتیجہ) {
  SmallString<256> راستہ;
  راستہ_3.مقصد_سمتیہ(راستہ);

  اگر (makeAbsolute(راستہ))
    واپس {};

  واپس ExternalFS->isLocal(راستہ, نتیجہ);
}

std::error_رمز RedirectingFileSystem::makeAbsolute(Small_سمتیہ_Impl<حرف> &راستہ) ثابت {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  اگر (llvm::sys::راستہ_2::is_absolute(راستہ, llvm::sys::راستہ_2::Style::posix) ||
      llvm::sys::راستہ_2::is_absolute(راستہ,
                                   llvm::sys::راستہ_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    واپس {};

  خودکار WorkingDir = getCurrentWorkingDirectory();
  اگر (!WorkingDir)
    واپس WorkingDir.getError();

  واپس makeAbsolute(WorkingDir.get(), راستہ);
}

std::error_رمز
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_سمتیہ_Impl<حرف> &راستہ) ثابت {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  اگر (!WorkingDir.خالی_خانے() &&
      !sys::راستہ_2::is_absolute(WorkingDir, sys::راستہ_2::Style::posix) &&
      !sys::راستہ_2::is_absolute(WorkingDir,
                              sys::راستہ_2::Style::windows_backslash)) {
    واپس std::error_رمز();
  }
  sys::راستہ_2::Style style = sys::راستہ_2::Style::windows_backslash;
  اگر (sys::راستہ_2::is_absolute(WorkingDir, sys::راستہ_2::Style::posix)) {
    style = sys::راستہ_2::Style::posix;
  } ورنہ {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    اگر (getExistingStyle(WorkingDir) != sys::راستہ_2::Style::windows_backslash)
      style = sys::راستہ_2::Style::windows_slash;
  }

  std::string نتیجہ = std::string(WorkingDir);
  StringRef Dir(نتیجہ);
  اگر (!Dir.سرے_with(sys::راستہ_2::get_separator(style))) {
    نتیجہ += sys::راستہ_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  نتیجہ.append(راستہ.ڈیٹا(), راستہ.سائز_2());
  راستہ.تفویض(نتیجہ.begin(), نتیجہ.انتہا());

  واپس {};
}

directory_iterator RedirectingFileSystem::dir_begin(ثابت Twine &Dir,
                                                    std::error_رمز &EC) {
  SmallString<256> راستہ;
  Dir.مقصد_سمتیہ(راستہ);

  EC = makeAbsolute(راستہ);
  اگر (EC)
    واپس {};

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_راستہ(راستہ);
  اگر (!نتیجہ) {
    اگر (Redirection != Redirect_قسم::RedirectOnly &&
        is_File_Not_ملا(نتیجہ.getError()))
      واپس ExternalFS->dir_begin(راستہ, EC);

    EC = نتیجہ.getError();
    واپس {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<حالت_2> S = حالت_3(راستہ, Dir, *نتیجہ);
  اگر (!S) {
    اگر (Redirection != Redirect_قسم::RedirectOnly &&
        is_File_Not_ملا(S.getError(), نتیجہ->E))
      واپس ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    واپس {};
  }

  اگر (!S->isDirectory()) {
    EC = errc::not_a_directory;
    واپس {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_رمز RedirectEC;
  اگر (خودکار ExtRedirect = نتیجہ->getExternalRedirect()) {
    خودکار RE = cast<RedirectingFileSystem::Remap_اندراج>(نتیجہ->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    اگر (!RE->useExternalName(Use_External_نام)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(راستہ), RedirectIter));
    }
  } ورنہ {
    خودکار DE = cast<Directory_اندراج>(نتیجہ->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            راستہ, DE->contents_begin(), DE->contents_انتہا(), RedirectEC));
  }

  اگر (RedirectEC) {
    اگر (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      واپس {};
    }
    RedirectIter = {};
  }

  اگر (Redirection == Redirect_قسم::RedirectOnly) {
    EC = RedirectEC;
    واپس RedirectIter;
  }

  std::error_رمز ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(راستہ, ExternalEC);
  اگر (ExternalEC) {
    اگر (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      واپس {};
    }
    ExternalIter = {};
  }

  Small_سمتیہ<directory_iterator, 2> Iters;
  انتخاب (Redirection) {
  حالت Redirect_قسم::Fallthrough:
    Iters.ڈالو_عقب(ExternalIter);
    Iters.ڈالو_عقب(RedirectIter);
    توڑو;
  حالت Redirect_قسم::Fallback:
    Iters.ڈالو_عقب(RedirectIter);
    Iters.ڈالو_عقب(ExternalIter);
    توڑو;
  طےشدہ:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  اگر (EC)
    واپس {};
  واپس Combined;
}

خالی RedirectingFileSystem::سیٹ_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ثابت {
  واپس OverlayFileDir;
}

خالی RedirectingFileSystem::سیٹ_Fallthrough(منطقی Fallthrough) {
  اگر (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_قسم::Fallthrough;
  } ورنہ {
    Redirection = RedirectingFileSystem::Redirect_قسم::RedirectOnly;
  }
}

خالی RedirectingFileSystem::سیٹ_Redirection(
    RedirectingFileSystem::Redirect_قسم قسم_3) {
  Redirection = قسم_3;
}

std::سمتیہ<StringRef> RedirectingFileSystem::getRoots() ثابت {
  std::سمتیہ<StringRef> R;
  R.reserve(Roots.سائز_2());
  برائے (ثابت خودکار &جڑ_2 : Roots)
    R.ڈالو_عقب(جڑ_2->getName());
  واپس R;
}

خالی RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      غیرعلامتی IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_نام ? "true" : "false") << ")\n";
  اگر (Type == PrintType::Summary)
    واپس;

  برائے (ثابت خودکار &جڑ_2 : Roots)
    print_اندراج(OS, جڑ_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

خالی RedirectingFileSystem::print_اندراج(raw_ostream &OS,
                                       RedirectingFileSystem::اندراج_2 *E,
                                       غیرعلامتی IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  انتخاب (E->get_قسم()) {
  حالت EK_Directory: {
    خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(E);

    OS << "\n";
    برائے (std::unique_ptr<اندراج_2> &Sub_اندراج :
         llvm::make_range(DE->contents_begin(), DE->contents_انتہا()))
      print_اندراج(OS, Sub_اندراج.get(), IndentLevel + 1);
    توڑو;
  }
  حالت EK_DirectoryRemap:
  حالت EK_File: {
    خودکار *RE = cast<RedirectingFileSystem::Remap_اندراج>(E);
    OS << " -> '" << RE->get_External_Contents_راستہ() << "'";
    انتخاب (RE->getUseName()) {
    حالت NK_Not_سیٹ:
      توڑو;
    حالت NK_External:
      OS << " (UseExternalName: true)";
      توڑو;
    حالت NK_Virtual:
      OS << " (UseExternalName: false)";
      توڑو;
    }
    OS << "\n";
    توڑو;
  }
  }
}

خالی RedirectingFileSystem::دورہ_بچہ_File_Systems(دورہ_Callback_Ty Callback) {
  اگر (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->دورہ_بچہ_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
جماعت llvm::vfs::RedirectingFileSystemParser {
  yaml::دھارا &دھارا;

  خالی error(yaml::گرہ *N, ثابت Twine &پیغام_4) { دھارا.printError(N, پیغام_4); }

  // false on error
  منطقی parseScalarString(yaml::گرہ *N, StringRef &نتیجہ,
                         Small_سمتیہ_Impl<حرف> &Storage) {
    ثابت خودکار *S = dyn_cast_if_present<yaml::Scalar_گرہ>(N);

    اگر (!S) {
      error(N, "expected string");
      واپس جھوٹ;
    }
    نتیجہ = S->get_قدر(Storage);
    واپس سچ;
  }

  // false on error
  منطقی parseScalarBool(yaml::گرہ *N, منطقی &نتیجہ) {
    SmallString<5> Storage;
    StringRef قدر;
    اگر (!parseScalarString(N, قدر, Storage))
      واپس جھوٹ;

    اگر (قدر.equals_insensitive("true") || قدر.equals_insensitive("on") ||
        قدر.equals_insensitive("yes") || قدر == "1") {
      نتیجہ = سچ;
      واپس سچ;
    } ورنہ اگر (قدر.equals_insensitive("false") ||
               قدر.equals_insensitive("off") ||
               قدر.equals_insensitive("no") || قدر == "0") {
      نتیجہ = جھوٹ;
      واپس سچ;
    }

    error(N, "expected boolean value");
    واپس جھوٹ;
  }

  std::optional<RedirectingFileSystem::Redirect_قسم>
  parse_Redirect_قسم(yaml::گرہ *N) {
    SmallString<12> Storage;
    StringRef قدر;
    اگر (!parseScalarString(N, قدر, Storage))
      واپس std::nullopt;

    اگر (قدر.equals_insensitive("fallthrough")) {
      واپس RedirectingFileSystem::Redirect_قسم::Fallthrough;
    } ورنہ اگر (قدر.equals_insensitive("fallback")) {
      واپس RedirectingFileSystem::Redirect_قسم::Fallback;
    } ورنہ اگر (قدر.equals_insensitive("redirect-only")) {
      واپس RedirectingFileSystem::Redirect_قسم::RedirectOnly;
    }
    واپس std::nullopt;
  }

  std::optional<RedirectingFileSystem::جڑ_Relative_قسم>
  parse_جڑ_Relative_قسم(yaml::گرہ *N) {
    SmallString<12> Storage;
    StringRef قدر;
    اگر (!parseScalarString(N, قدر, Storage))
      واپس std::nullopt;
    اگر (قدر.equals_insensitive("cwd")) {
      واپس RedirectingFileSystem::جڑ_Relative_قسم::CWD;
    } ورنہ اگر (قدر.equals_insensitive("overlay-dir")) {
      واپس RedirectingFileSystem::جڑ_Relative_قسم::OverlayDir;
    }
    واپس std::nullopt;
  }

  ساخت کلید_حالت {
    منطقی Required;
    منطقی نظر_آیا = جھوٹ;

    کلید_حالت(منطقی Required = جھوٹ) : Required(Required) {}
  };

  استعمال کلید_حالت_Pair = std::pair<StringRef, کلید_حالت>;

  // false on error
  منطقی check_Duplicate_Or_Unknown_کلید(yaml::گرہ *کلید_گرہ, StringRef کلید,
                                  DenseMap<StringRef, کلید_حالت> &کلیدیں_2) {
    خودکار It = کلیدیں_2.ڈھونڈو(کلید);
    اگر (It == کلیدیں_2.انتہا()) {
      error(کلید_گرہ, "unknown key");
      واپس جھوٹ;
    }
    کلید_حالت &S = It->دوسرا_2;
    اگر (S.نظر_آیا) {
      error(کلید_گرہ, Twine("duplicate key '") + کلید + "'");
      واپس جھوٹ;
    }
    S.نظر_آیا = سچ;
    واپس سچ;
  }

  // false on error
  منطقی check_Missing_کلیدیں(yaml::گرہ *Obj, DenseMap<StringRef, کلید_حالت> &کلیدیں_2) {
    برائے (ثابت خودکار &I : کلیدیں_2) {
      اگر (I.دوسرا_2.Required && !I.دوسرا_2.نظر_آیا) {
        error(Obj, Twine("missing key '") + I.پہلا_2 + "'");
        واپس جھوٹ;
      }
    }
    واپس سچ;
  }

عوامی:
  ساکن RedirectingFileSystem::اندراج_2 *
  lookup_Or_Create_اندراج(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::اندراج_2 *والد_اندراج = خالی_اشاریہ) {
    اگر (!والد_اندراج) { // Look for a existent root
      برائے (ثابت خودکار &جڑ_2 : FS->Roots) {
        اگر (Name == جڑ_2->getName()) {
          والد_اندراج = جڑ_2.get();
          واپس والد_اندراج;
        }
      }
    } ورنہ { // Advance to the next component
      خودکار *DE = dyn_cast<RedirectingFileSystem::Directory_اندراج>(والد_اندراج);
      برائے (std::unique_ptr<RedirectingFileSystem::اندراج_2> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_انتہا())) {
        خودکار *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_اندراج>(Content.get());
        اگر (DirContent && Name == Content->getName())
          واپس DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::اندراج_2> E =
        std::make_unique<RedirectingFileSystem::Directory_اندراج>(
            Name, حالت_2("", get_اگلا_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    اگر (!والد_اندراج) { // Add a new root to the overlay
      FS->Roots.ڈالو_عقب(std::move(E));
      والد_اندراج = FS->Roots.عقب().get();
      واپس والد_اندراج;
    }

    خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(والد_اندراج);
    DE->جوڑو_Content(std::move(E));
    واپس DE->get_آخری_Content();
  }

نجی:
  خالی unique_Overlay_درخت(RedirectingFileSystem *FS,
                         RedirectingFileSystem::اندراج_2 *SrcE,
                         RedirectingFileSystem::اندراج_2 *New_والد_E = خالی_اشاریہ) {
    StringRef Name = SrcE->getName();
    انتخاب (SrcE->get_قسم()) {
    حالت RedirectingFileSystem::EK_Directory: {
      خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      اگر (!Name.خالی_خانے())
        New_والد_E = lookup_Or_Create_اندراج(FS, Name, New_والد_E);
      برائے (std::unique_ptr<RedirectingFileSystem::اندراج_2> &Sub_اندراج :
           llvm::make_range(DE->contents_begin(), DE->contents_انتہا()))
        unique_Overlay_درخت(FS, Sub_اندراج.get(), New_والد_E);
      توڑو;
    }
    حالت RedirectingFileSystem::EK_DirectoryRemap: {
      أكد(New_والد_E && "Parent entry must exist");
      خودکار *DR = cast<RedirectingFileSystem::Directory_Remap_اندراج>(SrcE);
      خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(New_والد_E);
      DE->جوڑو_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_اندراج>(
              Name, DR->get_External_Contents_راستہ(), DR->getUseName()));
      توڑو;
    }
    حالت RedirectingFileSystem::EK_File: {
      أكد(New_والد_E && "Parent entry must exist");
      خودکار *FE = cast<RedirectingFileSystem::File_اندراج>(SrcE);
      خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(New_والد_E);
      DE->جوڑو_Content(std::make_unique<RedirectingFileSystem::File_اندراج>(
          Name, FE->get_External_Contents_راستہ(), FE->getUseName()));
      توڑو;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::اندراج_2>
  parse_اندراج(yaml::گرہ *N, RedirectingFileSystem *FS, منطقی Is_جڑ_اندراج) {
    خودکار *M = dyn_cast<yaml::Mapping_گرہ>(N);
    اگر (!M) {
      error(N, "expected mapping node for file or directory entry");
      واپس خالی_اشاریہ;
    }

    کلید_حالت_Pair Fields[] = {
        کلید_حالت_Pair("name", سچ),
        کلید_حالت_Pair("type", سچ),
        کلید_حالت_Pair("contents", جھوٹ),
        کلید_حالت_Pair("external-contents", جھوٹ),
        کلید_حالت_Pair("use-external-name", جھوٹ),
    };

    DenseMap<StringRef, کلید_حالت> کلیدیں_2(std::begin(Fields), std::انتہا(Fields));

    شمار { CF_Not_سیٹ, CF_List, CF_External } Contents_میدان = CF_Not_سیٹ;
    std::سمتیہ<std::unique_ptr<RedirectingFileSystem::اندراج_2>>
        اندراج_Array_Contents;
    SmallString<256> External_Contents_راستہ;
    SmallString<256> Name;
    yaml::گرہ *Name_قدر_گرہ = خالی_اشاریہ;
    خودکار UseExternalName = RedirectingFileSystem::NK_Not_سیٹ;
    RedirectingFileSystem::اندراج_قسم قسم_3;

    برائے (خودکار &I : *M) {
      StringRef کلید;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> بفر;
      اگر (!parseScalarString(I.get_کلید(), کلید, بفر))
        واپس خالی_اشاریہ;

      اگر (!check_Duplicate_Or_Unknown_کلید(I.get_کلید(), کلید, کلیدیں_2))
        واپس خالی_اشاریہ;

      StringRef قدر;
      اگر (کلید == "name") {
        اگر (!parseScalarString(I.get_قدر(), قدر, بفر))
          واپس خالی_اشاریہ;

        Name_قدر_گرہ = I.get_قدر();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(قدر).str();
      } ورنہ اگر (کلید == "type") {
        اگر (!parseScalarString(I.get_قدر(), قدر, بفر))
          واپس خالی_اشاریہ;
        اگر (قدر == "file")
          قسم_3 = RedirectingFileSystem::EK_File;
        ورنہ اگر (قدر == "directory")
          قسم_3 = RedirectingFileSystem::EK_Directory;
        ورنہ اگر (قدر == "directory-remap")
          قسم_3 = RedirectingFileSystem::EK_DirectoryRemap;
        ورنہ {
          error(I.get_قدر(), "unknown value for 'type'");
          واپس خالی_اشاریہ;
        }
      } ورنہ اگر (کلید == "contents") {
        اگر (Contents_میدان != CF_Not_سیٹ) {
          error(I.get_کلید(),
                "entry already has 'contents' or 'external-contents'");
          واپس خالی_اشاریہ;
        }
        Contents_میدان = CF_List;
        خودکار *Contents = dyn_cast_if_present<yaml::Sequence_گرہ>(I.get_قدر());
        اگر (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_قدر(), "expected array");
          واپس خالی_اشاریہ;
        }

        برائے (خودکار &I : *Contents) {
          اگر (std::unique_ptr<RedirectingFileSystem::اندراج_2> E =
                  parse_اندراج(&I, FS, /*IsRootEntry*/ جھوٹ))
            اندراج_Array_Contents.ڈالو_عقب(std::move(E));
          ورنہ
            واپس خالی_اشاریہ;
        }
      } ورنہ اگر (کلید == "external-contents") {
        اگر (Contents_میدان != CF_Not_سیٹ) {
          error(I.get_کلید(),
                "entry already has 'contents' or 'external-contents'");
          واپس خالی_اشاریہ;
        }
        Contents_میدان = CF_External;
        اگر (!parseScalarString(I.get_قدر(), قدر, بفر))
          واپس خالی_اشاریہ;

        SmallString<256> بھرا_راستہ;
        اگر (FS->IsRelativeOverlay) {
          بھرا_راستہ = FS->getOverlayFileDir();
          أكد(!بھرا_راستہ.خالی_خانے() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_بھرا_راستہ = قدر;
          اگر (FS->makeAbsolute(بھرا_راستہ, Abs_بھرا_راستہ)) {
            error(N, "failed to make 'external-contents' absolute");
            واپس خالی_اشاریہ;
          }
          بھرا_راستہ = Abs_بھرا_راستہ;
        } ورنہ {
          بھرا_راستہ = قدر;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        بھرا_راستہ = canonicalize(بھرا_راستہ);
        External_Contents_راستہ = بھرا_راستہ.str();
      } ورنہ اگر (کلید == "use-external-name") {
        منطقی Val;
        اگر (!parseScalarBool(I.get_قدر(), Val))
          واپس خالی_اشاریہ;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } ورنہ {
        llvm_unreachable("key missing from Keys");
      }
    }

    اگر (دھارا.failed())
      واپس خالی_اشاریہ;

    // check for missing keys
    اگر (Contents_میدان == CF_Not_سیٹ) {
      error(N, "missing key 'contents' or 'external-contents'");
      واپس خالی_اشاریہ;
    }
    اگر (!check_Missing_کلیدیں(N, کلیدیں_2))
      واپس خالی_اشاریہ;

    // check invalid configuration
    اگر (قسم_3 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_سیٹ) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      واپس خالی_اشاریہ;
    }

    اگر (قسم_3 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_میدان == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      واپس خالی_اشاریہ;
    }

    sys::راستہ_2::Style راستہ_style = sys::راستہ_2::Style::native;
    اگر (Is_جڑ_اندراج) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      اگر (sys::راستہ_2::is_absolute(Name, sys::راستہ_2::Style::posix)) {
        راستہ_style = sys::راستہ_2::Style::posix;
      } ورنہ اگر (sys::راستہ_2::is_absolute(Name,
                                        sys::راستہ_2::Style::windows_backslash)) {
        راستہ_style = sys::راستہ_2::Style::windows_backslash;
      } ورنہ {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_رمز EC;
        اگر (FS->جڑ_Relative ==
            RedirectingFileSystem::جڑ_Relative_قسم::OverlayDir) {
          StringRef بھرا_راستہ = FS->getOverlayFileDir();
          أكد(!بھرا_راستہ.خالی_خانے() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(بھرا_راستہ, Name);
          Name = canonicalize(Name);
        } ورنہ {
          EC = FS->makeAbsolute(Name);
        }
        اگر (EC) {
          أكد(Name_قدر_گرہ && "Name presence should be checked earlier");
          error(
              Name_قدر_گرہ,
              "entry with relative path at the root level is not discoverable");
          واپس خالی_اشاریہ;
        }
        راستہ_style = sys::راستہ_2::is_absolute(Name, sys::راستہ_2::Style::posix)
                         ? sys::راستہ_2::Style::posix
                         : sys::راستہ_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      اگر (راستہ_style == sys::راستہ_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::راستہ_2::Style::windows_backslash)
        راستہ_style = sys::راستہ_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t جڑ_راستہ_Len = sys::راستہ_2::جڑ_راستہ_2(Trimmed, راستہ_style).سائز_2();
    جب (Trimmed.سائز_2() > جڑ_راستہ_Len &&
           sys::راستہ_2::is_separator(Trimmed.عقب(), راستہ_style))
      Trimmed = Trimmed.قاش(0, Trimmed.سائز_2() - 1);

    // Get the last component
    StringRef آخری_Component = sys::راستہ_2::filename(Trimmed, راستہ_style);

    std::unique_ptr<RedirectingFileSystem::اندراج_2> نتیجہ;
    انتخاب (قسم_3) {
    حالت RedirectingFileSystem::EK_File:
      نتیجہ = std::make_unique<RedirectingFileSystem::File_اندراج>(
          آخری_Component, std::move(External_Contents_راستہ), UseExternalName);
      توڑو;
    حالت RedirectingFileSystem::EK_DirectoryRemap:
      نتیجہ = std::make_unique<RedirectingFileSystem::Directory_Remap_اندراج>(
          آخری_Component, std::move(External_Contents_راستہ), UseExternalName);
      توڑو;
    حالت RedirectingFileSystem::EK_Directory:
      نتیجہ = std::make_unique<RedirectingFileSystem::Directory_اندراج>(
          آخری_Component, std::move(اندراج_Array_Contents),
          حالت_2("", get_اگلا_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      توڑو;
    }

    StringRef والد_2 = sys::راستہ_2::والد_راستہ(Trimmed, راستہ_style);
    اگر (والد_2.خالی_خانے())
      واپس نتیجہ;

    // if 'name' contains multiple components, create implicit directory entries
    برائے (sys::راستہ_2::الٹ_iterator I = sys::راستہ_2::rbegin(والد_2, راستہ_style),
                                     E = sys::راستہ_2::rend(والد_2);
         I != E; ++I) {
      std::سمتیہ<std::unique_ptr<RedirectingFileSystem::اندراج_2>> Entries;
      Entries.ڈالو_عقب(std::move(نتیجہ));
      نتیجہ = std::make_unique<RedirectingFileSystem::Directory_اندراج>(
          *I, std::move(Entries),
          حالت_2("", get_اگلا_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    واپس نتیجہ;
  }

عوامی:
  RedirectingFileSystemParser(yaml::دھارا &S) : دھارا(S) {}

  // false on error
  منطقی parse(yaml::گرہ *جڑ_2, RedirectingFileSystem *FS) {
    خودکار *چوٹی = dyn_cast<yaml::Mapping_گرہ>(جڑ_2);
    اگر (!چوٹی) {
      error(جڑ_2, "expected mapping node");
      واپس جھوٹ;
    }

    کلید_حالت_Pair Fields[] = {
        کلید_حالت_Pair("version", سچ),
        کلید_حالت_Pair("case-sensitive", جھوٹ),
        کلید_حالت_Pair("use-external-names", جھوٹ),
        کلید_حالت_Pair("root-relative", جھوٹ),
        کلید_حالت_Pair("overlay-relative", جھوٹ),
        کلید_حالت_Pair("fallthrough", جھوٹ),
        کلید_حالت_Pair("redirecting-with", جھوٹ),
        کلید_حالت_Pair("roots", سچ),
    };

    DenseMap<StringRef, کلید_حالت> کلیدیں_2(std::begin(Fields), std::انتہا(Fields));
    std::سمتیہ<std::unique_ptr<RedirectingFileSystem::اندراج_2>> جڑ_Entries;

    // Parse configuration and 'roots'
    برائے (خودکار &I : *چوٹی) {
      SmallString<10> کلید_بفر;
      StringRef کلید;
      اگر (!parseScalarString(I.get_کلید(), کلید, کلید_بفر))
        واپس جھوٹ;

      اگر (!check_Duplicate_Or_Unknown_کلید(I.get_کلید(), کلید, کلیدیں_2))
        واپس جھوٹ;

      اگر (کلید == "roots") {
        خودکار *Roots = dyn_cast_if_present<yaml::Sequence_گرہ>(I.get_قدر());
        اگر (!Roots) {
          error(I.get_قدر(), "expected array");
          واپس جھوٹ;
        }

        برائے (خودکار &I : *Roots) {
          اگر (std::unique_ptr<RedirectingFileSystem::اندراج_2> E =
                  parse_اندراج(&I, FS, /*IsRootEntry*/ سچ))
            جڑ_Entries.ڈالو_عقب(std::move(E));
          ورنہ
            واپس جھوٹ;
        }
      } ورنہ اگر (کلید == "version") {
        StringRef نسخہ_String;
        SmallString<4> Storage;
        اگر (!parseScalarString(I.get_قدر(), نسخہ_String, Storage))
          واپس جھوٹ;
        صحیح نسخہ;
        اگر (نسخہ_String.getAsInteger<صحیح>(10, نسخہ)) {
          error(I.get_قدر(), "expected integer");
          واپس جھوٹ;
        }
        اگر (نسخہ < 0) {
          error(I.get_قدر(), "invalid version number");
          واپس جھوٹ;
        }
        اگر (نسخہ != 0) {
          error(I.get_قدر(), "version mismatch, expected 0");
          واپس جھوٹ;
        }
      } ورنہ اگر (کلید == "case-sensitive") {
        اگر (!parseScalarBool(I.get_قدر(), FS->CaseSensitive))
          واپس جھوٹ;
      } ورنہ اگر (کلید == "overlay-relative") {
        اگر (!parseScalarBool(I.get_قدر(), FS->IsRelativeOverlay))
          واپس جھوٹ;
      } ورنہ اگر (کلید == "use-external-names") {
        اگر (!parseScalarBool(I.get_قدر(), FS->Use_External_نام))
          واپس جھوٹ;
      } ورنہ اگر (کلید == "fallthrough") {
        اگر (کلیدیں_2["redirecting-with"].نظر_آیا) {
          error(I.get_قدر(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          واپس جھوٹ;
        }

        منطقی ShouldFallthrough = جھوٹ;
        اگر (!parseScalarBool(I.get_قدر(), ShouldFallthrough))
          واپس جھوٹ;

        اگر (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::Redirect_قسم::Fallthrough;
        } ورنہ {
          FS->Redirection = RedirectingFileSystem::Redirect_قسم::RedirectOnly;
        }
      } ورنہ اگر (کلید == "redirecting-with") {
        اگر (کلیدیں_2["fallthrough"].نظر_آیا) {
          error(I.get_قدر(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          واپس جھوٹ;
        }

        اگر (خودکار قسم_3 = parse_Redirect_قسم(I.get_قدر())) {
          FS->Redirection = *قسم_3;
        } ورنہ {
          error(I.get_قدر(), "expected valid redirect kind");
          واپس جھوٹ;
        }
      } ورنہ اگر (کلید == "root-relative") {
        اگر (خودکار قسم_3 = parse_جڑ_Relative_قسم(I.get_قدر())) {
          FS->جڑ_Relative = *قسم_3;
        } ورنہ {
          error(I.get_قدر(), "expected valid root-relative kind");
          واپس جھوٹ;
        }
      } ورنہ {
        llvm_unreachable("key missing from Keys");
      }
    }

    اگر (دھارا.failed())
      واپس جھوٹ;

    اگر (!check_Missing_کلیدیں(چوٹی, کلیدیں_2))
      واپس جھوٹ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    برائے (خودکار &E : جڑ_Entries)
      unique_Overlay_درخت(FS, E.get());

    واپس سچ;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<حافظہ_بفر> بفر,
                              ماخذ_Mgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_راستہ, خالی *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ماخذ_Mgr SM;
  yaml::دھارا دھارا(بفر->get_Mem_بفر_Ref(), SM);

  SM.سیٹ_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = دھارا.begin();
  yaml::گرہ *جڑ_2 = DI->get_جڑ();
  اگر (DI == دھارا.انتہا() || !جڑ_2) {
    SM.Print_پیغام(SMLoc(), ماخذ_Mgr::DK_Error, "expected root node");
    واپس خالی_اشاریہ;
  }

  RedirectingFileSystemParser P(دھارا);

  std::unique_ptr<RedirectingFileSystem> FS(
      نیا RedirectingFileSystem(ExternalFS));

  اگر (!YAMLFile_راستہ.خالی_خانے()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::راستہ_2::والد_راستہ(YAMLFile_راستہ);
    std::error_رمز EC = FS->makeAbsolute(OverlayAbsDir);
    أكد(!EC && "Overlay dir final path must be absolute");
    (خالی)EC;
    FS->سیٹ_Overlay_File_Dir(OverlayAbsDir);
  }

  اگر (!P.parse(جڑ_2, FS.get()))
    واپس خالی_اشاریہ;

  واپس FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    منطقی Use_External_نام, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      نیا RedirectingFileSystem(ExternalFS));
  FS->Use_External_نام = Use_External_نام;

  StringMap<RedirectingFileSystem::اندراج_2 *> Entries;

  برائے (خودکار &Mapping : llvm::الٹ(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.پہلا_2);
    SmallString<128> مقصد = StringRef(Mapping.دوسرا_2);
    {
      خودکار EC = ExternalFS->makeAbsolute(From);
      (خالی)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::اندراج_2 *&مقصد_اندراج = Entries[From];
    اگر (مقصد_اندراج)
      جاری;

    // Add parent directories.
    RedirectingFileSystem::اندراج_2 *والد_2 = خالی_اشاریہ;
    StringRef FromDirectory = llvm::sys::راستہ_2::والد_راستہ(From);
    برائے (خودکار I = llvm::sys::راستہ_2::begin(FromDirectory),
              E = llvm::sys::راستہ_2::انتہا(FromDirectory);
         I != E; ++I) {
      والد_2 = RedirectingFileSystemParser::lookup_Or_Create_اندراج(FS.get(), *I,
                                                                والد_2);
    }
    أكد(والد_2 && "File without a directory?");
    {
      خودکار EC = ExternalFS->makeAbsolute(مقصد);
      (خالی)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Add the file.
    خودکار NewFile = std::make_unique<RedirectingFileSystem::File_اندراج>(
        llvm::sys::راستہ_2::filename(From), مقصد,
        Use_External_نام ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    مقصد_اندراج = NewFile.get();
    cast<RedirectingFileSystem::Directory_اندراج>(والد_2)->جوڑو_Content(
        std::move(NewFile));
  }

  واپس FS;
}

RedirectingFileSystem::Lookup_نتیجہ::Lookup_نتیجہ(
    اندراج_2 *E, sys::راستہ_2::const_iterator آغاز, sys::راستہ_2::const_iterator انتہا_2)
    : E(E) {
  أكد(E != خالی_اشاریہ);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  اگر (خودکار *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_اندراج>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_راستہ());
    sys::راستہ_2::append(Redirect, آغاز, انتہا_2,
                      getExistingStyle(DRE->get_External_Contents_راستہ()));
    ExternalRedirect = std::string(Redirect);
  }
}

خالی RedirectingFileSystem::Lookup_نتیجہ::get_راستہ(
    llvm::Small_سمتیہ_Impl<حرف> &نتیجہ) ثابت {
  نتیجہ.clear();
  برائے (اندراج_2 *والد_2 : Parents)
    llvm::sys::راستہ_2::append(نتیجہ, والد_2->getName());
  llvm::sys::راستہ_2::append(نتیجہ, E->getName());
}

std::error_رمز RedirectingFileSystem::makeCanonicalForLookup(
    Small_سمتیہ_Impl<حرف> &راستہ) ثابت {
  اگر (std::error_رمز EC = makeAbsolute(راستہ))
    واپس EC;

  llvm::SmallString<256> Canonical_راستہ =
      canonicalize(StringRef(راستہ.ڈیٹا(), راستہ.سائز_2()));
  اگر (Canonical_راستہ.خالی_خانے())
    واپس make_error_رمز(llvm::errc::invalid_دلیل);

  راستہ.تفویض(Canonical_راستہ.begin(), Canonical_راستہ.انتہا());
  واپس {};
}

ErrorOr<RedirectingFileSystem::Lookup_نتیجہ>
RedirectingFileSystem::lookup_راستہ(StringRef راستہ) ثابت {
  llvm::SmallString<128> Canonical_راستہ(راستہ);
  اگر (std::error_رمز EC = makeCanonicalForLookup(Canonical_راستہ))
    واپس EC;

  // RedirectOnly means the VFS is always used.
  اگر (Usage_Tracking_فعال && Redirection == Redirect_قسم::RedirectOnly)
    Has_Been_مستعمل = سچ;

  sys::راستہ_2::const_iterator آغاز = sys::راستہ_2::begin(Canonical_راستہ);
  sys::راستہ_2::const_iterator انتہا_2 = sys::راستہ_2::انتہا(Canonical_راستہ);
  llvm::Small_سمتیہ<اندراج_2 *, 32> Entries;
  برائے (ثابت خودکار &جڑ_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ =
        lookup_راستہ_Impl(آغاز, انتہا_2, جڑ_2.get(), Entries);
    اگر (Usage_Tracking_فعال && نتیجہ && isa<Remap_اندراج>(نتیجہ->E))
      Has_Been_مستعمل = سچ;
    اگر (نتیجہ) {
      نتیجہ->Parents = std::move(Entries);
      واپس نتیجہ;
    }

    اگر (نتیجہ.getError() != llvm::errc::no_such_file_or_directory)
      واپس نتیجہ;
  }
  واپس make_error_رمز(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_نتیجہ>
RedirectingFileSystem::lookup_راستہ_Impl(
    sys::راستہ_2::const_iterator آغاز, sys::راستہ_2::const_iterator انتہا_2,
    RedirectingFileSystem::اندراج_2 *From,
    llvm::Small_سمتیہ_Impl<اندراج_2 *> &Entries) ثابت {
  أكد(!isTraversalComponent(*آغاز) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  اگر (!FromName.خالی_خانے()) {
    اگر (!راستہ_Component_Matches(*آغاز, FromName))
      واپس make_error_رمز(llvm::errc::no_such_file_or_directory);

    ++آغاز;

    اگر (آغاز == انتہا_2) {
      // Match!
      واپس Lookup_نتیجہ(From, آغاز, انتہا_2);
    }
  }

  اگر (isa<RedirectingFileSystem::File_اندراج>(From))
    واپس make_error_رمز(llvm::errc::not_a_directory);

  اگر (isa<RedirectingFileSystem::Directory_Remap_اندراج>(From))
    واپس Lookup_نتیجہ(From, آغاز, انتہا_2);

  خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(From);
  برائے (ثابت std::unique_ptr<RedirectingFileSystem::اندراج_2> &Dir_اندراج :
       llvm::make_range(DE->contents_begin(), DE->contents_انتہا())) {
    Entries.ڈالو_عقب(From);
    ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ =
        lookup_راستہ_Impl(آغاز, انتہا_2, Dir_اندراج.get(), Entries);
    اگر (نتیجہ || نتیجہ.getError() != llvm::errc::no_such_file_or_directory)
      واپس نتیجہ;
    Entries.نکالو_عقب();
  }

  واپس make_error_رمز(llvm::errc::no_such_file_or_directory);
}

ساکن حالت_2 get_Redirected_File_حالت(ثابت Twine &Original_راستہ,
                                      منطقی Use_External_نام,
                                      حالت_2 External_حالت) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  اگر (External_حالت.ExposesExternalVFSPath)
    واپس External_حالت;

  حالت_2 S = External_حالت;
  اگر (!Use_External_نام)
    S = حالت_2::copyWithNewName(S, Original_راستہ);
  ورنہ
    S.ExposesExternalVFSPath = سچ;
  واپس S;
}

ErrorOr<حالت_2> RedirectingFileSystem::حالت_3(
    ثابت Twine &Lookup_راستہ, ثابت Twine &Original_راستہ,
    ثابت RedirectingFileSystem::Lookup_نتیجہ &نتیجہ) {
  اگر (std::optional<StringRef> ExtRedirect = نتیجہ.getExternalRedirect()) {
    SmallString<256> Remapped_راستہ((*ExtRedirect).str());
    اگر (std::error_رمز EC = makeAbsolute(Remapped_راستہ))
      واپس EC;

    ErrorOr<حالت_2> S = ExternalFS->حالت_3(Remapped_راستہ);
    اگر (!S)
      واپس S;
    S = حالت_2::copyWithNewName(*S, *ExtRedirect);
    خودکار *RE = cast<RedirectingFileSystem::Remap_اندراج>(نتیجہ.E);
    واپس get_Redirected_File_حالت(Original_راستہ,
                                   RE->useExternalName(Use_External_نام), *S);
  }

  خودکار *DE = cast<RedirectingFileSystem::Directory_اندراج>(نتیجہ.E);
  واپس حالت_2::copyWithNewName(DE->get_حالت(), Lookup_راستہ);
}

ErrorOr<حالت_2>
RedirectingFileSystem::get_External_حالت(ثابت Twine &Lookup_راستہ,
                                         ثابت Twine &Original_راستہ) ثابت {
  خودکار نتیجہ = ExternalFS->حالت_3(Lookup_راستہ);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  اگر (!نتیجہ || نتیجہ->ExposesExternalVFSPath)
    واپس نتیجہ;
  واپس حالت_2::copyWithNewName(نتیجہ.get(), Original_راستہ);
}

ErrorOr<حالت_2> RedirectingFileSystem::حالت_3(ثابت Twine &Original_راستہ) {
  SmallString<256> راستہ;
  Original_راستہ.مقصد_سمتیہ(راستہ);

  اگر (std::error_رمز EC = makeAbsolute(راستہ))
    واپس EC;

  اگر (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<حالت_2> S = get_External_حالت(راستہ, Original_راستہ);
    اگر (S)
      واپس S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_راستہ(راستہ);
  اگر (!نتیجہ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    اگر (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_ملا(نتیجہ.getError()))
      واپس get_External_حالت(راستہ, Original_راستہ);
    واپس نتیجہ.getError();
  }

  ErrorOr<حالت_2> S = حالت_3(راستہ, Original_راستہ, *نتیجہ);
  اگر (!S && Redirection == Redirect_قسم::Fallthrough &&
      is_File_Not_ملا(S.getError(), نتیجہ->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    واپس get_External_حالت(راستہ, Original_راستہ);
  }

  واپس S;
}

منطقی RedirectingFileSystem::exists(ثابت Twine &Original_راستہ) {
  SmallString<256> راستہ;
  Original_راستہ.مقصد_سمتیہ(راستہ);

  اگر (makeAbsolute(راستہ))
    واپس جھوٹ;

  اگر (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    اگر (ExternalFS->exists(راستہ))
      واپس سچ;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_راستہ(راستہ);
  اگر (!نتیجہ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    اگر (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_ملا(نتیجہ.getError()))
      واپس ExternalFS->exists(راستہ);
    واپس جھوٹ;
  }

  std::optional<StringRef> ExtRedirect = نتیجہ->getExternalRedirect();
  اگر (!ExtRedirect) {
    أكد(isa<RedirectingFileSystem::Directory_اندراج>(نتیجہ->E));
    واپس سچ;
  }

  SmallString<256> Remapped_راستہ((*ExtRedirect).str());
  اگر (makeAbsolute(Remapped_راستہ))
    واپس جھوٹ;

  اگر (ExternalFS->exists(Remapped_راستہ))
    واپس سچ;

  اگر (Redirection == Redirect_قسم::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    واپس ExternalFS->exists(راستہ);
  }

  واپس جھوٹ;
}

نامخلا {

/// Provide a file wrapper with an overriden status.
جماعت File_With_Fixed_حالت : عوامی File {
  std::unique_ptr<File> InnerFile;
  حالت_2 S;

عوامی:
  File_With_Fixed_حالت(std::unique_ptr<File> InnerFile, حالت_2 S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<حالت_2> حالت_3() override { واپس S; }
  ErrorOr<std::unique_ptr<llvm::حافظہ_بفر>>

  get_بفر(ثابت Twine &Name, int64_t File_سائز, منطقی RequiresNullTerminator,
            منطقی IsVolatile) override {
    واپس InnerFile->get_بفر(Name, File_سائز, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_رمز close() override { واپس InnerFile->close(); }

  خالی سیٹ_راستہ(ثابت Twine &راستہ) override { S = S.copyWithNewName(S, راستہ); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_راستہ(ErrorOr<std::unique_ptr<File>> نتیجہ, ثابت Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  اگر (!نتیجہ || (*نتیجہ)->حالت_3()->ExposesExternalVFSPath)
    واپس نتیجہ;

  ErrorOr<std::unique_ptr<File>> F = std::move(*نتیجہ);
  خودکار Name = F->get()->getName();
  اگر (Name && Name.get() != P.str())
    F->get()->سیٹ_راستہ(P);
  واپس F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ثابت Twine &Original_راستہ) {
  SmallString<256> راستہ;
  Original_راستہ.مقصد_سمتیہ(راستہ);

  اگر (std::error_رمز EC = makeAbsolute(راستہ))
    واپس EC;

  اگر (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    خودکار F = File::get_With_راستہ(ExternalFS->openFileForRead(راستہ), Original_راستہ);
    اگر (F)
      واپس F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_راستہ(راستہ);
  اگر (!نتیجہ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    اگر (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_ملا(نتیجہ.getError()))
      واپس File::get_With_راستہ(ExternalFS->openFileForRead(راستہ), Original_راستہ);
    واپس نتیجہ.getError();
  }

  اگر (!نتیجہ->getExternalRedirect()) // FIXME: errc::not_a_file?
    واپس make_error_رمز(llvm::errc::invalid_دلیل);

  StringRef ExtRedirect = *نتیجہ->getExternalRedirect();
  SmallString<256> Remapped_راستہ(ExtRedirect.str());
  اگر (std::error_رمز EC = makeAbsolute(Remapped_راستہ))
    واپس EC;

  خودکار *RE = cast<RedirectingFileSystem::Remap_اندراج>(نتیجہ->E);

  خودکار ExternalFile =
      File::get_With_راستہ(ExternalFS->openFileForRead(Remapped_راستہ), ExtRedirect);
  اگر (!ExternalFile) {
    اگر (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_ملا(ExternalFile.getError(), نتیجہ->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      واپس File::get_With_راستہ(ExternalFS->openFileForRead(راستہ), Original_راستہ);
    }
    واپس ExternalFile;
  }

  خودکار External_حالت = (*ExternalFile)->حالت_3();
  اگر (!External_حالت)
    واپس External_حالت.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  حالت_2 S = get_Redirected_File_حالت(
      Original_راستہ, RE->useExternalName(Use_External_نام), *External_حالت);
  واپس std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_حالت>(std::move(*ExternalFile), S));
}

std::error_رمز
RedirectingFileSystem::get_Real_راستہ(ثابت Twine &Original_راستہ,
                                   Small_سمتیہ_Impl<حرف> &Output) {
  SmallString<256> راستہ;
  Original_راستہ.مقصد_سمتیہ(راستہ);

  اگر (std::error_رمز EC = makeAbsolute(راستہ))
    واپس EC;

  اگر (Redirection == Redirect_قسم::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_رمز EC = ExternalFS->get_Real_راستہ(راستہ, Output);
    اگر (!EC)
      واپس EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_راستہ(راستہ);
  اگر (!نتیجہ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    اگر (Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_ملا(نتیجہ.getError()))
      واپس ExternalFS->get_Real_راستہ(راستہ, Output);
    واپس نتیجہ.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  اگر (خودکار ExtRedirect = نتیجہ->getExternalRedirect()) {
    خودکار P = ExternalFS->get_Real_راستہ(*ExtRedirect, Output);
    اگر (P && Redirection == Redirect_قسم::Fallthrough &&
        is_File_Not_ملا(P, نتیجہ->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      واپس ExternalFS->get_Real_راستہ(راستہ, Output);
    }
    واپس P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  اگر (Redirection == Redirect_قسم::Fallthrough) {
    نتیجہ->get_راستہ(Output);
    واپس {};
  }
  واپس llvm::errc::invalid_دلیل;
}

خالی RedirectingFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, Small_سمتیہ_Impl<std::string> &برآمد) {
  SmallString<256> راستہ;
  Dir.مقصد_سمتیہ(راستہ);

  اگر (makeAbsolute(راستہ))
    واپس;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ثابت منطقی ConsultsExternalFS = Redirection != Redirect_قسم::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> نتیجہ = lookup_راستہ(راستہ);
  اگر (!نتیجہ) {
    // dir_begin() delegates entirely to ExternalFS.
    اگر (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(راستہ, برآمد);
    واپس;
  }

  انتخاب (نتیجہ->E->get_قسم()) {
  حالت EK_File:
    واپس;
  حالت EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    توڑو;
  حالت EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_راستہ(*نتیجہ->getExternalRedirect());
    اگر (!makeAbsolute(Remapped_راستہ))
      ExternalFS->getDirectoryContentRealSources(Remapped_راستہ, برآمد);
    توڑو;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  اگر (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(راستہ, برآمد);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<حافظہ_بفر> بفر,
                    ماخذ_Mgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_راستہ, خالی *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  واپس RedirectingFileSystem::create(std::move(بفر), DiagHandler,
                                       YAMLFile_راستہ, DiagContext,
                                       std::move(ExternalFS));
}

ساکن خالی getVFSEntries(RedirectingFileSystem::اندراج_2 *SrcE,
                          Small_سمتیہ_Impl<StringRef> &راستہ,
                          Small_سمتیہ_Impl<YAMLVFSEntry> &Entries) {
  خودکار قسم_3 = SrcE->get_قسم();
  اگر (قسم_3 == RedirectingFileSystem::EK_Directory) {
    خودکار *DE = dyn_cast<RedirectingFileSystem::Directory_اندراج>(SrcE);
    أكد(DE && "Must be a directory");
    برائے (std::unique_ptr<RedirectingFileSystem::اندراج_2> &Sub_اندراج :
         llvm::make_range(DE->contents_begin(), DE->contents_انتہا())) {
      راستہ.ڈالو_عقب(Sub_اندراج->getName());
      getVFSEntries(Sub_اندراج.get(), راستہ, Entries);
      راستہ.نکالو_عقب();
    }
    واپس;
  }

  اگر (قسم_3 == RedirectingFileSystem::EK_DirectoryRemap) {
    خودکار *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_اندراج>(SrcE);
    أكد(DR && "Must be a directory remap");
    SmallString<128> VPath;
    برائے (خودکار &Comp : راستہ)
      llvm::sys::راستہ_2::append(VPath, Comp);
    Entries.ڈالو_عقب(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_راستہ()));
    واپس;
  }

  أكد(قسم_3 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  خودکار *FE = dyn_cast<RedirectingFileSystem::File_اندراج>(SrcE);
  أكد(FE && "Must be a file");
  SmallString<128> VPath;
  برائے (خودکار &Comp : راستہ)
    llvm::sys::راستہ_2::append(VPath, Comp);
  Entries.ڈالو_عقب(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_راستہ()));
}

خالی vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_سمتیہ_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_نتیجہ> جڑ_نتیجہ = VFS.lookup_راستہ("/");
  اگر (!جڑ_نتیجہ)
    واپس;
  Small_سمتیہ<StringRef, 8> Components;
  Components.ڈالو_عقب("/");
  getVFSEntries(جڑ_نتیجہ->E, Components, CollectedEntries);
}

UniqueID vfs::get_اگلا_Virtual_Unique_ID() {
  ساکن std::atomic<غیرعلامتی> UID;
  غیرعلامتی ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  واپس UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

خالی YAMLVFSWriter::جوڑو_اندراج(StringRef Virtual_راستہ, StringRef Real_راستہ,
                             منطقی IsDirectory) {
  أكد(sys::راستہ_2::is_absolute(Virtual_راستہ) && "virtual path not absolute");
  أكد(sys::راستہ_2::is_absolute(Real_راستہ) && "real path not absolute");
  أكد(!راستہ_Has_Traversal(Virtual_راستہ) && "path traversal is not supported");
  Mappings.emplace_عقب(Virtual_راستہ, Real_راستہ, IsDirectory);
}

خالی YAMLVFSWriter::جوڑو_File_Mapping(StringRef Virtual_راستہ, StringRef Real_راستہ) {
  جوڑو_اندراج(Virtual_راستہ, Real_راستہ, /*IsDirectory=*/جھوٹ);
}

خالی YAMLVFSWriter::جوڑو_Directory_Mapping(StringRef Virtual_راستہ,
                                        StringRef Real_راستہ) {
  جوڑو_اندراج(Virtual_راستہ, Real_راستہ, /*IsDirectory=*/سچ);
}

نامخلا {

جماعت JSONWriter {
  llvm::raw_ostream &OS;
  Small_سمتیہ<StringRef, 16> Dir_ڈھیر;

  غیرعلامتی getDirIndent() { واپس 4 * Dir_ڈھیر.سائز_2(); }
  غیرعلامتی getFileIndent() { واپس 4 * (Dir_ڈھیر.سائز_2() + 1); }
  منطقی contained_اندر(StringRef والد_2, StringRef راستہ);
  StringRef containedPart(StringRef والد_2, StringRef راستہ);
  خالی آغاز_Directory(StringRef راستہ);
  خالی انتہا_Directory();
  خالی write_اندراج(StringRef VPath, StringRef RPath);

عوامی:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  خالی اكتب(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<منطقی> Use_External_نام,
             std::optional<منطقی> IsCaseSensitive,
             std::optional<منطقی> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

منطقی JSONWriter::contained_اندر(StringRef والد_2, StringRef راستہ) {
  استعمال نامخلا llvm::sys;

  // Compare each path component.
  خودکار IParent = راستہ_2::begin(والد_2), EParent = راستہ_2::انتہا(والد_2);
  برائے (خودکار IChild = راستہ_2::begin(راستہ), EChild = راستہ_2::انتہا(راستہ);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    اگر (*IParent != *IChild)
      واپس جھوٹ;
  }
  // Have we exhausted the parent path?
  واپس IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef والد_2, StringRef راستہ) {
  أكد(!والد_2.خالی_خانے());
  أكد(contained_اندر(والد_2, راستہ));
  واپس راستہ.substr(والد_2.سائز_2() + 1);
}

خالی JSONWriter::آغاز_Directory(StringRef راستہ) {
  StringRef Name =
      Dir_ڈھیر.خالی_خانے() ? راستہ : containedPart(Dir_ڈھیر.عقب(), راستہ);
  Dir_ڈھیر.ڈالو_عقب(راستہ);
  غیرعلامتی Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

خالی JSONWriter::انتہا_Directory() {
  غیرعلامتی Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ڈھیر.نکالو_عقب();
}

خالی JSONWriter::write_اندراج(StringRef VPath, StringRef RPath) {
  غیرعلامتی Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

خالی JSONWriter::اكتب(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<منطقی> Use_External_نام,
                       std::optional<منطقی> IsCaseSensitive,
                       std::optional<منطقی> IsOverlayRelative,
                       StringRef OverlayDir) {
  استعمال نامخلا llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  اگر (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  اگر (Use_External_نام)
    OS << "  'use-external-names': '" << (*Use_External_نام ? "true" : "false")
       << "',\n";
  منطقی UseOverlayRelative = جھوٹ;
  اگر (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  اگر (!Entries.خالی_خانے()) {
    ثابت YAMLVFSEntry &اندراج_2 = Entries.front();

    آغاز_Directory(
      اندراج_2.IsDirectory ? اندراج_2.VPath : راستہ_2::والد_راستہ(اندراج_2.VPath)
    );

    StringRef RPath = اندراج_2.RPath;
    اگر (UseOverlayRelative) {
      أكد(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.سائز_2());
    }

    منطقی Is_Current_Dir_خالی_خانے = سچ;
    اگر (!اندراج_2.IsDirectory) {
      write_اندراج(راستہ_2::filename(اندراج_2.VPath), RPath);
      Is_Current_Dir_خالی_خانے = جھوٹ;
    }

    برائے (ثابت خودکار &اندراج_2 : Entries.قاش(1)) {
      StringRef Dir =
          اندراج_2.IsDirectory ? اندراج_2.VPath : راستہ_2::والد_راستہ(اندراج_2.VPath);
      اگر (Dir == Dir_ڈھیر.عقب()) {
        اگر (!Is_Current_Dir_خالی_خانے) {
          OS << ",\n";
        }
      } ورنہ {
        منطقی Is_Dir_Popped_From_ڈھیر = جھوٹ;
        جب (!Dir_ڈھیر.خالی_خانے() && !contained_اندر(Dir_ڈھیر.عقب(), Dir)) {
          OS << "\n";
          انتہا_Directory();
          Is_Dir_Popped_From_ڈھیر = سچ;
        }
        اگر (Is_Dir_Popped_From_ڈھیر || !Is_Current_Dir_خالی_خانے) {
          OS << ",\n";
        }
        آغاز_Directory(Dir);
        Is_Current_Dir_خالی_خانے = سچ;
      }
      StringRef RPath = اندراج_2.RPath;
      اگر (UseOverlayRelative) {
        أكد(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.سائز_2());
      }
      اگر (!اندراج_2.IsDirectory) {
        write_اندراج(راستہ_2::filename(اندراج_2.VPath), RPath);
        Is_Current_Dir_خالی_خانے = جھوٹ;
      }
    }

    جب (!Dir_ڈھیر.خالی_خانے()) {
      OS << "\n";
      انتہا_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

خالی YAMLVFSWriter::اكتب(llvm::raw_ostream &OS) {
  llvm::ترتیب_دو(Mappings, [](ثابت YAMLVFSEntry &LHS, ثابت YAMLVFSEntry &RHS) {
    واپس LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).اكتب(Mappings, Use_External_نام, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ثابت Twine &راستہ, std::error_رمز &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(راستہ, EC);
  اگر (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ڈھیر.ڈالو_عقب(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_رمز &EC) {
  أكد(FS && State && !State->ڈھیر.خالی_خانے() && "incrementing past end");
  أكد(!State->ڈھیر.عقب()->راستہ_2().خالی_خانے() && "non-canonical end iterator");
  vfs::directory_iterator انتہا_2;

  اگر (State->Has_No_ڈالو_Request)
    State->Has_No_ڈالو_Request = جھوٹ;
  ورنہ {
    اگر (State->ڈھیر.عقب()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ڈھیر.عقب()->راستہ_2(), EC);
      اگر (I != انتہا_2) {
        State->ڈھیر.ڈالو_عقب(I);
        واپس *یہ;
      }
    }
  }

  جب (!State->ڈھیر.خالی_خانے() && State->ڈھیر.عقب().increment(EC) == انتہا_2)
    State->ڈھیر.نکالو_عقب();

  اگر (State->ڈھیر.خالی_خانے())
    State.reset(); // end iterator

  واپس *یہ;
}

ثابت حرف FileSystem::ID = 0;
ثابت حرف OverlayFileSystem::ID = 0;
ثابت حرف ProxyFileSystem::ID = 0;
ثابت حرف اندر_حافظہ_File_System::ID = 0;
ثابت حرف RedirectingFileSystem::ID = 0;

غیرعلامتی ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_گنتی(
    ثابت FileSystem *FS) {
  واپس FS->Use_گنتی();
}

خالی ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

خالی ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
