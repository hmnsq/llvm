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

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(ಧ್ರುವ file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(ಧ್ರುವ Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(ಧ್ರುವ Status &In, uint64_t NewSize) {
  ಪ್ರತ್ಯಾಗಮ Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ಧ್ರುವ Status &In, ಧ್ರುವ Twine &NewName) {
  ಪ್ರತ್ಯಾಗಮ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ಧ್ರುವ file_status &In, ಧ್ರುವ Twine &NewName) {
  ಪ್ರತ್ಯಾಗಮ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

ತಾರ್ಕಿಕ Status::equivalent(ಧ್ರುವ Status &Other) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(isStatusKnown() && Other.isStatusKnown());
  ಪ್ರತ್ಯಾಗಮ getUniqueID() == Other.getUniqueID();
}

ತಾರ್ಕಿಕ Status::isDirectory() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Type == file_type::directory_file; }

ತಾರ್ಕಿಕ Status::isRegularFile() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Type == file_type::regular_file; }

ತಾರ್ಕಿಕ Status::isOther() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

ತಾರ್ಕಿಕ Status::isSymlink() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Type == file_type::symlink_file; }

ತಾರ್ಕಿಕ Status::isStatusKnown() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Type != file_type::status_error; }

ತಾರ್ಕಿಕ Status::exists() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = ಸ್ವಭಾವ;

