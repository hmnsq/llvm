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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualFileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/DenseMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/IntrusiveRefCntPtr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringSet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/iterator_range.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Casting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Chrono.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorOr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem/UniqueID.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/IOSandbox.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SMLoc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SourceMgr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/YAMLParser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <atomic>
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdint>
#ਅਨ੍ਤਰ੍ਭਾਵ <iterator>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits>
#ਅਨ੍ਤਰ੍ਭਾਵ <map>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>
#ਅਨ੍ਤਰ੍ਭਾਵ <utility>
#ਅਨ੍ਤਰ੍ਭਾਵ <vector>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::vfs;

ਵਰਤੋਂ llvm::sys::fs::file_ਸਥਿਤੀ;
ਵਰਤੋਂ llvm::sys::fs::file_t;
ਵਰਤੋਂ llvm::sys::fs::file_type;
ਵਰਤੋਂ llvm::sys::fs::perms;
ਵਰਤੋਂ llvm::sys::fs::UniqueID;

ਸਥਿਤੀ::ਸਥਿਤੀ(ਅਚਲ file_ਸਥਿਤੀ &ਸਥਿਤੀ)
    : UID(ਸਥਿਤੀ.getUniqueID()), MTime(ਸਥਿਤੀ.get_ਆਖਰੀ_Modification_ਸਮਾਂ()),
      User(ਸਥਿਤੀ.getUser()), ਸਮੂਹ(ਸਥਿਤੀ.get_ਸਮੂਹ()), ਮਾਪ(ਸਥਿਤੀ.get_ਮਾਪ()),
      Type(ਸਥਿਤੀ.type()), Perms(ਸਥਿਤੀ.permissions()) {}

ਸਥਿਤੀ::ਸਥਿਤੀ(ਅਚਲ Twine &Name, UniqueID UID, sys::ਸਮਾਂ_ਬਿੰਦੂ<> MTime,
               uint32_t User, uint32_t ਸਮੂਹ, uint64_t ਮਾਪ, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), ਸਮੂਹ(ਸਮੂਹ),
      ਮਾਪ(ਮਾਪ), Type(Type), Perms(Perms) {}

ਸਥਿਤੀ ਸਥਿਤੀ::copy_With_New_ਮਾਪ(ਅਚਲ ਸਥਿਤੀ &ਵਿੱਚ_3, uint64_t New_ਮਾਪ) {
  ਵਾਪਸ ਸਥਿਤੀ(ਵਿੱਚ_3.getName(), ਵਿੱਚ_3.getUniqueID(), ਵਿੱਚ_3.get_ਆਖਰੀ_Modification_ਸਮਾਂ(),
                ਵਿੱਚ_3.getUser(), ਵਿੱਚ_3.get_ਸਮੂਹ(), New_ਮਾਪ, ਵਿੱਚ_3.getType(),
                ਵਿੱਚ_3.getPermissions());
}

ਸਥਿਤੀ ਸਥਿਤੀ::copyWithNewName(ਅਚਲ ਸਥਿਤੀ &ਵਿੱਚ_3, ਅਚਲ Twine &NewName) {
  ਵਾਪਸ ਸਥਿਤੀ(NewName, ਵਿੱਚ_3.getUniqueID(), ਵਿੱਚ_3.get_ਆਖਰੀ_Modification_ਸਮਾਂ(),
                ਵਿੱਚ_3.getUser(), ਵਿੱਚ_3.get_ਸਮੂਹ(), ਵਿੱਚ_3.get_ਮਾਪ(), ਵਿੱਚ_3.getType(),
                ਵਿੱਚ_3.getPermissions());
}

ਸਥਿਤੀ ਸਥਿਤੀ::copyWithNewName(ਅਚਲ file_ਸਥਿਤੀ &ਵਿੱਚ_3, ਅਚਲ Twine &NewName) {
  ਵਾਪਸ ਸਥਿਤੀ(NewName, ਵਿੱਚ_3.getUniqueID(), ਵਿੱਚ_3.get_ਆਖਰੀ_Modification_ਸਮਾਂ(),
                ਵਿੱਚ_3.getUser(), ਵਿੱਚ_3.get_ਸਮੂਹ(), ਵਿੱਚ_3.get_ਮਾਪ(), ਵਿੱਚ_3.type(),
                ਵਿੱਚ_3.permissions());
}

ਤਾਰਕਿਕ ਸਥਿਤੀ::equivalent(ਅਚਲ ਸਥਿਤੀ &Other) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(is_ਸਥਿਤੀ_ਗਿਆਤ() && Other.is_ਸਥਿਤੀ_ਗਿਆਤ());
  ਵਾਪਸ getUniqueID() == Other.getUniqueID();
}

ਤਾਰਕਿਕ ਸਥਿਤੀ::isDirectory() ਅਚਲ { ਵਾਪਸ Type == file_type::directory_file; }

ਤਾਰਕਿਕ ਸਥਿਤੀ::isRegularFile() ਅਚਲ { ਵਾਪਸ Type == file_type::regular_file; }

ਤਾਰਕਿਕ ਸਥਿਤੀ::isOther() ਅਚਲ {
  ਵਾਪਸ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

ਤਾਰਕਿਕ ਸਥਿਤੀ::isSymlink() ਅਚਲ { ਵਾਪਸ Type == file_type::symlink_file; }

ਤਾਰਕਿਕ ਸਥਿਤੀ::is_ਸਥਿਤੀ_ਗਿਆਤ() ਅਚਲ { ਵਾਪਸ Type != file_type::ਸਥਿਤੀ_error; }

ਤਾਰਕਿਕ ਸਥਿਤੀ::exists() ਅਚਲ {
  ਵਾਪਸ is_ਸਥਿਤੀ_ਗਿਆਤ() && Type != file_type::file_not_ਮਿਲਿਆ;
}

File::~File() = ਮੂਲ;

FileSystem::~FileSystem() = ਮੂਲ;

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
FileSystem::get_ਬਫਰ_For_File(ਅਚਲ llvm::Twine &Name, int64_t File_ਮਾਪ,
                             ਤਾਰਕਿਕ RequiresNullTerminator, ਤਾਰਕਿਕ IsVolatile,
                             ਤਾਰਕਿਕ Is_ਪਾਠ) {
  ਸਵੈਚਾਲਿਤ F = Is_ਪਾਠ ? openFileForRead(Name) : open_File_For_Read_ਦੁਆਧਾਰੀ(Name);
  ਜੇ (!F)
    ਵਾਪਸ F.getError();

  ਵਾਪਸ (*F)->get_ਬਫਰ(Name, File_ਮਾਪ, RequiresNullTerminator, IsVolatile);
}

std::error_ਕੋਡ FileSystem::makeAbsolute(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ) ਅਚਲ {
  ਜੇ (llvm::sys::ਰਸਤਾ_2::is_absolute(ਰਸਤਾ))
    ਵਾਪਸ {};

  ਸਵੈਚਾਲਿਤ WorkingDir = getCurrentWorkingDirectory();
  ਜੇ (!WorkingDir)
    ਵਾਪਸ WorkingDir.getError();

  sys::ਰਸਤਾ_2::make_absolute(WorkingDir.get(), ਰਸਤਾ);
  ਵਾਪਸ {};
}

std::error_ਕੋਡ FileSystem::get_Real_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ,
                                        Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Output) {
  ਵਾਪਸ errc::operation_not_permitted;
}

std::error_ਕੋਡ FileSystem::isLocal(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ &ਨਤੀਜਾ) {
  ਵਾਪਸ errc::operation_not_permitted;
}

ਤਾਰਕਿਕ FileSystem::exists(ਅਚਲ Twine &ਰਸਤਾ) {
  ਸਵੈਚਾਲਿਤ ਸਥਿਤੀ = ਸਥਿਤੀ_2(ਰਸਤਾ);
  ਵਾਪਸ ਸਥਿਤੀ && ਸਥਿਤੀ->exists();
}

