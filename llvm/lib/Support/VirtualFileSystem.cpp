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

#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualFileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/DenseMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/IntrusiveRefCntPtr.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/iterator_range.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Casting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Chrono.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorOr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem/UniqueID.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SMLoc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SourceMgr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/YAMLParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <atomic>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstdint>
#ಅನ್ತರ್ಭಾವ <iterator>
#ಅನ್ತರ್ಭಾವ <limits>
#ಅನ್ತರ್ಭಾವ <map>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <optional>
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <system_error>
#ಅನ್ತರ್ಭಾವ <utility>
#ಅನ್ತರ್ಭಾವ <vector>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::vfs;

ಬಳಸಿ llvm::sys::fs::file_ಸ್ಥಿತಿ;
ಬಳಸಿ llvm::sys::fs::file_t;
ಬಳಸಿ llvm::sys::fs::file_type;
ಬಳಸಿ llvm::sys::fs::perms;
ಬಳಸಿ llvm::sys::fs::UniqueID;

ಸ್ಥಿತಿ_2::ಸ್ಥಿತಿ_2(ಸ್ಥಿರಾಂಕ file_ಸ್ಥಿತಿ &ಸ್ಥಿತಿ_2)
    : UID(ಸ್ಥಿತಿ_2.getUniqueID()), MTime(ಸ್ಥಿತಿ_2.get_ಕೊನೆಯ_Modification_ಸಮಯ()),
      User(ಸ್ಥಿತಿ_2.getUser()), ಗುಂಪು(ಸ್ಥಿತಿ_2.get_ಗುಂಪು()), ಗಾತ್ರ_2(ಸ್ಥಿತಿ_2.get_ಗಾತ್ರ()),
      Type(ಸ್ಥಿತಿ_2.type()), Perms(ಸ್ಥಿತಿ_2.permissions()) {}

ಸ್ಥಿತಿ_2::ಸ್ಥಿತಿ_2(ಸ್ಥಿರಾಂಕ Twine &Name, UniqueID UID, sys::ಸಮಯ_ಬಿಂದು<> MTime,
               uint32_t User, uint32_t ಗುಂಪು, uint64_t ಗಾತ್ರ_2, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), ಗುಂಪು(ಗುಂಪು),
      ಗಾತ್ರ_2(ಗಾತ್ರ_2), Type(Type), Perms(Perms) {}

ಸ್ಥಿತಿ_2 ಸ್ಥಿತಿ_2::copy_With_New_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಸ್ಥಿತಿ_2 &ಒಳ_2, uint64_t New_ಗಾತ್ರ) {
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2(ಒಳ_2.getName(), ಒಳ_2.getUniqueID(), ಒಳ_2.get_ಕೊನೆಯ_Modification_ಸಮಯ(),
                ಒಳ_2.getUser(), ಒಳ_2.get_ಗುಂಪು(), New_ಗಾತ್ರ, ಒಳ_2.getType(),
                ಒಳ_2.getPermissions());
}

ಸ್ಥಿತಿ_2 ಸ್ಥಿತಿ_2::copyWithNewName(ಸ್ಥಿರಾಂಕ ಸ್ಥಿತಿ_2 &ಒಳ_2, ಸ್ಥಿರಾಂಕ Twine &NewName) {
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2(NewName, ಒಳ_2.getUniqueID(), ಒಳ_2.get_ಕೊನೆಯ_Modification_ಸಮಯ(),
                ಒಳ_2.getUser(), ಒಳ_2.get_ಗುಂಪು(), ಒಳ_2.get_ಗಾತ್ರ(), ಒಳ_2.getType(),
                ಒಳ_2.getPermissions());
}

ಸ್ಥಿತಿ_2 ಸ್ಥಿತಿ_2::copyWithNewName(ಸ್ಥಿರಾಂಕ file_ಸ್ಥಿತಿ &ಒಳ_2, ಸ್ಥಿರಾಂಕ Twine &NewName) {
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2(NewName, ಒಳ_2.getUniqueID(), ಒಳ_2.get_ಕೊನೆಯ_Modification_ಸಮಯ(),
                ಒಳ_2.getUser(), ಒಳ_2.get_ಗುಂಪು(), ಒಳ_2.get_ಗಾತ್ರ(), ಒಳ_2.type(),
                ಒಳ_2.permissions());
}

ತಾರ್ಕಿಕ ಸ್ಥಿತಿ_2::equivalent(ಸ್ಥಿರಾಂಕ ಸ್ಥಿತಿ_2 &Other) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(is_ಸ್ಥಿತಿ_ತಿಳಿದ() && Other.is_ಸ್ಥಿತಿ_ತಿಳಿದ());
  ಹಿಂತಿರುಗಿ getUniqueID() == Other.getUniqueID();
}

ತಾರ್ಕಿಕ ಸ್ಥಿತಿ_2::isDirectory() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Type == file_type::directory_file; }

ತಾರ್ಕಿಕ ಸ್ಥಿತಿ_2::isRegularFile() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Type == file_type::regular_file; }

ತಾರ್ಕಿಕ ಸ್ಥಿತಿ_2::isOther() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

ತಾರ್ಕಿಕ ಸ್ಥಿತಿ_2::isSymlink() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Type == file_type::symlink_file; }

ತಾರ್ಕಿಕ ಸ್ಥಿತಿ_2::is_ಸ್ಥಿತಿ_ತಿಳಿದ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Type != file_type::ಸ್ಥಿತಿ_error; }

ತಾರ್ಕಿಕ ಸ್ಥಿತಿ_2::exists() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ is_ಸ್ಥಿತಿ_ತಿಳಿದ() && Type != file_type::file_not_ಸಿಕ್ಕಿತು;
}

File::~File() = ಪೂರ್ವನಿಯೋಜಿತ;

FileSystem::~FileSystem() = ಪೂರ್ವನಿಯೋಜಿತ;

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>>
FileSystem::get_ಬಫರ್_For_File(ಸ್ಥಿರಾಂಕ llvm::Twine &Name, int64_t File_ಗಾತ್ರ,
                             ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile,
                             ತಾರ್ಕಿಕ Is_ಪಠ್ಯ) {
  ಸ್ವಯಂಚಾಲಿತ F = Is_ಪಠ್ಯ ? openFileForRead(Name) : open_File_For_Read_ದ್ವಿಮಾನ(Name);
  ಆದರೆ (!F)
    ಹಿಂತಿರುಗಿ F.getError();

  ಹಿಂತಿರುಗಿ (*F)->get_ಬಫರ್(Name, File_ಗಾತ್ರ, RequiresNullTerminator, IsVolatile);
}