FileSystem::~FileSystem() = ಸ್ವಭಾವ;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(ಧ್ರುವ llvm::Twine &Name, int64_t FileSize,
                             ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile,
                             ತಾರ್ಕಿಕ IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_ದ್ವಿಮಾನ(Name);
  ಯದಿ (!F)
    ಪ್ರತ್ಯಾಗಮ F.getError();

  ಪ್ರತ್ಯಾಗಮ (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<ವರ್ಣ> &Path) ಧ್ರುವ {
  ಯದಿ (llvm::sys::path::is_absolute(Path))
    ಪ್ರತ್ಯಾಗಮ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ಯದಿ (!WorkingDir)
    ಪ್ರತ್ಯಾಗಮ WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  ಪ್ರತ್ಯಾಗಮ {};
}

std::error_code FileSystem::getRealPath(ಧ್ರುವ Twine &Path,
                                        SmallVectorImpl<ವರ್ಣ> &Output) {
  ಪ್ರತ್ಯಾಗಮ errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(ಧ್ರುವ Twine &Path, ತಾರ್ಕಿಕ &Result) {
  ಪ್ರತ್ಯಾಗಮ errc::operation_not_permitted;
}

ತಾರ್ಕಿಕ FileSystem::exists(ಧ್ರುವ Twine &Path) {
  auto Status = status(Path);
  ಪ್ರತ್ಯಾಗಮ Status && Status->exists();
}

llvm::ErrorOr<ತಾರ್ಕಿಕ> FileSystem::equivalent(ಧ್ರುವ Twine &A, ಧ್ರುವ Twine &B) {
  auto StatusA = status(A);
  ಯದಿ (!StatusA)
    ಪ್ರತ್ಯಾಗಮ StatusA.getError();
  auto StatusB = status(B);
  ಯದಿ (!StatusB)
    ಪ್ರತ್ಯಾಗಮ StatusB.getError();
  ಪ್ರತ್ಯಾಗಮ StatusA->equivalent(*StatusB);
}

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
ಶೂನ್ಯ FileSystem::dump() ಧ್ರುವ { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
ಸ್ಥಿರ ತಾರ್ಕಿಕ isTraversalComponent(StringRef Component) {
  ಪ್ರತ್ಯಾಗಮ Component == ".." || Component == ".";
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ pathHasTraversal(StringRef Path) {
  using ನಾಮಾಕಾಶ llvm::sys;

  ಪ್ರತಿ (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    ಯದಿ (isTraversalComponent(Comp))
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

ನಾಮಾಕಾಶ {

/// Wrapper around a raw file descriptor.
ವರ್ಗ RealFile : ಸಾರ್ವಜನಿಕ File {
  friend ವರ್ಗ RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    ಪ್ರತಿಜ್ಞಾ(FD.isValid() && "Invalid or inactive file descriptor");
  }

ಸಾರ್ವಜನಿಕ:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(ಧ್ರುವ Twine &Name,
                                                   int64_t FileSize,
                                                   ತಾರ್ಕಿಕ RequiresNullTerminator,
                                                   ತಾರ್ಕಿಕ IsVolatile) override;
  std::error_code close() override;
  ಶೂನ್ಯ setPath(ಧ್ರುವ Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ(FD.isValid() && "cannot stat closed file");
  ಯದಿ (!S.isStatusKnown()) {
    file_status RealStatus;
    ಯದಿ (std::error_code EC = sys::fs::status(FD, RealStatus))
      ಪ್ರತ್ಯಾಗಮ EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  ಪ್ರತ್ಯಾಗಮ S;
}

ErrorOr<std::string> RealFile::getName() {
  ಪ್ರತ್ಯಾಗಮ RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(ಧ್ರುವ Twine &Name, int64_t FileSize,
                    ತಾರ್ಕಿಕ RequiresNullTerminator, ತಾರ್ಕಿಕ IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ(FD.isValid() && "cannot get buffer for closed file");
  ಪ್ರತ್ಯಾಗಮ MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ಪ್ರತ್ಯಾಗಮ EC;
}

ಶೂನ್ಯ RealFile::setPath(ಧ್ರುವ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  ಯದಿ (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
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
  explicit RealFileSystem(ತಾರ್ಕಿಕ LinkCWDToProcess) {
    ಯದಿ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      ಯದಿ (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      ಅನ್ಯಥಾ ಯದಿ (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      ಅನ್ಯಥಾ
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(ಧ್ರುವ Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ಧ್ರುವ Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ದ್ವಿಮಾನ(ಧ್ರುವ Twine &Path) override;
  directory_iterator dir_begin(ಧ್ರುವ Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ಧ್ರುವ override;
  std::error_code setCurrentWorkingDirectory(ಧ್ರುವ Twine &Path) override;
  std::error_code isLocal(ಧ್ರುವ Twine &Path, ತಾರ್ಕಿಕ &Result) override;
  ಶೂನ್ಯ
  getDirectoryContentRealSources(ಧ್ರುವ Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.ತಳ್ಳು_back(Dir.str());
  }
  std::error_code getRealPath(ಧ್ರುವ Twine &Path,
                              SmallVectorImpl<ವರ್ಣ> &Output) override;

ರಕ್ಷಿತ:
  ಶೂನ್ಯ printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) ಧ್ರುವ override;

ಗುಪ್ತ:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(ಧ್ರುವ Twine &Path, SmallVectorImpl<ವರ್ಣ> &Storage) ಧ್ರುವ {
    ಯದಿ (!WD || !*WD)
      ಪ್ರತ್ಯಾಗಮ Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    ಪ್ರತ್ಯಾಗಮ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ಧ್ರುವ Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    ಯದಿ (!FDOrErr)
      ಪ್ರತ್ಯಾಗಮ errorToErrorCode(FDOrErr.takeError());
    ಪ್ರತ್ಯಾಗಮ std::unique_ptr<File>(
        ನೂತನ RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  ಸಂರಚನಾ WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(ಧ್ರುವ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  ಯದಿ (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    ಪ್ರತ್ಯಾಗಮ EC;
  ಪ್ರತ್ಯಾಗಮ Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ಧ್ರುವ Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತ್ಯಾಗಮ openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ದ್ವಿಮಾನ(ಧ್ರುವ Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತ್ಯಾಗಮ openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ಧ್ರುವ {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಯದಿ (WD && *WD)
    ಪ್ರತ್ಯಾಗಮ std::string(WD->get().Specified);
  ಯದಿ (WD)
    ಪ್ರತ್ಯಾಗಮ WD->getError();

  SmallString<128> Dir;
  ಯದಿ (std::error_code EC = llvm::sys::fs::current_path(Dir))
    ಪ್ರತ್ಯಾಗಮ EC;
  ಪ್ರತ್ಯಾಗಮ std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(ಧ್ರುವ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಯದಿ (!WD)
    ಪ್ರತ್ಯಾಗಮ llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  ತಾರ್ಕಿಕ IsDir;
  ಯದಿ (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ಪ್ರತ್ಯಾಗಮ Err;
  ಯದಿ (!IsDir)
    ಪ್ರತ್ಯಾಗಮ std::make_error_code(std::errc::not_a_directory);
  ಯದಿ (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    ಪ್ರತ್ಯಾಗಮ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

std::error_code RealFileSystem::isLocal(ಧ್ರುವ Twine &Path, ತಾರ್ಕಿಕ &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ಪ್ರತ್ಯಾಗಮ llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(ಧ್ರುವ Twine &Path,
                                            SmallVectorImpl<ವರ್ಣ> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ಪ್ರತ್ಯಾಗಮ llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

ಶೂನ್ಯ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) ಧ್ರುವ {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  ಯದಿ (WD)
    OS << "own";
  ಅನ್ಯಥಾ
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  ಸ್ಥಿರ IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(ಸತ್ಯ);
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ಪ್ರತ್ಯಾಗಮ std::make_unique<RealFileSystem>(ಅಸತ್ಯ);
}

ನಾಮಾಕಾಶ {

ವರ್ಗ RealFSDirIter : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

ಸಾರ್ವಜನಿಕ:
  RealFSDirIter(ಧ್ರುವ Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    ಯದಿ (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    ಪ್ರತ್ಯಾಗಮ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ಧ್ರುವ Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ಪ್ರತ್ಯಾಗಮ directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.ತಳ್ಳು_back(std::move(BaseFS));
}

ಶೂನ್ಯ OverlayFileSystem::ತಳ್ಳು_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ತಳ್ಳು_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(ಧ್ರುವ Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ಪ್ರತಿ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    ಯದಿ (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      ಪ್ರತ್ಯಾಗಮ Status;
  }
  ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::no_such_file_or_directory);
}

ತಾರ್ಕಿಕ OverlayFileSystem::exists(ಧ್ರುವ Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ಪ್ರತಿ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ಯದಿ ((*I)->exists(Path))
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ಧ್ರುವ llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ಪ್ರತಿ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    ಯದಿ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ಪ್ರತ್ಯಾಗಮ Result;
  }
  ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ಧ್ರುವ {
  // All file systems are synchronized, just take the first working directory.
  ಪ್ರತ್ಯಾಗಮ FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(ಧ್ರುವ Twine &Path) {
  ಪ್ರತಿ (auto &FS : FSList)
    ಯದಿ (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      ಪ್ರತ್ಯಾಗಮ EC;
  ಪ್ರತ್ಯಾಗಮ {};
}

std::error_code OverlayFileSystem::isLocal(ಧ್ರುವ Twine &Path, ತಾರ್ಕಿಕ &Result) {
  ಪ್ರತಿ (auto &FS : FSList)
    ಯದಿ (FS->exists(Path))
      ಪ್ರತ್ಯಾಗಮ FS->isLocal(Path, Result);
  ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(ಧ್ರುವ Twine &Path,
                                               SmallVectorImpl<ವರ್ಣ> &Output) {
  ಪ್ರತಿ (ಧ್ರುವ auto &FS : FSList)
    ಯದಿ (FS->exists(Path))
      ಪ್ರತ್ಯಾಗಮ FS->getRealPath(Path, Output);
  ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;
}

ಶೂನ್ಯ OverlayFileSystem::getDirectoryContentRealSources(
    ಧ್ರುವ Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  ಪ್ರತಿ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

ಶೂನ್ಯ OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  ಪ್ರತಿ (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

ಶೂನ್ಯ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) ಧ್ರುವ {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  ಯದಿ (Type == PrintType::Summary)
    ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  ಪ್ರತಿ (ಧ್ರುವ auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = ಸ್ವಭಾವ;

ನಾಮಾಕಾಶ {

/// Combines and deduplicates directory entries across multiple file systems.
ವರ್ಗ CombiningDirIterImpl : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(ತಾರ್ಕಿಕ IsFirstTime) {
    ಯಾವತ್ (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.ತೆಗೆ_back();
      ಯದಿ (CurrentDirIter != directory_iterator())
        ಭಙ್ಗ; // found
    }

    ಯದಿ (IsFirstTime && CurrentDirIter == directory_iterator())
      ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;
    ಪ್ರತ್ಯಾಗಮ {};
  }

  std::error_code incrementDirIter(ತಾರ್ಕಿಕ IsFirstTime) {
    ಪ್ರತಿಜ್ಞಾ((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    ಯದಿ (!IsFirstTime)
      CurrentDirIter.increment(EC);
    ಯದಿ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    ಪ್ರತ್ಯಾಗಮ EC;
  }

  std::error_code incrementImpl(ತಾರ್ಕಿಕ IsFirstTime) {
    ಯಾವತ್ (ಸತ್ಯ) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      ಯದಿ (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        ಪ್ರತ್ಯಾಗಮ EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      ಯದಿ (SeenNames.ಸೇರಿಸು(Name).second)
        ಪ್ರತ್ಯಾಗಮ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

ಸಾರ್ವಜನಿಕ:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    ಪ್ರತಿ (ಧ್ರುವ auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      ಯದಿ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ಪ್ರತ್ಯಾಗಮ;
      }
      ಯದಿ (!FEC)
        IterList.ತಳ್ಳು_back(Iter);
    }
    EC = incrementImpl(ಸತ್ಯ);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(ಸತ್ಯ);
  }

  std::error_code increment() override { ಪ್ರತ್ಯಾಗಮ incrementImpl(ಅಸತ್ಯ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ಧ್ರುವ Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ {};
  ಪ್ರತ್ಯಾಗಮ Combined;
}

ಶೂನ್ಯ ProxyFileSystem::anchor() {}

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ vfs {

ನಾಮಾಕಾಶ detail {

ಗಣನಾ In_Memory_ಗಂಟು_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
ವರ್ಗ In_Memory_ಗಂಟು {
  In_Memory_ಗಂಟು_Kind Kind;
  std::string FileName;

ಸಾರ್ವಜನಿಕ:
  In_Memory_ಗಂಟು(llvm::StringRef FileName, In_Memory_ಗಂಟು_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  ಆಭಾಸ ~In_Memory_ಗಂಟು() = ಸ್ವಭಾವ;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  ಆಭಾಸ Status getStatus(ಧ್ರುವ Twine &RequestedName) ಧ್ರುವ = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ FileName; }
  In_Memory_ಗಂಟು_Kind getKind() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Kind; }
  ಆಭಾಸ std::string toString(unsigned Indent) ಧ್ರುವ = 0;
};

ವರ್ಗ InMemoryFile : ಸಾರ್ವಜನಿಕ In_Memory_ಗಂಟು {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

ಸಾರ್ವಜನಿಕ:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_ಗಂಟು(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(ಧ್ರುವ Twine &RequestedName) ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Buffer.get(); }

  std::string toString(unsigned Indent) ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ In_Memory_ಗಂಟು *N) {
    ಪ್ರತ್ಯಾಗಮ N->getKind() == IME_File;
  }
};

ನಾಮಾಕಾಶ {

ವರ್ಗ InMemoryHardLink : ಸಾರ್ವಜನಿಕ In_Memory_ಗಂಟು {
  ಧ್ರುವ InMemoryFile &ResolvedFile;

ಸಾರ್ವಜನಿಕ:
  InMemoryHardLink(StringRef Path, ಧ್ರುವ InMemoryFile &ResolvedFile)
      : In_Memory_ಗಂಟು(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ಧ್ರುವ InMemoryFile &getResolvedFile() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ResolvedFile; }

  Status getStatus(ಧ್ರುವ Twine &RequestedName) ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ In_Memory_ಗಂಟು *N) {
    ಪ್ರತ್ಯಾಗಮ N->getKind() == IME_HardLink;
  }
};

ವರ್ಗ InMemorySymbolicLink : ಸಾರ್ವಜನಿಕ In_Memory_ಗಂಟು {
  std::string TargetPath;
  Status Stat;

ಸಾರ್ವಜನಿಕ:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_ಗಂಟು(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(ಧ್ರುವ Twine &RequestedName) ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ TargetPath; }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ In_Memory_ಗಂಟು *N) {
    ಪ್ರತ್ಯಾಗಮ N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
ವರ್ಗ InMemoryFileAdaptor : ಸಾರ್ವಜನಿಕ File {
  ಧ್ರುವ InMemoryFile &ಗಂಟು;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

ಸಾರ್ವಜನಿಕ:
  explicit InMemoryFileAdaptor(ಧ್ರುವ InMemoryFile &ಗಂಟು,
                               std::string RequestedName)
      : ಗಂಟು(ಗಂಟು), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(ಧ್ರುವ Twine &Name, int64_t FileSize, ತಾರ್ಕಿಕ RequiresNullTerminator,
            ತಾರ್ಕಿಕ IsVolatile) override {
    llvm::MemoryBuffer *Buf = ಗಂಟು.getBuffer();
    ಪ್ರತ್ಯಾಗಮ llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { ಪ್ರತ್ಯಾಗಮ {}; }

  ಶೂನ್ಯ setPath(ಧ್ರುವ Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

ವರ್ಗ InMemoryDirectory : ಸಾರ್ವಜನಿಕ In_Memory_ಗಂಟು {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_ಗಂಟು>, std::less<>> Entries;

ಸಾರ್ವಜನಿಕ:
  InMemoryDirectory(Status Stat)
      : In_Memory_ಗಂಟು(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(ಧ್ರುವ Twine &RequestedName) ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Stat.getUniqueID(); }

  In_Memory_ಗಂಟು *getChild(StringRef Name) ಧ್ರುವ {
    auto I = Entries.find(Name);
    ಯದಿ (I != Entries.end())
      ಪ್ರತ್ಯಾಗಮ I->second.get();
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }

  In_Memory_ಗಂಟು *addChild(StringRef Name, std::unique_ptr<In_Memory_ಗಂಟು> Child) {
    ಪ್ರತ್ಯಾಗಮ Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Entries.begin(); }
  const_iterator end() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Entries.end(); }

  std::string toString(unsigned Indent) ಧ್ರುವ override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    ಪ್ರತಿ (ಧ್ರುವ auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    ಪ್ರತ್ಯಾಗಮ Result;
  }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ In_Memory_ಗಂಟು *N) {
    ಪ್ರತ್ಯಾಗಮ N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ಸ್ಥಿರ sys::fs::UniqueID getUniqueID(hash_code Hash) {
  ಪ್ರತ್ಯಾಗಮ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ಸ್ಥಿರ sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ಪ್ರತ್ಯಾಗಮ getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
ಸ್ಥಿರ sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  ಪ್ರತ್ಯಾಗಮ getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_ಗಂಟು_Info::makeStatus() ಧ್ರುವ {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  ಪ್ರತ್ಯಾಗಮ Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(ತಾರ್ಕಿಕ UseNormalizedPaths)
    : ಬೇರು_2(ನೂತನ detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = ಸ್ವಭಾವ;

std::string InMemoryFileSystem::toString() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ಬೇರು_2->toString(/*Indent=*/0);
}

ತಾರ್ಕಿಕ InMemoryFileSystem::addFile(ಧ್ರುವ Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_ಗಂಟು_Fn Make_ಗಂಟು) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  (ಶೂನ್ಯ)EC;

  ಯದಿ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ಸತ್ಯ);

  ಯದಿ (Path.empty())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  detail::InMemoryDirectory *Dir = ಬೇರು_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  ಧ್ರುವ auto ResolvedUser = User.ಮೌಲ್ಯ_or(0);
  ಧ್ರುವ auto ResolvedGroup = Group.ಮೌಲ್ಯ_or(0);
  ಧ್ರುವ auto ResolvedType = Type.ಮೌಲ್ಯ_or(sys::fs::file_type::regular_file);
  ಧ್ರುವ auto ResolvedPerms = Perms.ಮೌಲ್ಯ_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ಧ್ರುವ auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  ಯಾವತ್ (ಸತ್ಯ) {
    Name = *I;
    ++I;
    ಯದಿ (I == E)
      ಭಙ್ಗ;
    detail::In_Memory_ಗಂಟು *ಗಂಟು = Dir->getChild(Name);
    ಯದಿ (!ಗಂಟು) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      ಅನುವರ್ತ;
    }
    // Creating file under another file.
    ಯದಿ (!isa<detail::InMemoryDirectory>(ಗಂಟು))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    Dir = cast<detail::InMemoryDirectory>(ಗಂಟು);
  }
  detail::In_Memory_ಗಂಟು *ಗಂಟು = Dir->getChild(Name);
  ಯದಿ (!ಗಂಟು) {
    Dir->addChild(Name,
                  Make_ಗಂಟು({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಯದಿ (isa<detail::InMemoryDirectory>(ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ ResolvedType == sys::fs::file_type::directory_file;

  ಪ್ರತಿಜ್ಞಾ((isa<detail::InMemoryFile>(ಗಂಟು) ||
          isa<detail::InMemoryHardLink>(ಗಂಟು)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  ಯದಿ (auto *Link = dyn_cast<detail::InMemoryHardLink>(ಗಂಟು)) {
    ಪ್ರತ್ಯಾಗಮ Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  ಪ್ರತ್ಯಾಗಮ cast<detail::InMemoryFile>(ಗಂಟು)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

ತಾರ್ಕಿಕ InMemoryFileSystem::addFile(ಧ್ರುವ Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ಪ್ರತ್ಯಾಗಮ addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_ಗಂಟು_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_ಗಂಟು> {
                   Status Stat = NNI.makeStatus();
                   ಯದಿ (Stat.getType() == sys::fs::file_type::directory_file)
                     ಪ್ರತ್ಯಾಗಮ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ಪ್ರತ್ಯಾಗಮ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

ತಾರ್ಕಿಕ InMemoryFileSystem::addFileNoOwn(
    ಧ್ರುವ Twine &P, time_t ModificationTime,
    ಧ್ರುವ llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ಪ್ರತ್ಯಾಗಮ addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_ಗಂಟು_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_ಗಂಟು> {
                   Status Stat = NNI.makeStatus();
                   ಯದಿ (Stat.getType() == sys::fs::file_type::directory_file)
                     ಪ್ರತ್ಯಾಗಮ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ಪ್ರತ್ಯಾಗಮ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_ಗಂಟು_Or_Error
InMemoryFileSystem::lookup_ಗಂಟು(ಧ್ರುವ Twine &P, ತಾರ್ಕಿಕ FollowFinalSymlink,
                               size_t SymlinkDepth) ಧ್ರುವ {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  (ಶೂನ್ಯ)EC;

  ಯದಿ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ಸತ್ಯ);

  ಧ್ರುವ detail::InMemoryDirectory *Dir = ಬೇರು_2.get();
  ಯದಿ (Path.empty())
    ಪ್ರತ್ಯಾಗಮ detail::Named_ಗಂಟು_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  ಯಾವತ್ (ಸತ್ಯ) {
    detail::In_Memory_ಗಂಟು *ಗಂಟು = Dir->getChild(*I);
    ++I;
    ಯದಿ (!ಗಂಟು)
      ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;

    ಯದಿ (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(ಗಂಟು)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      ಯದಿ (I == E && !FollowFinalSymlink)
        ಪ್ರತ್ಯಾಗಮ detail::Named_ಗಂಟು_Or_Error(Path, Symlink);

      ಯದಿ (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      ಯದಿ (std::error_code EC = makeAbsolute(TargetPath))
        ಪ್ರತ್ಯಾಗಮ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_ಗಂಟು(TargetPath, /*FollowFinalSymlink=*/ಸತ್ಯ, SymlinkDepth + 1);
      ಯದಿ (!Target || I == E)
        ಪ್ರತ್ಯಾಗಮ Target;

      ಯದಿ (!isa<detail::InMemoryDirectory>(*Target))
        ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      ಅನುವರ್ತ;
    }

    // Return the file if it's at the end of the path.
    ಯದಿ (auto File = dyn_cast<detail::InMemoryFile>(ಗಂಟು)) {
      ಯದಿ (I == E)
        ಪ್ರತ್ಯಾಗಮ detail::Named_ಗಂಟು_Or_Error(Path, File);
      ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    ಯದಿ (auto File = dyn_cast<detail::InMemoryHardLink>(ಗಂಟು)) {
      ಯದಿ (I == E)
        ಪ್ರತ್ಯಾಗಮ detail::Named_ಗಂಟು_Or_Error(Path, &File->getResolvedFile());
      ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(ಗಂಟು);
    ಯದಿ (I == E)
      ಪ್ರತ್ಯಾಗಮ detail::Named_ಗಂಟು_Or_Error(Path, Dir);
  }
}

ತಾರ್ಕಿಕ InMemoryFileSystem::addHardLink(ಧ್ರುವ Twine &NewLink,
                                     ಧ್ರುವ Twine &Target) {
  auto New_Link_ಗಂಟು = lookup_ಗಂಟು(NewLink, /*FollowFinalSymlink=*/ಅಸತ್ಯ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_ಗಂಟು = lookup_ಗಂಟು(Target, /*FollowFinalSymlink=*/ಸತ್ಯ);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  ಯದಿ (!Target_ಗಂಟು || New_Link_ಗಂಟು || !isa<detail::InMemoryFile>(*Target_ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_ಗಂಟು_Info NNI) {
                   ಪ್ರತ್ಯಾಗಮ std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_ಗಂಟು));
                 });
}

ತಾರ್ಕಿಕ InMemoryFileSystem::addSymbolicLink(
    ಧ್ರುವ Twine &NewLink, ಧ್ರುವ Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_ಗಂಟು = lookup_ಗಂಟು(NewLink, /*FollowFinalSymlink=*/ಅಸತ್ಯ);
  ಯದಿ (New_Link_ಗಂಟು)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  ಪ್ರತ್ಯಾಗಮ addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_ಗಂಟು_Info NNI) {
                   ಪ್ರತ್ಯಾಗಮ std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(ಧ್ರುವ Twine &Path) {
  auto ಗಂಟು = lookup_ಗಂಟು(Path, /*FollowFinalSymlink=*/ಸತ್ಯ);
  ಯದಿ (ಗಂಟು)
    ಪ್ರತ್ಯಾಗಮ (*ಗಂಟು)->getStatus(Path);
  ಪ್ರತ್ಯಾಗಮ ಗಂಟು.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(ಧ್ರುವ Twine &Path) {
  auto ಗಂಟು = lookup_ಗಂಟು(Path,/*FollowFinalSymlink=*/ಸತ್ಯ);
  ಯದಿ (!ಗಂಟು)
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  ಯದಿ (auto *F = dyn_cast<detail::InMemoryFile>(*ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ std::unique_ptr<File>(
        ನೂತನ detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
ವರ್ಗ InMemoryFileSystem::DirIterator : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  ಧ್ರುವ InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  ಶೂನ್ಯ setCurrentEntry() {
    ಯದಿ (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ವಿಕಲ್ಪ (I->second->getKind()) {
      ಪಕ್ಷ detail::IME_File:
      ಪಕ್ಷ detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        ಭಙ್ಗ;
      ಪಕ್ಷ detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        ಭಙ್ಗ;
      ಪಕ್ಷ detail::IME_SymbolicLink:
        ಯದಿ (auto SymlinkTarget =
                FS->lookup_ಗಂಟು(Path, /*FollowFinalSymlink=*/ಸತ್ಯ)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        ಭಙ್ಗ;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } ಅನ್ಯಥಾ {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

ಸಾರ್ವಜನಿಕ:
  DirIterator() = ಸ್ವಭಾವ;

  DirIterator(ಧ್ರುವ InMemoryFileSystem *FS,
              ಧ್ರುವ detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    ಪ್ರತ್ಯಾಗಮ {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(ಧ್ರುವ Twine &Dir,
                                                 std::error_code &EC) {
  auto ಗಂಟು = lookup_ಗಂಟು(Dir, /*FollowFinalSymlink=*/ಸತ್ಯ);
  ಯದಿ (!ಗಂಟು) {
    EC = ಗಂಟು.getError();
    ಪ್ರತ್ಯಾಗಮ directory_iterator(std::make_shared<DirIterator>());
  }

  ಯದಿ (auto *Dir_ಗಂಟು = dyn_cast<detail::InMemoryDirectory>(*ಗಂಟು))
    ಪ್ರತ್ಯಾಗಮ directory_iterator(
        std::make_shared<DirIterator>(ಏತತ್, *Dir_ಗಂಟು, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  ಪ್ರತ್ಯಾಗಮ directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(ಧ್ರುವ Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  (ಶೂನ್ಯ)EC;

  ಯದಿ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ಸತ್ಯ);

  ಯದಿ (!Path.empty())
    WorkingDirectory = std::string(Path);
  ಪ್ರತ್ಯಾಗಮ {};
}

std::error_code InMemoryFileSystem::getRealPath(ಧ್ರುವ Twine &Path,
                                                SmallVectorImpl<ವರ್ಣ> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  ಯದಿ (!CWD || CWD->empty())
    ಪ್ರತ್ಯಾಗಮ errc::operation_not_permitted;
  Path.toVector(Output);
  ಯದಿ (auto EC = makeAbsolute(Output))
    ಪ್ರತ್ಯಾಗಮ EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/ಸತ್ಯ);
  ಪ್ರತ್ಯಾಗಮ {};
}

std::error_code InMemoryFileSystem::isLocal(ಧ್ರುವ Twine &Path, ತಾರ್ಕಿಕ &Result) {
  Result = ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ {};
}

ಶೂನ್ಯ InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) ಧ್ರುವ {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

ನಾಮಾಕಾಶ {

ಸ್ಥಿರ llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  ಧ್ರುವ size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  ಯದಿ (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  ಪ್ರತ್ಯಾಗಮ style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ಸ್ಥಿರ llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/ಸತ್ಯ, style);
  ಪ್ರತ್ಯಾಗಮ result;
}

/// Whether the error and entry specify a file/directory that was not found.
ಸ್ಥಿರ ತಾರ್ಕಿಕ isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  ಯದಿ (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  ಪ್ರತಿಜ್ಞಾ(ExternalFS && "RedirectingFileSystem requires an external FS");
  ಯದಿ (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
ವರ್ಗ llvm::vfs::RedirectingFSDirIterImpl
    : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(ತಾರ್ಕಿಕ IsFirstTime) {
    ಪ್ರತಿಜ್ಞಾ((IsFirstTime || Current != End) && "cannot iterate past end");
    ಯದಿ (!IsFirstTime)
      ++Current;
    ಯದಿ (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ವಿಕಲ್ಪ ((*Current)->getKind()) {
      ಪಕ್ಷ RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      ಪಕ್ಷ RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        ಭಙ್ಗ;
      ಪಕ್ಷ RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        ಭಙ್ಗ;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } ಅನ್ಯಥಾ {
      CurrentEntry = directory_entry();
    }
    ಪ್ರತ್ಯಾಗಮ {};
  };

ಸಾರ್ವಜನಿಕ:
  RedirectingFSDirIterImpl(
      ಧ್ರುವ Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/ಸತ್ಯ);
  }

  std::error_code increment() override {
    ಪ್ರತ್ಯಾಗಮ incrementImpl(/*IsFirstTime=*/ಅಸತ್ಯ);
  }
};

ನಾಮಾಕಾಶ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
ವರ್ಗ RedirectingFSDirRemapIterImpl : ಸಾರ್ವಜನಿಕ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

ಸಾರ್ವಜನಿಕ:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    ಯದಿ (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  ಶೂನ್ಯ setCurrentEntry() {
    StringRef ExternalPath = ExternalIter->path();
    llvm::sys::path::Style ExternalStyle = getExistingStyle(ExternalPath);
    StringRef File = llvm::sys::path::filename(ExternalPath, ExternalStyle);

    SmallString<128> NewPath(Dir);
    llvm::sys::path::append(NewPath, DirStyle, File);

    CurrentEntry = directory_entry(std::string(NewPath), ExternalIter->type());
  }

  std::error_code increment() override {
    std::error_code EC;
    ExternalIter.increment(EC);
    ಯದಿ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    ಅನ್ಯಥಾ
      CurrentEntry = directory_entry();
    ಪ್ರತ್ಯಾಗಮ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(ಧ್ರುವ Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  ಯದಿ (!exists(Path))
    ಪ್ರತ್ಯಾಗಮ errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  ಯದಿ (std::error_code EC = makeAbsolute(AbsolutePath))
    ಪ್ರತ್ಯಾಗಮ EC;
  WorkingDirectory = std::string(AbsolutePath);
  ಪ್ರತ್ಯಾಗಮ {};
}

std::error_code RedirectingFileSystem::isLocal(ಧ್ರುವ Twine &Path_,
                                               ತಾರ್ಕಿಕ &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  ಯದಿ (makeAbsolute(Path))
    ಪ್ರತ್ಯಾಗಮ {};

  ಪ್ರತ್ಯಾಗಮ ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<ವರ್ಣ> &Path) ಧ್ರುವ {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  ಯದಿ (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ಪ್ರತ್ಯಾಗಮ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ಯದಿ (!WorkingDir)
    ಪ್ರತ್ಯಾಗಮ WorkingDir.getError();

  ಪ್ರತ್ಯಾಗಮ makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<ವರ್ಣ> &Path) ಧ್ರುವ {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  ಯದಿ (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    ಪ್ರತ್ಯಾಗಮ std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  ಯದಿ (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } ಅನ್ಯಥಾ {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    ಯದಿ (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  ಯದಿ (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  ಪ್ರತ್ಯಾಗಮ {};
}

directory_iterator RedirectingFileSystem::dir_begin(ಧ್ರುವ Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ಯದಿ (!Result) {
    ಯದಿ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      ಪ್ರತ್ಯಾಗಮ ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    ಪ್ರತ್ಯಾಗಮ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  ಯದಿ (!S) {
    ಯದಿ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      ಪ್ರತ್ಯಾಗಮ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ಪ್ರತ್ಯಾಗಮ {};
  }

  ಯದಿ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ಪ್ರತ್ಯಾಗಮ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  ಯದಿ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    ಯದಿ (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } ಅನ್ಯಥಾ {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  ಯದಿ (RedirectEC) {
    ಯದಿ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ಪ್ರತ್ಯಾಗಮ {};
    }
    RedirectIter = {};
  }

  ಯದಿ (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    ಪ್ರತ್ಯಾಗಮ RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  ಯದಿ (ExternalEC) {
    ಯದಿ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ಪ್ರತ್ಯಾಗಮ {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  ವಿಕಲ್ಪ (Redirection) {
  ಪಕ್ಷ RedirectKind::Fallthrough:
    Iters.ತಳ್ಳು_back(ExternalIter);
    Iters.ತಳ್ಳು_back(RedirectIter);
    ಭಙ್ಗ;
  ಪಕ್ಷ RedirectKind::Fallback:
    Iters.ತಳ್ಳು_back(RedirectIter);
    Iters.ತಳ್ಳು_back(ExternalIter);
    ಭಙ್ಗ;
  ಸ್ವಭಾವ:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ {};
  ಪ್ರತ್ಯಾಗಮ Combined;
}

ಶೂನ್ಯ RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ OverlayFileDir;
}

ಶೂನ್ಯ RedirectingFileSystem::setFallthrough(ತಾರ್ಕಿಕ Fallthrough) {
  ಯದಿ (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } ಅನ್ಯಥಾ {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

ಶೂನ್ಯ RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() ಧ್ರುವ {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  ಪ್ರತಿ (ಧ್ರುವ auto &ಬೇರು_2 : Roots)
    R.ತಳ್ಳು_back(ಬೇರು_2->getName());
  ಪ್ರತ್ಯಾಗಮ R;
}

ಶೂನ್ಯ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) ಧ್ರುವ {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  ಯದಿ (Type == PrintType::Summary)
    ಪ್ರತ್ಯಾಗಮ;

  ಪ್ರತಿ (ಧ್ರುವ auto &ಬೇರು_2 : Roots)
    printEntry(OS, ಬೇರು_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

ಶೂನ್ಯ RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) ಧ್ರುವ {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  ವಿಕಲ್ಪ (E->getKind()) {
  ಪಕ್ಷ EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    ಪ್ರತಿ (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    ಭಙ್ಗ;
  }
  ಪಕ್ಷ EK_DirectoryRemap:
  ಪಕ್ಷ EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    ವಿಕಲ್ಪ (RE->getUseName()) {
    ಪಕ್ಷ NK_NotSet:
      ಭಙ್ಗ;
    ಪಕ್ಷ NK_External:
      OS << " (UseExternalName: true)";
      ಭಙ್ಗ;
    ಪಕ್ಷ NK_Virtual:
      OS << " (UseExternalName: false)";
      ಭಙ್ಗ;
    }
    OS << "\n";
    ಭಙ್ಗ;
  }
  }
}

ಶೂನ್ಯ RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  ಯದಿ (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
ವರ್ಗ llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  ಶೂನ್ಯ error(yaml::ಗಂಟು *N, ಧ್ರುವ Twine &ಸಂದೇಶ_2) { Stream.printError(N, ಸಂದೇಶ_2); }

  // false on error
  ತಾರ್ಕಿಕ parseScalarString(yaml::ಗಂಟು *N, StringRef &Result,
                         SmallVectorImpl<ವರ್ಣ> &Storage) {
    ಧ್ರುವ auto *S = dyn_cast_if_present<yaml::Scalar_ಗಂಟು>(N);

    ಯದಿ (!S) {
      error(N, "expected string");
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
    Result = S->get_ಮೌಲ್ಯ(Storage);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  // false on error
  ತಾರ್ಕಿಕ parseScalarBool(yaml::ಗಂಟು *N, ತಾರ್ಕಿಕ &Result) {
    SmallString<5> Storage;
    StringRef ಮೌಲ್ಯ;
    ಯದಿ (!parseScalarString(N, ಮೌಲ್ಯ, Storage))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    ಯದಿ (ಮೌಲ್ಯ.equals_insensitive("true") || ಮೌಲ್ಯ.equals_insensitive("on") ||
        ಮೌಲ್ಯ.equals_insensitive("yes") || ಮೌಲ್ಯ == "1") {
      Result = ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    } ಅನ್ಯಥಾ ಯದಿ (ಮೌಲ್ಯ.equals_insensitive("false") ||
               ಮೌಲ್ಯ.equals_insensitive("off") ||
               ಮೌಲ್ಯ.equals_insensitive("no") || ಮೌಲ್ಯ == "0") {
      Result = ಅಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }

    error(N, "expected boolean value");
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::ಗಂಟು *N) {
    SmallString<12> Storage;
    StringRef ಮೌಲ್ಯ;
    ಯದಿ (!parseScalarString(N, ಮೌಲ್ಯ, Storage))
      ಪ್ರತ್ಯಾಗಮ std::nullopt;

    ಯದಿ (ಮೌಲ್ಯ.equals_insensitive("fallthrough")) {
      ಪ್ರತ್ಯಾಗಮ RedirectingFileSystem::RedirectKind::Fallthrough;
    } ಅನ್ಯಥಾ ಯದಿ (ಮೌಲ್ಯ.equals_insensitive("fallback")) {
      ಪ್ರತ್ಯಾಗಮ RedirectingFileSystem::RedirectKind::Fallback;
    } ಅನ್ಯಥಾ ಯದಿ (ಮೌಲ್ಯ.equals_insensitive("redirect-only")) {
      ಪ್ರತ್ಯಾಗಮ RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  }

  std::optional<RedirectingFileSystem::ಬೇರು_Relative_Kind>
  parse_ಬೇರು_Relative_Kind(yaml::ಗಂಟು *N) {
    SmallString<12> Storage;
    StringRef ಮೌಲ್ಯ;
    ಯದಿ (!parseScalarString(N, ಮೌಲ್ಯ, Storage))
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    ಯದಿ (ಮೌಲ್ಯ.equals_insensitive("cwd")) {
      ಪ್ರತ್ಯಾಗಮ RedirectingFileSystem::ಬೇರು_Relative_Kind::CWD;
    } ಅನ್ಯಥಾ ಯದಿ (ಮೌಲ್ಯ.equals_insensitive("overlay-dir")) {
      ಪ್ರತ್ಯಾಗಮ RedirectingFileSystem::ಬೇರು_Relative_Kind::OverlayDir;
    }
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  }

  ಸಂರಚನಾ ಕೀಲಿ_Status {
    ತಾರ್ಕಿಕ Required;
    ತಾರ್ಕಿಕ Seen = ಅಸತ್ಯ;

    ಕೀಲಿ_Status(ತಾರ್ಕಿಕ Required = ಅಸತ್ಯ) : Required(Required) {}
  };

  using ಕೀಲಿ_Status_Pair = std::pair<StringRef, ಕೀಲಿ_Status>;

  // false on error
  ತಾರ್ಕಿಕ check_Duplicate_Or_Unknown_ಕೀಲಿ(yaml::ಗಂಟು *ಕೀಲಿ_ಗಂಟು, StringRef ಕೀಲಿ,
                                  DenseMap<StringRef, ಕೀಲಿ_Status> &ಕೀಲಿಗಳು_2) {
    auto It = ಕೀಲಿಗಳು_2.find(ಕೀಲಿ);
    ಯದಿ (It == ಕೀಲಿಗಳು_2.end()) {
      error(ಕೀಲಿ_ಗಂಟು, "unknown key");
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
    ಕೀಲಿ_Status &S = It->second;
    ಯದಿ (S.Seen) {
      error(ಕೀಲಿ_ಗಂಟು, Twine("duplicate key '") + ಕೀಲಿ + "'");
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
    S.Seen = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  // false on error
  ತಾರ್ಕಿಕ check_Missing_ಕೀಲಿಗಳು(yaml::ಗಂಟು *Obj, DenseMap<StringRef, ಕೀಲಿ_Status> &ಕೀಲಿಗಳು_2) {
    ಪ್ರತಿ (ಧ್ರುವ auto &I : ಕೀಲಿಗಳು_2) {
      ಯದಿ (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      }
    }
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

ಸಾರ್ವಜನಿಕ:
  ಸ್ಥಿರ RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    ಯದಿ (!ParentEntry) { // Look for a existent root
      ಪ್ರತಿ (ಧ್ರುವ auto &ಬೇರು_2 : FS->Roots) {
        ಯದಿ (Name == ಬೇರು_2->getName()) {
          ParentEntry = ಬೇರು_2.get();
          ಪ್ರತ್ಯಾಗಮ ParentEntry;
        }
      }
    } ಅನ್ಯಥಾ { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      ಪ್ರತಿ (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        ಯದಿ (DirContent && Name == Content->getName())
          ಪ್ರತ್ಯಾಗಮ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    ಯದಿ (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.ತಳ್ಳು_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      ಪ್ರತ್ಯಾಗಮ ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    ಪ್ರತ್ಯಾಗಮ DE->getLastContent();
  }

ಗುಪ್ತ:
  ಶೂನ್ಯ unique_Overlay_ವೃಕ್ಷ(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    ವಿಕಲ್ಪ (SrcE->getKind()) {
    ಪಕ್ಷ RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      ಯದಿ (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      ಪ್ರತಿ (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_ವೃಕ್ಷ(FS, SubEntry.get(), NewParentE);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ RedirectingFileSystem::EK_DirectoryRemap: {
      ಪ್ರತಿಜ್ಞಾ(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ RedirectingFileSystem::EK_File: {
      ಪ್ರತಿಜ್ಞಾ(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      ಭಙ್ಗ;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::ಗಂಟು *N, RedirectingFileSystem *FS, ತಾರ್ಕಿಕ Is_ಬೇರು_Entry) {
    auto *M = dyn_cast<yaml::Mapping_ಗಂಟು>(N);
    ಯದಿ (!M) {
      error(N, "expected mapping node for file or directory entry");
      ಪ್ರತ್ಯಾಗಮ nullptr;
    }

    ಕೀಲಿ_Status_Pair Fields[] = {
        ಕೀಲಿ_Status_Pair("name", ಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("type", ಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("contents", ಅಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("external-contents", ಅಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("use-external-name", ಅಸತ್ಯ),
    };

    DenseMap<StringRef, ಕೀಲಿ_Status> ಕೀಲಿಗಳು_2(std::begin(Fields), std::end(Fields));

    ಗಣನಾ { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::ಗಂಟು *Name_ಮೌಲ್ಯ_ಗಂಟು = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    ಪ್ರತಿ (auto &I : *M) {
      StringRef ಕೀಲಿ;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      ಯದಿ (!parseScalarString(I.get_ಕೀಲಿ(), ಕೀಲಿ, Buffer))
        ಪ್ರತ್ಯಾಗಮ nullptr;

      ಯದಿ (!check_Duplicate_Or_Unknown_ಕೀಲಿ(I.get_ಕೀಲಿ(), ಕೀಲಿ, ಕೀಲಿಗಳು_2))
        ಪ್ರತ್ಯಾಗಮ nullptr;

      StringRef ಮೌಲ್ಯ;
      ಯದಿ (ಕೀಲಿ == "name") {
        ಯದಿ (!parseScalarString(I.get_ಮೌಲ್ಯ(), ಮೌಲ್ಯ, Buffer))
          ಪ್ರತ್ಯಾಗಮ nullptr;

        Name_ಮೌಲ್ಯ_ಗಂಟು = I.get_ಮೌಲ್ಯ();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(ಮೌಲ್ಯ).str();
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "type") {
        ಯದಿ (!parseScalarString(I.get_ಮೌಲ್ಯ(), ಮೌಲ್ಯ, Buffer))
          ಪ್ರತ್ಯಾಗಮ nullptr;
        ಯದಿ (ಮೌಲ್ಯ == "file")
          Kind = RedirectingFileSystem::EK_File;
        ಅನ್ಯಥಾ ಯದಿ (ಮೌಲ್ಯ == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        ಅನ್ಯಥಾ ಯದಿ (ಮೌಲ್ಯ == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        ಅನ್ಯಥಾ {
          error(I.get_ಮೌಲ್ಯ(), "unknown value for 'type'");
          ಪ್ರತ್ಯಾಗಮ nullptr;
        }
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "contents") {
        ಯದಿ (ContentsField != CF_NotSet) {
          error(I.get_ಕೀಲಿ(),
                "entry already has 'contents' or 'external-contents'");
          ಪ್ರತ್ಯಾಗಮ nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_ಗಂಟು>(I.get_ಮೌಲ್ಯ());
        ಯದಿ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_ಮೌಲ್ಯ(), "expected array");
          ಪ್ರತ್ಯಾಗಮ nullptr;
        }

        ಪ್ರತಿ (auto &I : *Contents) {
          ಯದಿ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ ಅಸತ್ಯ))
            EntryArrayContents.ತಳ್ಳು_back(std::move(E));
          ಅನ್ಯಥಾ
            ಪ್ರತ್ಯಾಗಮ nullptr;
        }
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "external-contents") {
        ಯದಿ (ContentsField != CF_NotSet) {
          error(I.get_ಕೀಲಿ(),
                "entry already has 'contents' or 'external-contents'");
          ಪ್ರತ್ಯಾಗಮ nullptr;
        }
        ContentsField = CF_External;
        ಯದಿ (!parseScalarString(I.get_ಮೌಲ್ಯ(), ಮೌಲ್ಯ, Buffer))
          ಪ್ರತ್ಯಾಗಮ nullptr;

        SmallString<256> FullPath;
        ಯದಿ (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          ಪ್ರತಿಜ್ಞಾ(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = ಮೌಲ್ಯ;
          ಯದಿ (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            ಪ್ರತ್ಯಾಗಮ nullptr;
          }
          FullPath = AbsFullPath;
        } ಅನ್ಯಥಾ {
          FullPath = ಮೌಲ್ಯ;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "use-external-name") {
        ತಾರ್ಕಿಕ Val;
        ಯದಿ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), Val))
          ಪ್ರತ್ಯಾಗಮ nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } ಅನ್ಯಥಾ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ಯದಿ (Stream.failed())
      ಪ್ರತ್ಯಾಗಮ nullptr;

    // check for missing keys
    ಯದಿ (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      ಪ್ರತ್ಯಾಗಮ nullptr;
    }
    ಯದಿ (!check_Missing_ಕೀಲಿಗಳು(N, ಕೀಲಿಗಳು_2))
      ಪ್ರತ್ಯಾಗಮ nullptr;

    // check invalid configuration
    ಯದಿ (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ಪ್ರತ್ಯಾಗಮ nullptr;
    }

    ಯದಿ (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ಪ್ರತ್ಯಾಗಮ nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    ಯದಿ (Is_ಬೇರು_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      ಯದಿ (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } ಅನ್ಯಥಾ ಯದಿ (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } ಅನ್ಯಥಾ {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        ಯದಿ (FS->ಬೇರು_Relative ==
            RedirectingFileSystem::ಬೇರು_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          ಪ್ರತಿಜ್ಞಾ(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } ಅನ್ಯಥಾ {
          EC = FS->makeAbsolute(Name);
        }
        ಯದಿ (EC) {
          ಪ್ರತಿಜ್ಞಾ(Name_ಮೌಲ್ಯ_ಗಂಟು && "Name presence should be checked earlier");
          error(
              Name_ಮೌಲ್ಯ_ಗಂಟು,
              "entry with relative path at the root level is not discoverable");
          ಪ್ರತ್ಯಾಗಮ nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      ಯದಿ (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t ಬೇರು_Path_Len = sys::path::ಬೇರು_path(Trimmed, path_style).size();
    ಯಾವತ್ (Trimmed.size() > ಬೇರು_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    ವಿಕಲ್ಪ (Kind) {
    ಪಕ್ಷ RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      ಭಙ್ಗ;
    ಪಕ್ಷ RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      ಭಙ್ಗ;
    ಪಕ್ಷ RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      ಭಙ್ಗ;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    ಯದಿ (Parent.empty())
      ಪ್ರತ್ಯಾಗಮ Result;

    // if 'name' contains multiple components, create implicit directory entries
    ಪ್ರತಿ (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.ತಳ್ಳು_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ಪ್ರತ್ಯಾಗಮ Result;
  }

ಸಾರ್ವಜನಿಕ:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  ತಾರ್ಕಿಕ parse(yaml::ಗಂಟು *ಬೇರು_2, RedirectingFileSystem *FS) {
    auto *ತುದಿ = dyn_cast<yaml::Mapping_ಗಂಟು>(ಬೇರು_2);
    ಯದಿ (!ತುದಿ) {
      error(ಬೇರು_2, "expected mapping node");
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }

    ಕೀಲಿ_Status_Pair Fields[] = {
        ಕೀಲಿ_Status_Pair("version", ಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("case-sensitive", ಅಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("use-external-names", ಅಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("root-relative", ಅಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("overlay-relative", ಅಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("fallthrough", ಅಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("redirecting-with", ಅಸತ್ಯ),
        ಕೀಲಿ_Status_Pair("roots", ಸತ್ಯ),
    };

    DenseMap<StringRef, ಕೀಲಿ_Status> ಕೀಲಿಗಳು_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> ಬೇರು_Entries;

    // Parse configuration and 'roots'
    ಪ್ರತಿ (auto &I : *ತುದಿ) {
      SmallString<10> ಕೀಲಿ_Buffer;
      StringRef ಕೀಲಿ;
      ಯದಿ (!parseScalarString(I.get_ಕೀಲಿ(), ಕೀಲಿ, ಕೀಲಿ_Buffer))
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

      ಯದಿ (!check_Duplicate_Or_Unknown_ಕೀಲಿ(I.get_ಕೀಲಿ(), ಕೀಲಿ, ಕೀಲಿಗಳು_2))
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

      ಯದಿ (ಕೀಲಿ == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_ಗಂಟು>(I.get_ಮೌಲ್ಯ());
        ಯದಿ (!Roots) {
          error(I.get_ಮೌಲ್ಯ(), "expected array");
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }

        ಪ್ರತಿ (auto &I : *Roots) {
          ಯದಿ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ ಸತ್ಯ))
            ಬೇರು_Entries.ತಳ್ಳು_back(std::move(E));
          ಅನ್ಯಥಾ
            ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        ಯದಿ (!parseScalarString(I.get_ಮೌಲ್ಯ(), VersionString, Storage))
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        ಪೂರ್ಣಾಙ್ಕ Version;
        ಯದಿ (VersionString.getAsInteger<ಪೂರ್ಣಾಙ್ಕ>(10, Version)) {
          error(I.get_ಮೌಲ್ಯ(), "expected integer");
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }
        ಯದಿ (Version < 0) {
          error(I.get_ಮೌಲ್ಯ(), "invalid version number");
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }
        ಯದಿ (Version != 0) {
          error(I.get_ಮೌಲ್ಯ(), "version mismatch, expected 0");
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "case-sensitive") {
        ಯದಿ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), FS->CaseSensitive))
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "overlay-relative") {
        ಯದಿ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), FS->IsRelativeOverlay))
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "use-external-names") {
        ಯದಿ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), FS->UseExternalNames))
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "fallthrough") {
        ಯದಿ (ಕೀಲಿಗಳು_2["redirecting-with"].Seen) {
          error(I.get_ಮೌಲ್ಯ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }

        ತಾರ್ಕಿಕ ShouldFallthrough = ಅಸತ್ಯ;
        ಯದಿ (!parseScalarBool(I.get_ಮೌಲ್ಯ(), ShouldFallthrough))
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

        ಯದಿ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } ಅನ್ಯಥಾ {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "redirecting-with") {
        ಯದಿ (ಕೀಲಿಗಳು_2["fallthrough"].Seen) {
          error(I.get_ಮೌಲ್ಯ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }

        ಯದಿ (auto Kind = parseRedirectKind(I.get_ಮೌಲ್ಯ())) {
          FS->Redirection = *Kind;
        } ಅನ್ಯಥಾ {
          error(I.get_ಮೌಲ್ಯ(), "expected valid redirect kind");
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }
      } ಅನ್ಯಥಾ ಯದಿ (ಕೀಲಿ == "root-relative") {
        ಯದಿ (auto Kind = parse_ಬೇರು_Relative_Kind(I.get_ಮೌಲ್ಯ())) {
          FS->ಬೇರು_Relative = *Kind;
        } ಅನ್ಯಥಾ {
          error(I.get_ಮೌಲ್ಯ(), "expected valid root-relative kind");
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }
      } ಅನ್ಯಥಾ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ಯದಿ (Stream.failed())
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    ಯದಿ (!check_Missing_ಕೀಲಿಗಳು(ತುದಿ, ಕೀಲಿಗಳು_2))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    ಪ್ರತಿ (auto &E : ಬೇರು_Entries)
      unique_Overlay_ವೃಕ್ಷ(FS, E.get());

    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, ಶೂನ್ಯ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::ಗಂಟು *ಬೇರು_2 = DI->get_ಬೇರು();
  ಯದಿ (DI == Stream.end() || !ಬೇರು_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      ನೂತನ RedirectingFileSystem(ExternalFS));

  ಯದಿ (!YAMLFilePath.empty()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::path::parent_path(YAMLFilePath);
    std::error_code EC = FS->makeAbsolute(OverlayAbsDir);
    ಪ್ರತಿಜ್ಞಾ(!EC && "Overlay dir final path must be absolute");
    (ಶೂನ್ಯ)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  ಯದಿ (!P.parse(ಬೇರು_2, FS.get()))
    ಪ್ರತ್ಯಾಗಮ nullptr;

  ಪ್ರತ್ಯಾಗಮ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    ತಾರ್ಕಿಕ UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      ನೂತನ RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  ಪ್ರತಿ (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (ಶೂನ್ಯ)EC;
      ಪ್ರತಿಜ್ಞಾ(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    ಯದಿ (ToEntry)
      ಅನುವರ್ತ;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    ಪ್ರತಿ (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    ಪ್ರತಿಜ್ಞಾ(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (ಶೂನ್ಯ)EC;
      ಪ್ರತಿಜ್ಞಾ(!EC && "Could not make absolute path");
    }

    // Add the file.
    auto NewFile = std::make_unique<RedirectingFileSystem::FileEntry>(
        llvm::sys::path::filename(From), To,
        UseExternalNames ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    ToEntry = NewFile.get();
    cast<RedirectingFileSystem::DirectoryEntry>(Parent)->addContent(
        std::move(NewFile));
  }

  ಪ್ರತ್ಯಾಗಮ FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  ಪ್ರತಿಜ್ಞಾ(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  ಯದಿ (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

ಶೂನ್ಯ RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<ವರ್ಣ> &Result) ಧ್ರುವ {
  Result.clear();
  ಪ್ರತಿ (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<ವರ್ಣ> &Path) ಧ್ರುವ {
  ಯದಿ (std::error_code EC = makeAbsolute(Path))
    ಪ್ರತ್ಯಾಗಮ EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  ಯದಿ (CanonicalPath.empty())
    ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  ಪ್ರತ್ಯಾಗಮ {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) ಧ್ರುವ {
  llvm::SmallString<128> CanonicalPath(Path);
  ಯದಿ (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    ಪ್ರತ್ಯಾಗಮ EC;

  // RedirectOnly means the VFS is always used.
  ಯದಿ (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = ಸತ್ಯ;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  ಪ್ರತಿ (ಧ್ರುವ auto &ಬೇರು_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, ಬೇರು_2.get(), Entries);
    ಯದಿ (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = ಸತ್ಯ;
    ಯದಿ (Result) {
      Result->Parents = std::move(Entries);
      ಪ್ರತ್ಯಾಗಮ Result;
    }

    ಯದಿ (Result.getError() != llvm::errc::no_such_file_or_directory)
      ಪ್ರತ್ಯಾಗಮ Result;
  }
  ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  ಯದಿ (!FromName.empty()) {
    ಯದಿ (!pathComponentMatches(*Start, FromName))
      ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    ಯದಿ (Start == End) {
      // Match!
      ಪ್ರತ್ಯಾಗಮ LookupResult(From, Start, End);
    }
  }

  ಯದಿ (isa<RedirectingFileSystem::FileEntry>(From))
    ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::not_a_directory);

  ಯದಿ (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    ಪ್ರತ್ಯಾಗಮ LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  ಪ್ರತಿ (ಧ್ರುವ std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.ತಳ್ಳು_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    ಯದಿ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ಪ್ರತ್ಯಾಗಮ Result;
    Entries.ತೆಗೆ_back();
  }

  ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::no_such_file_or_directory);
}

ಸ್ಥಿರ Status getRedirectedFileStatus(ಧ್ರುವ Twine &OriginalPath,
                                      ತಾರ್ಕಿಕ UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  ಯದಿ (ExternalStatus.ExposesExternalVFSPath)
    ಪ್ರತ್ಯಾಗಮ ExternalStatus;

  Status S = ExternalStatus;
  ಯದಿ (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  ಅನ್ಯಥಾ
    S.ExposesExternalVFSPath = ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    ಧ್ರುವ Twine &LookupPath, ಧ್ರುವ Twine &OriginalPath,
    ಧ್ರುವ RedirectingFileSystem::LookupResult &Result) {
  ಯದಿ (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    ಯದಿ (std::error_code EC = makeAbsolute(RemappedPath))
      ಪ್ರತ್ಯಾಗಮ EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    ಯದಿ (!S)
      ಪ್ರತ್ಯಾಗಮ S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    ಪ್ರತ್ಯಾಗಮ getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  ಪ್ರತ್ಯಾಗಮ Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(ಧ್ರುವ Twine &LookupPath,
                                         ಧ್ರುವ Twine &OriginalPath) ಧ್ರುವ {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  ಯದಿ (!Result || Result->ExposesExternalVFSPath)
    ಪ್ರತ್ಯಾಗಮ Result;
  ಪ್ರತ್ಯಾಗಮ Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(ಧ್ರುವ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ಯದಿ (std::error_code EC = makeAbsolute(Path))
    ಪ್ರತ್ಯಾಗಮ EC;

  ಯದಿ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    ಯದಿ (S)
      ಪ್ರತ್ಯಾಗಮ S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ಯದಿ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ಯದಿ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ಪ್ರತ್ಯಾಗಮ getExternalStatus(Path, OriginalPath);
    ಪ್ರತ್ಯಾಗಮ Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  ಯದಿ (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ಪ್ರತ್ಯಾಗಮ getExternalStatus(Path, OriginalPath);
  }

  ಪ್ರತ್ಯಾಗಮ S;
}

ತಾರ್ಕಿಕ RedirectingFileSystem::exists(ಧ್ರುವ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ಯದಿ (makeAbsolute(Path))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಯದಿ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ಯದಿ (ExternalFS->exists(Path))
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ಯದಿ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ಯದಿ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ಪ್ರತ್ಯಾಗಮ ExternalFS->exists(Path);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  ಯದಿ (!ExtRedirect) {
    ಪ್ರತಿಜ್ಞಾ(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  ಯದಿ (makeAbsolute(RemappedPath))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಯದಿ (ExternalFS->exists(RemappedPath))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ಪ್ರತ್ಯಾಗಮ ExternalFS->exists(Path);
  }

  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ನಾಮಾಕಾಶ {

/// Provide a file wrapper with an overriden status.
ವರ್ಗ FileWithFixedStatus : ಸಾರ್ವಜನಿಕ File {
  std::unique_ptr<File> InnerFile;
  Status S;

ಸಾರ್ವಜನಿಕ:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { ಪ್ರತ್ಯಾಗಮ S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(ಧ್ರುವ Twine &Name, int64_t FileSize, ತಾರ್ಕಿಕ RequiresNullTerminator,
            ತಾರ್ಕಿಕ IsVolatile) override {
    ಪ್ರತ್ಯಾಗಮ InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { ಪ್ರತ್ಯಾಗಮ InnerFile->close(); }

  ಶೂನ್ಯ setPath(ಧ್ರುವ Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, ಧ್ರುವ Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  ಯದಿ (!Result || (*Result)->status()->ExposesExternalVFSPath)
    ಪ್ರತ್ಯಾಗಮ Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  ಯದಿ (Name && Name.get() != P.str())
    F->get()->setPath(P);
  ಪ್ರತ್ಯಾಗಮ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ಧ್ರುವ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ಯದಿ (std::error_code EC = makeAbsolute(Path))
    ಪ್ರತ್ಯಾಗಮ EC;

  ಯದಿ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ಯದಿ (F)
      ಪ್ರತ್ಯಾಗಮ F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ಯದಿ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ಯದಿ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ಪ್ರತ್ಯಾಗಮ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ಪ್ರತ್ಯಾಗಮ Result.getError();
  }

  ಯದಿ (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    ಪ್ರತ್ಯಾಗಮ make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  ಯದಿ (std::error_code EC = makeAbsolute(RemappedPath))
    ಪ್ರತ್ಯಾಗಮ EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  ಯದಿ (!ExternalFile) {
    ಯದಿ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ಪ್ರತ್ಯಾಗಮ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    ಪ್ರತ್ಯಾಗಮ ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  ಯದಿ (!ExternalStatus)
    ಪ್ರತ್ಯಾಗಮ ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  ಪ್ರತ್ಯಾಗಮ std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(ಧ್ರುವ Twine &OriginalPath,
                                   SmallVectorImpl<ವರ್ಣ> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ಯದಿ (std::error_code EC = makeAbsolute(Path))
    ಪ್ರತ್ಯಾಗಮ EC;

  ಯದಿ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    ಯದಿ (!EC)
      ಪ್ರತ್ಯಾಗಮ EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ಯದಿ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ಯದಿ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ಪ್ರತ್ಯಾಗಮ ExternalFS->getRealPath(Path, Output);
    ಪ್ರತ್ಯಾಗಮ Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  ಯದಿ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    ಯದಿ (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ಪ್ರತ್ಯಾಗಮ ExternalFS->getRealPath(Path, Output);
    }
    ಪ್ರತ್ಯಾಗಮ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  ಯದಿ (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    ಪ್ರತ್ಯಾಗಮ {};
  }
  ಪ್ರತ್ಯಾಗಮ llvm::errc::invalid_argument;
}

ಶೂನ್ಯ RedirectingFileSystem::getDirectoryContentRealSources(
    ಧ್ರುವ Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  ಯದಿ (makeAbsolute(Path))
    ಪ್ರತ್ಯಾಗಮ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ಧ್ರುವ ತಾರ್ಕಿಕ ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ಯದಿ (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    ಯದಿ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    ಪ್ರತ್ಯಾಗಮ;
  }

  ವಿಕಲ್ಪ (Result->E->getKind()) {
  ಪಕ್ಷ EK_File:
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    ಭಙ್ಗ;
  ಪಕ್ಷ EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    ಯದಿ (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    ಭಙ್ಗ;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  ಯದಿ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, ಶೂನ್ಯ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ಪ್ರತ್ಯಾಗಮ RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

ಸ್ಥಿರ ಶೂನ್ಯ getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  ಯದಿ (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    ಪ್ರತಿಜ್ಞಾ(DE && "Must be a directory");
    ಪ್ರತಿ (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.ತಳ್ಳು_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.ತೆಗೆ_back();
    }
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    ಪ್ರತಿಜ್ಞಾ(DR && "Must be a directory remap");
    SmallString<128> VPath;
    ಪ್ರತಿ (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.ತಳ್ಳು_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಪ್ರತಿಜ್ಞಾ(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  ಪ್ರತಿಜ್ಞಾ(FE && "Must be a file");
  SmallString<128> VPath;
  ಪ್ರತಿ (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.ತಳ್ಳು_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

ಶೂನ್ಯ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> ಬೇರು_Result = VFS.lookupPath("/");
  ಯದಿ (!ಬೇರು_Result)
    ಪ್ರತ್ಯಾಗಮ;
  SmallVector<StringRef, 8> Components;
  Components.ತಳ್ಳು_back("/");
  getVFSEntries(ಬೇರು_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  ಸ್ಥಿರ std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ಪ್ರತ್ಯಾಗಮ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

ಶೂನ್ಯ YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             ತಾರ್ಕಿಕ IsDirectory) {
  ಪ್ರತಿಜ್ಞಾ(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  ಪ್ರತಿಜ್ಞಾ(sys::path::is_absolute(RealPath) && "real path not absolute");
  ಪ್ರತಿಜ್ಞಾ(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

ಶೂನ್ಯ YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/ಅಸತ್ಯ);
}

ಶೂನ್ಯ YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/ಸತ್ಯ);
}

ನಾಮಾಕಾಶ {

ವರ್ಗ JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_ರಾಶಿ;

  unsigned getDirIndent() { ಪ್ರತ್ಯಾಗಮ 4 * Dir_ರಾಶಿ.size(); }
  unsigned getFileIndent() { ಪ್ರತ್ಯಾಗಮ 4 * (Dir_ರಾಶಿ.size() + 1); }
  ತಾರ್ಕಿಕ containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  ಶೂನ್ಯ startDirectory(StringRef Path);
  ಶೂನ್ಯ endDirectory();
  ಶೂನ್ಯ writeEntry(StringRef VPath, StringRef RPath);

ಸಾರ್ವಜನಿಕ:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  ಶೂನ್ಯ ಲಿಖ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<ತಾರ್ಕಿಕ> UseExternalNames,
             std::optional<ತಾರ್ಕಿಕ> IsCaseSensitive,
             std::optional<ತಾರ್ಕಿಕ> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

ತಾರ್ಕಿಕ JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using ನಾಮಾಕಾಶ llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  ಪ್ರತಿ (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    ಯದಿ (*IParent != *IChild)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  // Have we exhausted the parent path?
  ಪ್ರತ್ಯಾಗಮ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  ಪ್ರತಿಜ್ಞಾ(!Parent.empty());
  ಪ್ರತಿಜ್ಞಾ(containedIn(Parent, Path));
  ಪ್ರತ್ಯಾಗಮ Path.substr(Parent.size() + 1);
}

ಶೂನ್ಯ JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_ರಾಶಿ.empty() ? Path : containedPart(Dir_ರಾಶಿ.back(), Path);
  Dir_ರಾಶಿ.ತಳ್ಳು_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

ಶೂನ್ಯ JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ರಾಶಿ.ತೆಗೆ_back();
}

ಶೂನ್ಯ JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

ಶೂನ್ಯ JSONWriter::ಲಿಖ(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<ತಾರ್ಕಿಕ> UseExternalNames,
                       std::optional<ತಾರ್ಕಿಕ> IsCaseSensitive,
                       std::optional<ತಾರ್ಕಿಕ> IsOverlayRelative,
                       StringRef OverlayDir) {
  using ನಾಮಾಕಾಶ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  ಯದಿ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  ಯದಿ (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  ತಾರ್ಕಿಕ UseOverlayRelative = ಅಸತ್ಯ;
  ಯದಿ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  ಯದಿ (!Entries.empty()) {
    ಧ್ರುವ YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    ಯದಿ (UseOverlayRelative) {
      ಪ್ರತಿಜ್ಞಾ(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    ತಾರ್ಕಿಕ IsCurrentDirEmpty = ಸತ್ಯ;
    ಯದಿ (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = ಅಸತ್ಯ;
    }

    ಪ್ರತಿ (ಧ್ರುವ auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      ಯದಿ (Dir == Dir_ರಾಶಿ.back()) {
        ಯದಿ (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } ಅನ್ಯಥಾ {
        ತಾರ್ಕಿಕ Is_Dir_Popped_From_ರಾಶಿ = ಅಸತ್ಯ;
        ಯಾವತ್ (!Dir_ರಾಶಿ.empty() && !containedIn(Dir_ರಾಶಿ.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_ರಾಶಿ = ಸತ್ಯ;
        }
        ಯದಿ (Is_Dir_Popped_From_ರಾಶಿ || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = ಸತ್ಯ;
      }
      StringRef RPath = Entry.RPath;
      ಯದಿ (UseOverlayRelative) {
        ಪ್ರತಿಜ್ಞಾ(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      ಯದಿ (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = ಅಸತ್ಯ;
      }
    }

    ಯಾವತ್ (!Dir_ರಾಶಿ.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

ಶೂನ್ಯ YAMLVFSWriter::ಲಿಖ(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](ಧ್ರುವ YAMLVFSEntry &LHS, ಧ್ರುವ YAMLVFSEntry &RHS) {
    ಪ್ರತ್ಯಾಗಮ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).ಲಿಖ(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ಧ್ರುವ Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  ಯದಿ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ರಾಶಿ.ತಳ್ಳು_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  ಪ್ರತಿಜ್ಞಾ(FS && State && !State->ರಾಶಿ.empty() && "incrementing past end");
  ಪ್ರತಿಜ್ಞಾ(!State->ರಾಶಿ.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  ಯದಿ (State->Has_No_ತಳ್ಳು_Request)
    State->Has_No_ತಳ್ಳು_Request = ಅಸತ್ಯ;
  ಅನ್ಯಥಾ {
    ಯದಿ (State->ರಾಶಿ.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ರಾಶಿ.back()->path(), EC);
      ಯದಿ (I != End) {
        State->ರಾಶಿ.ತಳ್ಳು_back(I);
        ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
      }
    }
  }

  ಯಾವತ್ (!State->ರಾಶಿ.empty() && State->ರಾಶಿ.back().increment(EC) == End)
    State->ರಾಶಿ.ತೆಗೆ_back();

  ಯದಿ (State->ರಾಶಿ.empty())
    State.reset(); // end iterator

  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ಧ್ರುವ ವರ್ಣ FileSystem::ID = 0;
ಧ್ರುವ ವರ್ಣ OverlayFileSystem::ID = 0;
ಧ್ರುವ ವರ್ಣ ProxyFileSystem::ID = 0;
ಧ್ರುವ ವರ್ಣ InMemoryFileSystem::ID = 0;
ಧ್ರುವ ವರ್ಣ RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_ಎಣಿಕೆ(
    ಧ್ರುವ FileSystem *FS) {
  ಪ್ರತ್ಯಾಗಮ FS->Use_ಎಣಿಕೆ();
}

ಶೂನ್ಯ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

ಶೂನ್ಯ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