llvm::ErrorOr<ਤਾਰਕਿਕ> FileSystem::equivalent(ਅਚਲ Twine &A, ਅਚਲ Twine &B) {
  ਸਵੈਚਾਲਿਤ ਸਥਿਤੀ_A = ਸਥਿਤੀ_2(A);
  ਜੇ (!ਸਥਿਤੀ_A)
    ਵਾਪਸ ਸਥਿਤੀ_A.getError();
  ਸਵੈਚਾਲਿਤ ਸਥਿਤੀ_B = ਸਥਿਤੀ_2(B);
  ਜੇ (!ਸਥਿਤੀ_B)
    ਵਾਪਸ ਸਥਿਤੀ_B.getError();
  ਵਾਪਸ ਸਥਿਤੀ_A->equivalent(*ਸਥਿਤੀ_B);
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
ਖਾਲੀ FileSystem::dump() ਅਚਲ { print(dbgs(), PrintType::RecursiveContents); }
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
ਸਥਿਰ ਤਾਰਕਿਕ isTraversalComponent(StringRef Component) {
  ਵਾਪਸ Component == ".." || Component == ".";
}

ਸਥਿਰ ਤਾਰਕਿਕ ਰਸਤਾ_Has_Traversal(StringRef ਰਸਤਾ) {
  ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::sys;

  ਲਈ (StringRef Comp : llvm::make_range(ਰਸਤਾ_2::begin(ਰਸਤਾ), ਰਸਤਾ_2::ਸਿਰਾ(ਰਸਤਾ)))
    ਜੇ (isTraversalComponent(Comp))
      ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ ਝੂਠ;
}
#ਜੇ_ਅੰਤ

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

ਨਾਮਾਕਾਸ਼ {

/// Wrapper around a raw file descriptor.
ਵਰਗ RealFile : ਜਨਤਕ File {
  ਮਿੱਤਰ ਵਰਗ RealFileSystem;

  file_t FD;
  ਸਥਿਤੀ S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_ਰਸਤਾ_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::ਸਥਿਤੀ_error, {}),
        RealName(New_Real_ਰਸਤਾ_Name.str()) {
    ਪ੍ਰਤਿਜ੍ਞਾ(FD.isValid() && "Invalid or inactive file descriptor");
  }

ਜਨਤਕ:
  ~RealFile() override;

  ErrorOr<ਸਥਿਤੀ> ਸਥਿਤੀ_2() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>> get_ਬਫਰ(ਅਚਲ Twine &Name,
                                                   int64_t File_ਮਾਪ,
                                                   ਤਾਰਕਿਕ RequiresNullTerminator,
                                                   ਤਾਰਕਿਕ IsVolatile) override;
  std::error_ਕੋਡ close() override;
  ਖਾਲੀ ਸੈੱਟ_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<ਸਥਿਤੀ> RealFile::ਸਥਿਤੀ_2() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਪ੍ਰਤਿਜ੍ਞਾ(FD.isValid() && "cannot stat closed file");
  ਜੇ (!S.is_ਸਥਿਤੀ_ਗਿਆਤ()) {
    file_ਸਥਿਤੀ Real_ਸਥਿਤੀ;
    ਜੇ (std::error_ਕੋਡ EC = sys::fs::ਸਥਿਤੀ_2(FD, Real_ਸਥਿਤੀ))
      ਵਾਪਸ EC;
    S = ਸਥਿਤੀ::copyWithNewName(Real_ਸਥਿਤੀ, S.getName());
  }
  ਵਾਪਸ S;
}

ErrorOr<std::string> RealFile::getName() {
  ਵਾਪਸ RealName.ਖਾਲੀ_ਥਾਵਾਂ() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
RealFile::get_ਬਫਰ(ਅਚਲ Twine &Name, int64_t File_ਮਾਪ,
                    ਤਾਰਕਿਕ RequiresNullTerminator, ਤਾਰਕਿਕ IsVolatile) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਪ੍ਰਤਿਜ੍ਞਾ(FD.isValid() && "cannot get buffer for closed file");
  ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਬਫਰ::getOpenFile(FD, Name, File_ਮਾਪ, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_ਕੋਡ RealFile::close() {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  std::error_ਕੋਡ EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ਵਾਪਸ EC;
}

ਖਾਲੀ RealFile::ਸੈੱਟ_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  RealName = ਰਸਤਾ.str();
  ਜੇ (ਸਵੈਚਾਲਿਤ ਸਥਿਤੀ = ਸਥਿਤੀ_2())
    S = ਸਥਿਤੀ.get().copyWithNewName(ਸਥਿਤੀ.get(), ਰਸਤਾ);
}

ਨਾਮਾਕਾਸ਼ {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
ਵਰਗ RealFileSystem : ਜਨਤਕ FileSystem {
ਜਨਤਕ:
  ਸਪਸ਼ਟ RealFileSystem(ਤਾਰਕਿਕ LinkCWDToProcess) {
    ਜੇ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      ਜੇ (std::error_ਕੋਡ EC = llvm::sys::fs::current_ਰਸਤਾ(PWD))
        WD = std::move(EC);
      ਨਹੀਂਤਾਂ ਜੇ (llvm::sys::fs::real_ਰਸਤਾ(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      ਨਹੀਂਤਾਂ
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<ਸਥਿਤੀ> ਸਥਿਤੀ_2(ਅਚਲ Twine &ਰਸਤਾ) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ਅਚਲ Twine &ਰਸਤਾ) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ਦੁਆਧਾਰੀ(ਅਚਲ Twine &ਰਸਤਾ) override;
  directory_iterator dir_begin(ਅਚਲ Twine &Dir, std::error_ਕੋਡ &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ਅਚਲ override;
  std::error_ਕੋਡ ਸੈੱਟ_Current_Working_Directory(ਅਚਲ Twine &ਰਸਤਾ) override;
  std::error_ਕੋਡ isLocal(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ &ਨਤੀਜਾ) override;
  ਖਾਲੀ
  getDirectoryContentRealSources(ਅਚਲ Twine &Dir,
                                 Small_ਸਦਿਸ਼_Impl<std::string> &ਬਾਹਰਲਾ) override {
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(Dir.str());
  }
  std::error_ਕੋਡ get_Real_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ,
                              Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Output) override;

ਸੁਰੱਖਿਅਤ:
  ਖਾਲੀ printImpl(raw_ostream &OS, PrintType Type,
                 ਬਿਨਾਨਿਸ਼ਾਨੀ IndentLevel) ਅਚਲ override;

ਨਿਜੀ:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ, Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage) ਅਚਲ {
    ਜੇ (!WD || !*WD)
      ਵਾਪਸ ਰਸਤਾ;
    ਰਸਤਾ.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(Storage);
    sys::ਰਸਤਾ_2::make_absolute(WD->get().Resolved, Storage);
    ਵਾਪਸ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ਅਚਲ Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_ਰਸਤਾ(Name, Storage), Flags, &RealName);
    ਜੇ (!FDOrErr)
      ਵਾਪਸ error_ਮੰਜ਼ਿਲ_Error_ਕੋਡ(FDOrErr.takeError());
    ਵਾਪਸ std::unique_ptr<File>(
        ਨਵਾਂ RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  ਬਣਤਰ WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<ਸਥਿਤੀ> RealFileSystem::ਸਥਿਤੀ_2(ਅਚਲ Twine &ਰਸਤਾ) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_ਸਥਿਤੀ Real_ਸਥਿਤੀ;
  ਜੇ (std::error_ਕੋਡ EC =
          sys::fs::ਸਥਿਤੀ_2(adjust_ਰਸਤਾ(ਰਸਤਾ, Storage), Real_ਸਥਿਤੀ))
    ਵਾਪਸ EC;
  ਵਾਪਸ ਸਥਿਤੀ::copyWithNewName(Real_ਸਥਿਤੀ, ਰਸਤਾ);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ਅਚਲ Twine &Name) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਵਾਪਸ openFileForReadWithFlags(Name, sys::fs::ਦਾ_ਪਾਠ);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ਦੁਆਧਾਰੀ(ਅਚਲ Twine &Name) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਵਾਪਸ openFileForReadWithFlags(Name, sys::fs::ਦਾ_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ਅਚਲ {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਜੇ (WD && *WD)
    ਵਾਪਸ std::string(WD->get().Specified);
  ਜੇ (WD)
    ਵਾਪਸ WD->getError();

  SmallString<128> Dir;
  ਜੇ (std::error_ਕੋਡ EC = llvm::sys::fs::current_ਰਸਤਾ(Dir))
    ਵਾਪਸ EC;
  ਵਾਪਸ std::string(Dir);
}

std::error_ਕੋਡ RealFileSystem::ਸੈੱਟ_Current_Working_Directory(ਅਚਲ Twine &ਰਸਤਾ) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਜੇ (!WD)
    ਵਾਪਸ llvm::sys::fs::ਸੈੱਟ_current_ਰਸਤਾ(ਰਸਤਾ);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_ਰਸਤਾ(ਰਸਤਾ, Storage).ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(Absolute);
  ਤਾਰਕਿਕ IsDir;
  ਜੇ (ਸਵੈਚਾਲਿਤ Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ਵਾਪਸ Err;
  ਜੇ (!IsDir)
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::not_a_directory);
  ਜੇ (ਸਵੈਚਾਲਿਤ Err = llvm::sys::fs::real_ਰਸਤਾ(Absolute, Resolved))
    ਵਾਪਸ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ਵਾਪਸ std::error_ਕੋਡ();
}

std::error_ਕੋਡ RealFileSystem::isLocal(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ &ਨਤੀਜਾ) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ਵਾਪਸ llvm::sys::fs::is_local(adjust_ਰਸਤਾ(ਰਸਤਾ, Storage), ਨਤੀਜਾ);
}

std::error_ਕੋਡ RealFileSystem::get_Real_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ,
                                            Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Output) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ਵਾਪਸ llvm::sys::fs::real_ਰਸਤਾ(adjust_ਰਸਤਾ(ਰਸਤਾ, Storage), Output);
}

ਖਾਲੀ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               ਬਿਨਾਨਿਸ਼ਾਨੀ IndentLevel) ਅਚਲ {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  ਜੇ (WD)
    OS << "own";
  ਨਹੀਂਤਾਂ
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  ਸਥਿਰ IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(ਸੱਚ);
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ਵਾਪਸ std::make_unique<RealFileSystem>(ਝੂਠ);
}

ਨਾਮਾਕਾਸ਼ {

ਵਰਗ RealFSDirIter : ਜਨਤਕ llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

ਜਨਤਕ:
  RealFSDirIter(ਅਚਲ Twine &ਰਸਤਾ, std::error_ਕੋਡ &EC) {
    ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(ਰਸਤਾ, EC);
    ਜੇ (Iter != sys::fs::directory_iterator())
      Current_ਇੰਦਰਾਜ = directory_ਇੰਦਰਾਜ(Iter->ਰਸਤਾ_2(), Iter->type());
  }

  std::error_ਕੋਡ increment() override {
    ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

    std::error_ਕੋਡ EC;
    Iter.increment(EC);
    Current_ਇੰਦਰਾਜ = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_ਇੰਦਰਾਜ()
                       : directory_ਇੰਦਰਾਜ(Iter->ਰਸਤਾ_2(), Iter->type());
    ਵਾਪਸ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ਅਚਲ Twine &Dir,
                                             std::error_ਕੋਡ &EC) {
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ਵਾਪਸ directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_ਰਸਤਾ(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> ਅਧਾਰ_FS) {
  FSList.ਧੱਕੋ_ਪਿੱਠ(std::move(ਅਧਾਰ_FS));
}

ਖਾਲੀ OverlayFileSystem::ਧੱਕੋ_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ਧੱਕੋ_ਪਿੱਠ(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->ਸੈੱਟ_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<ਸਥਿਤੀ> OverlayFileSystem::ਸਥਿਤੀ_2(ਅਚਲ Twine &ਰਸਤਾ) {
  // FIXME: handle symlinks that cross file systems
  ਲਈ (iterator I = overlays_begin(), E = overlays_ਸਿਰਾ(); I != E; ++I) {
    ErrorOr<ਸਥਿਤੀ> ਸਥਿਤੀ = (*I)->ਸਥਿਤੀ_2(ਰਸਤਾ);
    ਜੇ (ਸਥਿਤੀ || ਸਥਿਤੀ.getError() != llvm::errc::no_such_file_or_directory)
      ਵਾਪਸ ਸਥਿਤੀ;
  }
  ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::no_such_file_or_directory);
}

ਤਾਰਕਿਕ OverlayFileSystem::exists(ਅਚਲ Twine &ਰਸਤਾ) {
  // FIXME: handle symlinks that cross file systems
  ਲਈ (iterator I = overlays_begin(), E = overlays_ਸਿਰਾ(); I != E; ++I) {
    ਜੇ ((*I)->exists(ਰਸਤਾ))
      ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ਅਚਲ llvm::Twine &ਰਸਤਾ) {
  // FIXME: handle symlinks that cross file systems
  ਲਈ (iterator I = overlays_begin(), E = overlays_ਸਿਰਾ(); I != E; ++I) {
    ਸਵੈਚਾਲਿਤ ਨਤੀਜਾ = (*I)->openFileForRead(ਰਸਤਾ);
    ਜੇ (ਨਤੀਜਾ || ਨਤੀਜਾ.getError() != llvm::errc::no_such_file_or_directory)
      ਵਾਪਸ ਨਤੀਜਾ;
  }
  ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ਅਚਲ {
  // All file systems are synchronized, just take the first working directory.
  ਵਾਪਸ FSList.front()->getCurrentWorkingDirectory();
}

std::error_ਕੋਡ
OverlayFileSystem::ਸੈੱਟ_Current_Working_Directory(ਅਚਲ Twine &ਰਸਤਾ) {
  ਲਈ (ਸਵੈਚਾਲਿਤ &FS : FSList)
    ਜੇ (std::error_ਕੋਡ EC = FS->ਸੈੱਟ_Current_Working_Directory(ਰਸਤਾ))
      ਵਾਪਸ EC;
  ਵਾਪਸ {};
}

std::error_ਕੋਡ OverlayFileSystem::isLocal(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ &ਨਤੀਜਾ) {
  ਲਈ (ਸਵੈਚਾਲਿਤ &FS : FSList)
    ਜੇ (FS->exists(ਰਸਤਾ))
      ਵਾਪਸ FS->isLocal(ਰਸਤਾ, ਨਤੀਜਾ);
  ਵਾਪਸ errc::no_such_file_or_directory;
}

std::error_ਕੋਡ OverlayFileSystem::get_Real_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ,
                                               Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Output) {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &FS : FSList)
    ਜੇ (FS->exists(ਰਸਤਾ))
      ਵਾਪਸ FS->get_Real_ਰਸਤਾ(ਰਸਤਾ, Output);
  ਵਾਪਸ errc::no_such_file_or_directory;
}

ਖਾਲੀ OverlayFileSystem::getDirectoryContentRealSources(
    ਅਚਲ Twine &Dir, Small_ਸਦਿਸ਼_Impl<std::string> &ਬਾਹਰਲਾ) {
  // All layers contribute.
  ਲਈ (iterator I = overlays_begin(), E = overlays_ਸਿਰਾ(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, ਬਾਹਰਲਾ);
}

ਖਾਲੀ OverlayFileSystem::ਦੇਖੋ_ਬੱਚਾ_File_Systems(ਦੇਖੋ_Callback_Ty Callback) {
  ਲਈ (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->ਦੇਖੋ_ਬੱਚਾ_File_Systems(Callback);
  }
}

ਖਾਲੀ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  ਬਿਨਾਨਿਸ਼ਾਨੀ IndentLevel) ਅਚਲ {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  ਜੇ (Type == PrintType::Summary)
    ਵਾਪਸ;

  ਜੇ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = ਮੂਲ;

ਨਾਮਾਕਾਸ਼ {

/// Combines and deduplicates directory entries across multiple file systems.
ਵਰਗ CombiningDirIterImpl : ਜਨਤਕ llvm::vfs::detail::DirIterImpl {
  ਵਰਤੋਂ FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_ਸਦਿਸ਼<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_ਸੈੱਟ<> ਵੇਖਿਆ_ਨਾਂ;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_ਕੋਡ incrementIter(ਤਾਰਕਿਕ Is_ਪਹਿਲਾ_ਸਮਾਂ) {
    ਜਦੋਂ (!IterList.ਖਾਲੀ_ਥਾਵਾਂ()) {
      CurrentDirIter = IterList.ਪਿੱਠ();
      IterList.ਕੱਢੋ_ਪਿੱਠ();
      ਜੇ (CurrentDirIter != directory_iterator())
        ਰੋਕੋ; //ਮਿਲਿਆ
    }

    ਜੇ (Is_ਪਹਿਲਾ_ਸਮਾਂ && CurrentDirIter == directory_iterator())
      ਵਾਪਸ errc::no_such_file_or_directory;
    ਵਾਪਸ {};
  }

  std::error_ਕੋਡ incrementDirIter(ਤਾਰਕਿਕ Is_ਪਹਿਲਾ_ਸਮਾਂ) {
    ਪ੍ਰਤਿਜ੍ਞਾ((Is_ਪਹਿਲਾ_ਸਮਾਂ || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_ਕੋਡ EC;
    ਜੇ (!Is_ਪਹਿਲਾ_ਸਮਾਂ)
      CurrentDirIter.increment(EC);
    ਜੇ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(Is_ਪਹਿਲਾ_ਸਮਾਂ);
    ਵਾਪਸ EC;
  }

  std::error_ਕੋਡ incrementImpl(ਤਾਰਕਿਕ Is_ਪਹਿਲਾ_ਸਮਾਂ) {
    ਜਦੋਂ (ਸੱਚ) {
      std::error_ਕੋਡ EC = incrementDirIter(Is_ਪਹਿਲਾ_ਸਮਾਂ);
      ਜੇ (EC || CurrentDirIter == directory_iterator()) {
        Current_ਇੰਦਰਾਜ = directory_ਇੰਦਰਾਜ();
        ਵਾਪਸ EC;
      }
      Current_ਇੰਦਰਾਜ = *CurrentDirIter;
      StringRef Name = llvm::sys::ਰਸਤਾ_2::filename(Current_ਇੰਦਰਾਜ.ਰਸਤਾ_2());
      ਜੇ (ਵੇਖਿਆ_ਨਾਂ.ਪਾਓ(Name).ਦੂਜਾ_2)
        ਵਾਪਸ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

ਜਨਤਕ:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_ਕੋਡ &EC) {
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &FS : FileSystems) {
      std::error_ਕੋਡ FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      ਜੇ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ਵਾਪਸ;
      }
      ਜੇ (!FEC)
        IterList.ਧੱਕੋ_ਪਿੱਠ(Iter);
    }
    EC = incrementImpl(ਸੱਚ);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_ਕੋਡ &EC)
      : IterList(DirIters) {
    EC = incrementImpl(ਸੱਚ);
  }

  std::error_ਕੋਡ increment() override { ਵਾਪਸ incrementImpl(ਝੂਠ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ਅਚਲ Twine &Dir,
                                                std::error_ਕੋਡ &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  ਜੇ (EC)
    ਵਾਪਸ {};
  ਵਾਪਸ Combined;
}

ਖਾਲੀ ProxyFileSystem::anchor() {}

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ vfs {

ਨਾਮਾਕਾਸ਼ detail {

ਗਿਣਤੀ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_ਕਿਸਮ {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
ਵਰਗ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ {
  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_ਕਿਸਮ ਕਿਸਮ_3;
  std::string FileName;

ਜਨਤਕ:
  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ(llvm::StringRef FileName, ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_ਕਿਸਮ ਕਿਸਮ_3)
      : ਕਿਸਮ_3(ਕਿਸਮ_3), FileName(std::string(llvm::sys::ਰਸਤਾ_2::filename(FileName))) {
  }
  ਆਭਾਸੀ ~ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ() = ਮੂਲ;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  ਆਭਾਸੀ ਸਥਿਤੀ get_ਸਥਿਤੀ(ਅਚਲ Twine &RequestedName) ਅਚਲ = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ਅਚਲ { ਵਾਪਸ FileName; }
  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_ਕਿਸਮ get_ਕਿਸਮ() ਅਚਲ { ਵਾਪਸ ਕਿਸਮ_3; }
  ਆਭਾਸੀ std::string ਮੰਜ਼ਿਲ_String(ਬਿਨਾਨਿਸ਼ਾਨੀ Indent) ਅਚਲ = 0;
};

ਵਰਗ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File : ਜਨਤਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ {
  ਸਥਿਤੀ Stat;
  std::unique_ptr<llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ> ਬਫਰ;

ਜਨਤਕ:
  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File(ਸਥਿਤੀ Stat, std::unique_ptr<llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ> ਬਫਰ)
      : ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ(Stat.getName(), IME_File), Stat(std::move(Stat)),
        ਬਫਰ(std::move(ਬਫਰ)) {}

  ਸਥਿਤੀ get_ਸਥਿਤੀ(ਅਚਲ Twine &RequestedName) ਅਚਲ override {
    ਵਾਪਸ ਸਥਿਤੀ::copyWithNewName(Stat, RequestedName);
  }
  llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ *get_ਬਫਰ() ਅਚਲ { ਵਾਪਸ ਬਫਰ.get(); }

  std::string ਮੰਜ਼ਿਲ_String(ਬਿਨਾਨਿਸ਼ਾਨੀ Indent) ਅਚਲ override {
    ਵਾਪਸ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *N) {
    ਵਾਪਸ N->get_ਕਿਸਮ() == IME_File;
  }
};

ਨਾਮਾਕਾਸ਼ {

ਵਰਗ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Hard_Link : ਜਨਤਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ {
  ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File &ResolvedFile;

ਜਨਤਕ:
  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Hard_Link(StringRef ਰਸਤਾ, ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File &ResolvedFile)
      : ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ(ਰਸਤਾ, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File &getResolvedFile() ਅਚਲ { ਵਾਪਸ ResolvedFile; }

  ਸਥਿਤੀ get_ਸਥਿਤੀ(ਅਚਲ Twine &RequestedName) ਅਚਲ override {
    ਵਾਪਸ ResolvedFile.get_ਸਥਿਤੀ(RequestedName);
  }

  std::string ਮੰਜ਼ਿਲ_String(ਬਿਨਾਨਿਸ਼ਾਨੀ Indent) ਅਚਲ override {
    ਵਾਪਸ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.ਮੰਜ਼ਿਲ_String(0);
  }

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *N) {
    ਵਾਪਸ N->get_ਕਿਸਮ() == IME_HardLink;
  }
};

ਵਰਗ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Symbolic_Link : ਜਨਤਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ {
  std::string ਨਿਸ਼ਾਨਾ_ਰਸਤਾ;
  ਸਥਿਤੀ Stat;

ਜਨਤਕ:
  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Symbolic_Link(StringRef ਰਸਤਾ, StringRef ਨਿਸ਼ਾਨਾ_ਰਸਤਾ, ਸਥਿਤੀ Stat)
      : ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ(ਰਸਤਾ, IME_SymbolicLink), ਨਿਸ਼ਾਨਾ_ਰਸਤਾ(std::move(ਨਿਸ਼ਾਨਾ_ਰਸਤਾ)),
        Stat(Stat) {}

  std::string ਮੰਜ਼ਿਲ_String(ਬਿਨਾਨਿਸ਼ਾਨੀ Indent) ਅਚਲ override {
    ਵਾਪਸ std::string(Indent, ' ') + "SymbolicLink to -> " + ਨਿਸ਼ਾਨਾ_ਰਸਤਾ;
  }

  ਸਥਿਤੀ get_ਸਥਿਤੀ(ਅਚਲ Twine &RequestedName) ਅਚਲ override {
    ਵਾਪਸ ਸਥਿਤੀ::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_ਨਿਸ਼ਾਨਾ_ਰਸਤਾ() ਅਚਲ { ਵਾਪਸ ਨਿਸ਼ਾਨਾ_ਰਸਤਾ; }

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *N) {
    ਵਾਪਸ N->get_ਕਿਸਮ() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
ਵਰਗ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_Adaptor : ਜਨਤਕ File {
  ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File &ਗੰਢ;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

ਜਨਤਕ:
  ਸਪਸ਼ਟ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_Adaptor(ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File &ਗੰਢ,
                               std::string RequestedName)
      : ਗੰਢ(ਗੰਢ), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<ਸਥਿਤੀ> ਸਥਿਤੀ_2() override {
    ਵਾਪਸ ਗੰਢ.get_ਸਥਿਤੀ(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ>>
  get_ਬਫਰ(ਅਚਲ Twine &Name, int64_t File_ਮਾਪ, ਤਾਰਕਿਕ RequiresNullTerminator,
            ਤਾਰਕਿਕ IsVolatile) override {
    llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ *Buf = ਗੰਢ.get_ਬਫਰ();
    ਵਾਪਸ llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ(
        Buf->get_ਬਫਰ(), Buf->get_ਬਫਰ_Identifier(), RequiresNullTerminator);
  }

  std::error_ਕੋਡ close() override { ਵਾਪਸ {}; }

  ਖਾਲੀ ਸੈੱਟ_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ) override { RequestedName = ਰਸਤਾ.str(); }
};
} // namespace

ਵਰਗ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory : ਜਨਤਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ {
  ਸਥਿਤੀ Stat;
  std::map<std::string, std::unique_ptr<ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ>, std::less<>> Entries;

ਜਨਤਕ:
  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory(ਸਥਿਤੀ Stat)
      : ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  ਸਥਿਤੀ get_ਸਥਿਤੀ(ਅਚਲ Twine &RequestedName) ਅਚਲ override {
    ਵਾਪਸ ਸਥਿਤੀ::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ਅਚਲ { ਵਾਪਸ Stat.getUniqueID(); }

  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *get_ਬੱਚਾ(StringRef Name) ਅਚਲ {
    ਸਵੈਚਾਲਿਤ I = Entries.ਲੱਭੋ(Name);
    ਜੇ (I != Entries.ਸਿਰਾ())
      ਵਾਪਸ I->ਦੂਜਾ_2.get();
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }

  ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *ਜੋੜੋ_ਬੱਚਾ(StringRef Name, std::unique_ptr<ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ> ਬੱਚਾ) {
    ਵਾਪਸ Entries.emplace(Name, std::move(ਬੱਚਾ)).ਪਹਿਲਾ_2->ਦੂਜਾ_2.get();
  }

  ਵਰਤੋਂ const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ਅਚਲ { ਵਾਪਸ Entries.begin(); }
  const_iterator ਸਿਰਾ() ਅਚਲ { ਵਾਪਸ Entries.ਸਿਰਾ(); }

  std::string ਮੰਜ਼ਿਲ_String(ਬਿਨਾਨਿਸ਼ਾਨੀ Indent) ਅਚਲ override {
    std::string ਨਤੀਜਾ =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਇੰਦਰਾਜ : Entries)
      ਨਤੀਜਾ += ਇੰਦਰਾਜ.ਦੂਜਾ_2->ਮੰਜ਼ਿਲ_String(Indent + 2);
    ਵਾਪਸ ਨਤੀਜਾ;
  }

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *N) {
    ਵਾਪਸ N->get_ਕਿਸਮ() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ਸਥਿਰ sys::fs::UniqueID getUniqueID(hash_ਕੋਡ Hash) {
  ਵਾਪਸ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ਸਥਿਰ sys::fs::UniqueID getFileID(sys::fs::UniqueID ਮਾਪਾ_2,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ਵਾਪਸ getUniqueID(llvm::hash_combine(ਮਾਪਾ_2.getFile(), Name, Contents));
}
ਸਥਿਰ sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID ਮਾਪਾ_2,
                                        llvm::StringRef Name) {
  ਵਾਪਸ getUniqueID(llvm::hash_combine(ਮਾਪਾ_2.getFile(), Name));
}

ਸਥਿਤੀ detail::New_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_Info::make_ਸਥਿਤੀ() ਅਚਲ {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, ਬਫਰ ? ਬਫਰ->get_ਬਫਰ() : "");

  ਵਾਪਸ ਸਥਿਤੀ(ਰਸਤਾ, UID, llvm::sys::ਮੰਜ਼ਿਲ_ਸਮਾਂ_ਬਿੰਦੂ(Modification_ਸਮਾਂ), User,
                ਸਮੂਹ, ਬਫਰ ? ਬਫਰ->get_ਬਫਰ_ਮਾਪ() : 0, Type, Perms);
}

ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System(ਤਾਰਕਿਕ UseNormalizedPaths)
    : ਜੜ੍ਹ_2(ਨਵਾਂ detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory(
          ਸਥਿਤੀ("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::ਸਮਾਂ_ਬਿੰਦੂ<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::~ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System() = ਮੂਲ;

std::string ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਮੰਜ਼ਿਲ_String() ਅਚਲ {
  ਵਾਪਸ ਜੜ੍ਹ_2->ਮੰਜ਼ਿਲ_String(/*Indent=*/0);
}

ਤਾਰਕਿਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਜੋੜੋ_File(ਅਚਲ Twine &P, time_t Modification_ਸਮਾਂ,
                                 std::unique_ptr<llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ> ਬਫਰ,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> ਸਮੂਹ,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_ਗੰਢ_Fn Make_ਗੰਢ) {
  SmallString<128> ਰਸਤਾ;
  P.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ਕੋਡ EC = makeAbsolute(ਰਸਤਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ(!EC);
  (ਖਾਲੀ)EC;

  ਜੇ (useNormalizedPaths())
    llvm::sys::ਰਸਤਾ_2::remove_dots(ਰਸਤਾ, /*remove_dot_dot=*/ਸੱਚ);

  ਜੇ (ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਝੂਠ;

  detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory *Dir = ਜੜ੍ਹ_2.get();
  ਸਵੈਚਾਲਿਤ I = llvm::sys::ਰਸਤਾ_2::begin(ਰਸਤਾ), E = sys::ਰਸਤਾ_2::ਸਿਰਾ(ਰਸਤਾ);
  ਅਚਲ ਸਵੈਚਾਲਿਤ ResolvedUser = User.ਮੁੱਲ_or(0);
  ਅਚਲ ਸਵੈਚਾਲਿਤ Resolved_ਸਮੂਹ = ਸਮੂਹ.ਮੁੱਲ_or(0);
  ਅਚਲ ਸਵੈਚਾਲਿਤ ResolvedType = Type.ਮੁੱਲ_or(sys::fs::file_type::regular_file);
  ਅਚਲ ਸਵੈਚਾਲਿਤ ResolvedPerms = Perms.ਮੁੱਲ_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ਅਚਲ ਸਵੈਚਾਲਿਤ NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  ਜਦੋਂ (ਸੱਚ) {
    Name = *I;
    ++I;
    ਜੇ (I == E)
      ਰੋਕੋ;
    detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *ਗੰਢ = Dir->get_ਬੱਚਾ(Name);
    ਜੇ (!ਗੰਢ) {
      // This isn't the last element, so we create a new directory.
      ਸਥਿਤੀ Stat(
          StringRef(ਰਸਤਾ.str().begin(), Name.ਸਿਰਾ() - ਰਸਤਾ.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::ਮੰਜ਼ਿਲ_ਸਮਾਂ_ਬਿੰਦੂ(Modification_ਸਮਾਂ), ResolvedUser, Resolved_ਸਮੂਹ,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(Dir->ਜੋੜੋ_ਬੱਚਾ(
          Name, std::make_unique<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(std::move(Stat))));
      ਜਾਰੀ;
    }
    // Creating file under another file.
    ਜੇ (!isa<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(ਗੰਢ))
      ਵਾਪਸ ਝੂਠ;
    Dir = cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(ਗੰਢ);
  }
  detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *ਗੰਢ = Dir->get_ਬੱਚਾ(Name);
  ਜੇ (!ਗੰਢ) {
    Dir->ਜੋੜੋ_ਬੱਚਾ(Name,
                  Make_ਗੰਢ({Dir->getUniqueID(), ਰਸਤਾ, Name, Modification_ਸਮਾਂ,
                            std::move(ਬਫਰ), ResolvedUser, Resolved_ਸਮੂਹ,
                            ResolvedType, ResolvedPerms}));
    ਵਾਪਸ ਸੱਚ;
  }
  ਜੇ (isa<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(ਗੰਢ))
    ਵਾਪਸ ResolvedType == sys::fs::file_type::directory_file;

  ਪ੍ਰਤਿਜ੍ਞਾ((isa<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File>(ਗੰਢ) ||
          isa<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Hard_Link>(ਗੰਢ)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  ਜੇ (ਸਵੈਚਾਲਿਤ *Link = dyn_cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Hard_Link>(ਗੰਢ)) {
    ਵਾਪਸ Link->getResolvedFile().get_ਬਫਰ()->get_ਬਫਰ() ==
           ਬਫਰ->get_ਬਫਰ();
  }
  ਵਾਪਸ cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File>(ਗੰਢ)->get_ਬਫਰ()->get_ਬਫਰ() ==
         ਬਫਰ->get_ਬਫਰ();
}

ਤਾਰਕਿਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਜੋੜੋ_File(ਅਚਲ Twine &P, time_t Modification_ਸਮਾਂ,
                                 std::unique_ptr<llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ> ਬਫਰ,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> ਸਮੂਹ,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ਵਾਪਸ ਜੋੜੋ_File(P, Modification_ਸਮਾਂ, std::move(ਬਫਰ), User, ਸਮੂਹ, Type,
                 Perms,
                 [](detail::New_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_Info NNI)
                     -> std::unique_ptr<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ> {
                   ਸਥਿਤੀ Stat = NNI.make_ਸਥਿਤੀ();
                   ਜੇ (Stat.getType() == sys::fs::file_type::directory_file)
                     ਵਾਪਸ std::make_unique<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(Stat);
                   ਵਾਪਸ std::make_unique<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File>(
                       Stat, std::move(NNI.ਬਫਰ));
                 });
}

ਤਾਰਕਿਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਜੋੜੋ_File_No_Own(
    ਅਚਲ Twine &P, time_t Modification_ਸਮਾਂ,
    ਅਚਲ llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref &ਬਫਰ, std::optional<uint32_t> User,
    std::optional<uint32_t> ਸਮੂਹ, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ਵਾਪਸ ਜੋੜੋ_File(P, Modification_ਸਮਾਂ, llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ(ਬਫਰ),
                 std::move(User), std::move(ਸਮੂਹ), std::move(Type),
                 std::move(Perms),
                 [](detail::New_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_Info NNI)
                     -> std::unique_ptr<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ> {
                   ਸਥਿਤੀ Stat = NNI.make_ਸਥਿਤੀ();
                   ਜੇ (Stat.getType() == sys::fs::file_type::directory_file)
                     ਵਾਪਸ std::make_unique<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(Stat);
                   ਵਾਪਸ std::make_unique<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File>(
                       Stat, std::move(NNI.ਬਫਰ));
                 });
}

detail::Named_ਗੰਢ_Or_Error
ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::lookup_ਗੰਢ(ਅਚਲ Twine &P, ਤਾਰਕਿਕ FollowFinalSymlink,
                               size_t Symlink_ਡੂੰਘਾਈ) ਅਚਲ {
  SmallString<128> ਰਸਤਾ;
  P.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ਕੋਡ EC = makeAbsolute(ਰਸਤਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ(!EC);
  (ਖਾਲੀ)EC;

  ਜੇ (useNormalizedPaths())
    llvm::sys::ਰਸਤਾ_2::remove_dots(ਰਸਤਾ, /*remove_dot_dot=*/ਸੱਚ);

  ਅਚਲ detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory *Dir = ਜੜ੍ਹ_2.get();
  ਜੇ (ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ detail::Named_ਗੰਢ_Or_Error(ਰਸਤਾ, Dir);

  ਸਵੈਚਾਲਿਤ I = llvm::sys::ਰਸਤਾ_2::begin(ਰਸਤਾ), E = llvm::sys::ਰਸਤਾ_2::ਸਿਰਾ(ਰਸਤਾ);
  ਜਦੋਂ (ਸੱਚ) {
    detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ *ਗੰਢ = Dir->get_ਬੱਚਾ(*I);
    ++I;
    ਜੇ (!ਗੰਢ)
      ਵਾਪਸ errc::no_such_file_or_directory;

    ਜੇ (ਸਵੈਚਾਲਿਤ Symlink = dyn_cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Symbolic_Link>(ਗੰਢ)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      ਜੇ (I == E && !FollowFinalSymlink)
        ਵਾਪਸ detail::Named_ਗੰਢ_Or_Error(ਰਸਤਾ, Symlink);

      ਜੇ (Symlink_ਡੂੰਘਾਈ > ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::Max_Symlink_ਡੂੰਘਾਈ)
        ਵਾਪਸ errc::no_such_file_or_directory;

      SmallString<128> ਨਿਸ਼ਾਨਾ_ਰਸਤਾ = Symlink->get_ਨਿਸ਼ਾਨਾ_ਰਸਤਾ();
      ਜੇ (std::error_ਕੋਡ EC = makeAbsolute(ਨਿਸ਼ਾਨਾ_ਰਸਤਾ))
        ਵਾਪਸ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      ਸਵੈਚਾਲਿਤ ਨਿਸ਼ਾਨਾ_2 =
          lookup_ਗੰਢ(ਨਿਸ਼ਾਨਾ_ਰਸਤਾ, /*FollowFinalSymlink=*/ਸੱਚ, Symlink_ਡੂੰਘਾਈ + 1);
      ਜੇ (!ਨਿਸ਼ਾਨਾ_2 || I == E)
        ਵਾਪਸ ਨਿਸ਼ਾਨਾ_2;

      ਜੇ (!isa<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(*ਨਿਸ਼ਾਨਾ_2))
        ਵਾਪਸ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(*ਨਿਸ਼ਾਨਾ_2);
      ਜਾਰੀ;
    }

    // Return the file if it's at the end of the path.
    ਜੇ (ਸਵੈਚਾਲਿਤ File = dyn_cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File>(ਗੰਢ)) {
      ਜੇ (I == E)
        ਵਾਪਸ detail::Named_ਗੰਢ_Or_Error(ਰਸਤਾ, File);
      ਵਾਪਸ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    ਜੇ (ਸਵੈਚਾਲਿਤ File = dyn_cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Hard_Link>(ਗੰਢ)) {
      ਜੇ (I == E)
        ਵਾਪਸ detail::Named_ਗੰਢ_Or_Error(ਰਸਤਾ, &File->getResolvedFile());
      ਵਾਪਸ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(ਗੰਢ);
    ਜੇ (I == E)
      ਵਾਪਸ detail::Named_ਗੰਢ_Or_Error(ਰਸਤਾ, Dir);
  }
}

ਤਾਰਕਿਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਜੋੜੋ_Hard_Link(ਅਚਲ Twine &NewLink,
                                     ਅਚਲ Twine &ਨਿਸ਼ਾਨਾ_2) {
  ਸਵੈਚਾਲਿਤ New_Link_ਗੰਢ = lookup_ਗੰਢ(NewLink, /*FollowFinalSymlink=*/ਝੂਠ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  ਸਵੈਚਾਲਿਤ ਨਿਸ਼ਾਨਾ_ਗੰਢ = lookup_ਗੰਢ(ਨਿਸ਼ਾਨਾ_2, /*FollowFinalSymlink=*/ਸੱਚ);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  ਜੇ (!ਨਿਸ਼ਾਨਾ_ਗੰਢ || New_Link_ਗੰਢ || !isa<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File>(*ਨਿਸ਼ਾਨਾ_ਗੰਢ))
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ ਜੋੜੋ_File(NewLink, 0, ਖਾਲੀ_ਸੂਚਕ, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_Info NNI) {
                   ਵਾਪਸ std::make_unique<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Hard_Link>(
                       NNI.ਰਸਤਾ.str(),
                       *cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File>(*ਨਿਸ਼ਾਨਾ_ਗੰਢ));
                 });
}

ਤਾਰਕਿਕ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਜੋੜੋ_Symbolic_Link(
    ਅਚਲ Twine &NewLink, ਅਚਲ Twine &ਨਿਸ਼ਾਨਾ_2, time_t Modification_ਸਮਾਂ,
    std::optional<uint32_t> User, std::optional<uint32_t> ਸਮੂਹ,
    std::optional<llvm::sys::fs::perms> Perms) {
  ਸਵੈਚਾਲਿਤ New_Link_ਗੰਢ = lookup_ਗੰਢ(NewLink, /*FollowFinalSymlink=*/ਝੂਠ);
  ਜੇ (New_Link_ਗੰਢ)
    ਵਾਪਸ ਝੂਠ;

  SmallString<128> NewLinkStr, ਨਿਸ਼ਾਨਾ_Str;
  NewLink.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(NewLinkStr);
  ਨਿਸ਼ਾਨਾ_2.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਨਿਸ਼ਾਨਾ_Str);

  ਵਾਪਸ ਜੋੜੋ_File(NewLinkStr, Modification_ਸਮਾਂ, ਖਾਲੀ_ਸੂਚਕ, User, ਸਮੂਹ,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_ਗੰਢ_Info NNI) {
                   ਵਾਪਸ std::make_unique<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Symbolic_Link>(
                       NewLinkStr, ਨਿਸ਼ਾਨਾ_Str, NNI.make_ਸਥਿਤੀ());
                 });
}

llvm::ErrorOr<ਸਥਿਤੀ> ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਸਥਿਤੀ_2(ਅਚਲ Twine &ਰਸਤਾ) {
  ਸਵੈਚਾਲਿਤ ਗੰਢ = lookup_ਗੰਢ(ਰਸਤਾ, /*FollowFinalSymlink=*/ਸੱਚ);
  ਜੇ (ਗੰਢ)
    ਵਾਪਸ (*ਗੰਢ)->get_ਸਥਿਤੀ(ਰਸਤਾ);
  ਵਾਪਸ ਗੰਢ.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::openFileForRead(ਅਚਲ Twine &ਰਸਤਾ) {
  ਸਵੈਚਾਲਿਤ ਗੰਢ = lookup_ਗੰਢ(ਰਸਤਾ,/*FollowFinalSymlink=*/ਸੱਚ);
  ਜੇ (!ਗੰਢ)
    ਵਾਪਸ ਗੰਢ.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  ਜੇ (ਸਵੈਚਾਲਿਤ *F = dyn_cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File>(*ਗੰਢ))
    ਵਾਪਸ std::unique_ptr<File>(
        ਨਵਾਂ detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_Adaptor(*F, ਰਸਤਾ.str()));

  // FIXME: errc::not_a_file?
  ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::invalid_ਦਲੀਲ);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
ਵਰਗ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::DirIterator : ਜਨਤਕ llvm::vfs::detail::DirIterImpl {
  ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System *FS;
  detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory::const_iterator I;
  detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory::const_iterator E;
  std::string RequestedDirName;

  ਖਾਲੀ ਸੈੱਟ_Current_ਇੰਦਰਾਜ() {
    ਜੇ (I != E) {
      SmallString<256> ਰਸਤਾ(RequestedDirName);
      llvm::sys::ਰਸਤਾ_2::append(ਰਸਤਾ, I->ਦੂਜਾ_2->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ਚੋਣ (I->ਦੂਜਾ_2->get_ਕਿਸਮ()) {
      ਹਾਲਤ detail::IME_File:
      ਹਾਲਤ detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        ਰੋਕੋ;
      ਹਾਲਤ detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        ਰੋਕੋ;
      ਹਾਲਤ detail::IME_SymbolicLink:
        ਜੇ (ਸਵੈਚਾਲਿਤ Symlink_ਨਿਸ਼ਾਨਾ =
                FS->lookup_ਗੰਢ(ਰਸਤਾ, /*FollowFinalSymlink=*/ਸੱਚ)) {
          ਰਸਤਾ = Symlink_ਨਿਸ਼ਾਨਾ.getName();
          Type = (*Symlink_ਨਿਸ਼ਾਨਾ)->get_ਸਥਿਤੀ(ਰਸਤਾ).getType();
        }
        ਰੋਕੋ;
      }
      Current_ਇੰਦਰਾਜ = directory_ਇੰਦਰਾਜ(std::string(ਰਸਤਾ), Type);
    } ਨਹੀਂਤਾਂ {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_ਇੰਦਰਾਜ = directory_ਇੰਦਰਾਜ();
    }
  }

ਜਨਤਕ:
  DirIterator() = ਮੂਲ;

  DirIterator(ਅਚਲ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System *FS,
              ਅਚਲ detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.ਸਿਰਾ()),
        RequestedDirName(std::move(RequestedDirName)) {
    ਸੈੱਟ_Current_ਇੰਦਰਾਜ();
  }

  std::error_ਕੋਡ increment() override {
    ++I;
    ਸੈੱਟ_Current_ਇੰਦਰਾਜ();
    ਵਾਪਸ {};
  }
};

directory_iterator ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::dir_begin(ਅਚਲ Twine &Dir,
                                                 std::error_ਕੋਡ &EC) {
  ਸਵੈਚਾਲਿਤ ਗੰਢ = lookup_ਗੰਢ(Dir, /*FollowFinalSymlink=*/ਸੱਚ);
  ਜੇ (!ਗੰਢ) {
    EC = ਗੰਢ.getError();
    ਵਾਪਸ directory_iterator(std::make_shared<DirIterator>());
  }

  ਜੇ (ਸਵੈਚਾਲਿਤ *Dir_ਗੰਢ = dyn_cast<detail::ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_Directory>(*ਗੰਢ))
    ਵਾਪਸ directory_iterator(
        std::make_shared<DirIterator>(ਇਹ, *Dir_ਗੰਢ, Dir.str()));

  EC = make_error_ਕੋਡ(llvm::errc::not_a_directory);
  ਵਾਪਸ directory_iterator(std::make_shared<DirIterator>());
}

std::error_ਕੋਡ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ਸੈੱਟ_Current_Working_Directory(ਅਚਲ Twine &P) {
  SmallString<128> ਰਸਤਾ;
  P.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ਕੋਡ EC = makeAbsolute(ਰਸਤਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ(!EC);
  (ਖਾਲੀ)EC;

  ਜੇ (useNormalizedPaths())
    llvm::sys::ਰਸਤਾ_2::remove_dots(ਰਸਤਾ, /*remove_dot_dot=*/ਸੱਚ);

  ਜੇ (!ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ())
    WorkingDirectory = std::string(ਰਸਤਾ);
  ਵਾਪਸ {};
}

std::error_ਕੋਡ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::get_Real_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ,
                                                Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Output) {
  ਸਵੈਚਾਲਿਤ CWD = getCurrentWorkingDirectory();
  ਜੇ (!CWD || CWD->ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ errc::operation_not_permitted;
  ਰਸਤਾ.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(Output);
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = makeAbsolute(Output))
    ਵਾਪਸ EC;
  llvm::sys::ਰਸਤਾ_2::remove_dots(Output, /*remove_dot_dot=*/ਸੱਚ);
  ਵਾਪਸ {};
}

std::error_ਕੋਡ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::isLocal(ਅਚਲ Twine &ਰਸਤਾ, ਤਾਰਕਿਕ &ਨਤੀਜਾ) {
  ਨਤੀਜਾ = ਝੂਠ;
  ਵਾਪਸ {};
}

ਖਾਲੀ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   ਬਿਨਾਨਿਸ਼ਾਨੀ IndentLevel) ਅਚਲ {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

ਨਾਮਾਕਾਸ਼ {

ਸਥਿਰ llvm::sys::ਰਸਤਾ_2::Style getExistingStyle(llvm::StringRef ਰਸਤਾ) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::ਰਸਤਾ_2::Style style = llvm::sys::ਰਸਤਾ_2::Style::native;
  ਅਚਲ size_t n = ਰਸਤਾ.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ("/\\");
  // Can't distinguish between posix and windows_slash here.
  ਜੇ (n != static_cast<size_t>(-1))
    style = (ਰਸਤਾ[n] == '/') ? llvm::sys::ਰਸਤਾ_2::Style::posix
                             : llvm::sys::ਰਸਤਾ_2::Style::windows_backslash;
  ਵਾਪਸ style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ਸਥਿਰ llvm::SmallString<256> canonicalize(llvm::StringRef ਰਸਤਾ) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::ਰਸਤਾ_2::Style style = getExistingStyle(ਰਸਤਾ);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> ਨਤੀਜਾ_2 =
      llvm::sys::ਰਸਤਾ_2::remove_leading_dotslash(ਰਸਤਾ, style);
  llvm::sys::ਰਸਤਾ_2::remove_dots(ਨਤੀਜਾ_2, /*remove_dot_dot=*/ਸੱਚ, style);
  ਵਾਪਸ ਨਤੀਜਾ_2;
}

/// Whether the error and entry specify a file/directory that was not found.
ਸਥਿਰ ਤਾਰਕਿਕ is_File_Not_ਮਿਲਿਆ(std::error_ਕੋਡ EC,
                           RedirectingFileSystem::ਇੰਦਰਾਜ *E = ਖਾਲੀ_ਸੂਚਕ) {
  ਜੇ (E && !isa<RedirectingFileSystem::Directory_Remap_ਇੰਦਰਾਜ>(E))
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ExternalFS && "RedirectingFileSystem requires an external FS");
  ਜੇ (ਸਵੈਚਾਲਿਤ ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
ਵਰਗ llvm::vfs::RedirectingFSDirIterImpl
    : ਜਨਤਕ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::Directory_ਇੰਦਰਾਜ::iterator Current, ਸਿਰਾ_2;

  std::error_ਕੋਡ incrementImpl(ਤਾਰਕਿਕ Is_ਪਹਿਲਾ_ਸਮਾਂ) {
    ਪ੍ਰਤਿਜ੍ਞਾ((Is_ਪਹਿਲਾ_ਸਮਾਂ || Current != ਸਿਰਾ_2) && "cannot iterate past end");
    ਜੇ (!Is_ਪਹਿਲਾ_ਸਮਾਂ)
      ++Current;
    ਜੇ (Current != ਸਿਰਾ_2) {
      SmallString<128> ਰਸਤਾ_Str(Dir);
      llvm::sys::ਰਸਤਾ_2::append(ਰਸਤਾ_Str, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ਚੋਣ ((*Current)->get_ਕਿਸਮ()) {
      ਹਾਲਤ RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      ਹਾਲਤ RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        ਰੋਕੋ;
      ਹਾਲਤ RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        ਰੋਕੋ;
      }
      Current_ਇੰਦਰਾਜ = directory_ਇੰਦਰਾਜ(std::string(ਰਸਤਾ_Str), Type);
    } ਨਹੀਂਤਾਂ {
      Current_ਇੰਦਰਾਜ = directory_ਇੰਦਰਾਜ();
    }
    ਵਾਪਸ {};
  };

ਜਨਤਕ:
  RedirectingFSDirIterImpl(
      ਅਚਲ Twine &ਰਸਤਾ, RedirectingFileSystem::Directory_ਇੰਦਰਾਜ::iterator Begin,
      RedirectingFileSystem::Directory_ਇੰਦਰਾਜ::iterator ਸਿਰਾ_2, std::error_ਕੋਡ &EC)
      : Dir(ਰਸਤਾ.str()), Current(Begin), ਸਿਰਾ_2(ਸਿਰਾ_2) {
    EC = incrementImpl(/*IsFirstTime=*/ਸੱਚ);
  }

  std::error_ਕੋਡ increment() override {
    ਵਾਪਸ incrementImpl(/*IsFirstTime=*/ਝੂਠ);
  }
};

ਨਾਮਾਕਾਸ਼ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
ਵਰਗ RedirectingFSDirRemapIterImpl : ਜਨਤਕ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::ਰਸਤਾ_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

ਜਨਤਕ:
  RedirectingFSDirRemapIterImpl(std::string Dir_ਰਸਤਾ,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_ਰਸਤਾ)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    ਜੇ (ExternalIter != llvm::vfs::directory_iterator())
      ਸੈੱਟ_Current_ਇੰਦਰਾਜ();
  }

  ਖਾਲੀ ਸੈੱਟ_Current_ਇੰਦਰਾਜ() {
    StringRef External_ਰਸਤਾ = ExternalIter->ਰਸਤਾ_2();
    llvm::sys::ਰਸਤਾ_2::Style ExternalStyle = getExistingStyle(External_ਰਸਤਾ);
    StringRef File = llvm::sys::ਰਸਤਾ_2::filename(External_ਰਸਤਾ, ExternalStyle);

    SmallString<128> New_ਰਸਤਾ(Dir);
    llvm::sys::ਰਸਤਾ_2::append(New_ਰਸਤਾ, DirStyle, File);

    Current_ਇੰਦਰਾਜ = directory_ਇੰਦਰਾਜ(std::string(New_ਰਸਤਾ), ExternalIter->type());
  }

  std::error_ਕੋਡ increment() override {
    std::error_ਕੋਡ EC;
    ExternalIter.increment(EC);
    ਜੇ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      ਸੈੱਟ_Current_ਇੰਦਰਾਜ();
    ਨਹੀਂਤਾਂ
      Current_ਇੰਦਰਾਜ = directory_ਇੰਦਰਾਜ();
    ਵਾਪਸ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ਅਚਲ {
  ਵਾਪਸ WorkingDirectory;
}

std::error_ਕੋਡ
RedirectingFileSystem::ਸੈੱਟ_Current_Working_Directory(ਅਚਲ Twine &ਰਸਤਾ) {
  // Don't change the working directory if the path doesn't exist.
  ਜੇ (!exists(ਰਸਤਾ))
    ਵਾਪਸ errc::no_such_file_or_directory;

  SmallString<128> Absolute_ਰਸਤਾ;
  ਰਸਤਾ.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(Absolute_ਰਸਤਾ);
  ਜੇ (std::error_ਕੋਡ EC = makeAbsolute(Absolute_ਰਸਤਾ))
    ਵਾਪਸ EC;
  WorkingDirectory = std::string(Absolute_ਰਸਤਾ);
  ਵਾਪਸ {};
}

std::error_ਕੋਡ RedirectingFileSystem::isLocal(ਅਚਲ Twine &ਰਸਤਾ_3,
                                               ਤਾਰਕਿਕ &ਨਤੀਜਾ) {
  SmallString<256> ਰਸਤਾ;
  ਰਸਤਾ_3.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  ਜੇ (makeAbsolute(ਰਸਤਾ))
    ਵਾਪਸ {};

  ਵਾਪਸ ExternalFS->isLocal(ਰਸਤਾ, ਨਤੀਜਾ);
}

std::error_ਕੋਡ RedirectingFileSystem::makeAbsolute(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ) ਅਚਲ {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  ਜੇ (llvm::sys::ਰਸਤਾ_2::is_absolute(ਰਸਤਾ, llvm::sys::ਰਸਤਾ_2::Style::posix) ||
      llvm::sys::ਰਸਤਾ_2::is_absolute(ਰਸਤਾ,
                                   llvm::sys::ਰਸਤਾ_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ਵਾਪਸ {};

  ਸਵੈਚਾਲਿਤ WorkingDir = getCurrentWorkingDirectory();
  ਜੇ (!WorkingDir)
    ਵਾਪਸ WorkingDir.getError();

  ਵਾਪਸ makeAbsolute(WorkingDir.get(), ਰਸਤਾ);
}

std::error_ਕੋਡ
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ) ਅਚਲ {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  ਜੇ (!WorkingDir.ਖਾਲੀ_ਥਾਵਾਂ() &&
      !sys::ਰਸਤਾ_2::is_absolute(WorkingDir, sys::ਰਸਤਾ_2::Style::posix) &&
      !sys::ਰਸਤਾ_2::is_absolute(WorkingDir,
                              sys::ਰਸਤਾ_2::Style::windows_backslash)) {
    ਵਾਪਸ std::error_ਕੋਡ();
  }
  sys::ਰਸਤਾ_2::Style style = sys::ਰਸਤਾ_2::Style::windows_backslash;
  ਜੇ (sys::ਰਸਤਾ_2::is_absolute(WorkingDir, sys::ਰਸਤਾ_2::Style::posix)) {
    style = sys::ਰਸਤਾ_2::Style::posix;
  } ਨਹੀਂਤਾਂ {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    ਜੇ (getExistingStyle(WorkingDir) != sys::ਰਸਤਾ_2::Style::windows_backslash)
      style = sys::ਰਸਤਾ_2::Style::windows_slash;
  }

  std::string ਨਤੀਜਾ = std::string(WorkingDir);
  StringRef Dir(ਨਤੀਜਾ);
  ਜੇ (!Dir.ਸਿਰੇ_with(sys::ਰਸਤਾ_2::get_separator(style))) {
    ਨਤੀਜਾ += sys::ਰਸਤਾ_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  ਨਤੀਜਾ.append(ਰਸਤਾ.ਅੰਕੜੇ(), ਰਸਤਾ.ਮਾਪ_2());
  ਰਸਤਾ.ਸੌਂਪੋ(ਨਤੀਜਾ.begin(), ਨਤੀਜਾ.ਸਿਰਾ());

  ਵਾਪਸ {};
}

directory_iterator RedirectingFileSystem::dir_begin(ਅਚਲ Twine &Dir,
                                                    std::error_ਕੋਡ &EC) {
  SmallString<256> ਰਸਤਾ;
  Dir.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  EC = makeAbsolute(ਰਸਤਾ);
  ਜੇ (EC)
    ਵਾਪਸ {};

  ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਨਤੀਜਾ = lookup_ਰਸਤਾ(ਰਸਤਾ);
  ਜੇ (!ਨਤੀਜਾ) {
    ਜੇ (Redirection != Redirect_ਕਿਸਮ::RedirectOnly &&
        is_File_Not_ਮਿਲਿਆ(ਨਤੀਜਾ.getError()))
      ਵਾਪਸ ExternalFS->dir_begin(ਰਸਤਾ, EC);

    EC = ਨਤੀਜਾ.getError();
    ਵਾਪਸ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<ਸਥਿਤੀ> S = ਸਥਿਤੀ_2(ਰਸਤਾ, Dir, *ਨਤੀਜਾ);
  ਜੇ (!S) {
    ਜੇ (Redirection != Redirect_ਕਿਸਮ::RedirectOnly &&
        is_File_Not_ਮਿਲਿਆ(S.getError(), ਨਤੀਜਾ->E))
      ਵਾਪਸ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ਵਾਪਸ {};
  }

  ਜੇ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ਵਾਪਸ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_ਕੋਡ RedirectEC;
  ਜੇ (ਸਵੈਚਾਲਿਤ ExtRedirect = ਨਤੀਜਾ->getExternalRedirect()) {
    ਸਵੈਚਾਲਿਤ RE = cast<RedirectingFileSystem::Remap_ਇੰਦਰਾਜ>(ਨਤੀਜਾ->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    ਜੇ (!RE->useExternalName(Use_External_ਨਾਂ)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(ਰਸਤਾ), RedirectIter));
    }
  } ਨਹੀਂਤਾਂ {
    ਸਵੈਚਾਲਿਤ DE = cast<Directory_ਇੰਦਰਾਜ>(ਨਤੀਜਾ->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            ਰਸਤਾ, DE->contents_begin(), DE->contents_ਸਿਰਾ(), RedirectEC));
  }

  ਜੇ (RedirectEC) {
    ਜੇ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ਵਾਪਸ {};
    }
    RedirectIter = {};
  }

  ਜੇ (Redirection == Redirect_ਕਿਸਮ::RedirectOnly) {
    EC = RedirectEC;
    ਵਾਪਸ RedirectIter;
  }

  std::error_ਕੋਡ ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(ਰਸਤਾ, ExternalEC);
  ਜੇ (ExternalEC) {
    ਜੇ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ਵਾਪਸ {};
    }
    ExternalIter = {};
  }

  Small_ਸਦਿਸ਼<directory_iterator, 2> Iters;
  ਚੋਣ (Redirection) {
  ਹਾਲਤ Redirect_ਕਿਸਮ::Fallthrough:
    Iters.ਧੱਕੋ_ਪਿੱਠ(ExternalIter);
    Iters.ਧੱਕੋ_ਪਿੱਠ(RedirectIter);
    ਰੋਕੋ;
  ਹਾਲਤ Redirect_ਕਿਸਮ::Fallback:
    Iters.ਧੱਕੋ_ਪਿੱਠ(RedirectIter);
    Iters.ਧੱਕੋ_ਪਿੱਠ(ExternalIter);
    ਰੋਕੋ;
  ਮੂਲ:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  ਜੇ (EC)
    ਵਾਪਸ {};
  ਵਾਪਸ Combined;
}

ਖਾਲੀ RedirectingFileSystem::ਸੈੱਟ_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ਅਚਲ {
  ਵਾਪਸ OverlayFileDir;
}

ਖਾਲੀ RedirectingFileSystem::ਸੈੱਟ_Fallthrough(ਤਾਰਕਿਕ Fallthrough) {
  ਜੇ (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_ਕਿਸਮ::Fallthrough;
  } ਨਹੀਂਤਾਂ {
    Redirection = RedirectingFileSystem::Redirect_ਕਿਸਮ::RedirectOnly;
  }
}

ਖਾਲੀ RedirectingFileSystem::ਸੈੱਟ_Redirection(
    RedirectingFileSystem::Redirect_ਕਿਸਮ ਕਿਸਮ_3) {
  Redirection = ਕਿਸਮ_3;
}

std::ਸਦਿਸ਼<StringRef> RedirectingFileSystem::getRoots() ਅਚਲ {
  std::ਸਦਿਸ਼<StringRef> R;
  R.reserve(Roots.ਮਾਪ_2());
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਜੜ੍ਹ_2 : Roots)
    R.ਧੱਕੋ_ਪਿੱਠ(ਜੜ੍ਹ_2->getName());
  ਵਾਪਸ R;
}

ਖਾਲੀ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ IndentLevel) ਅਚਲ {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_ਨਾਂ ? "true" : "false") << ")\n";
  ਜੇ (Type == PrintType::Summary)
    ਵਾਪਸ;

  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਜੜ੍ਹ_2 : Roots)
    print_ਇੰਦਰਾਜ(OS, ਜੜ੍ਹ_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

ਖਾਲੀ RedirectingFileSystem::print_ਇੰਦਰਾਜ(raw_ostream &OS,
                                       RedirectingFileSystem::ਇੰਦਰਾਜ *E,
                                       ਬਿਨਾਨਿਸ਼ਾਨੀ IndentLevel) ਅਚਲ {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  ਚੋਣ (E->get_ਕਿਸਮ()) {
  ਹਾਲਤ EK_Directory: {
    ਸਵੈਚਾਲਿਤ *DE = cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(E);

    OS << "\n";
    ਲਈ (std::unique_ptr<ਇੰਦਰਾਜ> &Sub_ਇੰਦਰਾਜ :
         llvm::make_range(DE->contents_begin(), DE->contents_ਸਿਰਾ()))
      print_ਇੰਦਰਾਜ(OS, Sub_ਇੰਦਰਾਜ.get(), IndentLevel + 1);
    ਰੋਕੋ;
  }
  ਹਾਲਤ EK_DirectoryRemap:
  ਹਾਲਤ EK_File: {
    ਸਵੈਚਾਲਿਤ *RE = cast<RedirectingFileSystem::Remap_ਇੰਦਰਾਜ>(E);
    OS << " -> '" << RE->get_External_Contents_ਰਸਤਾ() << "'";
    ਚੋਣ (RE->getUseName()) {
    ਹਾਲਤ NK_Not_ਸੈੱਟ:
      ਰੋਕੋ;
    ਹਾਲਤ NK_External:
      OS << " (UseExternalName: true)";
      ਰੋਕੋ;
    ਹਾਲਤ NK_Virtual:
      OS << " (UseExternalName: false)";
      ਰੋਕੋ;
    }
    OS << "\n";
    ਰੋਕੋ;
  }
  }
}

ਖਾਲੀ RedirectingFileSystem::ਦੇਖੋ_ਬੱਚਾ_File_Systems(ਦੇਖੋ_Callback_Ty Callback) {
  ਜੇ (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->ਦੇਖੋ_ਬੱਚਾ_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
ਵਰਗ llvm::vfs::RedirectingFileSystemParser {
  yaml::ਧਾਰਾ &ਧਾਰਾ;

  ਖਾਲੀ error(yaml::ਗੰਢ *N, ਅਚਲ Twine &ਸੁਨੇਹਾ_4) { ਧਾਰਾ.printError(N, ਸੁਨੇਹਾ_4); }

  // false on error
  ਤਾਰਕਿਕ parseScalarString(yaml::ਗੰਢ *N, StringRef &ਨਤੀਜਾ,
                         Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage) {
    ਅਚਲ ਸਵੈਚਾਲਿਤ *S = dyn_cast_if_present<yaml::Scalar_ਗੰਢ>(N);

    ਜੇ (!S) {
      error(N, "expected string");
      ਵਾਪਸ ਝੂਠ;
    }
    ਨਤੀਜਾ = S->get_ਮੁੱਲ(Storage);
    ਵਾਪਸ ਸੱਚ;
  }

  // false on error
  ਤਾਰਕਿਕ parseScalarBool(yaml::ਗੰਢ *N, ਤਾਰਕਿਕ &ਨਤੀਜਾ) {
    SmallString<5> Storage;
    StringRef ਮੁੱਲ;
    ਜੇ (!parseScalarString(N, ਮੁੱਲ, Storage))
      ਵਾਪਸ ਝੂਠ;

    ਜੇ (ਮੁੱਲ.equals_insensitive("true") || ਮੁੱਲ.equals_insensitive("on") ||
        ਮੁੱਲ.equals_insensitive("yes") || ਮੁੱਲ == "1") {
      ਨਤੀਜਾ = ਸੱਚ;
      ਵਾਪਸ ਸੱਚ;
    } ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ.equals_insensitive("false") ||
               ਮੁੱਲ.equals_insensitive("off") ||
               ਮੁੱਲ.equals_insensitive("no") || ਮੁੱਲ == "0") {
      ਨਤੀਜਾ = ਝੂਠ;
      ਵਾਪਸ ਸੱਚ;
    }

    error(N, "expected boolean value");
    ਵਾਪਸ ਝੂਠ;
  }

  std::optional<RedirectingFileSystem::Redirect_ਕਿਸਮ>
  parse_Redirect_ਕਿਸਮ(yaml::ਗੰਢ *N) {
    SmallString<12> Storage;
    StringRef ਮੁੱਲ;
    ਜੇ (!parseScalarString(N, ਮੁੱਲ, Storage))
      ਵਾਪਸ std::nullopt;

    ਜੇ (ਮੁੱਲ.equals_insensitive("fallthrough")) {
      ਵਾਪਸ RedirectingFileSystem::Redirect_ਕਿਸਮ::Fallthrough;
    } ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ.equals_insensitive("fallback")) {
      ਵਾਪਸ RedirectingFileSystem::Redirect_ਕਿਸਮ::Fallback;
    } ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ.equals_insensitive("redirect-only")) {
      ਵਾਪਸ RedirectingFileSystem::Redirect_ਕਿਸਮ::RedirectOnly;
    }
    ਵਾਪਸ std::nullopt;
  }

  std::optional<RedirectingFileSystem::ਜੜ੍ਹ_Relative_ਕਿਸਮ>
  parse_ਜੜ੍ਹ_Relative_ਕਿਸਮ(yaml::ਗੰਢ *N) {
    SmallString<12> Storage;
    StringRef ਮੁੱਲ;
    ਜੇ (!parseScalarString(N, ਮੁੱਲ, Storage))
      ਵਾਪਸ std::nullopt;
    ਜੇ (ਮੁੱਲ.equals_insensitive("cwd")) {
      ਵਾਪਸ RedirectingFileSystem::ਜੜ੍ਹ_Relative_ਕਿਸਮ::CWD;
    } ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ.equals_insensitive("overlay-dir")) {
      ਵਾਪਸ RedirectingFileSystem::ਜੜ੍ਹ_Relative_ਕਿਸਮ::OverlayDir;
    }
    ਵਾਪਸ std::nullopt;
  }

  ਬਣਤਰ ਕੁੰਜੀ_ਸਥਿਤੀ {
    ਤਾਰਕਿਕ Required;
    ਤਾਰਕਿਕ ਵੇਖਿਆ = ਝੂਠ;

    ਕੁੰਜੀ_ਸਥਿਤੀ(ਤਾਰਕਿਕ Required = ਝੂਠ) : Required(Required) {}
  };

  ਵਰਤੋਂ ਕੁੰਜੀ_ਸਥਿਤੀ_Pair = std::pair<StringRef, ਕੁੰਜੀ_ਸਥਿਤੀ>;

  // false on error
  ਤਾਰਕਿਕ check_Duplicate_Or_Unknown_ਕੁੰਜੀ(yaml::ਗੰਢ *ਕੁੰਜੀ_ਗੰਢ, StringRef ਕੁੰਜੀ,
                                  DenseMap<StringRef, ਕੁੰਜੀ_ਸਥਿਤੀ> &ਕੁੰਜੀਆਂ_2) {
    ਸਵੈਚਾਲਿਤ It = ਕੁੰਜੀਆਂ_2.ਲੱਭੋ(ਕੁੰਜੀ);
    ਜੇ (It == ਕੁੰਜੀਆਂ_2.ਸਿਰਾ()) {
      error(ਕੁੰਜੀ_ਗੰਢ, "unknown key");
      ਵਾਪਸ ਝੂਠ;
    }
    ਕੁੰਜੀ_ਸਥਿਤੀ &S = It->ਦੂਜਾ_2;
    ਜੇ (S.ਵੇਖਿਆ) {
      error(ਕੁੰਜੀ_ਗੰਢ, Twine("duplicate key '") + ਕੁੰਜੀ + "'");
      ਵਾਪਸ ਝੂਠ;
    }
    S.ਵੇਖਿਆ = ਸੱਚ;
    ਵਾਪਸ ਸੱਚ;
  }

  // false on error
  ਤਾਰਕਿਕ check_Missing_ਕੁੰਜੀਆਂ(yaml::ਗੰਢ *Obj, DenseMap<StringRef, ਕੁੰਜੀ_ਸਥਿਤੀ> &ਕੁੰਜੀਆਂ_2) {
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &I : ਕੁੰਜੀਆਂ_2) {
      ਜੇ (I.ਦੂਜਾ_2.Required && !I.ਦੂਜਾ_2.ਵੇਖਿਆ) {
        error(Obj, Twine("missing key '") + I.ਪਹਿਲਾ_2 + "'");
        ਵਾਪਸ ਝੂਠ;
      }
    }
    ਵਾਪਸ ਸੱਚ;
  }

ਜਨਤਕ:
  ਸਥਿਰ RedirectingFileSystem::ਇੰਦਰਾਜ *
  lookup_Or_Create_ਇੰਦਰਾਜ(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::ਇੰਦਰਾਜ *ਮਾਪਾ_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ) {
    ਜੇ (!ਮਾਪਾ_ਇੰਦਰਾਜ) { // Look for a existent root
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਜੜ੍ਹ_2 : FS->Roots) {
        ਜੇ (Name == ਜੜ੍ਹ_2->getName()) {
          ਮਾਪਾ_ਇੰਦਰਾਜ = ਜੜ੍ਹ_2.get();
          ਵਾਪਸ ਮਾਪਾ_ਇੰਦਰਾਜ;
        }
      }
    } ਨਹੀਂਤਾਂ { // Advance to the next component
      ਸਵੈਚਾਲਿਤ *DE = dyn_cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(ਮਾਪਾ_ਇੰਦਰਾਜ);
      ਲਈ (std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_ਸਿਰਾ())) {
        ਸਵੈਚਾਲਿਤ *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(Content.get());
        ਜੇ (DirContent && Name == Content->getName())
          ਵਾਪਸ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ> E =
        std::make_unique<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(
            Name, ਸਥਿਤੀ("", get_ਅਗਲਾ_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    ਜੇ (!ਮਾਪਾ_ਇੰਦਰਾਜ) { // Add a new root to the overlay
      FS->Roots.ਧੱਕੋ_ਪਿੱਠ(std::move(E));
      ਮਾਪਾ_ਇੰਦਰਾਜ = FS->Roots.ਪਿੱਠ().get();
      ਵਾਪਸ ਮਾਪਾ_ਇੰਦਰਾਜ;
    }

    ਸਵੈਚਾਲਿਤ *DE = cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(ਮਾਪਾ_ਇੰਦਰਾਜ);
    DE->ਜੋੜੋ_Content(std::move(E));
    ਵਾਪਸ DE->get_ਆਖਰੀ_Content();
  }

ਨਿਜੀ:
  ਖਾਲੀ unique_Overlay_ਰੁੱਖ(RedirectingFileSystem *FS,
                         RedirectingFileSystem::ਇੰਦਰਾਜ *SrcE,
                         RedirectingFileSystem::ਇੰਦਰਾਜ *New_ਮਾਪਾ_E = ਖਾਲੀ_ਸੂਚਕ) {
    StringRef Name = SrcE->getName();
    ਚੋਣ (SrcE->get_ਕਿਸਮ()) {
    ਹਾਲਤ RedirectingFileSystem::EK_Directory: {
      ਸਵੈਚਾਲਿਤ *DE = cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      ਜੇ (!Name.ਖਾਲੀ_ਥਾਵਾਂ())
        New_ਮਾਪਾ_E = lookup_Or_Create_ਇੰਦਰਾਜ(FS, Name, New_ਮਾਪਾ_E);
      ਲਈ (std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ> &Sub_ਇੰਦਰਾਜ :
           llvm::make_range(DE->contents_begin(), DE->contents_ਸਿਰਾ()))
        unique_Overlay_ਰੁੱਖ(FS, Sub_ਇੰਦਰਾਜ.get(), New_ਮਾਪਾ_E);
      ਰੋਕੋ;
    }
    ਹਾਲਤ RedirectingFileSystem::EK_DirectoryRemap: {
      ਪ੍ਰਤਿਜ੍ਞਾ(New_ਮਾਪਾ_E && "Parent entry must exist");
      ਸਵੈਚਾਲਿਤ *DR = cast<RedirectingFileSystem::Directory_Remap_ਇੰਦਰਾਜ>(SrcE);
      ਸਵੈਚਾਲਿਤ *DE = cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(New_ਮਾਪਾ_E);
      DE->ਜੋੜੋ_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_ਇੰਦਰਾਜ>(
              Name, DR->get_External_Contents_ਰਸਤਾ(), DR->getUseName()));
      ਰੋਕੋ;
    }
    ਹਾਲਤ RedirectingFileSystem::EK_File: {
      ਪ੍ਰਤਿਜ੍ਞਾ(New_ਮਾਪਾ_E && "Parent entry must exist");
      ਸਵੈਚਾਲਿਤ *FE = cast<RedirectingFileSystem::File_ਇੰਦਰਾਜ>(SrcE);
      ਸਵੈਚਾਲਿਤ *DE = cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(New_ਮਾਪਾ_E);
      DE->ਜੋੜੋ_Content(std::make_unique<RedirectingFileSystem::File_ਇੰਦਰਾਜ>(
          Name, FE->get_External_Contents_ਰਸਤਾ(), FE->getUseName()));
      ਰੋਕੋ;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ>
  parse_ਇੰਦਰਾਜ(yaml::ਗੰਢ *N, RedirectingFileSystem *FS, ਤਾਰਕਿਕ Is_ਜੜ੍ਹ_ਇੰਦਰਾਜ) {
    ਸਵੈਚਾਲਿਤ *M = dyn_cast<yaml::Mapping_ਗੰਢ>(N);
    ਜੇ (!M) {
      error(N, "expected mapping node for file or directory entry");
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    }

    ਕੁੰਜੀ_ਸਥਿਤੀ_Pair Fields[] = {
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("name", ਸੱਚ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("type", ਸੱਚ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("contents", ਝੂਠ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("external-contents", ਝੂਠ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("use-external-name", ਝੂਠ),
    };

    DenseMap<StringRef, ਕੁੰਜੀ_ਸਥਿਤੀ> ਕੁੰਜੀਆਂ_2(std::begin(Fields), std::ਸਿਰਾ(Fields));

    ਗਿਣਤੀ { CF_Not_ਸੈੱਟ, CF_List, CF_External } Contents_ਖੇਤਰ = CF_Not_ਸੈੱਟ;
    std::ਸਦਿਸ਼<std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ>>
        ਇੰਦਰਾਜ_Array_Contents;
    SmallString<256> External_Contents_ਰਸਤਾ;
    SmallString<256> Name;
    yaml::ਗੰਢ *Name_ਮੁੱਲ_ਗੰਢ = ਖਾਲੀ_ਸੂਚਕ;
    ਸਵੈਚਾਲਿਤ UseExternalName = RedirectingFileSystem::NK_Not_ਸੈੱਟ;
    RedirectingFileSystem::ਇੰਦਰਾਜ_ਕਿਸਮ ਕਿਸਮ_3;

    ਲਈ (ਸਵੈਚਾਲਿਤ &I : *M) {
      StringRef ਕੁੰਜੀ;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> ਬਫਰ;
      ਜੇ (!parseScalarString(I.get_ਕੁੰਜੀ(), ਕੁੰਜੀ, ਬਫਰ))
        ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

      ਜੇ (!check_Duplicate_Or_Unknown_ਕੁੰਜੀ(I.get_ਕੁੰਜੀ(), ਕੁੰਜੀ, ਕੁੰਜੀਆਂ_2))
        ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

      StringRef ਮੁੱਲ;
      ਜੇ (ਕੁੰਜੀ == "name") {
        ਜੇ (!parseScalarString(I.get_ਮੁੱਲ(), ਮੁੱਲ, ਬਫਰ))
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

        Name_ਮੁੱਲ_ਗੰਢ = I.get_ਮੁੱਲ();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(ਮੁੱਲ).str();
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "type") {
        ਜੇ (!parseScalarString(I.get_ਮੁੱਲ(), ਮੁੱਲ, ਬਫਰ))
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
        ਜੇ (ਮੁੱਲ == "file")
          ਕਿਸਮ_3 = RedirectingFileSystem::EK_File;
        ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ == "directory")
          ਕਿਸਮ_3 = RedirectingFileSystem::EK_Directory;
        ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ == "directory-remap")
          ਕਿਸਮ_3 = RedirectingFileSystem::EK_DirectoryRemap;
        ਨਹੀਂਤਾਂ {
          error(I.get_ਮੁੱਲ(), "unknown value for 'type'");
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "contents") {
        ਜੇ (Contents_ਖੇਤਰ != CF_Not_ਸੈੱਟ) {
          error(I.get_ਕੁੰਜੀ(),
                "entry already has 'contents' or 'external-contents'");
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
        }
        Contents_ਖੇਤਰ = CF_List;
        ਸਵੈਚਾਲਿਤ *Contents = dyn_cast_if_present<yaml::Sequence_ਗੰਢ>(I.get_ਮੁੱਲ());
        ਜੇ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_ਮੁੱਲ(), "expected array");
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
        }

        ਲਈ (ਸਵੈਚਾਲਿਤ &I : *Contents) {
          ਜੇ (std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ> E =
                  parse_ਇੰਦਰਾਜ(&I, FS, /*IsRootEntry*/ ਝੂਠ))
            ਇੰਦਰਾਜ_Array_Contents.ਧੱਕੋ_ਪਿੱਠ(std::move(E));
          ਨਹੀਂਤਾਂ
            ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "external-contents") {
        ਜੇ (Contents_ਖੇਤਰ != CF_Not_ਸੈੱਟ) {
          error(I.get_ਕੁੰਜੀ(),
                "entry already has 'contents' or 'external-contents'");
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
        }
        Contents_ਖੇਤਰ = CF_External;
        ਜੇ (!parseScalarString(I.get_ਮੁੱਲ(), ਮੁੱਲ, ਬਫਰ))
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

        SmallString<256> ਭਰਿਆ_ਰਸਤਾ;
        ਜੇ (FS->IsRelativeOverlay) {
          ਭਰਿਆ_ਰਸਤਾ = FS->getOverlayFileDir();
          ਪ੍ਰਤਿਜ੍ਞਾ(!ਭਰਿਆ_ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_ਭਰਿਆ_ਰਸਤਾ = ਮੁੱਲ;
          ਜੇ (FS->makeAbsolute(ਭਰਿਆ_ਰਸਤਾ, Abs_ਭਰਿਆ_ਰਸਤਾ)) {
            error(N, "failed to make 'external-contents' absolute");
            ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
          }
          ਭਰਿਆ_ਰਸਤਾ = Abs_ਭਰਿਆ_ਰਸਤਾ;
        } ਨਹੀਂਤਾਂ {
          ਭਰਿਆ_ਰਸਤਾ = ਮੁੱਲ;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        ਭਰਿਆ_ਰਸਤਾ = canonicalize(ਭਰਿਆ_ਰਸਤਾ);
        External_Contents_ਰਸਤਾ = ਭਰਿਆ_ਰਸਤਾ.str();
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "use-external-name") {
        ਤਾਰਕਿਕ Val;
        ਜੇ (!parseScalarBool(I.get_ਮੁੱਲ(), Val))
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } ਨਹੀਂਤਾਂ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ਜੇ (ਧਾਰਾ.failed())
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

    // check for missing keys
    ਜੇ (Contents_ਖੇਤਰ == CF_Not_ਸੈੱਟ) {
      error(N, "missing key 'contents' or 'external-contents'");
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    }
    ਜੇ (!check_Missing_ਕੁੰਜੀਆਂ(N, ਕੁੰਜੀਆਂ_2))
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

    // check invalid configuration
    ਜੇ (ਕਿਸਮ_3 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_ਸੈੱਟ) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    }

    ਜੇ (ਕਿਸਮ_3 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_ਖੇਤਰ == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    }

    sys::ਰਸਤਾ_2::Style ਰਸਤਾ_style = sys::ਰਸਤਾ_2::Style::native;
    ਜੇ (Is_ਜੜ੍ਹ_ਇੰਦਰਾਜ) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      ਜੇ (sys::ਰਸਤਾ_2::is_absolute(Name, sys::ਰਸਤਾ_2::Style::posix)) {
        ਰਸਤਾ_style = sys::ਰਸਤਾ_2::Style::posix;
      } ਨਹੀਂਤਾਂ ਜੇ (sys::ਰਸਤਾ_2::is_absolute(Name,
                                        sys::ਰਸਤਾ_2::Style::windows_backslash)) {
        ਰਸਤਾ_style = sys::ਰਸਤਾ_2::Style::windows_backslash;
      } ਨਹੀਂਤਾਂ {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_ਕੋਡ EC;
        ਜੇ (FS->ਜੜ੍ਹ_Relative ==
            RedirectingFileSystem::ਜੜ੍ਹ_Relative_ਕਿਸਮ::OverlayDir) {
          StringRef ਭਰਿਆ_ਰਸਤਾ = FS->getOverlayFileDir();
          ਪ੍ਰਤਿਜ੍ਞਾ(!ਭਰਿਆ_ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(ਭਰਿਆ_ਰਸਤਾ, Name);
          Name = canonicalize(Name);
        } ਨਹੀਂਤਾਂ {
          EC = FS->makeAbsolute(Name);
        }
        ਜੇ (EC) {
          ਪ੍ਰਤਿਜ੍ਞਾ(Name_ਮੁੱਲ_ਗੰਢ && "Name presence should be checked earlier");
          error(
              Name_ਮੁੱਲ_ਗੰਢ,
              "entry with relative path at the root level is not discoverable");
          ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
        }
        ਰਸਤਾ_style = sys::ਰਸਤਾ_2::is_absolute(Name, sys::ਰਸਤਾ_2::Style::posix)
                         ? sys::ਰਸਤਾ_2::Style::posix
                         : sys::ਰਸਤਾ_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      ਜੇ (ਰਸਤਾ_style == sys::ਰਸਤਾ_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::ਰਸਤਾ_2::Style::windows_backslash)
        ਰਸਤਾ_style = sys::ਰਸਤਾ_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t ਜੜ੍ਹ_ਰਸਤਾ_Len = sys::ਰਸਤਾ_2::ਜੜ੍ਹ_ਰਸਤਾ_2(Trimmed, ਰਸਤਾ_style).ਮਾਪ_2();
    ਜਦੋਂ (Trimmed.ਮਾਪ_2() > ਜੜ੍ਹ_ਰਸਤਾ_Len &&
           sys::ਰਸਤਾ_2::is_separator(Trimmed.ਪਿੱਠ(), ਰਸਤਾ_style))
      Trimmed = Trimmed.ਟੁਕੜਾ(0, Trimmed.ਮਾਪ_2() - 1);

    // Get the last component
    StringRef ਆਖਰੀ_Component = sys::ਰਸਤਾ_2::filename(Trimmed, ਰਸਤਾ_style);

    std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ> ਨਤੀਜਾ;
    ਚੋਣ (ਕਿਸਮ_3) {
    ਹਾਲਤ RedirectingFileSystem::EK_File:
      ਨਤੀਜਾ = std::make_unique<RedirectingFileSystem::File_ਇੰਦਰਾਜ>(
          ਆਖਰੀ_Component, std::move(External_Contents_ਰਸਤਾ), UseExternalName);
      ਰੋਕੋ;
    ਹਾਲਤ RedirectingFileSystem::EK_DirectoryRemap:
      ਨਤੀਜਾ = std::make_unique<RedirectingFileSystem::Directory_Remap_ਇੰਦਰਾਜ>(
          ਆਖਰੀ_Component, std::move(External_Contents_ਰਸਤਾ), UseExternalName);
      ਰੋਕੋ;
    ਹਾਲਤ RedirectingFileSystem::EK_Directory:
      ਨਤੀਜਾ = std::make_unique<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(
          ਆਖਰੀ_Component, std::move(ਇੰਦਰਾਜ_Array_Contents),
          ਸਥਿਤੀ("", get_ਅਗਲਾ_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      ਰੋਕੋ;
    }

    StringRef ਮਾਪਾ_2 = sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(Trimmed, ਰਸਤਾ_style);
    ਜੇ (ਮਾਪਾ_2.ਖਾਲੀ_ਥਾਵਾਂ())
      ਵਾਪਸ ਨਤੀਜਾ;

    // if 'name' contains multiple components, create implicit directory entries
    ਲਈ (sys::ਰਸਤਾ_2::ਉਲਟ_iterator I = sys::ਰਸਤਾ_2::rbegin(ਮਾਪਾ_2, ਰਸਤਾ_style),
                                     E = sys::ਰਸਤਾ_2::rend(ਮਾਪਾ_2);
         I != E; ++I) {
      std::ਸਦਿਸ਼<std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ>> Entries;
      Entries.ਧੱਕੋ_ਪਿੱਠ(std::move(ਨਤੀਜਾ));
      ਨਤੀਜਾ = std::make_unique<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(
          *I, std::move(Entries),
          ਸਥਿਤੀ("", get_ਅਗਲਾ_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ਵਾਪਸ ਨਤੀਜਾ;
  }

ਜਨਤਕ:
  RedirectingFileSystemParser(yaml::ਧਾਰਾ &S) : ਧਾਰਾ(S) {}

  // false on error
  ਤਾਰਕਿਕ parse(yaml::ਗੰਢ *ਜੜ੍ਹ_2, RedirectingFileSystem *FS) {
    ਸਵੈਚਾਲਿਤ *ਸਿਖਰ = dyn_cast<yaml::Mapping_ਗੰਢ>(ਜੜ੍ਹ_2);
    ਜੇ (!ਸਿਖਰ) {
      error(ਜੜ੍ਹ_2, "expected mapping node");
      ਵਾਪਸ ਝੂਠ;
    }

    ਕੁੰਜੀ_ਸਥਿਤੀ_Pair Fields[] = {
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("version", ਸੱਚ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("case-sensitive", ਝੂਠ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("use-external-names", ਝੂਠ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("root-relative", ਝੂਠ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("overlay-relative", ਝੂਠ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("fallthrough", ਝੂਠ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("redirecting-with", ਝੂਠ),
        ਕੁੰਜੀ_ਸਥਿਤੀ_Pair("roots", ਸੱਚ),
    };

    DenseMap<StringRef, ਕੁੰਜੀ_ਸਥਿਤੀ> ਕੁੰਜੀਆਂ_2(std::begin(Fields), std::ਸਿਰਾ(Fields));
    std::ਸਦਿਸ਼<std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ>> ਜੜ੍ਹ_Entries;

    // Parse configuration and 'roots'
    ਲਈ (ਸਵੈਚਾਲਿਤ &I : *ਸਿਖਰ) {
      SmallString<10> ਕੁੰਜੀ_ਬਫਰ;
      StringRef ਕੁੰਜੀ;
      ਜੇ (!parseScalarString(I.get_ਕੁੰਜੀ(), ਕੁੰਜੀ, ਕੁੰਜੀ_ਬਫਰ))
        ਵਾਪਸ ਝੂਠ;

      ਜੇ (!check_Duplicate_Or_Unknown_ਕੁੰਜੀ(I.get_ਕੁੰਜੀ(), ਕੁੰਜੀ, ਕੁੰਜੀਆਂ_2))
        ਵਾਪਸ ਝੂਠ;

      ਜੇ (ਕੁੰਜੀ == "roots") {
        ਸਵੈਚਾਲਿਤ *Roots = dyn_cast_if_present<yaml::Sequence_ਗੰਢ>(I.get_ਮੁੱਲ());
        ਜੇ (!Roots) {
          error(I.get_ਮੁੱਲ(), "expected array");
          ਵਾਪਸ ਝੂਠ;
        }

        ਲਈ (ਸਵੈਚਾਲਿਤ &I : *Roots) {
          ਜੇ (std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ> E =
                  parse_ਇੰਦਰਾਜ(&I, FS, /*IsRootEntry*/ ਸੱਚ))
            ਜੜ੍ਹ_Entries.ਧੱਕੋ_ਪਿੱਠ(std::move(E));
          ਨਹੀਂਤਾਂ
            ਵਾਪਸ ਝੂਠ;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "version") {
        StringRef ਸੰਸਕਰਣ_String;
        SmallString<4> Storage;
        ਜੇ (!parseScalarString(I.get_ਮੁੱਲ(), ਸੰਸਕਰਣ_String, Storage))
          ਵਾਪਸ ਝੂਠ;
        ਪੂਰਨ ਸੰਸਕਰਣ;
        ਜੇ (ਸੰਸਕਰਣ_String.getAsInteger<ਪੂਰਨ>(10, ਸੰਸਕਰਣ)) {
          error(I.get_ਮੁੱਲ(), "expected integer");
          ਵਾਪਸ ਝੂਠ;
        }
        ਜੇ (ਸੰਸਕਰਣ < 0) {
          error(I.get_ਮੁੱਲ(), "invalid version number");
          ਵਾਪਸ ਝੂਠ;
        }
        ਜੇ (ਸੰਸਕਰਣ != 0) {
          error(I.get_ਮੁੱਲ(), "version mismatch, expected 0");
          ਵਾਪਸ ਝੂਠ;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "case-sensitive") {
        ਜੇ (!parseScalarBool(I.get_ਮੁੱਲ(), FS->CaseSensitive))
          ਵਾਪਸ ਝੂਠ;
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "overlay-relative") {
        ਜੇ (!parseScalarBool(I.get_ਮੁੱਲ(), FS->IsRelativeOverlay))
          ਵਾਪਸ ਝੂਠ;
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "use-external-names") {
        ਜੇ (!parseScalarBool(I.get_ਮੁੱਲ(), FS->Use_External_ਨਾਂ))
          ਵਾਪਸ ਝੂਠ;
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "fallthrough") {
        ਜੇ (ਕੁੰਜੀਆਂ_2["redirecting-with"].ਵੇਖਿਆ) {
          error(I.get_ਮੁੱਲ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ਵਾਪਸ ਝੂਠ;
        }

        ਤਾਰਕਿਕ ShouldFallthrough = ਝੂਠ;
        ਜੇ (!parseScalarBool(I.get_ਮੁੱਲ(), ShouldFallthrough))
          ਵਾਪਸ ਝੂਠ;

        ਜੇ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::Redirect_ਕਿਸਮ::Fallthrough;
        } ਨਹੀਂਤਾਂ {
          FS->Redirection = RedirectingFileSystem::Redirect_ਕਿਸਮ::RedirectOnly;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "redirecting-with") {
        ਜੇ (ਕੁੰਜੀਆਂ_2["fallthrough"].ਵੇਖਿਆ) {
          error(I.get_ਮੁੱਲ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ਵਾਪਸ ਝੂਠ;
        }

        ਜੇ (ਸਵੈਚਾਲਿਤ ਕਿਸਮ_3 = parse_Redirect_ਕਿਸਮ(I.get_ਮੁੱਲ())) {
          FS->Redirection = *ਕਿਸਮ_3;
        } ਨਹੀਂਤਾਂ {
          error(I.get_ਮੁੱਲ(), "expected valid redirect kind");
          ਵਾਪਸ ਝੂਠ;
        }
      } ਨਹੀਂਤਾਂ ਜੇ (ਕੁੰਜੀ == "root-relative") {
        ਜੇ (ਸਵੈਚਾਲਿਤ ਕਿਸਮ_3 = parse_ਜੜ੍ਹ_Relative_ਕਿਸਮ(I.get_ਮੁੱਲ())) {
          FS->ਜੜ੍ਹ_Relative = *ਕਿਸਮ_3;
        } ਨਹੀਂਤਾਂ {
          error(I.get_ਮੁੱਲ(), "expected valid root-relative kind");
          ਵਾਪਸ ਝੂਠ;
        }
      } ਨਹੀਂਤਾਂ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ਜੇ (ਧਾਰਾ.failed())
      ਵਾਪਸ ਝੂਠ;

    ਜੇ (!check_Missing_ਕੁੰਜੀਆਂ(ਸਿਖਰ, ਕੁੰਜੀਆਂ_2))
      ਵਾਪਸ ਝੂਠ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    ਲਈ (ਸਵੈਚਾਲਿਤ &E : ਜੜ੍ਹ_Entries)
      unique_Overlay_ਰੁੱਖ(FS, E.get());

    ਵਾਪਸ ਸੱਚ;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ> ਬਫਰ,
                              ਸਰੋਤ_Mgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_ਰਸਤਾ, ਖਾਲੀ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ਸਰੋਤ_Mgr SM;
  yaml::ਧਾਰਾ ਧਾਰਾ(ਬਫਰ->get_Mem_ਬਫਰ_Ref(), SM);

  SM.ਸੈੱਟ_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = ਧਾਰਾ.begin();
  yaml::ਗੰਢ *ਜੜ੍ਹ_2 = DI->get_ਜੜ੍ਹ();
  ਜੇ (DI == ਧਾਰਾ.ਸਿਰਾ() || !ਜੜ੍ਹ_2) {
    SM.Print_ਸੁਨੇਹਾ(SMLoc(), ਸਰੋਤ_Mgr::DK_Error, "expected root node");
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }

  RedirectingFileSystemParser P(ਧਾਰਾ);

  std::unique_ptr<RedirectingFileSystem> FS(
      ਨਵਾਂ RedirectingFileSystem(ExternalFS));

  ਜੇ (!YAMLFile_ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(YAMLFile_ਰਸਤਾ);
    std::error_ਕੋਡ EC = FS->makeAbsolute(OverlayAbsDir);
    ਪ੍ਰਤਿਜ੍ਞਾ(!EC && "Overlay dir final path must be absolute");
    (ਖਾਲੀ)EC;
    FS->ਸੈੱਟ_Overlay_File_Dir(OverlayAbsDir);
  }

  ਜੇ (!P.parse(ਜੜ੍ਹ_2, FS.get()))
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  ਵਾਪਸ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    ਤਾਰਕਿਕ Use_External_ਨਾਂ, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      ਨਵਾਂ RedirectingFileSystem(ExternalFS));
  FS->Use_External_ਨਾਂ = Use_External_ਨਾਂ;

  StringMap<RedirectingFileSystem::ਇੰਦਰਾਜ *> Entries;

  ਲਈ (ਸਵੈਚਾਲਿਤ &Mapping : llvm::ਉਲਟ(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.ਪਹਿਲਾ_2);
    SmallString<128> ਮੰਜ਼ਿਲ = StringRef(Mapping.ਦੂਜਾ_2);
    {
      ਸਵੈਚਾਲਿਤ EC = ExternalFS->makeAbsolute(From);
      (ਖਾਲੀ)EC;
      ਪ੍ਰਤਿਜ੍ਞਾ(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::ਇੰਦਰਾਜ *&ਮੰਜ਼ਿਲ_ਇੰਦਰਾਜ = Entries[From];
    ਜੇ (ਮੰਜ਼ਿਲ_ਇੰਦਰਾਜ)
      ਜਾਰੀ;

    // Add parent directories.
    RedirectingFileSystem::ਇੰਦਰਾਜ *ਮਾਪਾ_2 = ਖਾਲੀ_ਸੂਚਕ;
    StringRef FromDirectory = llvm::sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(From);
    ਲਈ (ਸਵੈਚਾਲਿਤ I = llvm::sys::ਰਸਤਾ_2::begin(FromDirectory),
              E = llvm::sys::ਰਸਤਾ_2::ਸਿਰਾ(FromDirectory);
         I != E; ++I) {
      ਮਾਪਾ_2 = RedirectingFileSystemParser::lookup_Or_Create_ਇੰਦਰਾਜ(FS.get(), *I,
                                                                ਮਾਪਾ_2);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ_2 && "File without a directory?");
    {
      ਸਵੈਚਾਲਿਤ EC = ExternalFS->makeAbsolute(ਮੰਜ਼ਿਲ);
      (ਖਾਲੀ)EC;
      ਪ੍ਰਤਿਜ੍ਞਾ(!EC && "Could not make absolute path");
    }

    // Add the file.
    ਸਵੈਚਾਲਿਤ NewFile = std::make_unique<RedirectingFileSystem::File_ਇੰਦਰਾਜ>(
        llvm::sys::ਰਸਤਾ_2::filename(From), ਮੰਜ਼ਿਲ,
        Use_External_ਨਾਂ ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    ਮੰਜ਼ਿਲ_ਇੰਦਰਾਜ = NewFile.get();
    cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(ਮਾਪਾ_2)->ਜੋੜੋ_Content(
        std::move(NewFile));
  }

  ਵਾਪਸ FS;
}

RedirectingFileSystem::Lookup_ਨਤੀਜਾ::Lookup_ਨਤੀਜਾ(
    ਇੰਦਰਾਜ *E, sys::ਰਸਤਾ_2::const_iterator ਸ਼ੁਰੂ, sys::ਰਸਤਾ_2::const_iterator ਸਿਰਾ_2)
    : E(E) {
  ਪ੍ਰਤਿਜ੍ਞਾ(E != ਖਾਲੀ_ਸੂਚਕ);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  ਜੇ (ਸਵੈਚਾਲਿਤ *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_ਇੰਦਰਾਜ>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_ਰਸਤਾ());
    sys::ਰਸਤਾ_2::append(Redirect, ਸ਼ੁਰੂ, ਸਿਰਾ_2,
                      getExistingStyle(DRE->get_External_Contents_ਰਸਤਾ()));
    ExternalRedirect = std::string(Redirect);
  }
}

ਖਾਲੀ RedirectingFileSystem::Lookup_ਨਤੀਜਾ::get_ਰਸਤਾ(
    llvm::Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) ਅਚਲ {
  ਨਤੀਜਾ.clear();
  ਲਈ (ਇੰਦਰਾਜ *ਮਾਪਾ_2 : Parents)
    llvm::sys::ਰਸਤਾ_2::append(ਨਤੀਜਾ, ਮਾਪਾ_2->getName());
  llvm::sys::ਰਸਤਾ_2::append(ਨਤੀਜਾ, E->getName());
}

std::error_ਕੋਡ RedirectingFileSystem::makeCanonicalForLookup(
    Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਰਸਤਾ) ਅਚਲ {
  ਜੇ (std::error_ਕੋਡ EC = makeAbsolute(ਰਸਤਾ))
    ਵਾਪਸ EC;

  llvm::SmallString<256> Canonical_ਰਸਤਾ =
      canonicalize(StringRef(ਰਸਤਾ.ਅੰਕੜੇ(), ਰਸਤਾ.ਮਾਪ_2()));
  ਜੇ (Canonical_ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::invalid_ਦਲੀਲ);

  ਰਸਤਾ.ਸੌਂਪੋ(Canonical_ਰਸਤਾ.begin(), Canonical_ਰਸਤਾ.ਸਿਰਾ());
  ਵਾਪਸ {};
}

ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ>
RedirectingFileSystem::lookup_ਰਸਤਾ(StringRef ਰਸਤਾ) ਅਚਲ {
  llvm::SmallString<128> Canonical_ਰਸਤਾ(ਰਸਤਾ);
  ਜੇ (std::error_ਕੋਡ EC = makeCanonicalForLookup(Canonical_ਰਸਤਾ))
    ਵਾਪਸ EC;

  // RedirectOnly means the VFS is always used.
  ਜੇ (Usage_Tracking_ਸਰਗਰਮ && Redirection == Redirect_ਕਿਸਮ::RedirectOnly)
    Has_Been_ਵਰਤਿਆ = ਸੱਚ;

  sys::ਰਸਤਾ_2::const_iterator ਸ਼ੁਰੂ = sys::ਰਸਤਾ_2::begin(Canonical_ਰਸਤਾ);
  sys::ਰਸਤਾ_2::const_iterator ਸਿਰਾ_2 = sys::ਰਸਤਾ_2::ਸਿਰਾ(Canonical_ਰਸਤਾ);
  llvm::Small_ਸਦਿਸ਼<ਇੰਦਰਾਜ *, 32> Entries;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਜੜ੍ਹ_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਨਤੀਜਾ =
        lookup_ਰਸਤਾ_Impl(ਸ਼ੁਰੂ, ਸਿਰਾ_2, ਜੜ੍ਹ_2.get(), Entries);
    ਜੇ (Usage_Tracking_ਸਰਗਰਮ && ਨਤੀਜਾ && isa<Remap_ਇੰਦਰਾਜ>(ਨਤੀਜਾ->E))
      Has_Been_ਵਰਤਿਆ = ਸੱਚ;
    ਜੇ (ਨਤੀਜਾ) {
      ਨਤੀਜਾ->Parents = std::move(Entries);
      ਵਾਪਸ ਨਤੀਜਾ;
    }

    ਜੇ (ਨਤੀਜਾ.getError() != llvm::errc::no_such_file_or_directory)
      ਵਾਪਸ ਨਤੀਜਾ;
  }
  ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ>
RedirectingFileSystem::lookup_ਰਸਤਾ_Impl(
    sys::ਰਸਤਾ_2::const_iterator ਸ਼ੁਰੂ, sys::ਰਸਤਾ_2::const_iterator ਸਿਰਾ_2,
    RedirectingFileSystem::ਇੰਦਰਾਜ *From,
    llvm::Small_ਸਦਿਸ਼_Impl<ਇੰਦਰਾਜ *> &Entries) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(!isTraversalComponent(*ਸ਼ੁਰੂ) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  ਜੇ (!FromName.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਜੇ (!ਰਸਤਾ_Component_Matches(*ਸ਼ੁਰੂ, FromName))
      ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::no_such_file_or_directory);

    ++ਸ਼ੁਰੂ;

    ਜੇ (ਸ਼ੁਰੂ == ਸਿਰਾ_2) {
      // Match!
      ਵਾਪਸ Lookup_ਨਤੀਜਾ(From, ਸ਼ੁਰੂ, ਸਿਰਾ_2);
    }
  }

  ਜੇ (isa<RedirectingFileSystem::File_ਇੰਦਰਾਜ>(From))
    ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::not_a_directory);

  ਜੇ (isa<RedirectingFileSystem::Directory_Remap_ਇੰਦਰਾਜ>(From))
    ਵਾਪਸ Lookup_ਨਤੀਜਾ(From, ਸ਼ੁਰੂ, ਸਿਰਾ_2);

  ਸਵੈਚਾਲਿਤ *DE = cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(From);
  ਲਈ (ਅਚਲ std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ> &Dir_ਇੰਦਰਾਜ :
       llvm::make_range(DE->contents_begin(), DE->contents_ਸਿਰਾ())) {
    Entries.ਧੱਕੋ_ਪਿੱਠ(From);
    ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਨਤੀਜਾ =
        lookup_ਰਸਤਾ_Impl(ਸ਼ੁਰੂ, ਸਿਰਾ_2, Dir_ਇੰਦਰਾਜ.get(), Entries);
    ਜੇ (ਨਤੀਜਾ || ਨਤੀਜਾ.getError() != llvm::errc::no_such_file_or_directory)
      ਵਾਪਸ ਨਤੀਜਾ;
    Entries.ਕੱਢੋ_ਪਿੱਠ();
  }

  ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::no_such_file_or_directory);
}

ਸਥਿਰ ਸਥਿਤੀ get_Redirected_File_ਸਥਿਤੀ(ਅਚਲ Twine &Original_ਰਸਤਾ,
                                      ਤਾਰਕਿਕ Use_External_ਨਾਂ,
                                      ਸਥਿਤੀ External_ਸਥਿਤੀ) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  ਜੇ (External_ਸਥਿਤੀ.ExposesExternalVFSPath)
    ਵਾਪਸ External_ਸਥਿਤੀ;

  ਸਥਿਤੀ S = External_ਸਥਿਤੀ;
  ਜੇ (!Use_External_ਨਾਂ)
    S = ਸਥਿਤੀ::copyWithNewName(S, Original_ਰਸਤਾ);
  ਨਹੀਂਤਾਂ
    S.ExposesExternalVFSPath = ਸੱਚ;
  ਵਾਪਸ S;
}

ErrorOr<ਸਥਿਤੀ> RedirectingFileSystem::ਸਥਿਤੀ_2(
    ਅਚਲ Twine &Lookup_ਰਸਤਾ, ਅਚਲ Twine &Original_ਰਸਤਾ,
    ਅਚਲ RedirectingFileSystem::Lookup_ਨਤੀਜਾ &ਨਤੀਜਾ) {
  ਜੇ (std::optional<StringRef> ExtRedirect = ਨਤੀਜਾ.getExternalRedirect()) {
    SmallString<256> Remapped_ਰਸਤਾ((*ExtRedirect).str());
    ਜੇ (std::error_ਕੋਡ EC = makeAbsolute(Remapped_ਰਸਤਾ))
      ਵਾਪਸ EC;

    ErrorOr<ਸਥਿਤੀ> S = ExternalFS->ਸਥਿਤੀ_2(Remapped_ਰਸਤਾ);
    ਜੇ (!S)
      ਵਾਪਸ S;
    S = ਸਥਿਤੀ::copyWithNewName(*S, *ExtRedirect);
    ਸਵੈਚਾਲਿਤ *RE = cast<RedirectingFileSystem::Remap_ਇੰਦਰਾਜ>(ਨਤੀਜਾ.E);
    ਵਾਪਸ get_Redirected_File_ਸਥਿਤੀ(Original_ਰਸਤਾ,
                                   RE->useExternalName(Use_External_ਨਾਂ), *S);
  }

  ਸਵੈਚਾਲਿਤ *DE = cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(ਨਤੀਜਾ.E);
  ਵਾਪਸ ਸਥਿਤੀ::copyWithNewName(DE->get_ਸਥਿਤੀ(), Lookup_ਰਸਤਾ);
}

ErrorOr<ਸਥਿਤੀ>
RedirectingFileSystem::get_External_ਸਥਿਤੀ(ਅਚਲ Twine &Lookup_ਰਸਤਾ,
                                         ਅਚਲ Twine &Original_ਰਸਤਾ) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ ਨਤੀਜਾ = ExternalFS->ਸਥਿਤੀ_2(Lookup_ਰਸਤਾ);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  ਜੇ (!ਨਤੀਜਾ || ਨਤੀਜਾ->ExposesExternalVFSPath)
    ਵਾਪਸ ਨਤੀਜਾ;
  ਵਾਪਸ ਸਥਿਤੀ::copyWithNewName(ਨਤੀਜਾ.get(), Original_ਰਸਤਾ);
}

ErrorOr<ਸਥਿਤੀ> RedirectingFileSystem::ਸਥਿਤੀ_2(ਅਚਲ Twine &Original_ਰਸਤਾ) {
  SmallString<256> ਰਸਤਾ;
  Original_ਰਸਤਾ.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  ਜੇ (std::error_ਕੋਡ EC = makeAbsolute(ਰਸਤਾ))
    ਵਾਪਸ EC;

  ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<ਸਥਿਤੀ> S = get_External_ਸਥਿਤੀ(ਰਸਤਾ, Original_ਰਸਤਾ);
    ਜੇ (S)
      ਵਾਪਸ S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਨਤੀਜਾ = lookup_ਰਸਤਾ(ਰਸਤਾ);
  ਜੇ (!ਨਤੀਜਾ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallthrough &&
        is_File_Not_ਮਿਲਿਆ(ਨਤੀਜਾ.getError()))
      ਵਾਪਸ get_External_ਸਥਿਤੀ(ਰਸਤਾ, Original_ਰਸਤਾ);
    ਵਾਪਸ ਨਤੀਜਾ.getError();
  }

  ErrorOr<ਸਥਿਤੀ> S = ਸਥਿਤੀ_2(ਰਸਤਾ, Original_ਰਸਤਾ, *ਨਤੀਜਾ);
  ਜੇ (!S && Redirection == Redirect_ਕਿਸਮ::Fallthrough &&
      is_File_Not_ਮਿਲਿਆ(S.getError(), ਨਤੀਜਾ->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ਵਾਪਸ get_External_ਸਥਿਤੀ(ਰਸਤਾ, Original_ਰਸਤਾ);
  }

  ਵਾਪਸ S;
}

ਤਾਰਕਿਕ RedirectingFileSystem::exists(ਅਚਲ Twine &Original_ਰਸਤਾ) {
  SmallString<256> ਰਸਤਾ;
  Original_ਰਸਤਾ.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  ਜੇ (makeAbsolute(ਰਸਤਾ))
    ਵਾਪਸ ਝੂਠ;

  ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ਜੇ (ExternalFS->exists(ਰਸਤਾ))
      ਵਾਪਸ ਸੱਚ;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਨਤੀਜਾ = lookup_ਰਸਤਾ(ਰਸਤਾ);
  ਜੇ (!ਨਤੀਜਾ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallthrough &&
        is_File_Not_ਮਿਲਿਆ(ਨਤੀਜਾ.getError()))
      ਵਾਪਸ ExternalFS->exists(ਰਸਤਾ);
    ਵਾਪਸ ਝੂਠ;
  }

  std::optional<StringRef> ExtRedirect = ਨਤੀਜਾ->getExternalRedirect();
  ਜੇ (!ExtRedirect) {
    ਪ੍ਰਤਿਜ੍ਞਾ(isa<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(ਨਤੀਜਾ->E));
    ਵਾਪਸ ਸੱਚ;
  }

  SmallString<256> Remapped_ਰਸਤਾ((*ExtRedirect).str());
  ਜੇ (makeAbsolute(Remapped_ਰਸਤਾ))
    ਵਾਪਸ ਝੂਠ;

  ਜੇ (ExternalFS->exists(Remapped_ਰਸਤਾ))
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ਵਾਪਸ ExternalFS->exists(ਰਸਤਾ);
  }

  ਵਾਪਸ ਝੂਠ;
}

ਨਾਮਾਕਾਸ਼ {

/// Provide a file wrapper with an overriden status.
ਵਰਗ File_With_Fixed_ਸਥਿਤੀ : ਜਨਤਕ File {
  std::unique_ptr<File> InnerFile;
  ਸਥਿਤੀ S;

ਜਨਤਕ:
  File_With_Fixed_ਸਥਿਤੀ(std::unique_ptr<File> InnerFile, ਸਥਿਤੀ S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<ਸਥਿਤੀ> ਸਥਿਤੀ_2() override { ਵਾਪਸ S; }
  ErrorOr<std::unique_ptr<llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ>>

  get_ਬਫਰ(ਅਚਲ Twine &Name, int64_t File_ਮਾਪ, ਤਾਰਕਿਕ RequiresNullTerminator,
            ਤਾਰਕਿਕ IsVolatile) override {
    ਵਾਪਸ InnerFile->get_ਬਫਰ(Name, File_ਮਾਪ, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_ਕੋਡ close() override { ਵਾਪਸ InnerFile->close(); }

  ਖਾਲੀ ਸੈੱਟ_ਰਸਤਾ(ਅਚਲ Twine &ਰਸਤਾ) override { S = S.copyWithNewName(S, ਰਸਤਾ); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_ਰਸਤਾ(ErrorOr<std::unique_ptr<File>> ਨਤੀਜਾ, ਅਚਲ Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  ਜੇ (!ਨਤੀਜਾ || (*ਨਤੀਜਾ)->ਸਥਿਤੀ_2()->ExposesExternalVFSPath)
    ਵਾਪਸ ਨਤੀਜਾ;

  ErrorOr<std::unique_ptr<File>> F = std::move(*ਨਤੀਜਾ);
  ਸਵੈਚਾਲਿਤ Name = F->get()->getName();
  ਜੇ (Name && Name.get() != P.str())
    F->get()->ਸੈੱਟ_ਰਸਤਾ(P);
  ਵਾਪਸ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ਅਚਲ Twine &Original_ਰਸਤਾ) {
  SmallString<256> ਰਸਤਾ;
  Original_ਰਸਤਾ.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  ਜੇ (std::error_ਕੋਡ EC = makeAbsolute(ਰਸਤਾ))
    ਵਾਪਸ EC;

  ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ਸਵੈਚਾਲਿਤ F = File::get_With_ਰਸਤਾ(ExternalFS->openFileForRead(ਰਸਤਾ), Original_ਰਸਤਾ);
    ਜੇ (F)
      ਵਾਪਸ F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਨਤੀਜਾ = lookup_ਰਸਤਾ(ਰਸਤਾ);
  ਜੇ (!ਨਤੀਜਾ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallthrough &&
        is_File_Not_ਮਿਲਿਆ(ਨਤੀਜਾ.getError()))
      ਵਾਪਸ File::get_With_ਰਸਤਾ(ExternalFS->openFileForRead(ਰਸਤਾ), Original_ਰਸਤਾ);
    ਵਾਪਸ ਨਤੀਜਾ.getError();
  }

  ਜੇ (!ਨਤੀਜਾ->getExternalRedirect()) // FIXME: errc::not_a_file?
    ਵਾਪਸ make_error_ਕੋਡ(llvm::errc::invalid_ਦਲੀਲ);

  StringRef ExtRedirect = *ਨਤੀਜਾ->getExternalRedirect();
  SmallString<256> Remapped_ਰਸਤਾ(ExtRedirect.str());
  ਜੇ (std::error_ਕੋਡ EC = makeAbsolute(Remapped_ਰਸਤਾ))
    ਵਾਪਸ EC;

  ਸਵੈਚਾਲਿਤ *RE = cast<RedirectingFileSystem::Remap_ਇੰਦਰਾਜ>(ਨਤੀਜਾ->E);

  ਸਵੈਚਾਲਿਤ ExternalFile =
      File::get_With_ਰਸਤਾ(ExternalFS->openFileForRead(Remapped_ਰਸਤਾ), ExtRedirect);
  ਜੇ (!ExternalFile) {
    ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallthrough &&
        is_File_Not_ਮਿਲਿਆ(ExternalFile.getError(), ਨਤੀਜਾ->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ਵਾਪਸ File::get_With_ਰਸਤਾ(ExternalFS->openFileForRead(ਰਸਤਾ), Original_ਰਸਤਾ);
    }
    ਵਾਪਸ ExternalFile;
  }

  ਸਵੈਚਾਲਿਤ External_ਸਥਿਤੀ = (*ExternalFile)->ਸਥਿਤੀ_2();
  ਜੇ (!External_ਸਥਿਤੀ)
    ਵਾਪਸ External_ਸਥਿਤੀ.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  ਸਥਿਤੀ S = get_Redirected_File_ਸਥਿਤੀ(
      Original_ਰਸਤਾ, RE->useExternalName(Use_External_ਨਾਂ), *External_ਸਥਿਤੀ);
  ਵਾਪਸ std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_ਸਥਿਤੀ>(std::move(*ExternalFile), S));
}

std::error_ਕੋਡ
RedirectingFileSystem::get_Real_ਰਸਤਾ(ਅਚਲ Twine &Original_ਰਸਤਾ,
                                   Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Output) {
  SmallString<256> ਰਸਤਾ;
  Original_ਰਸਤਾ.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  ਜੇ (std::error_ਕੋਡ EC = makeAbsolute(ਰਸਤਾ))
    ਵਾਪਸ EC;

  ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_ਕੋਡ EC = ExternalFS->get_Real_ਰਸਤਾ(ਰਸਤਾ, Output);
    ਜੇ (!EC)
      ਵਾਪਸ EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਨਤੀਜਾ = lookup_ਰਸਤਾ(ਰਸਤਾ);
  ਜੇ (!ਨਤੀਜਾ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallthrough &&
        is_File_Not_ਮਿਲਿਆ(ਨਤੀਜਾ.getError()))
      ਵਾਪਸ ExternalFS->get_Real_ਰਸਤਾ(ਰਸਤਾ, Output);
    ਵਾਪਸ ਨਤੀਜਾ.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  ਜੇ (ਸਵੈਚਾਲਿਤ ExtRedirect = ਨਤੀਜਾ->getExternalRedirect()) {
    ਸਵੈਚਾਲਿਤ P = ExternalFS->get_Real_ਰਸਤਾ(*ExtRedirect, Output);
    ਜੇ (P && Redirection == Redirect_ਕਿਸਮ::Fallthrough &&
        is_File_Not_ਮਿਲਿਆ(P, ਨਤੀਜਾ->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ਵਾਪਸ ExternalFS->get_Real_ਰਸਤਾ(ਰਸਤਾ, Output);
    }
    ਵਾਪਸ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  ਜੇ (Redirection == Redirect_ਕਿਸਮ::Fallthrough) {
    ਨਤੀਜਾ->get_ਰਸਤਾ(Output);
    ਵਾਪਸ {};
  }
  ਵਾਪਸ llvm::errc::invalid_ਦਲੀਲ;
}

ਖਾਲੀ RedirectingFileSystem::getDirectoryContentRealSources(
    ਅਚਲ Twine &Dir, Small_ਸਦਿਸ਼_Impl<std::string> &ਬਾਹਰਲਾ) {
  SmallString<256> ਰਸਤਾ;
  Dir.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);

  ਜੇ (makeAbsolute(ਰਸਤਾ))
    ਵਾਪਸ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ਅਚਲ ਤਾਰਕਿਕ ConsultsExternalFS = Redirection != Redirect_ਕਿਸਮ::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਨਤੀਜਾ = lookup_ਰਸਤਾ(ਰਸਤਾ);
  ਜੇ (!ਨਤੀਜਾ) {
    // dir_begin() delegates entirely to ExternalFS.
    ਜੇ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(ਰਸਤਾ, ਬਾਹਰਲਾ);
    ਵਾਪਸ;
  }

  ਚੋਣ (ਨਤੀਜਾ->E->get_ਕਿਸਮ()) {
  ਹਾਲਤ EK_File:
    ਵਾਪਸ;
  ਹਾਲਤ EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    ਰੋਕੋ;
  ਹਾਲਤ EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_ਰਸਤਾ(*ਨਤੀਜਾ->getExternalRedirect());
    ਜੇ (!makeAbsolute(Remapped_ਰਸਤਾ))
      ExternalFS->getDirectoryContentRealSources(Remapped_ਰਸਤਾ, ਬਾਹਰਲਾ);
    ਰੋਕੋ;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  ਜੇ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(ਰਸਤਾ, ਬਾਹਰਲਾ);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ> ਬਫਰ,
                    ਸਰੋਤ_Mgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_ਰਸਤਾ, ਖਾਲੀ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ਵਾਪਸ RedirectingFileSystem::create(std::move(ਬਫਰ), DiagHandler,
                                       YAMLFile_ਰਸਤਾ, DiagContext,
                                       std::move(ExternalFS));
}

ਸਥਿਰ ਖਾਲੀ getVFSEntries(RedirectingFileSystem::ਇੰਦਰਾਜ *SrcE,
                          Small_ਸਦਿਸ਼_Impl<StringRef> &ਰਸਤਾ,
                          Small_ਸਦਿਸ਼_Impl<YAMLVFSEntry> &Entries) {
  ਸਵੈਚਾਲਿਤ ਕਿਸਮ_3 = SrcE->get_ਕਿਸਮ();
  ਜੇ (ਕਿਸਮ_3 == RedirectingFileSystem::EK_Directory) {
    ਸਵੈਚਾਲਿਤ *DE = dyn_cast<RedirectingFileSystem::Directory_ਇੰਦਰਾਜ>(SrcE);
    ਪ੍ਰਤਿਜ੍ਞਾ(DE && "Must be a directory");
    ਲਈ (std::unique_ptr<RedirectingFileSystem::ਇੰਦਰਾਜ> &Sub_ਇੰਦਰਾਜ :
         llvm::make_range(DE->contents_begin(), DE->contents_ਸਿਰਾ())) {
      ਰਸਤਾ.ਧੱਕੋ_ਪਿੱਠ(Sub_ਇੰਦਰਾਜ->getName());
      getVFSEntries(Sub_ਇੰਦਰਾਜ.get(), ਰਸਤਾ, Entries);
      ਰਸਤਾ.ਕੱਢੋ_ਪਿੱਠ();
    }
    ਵਾਪਸ;
  }

  ਜੇ (ਕਿਸਮ_3 == RedirectingFileSystem::EK_DirectoryRemap) {
    ਸਵੈਚਾਲਿਤ *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_ਇੰਦਰਾਜ>(SrcE);
    ਪ੍ਰਤਿਜ੍ਞਾ(DR && "Must be a directory remap");
    SmallString<128> VPath;
    ਲਈ (ਸਵੈਚਾਲਿਤ &Comp : ਰਸਤਾ)
      llvm::sys::ਰਸਤਾ_2::append(VPath, Comp);
    Entries.ਧੱਕੋ_ਪਿੱਠ(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_ਰਸਤਾ()));
    ਵਾਪਸ;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(ਕਿਸਮ_3 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  ਸਵੈਚਾਲਿਤ *FE = dyn_cast<RedirectingFileSystem::File_ਇੰਦਰਾਜ>(SrcE);
  ਪ੍ਰਤਿਜ੍ਞਾ(FE && "Must be a file");
  SmallString<128> VPath;
  ਲਈ (ਸਵੈਚਾਲਿਤ &Comp : ਰਸਤਾ)
    llvm::sys::ਰਸਤਾ_2::append(VPath, Comp);
  Entries.ਧੱਕੋ_ਪਿੱਠ(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_ਰਸਤਾ()));
}

ਖਾਲੀ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_ਸਦਿਸ਼_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_ਨਤੀਜਾ> ਜੜ੍ਹ_ਨਤੀਜਾ = VFS.lookup_ਰਸਤਾ("/");
  ਜੇ (!ਜੜ੍ਹ_ਨਤੀਜਾ)
    ਵਾਪਸ;
  Small_ਸਦਿਸ਼<StringRef, 8> Components;
  Components.ਧੱਕੋ_ਪਿੱਠ("/");
  getVFSEntries(ਜੜ੍ਹ_ਨਤੀਜਾ->E, Components, CollectedEntries);
}

UniqueID vfs::get_ਅਗਲਾ_Virtual_Unique_ID() {
  ਸਥਿਰ std::atomic<ਬਿਨਾਨਿਸ਼ਾਨੀ> UID;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ਵਾਪਸ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

ਖਾਲੀ YAMLVFSWriter::ਜੋੜੋ_ਇੰਦਰਾਜ(StringRef Virtual_ਰਸਤਾ, StringRef Real_ਰਸਤਾ,
                             ਤਾਰਕਿਕ IsDirectory) {
  ਪ੍ਰਤਿਜ੍ਞਾ(sys::ਰਸਤਾ_2::is_absolute(Virtual_ਰਸਤਾ) && "virtual path not absolute");
  ਪ੍ਰਤਿਜ੍ਞਾ(sys::ਰਸਤਾ_2::is_absolute(Real_ਰਸਤਾ) && "real path not absolute");
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਰਸਤਾ_Has_Traversal(Virtual_ਰਸਤਾ) && "path traversal is not supported");
  Mappings.emplace_ਪਿੱਠ(Virtual_ਰਸਤਾ, Real_ਰਸਤਾ, IsDirectory);
}

ਖਾਲੀ YAMLVFSWriter::ਜੋੜੋ_File_Mapping(StringRef Virtual_ਰਸਤਾ, StringRef Real_ਰਸਤਾ) {
  ਜੋੜੋ_ਇੰਦਰਾਜ(Virtual_ਰਸਤਾ, Real_ਰਸਤਾ, /*IsDirectory=*/ਝੂਠ);
}

ਖਾਲੀ YAMLVFSWriter::ਜੋੜੋ_Directory_Mapping(StringRef Virtual_ਰਸਤਾ,
                                        StringRef Real_ਰਸਤਾ) {
  ਜੋੜੋ_ਇੰਦਰਾਜ(Virtual_ਰਸਤਾ, Real_ਰਸਤਾ, /*IsDirectory=*/ਸੱਚ);
}

ਨਾਮਾਕਾਸ਼ {

ਵਰਗ JSONWriter {
  llvm::raw_ostream &OS;
  Small_ਸਦਿਸ਼<StringRef, 16> Dir_ਢੇਰ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ getDirIndent() { ਵਾਪਸ 4 * Dir_ਢੇਰ.ਮਾਪ_2(); }
  ਬਿਨਾਨਿਸ਼ਾਨੀ getFileIndent() { ਵਾਪਸ 4 * (Dir_ਢੇਰ.ਮਾਪ_2() + 1); }
  ਤਾਰਕਿਕ contained_ਵਿੱਚ(StringRef ਮਾਪਾ_2, StringRef ਰਸਤਾ);
  StringRef containedPart(StringRef ਮਾਪਾ_2, StringRef ਰਸਤਾ);
  ਖਾਲੀ ਸ਼ੁਰੂ_Directory(StringRef ਰਸਤਾ);
  ਖਾਲੀ ਸਿਰਾ_Directory();
  ਖਾਲੀ write_ਇੰਦਰਾਜ(StringRef VPath, StringRef RPath);

ਜਨਤਕ:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  ਖਾਲੀ ਲਿਖ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<ਤਾਰਕਿਕ> Use_External_ਨਾਂ,
             std::optional<ਤਾਰਕਿਕ> IsCaseSensitive,
             std::optional<ਤਾਰਕਿਕ> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

ਤਾਰਕਿਕ JSONWriter::contained_ਵਿੱਚ(StringRef ਮਾਪਾ_2, StringRef ਰਸਤਾ) {
  ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::sys;

  // Compare each path component.
  ਸਵੈਚਾਲਿਤ IParent = ਰਸਤਾ_2::begin(ਮਾਪਾ_2), EParent = ਰਸਤਾ_2::ਸਿਰਾ(ਮਾਪਾ_2);
  ਲਈ (ਸਵੈਚਾਲਿਤ IChild = ਰਸਤਾ_2::begin(ਰਸਤਾ), EChild = ਰਸਤਾ_2::ਸਿਰਾ(ਰਸਤਾ);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    ਜੇ (*IParent != *IChild)
      ਵਾਪਸ ਝੂਠ;
  }
  // Have we exhausted the parent path?
  ਵਾਪਸ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef ਮਾਪਾ_2, StringRef ਰਸਤਾ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਮਾਪਾ_2.ਖਾਲੀ_ਥਾਵਾਂ());
  ਪ੍ਰਤਿਜ੍ਞਾ(contained_ਵਿੱਚ(ਮਾਪਾ_2, ਰਸਤਾ));
  ਵਾਪਸ ਰਸਤਾ.substr(ਮਾਪਾ_2.ਮਾਪ_2() + 1);
}

ਖਾਲੀ JSONWriter::ਸ਼ੁਰੂ_Directory(StringRef ਰਸਤਾ) {
  StringRef Name =
      Dir_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() ? ਰਸਤਾ : containedPart(Dir_ਢੇਰ.ਪਿੱਠ(), ਰਸਤਾ);
  Dir_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(ਰਸਤਾ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

ਖਾਲੀ JSONWriter::ਸਿਰਾ_Directory() {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
}

ਖਾਲੀ JSONWriter::write_ਇੰਦਰਾਜ(StringRef VPath, StringRef RPath) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

ਖਾਲੀ JSONWriter::ਲਿਖ(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<ਤਾਰਕਿਕ> Use_External_ਨਾਂ,
                       std::optional<ਤਾਰਕਿਕ> IsCaseSensitive,
                       std::optional<ਤਾਰਕਿਕ> IsOverlayRelative,
                       StringRef OverlayDir) {
  ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  ਜੇ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  ਜੇ (Use_External_ਨਾਂ)
    OS << "  'use-external-names': '" << (*Use_External_ਨਾਂ ? "true" : "false")
       << "',\n";
  ਤਾਰਕਿਕ UseOverlayRelative = ਝੂਠ;
  ਜੇ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  ਜੇ (!Entries.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਅਚਲ YAMLVFSEntry &ਇੰਦਰਾਜ = Entries.front();

    ਸ਼ੁਰੂ_Directory(
      ਇੰਦਰਾਜ.IsDirectory ? ਇੰਦਰਾਜ.VPath : ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(ਇੰਦਰਾਜ.VPath)
    );

    StringRef RPath = ਇੰਦਰਾਜ.RPath;
    ਜੇ (UseOverlayRelative) {
      ਪ੍ਰਤਿਜ੍ਞਾ(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.ਮਾਪ_2());
    }

    ਤਾਰਕਿਕ Is_Current_Dir_ਖਾਲੀ_ਥਾਵਾਂ = ਸੱਚ;
    ਜੇ (!ਇੰਦਰਾਜ.IsDirectory) {
      write_ਇੰਦਰਾਜ(ਰਸਤਾ_2::filename(ਇੰਦਰਾਜ.VPath), RPath);
      Is_Current_Dir_ਖਾਲੀ_ਥਾਵਾਂ = ਝੂਠ;
    }

    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਇੰਦਰਾਜ : Entries.ਟੁਕੜਾ(1)) {
      StringRef Dir =
          ਇੰਦਰਾਜ.IsDirectory ? ਇੰਦਰਾਜ.VPath : ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(ਇੰਦਰਾਜ.VPath);
      ਜੇ (Dir == Dir_ਢੇਰ.ਪਿੱਠ()) {
        ਜੇ (!Is_Current_Dir_ਖਾਲੀ_ਥਾਵਾਂ) {
          OS << ",\n";
        }
      } ਨਹੀਂਤਾਂ {
        ਤਾਰਕਿਕ Is_Dir_Popped_From_ਢੇਰ = ਝੂਠ;
        ਜਦੋਂ (!Dir_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() && !contained_ਵਿੱਚ(Dir_ਢੇਰ.ਪਿੱਠ(), Dir)) {
          OS << "\n";
          ਸਿਰਾ_Directory();
          Is_Dir_Popped_From_ਢੇਰ = ਸੱਚ;
        }
        ਜੇ (Is_Dir_Popped_From_ਢੇਰ || !Is_Current_Dir_ਖਾਲੀ_ਥਾਵਾਂ) {
          OS << ",\n";
        }
        ਸ਼ੁਰੂ_Directory(Dir);
        Is_Current_Dir_ਖਾਲੀ_ਥਾਵਾਂ = ਸੱਚ;
      }
      StringRef RPath = ਇੰਦਰਾਜ.RPath;
      ਜੇ (UseOverlayRelative) {
        ਪ੍ਰਤਿਜ੍ਞਾ(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.ਮਾਪ_2());
      }
      ਜੇ (!ਇੰਦਰਾਜ.IsDirectory) {
        write_ਇੰਦਰਾਜ(ਰਸਤਾ_2::filename(ਇੰਦਰਾਜ.VPath), RPath);
        Is_Current_Dir_ਖਾਲੀ_ਥਾਵਾਂ = ਝੂਠ;
      }
    }

    ਜਦੋਂ (!Dir_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ()) {
      OS << "\n";
      ਸਿਰਾ_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

ਖਾਲੀ YAMLVFSWriter::ਲਿਖ(llvm::raw_ostream &OS) {
  llvm::ਛਾਂਟੋ(Mappings, [](ਅਚਲ YAMLVFSEntry &LHS, ਅਚਲ YAMLVFSEntry &RHS) {
    ਵਾਪਸ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).ਲਿਖ(Mappings, Use_External_ਨਾਂ, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ਅਚਲ Twine &ਰਸਤਾ, std::error_ਕੋਡ &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(ਰਸਤਾ, EC);
  ਜੇ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_ਕੋਡ &EC) {
  ਪ੍ਰਤਿਜ੍ਞਾ(FS && State && !State->ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() && "incrementing past end");
  ਪ੍ਰਤਿਜ੍ਞਾ(!State->ਢੇਰ.ਪਿੱਠ()->ਰਸਤਾ_2().ਖਾਲੀ_ਥਾਵਾਂ() && "non-canonical end iterator");
  vfs::directory_iterator ਸਿਰਾ_2;

  ਜੇ (State->Has_No_ਧੱਕੋ_Request)
    State->Has_No_ਧੱਕੋ_Request = ਝੂਠ;
  ਨਹੀਂਤਾਂ {
    ਜੇ (State->ਢੇਰ.ਪਿੱਠ()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ਢੇਰ.ਪਿੱਠ()->ਰਸਤਾ_2(), EC);
      ਜੇ (I != ਸਿਰਾ_2) {
        State->ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ(I);
        ਵਾਪਸ *ਇਹ;
      }
    }
  }

  ਜਦੋਂ (!State->ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ() && State->ਢੇਰ.ਪਿੱਠ().increment(EC) == ਸਿਰਾ_2)
    State->ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();

  ਜੇ (State->ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    State.reset(); // end iterator

  ਵਾਪਸ *ਇਹ;
}

ਅਚਲ ਅੱਖਰ FileSystem::ID = 0;
ਅਚਲ ਅੱਖਰ OverlayFileSystem::ID = 0;
ਅਚਲ ਅੱਖਰ ProxyFileSystem::ID = 0;
ਅਚਲ ਅੱਖਰ ਵਿੱਚ_ਸਮ੍ਰਿਤੀ_File_System::ID = 0;
ਅਚਲ ਅੱਖਰ RedirectingFileSystem::ID = 0;

ਬਿਨਾਨਿਸ਼ਾਨੀ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_ਗਿਣਤੀ(
    ਅਚਲ FileSystem *FS) {
  ਵਾਪਸ FS->Use_ਗਿਣਤੀ();
}

ਖਾਲੀ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

ਖਾਲੀ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