std::error_ಸಂಕೇತ FileSystem::makeAbsolute(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (llvm::sys::ಮಾರ್ಗ_2::is_absolute(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ {};

  ಸ್ವಯಂಚಾಲಿತ WorkingDir = getCurrentWorkingDirectory();
  ಆದರೆ (!WorkingDir)
    ಹಿಂತಿರುಗಿ WorkingDir.getError();

  sys::ಮಾರ್ಗ_2::make_absolute(WorkingDir.get(), ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ {};
}

std::error_ಸಂಕೇತ FileSystem::get_Real_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ,
                                        Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Output) {
  ಹಿಂತಿರುಗಿ errc::operation_not_permitted;
}

std::error_ಸಂಕೇತ FileSystem::isLocal(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ) {
  ಹಿಂತಿರುಗಿ errc::operation_not_permitted;
}

ತಾರ್ಕಿಕ FileSystem::exists(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 && ಸ್ಥಿತಿ_2->exists();
}

llvm::ErrorOr<ತಾರ್ಕಿಕ> FileSystem::equivalent(ಸ್ಥಿರಾಂಕ Twine &A, ಸ್ಥಿರಾಂಕ Twine &B) {
  ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಿತಿ_A = ಸ್ಥಿತಿ_3(A);
  ಆದರೆ (!ಸ್ಥಿತಿ_A)
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_A.getError();
  ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಿತಿ_B = ಸ್ಥಿತಿ_3(B);
  ಆದರೆ (!ಸ್ಥಿತಿ_B)
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_B.getError();
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_A->equivalent(*ಸ್ಥಿತಿ_B);
}

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
ಶೂನ್ಯ FileSystem::dump() ಸ್ಥಿರಾಂಕ { print(dbgs(), PrintType::RecursiveContents); }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
ಸ್ಥಿರ ತಾರ್ಕಿಕ isTraversalComponent(StringRef Component) {
  ಹಿಂತಿರುಗಿ Component == ".." || Component == ".";
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ ಮಾರ್ಗ_Has_Traversal(StringRef ಮಾರ್ಗ) {
  ಬಳಸಿ ನಾಮಾಕಾಶ llvm::sys;

  ಗಾಗಿ (StringRef Comp : llvm::make_range(ಮಾರ್ಗ_2::begin(ಮಾರ್ಗ), ಮಾರ್ಗ_2::ಕೊನೆ(ಮಾರ್ಗ)))
    ಆದರೆ (isTraversalComponent(Comp))
      ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}
#ಆದರೆ_ಅಂತ್ಯ

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

ನಾಮಾಕಾಶ {

/// Wrapper around a raw file descriptor.
ವರ್ಗ RealFile : ಸಾರ್ವಜನಿಕ File {
  ಮಿತ್ರ ವರ್ಗ RealFileSystem;

  file_t FD;
  ಸ್ಥಿತಿ_2 S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_ಮಾರ್ಗ_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::ಸ್ಥಿತಿ_error, {}),
        RealName(New_Real_ಮಾರ್ಗ_Name.str()) {
    ಪ್ರತಿಜ್ಞಾ(FD.isValid() && "Invalid or inactive file descriptor");
  }

ಸಾರ್ವಜನಿಕ:
  ~RealFile() override;

  ErrorOr<ಸ್ಥಿತಿ_2> ಸ್ಥಿತಿ_3() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>> get_ಬಫರ್(ಸ್ಥಿರಾಂಕ Twine &Name,
                                                   int64_t File_ಗಾತ್ರ,
                                                   ತಾರ್ಕಿಕ RequiresNullTerminator,
                                                   ತಾರ್ಕಿಕ IsVolatile) override;
  std::error_ಸಂಕೇತ close() override;
  ಶೂನ್ಯ ಗಣ_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<ಸ್ಥಿತಿ_2> RealFile::ಸ್ಥಿತಿ_3() {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ(FD.isValid() && "cannot stat closed file");
  ಆದರೆ (!S.is_ಸ್ಥಿತಿ_ತಿಳಿದ()) {
    file_ಸ್ಥಿತಿ Real_ಸ್ಥಿತಿ;
    ಆದರೆ (std::error_ಸಂಕೇತ EC = sys::fs::ಸ್ಥಿತಿ_3(FD, Real_ಸ್ಥಿತಿ))
      ಹಿಂತಿರುಗಿ EC;
    S = ಸ್ಥಿತಿ_2::copyWithNewName(Real_ಸ್ಥಿತಿ, S.getName());
  }
  ಹಿಂತಿರುಗಿ S;
}

ErrorOr<std::string> RealFile::getName() {
  ಹಿಂತಿರುಗಿ RealName.ಖಾಲಿಗಳು() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್>>
RealFile::get_ಬಫರ್(ಸ್ಥಿರಾಂಕ Twine &Name, int64_t File_ಗಾತ್ರ,
                    ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ(FD.isValid() && "cannot get buffer for closed file");
  ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ಬಫರ್::getOpenFile(FD, Name, File_ಗಾತ್ರ, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_ಸಂಕೇತ RealFile::close() {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  std::error_ಸಂಕೇತ EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ಹಿಂತಿರುಗಿ EC;
}

ಶೂನ್ಯ RealFile::ಗಣ_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  RealName = ಮಾರ್ಗ.str();
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_3())
    S = ಸ್ಥಿತಿ_2.get().copyWithNewName(ಸ್ಥಿತಿ_2.get(), ಮಾರ್ಗ);
}

ನಾಮಾಕಾಶ {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
ವರ್ಗ RealFileSystem : ಸಾರ್ವಜನಿಕ FileSystem {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ RealFileSystem(ತಾರ್ಕಿಕ LinkCWDToProcess) {
    ಆದರೆ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      ಆದರೆ (std::error_ಸಂಕೇತ EC = llvm::sys::fs::current_ಮಾರ್ಗ(PWD))
        WD = std::move(EC);
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (llvm::sys::fs::real_ಮಾರ್ಗ(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      ಇಲ್ಲದಿದ್ದರೆ
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<ಸ್ಥಿತಿ_2> ಸ್ಥಿತಿ_3(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ದ್ವಿಮಾನ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) override;
  directory_iterator dir_begin(ಸ್ಥಿರಾಂಕ Twine &Dir, std::error_ಸಂಕೇತ &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ಸ್ಥಿರಾಂಕ override;
  std::error_ಸಂಕೇತ ಗಣ_Current_Working_Directory(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) override;
  std::error_ಸಂಕೇತ isLocal(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ) override;
  ಶೂನ್ಯ
  getDirectoryContentRealSources(ಸ್ಥಿರಾಂಕ Twine &Dir,
                                 Small_ಸದಿಶ_Impl<std::string> &ಹೊರಹೋಗು) override {
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(Dir.str());
  }
  std::error_ಸಂಕೇತ get_Real_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ,
                              Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Output) override;

ಸಂರಕ್ಷಿತ:
  ಶೂನ್ಯ printImpl(raw_ostream &OS, PrintType Type,
                 ಚಿಹ್ನೆರಹಿತ IndentLevel) ಸ್ಥಿರಾಂಕ override;

ಖಾಸಗಿ:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage) ಸ್ಥಿರಾಂಕ {
    ಆದರೆ (!WD || !*WD)
      ಹಿಂತಿರುಗಿ ಮಾರ್ಗ;
    ಮಾರ್ಗ.ಗಮ್ಯ_ಸದಿಶ(Storage);
    sys::ಮಾರ್ಗ_2::make_absolute(WD->get().Resolved, Storage);
    ಹಿಂತಿರುಗಿ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ಸ್ಥಿರಾಂಕ Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_ಮಾರ್ಗ(Name, Storage), Flags, &RealName);
    ಆದರೆ (!FDOrErr)
      ಹಿಂತಿರುಗಿ error_ಗಮ್ಯ_Error_ಸಂಕೇತ(FDOrErr.takeError());
    ಹಿಂತಿರುಗಿ std::unique_ptr<File>(
        ಹೊಸ RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  ರಚನೆ WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<ಸ್ಥಿತಿ_2> RealFileSystem::ಸ್ಥಿತಿ_3(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_ಸ್ಥಿತಿ Real_ಸ್ಥಿತಿ;
  ಆದರೆ (std::error_ಸಂಕೇತ EC =
          sys::fs::ಸ್ಥಿತಿ_3(adjust_ಮಾರ್ಗ(ಮಾರ್ಗ, Storage), Real_ಸ್ಥಿತಿ))
    ಹಿಂತಿರುಗಿ EC;
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2::copyWithNewName(Real_ಸ್ಥಿತಿ, ಮಾರ್ಗ);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ಸ್ಥಿರಾಂಕ Twine &Name) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಹಿಂತಿರುಗಿ openFileForReadWithFlags(Name, sys::fs::ದ_ಪಠ್ಯ);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ದ್ವಿಮಾನ(ಸ್ಥಿರಾಂಕ Twine &Name) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಹಿಂತಿರುಗಿ openFileForReadWithFlags(Name, sys::fs::ದ_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಆದರೆ (WD && *WD)
    ಹಿಂತಿರುಗಿ std::string(WD->get().Specified);
  ಆದರೆ (WD)
    ಹಿಂತಿರುಗಿ WD->getError();

  SmallString<128> Dir;
  ಆದರೆ (std::error_ಸಂಕೇತ EC = llvm::sys::fs::current_ಮಾರ್ಗ(Dir))
    ಹಿಂತಿರುಗಿ EC;
  ಹಿಂತಿರುಗಿ std::string(Dir);
}

std::error_ಸಂಕೇತ RealFileSystem::ಗಣ_Current_Working_Directory(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಆದರೆ (!WD)
    ಹಿಂತಿರುಗಿ llvm::sys::fs::ಗಣ_current_ಮಾರ್ಗ(ಮಾರ್ಗ);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_ಮಾರ್ಗ(ಮಾರ್ಗ, Storage).ಗಮ್ಯ_ಸದಿಶ(Absolute);
  ತಾರ್ಕಿಕ IsDir;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ಹಿಂತಿರುಗಿ Err;
  ಆದರೆ (!IsDir)
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::not_a_directory);
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = llvm::sys::fs::real_ಮಾರ್ಗ(Absolute, Resolved))
    ಹಿಂತಿರುಗಿ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

std::error_ಸಂಕೇತ RealFileSystem::isLocal(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ಹಿಂತಿರುಗಿ llvm::sys::fs::is_local(adjust_ಮಾರ್ಗ(ಮಾರ್ಗ, Storage), ಫಲಿತಾಂಶ);
}

std::error_ಸಂಕೇತ RealFileSystem::get_Real_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ,
                                            Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Output) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ಹಿಂತಿರುಗಿ llvm::sys::fs::real_ಮಾರ್ಗ(adjust_ಮಾರ್ಗ(ಮಾರ್ಗ, Storage), Output);
}

ಶೂನ್ಯ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               ಚಿಹ್ನೆರಹಿತ IndentLevel) ಸ್ಥಿರಾಂಕ {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  ಆದರೆ (WD)
    OS << "own";
  ಇಲ್ಲದಿದ್ದರೆ
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  ಸ್ಥಿರ IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(ನಿಜ);
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ಹಿಂತಿರುಗಿ std::make_unique<RealFileSystem>(ಸುಳ್ಳು);
}

ನಾಮಾಕಾಶ {

ವರ್ಗ RealFSDirIter : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

ಸಾರ್ವಜನಿಕ:
  RealFSDirIter(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, std::error_ಸಂಕೇತ &EC) {
    ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(ಮಾರ್ಗ, EC);
    ಆದರೆ (Iter != sys::fs::directory_iterator())
      Current_ನಮೂದು = directory_ನಮೂದು(Iter->ಮಾರ್ಗ_2(), Iter->type());
  }

  std::error_ಸಂಕೇತ increment() override {
    ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

    std::error_ಸಂಕೇತ EC;
    Iter.increment(EC);
    Current_ನಮೂದು = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_ನಮೂದು()
                       : directory_ನಮೂದು(Iter->ಮಾರ್ಗ_2(), Iter->type());
    ಹಿಂತಿರುಗಿ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ಸ್ಥಿರಾಂಕ Twine &Dir,
                                             std::error_ಸಂಕೇತ &EC) {
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ಹಿಂತಿರುಗಿ directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_ಮಾರ್ಗ(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> ತಳ_FS) {
  FSList.ತಳ್ಳು_ಹಿಂಭಾಗ(std::move(ತಳ_FS));
}

ಶೂನ್ಯ OverlayFileSystem::ತಳ್ಳು_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ತಳ್ಳು_ಹಿಂಭಾಗ(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->ಗಣ_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<ಸ್ಥಿತಿ_2> OverlayFileSystem::ಸ್ಥಿತಿ_3(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  // FIXME: handle symlinks that cross file systems
  ಗಾಗಿ (iterator I = overlays_begin(), E = overlays_ಕೊನೆ(); I != E; ++I) {
    ErrorOr<ಸ್ಥಿತಿ_2> ಸ್ಥಿತಿ_2 = (*I)->ಸ್ಥಿತಿ_3(ಮಾರ್ಗ);
    ಆದರೆ (ಸ್ಥಿತಿ_2 || ಸ್ಥಿತಿ_2.getError() != llvm::errc::no_such_file_or_directory)
      ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
  }
  ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::no_such_file_or_directory);
}

ತಾರ್ಕಿಕ OverlayFileSystem::exists(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  // FIXME: handle symlinks that cross file systems
  ಗಾಗಿ (iterator I = overlays_begin(), E = overlays_ಕೊನೆ(); I != E; ++I) {
    ಆದರೆ ((*I)->exists(ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ಸ್ಥಿರಾಂಕ llvm::Twine &ಮಾರ್ಗ) {
  // FIXME: handle symlinks that cross file systems
  ಗಾಗಿ (iterator I = overlays_begin(), E = overlays_ಕೊನೆ(); I != E; ++I) {
    ಸ್ವಯಂಚಾಲಿತ ಫಲಿತಾಂಶ = (*I)->openFileForRead(ಮಾರ್ಗ);
    ಆದರೆ (ಫಲಿತಾಂಶ || ಫಲಿತಾಂಶ.getError() != llvm::errc::no_such_file_or_directory)
      ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  }
  ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ಸ್ಥಿರಾಂಕ {
  // All file systems are synchronized, just take the first working directory.
  ಹಿಂತಿರುಗಿ FSList.front()->getCurrentWorkingDirectory();
}

std::error_ಸಂಕೇತ
OverlayFileSystem::ಗಣ_Current_Working_Directory(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &FS : FSList)
    ಆದರೆ (std::error_ಸಂಕೇತ EC = FS->ಗಣ_Current_Working_Directory(ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ EC;
  ಹಿಂತಿರುಗಿ {};
}

std::error_ಸಂಕೇತ OverlayFileSystem::isLocal(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ) {
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &FS : FSList)
    ಆದರೆ (FS->exists(ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ FS->isLocal(ಮಾರ್ಗ, ಫಲಿತಾಂಶ);
  ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;
}

std::error_ಸಂಕೇತ OverlayFileSystem::get_Real_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ,
                                               Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Output) {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &FS : FSList)
    ಆದರೆ (FS->exists(ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ FS->get_Real_ಮಾರ್ಗ(ಮಾರ್ಗ, Output);
  ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;
}

ಶೂನ್ಯ OverlayFileSystem::getDirectoryContentRealSources(
    ಸ್ಥಿರಾಂಕ Twine &Dir, Small_ಸದಿಶ_Impl<std::string> &ಹೊರಹೋಗು) {
  // All layers contribute.
  ಗಾಗಿ (iterator I = overlays_begin(), E = overlays_ಕೊನೆ(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, ಹೊರಹೋಗು);
}

ಶೂನ್ಯ OverlayFileSystem::ಭೇಟಿ_ಮಗು_File_Systems(ಭೇಟಿ_Callback_Ty Callback) {
  ಗಾಗಿ (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->ಭೇಟಿ_ಮಗು_File_Systems(Callback);
  }
}

ಶೂನ್ಯ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  ಚಿಹ್ನೆರಹಿತ IndentLevel) ಸ್ಥಿರಾಂಕ {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  ಆದರೆ (Type == PrintType::Summary)
    ಹಿಂತಿರುಗಿ;

  ಆದರೆ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = ಪೂರ್ವನಿಯೋಜಿತ;

ನಾಮಾಕಾಶ {

/// Combines and deduplicates directory entries across multiple file systems.
ವರ್ಗ CombiningDirIterImpl : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  ಬಳಸಿ FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_ಸದಿಶ<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_ಗಣ<> ನೋಡಿದ_ಹೆಸರುಗಳು;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_ಸಂಕೇತ incrementIter(ತಾರ್ಕಿಕ Is_ಮೊದಲ_ಸಮಯ) {
    ಆಗ (!IterList.ಖಾಲಿಗಳು()) {
      CurrentDirIter = IterList.ಹಿಂಭಾಗ();
      IterList.ತೆಗೆ_ಹಿಂಭಾಗ();
      ಆದರೆ (CurrentDirIter != directory_iterator())
        ನಿಲ್ಲಿಸು; //ಸಿಕ್ಕಿತು
    }

    ಆದರೆ (Is_ಮೊದಲ_ಸಮಯ && CurrentDirIter == directory_iterator())
      ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;
    ಹಿಂತಿರುಗಿ {};
  }

  std::error_ಸಂಕೇತ incrementDirIter(ತಾರ್ಕಿಕ Is_ಮೊದಲ_ಸಮಯ) {
    ಪ್ರತಿಜ್ಞಾ((Is_ಮೊದಲ_ಸಮಯ || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_ಸಂಕೇತ EC;
    ಆದರೆ (!Is_ಮೊದಲ_ಸಮಯ)
      CurrentDirIter.increment(EC);
    ಆದರೆ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(Is_ಮೊದಲ_ಸಮಯ);
    ಹಿಂತಿರುಗಿ EC;
  }

  std::error_ಸಂಕೇತ incrementImpl(ತಾರ್ಕಿಕ Is_ಮೊದಲ_ಸಮಯ) {
    ಆಗ (ನಿಜ) {
      std::error_ಸಂಕೇತ EC = incrementDirIter(Is_ಮೊದಲ_ಸಮಯ);
      ಆದರೆ (EC || CurrentDirIter == directory_iterator()) {
        Current_ನಮೂದು = directory_ನಮೂದು();
        ಹಿಂತಿರುಗಿ EC;
      }
      Current_ನಮೂದು = *CurrentDirIter;
      StringRef Name = llvm::sys::ಮಾರ್ಗ_2::filename(Current_ನಮೂದು.ಮಾರ್ಗ_2());
      ಆದರೆ (ನೋಡಿದ_ಹೆಸರುಗಳು.ಸೇರಿಸು(Name).ಎರಡನೇ_2)
        ಹಿಂತಿರುಗಿ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

ಸಾರ್ವಜನಿಕ:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_ಸಂಕೇತ &EC) {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &FS : FileSystems) {
      std::error_ಸಂಕೇತ FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      ಆದರೆ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ಹಿಂತಿರುಗಿ;
      }
      ಆದರೆ (!FEC)
        IterList.ತಳ್ಳು_ಹಿಂಭಾಗ(Iter);
    }
    EC = incrementImpl(ನಿಜ);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_ಸಂಕೇತ &EC)
      : IterList(DirIters) {
    EC = incrementImpl(ನಿಜ);
  }

  std::error_ಸಂಕೇತ increment() override { ಹಿಂತಿರುಗಿ incrementImpl(ಸುಳ್ಳು); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ಸ್ಥಿರಾಂಕ Twine &Dir,
                                                std::error_ಸಂಕೇತ &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ {};
  ಹಿಂತಿರುಗಿ Combined;
}

ಶೂನ್ಯ ProxyFileSystem::anchor() {}

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ vfs {

ನಾಮಾಕಾಶ detail {

ಎಣಿಕೆ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_ಬಗೆ {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
ವರ್ಗ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು {
  ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_ಬಗೆ ಬಗೆ_2;
  std::string FileName;

ಸಾರ್ವಜನಿಕ:
  ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು(llvm::StringRef FileName, ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_ಬಗೆ ಬಗೆ_2)
      : ಬಗೆ_2(ಬಗೆ_2), FileName(std::string(llvm::sys::ಮಾರ್ಗ_2::filename(FileName))) {
  }
  ವಾಸ್ತವಿಕ ~ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು() = ಪೂರ್ವನಿಯೋಜಿತ;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  ವಾಸ್ತವಿಕ ಸ್ಥಿತಿ_2 get_ಸ್ಥಿತಿ(ಸ್ಥಿರಾಂಕ Twine &RequestedName) ಸ್ಥಿರಾಂಕ = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ FileName; }
  ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_ಬಗೆ get_ಬಗೆ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಬಗೆ_2; }
  ವಾಸ್ತವಿಕ std::string ಗಮ್ಯ_String(ಚಿಹ್ನೆರಹಿತ Indent) ಸ್ಥಿರಾಂಕ = 0;
};

ವರ್ಗ ಒಳ_ಸ್ಮರಣೆ_File : ಸಾರ್ವಜನಿಕ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು {
  ಸ್ಥಿತಿ_2 Stat;
  std::unique_ptr<llvm::ಸ್ಮರಣೆ_ಬಫರ್> ಬಫರ್;

ಸಾರ್ವಜನಿಕ:
  ಒಳ_ಸ್ಮರಣೆ_File(ಸ್ಥಿತಿ_2 Stat, std::unique_ptr<llvm::ಸ್ಮರಣೆ_ಬಫರ್> ಬಫರ್)
      : ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು(Stat.getName(), IME_File), Stat(std::move(Stat)),
        ಬಫರ್(std::move(ಬಫರ್)) {}

  ಸ್ಥಿತಿ_2 get_ಸ್ಥಿತಿ(ಸ್ಥಿರಾಂಕ Twine &RequestedName) ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2::copyWithNewName(Stat, RequestedName);
  }
  llvm::ಸ್ಮರಣೆ_ಬಫರ್ *get_ಬಫರ್() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಬಫರ್.get(); }

  std::string ಗಮ್ಯ_String(ಚಿಹ್ನೆರಹಿತ Indent) ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *N) {
    ಹಿಂತಿರುಗಿ N->get_ಬಗೆ() == IME_File;
  }
};

ನಾಮಾಕಾಶ {

ವರ್ಗ ಒಳ_ಸ್ಮರಣೆ_Hard_Link : ಸಾರ್ವಜನಿಕ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು {
  ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_File &ResolvedFile;

ಸಾರ್ವಜನಿಕ:
  ಒಳ_ಸ್ಮರಣೆ_Hard_Link(StringRef ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_File &ResolvedFile)
      : ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು(ಮಾರ್ಗ, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_File &getResolvedFile() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ResolvedFile; }

  ಸ್ಥಿತಿ_2 get_ಸ್ಥಿತಿ(ಸ್ಥಿರಾಂಕ Twine &RequestedName) ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ ResolvedFile.get_ಸ್ಥಿತಿ(RequestedName);
  }

  std::string ಗಮ್ಯ_String(ಚಿಹ್ನೆರಹಿತ Indent) ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.ಗಮ್ಯ_String(0);
  }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *N) {
    ಹಿಂತಿರುಗಿ N->get_ಬಗೆ() == IME_HardLink;
  }
};

ವರ್ಗ ಒಳ_ಸ್ಮರಣೆ_Symbolic_Link : ಸಾರ್ವಜನಿಕ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು {
  std::string ಗುರಿ_ಮಾರ್ಗ;
  ಸ್ಥಿತಿ_2 Stat;

ಸಾರ್ವಜನಿಕ:
  ಒಳ_ಸ್ಮರಣೆ_Symbolic_Link(StringRef ಮಾರ್ಗ, StringRef ಗುರಿ_ಮಾರ್ಗ, ಸ್ಥಿತಿ_2 Stat)
      : ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು(ಮಾರ್ಗ, IME_SymbolicLink), ಗುರಿ_ಮಾರ್ಗ(std::move(ಗುರಿ_ಮಾರ್ಗ)),
        Stat(Stat) {}

  std::string ಗಮ್ಯ_String(ಚಿಹ್ನೆರಹಿತ Indent) ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ std::string(Indent, ' ') + "SymbolicLink to -> " + ಗುರಿ_ಮಾರ್ಗ;
  }

  ಸ್ಥಿತಿ_2 get_ಸ್ಥಿತಿ(ಸ್ಥಿರಾಂಕ Twine &RequestedName) ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_ಗುರಿ_ಮಾರ್ಗ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಗುರಿ_ಮಾರ್ಗ; }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *N) {
    ಹಿಂತಿರುಗಿ N->get_ಬಗೆ() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
ವರ್ಗ ಒಳ_ಸ್ಮರಣೆ_File_Adaptor : ಸಾರ್ವಜನಿಕ File {
  ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_File &ಗಂಟು;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

ಸಾರ್ವಜನಿಕ:
  ಸ್ಪಷ್ಟ ಒಳ_ಸ್ಮರಣೆ_File_Adaptor(ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_File &ಗಂಟು,
                               std::string RequestedName)
      : ಗಂಟು(ಗಂಟು), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<ಸ್ಥಿತಿ_2> ಸ್ಥಿತಿ_3() override {
    ಹಿಂತಿರುಗಿ ಗಂಟು.get_ಸ್ಥಿತಿ(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::ಸ್ಮರಣೆ_ಬಫರ್>>
  get_ಬಫರ್(ಸ್ಥಿರಾಂಕ Twine &Name, int64_t File_ಗಾತ್ರ, ತಾರ್ಕಿಕ RequiresNullTerminator,
            ತಾರ್ಕಿಕ IsVolatile) override {
    llvm::ಸ್ಮರಣೆ_ಬಫರ್ *Buf = ಗಂಟು.get_ಬಫರ್();
    ಹಿಂತಿರುಗಿ llvm::ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್(
        Buf->get_ಬಫರ್(), Buf->get_ಬಫರ್_Identifier(), RequiresNullTerminator);
  }

  std::error_ಸಂಕೇತ close() override { ಹಿಂತಿರುಗಿ {}; }

  ಶೂನ್ಯ ಗಣ_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) override { RequestedName = ಮಾರ್ಗ.str(); }
};
} // namespace

ವರ್ಗ ಒಳ_ಸ್ಮರಣೆ_Directory : ಸಾರ್ವಜನಿಕ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು {
  ಸ್ಥಿತಿ_2 Stat;
  std::map<std::string, std::unique_ptr<ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು>, std::less<>> Entries;

ಸಾರ್ವಜನಿಕ:
  ಒಳ_ಸ್ಮರಣೆ_Directory(ಸ್ಥಿತಿ_2 Stat)
      : ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  ಸ್ಥಿತಿ_2 get_ಸ್ಥಿತಿ(ಸ್ಥಿರಾಂಕ Twine &RequestedName) ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Stat.getUniqueID(); }

  ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *get_ಮಗು(StringRef Name) ಸ್ಥಿರಾಂಕ {
    ಸ್ವಯಂಚಾಲಿತ I = Entries.ಕಂಡುಹಿಡಿ(Name);
    ಆದರೆ (I != Entries.ಕೊನೆ())
      ಹಿಂತಿರುಗಿ I->ಎರಡನೇ_2.get();
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }

  ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *ಕೂಡಿಸು_ಮಗು(StringRef Name, std::unique_ptr<ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು> ಮಗು) {
    ಹಿಂತಿರುಗಿ Entries.emplace(Name, std::move(ಮಗು)).ಮೊದಲ_2->ಎರಡನೇ_2.get();
  }

  ಬಳಸಿ const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Entries.begin(); }
  const_iterator ಕೊನೆ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Entries.ಕೊನೆ(); }

  std::string ಗಮ್ಯ_String(ಚಿಹ್ನೆರಹಿತ Indent) ಸ್ಥಿರಾಂಕ override {
    std::string ಫಲಿತಾಂಶ =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &ನಮೂದು : Entries)
      ಫಲಿತಾಂಶ += ನಮೂದು.ಎರಡನೇ_2->ಗಮ್ಯ_String(Indent + 2);
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *N) {
    ಹಿಂತಿರುಗಿ N->get_ಬಗೆ() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ಸ್ಥಿರ sys::fs::UniqueID getUniqueID(hash_ಸಂಕೇತ Hash) {
  ಹಿಂತಿರುಗಿ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ಸ್ಥಿರ sys::fs::UniqueID getFileID(sys::fs::UniqueID ಪೋಷಕ_2,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ಹಿಂತಿರುಗಿ getUniqueID(llvm::hash_combine(ಪೋಷಕ_2.getFile(), Name, Contents));
}
ಸ್ಥಿರ sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID ಪೋಷಕ_2,
                                        llvm::StringRef Name) {
  ಹಿಂತಿರುಗಿ getUniqueID(llvm::hash_combine(ಪೋಷಕ_2.getFile(), Name));
}

ಸ್ಥಿತಿ_2 detail::New_ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_Info::make_ಸ್ಥಿತಿ() ಸ್ಥಿರಾಂಕ {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, ಬಫರ್ ? ಬಫರ್->get_ಬಫರ್() : "");

  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2(ಮಾರ್ಗ, UID, llvm::sys::ಗಮ್ಯ_ಸಮಯ_ಬಿಂದು(Modification_ಸಮಯ), User,
                ಗುಂಪು, ಬಫರ್ ? ಬಫರ್->get_ಬಫರ್_ಗಾತ್ರ() : 0, Type, Perms);
}

ಒಳ_ಸ್ಮರಣೆ_File_System::ಒಳ_ಸ್ಮರಣೆ_File_System(ತಾರ್ಕಿಕ UseNormalizedPaths)
    : ಬೇರು_2(ಹೊಸ detail::ಒಳ_ಸ್ಮರಣೆ_Directory(
          ಸ್ಥಿತಿ_2("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::ಸಮಯ_ಬಿಂದು<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

ಒಳ_ಸ್ಮರಣೆ_File_System::~ಒಳ_ಸ್ಮರಣೆ_File_System() = ಪೂರ್ವನಿಯೋಜಿತ;

std::string ಒಳ_ಸ್ಮರಣೆ_File_System::ಗಮ್ಯ_String() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ಬೇರು_2->ಗಮ್ಯ_String(/*Indent=*/0);
}

ತಾರ್ಕಿಕ ಒಳ_ಸ್ಮರಣೆ_File_System::ಕೂಡಿಸು_File(ಸ್ಥಿರಾಂಕ Twine &P, time_t Modification_ಸಮಯ,
                                 std::unique_ptr<llvm::ಸ್ಮರಣೆ_ಬಫರ್> ಬಫರ್,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> ಗುಂಪು,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_ಗಂಟು_Fn Make_ಗಂಟು) {
  SmallString<128> ಮಾರ್ಗ;
  P.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ಸಂಕೇತ EC = makeAbsolute(ಮಾರ್ಗ);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  (ಶೂನ್ಯ)EC;

  ಆದರೆ (useNormalizedPaths())
    llvm::sys::ಮಾರ್ಗ_2::remove_dots(ಮಾರ್ಗ, /*remove_dot_dot=*/ನಿಜ);

  ಆದರೆ (ಮಾರ್ಗ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  detail::ಒಳ_ಸ್ಮರಣೆ_Directory *Dir = ಬೇರು_2.get();
  ಸ್ವಯಂಚಾಲಿತ I = llvm::sys::ಮಾರ್ಗ_2::begin(ಮಾರ್ಗ), E = sys::ಮಾರ್ಗ_2::ಕೊನೆ(ಮಾರ್ಗ);
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ ResolvedUser = User.ಮೌಲ್ಯ_or(0);
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ Resolved_ಗುಂಪು = ಗುಂಪು.ಮೌಲ್ಯ_or(0);
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ ResolvedType = Type.ಮೌಲ್ಯ_or(sys::fs::file_type::regular_file);
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ ResolvedPerms = Perms.ಮೌಲ್ಯ_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  ಆಗ (ನಿಜ) {
    Name = *I;
    ++I;
    ಆದರೆ (I == E)
      ನಿಲ್ಲಿಸು;
    detail::ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *ಗಂಟು = Dir->get_ಮಗು(Name);
    ಆದರೆ (!ಗಂಟು) {
      // This isn't the last element, so we create a new directory.
      ಸ್ಥಿತಿ_2 Stat(
          StringRef(ಮಾರ್ಗ.str().begin(), Name.ಕೊನೆ() - ಮಾರ್ಗ.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::ಗಮ್ಯ_ಸಮಯ_ಬಿಂದು(Modification_ಸಮಯ), ResolvedUser, Resolved_ಗುಂಪು,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(Dir->ಕೂಡಿಸು_ಮಗು(
          Name, std::make_unique<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(std::move(Stat))));
      ಮುಂದುವರಿ;
    }
    // Creating file under another file.
    ಆದರೆ (!isa<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(ಗಂಟು))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    Dir = cast<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(ಗಂಟು);
  }
  detail::ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *ಗಂಟು = Dir->get_ಮಗು(Name);
  ಆದರೆ (!ಗಂಟು) {
    Dir->ಕೂಡಿಸು_ಮಗು(Name,
                  Make_ಗಂಟು({Dir->getUniqueID(), ಮಾರ್ಗ, Name, Modification_ಸಮಯ,
                            std::move(ಬಫರ್), ResolvedUser, Resolved_ಗುಂಪು,
                            ResolvedType, ResolvedPerms}));
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಆದರೆ (isa<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(ಗಂಟು))
    ಹಿಂತಿರುಗಿ ResolvedType == sys::fs::file_type::directory_file;

  ಪ್ರತಿಜ್ಞಾ((isa<detail::ಒಳ_ಸ್ಮರಣೆ_File>(ಗಂಟು) ||
          isa<detail::ಒಳ_ಸ್ಮರಣೆ_Hard_Link>(ಗಂಟು)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *Link = dyn_cast<detail::ಒಳ_ಸ್ಮರಣೆ_Hard_Link>(ಗಂಟು)) {
    ಹಿಂತಿರುಗಿ Link->getResolvedFile().get_ಬಫರ್()->get_ಬಫರ್() ==
           ಬಫರ್->get_ಬಫರ್();
  }
  ಹಿಂತಿರುಗಿ cast<detail::ಒಳ_ಸ್ಮರಣೆ_File>(ಗಂಟು)->get_ಬಫರ್()->get_ಬಫರ್() ==
         ಬಫರ್->get_ಬಫರ್();
}

ತಾರ್ಕಿಕ ಒಳ_ಸ್ಮರಣೆ_File_System::ಕೂಡಿಸು_File(ಸ್ಥಿರಾಂಕ Twine &P, time_t Modification_ಸಮಯ,
                                 std::unique_ptr<llvm::ಸ್ಮರಣೆ_ಬಫರ್> ಬಫರ್,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> ಗುಂಪು,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_File(P, Modification_ಸಮಯ, std::move(ಬಫರ್), User, ಗುಂಪು, Type,
                 Perms,
                 [](detail::New_ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_Info NNI)
                     -> std::unique_ptr<detail::ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು> {
                   ಸ್ಥಿತಿ_2 Stat = NNI.make_ಸ್ಥಿತಿ();
                   ಆದರೆ (Stat.getType() == sys::fs::file_type::directory_file)
                     ಹಿಂತಿರುಗಿ std::make_unique<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(Stat);
                   ಹಿಂತಿರುಗಿ std::make_unique<detail::ಒಳ_ಸ್ಮರಣೆ_File>(
                       Stat, std::move(NNI.ಬಫರ್));
                 });
}

ತಾರ್ಕಿಕ ಒಳ_ಸ್ಮರಣೆ_File_System::ಕೂಡಿಸು_File_No_Own(
    ಸ್ಥಿರಾಂಕ Twine &P, time_t Modification_ಸಮಯ,
    ಸ್ಥಿರಾಂಕ llvm::ಸ್ಮರಣೆ_ಬಫರ್_Ref &ಬಫರ್, std::optional<uint32_t> User,
    std::optional<uint32_t> ಗುಂಪು, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_File(P, Modification_ಸಮಯ, llvm::ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್(ಬಫರ್),
                 std::move(User), std::move(ಗುಂಪು), std::move(Type),
                 std::move(Perms),
                 [](detail::New_ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_Info NNI)
                     -> std::unique_ptr<detail::ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು> {
                   ಸ್ಥಿತಿ_2 Stat = NNI.make_ಸ್ಥಿತಿ();
                   ಆದರೆ (Stat.getType() == sys::fs::file_type::directory_file)
                     ಹಿಂತಿರುಗಿ std::make_unique<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(Stat);
                   ಹಿಂತಿರುಗಿ std::make_unique<detail::ಒಳ_ಸ್ಮರಣೆ_File>(
                       Stat, std::move(NNI.ಬಫರ್));
                 });
}

detail::Named_ಗಂಟು_Or_Error
ಒಳ_ಸ್ಮರಣೆ_File_System::lookup_ಗಂಟು(ಸ್ಥಿರಾಂಕ Twine &P, ತಾರ್ಕಿಕ FollowFinalSymlink,
                               size_t Symlink_ಆಳ) ಸ್ಥಿರಾಂಕ {
  SmallString<128> ಮಾರ್ಗ;
  P.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ಸಂಕೇತ EC = makeAbsolute(ಮಾರ್ಗ);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  (ಶೂನ್ಯ)EC;

  ಆದರೆ (useNormalizedPaths())
    llvm::sys::ಮಾರ್ಗ_2::remove_dots(ಮಾರ್ಗ, /*remove_dot_dot=*/ನಿಜ);

  ಸ್ಥಿರಾಂಕ detail::ಒಳ_ಸ್ಮರಣೆ_Directory *Dir = ಬೇರು_2.get();
  ಆದರೆ (ಮಾರ್ಗ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ detail::Named_ಗಂಟು_Or_Error(ಮಾರ್ಗ, Dir);

  ಸ್ವಯಂಚಾಲಿತ I = llvm::sys::ಮಾರ್ಗ_2::begin(ಮಾರ್ಗ), E = llvm::sys::ಮಾರ್ಗ_2::ಕೊನೆ(ಮಾರ್ಗ);
  ಆಗ (ನಿಜ) {
    detail::ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು *ಗಂಟು = Dir->get_ಮಗು(*I);
    ++I;
    ಆದರೆ (!ಗಂಟು)
      ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;

    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Symlink = dyn_cast<detail::ಒಳ_ಸ್ಮರಣೆ_Symbolic_Link>(ಗಂಟು)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      ಆದರೆ (I == E && !FollowFinalSymlink)
        ಹಿಂತಿರುಗಿ detail::Named_ಗಂಟು_Or_Error(ಮಾರ್ಗ, Symlink);

      ಆದರೆ (Symlink_ಆಳ > ಒಳ_ಸ್ಮರಣೆ_File_System::Max_Symlink_ಆಳ)
        ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;

      SmallString<128> ಗುರಿ_ಮಾರ್ಗ = Symlink->get_ಗುರಿ_ಮಾರ್ಗ();
      ಆದರೆ (std::error_ಸಂಕೇತ EC = makeAbsolute(ಗುರಿ_ಮಾರ್ಗ))
        ಹಿಂತಿರುಗಿ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      ಸ್ವಯಂಚಾಲಿತ ಗುರಿ_2 =
          lookup_ಗಂಟು(ಗುರಿ_ಮಾರ್ಗ, /*FollowFinalSymlink=*/ನಿಜ, Symlink_ಆಳ + 1);
      ಆದರೆ (!ಗುರಿ_2 || I == E)
        ಹಿಂತಿರುಗಿ ಗುರಿ_2;

      ಆದರೆ (!isa<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(*ಗುರಿ_2))
        ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(*ಗುರಿ_2);
      ಮುಂದುವರಿ;
    }

    // Return the file if it's at the end of the path.
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ File = dyn_cast<detail::ಒಳ_ಸ್ಮರಣೆ_File>(ಗಂಟು)) {
      ಆದರೆ (I == E)
        ಹಿಂತಿರುಗಿ detail::Named_ಗಂಟು_Or_Error(ಮಾರ್ಗ, File);
      ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ File = dyn_cast<detail::ಒಳ_ಸ್ಮರಣೆ_Hard_Link>(ಗಂಟು)) {
      ಆದರೆ (I == E)
        ಹಿಂತಿರುಗಿ detail::Named_ಗಂಟು_Or_Error(ಮಾರ್ಗ, &File->getResolvedFile());
      ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(ಗಂಟು);
    ಆದರೆ (I == E)
      ಹಿಂತಿರುಗಿ detail::Named_ಗಂಟು_Or_Error(ಮಾರ್ಗ, Dir);
  }
}

ತಾರ್ಕಿಕ ಒಳ_ಸ್ಮರಣೆ_File_System::ಕೂಡಿಸು_Hard_Link(ಸ್ಥಿರಾಂಕ Twine &NewLink,
                                     ಸ್ಥಿರಾಂಕ Twine &ಗುರಿ_2) {
  ಸ್ವಯಂಚಾಲಿತ New_Link_ಗಂಟು = lookup_ಗಂಟು(NewLink, /*FollowFinalSymlink=*/ಸುಳ್ಳು);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  ಸ್ವಯಂಚಾಲಿತ ಗುರಿ_ಗಂಟು = lookup_ಗಂಟು(ಗುರಿ_2, /*FollowFinalSymlink=*/ನಿಜ);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  ಆದರೆ (!ಗುರಿ_ಗಂಟು || New_Link_ಗಂಟು || !isa<detail::ಒಳ_ಸ್ಮರಣೆ_File>(*ಗುರಿ_ಗಂಟು))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_File(NewLink, 0, ಶೂನ್ಯಸೂಚಕ, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_Info NNI) {
                   ಹಿಂತಿರುಗಿ std::make_unique<detail::ಒಳ_ಸ್ಮರಣೆ_Hard_Link>(
                       NNI.ಮಾರ್ಗ.str(),
                       *cast<detail::ಒಳ_ಸ್ಮರಣೆ_File>(*ಗುರಿ_ಗಂಟು));
                 });
}

ತಾರ್ಕಿಕ ಒಳ_ಸ್ಮರಣೆ_File_System::ಕೂಡಿಸು_Symbolic_Link(
    ಸ್ಥಿರಾಂಕ Twine &NewLink, ಸ್ಥಿರಾಂಕ Twine &ಗುರಿ_2, time_t Modification_ಸಮಯ,
    std::optional<uint32_t> User, std::optional<uint32_t> ಗುಂಪು,
    std::optional<llvm::sys::fs::perms> Perms) {
  ಸ್ವಯಂಚಾಲಿತ New_Link_ಗಂಟು = lookup_ಗಂಟು(NewLink, /*FollowFinalSymlink=*/ಸುಳ್ಳು);
  ಆದರೆ (New_Link_ಗಂಟು)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  SmallString<128> NewLinkStr, ಗುರಿ_Str;
  NewLink.ಗಮ್ಯ_ಸದಿಶ(NewLinkStr);
  ಗುರಿ_2.ಗಮ್ಯ_ಸದಿಶ(ಗುರಿ_Str);

  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_File(NewLinkStr, Modification_ಸಮಯ, ಶೂನ್ಯಸೂಚಕ, User, ಗುಂಪು,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_ಒಳ_ಸ್ಮರಣೆ_ಗಂಟು_Info NNI) {
                   ಹಿಂತಿರುಗಿ std::make_unique<detail::ಒಳ_ಸ್ಮರಣೆ_Symbolic_Link>(
                       NewLinkStr, ಗುರಿ_Str, NNI.make_ಸ್ಥಿತಿ());
                 });
}

llvm::ErrorOr<ಸ್ಥಿತಿ_2> ಒಳ_ಸ್ಮರಣೆ_File_System::ಸ್ಥಿತಿ_3(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  ಸ್ವಯಂಚಾಲಿತ ಗಂಟು = lookup_ಗಂಟು(ಮಾರ್ಗ, /*FollowFinalSymlink=*/ನಿಜ);
  ಆದರೆ (ಗಂಟು)
    ಹಿಂತಿರುಗಿ (*ಗಂಟು)->get_ಸ್ಥಿತಿ(ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ ಗಂಟು.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
ಒಳ_ಸ್ಮರಣೆ_File_System::openFileForRead(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  ಸ್ವಯಂಚಾಲಿತ ಗಂಟು = lookup_ಗಂಟು(ಮಾರ್ಗ,/*FollowFinalSymlink=*/ನಿಜ);
  ಆದರೆ (!ಗಂಟು)
    ಹಿಂತಿರುಗಿ ಗಂಟು.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *F = dyn_cast<detail::ಒಳ_ಸ್ಮರಣೆ_File>(*ಗಂಟು))
    ಹಿಂತಿರುಗಿ std::unique_ptr<File>(
        ಹೊಸ detail::ಒಳ_ಸ್ಮರಣೆ_File_Adaptor(*F, ಮಾರ್ಗ.str()));

  // FIXME: errc::not_a_file?
  ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::invalid_ವಾದ);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
ವರ್ಗ ಒಳ_ಸ್ಮರಣೆ_File_System::DirIterator : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_File_System *FS;
  detail::ಒಳ_ಸ್ಮರಣೆ_Directory::const_iterator I;
  detail::ಒಳ_ಸ್ಮರಣೆ_Directory::const_iterator E;
  std::string RequestedDirName;

  ಶೂನ್ಯ ಗಣ_Current_ನಮೂದು() {
    ಆದರೆ (I != E) {
      SmallString<256> ಮಾರ್ಗ(RequestedDirName);
      llvm::sys::ಮಾರ್ಗ_2::append(ಮಾರ್ಗ, I->ಎರಡನೇ_2->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ಆಯ್ಕೆ (I->ಎರಡನೇ_2->get_ಬಗೆ()) {
      ಸ್ಥಿತಿ detail::IME_File:
      ಸ್ಥಿತಿ detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ detail::IME_SymbolicLink:
        ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Symlink_ಗುರಿ =
                FS->lookup_ಗಂಟು(ಮಾರ್ಗ, /*FollowFinalSymlink=*/ನಿಜ)) {
          ಮಾರ್ಗ = Symlink_ಗುರಿ.getName();
          Type = (*Symlink_ಗುರಿ)->get_ಸ್ಥಿತಿ(ಮಾರ್ಗ).getType();
        }
        ನಿಲ್ಲಿಸು;
      }
      Current_ನಮೂದು = directory_ನಮೂದು(std::string(ಮಾರ್ಗ), Type);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_ನಮೂದು = directory_ನಮೂದು();
    }
  }

ಸಾರ್ವಜನಿಕ:
  DirIterator() = ಪೂರ್ವನಿಯೋಜಿತ;

  DirIterator(ಸ್ಥಿರಾಂಕ ಒಳ_ಸ್ಮರಣೆ_File_System *FS,
              ಸ್ಥಿರಾಂಕ detail::ಒಳ_ಸ್ಮರಣೆ_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.ಕೊನೆ()),
        RequestedDirName(std::move(RequestedDirName)) {
    ಗಣ_Current_ನಮೂದು();
  }

  std::error_ಸಂಕೇತ increment() override {
    ++I;
    ಗಣ_Current_ನಮೂದು();
    ಹಿಂತಿರುಗಿ {};
  }
};

directory_iterator ಒಳ_ಸ್ಮರಣೆ_File_System::dir_begin(ಸ್ಥಿರಾಂಕ Twine &Dir,
                                                 std::error_ಸಂಕೇತ &EC) {
  ಸ್ವಯಂಚಾಲಿತ ಗಂಟು = lookup_ಗಂಟು(Dir, /*FollowFinalSymlink=*/ನಿಜ);
  ಆದರೆ (!ಗಂಟು) {
    EC = ಗಂಟು.getError();
    ಹಿಂತಿರುಗಿ directory_iterator(std::make_shared<DirIterator>());
  }

  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *Dir_ಗಂಟು = dyn_cast<detail::ಒಳ_ಸ್ಮರಣೆ_Directory>(*ಗಂಟು))
    ಹಿಂತಿರುಗಿ directory_iterator(
        std::make_shared<DirIterator>(ಇದು, *Dir_ಗಂಟು, Dir.str()));

  EC = make_error_ಸಂಕೇತ(llvm::errc::not_a_directory);
  ಹಿಂತಿರುಗಿ directory_iterator(std::make_shared<DirIterator>());
}

std::error_ಸಂಕೇತ ಒಳ_ಸ್ಮರಣೆ_File_System::ಗಣ_Current_Working_Directory(ಸ್ಥಿರಾಂಕ Twine &P) {
  SmallString<128> ಮಾರ್ಗ;
  P.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_ಸಂಕೇತ EC = makeAbsolute(ಮಾರ್ಗ);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  (ಶೂನ್ಯ)EC;

  ಆದರೆ (useNormalizedPaths())
    llvm::sys::ಮಾರ್ಗ_2::remove_dots(ಮಾರ್ಗ, /*remove_dot_dot=*/ನಿಜ);

  ಆದರೆ (!ಮಾರ್ಗ.ಖಾಲಿಗಳು())
    WorkingDirectory = std::string(ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ {};
}

std::error_ಸಂಕೇತ ಒಳ_ಸ್ಮರಣೆ_File_System::get_Real_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ,
                                                Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Output) {
  ಸ್ವಯಂಚಾಲಿತ CWD = getCurrentWorkingDirectory();
  ಆದರೆ (!CWD || CWD->ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ errc::operation_not_permitted;
  ಮಾರ್ಗ.ಗಮ್ಯ_ಸದಿಶ(Output);
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = makeAbsolute(Output))
    ಹಿಂತಿರುಗಿ EC;
  llvm::sys::ಮಾರ್ಗ_2::remove_dots(Output, /*remove_dot_dot=*/ನಿಜ);
  ಹಿಂತಿರುಗಿ {};
}

std::error_ಸಂಕೇತ ಒಳ_ಸ್ಮರಣೆ_File_System::isLocal(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ) {
  ಫಲಿತಾಂಶ = ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ {};
}

ಶೂನ್ಯ ಒಳ_ಸ್ಮರಣೆ_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   ಚಿಹ್ನೆರಹಿತ IndentLevel) ಸ್ಥಿರಾಂಕ {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

ನಾಮಾಕಾಶ {

ಸ್ಥಿರ llvm::sys::ಮಾರ್ಗ_2::Style getExistingStyle(llvm::StringRef ಮಾರ್ಗ) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::ಮಾರ್ಗ_2::Style style = llvm::sys::ಮಾರ್ಗ_2::Style::native;
  ಸ್ಥಿರಾಂಕ size_t n = ಮಾರ್ಗ.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ("/\\");
  // Can't distinguish between posix and windows_slash here.
  ಆದರೆ (n != static_cast<size_t>(-1))
    style = (ಮಾರ್ಗ[n] == '/') ? llvm::sys::ಮಾರ್ಗ_2::Style::posix
                             : llvm::sys::ಮಾರ್ಗ_2::Style::windows_backslash;
  ಹಿಂತಿರುಗಿ style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ಸ್ಥಿರ llvm::SmallString<256> canonicalize(llvm::StringRef ಮಾರ್ಗ) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::ಮಾರ್ಗ_2::Style style = getExistingStyle(ಮಾರ್ಗ);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> ಫಲಿತಾಂಶ_2 =
      llvm::sys::ಮಾರ್ಗ_2::remove_leading_dotslash(ಮಾರ್ಗ, style);
  llvm::sys::ಮಾರ್ಗ_2::remove_dots(ಫಲಿತಾಂಶ_2, /*remove_dot_dot=*/ನಿಜ, style);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

/// Whether the error and entry specify a file/directory that was not found.
ಸ್ಥಿರ ತಾರ್ಕಿಕ is_File_Not_ಸಿಕ್ಕಿತು(std::error_ಸಂಕೇತ EC,
                           RedirectingFileSystem::ನಮೂದು *E = ಶೂನ್ಯಸೂಚಕ) {
  ಆದರೆ (E && !isa<RedirectingFileSystem::Directory_Remap_ನಮೂದು>(E))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  ಪ್ರತಿಜ್ಞಾ(ExternalFS && "RedirectingFileSystem requires an external FS");
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
ವರ್ಗ llvm::vfs::RedirectingFSDirIterImpl
    : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::Directory_ನಮೂದು::iterator Current, ಕೊನೆ_2;

  std::error_ಸಂಕೇತ incrementImpl(ತಾರ್ಕಿಕ Is_ಮೊದಲ_ಸಮಯ) {
    ಪ್ರತಿಜ್ಞಾ((Is_ಮೊದಲ_ಸಮಯ || Current != ಕೊನೆ_2) && "cannot iterate past end");
    ಆದರೆ (!Is_ಮೊದಲ_ಸಮಯ)
      ++Current;
    ಆದರೆ (Current != ಕೊನೆ_2) {
      SmallString<128> ಮಾರ್ಗ_Str(Dir);
      llvm::sys::ಮಾರ್ಗ_2::append(ಮಾರ್ಗ_Str, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ಆಯ್ಕೆ ((*Current)->get_ಬಗೆ()) {
      ಸ್ಥಿತಿ RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      ಸ್ಥಿತಿ RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        ನಿಲ್ಲಿಸು;
      }
      Current_ನಮೂದು = directory_ನಮೂದು(std::string(ಮಾರ್ಗ_Str), Type);
    } ಇಲ್ಲದಿದ್ದರೆ {
      Current_ನಮೂದು = directory_ನಮೂದು();
    }
    ಹಿಂತಿರುಗಿ {};
  };

ಸಾರ್ವಜನಿಕ:
  RedirectingFSDirIterImpl(
      ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, RedirectingFileSystem::Directory_ನಮೂದು::iterator Begin,
      RedirectingFileSystem::Directory_ನಮೂದು::iterator ಕೊನೆ_2, std::error_ಸಂಕೇತ &EC)
      : Dir(ಮಾರ್ಗ.str()), Current(Begin), ಕೊನೆ_2(ಕೊನೆ_2) {
    EC = incrementImpl(/*IsFirstTime=*/ನಿಜ);
  }

  std::error_ಸಂಕೇತ increment() override {
    ಹಿಂತಿರುಗಿ incrementImpl(/*IsFirstTime=*/ಸುಳ್ಳು);
  }
};

ನಾಮಾಕಾಶ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
ವರ್ಗ RedirectingFSDirRemapIterImpl : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::ಮಾರ್ಗ_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

ಸಾರ್ವಜನಿಕ:
  RedirectingFSDirRemapIterImpl(std::string Dir_ಮಾರ್ಗ,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_ಮಾರ್ಗ)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    ಆದರೆ (ExternalIter != llvm::vfs::directory_iterator())
      ಗಣ_Current_ನಮೂದು();
  }

  ಶೂನ್ಯ ಗಣ_Current_ನಮೂದು() {
    StringRef External_ಮಾರ್ಗ = ExternalIter->ಮಾರ್ಗ_2();
    llvm::sys::ಮಾರ್ಗ_2::Style ExternalStyle = getExistingStyle(External_ಮಾರ್ಗ);
    StringRef File = llvm::sys::ಮಾರ್ಗ_2::filename(External_ಮಾರ್ಗ, ExternalStyle);

    SmallString<128> New_ಮಾರ್ಗ(Dir);
    llvm::sys::ಮಾರ್ಗ_2::append(New_ಮಾರ್ಗ, DirStyle, File);

    Current_ನಮೂದು = directory_ನಮೂದು(std::string(New_ಮಾರ್ಗ), ExternalIter->type());
  }

  std::error_ಸಂಕೇತ increment() override {
    std::error_ಸಂಕೇತ EC;
    ExternalIter.increment(EC);
    ಆದರೆ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      ಗಣ_Current_ನಮೂದು();
    ಇಲ್ಲದಿದ್ದರೆ
      Current_ನಮೂದು = directory_ನಮೂದು();
    ಹಿಂತಿರುಗಿ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ WorkingDirectory;
}

std::error_ಸಂಕೇತ
RedirectingFileSystem::ಗಣ_Current_Working_Directory(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) {
  // Don't change the working directory if the path doesn't exist.
  ಆದರೆ (!exists(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ errc::no_such_file_or_directory;

  SmallString<128> Absolute_ಮಾರ್ಗ;
  ಮಾರ್ಗ.ಗಮ್ಯ_ಸದಿಶ(Absolute_ಮಾರ್ಗ);
  ಆದರೆ (std::error_ಸಂಕೇತ EC = makeAbsolute(Absolute_ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ EC;
  WorkingDirectory = std::string(Absolute_ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ {};
}

std::error_ಸಂಕೇತ RedirectingFileSystem::isLocal(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ_3,
                                               ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ) {
  SmallString<256> ಮಾರ್ಗ;
  ಮಾರ್ಗ_3.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  ಆದರೆ (makeAbsolute(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ {};

  ಹಿಂತಿರುಗಿ ExternalFS->isLocal(ಮಾರ್ಗ, ಫಲಿತಾಂಶ);
}

std::error_ಸಂಕೇತ RedirectingFileSystem::makeAbsolute(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ) ಸ್ಥಿರಾಂಕ {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  ಆದರೆ (llvm::sys::ಮಾರ್ಗ_2::is_absolute(ಮಾರ್ಗ, llvm::sys::ಮಾರ್ಗ_2::Style::posix) ||
      llvm::sys::ಮಾರ್ಗ_2::is_absolute(ಮಾರ್ಗ,
                                   llvm::sys::ಮಾರ್ಗ_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ಹಿಂತಿರುಗಿ {};

  ಸ್ವಯಂಚಾಲಿತ WorkingDir = getCurrentWorkingDirectory();
  ಆದರೆ (!WorkingDir)
    ಹಿಂತಿರುಗಿ WorkingDir.getError();

  ಹಿಂತಿರುಗಿ makeAbsolute(WorkingDir.get(), ಮಾರ್ಗ);
}

std::error_ಸಂಕೇತ
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ) ಸ್ಥಿರಾಂಕ {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  ಆದರೆ (!WorkingDir.ಖಾಲಿಗಳು() &&
      !sys::ಮಾರ್ಗ_2::is_absolute(WorkingDir, sys::ಮಾರ್ಗ_2::Style::posix) &&
      !sys::ಮಾರ್ಗ_2::is_absolute(WorkingDir,
                              sys::ಮಾರ್ಗ_2::Style::windows_backslash)) {
    ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
  }
  sys::ಮಾರ್ಗ_2::Style style = sys::ಮಾರ್ಗ_2::Style::windows_backslash;
  ಆದರೆ (sys::ಮಾರ್ಗ_2::is_absolute(WorkingDir, sys::ಮಾರ್ಗ_2::Style::posix)) {
    style = sys::ಮಾರ್ಗ_2::Style::posix;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    ಆದರೆ (getExistingStyle(WorkingDir) != sys::ಮಾರ್ಗ_2::Style::windows_backslash)
      style = sys::ಮಾರ್ಗ_2::Style::windows_slash;
  }

  std::string ಫಲಿತಾಂಶ = std::string(WorkingDir);
  StringRef Dir(ಫಲಿತಾಂಶ);
  ಆದರೆ (!Dir.ತುದಿಗಳು_with(sys::ಮಾರ್ಗ_2::get_separator(style))) {
    ಫಲಿತಾಂಶ += sys::ಮಾರ್ಗ_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  ಫಲಿತಾಂಶ.append(ಮಾರ್ಗ.ದತ್ತಾಂಶ(), ಮಾರ್ಗ.ಗಾತ್ರ_3());
  ಮಾರ್ಗ.ನಿಯೋಜಿಸು(ಫಲಿತಾಂಶ.begin(), ಫಲಿತಾಂಶ.ಕೊನೆ());

  ಹಿಂತಿರುಗಿ {};
}

directory_iterator RedirectingFileSystem::dir_begin(ಸ್ಥಿರಾಂಕ Twine &Dir,
                                                    std::error_ಸಂಕೇತ &EC) {
  SmallString<256> ಮಾರ್ಗ;
  Dir.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  EC = makeAbsolute(ಮಾರ್ಗ);
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ {};

  ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಫಲಿತಾಂಶ = lookup_ಮಾರ್ಗ(ಮಾರ್ಗ);
  ಆದರೆ (!ಫಲಿತಾಂಶ) {
    ಆದರೆ (Redirection != Redirect_ಬಗೆ::RedirectOnly &&
        is_File_Not_ಸಿಕ್ಕಿತು(ಫಲಿತಾಂಶ.getError()))
      ಹಿಂತಿರುಗಿ ExternalFS->dir_begin(ಮಾರ್ಗ, EC);

    EC = ಫಲಿತಾಂಶ.getError();
    ಹಿಂತಿರುಗಿ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<ಸ್ಥಿತಿ_2> S = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ, Dir, *ಫಲಿತಾಂಶ);
  ಆದರೆ (!S) {
    ಆದರೆ (Redirection != Redirect_ಬಗೆ::RedirectOnly &&
        is_File_Not_ಸಿಕ್ಕಿತು(S.getError(), ಫಲಿತಾಂಶ->E))
      ಹಿಂತಿರುಗಿ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ಹಿಂತಿರುಗಿ {};
  }

  ಆದರೆ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ಹಿಂತಿರುಗಿ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_ಸಂಕೇತ RedirectEC;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ ExtRedirect = ಫಲಿತಾಂಶ->getExternalRedirect()) {
    ಸ್ವಯಂಚಾಲಿತ RE = cast<RedirectingFileSystem::Remap_ನಮೂದು>(ಫಲಿತಾಂಶ->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    ಆದರೆ (!RE->useExternalName(Use_External_ಹೆಸರುಗಳು)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(ಮಾರ್ಗ), RedirectIter));
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ವಯಂಚಾಲಿತ DE = cast<Directory_ನಮೂದು>(ಫಲಿತಾಂಶ->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            ಮಾರ್ಗ, DE->contents_begin(), DE->contents_ಕೊನೆ(), RedirectEC));
  }

  ಆದರೆ (RedirectEC) {
    ಆದರೆ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ಹಿಂತಿರುಗಿ {};
    }
    RedirectIter = {};
  }

  ಆದರೆ (Redirection == Redirect_ಬಗೆ::RedirectOnly) {
    EC = RedirectEC;
    ಹಿಂತಿರುಗಿ RedirectIter;
  }

  std::error_ಸಂಕೇತ ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(ಮಾರ್ಗ, ExternalEC);
  ಆದರೆ (ExternalEC) {
    ಆದರೆ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ಹಿಂತಿರುಗಿ {};
    }
    ExternalIter = {};
  }

  Small_ಸದಿಶ<directory_iterator, 2> Iters;
  ಆಯ್ಕೆ (Redirection) {
  ಸ್ಥಿತಿ Redirect_ಬಗೆ::Fallthrough:
    Iters.ತಳ್ಳು_ಹಿಂಭಾಗ(ExternalIter);
    Iters.ತಳ್ಳು_ಹಿಂಭಾಗ(RedirectIter);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Redirect_ಬಗೆ::Fallback:
    Iters.ತಳ್ಳು_ಹಿಂಭಾಗ(RedirectIter);
    Iters.ತಳ್ಳು_ಹಿಂಭಾಗ(ExternalIter);
    ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ {};
  ಹಿಂತಿರುಗಿ Combined;
}

ಶೂನ್ಯ RedirectingFileSystem::ಗಣ_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ OverlayFileDir;
}

ಶೂನ್ಯ RedirectingFileSystem::ಗಣ_Fallthrough(ತಾರ್ಕಿಕ Fallthrough) {
  ಆದರೆ (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_ಬಗೆ::Fallthrough;
  } ಇಲ್ಲದಿದ್ದರೆ {
    Redirection = RedirectingFileSystem::Redirect_ಬಗೆ::RedirectOnly;
  }
}

ಶೂನ್ಯ RedirectingFileSystem::ಗಣ_Redirection(
    RedirectingFileSystem::Redirect_ಬಗೆ ಬಗೆ_2) {
  Redirection = ಬಗೆ_2;
}

std::ಸದಿಶ<StringRef> RedirectingFileSystem::getRoots() ಸ್ಥಿರಾಂಕ {
  std::ಸದಿಶ<StringRef> R;
  R.reserve(Roots.ಗಾತ್ರ_3());
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &ಬೇರು_2 : Roots)
    R.ತಳ್ಳು_ಹಿಂಭಾಗ(ಬೇರು_2->getName());
  ಹಿಂತಿರುಗಿ R;
}

ಶೂನ್ಯ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      ಚಿಹ್ನೆರಹಿತ IndentLevel) ಸ್ಥಿರಾಂಕ {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_ಹೆಸರುಗಳು ? "true" : "false") << ")\n";
  ಆದರೆ (Type == PrintType::Summary)
    ಹಿಂತಿರುಗಿ;

  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &ಬೇರು_2 : Roots)
    print_ನಮೂದು(OS, ಬೇರು_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

ಶೂನ್ಯ RedirectingFileSystem::print_ನಮೂದು(raw_ostream &OS,
                                       RedirectingFileSystem::ನಮೂದು *E,
                                       ಚಿಹ್ನೆರಹಿತ IndentLevel) ಸ್ಥಿರಾಂಕ {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  ಆಯ್ಕೆ (E->get_ಬಗೆ()) {
  ಸ್ಥಿತಿ EK_Directory: {
    ಸ್ವಯಂಚಾಲಿತ *DE = cast<RedirectingFileSystem::Directory_ನಮೂದು>(E);

    OS << "\n";
    ಗಾಗಿ (std::unique_ptr<ನಮೂದು> &Sub_ನಮೂದು :
         llvm::make_range(DE->contents_begin(), DE->contents_ಕೊನೆ()))
      print_ನಮೂದು(OS, Sub_ನಮೂದು.get(), IndentLevel + 1);
    ನಿಲ್ಲಿಸು;
  }
  ಸ್ಥಿತಿ EK_DirectoryRemap:
  ಸ್ಥಿತಿ EK_File: {
    ಸ್ವಯಂಚಾಲಿತ *RE = cast<RedirectingFileSystem::Remap_ನಮೂದು>(E);
    OS << " -> '" << RE->get_External_Contents_ಮಾರ್ಗ() << "'";
    ಆಯ್ಕೆ (RE->getUseName()) {
    ಸ್ಥಿತಿ NK_Not_ಗಣ:
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ NK_External:
      OS << " (UseExternalName: true)";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ NK_Virtual:
      OS << " (UseExternalName: false)";
      ನಿಲ್ಲಿಸು;
    }
    OS << "\n";
    ನಿಲ್ಲಿಸು;
  }
  }
}

ಶೂನ್ಯ RedirectingFileSystem::ಭೇಟಿ_ಮಗು_File_Systems(ಭೇಟಿ_Callback_Ty Callback) {
  ಆದರೆ (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->ಭೇಟಿ_ಮಗು_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
ವರ್ಗ llvm::vfs::RedirectingFileSystemParser {
  yaml::ಹರಿವು &ಹರಿವು;

  ಶೂನ್ಯ error(yaml::ಗಂಟು *N, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_4) { ಹರಿವು.printError(N, ಸಂದೇಶ_4); }

  // false on error
  ತಾರ್ಕಿಕ parseScalarString(yaml::ಗಂಟು *N, StringRef &ಫಲಿತಾಂಶ,
                         Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage) {
    ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *S = dyn_cast_if_present<yaml::Scalar_ಗಂಟು>(N);

    ಆದರೆ (!S) {
      error(N, "expected string");
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    ಫಲಿತಾಂಶ = S->get_ಮೌಲ್ಯ(Storage);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  // false on error
  ತಾರ್ಕಿಕ parseScalarBool(yaml::ಗಂಟು *N, ತಾರ್ಕಿಕ &ಫಲಿತಾಂಶ) {
    SmallString<5> Storage;
    StringRef ಮೌಲ್ಯ;
    ಆದರೆ (!parseScalarString(N, ಮೌಲ್ಯ, Storage))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    ಆದರೆ (ಮೌಲ್ಯ.equals_insensitive("true") || ಮೌಲ್ಯ.equals_insensitive("on") ||
        ಮೌಲ್ಯ.equals_insensitive("yes") || ಮೌಲ್ಯ == "1") {
      ಫಲಿತಾಂಶ = ನಿಜ;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ.equals_insensitive("false") ||
               ಮೌಲ್ಯ.equals_insensitive("off") ||
               ಮೌಲ್ಯ.equals_insensitive("no") || ಮೌಲ್ಯ == "0") {
      ಫಲಿತಾಂಶ = ಸುಳ್ಳು;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }

    error(N, "expected boolean value");
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  std::optional<RedirectingFileSystem::Redirect_ಬಗೆ>
  parse_Redirect_ಬಗೆ(yaml::ಗಂಟು *N) {
    SmallString<12> Storage;
    StringRef ಮೌಲ್ಯ;
    ಆದರೆ (!parseScalarString(N, ಮೌಲ್ಯ, Storage))
      ಹಿಂತಿರುಗಿ std::nullopt;

    ಆದರೆ (ಮೌಲ್ಯ.equals_insensitive("fallthrough")) {
      ಹಿಂತಿರುಗಿ RedirectingFileSystem::Redirect_ಬಗೆ::Fallthrough;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ.equals_insensitive("fallback")) {
      ಹಿಂತಿರುಗಿ RedirectingFileSystem::Redirect_ಬಗೆ::Fallback;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ.equals_insensitive("redirect-only")) {
      ಹಿಂತಿರುಗಿ RedirectingFileSystem::Redirect_ಬಗೆ::RedirectOnly;
    }
    ಹಿಂತಿರುಗಿ std::nullopt;
  }

  std::optional<RedirectingFileSystem::ಬೇರು_Relative_ಬಗೆ>
  parse_ಬೇರು_Relative_ಬಗೆ(yaml::ಗಂಟು *N) {
    SmallString<12> Storage;
    StringRef ಮೌಲ್ಯ;
    ಆದರೆ (!parseScalarString(N, ಮೌಲ್ಯ, Storage))
      ಹಿಂತಿರುಗಿ std::nullopt;
    ಆದರೆ (ಮೌಲ್ಯ.equals_insensitive("cwd")) {
      ಹಿಂತಿರುಗಿ RedirectingFileSystem::ಬೇರು_Relative_ಬಗೆ::CWD;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ.equals_insensitive("overlay-dir")) {
      ಹಿಂತಿರುಗಿ RedirectingFileSystem::ಬೇರು_Relative_ಬಗೆ::OverlayDir;
    }
    ಹಿಂತಿರುಗಿ std::nullopt;
  }

  ರಚನೆ ಕೀಲಿ_ಸ್ಥಿತಿ {
    ತಾರ್ಕಿಕ Required;
    ತಾರ್ಕಿಕ ನೋಡಿದ = ಸುಳ್ಳು;

    ಕೀಲಿ_ಸ್ಥಿತಿ(ತಾರ್ಕಿಕ Required = ಸುಳ್ಳು) : Required(Required) {}
  };

  ಬಳಸಿ ಕೀಲಿ_ಸ್ಥಿತಿ_Pair = std::pair<StringRef, ಕೀಲಿ_ಸ್ಥಿತಿ>;

  // false on error
  ತಾರ್ಕಿಕ check_Duplicate_Or_Unknown_ಕೀಲಿ(yaml::ಗಂಟು *ಕೀಲಿ_ಗಂಟು, StringRef ಕೀಲಿ,
                                  DenseMap<StringRef, ಕೀಲಿ_ಸ್ಥಿತಿ> &ಕೀಲಿಗಳು_2) {
    ಸ್ವಯಂಚಾಲಿತ It = ಕೀಲಿಗಳು_2.ಕಂಡುಹಿಡಿ(ಕೀಲಿ);
    ಆದರೆ (It == ಕೀಲಿಗಳು_2.ಕೊನೆ()) {
      error(ಕೀಲಿ_ಗಂಟು, "unknown key");
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    ಕೀಲಿ_ಸ್ಥಿತಿ &S = It->ಎರಡನೇ_2;
    ಆದರೆ (S.ನೋಡಿದ) {
      error(ಕೀಲಿ_ಗಂಟು, Twine("duplicate key '") + ಕೀಲಿ + "'");
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    S.ನೋಡಿದ = ನಿಜ;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  // false on error
  ತಾರ್ಕಿಕ check_Missing_ಕೀಲಿಗಳು(yaml::ಗಂಟು *Obj, DenseMap<StringRef, ಕೀಲಿ_ಸ್ಥಿತಿ> &ಕೀಲಿಗಳು_2) {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &I : ಕೀಲಿಗಳು_2) {
      ಆದರೆ (I.ಎರಡನೇ_2.Required && !I.ಎರಡನೇ_2.ನೋಡಿದ) {
        error(Obj, Twine("missing key '") + I.ಮೊದಲ_2 + "'");
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      }
    }
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

ಸಾರ್ವಜನಿಕ:
  ಸ್ಥಿರ RedirectingFileSystem::ನಮೂದು *
  lookup_Or_Create_ನಮೂದು(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::ನಮೂದು *ಪೋಷಕ_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ) {
    ಆದರೆ (!ಪೋಷಕ_ನಮೂದು) { // Look for a existent root
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &ಬೇರು_2 : FS->Roots) {
        ಆದರೆ (Name == ಬೇರು_2->getName()) {
          ಪೋಷಕ_ನಮೂದು = ಬೇರು_2.get();
          ಹಿಂತಿರುಗಿ ಪೋಷಕ_ನಮೂದು;
        }
      }
    } ಇಲ್ಲದಿದ್ದರೆ { // Advance to the next component
      ಸ್ವಯಂಚಾಲಿತ *DE = dyn_cast<RedirectingFileSystem::Directory_ನಮೂದು>(ಪೋಷಕ_ನಮೂದು);
      ಗಾಗಿ (std::unique_ptr<RedirectingFileSystem::ನಮೂದು> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_ಕೊನೆ())) {
        ಸ್ವಯಂಚಾಲಿತ *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_ನಮೂದು>(Content.get());
        ಆದರೆ (DirContent && Name == Content->getName())
          ಹಿಂತಿರುಗಿ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::ನಮೂದು> E =
        std::make_unique<RedirectingFileSystem::Directory_ನಮೂದು>(
            Name, ಸ್ಥಿತಿ_2("", get_ಮುಂದಿನ_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    ಆದರೆ (!ಪೋಷಕ_ನಮೂದು) { // Add a new root to the overlay
      FS->Roots.ತಳ್ಳು_ಹಿಂಭಾಗ(std::move(E));
      ಪೋಷಕ_ನಮೂದು = FS->Roots.ಹಿಂಭಾಗ().get();
      ಹಿಂತಿರುಗಿ ಪೋಷಕ_ನಮೂದು;
    }

    ಸ್ವಯಂಚಾಲಿತ *DE = cast<RedirectingFileSystem::Directory_ನಮೂದು>(ಪೋಷಕ_ನಮೂದು);
    DE->ಕೂಡಿಸು_Content(std::move(E));
    ಹಿಂತಿರುಗಿ DE->get_ಕೊನೆಯ_Content();
  }

ಖಾಸಗಿ:
  ಶೂನ್ಯ unique_Overlay_ವೃಕ್ಷ(RedirectingFileSystem *FS,
                         RedirectingFileSystem::ನಮೂದು *SrcE,
                         RedirectingFileSystem::ನಮೂದು *New_ಪೋಷಕ_E = ಶೂನ್ಯಸೂಚಕ) {
    StringRef Name = SrcE->getName();
    ಆಯ್ಕೆ (SrcE->get_ಬಗೆ()) {
    ಸ್ಥಿತಿ RedirectingFileSystem::EK_Directory: {
      ಸ್ವಯಂಚಾಲಿತ *DE = cast<RedirectingFileSystem::Directory_ನಮೂದು>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      ಆದರೆ (!Name.ಖಾಲಿಗಳು())
        New_ಪೋಷಕ_E = lookup_Or_Create_ನಮೂದು(FS, Name, New_ಪೋಷಕ_E);
      ಗಾಗಿ (std::unique_ptr<RedirectingFileSystem::ನಮೂದು> &Sub_ನಮೂದು :
           llvm::make_range(DE->contents_begin(), DE->contents_ಕೊನೆ()))
        unique_Overlay_ವೃಕ್ಷ(FS, Sub_ನಮೂದು.get(), New_ಪೋಷಕ_E);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ RedirectingFileSystem::EK_DirectoryRemap: {
      ಪ್ರತಿಜ್ಞಾ(New_ಪೋಷಕ_E && "Parent entry must exist");
      ಸ್ವಯಂಚಾಲಿತ *DR = cast<RedirectingFileSystem::Directory_Remap_ನಮೂದು>(SrcE);
      ಸ್ವಯಂಚಾಲಿತ *DE = cast<RedirectingFileSystem::Directory_ನಮೂದು>(New_ಪೋಷಕ_E);
      DE->ಕೂಡಿಸು_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_ನಮೂದು>(
              Name, DR->get_External_Contents_ಮಾರ್ಗ(), DR->getUseName()));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ RedirectingFileSystem::EK_File: {
      ಪ್ರತಿಜ್ಞಾ(New_ಪೋಷಕ_E && "Parent entry must exist");
      ಸ್ವಯಂಚಾಲಿತ *FE = cast<RedirectingFileSystem::File_ನಮೂದು>(SrcE);
      ಸ್ವಯಂಚಾಲಿತ *DE = cast<RedirectingFileSystem::Directory_ನಮೂದು>(New_ಪೋಷಕ_E);
      DE->ಕೂಡಿಸು_Content(std::make_unique<RedirectingFileSystem::File_ನಮೂದು>(
          Name, FE->get_External_Contents_ಮಾರ್ಗ(), FE->getUseName()));
      ನಿಲ್ಲಿಸು;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::ನಮೂದು>
  parse_ನಮೂದು(yaml::ಗಂಟು *N, RedirectingFileSystem *FS, ತಾರ್ಕಿಕ Is_ಬೇರು_ನಮೂದು) {
    ಸ್ವಯಂಚಾಲಿತ *M = dyn_cast<yaml::Mapping_ಗಂಟು>(N);
    ಆದರೆ (!M) {
      error(N, "expected mapping node for file or directory entry");
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    }

    ಕೀಲಿ_ಸ್ಥಿತಿ_Pair Fields[] = {
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("name", ನಿಜ),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("type", ನಿಜ),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("contents", ಸುಳ್ಳು),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("external-contents", ಸುಳ್ಳು),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("use-external-name", ಸುಳ್ಳು),
    };

    DenseMap<StringRef, ಕೀಲಿ_ಸ್ಥಿತಿ> ಕೀಲಿಗಳು_2(std::begin(Fields), std::ಕೊನೆ(Fields));

    ಎಣಿಕೆ { CF_Not_ಗಣ, CF_List, CF_External } Contents_ಕ್ಷೇತ್ರ = CF_Not_ಗಣ;
    std::ಸದಿಶ<std::unique_ptr<RedirectingFileSystem::ನಮೂದು>>
        ನಮೂದು_Array_Contents;
    SmallString<256> External_Contents_ಮಾರ್ಗ;
    SmallString<256> Name;
    yaml::ಗಂಟು *Name_ಮೌಲ್ಯ_ಗಂಟು = ಶೂನ್ಯಸೂಚಕ;
    ಸ್ವಯಂಚಾಲಿತ UseExternalName = RedirectingFileSystem::NK_Not_ಗಣ;
    RedirectingFileSystem::ನಮೂದು_ಬಗೆ ಬಗೆ_2;

    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &I : *M) {
      StringRef ಕೀಲಿ;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> ಬಫರ್;
      ಆದರೆ (!parseScalarString(I.get_ಕೀಲಿ(), ಕೀಲಿ, ಬಫರ್))
        ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

      ಆದರೆ (!check_Duplicate_Or_Unknown_ಕೀಲಿ(I.get_ಕೀಲಿ(), ಕೀಲಿ, ಕೀಲಿಗಳು_2))
        ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

      StringRef ಮೌಲ್ಯ;
      ಆದರೆ (ಕೀಲಿ == "name") {
        ಆದರೆ (!parseScalarString(I.get_ಮೌಲ್ಯ(), ಮೌಲ್ಯ, ಬಫರ್))
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

        Name_ಮೌಲ್ಯ_ಗಂಟು = I.get_ಮೌಲ್ಯ();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(ಮೌಲ್ಯ).str();
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "type") {
        ಆದರೆ (!parseScalarString(I.get_ಮೌಲ್ಯ(), ಮೌಲ್ಯ, ಬಫರ್))
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
        ಆದರೆ (ಮೌಲ್ಯ == "file")
          ಬಗೆ_2 = RedirectingFileSystem::EK_File;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ == "directory")
          ಬಗೆ_2 = RedirectingFileSystem::EK_Directory;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ == "directory-remap")
          ಬಗೆ_2 = RedirectingFileSystem::EK_DirectoryRemap;
        ಇಲ್ಲದಿದ್ದರೆ {
          error(I.get_ಮೌಲ್ಯ(), "unknown value for 'type'");
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "contents") {
        ಆದರೆ (Contents_ಕ್ಷೇತ್ರ != CF_Not_ಗಣ) {
          error(I.get_ಕೀಲಿ(),
                "entry already has 'contents' or 'external-contents'");
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
        }
        Contents_ಕ್ಷೇತ್ರ = CF_List;
        ಸ್ವಯಂಚಾಲಿತ *Contents = dyn_cast_if_present<yaml::Sequence_ಗಂಟು>(I.get_ಮೌಲ್ಯ());
        ಆದರೆ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_ಮೌಲ್ಯ(), "expected array");
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
        }

        ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &I : *Contents) {
          ಆದರೆ (std::unique_ptr<RedirectingFileSystem::ನಮೂದು> E =
                  parse_ನಮೂದು(&I, FS, /*IsRootEntry*/ ಸುಳ್ಳು))
            ನಮೂದು_Array_Contents.ತಳ್ಳು_ಹಿಂಭಾಗ(std::move(E));
          ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "external-contents") {
        ಆದರೆ (Contents_ಕ್ಷೇತ್ರ != CF_Not_ಗಣ) {
          error(I.get_ಕೀಲಿ(),
                "entry already has 'contents' or 'external-contents'");
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
        }
        Contents_ಕ್ಷೇತ್ರ = CF_External;
        ಆದರೆ (!parseScalarString(I.get_ಮೌಲ್ಯ(), ಮೌಲ್ಯ, ಬಫರ್))
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

        SmallString<256> ತುಂಬಿದ_ಮಾರ್ಗ;
        ಆದರೆ (FS->IsRelativeOverlay) {
          ತುಂಬಿದ_ಮಾರ್ಗ = FS->getOverlayFileDir();
          ಪ್ರತಿಜ್ಞಾ(!ತುಂಬಿದ_ಮಾರ್ಗ.ಖಾಲಿಗಳು() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_ತುಂಬಿದ_ಮಾರ್ಗ = ಮೌಲ್ಯ;
          ಆದರೆ (FS->makeAbsolute(ತುಂಬಿದ_ಮಾರ್ಗ, Abs_ತುಂಬಿದ_ಮಾರ್ಗ)) {
            error(N, "failed to make 'external-contents' absolute");
            ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
          }
          ತುಂಬಿದ_ಮಾರ್ಗ = Abs_ತುಂಬಿದ_ಮಾರ್ಗ;
        } ಇಲ್ಲದಿದ್ದರೆ {
          ತುಂಬಿದ_ಮಾರ್ಗ = ಮೌಲ್ಯ;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        ತುಂಬಿದ_ಮಾರ್ಗ = canonicalize(ತುಂಬಿದ_ಮಾರ್ಗ);
        External_Contents_ಮಾರ್ಗ = ತುಂಬಿದ_ಮಾರ್ಗ.str();
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "use-external-name") {
        ತಾರ್ಕಿಕ Val;
        ಆದರೆ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), Val))
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } ಇಲ್ಲದಿದ್ದರೆ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ಆದರೆ (ಹರಿವು.failed())
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

    // check for missing keys
    ಆದರೆ (Contents_ಕ್ಷೇತ್ರ == CF_Not_ಗಣ) {
      error(N, "missing key 'contents' or 'external-contents'");
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    }
    ಆದರೆ (!check_Missing_ಕೀಲಿಗಳು(N, ಕೀಲಿಗಳು_2))
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

    // check invalid configuration
    ಆದರೆ (ಬಗೆ_2 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_ಗಣ) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    }

    ಆದರೆ (ಬಗೆ_2 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_ಕ್ಷೇತ್ರ == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    }

    sys::ಮಾರ್ಗ_2::Style ಮಾರ್ಗ_style = sys::ಮಾರ್ಗ_2::Style::native;
    ಆದರೆ (Is_ಬೇರು_ನಮೂದು) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      ಆದರೆ (sys::ಮಾರ್ಗ_2::is_absolute(Name, sys::ಮಾರ್ಗ_2::Style::posix)) {
        ಮಾರ್ಗ_style = sys::ಮಾರ್ಗ_2::Style::posix;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sys::ಮಾರ್ಗ_2::is_absolute(Name,
                                        sys::ಮಾರ್ಗ_2::Style::windows_backslash)) {
        ಮಾರ್ಗ_style = sys::ಮಾರ್ಗ_2::Style::windows_backslash;
      } ಇಲ್ಲದಿದ್ದರೆ {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_ಸಂಕೇತ EC;
        ಆದರೆ (FS->ಬೇರು_Relative ==
            RedirectingFileSystem::ಬೇರು_Relative_ಬಗೆ::OverlayDir) {
          StringRef ತುಂಬಿದ_ಮಾರ್ಗ = FS->getOverlayFileDir();
          ಪ್ರತಿಜ್ಞಾ(!ತುಂಬಿದ_ಮಾರ್ಗ.ಖಾಲಿಗಳು() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(ತುಂಬಿದ_ಮಾರ್ಗ, Name);
          Name = canonicalize(Name);
        } ಇಲ್ಲದಿದ್ದರೆ {
          EC = FS->makeAbsolute(Name);
        }
        ಆದರೆ (EC) {
          ಪ್ರತಿಜ್ಞಾ(Name_ಮೌಲ್ಯ_ಗಂಟು && "Name presence should be checked earlier");
          error(
              Name_ಮೌಲ್ಯ_ಗಂಟು,
              "entry with relative path at the root level is not discoverable");
          ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
        }
        ಮಾರ್ಗ_style = sys::ಮಾರ್ಗ_2::is_absolute(Name, sys::ಮಾರ್ಗ_2::Style::posix)
                         ? sys::ಮಾರ್ಗ_2::Style::posix
                         : sys::ಮಾರ್ಗ_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      ಆದರೆ (ಮಾರ್ಗ_style == sys::ಮಾರ್ಗ_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::ಮಾರ್ಗ_2::Style::windows_backslash)
        ಮಾರ್ಗ_style = sys::ಮಾರ್ಗ_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t ಬೇರು_ಮಾರ್ಗ_Len = sys::ಮಾರ್ಗ_2::ಬೇರು_ಮಾರ್ಗ_2(Trimmed, ಮಾರ್ಗ_style).ಗಾತ್ರ_3();
    ಆಗ (Trimmed.ಗಾತ್ರ_3() > ಬೇರು_ಮಾರ್ಗ_Len &&
           sys::ಮಾರ್ಗ_2::is_separator(Trimmed.ಹಿಂಭಾಗ(), ಮಾರ್ಗ_style))
      Trimmed = Trimmed.ತುಂಡು(0, Trimmed.ಗಾತ್ರ_3() - 1);

    // Get the last component
    StringRef ಕೊನೆಯ_Component = sys::ಮಾರ್ಗ_2::filename(Trimmed, ಮಾರ್ಗ_style);

    std::unique_ptr<RedirectingFileSystem::ನಮೂದು> ಫಲಿತಾಂಶ;
    ಆಯ್ಕೆ (ಬಗೆ_2) {
    ಸ್ಥಿತಿ RedirectingFileSystem::EK_File:
      ಫಲಿತಾಂಶ = std::make_unique<RedirectingFileSystem::File_ನಮೂದು>(
          ಕೊನೆಯ_Component, std::move(External_Contents_ಮಾರ್ಗ), UseExternalName);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ RedirectingFileSystem::EK_DirectoryRemap:
      ಫಲಿತಾಂಶ = std::make_unique<RedirectingFileSystem::Directory_Remap_ನಮೂದು>(
          ಕೊನೆಯ_Component, std::move(External_Contents_ಮಾರ್ಗ), UseExternalName);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ RedirectingFileSystem::EK_Directory:
      ಫಲಿತಾಂಶ = std::make_unique<RedirectingFileSystem::Directory_ನಮೂದು>(
          ಕೊನೆಯ_Component, std::move(ನಮೂದು_Array_Contents),
          ಸ್ಥಿತಿ_2("", get_ಮುಂದಿನ_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      ನಿಲ್ಲಿಸು;
    }

    StringRef ಪೋಷಕ_2 = sys::ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(Trimmed, ಮಾರ್ಗ_style);
    ಆದರೆ (ಪೋಷಕ_2.ಖಾಲಿಗಳು())
      ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;

    // if 'name' contains multiple components, create implicit directory entries
    ಗಾಗಿ (sys::ಮಾರ್ಗ_2::ಹಿಮ್ಮುಖ_iterator I = sys::ಮಾರ್ಗ_2::rbegin(ಪೋಷಕ_2, ಮಾರ್ಗ_style),
                                     E = sys::ಮಾರ್ಗ_2::rend(ಪೋಷಕ_2);
         I != E; ++I) {
      std::ಸದಿಶ<std::unique_ptr<RedirectingFileSystem::ನಮೂದು>> Entries;
      Entries.ತಳ್ಳು_ಹಿಂಭಾಗ(std::move(ಫಲಿತಾಂಶ));
      ಫಲಿತಾಂಶ = std::make_unique<RedirectingFileSystem::Directory_ನಮೂದು>(
          *I, std::move(Entries),
          ಸ್ಥಿತಿ_2("", get_ಮುಂದಿನ_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  }

ಸಾರ್ವಜನಿಕ:
  RedirectingFileSystemParser(yaml::ಹರಿವು &S) : ಹರಿವು(S) {}

  // false on error
  ತಾರ್ಕಿಕ parse(yaml::ಗಂಟು *ಬೇರು_2, RedirectingFileSystem *FS) {
    ಸ್ವಯಂಚಾಲಿತ *ತುದಿ = dyn_cast<yaml::Mapping_ಗಂಟು>(ಬೇರು_2);
    ಆದರೆ (!ತುದಿ) {
      error(ಬೇರು_2, "expected mapping node");
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }

    ಕೀಲಿ_ಸ್ಥಿತಿ_Pair Fields[] = {
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("version", ನಿಜ),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("case-sensitive", ಸುಳ್ಳು),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("use-external-names", ಸುಳ್ಳು),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("root-relative", ಸುಳ್ಳು),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("overlay-relative", ಸುಳ್ಳು),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("fallthrough", ಸುಳ್ಳು),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("redirecting-with", ಸುಳ್ಳು),
        ಕೀಲಿ_ಸ್ಥಿತಿ_Pair("roots", ನಿಜ),
    };

    DenseMap<StringRef, ಕೀಲಿ_ಸ್ಥಿತಿ> ಕೀಲಿಗಳು_2(std::begin(Fields), std::ಕೊನೆ(Fields));
    std::ಸದಿಶ<std::unique_ptr<RedirectingFileSystem::ನಮೂದು>> ಬೇರು_Entries;

    // Parse configuration and 'roots'
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &I : *ತುದಿ) {
      SmallString<10> ಕೀಲಿ_ಬಫರ್;
      StringRef ಕೀಲಿ;
      ಆದರೆ (!parseScalarString(I.get_ಕೀಲಿ(), ಕೀಲಿ, ಕೀಲಿ_ಬಫರ್))
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

      ಆದರೆ (!check_Duplicate_Or_Unknown_ಕೀಲಿ(I.get_ಕೀಲಿ(), ಕೀಲಿ, ಕೀಲಿಗಳು_2))
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

      ಆದರೆ (ಕೀಲಿ == "roots") {
        ಸ್ವಯಂಚಾಲಿತ *Roots = dyn_cast_if_present<yaml::Sequence_ಗಂಟು>(I.get_ಮೌಲ್ಯ());
        ಆದರೆ (!Roots) {
          error(I.get_ಮೌಲ್ಯ(), "expected array");
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }

        ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &I : *Roots) {
          ಆದರೆ (std::unique_ptr<RedirectingFileSystem::ನಮೂದು> E =
                  parse_ನಮೂದು(&I, FS, /*IsRootEntry*/ ನಿಜ))
            ಬೇರು_Entries.ತಳ್ಳು_ಹಿಂಭಾಗ(std::move(E));
          ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "version") {
        StringRef ಆವೃತ್ತಿ_String;
        SmallString<4> Storage;
        ಆದರೆ (!parseScalarString(I.get_ಮೌಲ್ಯ(), ಆವೃತ್ತಿ_String, Storage))
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        ಪೂರ್ಣಾಂಕ ಆವೃತ್ತಿ;
        ಆದರೆ (ಆವೃತ್ತಿ_String.getAsInteger<ಪೂರ್ಣಾಂಕ>(10, ಆವೃತ್ತಿ)) {
          error(I.get_ಮೌಲ್ಯ(), "expected integer");
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }
        ಆದರೆ (ಆವೃತ್ತಿ < 0) {
          error(I.get_ಮೌಲ್ಯ(), "invalid version number");
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }
        ಆದರೆ (ಆವೃತ್ತಿ != 0) {
          error(I.get_ಮೌಲ್ಯ(), "version mismatch, expected 0");
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "case-sensitive") {
        ಆದರೆ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), FS->CaseSensitive))
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "overlay-relative") {
        ಆದರೆ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), FS->IsRelativeOverlay))
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "use-external-names") {
        ಆದರೆ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), FS->Use_External_ಹೆಸರುಗಳು))
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "fallthrough") {
        ಆದರೆ (ಕೀಲಿಗಳು_2["redirecting-with"].ನೋಡಿದ) {
          error(I.get_ಮೌಲ್ಯ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }

        ತಾರ್ಕಿಕ ShouldFallthrough = ಸುಳ್ಳು;
        ಆದರೆ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), ShouldFallthrough))
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

        ಆದರೆ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::Redirect_ಬಗೆ::Fallthrough;
        } ಇಲ್ಲದಿದ್ದರೆ {
          FS->Redirection = RedirectingFileSystem::Redirect_ಬಗೆ::RedirectOnly;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "redirecting-with") {
        ಆದರೆ (ಕೀಲಿಗಳು_2["fallthrough"].ನೋಡಿದ) {
          error(I.get_ಮೌಲ್ಯ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }

        ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ ಬಗೆ_2 = parse_Redirect_ಬಗೆ(I.get_ಮೌಲ್ಯ())) {
          FS->Redirection = *ಬಗೆ_2;
        } ಇಲ್ಲದಿದ್ದರೆ {
          error(I.get_ಮೌಲ್ಯ(), "expected valid redirect kind");
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೀಲಿ == "root-relative") {
        ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ ಬಗೆ_2 = parse_ಬೇರು_Relative_ಬಗೆ(I.get_ಮೌಲ್ಯ())) {
          FS->ಬೇರು_Relative = *ಬಗೆ_2;
        } ಇಲ್ಲದಿದ್ದರೆ {
          error(I.get_ಮೌಲ್ಯ(), "expected valid root-relative kind");
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }
      } ಇಲ್ಲದಿದ್ದರೆ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ಆದರೆ (ಹರಿವು.failed())
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    ಆದರೆ (!check_Missing_ಕೀಲಿಗಳು(ತುದಿ, ಕೀಲಿಗಳು_2))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &E : ಬೇರು_Entries)
      unique_Overlay_ವೃಕ್ಷ(FS, E.get());

    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್> ಬಫರ್,
                              ಮೂಲ_Mgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_ಮಾರ್ಗ, ಶೂನ್ಯ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ಮೂಲ_Mgr SM;
  yaml::ಹರಿವು ಹರಿವು(ಬಫರ್->get_Mem_ಬಫರ್_Ref(), SM);

  SM.ಗಣ_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = ಹರಿವು.begin();
  yaml::ಗಂಟು *ಬೇರು_2 = DI->get_ಬೇರು();
  ಆದರೆ (DI == ಹರಿವು.ಕೊನೆ() || !ಬೇರು_2) {
    SM.Print_ಸಂದೇಶ(SMLoc(), ಮೂಲ_Mgr::DK_Error, "expected root node");
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }

  RedirectingFileSystemParser P(ಹರಿವು);

  std::unique_ptr<RedirectingFileSystem> FS(
      ಹೊಸ RedirectingFileSystem(ExternalFS));

  ಆದರೆ (!YAMLFile_ಮಾರ್ಗ.ಖಾಲಿಗಳು()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(YAMLFile_ಮಾರ್ಗ);
    std::error_ಸಂಕೇತ EC = FS->makeAbsolute(OverlayAbsDir);
    ಪ್ರತಿಜ್ಞಾ(!EC && "Overlay dir final path must be absolute");
    (ಶೂನ್ಯ)EC;
    FS->ಗಣ_Overlay_File_Dir(OverlayAbsDir);
  }

  ಆದರೆ (!P.parse(ಬೇರು_2, FS.get()))
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  ಹಿಂತಿರುಗಿ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    ತಾರ್ಕಿಕ Use_External_ಹೆಸರುಗಳು, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      ಹೊಸ RedirectingFileSystem(ExternalFS));
  FS->Use_External_ಹೆಸರುಗಳು = Use_External_ಹೆಸರುಗಳು;

  StringMap<RedirectingFileSystem::ನಮೂದು *> Entries;

  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &Mapping : llvm::ಹಿಮ್ಮುಖ(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.ಮೊದಲ_2);
    SmallString<128> ಗಮ್ಯ = StringRef(Mapping.ಎರಡನೇ_2);
    {
      ಸ್ವಯಂಚಾಲಿತ EC = ExternalFS->makeAbsolute(From);
      (ಶೂನ್ಯ)EC;
      ಪ್ರತಿಜ್ಞಾ(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::ನಮೂದು *&ಗಮ್ಯ_ನಮೂದು = Entries[From];
    ಆದರೆ (ಗಮ್ಯ_ನಮೂದು)
      ಮುಂದುವರಿ;

    // Add parent directories.
    RedirectingFileSystem::ನಮೂದು *ಪೋಷಕ_2 = ಶೂನ್ಯಸೂಚಕ;
    StringRef FromDirectory = llvm::sys::ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(From);
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = llvm::sys::ಮಾರ್ಗ_2::begin(FromDirectory),
              E = llvm::sys::ಮಾರ್ಗ_2::ಕೊನೆ(FromDirectory);
         I != E; ++I) {
      ಪೋಷಕ_2 = RedirectingFileSystemParser::lookup_Or_Create_ನಮೂದು(FS.get(), *I,
                                                                ಪೋಷಕ_2);
    }
    ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ_2 && "File without a directory?");
    {
      ಸ್ವಯಂಚಾಲಿತ EC = ExternalFS->makeAbsolute(ಗಮ್ಯ);
      (ಶೂನ್ಯ)EC;
      ಪ್ರತಿಜ್ಞಾ(!EC && "Could not make absolute path");
    }

    // Add the file.
    ಸ್ವಯಂಚಾಲಿತ NewFile = std::make_unique<RedirectingFileSystem::File_ನಮೂದು>(
        llvm::sys::ಮಾರ್ಗ_2::filename(From), ಗಮ್ಯ,
        Use_External_ಹೆಸರುಗಳು ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    ಗಮ್ಯ_ನಮೂದು = NewFile.get();
    cast<RedirectingFileSystem::Directory_ನಮೂದು>(ಪೋಷಕ_2)->ಕೂಡಿಸು_Content(
        std::move(NewFile));
  }

  ಹಿಂತಿರುಗಿ FS;
}

RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ::Lookup_ಫಲಿತಾಂಶ(
    ನಮೂದು *E, sys::ಮಾರ್ಗ_2::const_iterator ಆರಂಭ, sys::ಮಾರ್ಗ_2::const_iterator ಕೊನೆ_2)
    : E(E) {
  ಪ್ರತಿಜ್ಞಾ(E != ಶೂನ್ಯಸೂಚಕ);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_ನಮೂದು>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_ಮಾರ್ಗ());
    sys::ಮಾರ್ಗ_2::append(Redirect, ಆರಂಭ, ಕೊನೆ_2,
                      getExistingStyle(DRE->get_External_Contents_ಮಾರ್ಗ()));
    ExternalRedirect = std::string(Redirect);
  }
}

ಶೂನ್ಯ RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ::get_ಮಾರ್ಗ(
    llvm::Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) ಸ್ಥಿರಾಂಕ {
  ಫಲಿತಾಂಶ.clear();
  ಗಾಗಿ (ನಮೂದು *ಪೋಷಕ_2 : Parents)
    llvm::sys::ಮಾರ್ಗ_2::append(ಫಲಿತಾಂಶ, ಪೋಷಕ_2->getName());
  llvm::sys::ಮಾರ್ಗ_2::append(ಫಲಿತಾಂಶ, E->getName());
}

std::error_ಸಂಕೇತ RedirectingFileSystem::makeCanonicalForLookup(
    Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಮಾರ್ಗ) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (std::error_ಸಂಕೇತ EC = makeAbsolute(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ EC;

  llvm::SmallString<256> Canonical_ಮಾರ್ಗ =
      canonicalize(StringRef(ಮಾರ್ಗ.ದತ್ತಾಂಶ(), ಮಾರ್ಗ.ಗಾತ್ರ_3()));
  ಆದರೆ (Canonical_ಮಾರ್ಗ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::invalid_ವಾದ);

  ಮಾರ್ಗ.ನಿಯೋಜಿಸು(Canonical_ಮಾರ್ಗ.begin(), Canonical_ಮಾರ್ಗ.ಕೊನೆ());
  ಹಿಂತಿರುಗಿ {};
}

ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ>
RedirectingFileSystem::lookup_ಮಾರ್ಗ(StringRef ಮಾರ್ಗ) ಸ್ಥಿರಾಂಕ {
  llvm::SmallString<128> Canonical_ಮಾರ್ಗ(ಮಾರ್ಗ);
  ಆದರೆ (std::error_ಸಂಕೇತ EC = makeCanonicalForLookup(Canonical_ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ EC;

  // RedirectOnly means the VFS is always used.
  ಆದರೆ (Usage_Tracking_ಸಕ್ರಿಯ && Redirection == Redirect_ಬಗೆ::RedirectOnly)
    Has_Been_ಬಳಸಿದ = ನಿಜ;

  sys::ಮಾರ್ಗ_2::const_iterator ಆರಂಭ = sys::ಮಾರ್ಗ_2::begin(Canonical_ಮಾರ್ಗ);
  sys::ಮಾರ್ಗ_2::const_iterator ಕೊನೆ_2 = sys::ಮಾರ್ಗ_2::ಕೊನೆ(Canonical_ಮಾರ್ಗ);
  llvm::Small_ಸದಿಶ<ನಮೂದು *, 32> Entries;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &ಬೇರು_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಫಲಿತಾಂಶ =
        lookup_ಮಾರ್ಗ_Impl(ಆರಂಭ, ಕೊನೆ_2, ಬೇರು_2.get(), Entries);
    ಆದರೆ (Usage_Tracking_ಸಕ್ರಿಯ && ಫಲಿತಾಂಶ && isa<Remap_ನಮೂದು>(ಫಲಿತಾಂಶ->E))
      Has_Been_ಬಳಸಿದ = ನಿಜ;
    ಆದರೆ (ಫಲಿತಾಂಶ) {
      ಫಲಿತಾಂಶ->Parents = std::move(Entries);
      ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
    }

    ಆದರೆ (ಫಲಿತಾಂಶ.getError() != llvm::errc::no_such_file_or_directory)
      ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  }
  ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ>
RedirectingFileSystem::lookup_ಮಾರ್ಗ_Impl(
    sys::ಮಾರ್ಗ_2::const_iterator ಆರಂಭ, sys::ಮಾರ್ಗ_2::const_iterator ಕೊನೆ_2,
    RedirectingFileSystem::ನಮೂದು *From,
    llvm::Small_ಸದಿಶ_Impl<ನಮೂದು *> &Entries) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(!isTraversalComponent(*ಆರಂಭ) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  ಆದರೆ (!FromName.ಖಾಲಿಗಳು()) {
    ಆದರೆ (!ಮಾರ್ಗ_Component_Matches(*ಆರಂಭ, FromName))
      ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::no_such_file_or_directory);

    ++ಆರಂಭ;

    ಆದರೆ (ಆರಂಭ == ಕೊನೆ_2) {
      // Match!
      ಹಿಂತಿರುಗಿ Lookup_ಫಲಿತಾಂಶ(From, ಆರಂಭ, ಕೊನೆ_2);
    }
  }

  ಆದರೆ (isa<RedirectingFileSystem::File_ನಮೂದು>(From))
    ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::not_a_directory);

  ಆದರೆ (isa<RedirectingFileSystem::Directory_Remap_ನಮೂದು>(From))
    ಹಿಂತಿರುಗಿ Lookup_ಫಲಿತಾಂಶ(From, ಆರಂಭ, ಕೊನೆ_2);

  ಸ್ವಯಂಚಾಲಿತ *DE = cast<RedirectingFileSystem::Directory_ನಮೂದು>(From);
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ std::unique_ptr<RedirectingFileSystem::ನಮೂದು> &Dir_ನಮೂದು :
       llvm::make_range(DE->contents_begin(), DE->contents_ಕೊನೆ())) {
    Entries.ತಳ್ಳು_ಹಿಂಭಾಗ(From);
    ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಫಲಿತಾಂಶ =
        lookup_ಮಾರ್ಗ_Impl(ಆರಂಭ, ಕೊನೆ_2, Dir_ನಮೂದು.get(), Entries);
    ಆದರೆ (ಫಲಿತಾಂಶ || ಫಲಿತಾಂಶ.getError() != llvm::errc::no_such_file_or_directory)
      ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
    Entries.ತೆಗೆ_ಹಿಂಭಾಗ();
  }

  ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::no_such_file_or_directory);
}

ಸ್ಥಿರ ಸ್ಥಿತಿ_2 get_Redirected_File_ಸ್ಥಿತಿ(ಸ್ಥಿರಾಂಕ Twine &Original_ಮಾರ್ಗ,
                                      ತಾರ್ಕಿಕ Use_External_ಹೆಸರುಗಳು,
                                      ಸ್ಥಿತಿ_2 External_ಸ್ಥಿತಿ) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  ಆದರೆ (External_ಸ್ಥಿತಿ.ExposesExternalVFSPath)
    ಹಿಂತಿರುಗಿ External_ಸ್ಥಿತಿ;

  ಸ್ಥಿತಿ_2 S = External_ಸ್ಥಿತಿ;
  ಆದರೆ (!Use_External_ಹೆಸರುಗಳು)
    S = ಸ್ಥಿತಿ_2::copyWithNewName(S, Original_ಮಾರ್ಗ);
  ಇಲ್ಲದಿದ್ದರೆ
    S.ExposesExternalVFSPath = ನಿಜ;
  ಹಿಂತಿರುಗಿ S;
}

ErrorOr<ಸ್ಥಿತಿ_2> RedirectingFileSystem::ಸ್ಥಿತಿ_3(
    ಸ್ಥಿರಾಂಕ Twine &Lookup_ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ Twine &Original_ಮಾರ್ಗ,
    ಸ್ಥಿರಾಂಕ RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ &ಫಲಿತಾಂಶ) {
  ಆದರೆ (std::optional<StringRef> ExtRedirect = ಫಲಿತಾಂಶ.getExternalRedirect()) {
    SmallString<256> Remapped_ಮಾರ್ಗ((*ExtRedirect).str());
    ಆದರೆ (std::error_ಸಂಕೇತ EC = makeAbsolute(Remapped_ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ EC;

    ErrorOr<ಸ್ಥಿತಿ_2> S = ExternalFS->ಸ್ಥಿತಿ_3(Remapped_ಮಾರ್ಗ);
    ಆದರೆ (!S)
      ಹಿಂತಿರುಗಿ S;
    S = ಸ್ಥಿತಿ_2::copyWithNewName(*S, *ExtRedirect);
    ಸ್ವಯಂಚಾಲಿತ *RE = cast<RedirectingFileSystem::Remap_ನಮೂದು>(ಫಲಿತಾಂಶ.E);
    ಹಿಂತಿರುಗಿ get_Redirected_File_ಸ್ಥಿತಿ(Original_ಮಾರ್ಗ,
                                   RE->useExternalName(Use_External_ಹೆಸರುಗಳು), *S);
  }

  ಸ್ವಯಂಚಾಲಿತ *DE = cast<RedirectingFileSystem::Directory_ನಮೂದು>(ಫಲಿತಾಂಶ.E);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2::copyWithNewName(DE->get_ಸ್ಥಿತಿ(), Lookup_ಮಾರ್ಗ);
}

ErrorOr<ಸ್ಥಿತಿ_2>
RedirectingFileSystem::get_External_ಸ್ಥಿತಿ(ಸ್ಥಿರಾಂಕ Twine &Lookup_ಮಾರ್ಗ,
                                         ಸ್ಥಿರಾಂಕ Twine &Original_ಮಾರ್ಗ) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ ಫಲಿತಾಂಶ = ExternalFS->ಸ್ಥಿತಿ_3(Lookup_ಮಾರ್ಗ);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  ಆದರೆ (!ಫಲಿತಾಂಶ || ಫಲಿತಾಂಶ->ExposesExternalVFSPath)
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2::copyWithNewName(ಫಲಿತಾಂಶ.get(), Original_ಮಾರ್ಗ);
}

ErrorOr<ಸ್ಥಿತಿ_2> RedirectingFileSystem::ಸ್ಥಿತಿ_3(ಸ್ಥಿರಾಂಕ Twine &Original_ಮಾರ್ಗ) {
  SmallString<256> ಮಾರ್ಗ;
  Original_ಮಾರ್ಗ.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  ಆದರೆ (std::error_ಸಂಕೇತ EC = makeAbsolute(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ EC;

  ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<ಸ್ಥಿತಿ_2> S = get_External_ಸ್ಥಿತಿ(ಮಾರ್ಗ, Original_ಮಾರ್ಗ);
    ಆದರೆ (S)
      ಹಿಂತಿರುಗಿ S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಫಲಿತಾಂಶ = lookup_ಮಾರ್ಗ(ಮಾರ್ಗ);
  ಆದರೆ (!ಫಲಿತಾಂಶ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallthrough &&
        is_File_Not_ಸಿಕ್ಕಿತು(ಫಲಿತಾಂಶ.getError()))
      ಹಿಂತಿರುಗಿ get_External_ಸ್ಥಿತಿ(ಮಾರ್ಗ, Original_ಮಾರ್ಗ);
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ.getError();
  }

  ErrorOr<ಸ್ಥಿತಿ_2> S = ಸ್ಥಿತಿ_3(ಮಾರ್ಗ, Original_ಮಾರ್ಗ, *ಫಲಿತಾಂಶ);
  ಆದರೆ (!S && Redirection == Redirect_ಬಗೆ::Fallthrough &&
      is_File_Not_ಸಿಕ್ಕಿತು(S.getError(), ಫಲಿತಾಂಶ->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ಹಿಂತಿರುಗಿ get_External_ಸ್ಥಿತಿ(ಮಾರ್ಗ, Original_ಮಾರ್ಗ);
  }

  ಹಿಂತಿರುಗಿ S;
}

ತಾರ್ಕಿಕ RedirectingFileSystem::exists(ಸ್ಥಿರಾಂಕ Twine &Original_ಮಾರ್ಗ) {
  SmallString<256> ಮಾರ್ಗ;
  Original_ಮಾರ್ಗ.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  ಆದರೆ (makeAbsolute(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ಆದರೆ (ExternalFS->exists(ಮಾರ್ಗ))
      ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಫಲಿತಾಂಶ = lookup_ಮಾರ್ಗ(ಮಾರ್ಗ);
  ಆದರೆ (!ಫಲಿತಾಂಶ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallthrough &&
        is_File_Not_ಸಿಕ್ಕಿತು(ಫಲಿತಾಂಶ.getError()))
      ಹಿಂತಿರುಗಿ ExternalFS->exists(ಮಾರ್ಗ);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  std::optional<StringRef> ExtRedirect = ಫಲಿತಾಂಶ->getExternalRedirect();
  ಆದರೆ (!ExtRedirect) {
    ಪ್ರತಿಜ್ಞಾ(isa<RedirectingFileSystem::Directory_ನಮೂದು>(ಫಲಿತಾಂಶ->E));
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  SmallString<256> Remapped_ಮಾರ್ಗ((*ExtRedirect).str());
  ಆದರೆ (makeAbsolute(Remapped_ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆದರೆ (ExternalFS->exists(Remapped_ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ಹಿಂತಿರುಗಿ ExternalFS->exists(ಮಾರ್ಗ);
  }

  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ನಾಮಾಕಾಶ {

/// Provide a file wrapper with an overriden status.
ವರ್ಗ File_With_Fixed_ಸ್ಥಿತಿ : ಸಾರ್ವಜನಿಕ File {
  std::unique_ptr<File> InnerFile;
  ಸ್ಥಿತಿ_2 S;

ಸಾರ್ವಜನಿಕ:
  File_With_Fixed_ಸ್ಥಿತಿ(std::unique_ptr<File> InnerFile, ಸ್ಥಿತಿ_2 S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<ಸ್ಥಿತಿ_2> ಸ್ಥಿತಿ_3() override { ಹಿಂತಿರುಗಿ S; }
  ErrorOr<std::unique_ptr<llvm::ಸ್ಮರಣೆ_ಬಫರ್>>

  get_ಬಫರ್(ಸ್ಥಿರಾಂಕ Twine &Name, int64_t File_ಗಾತ್ರ, ತಾರ್ಕಿಕ RequiresNullTerminator,
            ತಾರ್ಕಿಕ IsVolatile) override {
    ಹಿಂತಿರುಗಿ InnerFile->get_ಬಫರ್(Name, File_ಗಾತ್ರ, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_ಸಂಕೇತ close() override { ಹಿಂತಿರುಗಿ InnerFile->close(); }

  ಶೂನ್ಯ ಗಣ_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ) override { S = S.copyWithNewName(S, ಮಾರ್ಗ); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_ಮಾರ್ಗ(ErrorOr<std::unique_ptr<File>> ಫಲಿತಾಂಶ, ಸ್ಥಿರಾಂಕ Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  ಆದರೆ (!ಫಲಿತಾಂಶ || (*ಫಲಿತಾಂಶ)->ಸ್ಥಿತಿ_3()->ExposesExternalVFSPath)
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;

  ErrorOr<std::unique_ptr<File>> F = std::move(*ಫಲಿತಾಂಶ);
  ಸ್ವಯಂಚಾಲಿತ Name = F->get()->getName();
  ಆದರೆ (Name && Name.get() != P.str())
    F->get()->ಗಣ_ಮಾರ್ಗ(P);
  ಹಿಂತಿರುಗಿ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ಸ್ಥಿರಾಂಕ Twine &Original_ಮಾರ್ಗ) {
  SmallString<256> ಮಾರ್ಗ;
  Original_ಮಾರ್ಗ.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  ಆದರೆ (std::error_ಸಂಕೇತ EC = makeAbsolute(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ EC;

  ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ಸ್ವಯಂಚಾಲಿತ F = File::get_With_ಮಾರ್ಗ(ExternalFS->openFileForRead(ಮಾರ್ಗ), Original_ಮಾರ್ಗ);
    ಆದರೆ (F)
      ಹಿಂತಿರುಗಿ F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಫಲಿತಾಂಶ = lookup_ಮಾರ್ಗ(ಮಾರ್ಗ);
  ಆದರೆ (!ಫಲಿತಾಂಶ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallthrough &&
        is_File_Not_ಸಿಕ್ಕಿತು(ಫಲಿತಾಂಶ.getError()))
      ಹಿಂತಿರುಗಿ File::get_With_ಮಾರ್ಗ(ExternalFS->openFileForRead(ಮಾರ್ಗ), Original_ಮಾರ್ಗ);
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ.getError();
  }

  ಆದರೆ (!ಫಲಿತಾಂಶ->getExternalRedirect()) // FIXME: errc::not_a_file?
    ಹಿಂತಿರುಗಿ make_error_ಸಂಕೇತ(llvm::errc::invalid_ವಾದ);

  StringRef ExtRedirect = *ಫಲಿತಾಂಶ->getExternalRedirect();
  SmallString<256> Remapped_ಮಾರ್ಗ(ExtRedirect.str());
  ಆದರೆ (std::error_ಸಂಕೇತ EC = makeAbsolute(Remapped_ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ EC;

  ಸ್ವಯಂಚಾಲಿತ *RE = cast<RedirectingFileSystem::Remap_ನಮೂದು>(ಫಲಿತಾಂಶ->E);

  ಸ್ವಯಂಚಾಲಿತ ExternalFile =
      File::get_With_ಮಾರ್ಗ(ExternalFS->openFileForRead(Remapped_ಮಾರ್ಗ), ExtRedirect);
  ಆದರೆ (!ExternalFile) {
    ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallthrough &&
        is_File_Not_ಸಿಕ್ಕಿತು(ExternalFile.getError(), ಫಲಿತಾಂಶ->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ಹಿಂತಿರುಗಿ File::get_With_ಮಾರ್ಗ(ExternalFS->openFileForRead(ಮಾರ್ಗ), Original_ಮಾರ್ಗ);
    }
    ಹಿಂತಿರುಗಿ ExternalFile;
  }

  ಸ್ವಯಂಚಾಲಿತ External_ಸ್ಥಿತಿ = (*ExternalFile)->ಸ್ಥಿತಿ_3();
  ಆದರೆ (!External_ಸ್ಥಿತಿ)
    ಹಿಂತಿರುಗಿ External_ಸ್ಥಿತಿ.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  ಸ್ಥಿತಿ_2 S = get_Redirected_File_ಸ್ಥಿತಿ(
      Original_ಮಾರ್ಗ, RE->useExternalName(Use_External_ಹೆಸರುಗಳು), *External_ಸ್ಥಿತಿ);
  ಹಿಂತಿರುಗಿ std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_ಸ್ಥಿತಿ>(std::move(*ExternalFile), S));
}

std::error_ಸಂಕೇತ
RedirectingFileSystem::get_Real_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ Twine &Original_ಮಾರ್ಗ,
                                   Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Output) {
  SmallString<256> ಮಾರ್ಗ;
  Original_ಮಾರ್ಗ.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  ಆದರೆ (std::error_ಸಂಕೇತ EC = makeAbsolute(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ EC;

  ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_ಸಂಕೇತ EC = ExternalFS->get_Real_ಮಾರ್ಗ(ಮಾರ್ಗ, Output);
    ಆದರೆ (!EC)
      ಹಿಂತಿರುಗಿ EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಫಲಿತಾಂಶ = lookup_ಮಾರ್ಗ(ಮಾರ್ಗ);
  ಆದರೆ (!ಫಲಿತಾಂಶ) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallthrough &&
        is_File_Not_ಸಿಕ್ಕಿತು(ಫಲಿತಾಂಶ.getError()))
      ಹಿಂತಿರುಗಿ ExternalFS->get_Real_ಮಾರ್ಗ(ಮಾರ್ಗ, Output);
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ ExtRedirect = ಫಲಿತಾಂಶ->getExternalRedirect()) {
    ಸ್ವಯಂಚಾಲಿತ P = ExternalFS->get_Real_ಮಾರ್ಗ(*ExtRedirect, Output);
    ಆದರೆ (P && Redirection == Redirect_ಬಗೆ::Fallthrough &&
        is_File_Not_ಸಿಕ್ಕಿತು(P, ಫಲಿತಾಂಶ->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ಹಿಂತಿರುಗಿ ExternalFS->get_Real_ಮಾರ್ಗ(ಮಾರ್ಗ, Output);
    }
    ಹಿಂತಿರುಗಿ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  ಆದರೆ (Redirection == Redirect_ಬಗೆ::Fallthrough) {
    ಫಲಿತಾಂಶ->get_ಮಾರ್ಗ(Output);
    ಹಿಂತಿರುಗಿ {};
  }
  ಹಿಂತಿರುಗಿ llvm::errc::invalid_ವಾದ;
}

ಶೂನ್ಯ RedirectingFileSystem::getDirectoryContentRealSources(
    ಸ್ಥಿರಾಂಕ Twine &Dir, Small_ಸದಿಶ_Impl<std::string> &ಹೊರಹೋಗು) {
  SmallString<256> ಮಾರ್ಗ;
  Dir.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);

  ಆದರೆ (makeAbsolute(ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ ConsultsExternalFS = Redirection != Redirect_ಬಗೆ::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಫಲಿತಾಂಶ = lookup_ಮಾರ್ಗ(ಮಾರ್ಗ);
  ಆದರೆ (!ಫಲಿತಾಂಶ) {
    // dir_begin() delegates entirely to ExternalFS.
    ಆದರೆ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(ಮಾರ್ಗ, ಹೊರಹೋಗು);
    ಹಿಂತಿರುಗಿ;
  }

  ಆಯ್ಕೆ (ಫಲಿತಾಂಶ->E->get_ಬಗೆ()) {
  ಸ್ಥಿತಿ EK_File:
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_ಮಾರ್ಗ(*ಫಲಿತಾಂಶ->getExternalRedirect());
    ಆದರೆ (!makeAbsolute(Remapped_ಮಾರ್ಗ))
      ExternalFS->getDirectoryContentRealSources(Remapped_ಮಾರ್ಗ, ಹೊರಹೋಗು);
    ನಿಲ್ಲಿಸು;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  ಆದರೆ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(ಮಾರ್ಗ, ಹೊರಹೋಗು);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್> ಬಫರ್,
                    ಮೂಲ_Mgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_ಮಾರ್ಗ, ಶೂನ್ಯ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ಹಿಂತಿರುಗಿ RedirectingFileSystem::create(std::move(ಬಫರ್), DiagHandler,
                                       YAMLFile_ಮಾರ್ಗ, DiagContext,
                                       std::move(ExternalFS));
}

ಸ್ಥಿರ ಶೂನ್ಯ getVFSEntries(RedirectingFileSystem::ನಮೂದು *SrcE,
                          Small_ಸದಿಶ_Impl<StringRef> &ಮಾರ್ಗ,
                          Small_ಸದಿಶ_Impl<YAMLVFSEntry> &Entries) {
  ಸ್ವಯಂಚಾಲಿತ ಬಗೆ_2 = SrcE->get_ಬಗೆ();
  ಆದರೆ (ಬಗೆ_2 == RedirectingFileSystem::EK_Directory) {
    ಸ್ವಯಂಚಾಲಿತ *DE = dyn_cast<RedirectingFileSystem::Directory_ನಮೂದು>(SrcE);
    ಪ್ರತಿಜ್ಞಾ(DE && "Must be a directory");
    ಗಾಗಿ (std::unique_ptr<RedirectingFileSystem::ನಮೂದು> &Sub_ನಮೂದು :
         llvm::make_range(DE->contents_begin(), DE->contents_ಕೊನೆ())) {
      ಮಾರ್ಗ.ತಳ್ಳು_ಹಿಂಭಾಗ(Sub_ನಮೂದು->getName());
      getVFSEntries(Sub_ನಮೂದು.get(), ಮಾರ್ಗ, Entries);
      ಮಾರ್ಗ.ತೆಗೆ_ಹಿಂಭಾಗ();
    }
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (ಬಗೆ_2 == RedirectingFileSystem::EK_DirectoryRemap) {
    ಸ್ವಯಂಚಾಲಿತ *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_ನಮೂದು>(SrcE);
    ಪ್ರತಿಜ್ಞಾ(DR && "Must be a directory remap");
    SmallString<128> VPath;
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &Comp : ಮಾರ್ಗ)
      llvm::sys::ಮಾರ್ಗ_2::append(VPath, Comp);
    Entries.ತಳ್ಳು_ಹಿಂಭಾಗ(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_ಮಾರ್ಗ()));
    ಹಿಂತಿರುಗಿ;
  }

  ಪ್ರತಿಜ್ಞಾ(ಬಗೆ_2 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  ಸ್ವಯಂಚಾಲಿತ *FE = dyn_cast<RedirectingFileSystem::File_ನಮೂದು>(SrcE);
  ಪ್ರತಿಜ್ಞಾ(FE && "Must be a file");
  SmallString<128> VPath;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &Comp : ಮಾರ್ಗ)
    llvm::sys::ಮಾರ್ಗ_2::append(VPath, Comp);
  Entries.ತಳ್ಳು_ಹಿಂಭಾಗ(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_ಮಾರ್ಗ()));
}

ಶೂನ್ಯ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_ಸದಿಶ_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_ಫಲಿತಾಂಶ> ಬೇರು_ಫಲಿತಾಂಶ = VFS.lookup_ಮಾರ್ಗ("/");
  ಆದರೆ (!ಬೇರು_ಫಲಿತಾಂಶ)
    ಹಿಂತಿರುಗಿ;
  Small_ಸದಿಶ<StringRef, 8> Components;
  Components.ತಳ್ಳು_ಹಿಂಭಾಗ("/");
  getVFSEntries(ಬೇರು_ಫಲಿತಾಂಶ->E, Components, CollectedEntries);
}

UniqueID vfs::get_ಮುಂದಿನ_Virtual_Unique_ID() {
  ಸ್ಥಿರ std::atomic<ಚಿಹ್ನೆರಹಿತ> UID;
  ಚಿಹ್ನೆರಹಿತ ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ಹಿಂತಿರುಗಿ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

ಶೂನ್ಯ YAMLVFSWriter::ಕೂಡಿಸು_ನಮೂದು(StringRef Virtual_ಮಾರ್ಗ, StringRef Real_ಮಾರ್ಗ,
                             ತಾರ್ಕಿಕ IsDirectory) {
  ಪ್ರತಿಜ್ಞಾ(sys::ಮಾರ್ಗ_2::is_absolute(Virtual_ಮಾರ್ಗ) && "virtual path not absolute");
  ಪ್ರತಿಜ್ಞಾ(sys::ಮಾರ್ಗ_2::is_absolute(Real_ಮಾರ್ಗ) && "real path not absolute");
  ಪ್ರತಿಜ್ಞಾ(!ಮಾರ್ಗ_Has_Traversal(Virtual_ಮಾರ್ಗ) && "path traversal is not supported");
  Mappings.emplace_ಹಿಂಭಾಗ(Virtual_ಮಾರ್ಗ, Real_ಮಾರ್ಗ, IsDirectory);
}

ಶೂನ್ಯ YAMLVFSWriter::ಕೂಡಿಸು_File_Mapping(StringRef Virtual_ಮಾರ್ಗ, StringRef Real_ಮಾರ್ಗ) {
  ಕೂಡಿಸು_ನಮೂದು(Virtual_ಮಾರ್ಗ, Real_ಮಾರ್ಗ, /*IsDirectory=*/ಸುಳ್ಳು);
}

ಶೂನ್ಯ YAMLVFSWriter::ಕೂಡಿಸು_Directory_Mapping(StringRef Virtual_ಮಾರ್ಗ,
                                        StringRef Real_ಮಾರ್ಗ) {
  ಕೂಡಿಸು_ನಮೂದು(Virtual_ಮಾರ್ಗ, Real_ಮಾರ್ಗ, /*IsDirectory=*/ನಿಜ);
}

ನಾಮಾಕಾಶ {

ವರ್ಗ JSONWriter {
  llvm::raw_ostream &OS;
  Small_ಸದಿಶ<StringRef, 16> Dir_ರಾಶಿ;

  ಚಿಹ್ನೆರಹಿತ getDirIndent() { ಹಿಂತಿರುಗಿ 4 * Dir_ರಾಶಿ.ಗಾತ್ರ_3(); }
  ಚಿಹ್ನೆರಹಿತ getFileIndent() { ಹಿಂತಿರುಗಿ 4 * (Dir_ರಾಶಿ.ಗಾತ್ರ_3() + 1); }
  ತಾರ್ಕಿಕ contained_ಒಳ(StringRef ಪೋಷಕ_2, StringRef ಮಾರ್ಗ);
  StringRef containedPart(StringRef ಪೋಷಕ_2, StringRef ಮಾರ್ಗ);
  ಶೂನ್ಯ ಆರಂಭ_Directory(StringRef ಮಾರ್ಗ);
  ಶೂನ್ಯ ಕೊನೆ_Directory();
  ಶೂನ್ಯ write_ನಮೂದು(StringRef VPath, StringRef RPath);

ಸಾರ್ವಜನಿಕ:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  ಶೂನ್ಯ ಲಿಖ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<ತಾರ್ಕಿಕ> Use_External_ಹೆಸರುಗಳು,
             std::optional<ತಾರ್ಕಿಕ> IsCaseSensitive,
             std::optional<ತಾರ್ಕಿಕ> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

ತಾರ್ಕಿಕ JSONWriter::contained_ಒಳ(StringRef ಪೋಷಕ_2, StringRef ಮಾರ್ಗ) {
  ಬಳಸಿ ನಾಮಾಕಾಶ llvm::sys;

  // Compare each path component.
  ಸ್ವಯಂಚಾಲಿತ IParent = ಮಾರ್ಗ_2::begin(ಪೋಷಕ_2), EParent = ಮಾರ್ಗ_2::ಕೊನೆ(ಪೋಷಕ_2);
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ IChild = ಮಾರ್ಗ_2::begin(ಮಾರ್ಗ), EChild = ಮಾರ್ಗ_2::ಕೊನೆ(ಮಾರ್ಗ);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    ಆದರೆ (*IParent != *IChild)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  // Have we exhausted the parent path?
  ಹಿಂತಿರುಗಿ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef ಪೋಷಕ_2, StringRef ಮಾರ್ಗ) {
  ಪ್ರತಿಜ್ಞಾ(!ಪೋಷಕ_2.ಖಾಲಿಗಳು());
  ಪ್ರತಿಜ್ಞಾ(contained_ಒಳ(ಪೋಷಕ_2, ಮಾರ್ಗ));
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ.substr(ಪೋಷಕ_2.ಗಾತ್ರ_3() + 1);
}

ಶೂನ್ಯ JSONWriter::ಆರಂಭ_Directory(StringRef ಮಾರ್ಗ) {
  StringRef Name =
      Dir_ರಾಶಿ.ಖಾಲಿಗಳು() ? ಮಾರ್ಗ : containedPart(Dir_ರಾಶಿ.ಹಿಂಭಾಗ(), ಮಾರ್ಗ);
  Dir_ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮಾರ್ಗ);
  ಚಿಹ್ನೆರಹಿತ Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

ಶೂನ್ಯ JSONWriter::ಕೊನೆ_Directory() {
  ಚಿಹ್ನೆರಹಿತ Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
}

ಶೂನ್ಯ JSONWriter::write_ನಮೂದು(StringRef VPath, StringRef RPath) {
  ಚಿಹ್ನೆರಹಿತ Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

ಶೂನ್ಯ JSONWriter::ಲಿಖ(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<ತಾರ್ಕಿಕ> Use_External_ಹೆಸರುಗಳು,
                       std::optional<ತಾರ್ಕಿಕ> IsCaseSensitive,
                       std::optional<ತಾರ್ಕಿಕ> IsOverlayRelative,
                       StringRef OverlayDir) {
  ಬಳಸಿ ನಾಮಾಕಾಶ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  ಆದರೆ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  ಆದರೆ (Use_External_ಹೆಸರುಗಳು)
    OS << "  'use-external-names': '" << (*Use_External_ಹೆಸರುಗಳು ? "true" : "false")
       << "',\n";
  ತಾರ್ಕಿಕ UseOverlayRelative = ಸುಳ್ಳು;
  ಆದರೆ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  ಆದರೆ (!Entries.ಖಾಲಿಗಳು()) {
    ಸ್ಥಿರಾಂಕ YAMLVFSEntry &ನಮೂದು = Entries.front();

    ಆರಂಭ_Directory(
      ನಮೂದು.IsDirectory ? ನಮೂದು.VPath : ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(ನಮೂದು.VPath)
    );

    StringRef RPath = ನಮೂದು.RPath;
    ಆದರೆ (UseOverlayRelative) {
      ಪ್ರತಿಜ್ಞಾ(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.ಗಾತ್ರ_3());
    }

    ತಾರ್ಕಿಕ Is_Current_Dir_ಖಾಲಿಗಳು = ನಿಜ;
    ಆದರೆ (!ನಮೂದು.IsDirectory) {
      write_ನಮೂದು(ಮಾರ್ಗ_2::filename(ನಮೂದು.VPath), RPath);
      Is_Current_Dir_ಖಾಲಿಗಳು = ಸುಳ್ಳು;
    }

    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &ನಮೂದು : Entries.ತುಂಡು(1)) {
      StringRef Dir =
          ನಮೂದು.IsDirectory ? ನಮೂದು.VPath : ಮಾರ್ಗ_2::ಪೋಷಕ_ಮಾರ್ಗ(ನಮೂದು.VPath);
      ಆದರೆ (Dir == Dir_ರಾಶಿ.ಹಿಂಭಾಗ()) {
        ಆದರೆ (!Is_Current_Dir_ಖಾಲಿಗಳು) {
          OS << ",\n";
        }
      } ಇಲ್ಲದಿದ್ದರೆ {
        ತಾರ್ಕಿಕ Is_Dir_Popped_From_ರಾಶಿ = ಸುಳ್ಳು;
        ಆಗ (!Dir_ರಾಶಿ.ಖಾಲಿಗಳು() && !contained_ಒಳ(Dir_ರಾಶಿ.ಹಿಂಭಾಗ(), Dir)) {
          OS << "\n";
          ಕೊನೆ_Directory();
          Is_Dir_Popped_From_ರಾಶಿ = ನಿಜ;
        }
        ಆದರೆ (Is_Dir_Popped_From_ರಾಶಿ || !Is_Current_Dir_ಖಾಲಿಗಳು) {
          OS << ",\n";
        }
        ಆರಂಭ_Directory(Dir);
        Is_Current_Dir_ಖಾಲಿಗಳು = ನಿಜ;
      }
      StringRef RPath = ನಮೂದು.RPath;
      ಆದರೆ (UseOverlayRelative) {
        ಪ್ರತಿಜ್ಞಾ(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.ಗಾತ್ರ_3());
      }
      ಆದರೆ (!ನಮೂದು.IsDirectory) {
        write_ನಮೂದು(ಮಾರ್ಗ_2::filename(ನಮೂದು.VPath), RPath);
        Is_Current_Dir_ಖಾಲಿಗಳು = ಸುಳ್ಳು;
      }
    }

    ಆಗ (!Dir_ರಾಶಿ.ಖಾಲಿಗಳು()) {
      OS << "\n";
      ಕೊನೆ_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

ಶೂನ್ಯ YAMLVFSWriter::ಲಿಖ(llvm::raw_ostream &OS) {
  llvm::ವಿಂಗಡಿಸು(Mappings, [](ಸ್ಥಿರಾಂಕ YAMLVFSEntry &LHS, ಸ್ಥಿರಾಂಕ YAMLVFSEntry &RHS) {
    ಹಿಂತಿರುಗಿ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).ಲಿಖ(Mappings, Use_External_ಹೆಸರುಗಳು, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ, std::error_ಸಂಕೇತ &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(ಮಾರ್ಗ, EC);
  ಆದರೆ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_ಸಂಕೇತ &EC) {
  ಪ್ರತಿಜ್ಞಾ(FS && State && !State->ರಾಶಿ.ಖಾಲಿಗಳು() && "incrementing past end");
  ಪ್ರತಿಜ್ಞಾ(!State->ರಾಶಿ.ಹಿಂಭಾಗ()->ಮಾರ್ಗ_2().ಖಾಲಿಗಳು() && "non-canonical end iterator");
  vfs::directory_iterator ಕೊನೆ_2;

  ಆದರೆ (State->Has_No_ತಳ್ಳು_Request)
    State->Has_No_ತಳ್ಳು_Request = ಸುಳ್ಳು;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (State->ರಾಶಿ.ಹಿಂಭಾಗ()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ರಾಶಿ.ಹಿಂಭಾಗ()->ಮಾರ್ಗ_2(), EC);
      ಆದರೆ (I != ಕೊನೆ_2) {
        State->ರಾಶಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(I);
        ಹಿಂತಿರುಗಿ *ಇದು;
      }
    }
  }

  ಆಗ (!State->ರಾಶಿ.ಖಾಲಿಗಳು() && State->ರಾಶಿ.ಹಿಂಭಾಗ().increment(EC) == ಕೊನೆ_2)
    State->ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();

  ಆದರೆ (State->ರಾಶಿ.ಖಾಲಿಗಳು())
    State.reset(); // end iterator

  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ FileSystem::ID = 0;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ OverlayFileSystem::ID = 0;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ProxyFileSystem::ID = 0;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ಒಳ_ಸ್ಮರಣೆ_File_System::ID = 0;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ RedirectingFileSystem::ID = 0;

ಚಿಹ್ನೆರಹಿತ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_ಎಣಿಕೆ(
    ಸ್ಥಿರಾಂಕ FileSystem *FS) {
  ಹಿಂತಿರುಗಿ FS->Use_ಎಣಿಕೆ();
}

ಶೂನ್ಯ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

ಶೂನ್ಯ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
