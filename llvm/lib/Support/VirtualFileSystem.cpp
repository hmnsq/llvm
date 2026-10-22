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

#অন্তর্ভাৱ "llvm/Support/VirtualFileSystem.h"
#অন্তর্ভাৱ "llvm/ADT/ArrayRef.h"
#অন্তর্ভাৱ "llvm/ADT/DenseMap.h"
#অন্তর্ভাৱ "llvm/ADT/IntrusiveRefCntPtr.h"
#অন্তর্ভাৱ "llvm/ADT/STLExtras.h"
#অন্তর্ভাৱ "llvm/ADT/SmallString.h"
#অন্তর্ভাৱ "llvm/ADT/SmallVector.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/ADT/StringSet.h"
#অন্তর্ভাৱ "llvm/ADT/Twine.h"
#অন্তর্ভাৱ "llvm/ADT/iterator_range.h"
#অন্তর্ভাৱ "llvm/Config/llvm-config.h"
#অন্তর্ভাৱ "llvm/Support/Casting.h"
#অন্তর্ভাৱ "llvm/Support/Chrono.h"
#অন্তর্ভাৱ "llvm/Support/Compiler.h"
#অন্তর্ভাৱ "llvm/Support/Debug.h"
#অন্তর্ভাৱ "llvm/Support/Errc.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
#অন্তর্ভাৱ "llvm/Support/ErrorOr.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem/UniqueID.h"
#অন্তর্ভাৱ "llvm/Support/IOSandbox.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাৱ "llvm/Support/Path.h"
#অন্তর্ভাৱ "llvm/Support/SMLoc.h"
#অন্তর্ভাৱ "llvm/Support/SourceMgr.h"
#অন্তর্ভাৱ "llvm/Support/YAMLParser.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <atomic>
#অন্তর্ভাৱ <cassert>
#অন্তর্ভাৱ <cstdint>
#অন্তর্ভাৱ <iterator>
#অন্তর্ভাৱ <limits>
#অন্তর্ভাৱ <map>
#অন্তর্ভাৱ <memory>
#অন্তর্ভাৱ <optional>
#অন্তর্ভাৱ <string>
#অন্তর্ভাৱ <system_error>
#অন্তর্ভাৱ <utility>
#অন্তর্ভাৱ <vector>

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(ধ্রুৱ file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(ধ্রুৱ Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(ধ্রুৱ Status &In, uint64_t NewSize) {
  ঘূৰাই Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ধ্রুৱ Status &In, ধ্রুৱ Twine &NewName) {
  ঘূৰাই Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ধ্রুৱ file_status &In, ধ্রুৱ Twine &NewName) {
  ঘূৰাই Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

তার্কিক Status::equivalent(ধ্রুৱ Status &Other) ধ্রুৱ {
  প্রতিজ্ঞা(isStatusKnown() && Other.isStatusKnown());
  ঘূৰাই getUniqueID() == Other.getUniqueID();
}

তার্কিক Status::isDirectory() ধ্রুৱ { ঘূৰাই Type == file_type::directory_file; }

তার্কিক Status::isRegularFile() ধ্রুৱ { ঘূৰাই Type == file_type::regular_file; }

তার্কিক Status::isOther() ধ্রুৱ {
  ঘূৰাই exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

তার্কিক Status::isSymlink() ধ্রুৱ { ঘূৰাই Type == file_type::symlink_file; }

তার্কিক Status::isStatusKnown() ধ্রুৱ { ঘূৰাই Type != file_type::status_error; }

তার্কিক Status::exists() ধ্রুৱ {
  ঘূৰাই isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = স্ৱভাৱ;

FileSystem::~FileSystem() = স্ৱভাৱ;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(ধ্রুৱ llvm::Twine &Name, int64_t FileSize,
                             তার্কিক RequiresNullTerminator, তার্কিক IsVolatile,
                             তার্কিক IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_দ্বিমিক(Name);
  যদি (!F)
    ঘূৰাই F.getError();

  ঘূৰাই (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<ৱর্ণ> &Path) ধ্রুৱ {
  যদি (llvm::sys::path::is_absolute(Path))
    ঘূৰাই {};

  auto WorkingDir = getCurrentWorkingDirectory();
  যদি (!WorkingDir)
    ঘূৰাই WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  ঘূৰাই {};
}

std::error_code FileSystem::getRealPath(ধ্রুৱ Twine &Path,
                                        SmallVectorImpl<ৱর্ণ> &Output) {
  ঘূৰাই errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(ধ্রুৱ Twine &Path, তার্কিক &Result) {
  ঘূৰাই errc::operation_not_permitted;
}

তার্কিক FileSystem::exists(ধ্রুৱ Twine &Path) {
  auto Status = status(Path);
  ঘূৰাই Status && Status->exists();
}

llvm::ErrorOr<তার্কিক> FileSystem::equivalent(ধ্রুৱ Twine &A, ধ্রুৱ Twine &B) {
  auto StatusA = status(A);
  যদি (!StatusA)
    ঘূৰাই StatusA.getError();
  auto StatusB = status(B);
  যদি (!StatusB)
    ঘূৰাই StatusB.getError();
  ঘূৰাই StatusA->equivalent(*StatusB);
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
শূন্য FileSystem::dump() ধ্রুৱ { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
স্থির তার্কিক isTraversalComponent(StringRef Component) {
  ঘূৰাই Component == ".." || Component == ".";
}

স্থির তার্কিক pathHasTraversal(StringRef Path) {
  using নামাকাশ llvm::sys;

  বাবে (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    যদি (isTraversalComponent(Comp))
      ঘূৰাই সঁচা;
  ঘূৰাই মিছা;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

নামাকাশ {

/// Wrapper around a raw file descriptor.
শ্ৰেণী RealFile : সার্ৱজনিক File {
  friend শ্ৰেণী RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    প্রতিজ্ঞা(FD.isValid() && "Invalid or inactive file descriptor");
  }

সার্ৱজনিক:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(ধ্রুৱ Twine &Name,
                                                   int64_t FileSize,
                                                   তার্কিক RequiresNullTerminator,
                                                   তার্কিক IsVolatile) override;
  std::error_code close() override;
  শূন্য setPath(ধ্রুৱ Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  প্রতিজ্ঞা(FD.isValid() && "cannot stat closed file");
  যদি (!S.isStatusKnown()) {
    file_status RealStatus;
    যদি (std::error_code EC = sys::fs::status(FD, RealStatus))
      ঘূৰাই EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  ঘূৰাই S;
}

ErrorOr<std::string> RealFile::getName() {
  ঘূৰাই RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(ধ্রুৱ Twine &Name, int64_t FileSize,
                    তার্কিক RequiresNullTerminator, তার্কিক IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  প্রতিজ্ঞা(FD.isValid() && "cannot get buffer for closed file");
  ঘূৰাই MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ঘূৰাই EC;
}

শূন্য RealFile::setPath(ধ্রুৱ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  যদি (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
}

নামাকাশ {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
শ্ৰেণী RealFileSystem : সার্ৱজনিক FileSystem {
সার্ৱজনিক:
  explicit RealFileSystem(তার্কিক LinkCWDToProcess) {
    যদি (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      যদি (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      অন্যথা যদি (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      অন্যথা
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(ধ্রুৱ Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ধ্রুৱ Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_দ্বিমিক(ধ্রুৱ Twine &Path) override;
  directory_iterator dir_begin(ধ্রুৱ Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ধ্রুৱ override;
  std::error_code setCurrentWorkingDirectory(ধ্রুৱ Twine &Path) override;
  std::error_code isLocal(ধ্রুৱ Twine &Path, তার্কিক &Result) override;
  শূন্য
  getDirectoryContentRealSources(ধ্রুৱ Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.ঠেলো_back(Dir.str());
  }
  std::error_code getRealPath(ধ্রুৱ Twine &Path,
                              SmallVectorImpl<ৱর্ণ> &Output) override;

রক্ষিত:
  শূন্য printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) ধ্রুৱ override;

গুপ্ত:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(ধ্রুৱ Twine &Path, SmallVectorImpl<ৱর্ণ> &Storage) ধ্রুৱ {
    যদি (!WD || !*WD)
      ঘূৰাই Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    ঘূৰাই Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ধ্রুৱ Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    যদি (!FDOrErr)
      ঘূৰাই errorToErrorCode(FDOrErr.takeError());
    ঘূৰাই std::unique_ptr<File>(
        নূতন RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  সংরচনা WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(ধ্রুৱ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  যদি (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    ঘূৰাই EC;
  ঘূৰাই Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ধ্রুৱ Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ঘূৰাই openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_দ্বিমিক(ধ্রুৱ Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ঘূৰাই openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ধ্রুৱ {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (WD && *WD)
    ঘূৰাই std::string(WD->get().Specified);
  যদি (WD)
    ঘূৰাই WD->getError();

  SmallString<128> Dir;
  যদি (std::error_code EC = llvm::sys::fs::current_path(Dir))
    ঘূৰাই EC;
  ঘূৰাই std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(ধ্রুৱ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (!WD)
    ঘূৰাই llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  তার্কিক IsDir;
  যদি (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ঘূৰাই Err;
  যদি (!IsDir)
    ঘূৰাই std::make_error_code(std::errc::not_a_directory);
  যদি (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    ঘূৰাই Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ঘূৰাই std::error_code();
}

std::error_code RealFileSystem::isLocal(ধ্রুৱ Twine &Path, তার্কিক &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ঘূৰাই llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(ধ্রুৱ Twine &Path,
                                            SmallVectorImpl<ৱর্ণ> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ঘূৰাই llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

শূন্য RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) ধ্রুৱ {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  যদি (WD)
    OS << "own";
  অন্যথা
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  স্থির IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(সঁচা);
  sys::sandbox::violationIfEnabled();

  ঘূৰাই FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ঘূৰাই std::make_unique<RealFileSystem>(মিছা);
}

নামাকাশ {

শ্ৰেণী RealFSDirIter : সার্ৱজনিক llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

সার্ৱজনিক:
  RealFSDirIter(ধ্রুৱ Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    যদি (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    ঘূৰাই EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ধ্রুৱ Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ঘূৰাই directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.ঠেলো_back(std::move(BaseFS));
}

শূন্য OverlayFileSystem::ঠেলো_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ঠেলো_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(ধ্রুৱ Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  বাবে (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    যদি (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      ঘূৰাই Status;
  }
  ঘূৰাই make_error_code(llvm::errc::no_such_file_or_directory);
}

তার্কিক OverlayFileSystem::exists(ধ্রুৱ Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  বাবে (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    যদি ((*I)->exists(Path))
      ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ধ্রুৱ llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  বাবে (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    যদি (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ঘূৰাই Result;
  }
  ঘূৰাই make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ধ্রুৱ {
  // All file systems are synchronized, just take the first working directory.
  ঘূৰাই FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(ধ্রুৱ Twine &Path) {
  বাবে (auto &FS : FSList)
    যদি (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      ঘূৰাই EC;
  ঘূৰাই {};
}

std::error_code OverlayFileSystem::isLocal(ধ্রুৱ Twine &Path, তার্কিক &Result) {
  বাবে (auto &FS : FSList)
    যদি (FS->exists(Path))
      ঘূৰাই FS->isLocal(Path, Result);
  ঘূৰাই errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(ধ্রুৱ Twine &Path,
                                               SmallVectorImpl<ৱর্ণ> &Output) {
  বাবে (ধ্রুৱ auto &FS : FSList)
    যদি (FS->exists(Path))
      ঘূৰাই FS->getRealPath(Path, Output);
  ঘূৰাই errc::no_such_file_or_directory;
}

শূন্য OverlayFileSystem::getDirectoryContentRealSources(
    ধ্রুৱ Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  বাবে (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

শূন্য OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  বাবে (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

শূন্য OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) ধ্রুৱ {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  যদি (Type == PrintType::Summary)
    ঘূৰাই;

  যদি (Type == PrintType::Contents)
    Type = PrintType::Summary;
  বাবে (ধ্রুৱ auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = স্ৱভাৱ;

নামাকাশ {

/// Combines and deduplicates directory entries across multiple file systems.
শ্ৰেণী CombiningDirIterImpl : সার্ৱজনিক llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(তার্কিক IsFirstTime) {
    যেতিয়ালৈ (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.বেৰ_কৰো_back();
      যদি (CurrentDirIter != directory_iterator())
        ভঙ্গ; // found
    }

    যদি (IsFirstTime && CurrentDirIter == directory_iterator())
      ঘূৰাই errc::no_such_file_or_directory;
    ঘূৰাই {};
  }

  std::error_code incrementDirIter(তার্কিক IsFirstTime) {
    প্রতিজ্ঞা((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    যদি (!IsFirstTime)
      CurrentDirIter.increment(EC);
    যদি (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    ঘূৰাই EC;
  }

  std::error_code incrementImpl(তার্কিক IsFirstTime) {
    যেতিয়ালৈ (সঁচা) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      যদি (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        ঘূৰাই EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      যদি (SeenNames.ঢোকাও(Name).second)
        ঘূৰাই EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

সার্ৱজনিক:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    বাবে (ধ্রুৱ auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      যদি (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ঘূৰাই;
      }
      যদি (!FEC)
        IterList.ঠেলো_back(Iter);
    }
    EC = incrementImpl(সঁচা);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(সঁচা);
  }

  std::error_code increment() override { ঘূৰাই incrementImpl(মিছা); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ধ্রুৱ Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  যদি (EC)
    ঘূৰাই {};
  ঘূৰাই Combined;
}

শূন্য ProxyFileSystem::anchor() {}

নামাকাশ llvm {
নামাকাশ vfs {

নামাকাশ detail {

গণনা In_Memory_গ্ৰন্থি_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
শ্ৰেণী In_Memory_গ্ৰন্থি {
  In_Memory_গ্ৰন্থি_Kind Kind;
  std::string FileName;

সার্ৱজনিক:
  In_Memory_গ্ৰন্থি(llvm::StringRef FileName, In_Memory_গ্ৰন্থি_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  আভাস ~In_Memory_গ্ৰন্থি() = স্ৱভাৱ;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  আভাস Status getStatus(ধ্রুৱ Twine &RequestedName) ধ্রুৱ = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ধ্রুৱ { ঘূৰাই FileName; }
  In_Memory_গ্ৰন্থি_Kind getKind() ধ্রুৱ { ঘূৰাই Kind; }
  আভাস std::string toString(unsigned Indent) ধ্রুৱ = 0;
};

শ্ৰেণী InMemoryFile : সার্ৱজনিক In_Memory_গ্ৰন্থি {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

সার্ৱজনিক:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_গ্ৰন্থি(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(ধ্রুৱ Twine &RequestedName) ধ্রুৱ override {
    ঘূৰাই Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() ধ্রুৱ { ঘূৰাই Buffer.get(); }

  std::string toString(unsigned Indent) ধ্রুৱ override {
    ঘূৰাই (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  স্থির তার্কিক classof(ধ্রুৱ In_Memory_গ্ৰন্থি *N) {
    ঘূৰাই N->getKind() == IME_File;
  }
};

নামাকাশ {

শ্ৰেণী InMemoryHardLink : সার্ৱজনিক In_Memory_গ্ৰন্থি {
  ধ্রুৱ InMemoryFile &ResolvedFile;

সার্ৱজনিক:
  InMemoryHardLink(StringRef Path, ধ্রুৱ InMemoryFile &ResolvedFile)
      : In_Memory_গ্ৰন্থি(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ধ্রুৱ InMemoryFile &getResolvedFile() ধ্রুৱ { ঘূৰাই ResolvedFile; }

  Status getStatus(ধ্রুৱ Twine &RequestedName) ধ্রুৱ override {
    ঘূৰাই ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) ধ্রুৱ override {
    ঘূৰাই std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  স্থির তার্কিক classof(ধ্রুৱ In_Memory_গ্ৰন্থি *N) {
    ঘূৰাই N->getKind() == IME_HardLink;
  }
};

শ্ৰেণী InMemorySymbolicLink : সার্ৱজনিক In_Memory_গ্ৰন্থি {
  std::string TargetPath;
  Status Stat;

সার্ৱজনিক:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_গ্ৰন্থি(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) ধ্রুৱ override {
    ঘূৰাই std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(ধ্রুৱ Twine &RequestedName) ধ্রুৱ override {
    ঘূৰাই Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() ধ্রুৱ { ঘূৰাই TargetPath; }

  স্থির তার্কিক classof(ধ্রুৱ In_Memory_গ্ৰন্থি *N) {
    ঘূৰাই N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
শ্ৰেণী InMemoryFileAdaptor : সার্ৱজনিক File {
  ধ্রুৱ InMemoryFile &গ্ৰন্থি;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

সার্ৱজনিক:
  explicit InMemoryFileAdaptor(ধ্রুৱ InMemoryFile &গ্ৰন্থি,
                               std::string RequestedName)
      : গ্ৰন্থি(গ্ৰন্থি), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    ঘূৰাই গ্ৰন্থি.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(ধ্রুৱ Twine &Name, int64_t FileSize, তার্কিক RequiresNullTerminator,
            তার্কিক IsVolatile) override {
    llvm::MemoryBuffer *Buf = গ্ৰন্থি.getBuffer();
    ঘূৰাই llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { ঘূৰাই {}; }

  শূন্য setPath(ধ্রুৱ Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

শ্ৰেণী InMemoryDirectory : সার্ৱজনিক In_Memory_গ্ৰন্থি {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_গ্ৰন্থি>, std::less<>> Entries;

সার্ৱজনিক:
  InMemoryDirectory(Status Stat)
      : In_Memory_গ্ৰন্থি(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(ধ্রুৱ Twine &RequestedName) ধ্রুৱ override {
    ঘূৰাই Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ধ্রুৱ { ঘূৰাই Stat.getUniqueID(); }

  In_Memory_গ্ৰন্থি *getChild(StringRef Name) ধ্রুৱ {
    auto I = Entries.find(Name);
    যদি (I != Entries.end())
      ঘূৰাই I->second.get();
    ঘূৰাই nullptr;
  }

  In_Memory_গ্ৰন্থি *addChild(StringRef Name, std::unique_ptr<In_Memory_গ্ৰন্থি> Child) {
    ঘূৰাই Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ধ্রুৱ { ঘূৰাই Entries.begin(); }
  const_iterator end() ধ্রুৱ { ঘূৰাই Entries.end(); }

  std::string toString(unsigned Indent) ধ্রুৱ override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    বাবে (ধ্রুৱ auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    ঘূৰাই Result;
  }

  স্থির তার্কিক classof(ধ্রুৱ In_Memory_গ্ৰন্থি *N) {
    ঘূৰাই N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
স্থির sys::fs::UniqueID getUniqueID(hash_code Hash) {
  ঘূৰাই sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
স্থির sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ঘূৰাই getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
স্থির sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  ঘূৰাই getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_গ্ৰন্থি_Info::makeStatus() ধ্রুৱ {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  ঘূৰাই Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(তার্কিক UseNormalizedPaths)
    : মূল_2(নূতন detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = স্ৱভাৱ;

std::string InMemoryFileSystem::toString() ধ্রুৱ {
  ঘূৰাই মূল_2->toString(/*Indent=*/0);
}

তার্কিক InMemoryFileSystem::addFile(ধ্রুৱ Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_গ্ৰন্থি_Fn Make_গ্ৰন্থি) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  প্রতিজ্ঞা(!EC);
  (শূন্য)EC;

  যদি (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/সঁচা);

  যদি (Path.empty())
    ঘূৰাই মিছা;

  detail::InMemoryDirectory *Dir = মূল_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  ধ্রুৱ auto ResolvedUser = User.মান_or(0);
  ধ্রুৱ auto ResolvedGroup = Group.মান_or(0);
  ধ্রুৱ auto ResolvedType = Type.মান_or(sys::fs::file_type::regular_file);
  ধ্রুৱ auto ResolvedPerms = Perms.মান_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ধ্রুৱ auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  যেতিয়ালৈ (সঁচা) {
    Name = *I;
    ++I;
    যদি (I == E)
      ভঙ্গ;
    detail::In_Memory_গ্ৰন্থি *গ্ৰন্থি = Dir->getChild(Name);
    যদি (!গ্ৰন্থি) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      অনুৱর্ত;
    }
    // Creating file under another file.
    যদি (!isa<detail::InMemoryDirectory>(গ্ৰন্থি))
      ঘূৰাই মিছা;
    Dir = cast<detail::InMemoryDirectory>(গ্ৰন্থি);
  }
  detail::In_Memory_গ্ৰন্থি *গ্ৰন্থি = Dir->getChild(Name);
  যদি (!গ্ৰন্থি) {
    Dir->addChild(Name,
                  Make_গ্ৰন্থি({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    ঘূৰাই সঁচা;
  }
  যদি (isa<detail::InMemoryDirectory>(গ্ৰন্থি))
    ঘূৰাই ResolvedType == sys::fs::file_type::directory_file;

  প্রতিজ্ঞা((isa<detail::InMemoryFile>(গ্ৰন্থি) ||
          isa<detail::InMemoryHardLink>(গ্ৰন্থি)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  যদি (auto *Link = dyn_cast<detail::InMemoryHardLink>(গ্ৰন্থি)) {
    ঘূৰাই Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  ঘূৰাই cast<detail::InMemoryFile>(গ্ৰন্থি)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

তার্কিক InMemoryFileSystem::addFile(ধ্রুৱ Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ঘূৰাই addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_গ্ৰন্থি_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_গ্ৰন্থি> {
                   Status Stat = NNI.makeStatus();
                   যদি (Stat.getType() == sys::fs::file_type::directory_file)
                     ঘূৰাই std::make_unique<detail::InMemoryDirectory>(Stat);
                   ঘূৰাই std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

তার্কিক InMemoryFileSystem::addFileNoOwn(
    ধ্রুৱ Twine &P, time_t ModificationTime,
    ধ্রুৱ llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ঘূৰাই addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_গ্ৰন্থি_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_গ্ৰন্থি> {
                   Status Stat = NNI.makeStatus();
                   যদি (Stat.getType() == sys::fs::file_type::directory_file)
                     ঘূৰাই std::make_unique<detail::InMemoryDirectory>(Stat);
                   ঘূৰাই std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_গ্ৰন্থি_Or_Error
InMemoryFileSystem::lookup_গ্ৰন্থি(ধ্রুৱ Twine &P, তার্কিক FollowFinalSymlink,
                               size_t SymlinkDepth) ধ্রুৱ {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  প্রতিজ্ঞা(!EC);
  (শূন্য)EC;

  যদি (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/সঁচা);

  ধ্রুৱ detail::InMemoryDirectory *Dir = মূল_2.get();
  যদি (Path.empty())
    ঘূৰাই detail::Named_গ্ৰন্থি_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  যেতিয়ালৈ (সঁচা) {
    detail::In_Memory_গ্ৰন্থি *গ্ৰন্থি = Dir->getChild(*I);
    ++I;
    যদি (!গ্ৰন্থি)
      ঘূৰাই errc::no_such_file_or_directory;

    যদি (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(গ্ৰন্থি)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      যদি (I == E && !FollowFinalSymlink)
        ঘূৰাই detail::Named_গ্ৰন্থি_Or_Error(Path, Symlink);

      যদি (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        ঘূৰাই errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      যদি (std::error_code EC = makeAbsolute(TargetPath))
        ঘূৰাই EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_গ্ৰন্থি(TargetPath, /*FollowFinalSymlink=*/সঁচা, SymlinkDepth + 1);
      যদি (!Target || I == E)
        ঘূৰাই Target;

      যদি (!isa<detail::InMemoryDirectory>(*Target))
        ঘূৰাই errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      অনুৱর্ত;
    }

    // Return the file if it's at the end of the path.
    যদি (auto File = dyn_cast<detail::InMemoryFile>(গ্ৰন্থি)) {
      যদি (I == E)
        ঘূৰাই detail::Named_গ্ৰন্থি_Or_Error(Path, File);
      ঘূৰাই errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    যদি (auto File = dyn_cast<detail::InMemoryHardLink>(গ্ৰন্থি)) {
      যদি (I == E)
        ঘূৰাই detail::Named_গ্ৰন্থি_Or_Error(Path, &File->getResolvedFile());
      ঘূৰাই errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(গ্ৰন্থি);
    যদি (I == E)
      ঘূৰাই detail::Named_গ্ৰন্থি_Or_Error(Path, Dir);
  }
}

তার্কিক InMemoryFileSystem::addHardLink(ধ্রুৱ Twine &NewLink,
                                     ধ্রুৱ Twine &Target) {
  auto New_Link_গ্ৰন্থি = lookup_গ্ৰন্থি(NewLink, /*FollowFinalSymlink=*/মিছা);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_গ্ৰন্থি = lookup_গ্ৰন্থি(Target, /*FollowFinalSymlink=*/সঁচা);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  যদি (!Target_গ্ৰন্থি || New_Link_গ্ৰন্থি || !isa<detail::InMemoryFile>(*Target_গ্ৰন্থি))
    ঘূৰাই মিছা;
  ঘূৰাই addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_গ্ৰন্থি_Info NNI) {
                   ঘূৰাই std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_গ্ৰন্থি));
                 });
}

তার্কিক InMemoryFileSystem::addSymbolicLink(
    ধ্রুৱ Twine &NewLink, ধ্রুৱ Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_গ্ৰন্থি = lookup_গ্ৰন্থি(NewLink, /*FollowFinalSymlink=*/মিছা);
  যদি (New_Link_গ্ৰন্থি)
    ঘূৰাই মিছা;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  ঘূৰাই addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_গ্ৰন্থি_Info NNI) {
                   ঘূৰাই std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(ধ্রুৱ Twine &Path) {
  auto গ্ৰন্থি = lookup_গ্ৰন্থি(Path, /*FollowFinalSymlink=*/সঁচা);
  যদি (গ্ৰন্থি)
    ঘূৰাই (*গ্ৰন্থি)->getStatus(Path);
  ঘূৰাই গ্ৰন্থি.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(ধ্রুৱ Twine &Path) {
  auto গ্ৰন্থি = lookup_গ্ৰন্থি(Path,/*FollowFinalSymlink=*/সঁচা);
  যদি (!গ্ৰন্থি)
    ঘূৰাই গ্ৰন্থি.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  যদি (auto *F = dyn_cast<detail::InMemoryFile>(*গ্ৰন্থি))
    ঘূৰাই std::unique_ptr<File>(
        নূতন detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  ঘূৰাই make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
শ্ৰেণী InMemoryFileSystem::DirIterator : সার্ৱজনিক llvm::vfs::detail::DirIterImpl {
  ধ্রুৱ InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  শূন্য setCurrentEntry() {
    যদি (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ৱিকল্প (I->second->getKind()) {
      পক্ষ detail::IME_File:
      পক্ষ detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        ভঙ্গ;
      পক্ষ detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        ভঙ্গ;
      পক্ষ detail::IME_SymbolicLink:
        যদি (auto SymlinkTarget =
                FS->lookup_গ্ৰন্থি(Path, /*FollowFinalSymlink=*/সঁচা)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        ভঙ্গ;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } অন্যথা {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

সার্ৱজনিক:
  DirIterator() = স্ৱভাৱ;

  DirIterator(ধ্রুৱ InMemoryFileSystem *FS,
              ধ্রুৱ detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    ঘূৰাই {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(ধ্রুৱ Twine &Dir,
                                                 std::error_code &EC) {
  auto গ্ৰন্থি = lookup_গ্ৰন্থি(Dir, /*FollowFinalSymlink=*/সঁচা);
  যদি (!গ্ৰন্থি) {
    EC = গ্ৰন্থি.getError();
    ঘূৰাই directory_iterator(std::make_shared<DirIterator>());
  }

  যদি (auto *Dir_গ্ৰন্থি = dyn_cast<detail::InMemoryDirectory>(*গ্ৰন্থি))
    ঘূৰাই directory_iterator(
        std::make_shared<DirIterator>(এতত্, *Dir_গ্ৰন্থি, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  ঘূৰাই directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(ধ্রুৱ Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  প্রতিজ্ঞা(!EC);
  (শূন্য)EC;

  যদি (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/সঁচা);

  যদি (!Path.empty())
    WorkingDirectory = std::string(Path);
  ঘূৰাই {};
}

std::error_code InMemoryFileSystem::getRealPath(ধ্রুৱ Twine &Path,
                                                SmallVectorImpl<ৱর্ণ> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  যদি (!CWD || CWD->empty())
    ঘূৰাই errc::operation_not_permitted;
  Path.toVector(Output);
  যদি (auto EC = makeAbsolute(Output))
    ঘূৰাই EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/সঁচা);
  ঘূৰাই {};
}

std::error_code InMemoryFileSystem::isLocal(ধ্রুৱ Twine &Path, তার্কিক &Result) {
  Result = মিছা;
  ঘূৰাই {};
}

শূন্য InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) ধ্রুৱ {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

নামাকাশ {

স্থির llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  ধ্রুৱ size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  যদি (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  ঘূৰাই style;
}

/// Removes leading "./" as well as path components like ".." and ".".
স্থির llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/সঁচা, style);
  ঘূৰাই result;
}

/// Whether the error and entry specify a file/directory that was not found.
স্থির তার্কিক isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  যদি (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    ঘূৰাই মিছা;
  ঘূৰাই EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  প্রতিজ্ঞা(ExternalFS && "RedirectingFileSystem requires an external FS");
  যদি (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
শ্ৰেণী llvm::vfs::RedirectingFSDirIterImpl
    : সার্ৱজনিক llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(তার্কিক IsFirstTime) {
    প্রতিজ্ঞা((IsFirstTime || Current != End) && "cannot iterate past end");
    যদি (!IsFirstTime)
      ++Current;
    যদি (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ৱিকল্প ((*Current)->getKind()) {
      পক্ষ RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      পক্ষ RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        ভঙ্গ;
      পক্ষ RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        ভঙ্গ;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } অন্যথা {
      CurrentEntry = directory_entry();
    }
    ঘূৰাই {};
  };

সার্ৱজনিক:
  RedirectingFSDirIterImpl(
      ধ্রুৱ Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/সঁচা);
  }

  std::error_code increment() override {
    ঘূৰাই incrementImpl(/*IsFirstTime=*/মিছা);
  }
};

নামাকাশ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
শ্ৰেণী RedirectingFSDirRemapIterImpl : সার্ৱজনিক llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

সার্ৱজনিক:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    যদি (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  শূন্য setCurrentEntry() {
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
    যদি (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    অন্যথা
      CurrentEntry = directory_entry();
    ঘূৰাই EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ধ্রুৱ {
  ঘূৰাই WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(ধ্রুৱ Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  যদি (!exists(Path))
    ঘূৰাই errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  যদি (std::error_code EC = makeAbsolute(AbsolutePath))
    ঘূৰাই EC;
  WorkingDirectory = std::string(AbsolutePath);
  ঘূৰাই {};
}

std::error_code RedirectingFileSystem::isLocal(ধ্রুৱ Twine &Path_,
                                               তার্কিক &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  যদি (makeAbsolute(Path))
    ঘূৰাই {};

  ঘূৰাই ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<ৱর্ণ> &Path) ধ্রুৱ {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  যদি (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ঘূৰাই {};

  auto WorkingDir = getCurrentWorkingDirectory();
  যদি (!WorkingDir)
    ঘূৰাই WorkingDir.getError();

  ঘূৰাই makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<ৱর্ণ> &Path) ধ্রুৱ {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  যদি (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    ঘূৰাই std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  যদি (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } অন্যথা {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    যদি (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  যদি (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  ঘূৰাই {};
}

directory_iterator RedirectingFileSystem::dir_begin(ধ্রুৱ Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  যদি (EC)
    ঘূৰাই {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  যদি (!Result) {
    যদি (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      ঘূৰাই ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    ঘূৰাই {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  যদি (!S) {
    যদি (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      ঘূৰাই ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ঘূৰাই {};
  }

  যদি (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ঘূৰাই {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  যদি (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    যদি (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } অন্যথা {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  যদি (RedirectEC) {
    যদি (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ঘূৰাই {};
    }
    RedirectIter = {};
  }

  যদি (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    ঘূৰাই RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  যদি (ExternalEC) {
    যদি (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ঘূৰাই {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  ৱিকল্প (Redirection) {
  পক্ষ RedirectKind::Fallthrough:
    Iters.ঠেলো_back(ExternalIter);
    Iters.ঠেলো_back(RedirectIter);
    ভঙ্গ;
  পক্ষ RedirectKind::Fallback:
    Iters.ঠেলো_back(RedirectIter);
    Iters.ঠেলো_back(ExternalIter);
    ভঙ্গ;
  স্ৱভাৱ:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  যদি (EC)
    ঘূৰাই {};
  ঘূৰাই Combined;
}

শূন্য RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ধ্রুৱ {
  ঘূৰাই OverlayFileDir;
}

শূন্য RedirectingFileSystem::setFallthrough(তার্কিক Fallthrough) {
  যদি (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } অন্যথা {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

শূন্য RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() ধ্রুৱ {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  বাবে (ধ্রুৱ auto &মূল_2 : Roots)
    R.ঠেলো_back(মূল_2->getName());
  ঘূৰাই R;
}

শূন্য RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) ধ্রুৱ {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  যদি (Type == PrintType::Summary)
    ঘূৰাই;

  বাবে (ধ্রুৱ auto &মূল_2 : Roots)
    printEntry(OS, মূল_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

শূন্য RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) ধ্রুৱ {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  ৱিকল্প (E->getKind()) {
  পক্ষ EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    বাবে (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    ভঙ্গ;
  }
  পক্ষ EK_DirectoryRemap:
  পক্ষ EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    ৱিকল্প (RE->getUseName()) {
    পক্ষ NK_NotSet:
      ভঙ্গ;
    পক্ষ NK_External:
      OS << " (UseExternalName: true)";
      ভঙ্গ;
    পক্ষ NK_Virtual:
      OS << " (UseExternalName: false)";
      ভঙ্গ;
    }
    OS << "\n";
    ভঙ্গ;
  }
  }
}

শূন্য RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  যদি (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
শ্ৰেণী llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  শূন্য error(yaml::গ্ৰন্থি *N, ধ্রুৱ Twine &বাৰ্তা_2) { Stream.printError(N, বাৰ্তা_2); }

  // false on error
  তার্কিক parseScalarString(yaml::গ্ৰন্থি *N, StringRef &Result,
                         SmallVectorImpl<ৱর্ণ> &Storage) {
    ধ্রুৱ auto *S = dyn_cast_if_present<yaml::Scalar_গ্ৰন্থি>(N);

    যদি (!S) {
      error(N, "expected string");
      ঘূৰাই মিছা;
    }
    Result = S->get_মান(Storage);
    ঘূৰাই সঁচা;
  }

  // false on error
  তার্কিক parseScalarBool(yaml::গ্ৰন্থি *N, তার্কিক &Result) {
    SmallString<5> Storage;
    StringRef মান;
    যদি (!parseScalarString(N, মান, Storage))
      ঘূৰাই মিছা;

    যদি (মান.equals_insensitive("true") || মান.equals_insensitive("on") ||
        মান.equals_insensitive("yes") || মান == "1") {
      Result = সঁচা;
      ঘূৰাই সঁচা;
    } অন্যথা যদি (মান.equals_insensitive("false") ||
               মান.equals_insensitive("off") ||
               মান.equals_insensitive("no") || মান == "0") {
      Result = মিছা;
      ঘূৰাই সঁচা;
    }

    error(N, "expected boolean value");
    ঘূৰাই মিছা;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::গ্ৰন্থি *N) {
    SmallString<12> Storage;
    StringRef মান;
    যদি (!parseScalarString(N, মান, Storage))
      ঘূৰাই std::nullopt;

    যদি (মান.equals_insensitive("fallthrough")) {
      ঘূৰাই RedirectingFileSystem::RedirectKind::Fallthrough;
    } অন্যথা যদি (মান.equals_insensitive("fallback")) {
      ঘূৰাই RedirectingFileSystem::RedirectKind::Fallback;
    } অন্যথা যদি (মান.equals_insensitive("redirect-only")) {
      ঘূৰাই RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    ঘূৰাই std::nullopt;
  }

  std::optional<RedirectingFileSystem::মূল_Relative_Kind>
  parse_মূল_Relative_Kind(yaml::গ্ৰন্থি *N) {
    SmallString<12> Storage;
    StringRef মান;
    যদি (!parseScalarString(N, মান, Storage))
      ঘূৰাই std::nullopt;
    যদি (মান.equals_insensitive("cwd")) {
      ঘূৰাই RedirectingFileSystem::মূল_Relative_Kind::CWD;
    } অন্যথা যদি (মান.equals_insensitive("overlay-dir")) {
      ঘূৰাই RedirectingFileSystem::মূল_Relative_Kind::OverlayDir;
    }
    ঘূৰাই std::nullopt;
  }

  সংরচনা চাবি_Status {
    তার্কিক Required;
    তার্কিক Seen = মিছা;

    চাবি_Status(তার্কিক Required = মিছা) : Required(Required) {}
  };

  using চাবি_Status_Pair = std::pair<StringRef, চাবি_Status>;

  // false on error
  তার্কিক check_Duplicate_Or_Unknown_চাবি(yaml::গ্ৰন্থি *চাবি_গ্ৰন্থি, StringRef চাবি,
                                  DenseMap<StringRef, চাবি_Status> &চাবিগুলি_2) {
    auto It = চাবিগুলি_2.find(চাবি);
    যদি (It == চাবিগুলি_2.end()) {
      error(চাবি_গ্ৰন্থি, "unknown key");
      ঘূৰাই মিছা;
    }
    চাবি_Status &S = It->second;
    যদি (S.Seen) {
      error(চাবি_গ্ৰন্থি, Twine("duplicate key '") + চাবি + "'");
      ঘূৰাই মিছা;
    }
    S.Seen = সঁচা;
    ঘূৰাই সঁচা;
  }

  // false on error
  তার্কিক check_Missing_চাবিগুলি(yaml::গ্ৰন্থি *Obj, DenseMap<StringRef, চাবি_Status> &চাবিগুলি_2) {
    বাবে (ধ্রুৱ auto &I : চাবিগুলি_2) {
      যদি (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        ঘূৰাই মিছা;
      }
    }
    ঘূৰাই সঁচা;
  }

সার্ৱজনিক:
  স্থির RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    যদি (!ParentEntry) { // Look for a existent root
      বাবে (ধ্রুৱ auto &মূল_2 : FS->Roots) {
        যদি (Name == মূল_2->getName()) {
          ParentEntry = মূল_2.get();
          ঘূৰাই ParentEntry;
        }
      }
    } অন্যথা { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      বাবে (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        যদি (DirContent && Name == Content->getName())
          ঘূৰাই DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    যদি (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.ঠেলো_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      ঘূৰাই ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    ঘূৰাই DE->getLastContent();
  }

গুপ্ত:
  শূন্য unique_Overlay_বৃক্ষ(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    ৱিকল্প (SrcE->getKind()) {
    পক্ষ RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      যদি (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      বাবে (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_বৃক্ষ(FS, SubEntry.get(), NewParentE);
      ভঙ্গ;
    }
    পক্ষ RedirectingFileSystem::EK_DirectoryRemap: {
      প্রতিজ্ঞা(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      ভঙ্গ;
    }
    পক্ষ RedirectingFileSystem::EK_File: {
      প্রতিজ্ঞা(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      ভঙ্গ;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::গ্ৰন্থি *N, RedirectingFileSystem *FS, তার্কিক Is_মূল_Entry) {
    auto *M = dyn_cast<yaml::Mapping_গ্ৰন্থি>(N);
    যদি (!M) {
      error(N, "expected mapping node for file or directory entry");
      ঘূৰাই nullptr;
    }

    চাবি_Status_Pair Fields[] = {
        চাবি_Status_Pair("name", সঁচা),
        চাবি_Status_Pair("type", সঁচা),
        চাবি_Status_Pair("contents", মিছা),
        চাবি_Status_Pair("external-contents", মিছা),
        চাবি_Status_Pair("use-external-name", মিছা),
    };

    DenseMap<StringRef, চাবি_Status> চাবিগুলি_2(std::begin(Fields), std::end(Fields));

    গণনা { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::গ্ৰন্থি *Name_মান_গ্ৰন্থি = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    বাবে (auto &I : *M) {
      StringRef চাবি;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      যদি (!parseScalarString(I.get_চাবি(), চাবি, Buffer))
        ঘূৰাই nullptr;

      যদি (!check_Duplicate_Or_Unknown_চাবি(I.get_চাবি(), চাবি, চাবিগুলি_2))
        ঘূৰাই nullptr;

      StringRef মান;
      যদি (চাবি == "name") {
        যদি (!parseScalarString(I.get_মান(), মান, Buffer))
          ঘূৰাই nullptr;

        Name_মান_গ্ৰন্থি = I.get_মান();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(মান).str();
      } অন্যথা যদি (চাবি == "type") {
        যদি (!parseScalarString(I.get_মান(), মান, Buffer))
          ঘূৰাই nullptr;
        যদি (মান == "file")
          Kind = RedirectingFileSystem::EK_File;
        অন্যথা যদি (মান == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        অন্যথা যদি (মান == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        অন্যথা {
          error(I.get_মান(), "unknown value for 'type'");
          ঘূৰাই nullptr;
        }
      } অন্যথা যদি (চাবি == "contents") {
        যদি (ContentsField != CF_NotSet) {
          error(I.get_চাবি(),
                "entry already has 'contents' or 'external-contents'");
          ঘূৰাই nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_গ্ৰন্থি>(I.get_মান());
        যদি (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_মান(), "expected array");
          ঘূৰাই nullptr;
        }

        বাবে (auto &I : *Contents) {
          যদি (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ মিছা))
            EntryArrayContents.ঠেলো_back(std::move(E));
          অন্যথা
            ঘূৰাই nullptr;
        }
      } অন্যথা যদি (চাবি == "external-contents") {
        যদি (ContentsField != CF_NotSet) {
          error(I.get_চাবি(),
                "entry already has 'contents' or 'external-contents'");
          ঘূৰাই nullptr;
        }
        ContentsField = CF_External;
        যদি (!parseScalarString(I.get_মান(), মান, Buffer))
          ঘূৰাই nullptr;

        SmallString<256> FullPath;
        যদি (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          প্রতিজ্ঞা(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = মান;
          যদি (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            ঘূৰাই nullptr;
          }
          FullPath = AbsFullPath;
        } অন্যথা {
          FullPath = মান;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } অন্যথা যদি (চাবি == "use-external-name") {
        তার্কিক Val;
        যদি (!parseScalarBool(I.get_মান(), Val))
          ঘূৰাই nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } অন্যথা {
        llvm_unreachable("key missing from Keys");
      }
    }

    যদি (Stream.failed())
      ঘূৰাই nullptr;

    // check for missing keys
    যদি (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      ঘূৰাই nullptr;
    }
    যদি (!check_Missing_চাবিগুলি(N, চাবিগুলি_2))
      ঘূৰাই nullptr;

    // check invalid configuration
    যদি (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ঘূৰাই nullptr;
    }

    যদি (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ঘূৰাই nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    যদি (Is_মূল_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      যদি (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } অন্যথা যদি (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } অন্যথা {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        যদি (FS->মূল_Relative ==
            RedirectingFileSystem::মূল_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          প্রতিজ্ঞা(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } অন্যথা {
          EC = FS->makeAbsolute(Name);
        }
        যদি (EC) {
          প্রতিজ্ঞা(Name_মান_গ্ৰন্থি && "Name presence should be checked earlier");
          error(
              Name_মান_গ্ৰন্থি,
              "entry with relative path at the root level is not discoverable");
          ঘূৰাই nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      যদি (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t মূল_Path_Len = sys::path::মূল_path(Trimmed, path_style).size();
    যেতিয়ালৈ (Trimmed.size() > মূল_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    ৱিকল্প (Kind) {
    পক্ষ RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      ভঙ্গ;
    পক্ষ RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      ভঙ্গ;
    পক্ষ RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      ভঙ্গ;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    যদি (Parent.empty())
      ঘূৰাই Result;

    // if 'name' contains multiple components, create implicit directory entries
    বাবে (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.ঠেলো_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ঘূৰাই Result;
  }

সার্ৱজনিক:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  তার্কিক parse(yaml::গ্ৰন্থি *মূল_2, RedirectingFileSystem *FS) {
    auto *শীৰ্ষ = dyn_cast<yaml::Mapping_গ্ৰন্থি>(মূল_2);
    যদি (!শীৰ্ষ) {
      error(মূল_2, "expected mapping node");
      ঘূৰাই মিছা;
    }

    চাবি_Status_Pair Fields[] = {
        চাবি_Status_Pair("version", সঁচা),
        চাবি_Status_Pair("case-sensitive", মিছা),
        চাবি_Status_Pair("use-external-names", মিছা),
        চাবি_Status_Pair("root-relative", মিছা),
        চাবি_Status_Pair("overlay-relative", মিছা),
        চাবি_Status_Pair("fallthrough", মিছা),
        চাবি_Status_Pair("redirecting-with", মিছা),
        চাবি_Status_Pair("roots", সঁচা),
    };

    DenseMap<StringRef, চাবি_Status> চাবিগুলি_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> মূল_Entries;

    // Parse configuration and 'roots'
    বাবে (auto &I : *শীৰ্ষ) {
      SmallString<10> চাবি_Buffer;
      StringRef চাবি;
      যদি (!parseScalarString(I.get_চাবি(), চাবি, চাবি_Buffer))
        ঘূৰাই মিছা;

      যদি (!check_Duplicate_Or_Unknown_চাবি(I.get_চাবি(), চাবি, চাবিগুলি_2))
        ঘূৰাই মিছা;

      যদি (চাবি == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_গ্ৰন্থি>(I.get_মান());
        যদি (!Roots) {
          error(I.get_মান(), "expected array");
          ঘূৰাই মিছা;
        }

        বাবে (auto &I : *Roots) {
          যদি (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ সঁচা))
            মূল_Entries.ঠেলো_back(std::move(E));
          অন্যথা
            ঘূৰাই মিছা;
        }
      } অন্যথা যদি (চাবি == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        যদি (!parseScalarString(I.get_মান(), VersionString, Storage))
          ঘূৰাই মিছা;
        পূর্ণাঙ্ক Version;
        যদি (VersionString.getAsInteger<পূর্ণাঙ্ক>(10, Version)) {
          error(I.get_মান(), "expected integer");
          ঘূৰাই মিছা;
        }
        যদি (Version < 0) {
          error(I.get_মান(), "invalid version number");
          ঘূৰাই মিছা;
        }
        যদি (Version != 0) {
          error(I.get_মান(), "version mismatch, expected 0");
          ঘূৰাই মিছা;
        }
      } অন্যথা যদি (চাবি == "case-sensitive") {
        যদি (!parseScalarBool(I.get_মান(), FS->CaseSensitive))
          ঘূৰাই মিছা;
      } অন্যথা যদি (চাবি == "overlay-relative") {
        যদি (!parseScalarBool(I.get_মান(), FS->IsRelativeOverlay))
          ঘূৰাই মিছা;
      } অন্যথা যদি (চাবি == "use-external-names") {
        যদি (!parseScalarBool(I.get_মান(), FS->UseExternalNames))
          ঘূৰাই মিছা;
      } অন্যথা যদি (চাবি == "fallthrough") {
        যদি (চাবিগুলি_2["redirecting-with"].Seen) {
          error(I.get_মান(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ঘূৰাই মিছা;
        }

        তার্কিক ShouldFallthrough = মিছা;
        যদি (!parseScalarBool(I.get_মান(), ShouldFallthrough))
          ঘূৰাই মিছা;

        যদি (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } অন্যথা {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } অন্যথা যদি (চাবি == "redirecting-with") {
        যদি (চাবিগুলি_2["fallthrough"].Seen) {
          error(I.get_মান(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ঘূৰাই মিছা;
        }

        যদি (auto Kind = parseRedirectKind(I.get_মান())) {
          FS->Redirection = *Kind;
        } অন্যথা {
          error(I.get_মান(), "expected valid redirect kind");
          ঘূৰাই মিছা;
        }
      } অন্যথা যদি (চাবি == "root-relative") {
        যদি (auto Kind = parse_মূল_Relative_Kind(I.get_মান())) {
          FS->মূল_Relative = *Kind;
        } অন্যথা {
          error(I.get_মান(), "expected valid root-relative kind");
          ঘূৰাই মিছা;
        }
      } অন্যথা {
        llvm_unreachable("key missing from Keys");
      }
    }

    যদি (Stream.failed())
      ঘূৰাই মিছা;

    যদি (!check_Missing_চাবিগুলি(শীৰ্ষ, চাবিগুলি_2))
      ঘূৰাই মিছা;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    বাবে (auto &E : মূল_Entries)
      unique_Overlay_বৃক্ষ(FS, E.get());

    ঘূৰাই সঁচা;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, শূন্য *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::গ্ৰন্থি *মূল_2 = DI->get_মূল();
  যদি (DI == Stream.end() || !মূল_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    ঘূৰাই nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      নূতন RedirectingFileSystem(ExternalFS));

  যদি (!YAMLFilePath.empty()) {
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
    প্রতিজ্ঞা(!EC && "Overlay dir final path must be absolute");
    (শূন্য)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  যদি (!P.parse(মূল_2, FS.get()))
    ঘূৰাই nullptr;

  ঘূৰাই FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    তার্কিক UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      নূতন RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  বাবে (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (শূন্য)EC;
      প্রতিজ্ঞা(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    যদি (ToEntry)
      অনুৱর্ত;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    বাবে (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    প্রতিজ্ঞা(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (শূন্য)EC;
      প্রতিজ্ঞা(!EC && "Could not make absolute path");
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

  ঘূৰাই FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  প্রতিজ্ঞা(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  যদি (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

শূন্য RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<ৱর্ণ> &Result) ধ্রুৱ {
  Result.clear();
  বাবে (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<ৱর্ণ> &Path) ধ্রুৱ {
  যদি (std::error_code EC = makeAbsolute(Path))
    ঘূৰাই EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  যদি (CanonicalPath.empty())
    ঘূৰাই make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  ঘূৰাই {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) ধ্রুৱ {
  llvm::SmallString<128> CanonicalPath(Path);
  যদি (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    ঘূৰাই EC;

  // RedirectOnly means the VFS is always used.
  যদি (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = সঁচা;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  বাবে (ধ্রুৱ auto &মূল_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, মূল_2.get(), Entries);
    যদি (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = সঁচা;
    যদি (Result) {
      Result->Parents = std::move(Entries);
      ঘূৰাই Result;
    }

    যদি (Result.getError() != llvm::errc::no_such_file_or_directory)
      ঘূৰাই Result;
  }
  ঘূৰাই make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) ধ্রুৱ {
  প্রতিজ্ঞা(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  যদি (!FromName.empty()) {
    যদি (!pathComponentMatches(*Start, FromName))
      ঘূৰাই make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    যদি (Start == End) {
      // Match!
      ঘূৰাই LookupResult(From, Start, End);
    }
  }

  যদি (isa<RedirectingFileSystem::FileEntry>(From))
    ঘূৰাই make_error_code(llvm::errc::not_a_directory);

  যদি (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    ঘূৰাই LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  বাবে (ধ্রুৱ std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.ঠেলো_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    যদি (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ঘূৰাই Result;
    Entries.বেৰ_কৰো_back();
  }

  ঘূৰাই make_error_code(llvm::errc::no_such_file_or_directory);
}

স্থির Status getRedirectedFileStatus(ধ্রুৱ Twine &OriginalPath,
                                      তার্কিক UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  যদি (ExternalStatus.ExposesExternalVFSPath)
    ঘূৰাই ExternalStatus;

  Status S = ExternalStatus;
  যদি (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  অন্যথা
    S.ExposesExternalVFSPath = সঁচা;
  ঘূৰাই S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    ধ্রুৱ Twine &LookupPath, ধ্রুৱ Twine &OriginalPath,
    ধ্রুৱ RedirectingFileSystem::LookupResult &Result) {
  যদি (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    যদি (std::error_code EC = makeAbsolute(RemappedPath))
      ঘূৰাই EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    যদি (!S)
      ঘূৰাই S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    ঘূৰাই getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  ঘূৰাই Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(ধ্রুৱ Twine &LookupPath,
                                         ধ্রুৱ Twine &OriginalPath) ধ্রুৱ {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  যদি (!Result || Result->ExposesExternalVFSPath)
    ঘূৰাই Result;
  ঘূৰাই Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(ধ্রুৱ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  যদি (std::error_code EC = makeAbsolute(Path))
    ঘূৰাই EC;

  যদি (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    যদি (S)
      ঘূৰাই S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  যদি (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ঘূৰাই getExternalStatus(Path, OriginalPath);
    ঘূৰাই Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  যদি (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ঘূৰাই getExternalStatus(Path, OriginalPath);
  }

  ঘূৰাই S;
}

তার্কিক RedirectingFileSystem::exists(ধ্রুৱ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  যদি (makeAbsolute(Path))
    ঘূৰাই মিছা;

  যদি (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    যদি (ExternalFS->exists(Path))
      ঘূৰাই সঁচা;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  যদি (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ঘূৰাই ExternalFS->exists(Path);
    ঘূৰাই মিছা;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  যদি (!ExtRedirect) {
    প্রতিজ্ঞা(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    ঘূৰাই সঁচা;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  যদি (makeAbsolute(RemappedPath))
    ঘূৰাই মিছা;

  যদি (ExternalFS->exists(RemappedPath))
    ঘূৰাই সঁচা;

  যদি (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ঘূৰাই ExternalFS->exists(Path);
  }

  ঘূৰাই মিছা;
}

নামাকাশ {

/// Provide a file wrapper with an overriden status.
শ্ৰেণী FileWithFixedStatus : সার্ৱজনিক File {
  std::unique_ptr<File> InnerFile;
  Status S;

সার্ৱজনিক:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { ঘূৰাই S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(ধ্রুৱ Twine &Name, int64_t FileSize, তার্কিক RequiresNullTerminator,
            তার্কিক IsVolatile) override {
    ঘূৰাই InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { ঘূৰাই InnerFile->close(); }

  শূন্য setPath(ধ্রুৱ Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, ধ্রুৱ Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  যদি (!Result || (*Result)->status()->ExposesExternalVFSPath)
    ঘূৰাই Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  যদি (Name && Name.get() != P.str())
    F->get()->setPath(P);
  ঘূৰাই F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ধ্রুৱ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  যদি (std::error_code EC = makeAbsolute(Path))
    ঘূৰাই EC;

  যদি (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    যদি (F)
      ঘূৰাই F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  যদি (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ঘূৰাই File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ঘূৰাই Result.getError();
  }

  যদি (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    ঘূৰাই make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  যদি (std::error_code EC = makeAbsolute(RemappedPath))
    ঘূৰাই EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  যদি (!ExternalFile) {
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ঘূৰাই File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    ঘূৰাই ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  যদি (!ExternalStatus)
    ঘূৰাই ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  ঘূৰাই std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(ধ্রুৱ Twine &OriginalPath,
                                   SmallVectorImpl<ৱর্ণ> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  যদি (std::error_code EC = makeAbsolute(Path))
    ঘূৰাই EC;

  যদি (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    যদি (!EC)
      ঘূৰাই EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  যদি (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ঘূৰাই ExternalFS->getRealPath(Path, Output);
    ঘূৰাই Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  যদি (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    যদি (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ঘূৰাই ExternalFS->getRealPath(Path, Output);
    }
    ঘূৰাই P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  যদি (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    ঘূৰাই {};
  }
  ঘূৰাই llvm::errc::invalid_argument;
}

শূন্য RedirectingFileSystem::getDirectoryContentRealSources(
    ধ্রুৱ Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  যদি (makeAbsolute(Path))
    ঘূৰাই;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ধ্রুৱ তার্কিক ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  যদি (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    যদি (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    ঘূৰাই;
  }

  ৱিকল্প (Result->E->getKind()) {
  পক্ষ EK_File:
    ঘূৰাই;
  পক্ষ EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    ভঙ্গ;
  পক্ষ EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    যদি (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    ভঙ্গ;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  যদি (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, শূন্য *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ঘূৰাই RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

স্থির শূন্য getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  যদি (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    প্রতিজ্ঞা(DE && "Must be a directory");
    বাবে (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.ঠেলো_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.বেৰ_কৰো_back();
    }
    ঘূৰাই;
  }

  যদি (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    প্রতিজ্ঞা(DR && "Must be a directory remap");
    SmallString<128> VPath;
    বাবে (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.ঠেলো_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    ঘূৰাই;
  }

  প্রতিজ্ঞা(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  প্রতিজ্ঞা(FE && "Must be a file");
  SmallString<128> VPath;
  বাবে (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.ঠেলো_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

শূন্য vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> মূল_Result = VFS.lookupPath("/");
  যদি (!মূল_Result)
    ঘূৰাই;
  SmallVector<StringRef, 8> Components;
  Components.ঠেলো_back("/");
  getVFSEntries(মূল_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  স্থির std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ঘূৰাই UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

শূন্য YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             তার্কিক IsDirectory) {
  প্রতিজ্ঞা(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  প্রতিজ্ঞা(sys::path::is_absolute(RealPath) && "real path not absolute");
  প্রতিজ্ঞা(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

শূন্য YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/মিছা);
}

শূন্য YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/সঁচা);
}

নামাকাশ {

শ্ৰেণী JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_স্তূপ;

  unsigned getDirIndent() { ঘূৰাই 4 * Dir_স্তূপ.size(); }
  unsigned getFileIndent() { ঘূৰাই 4 * (Dir_স্তূপ.size() + 1); }
  তার্কিক containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  শূন্য startDirectory(StringRef Path);
  শূন্য endDirectory();
  শূন্য writeEntry(StringRef VPath, StringRef RPath);

সার্ৱজনিক:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  শূন্য লিখ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<তার্কিক> UseExternalNames,
             std::optional<তার্কিক> IsCaseSensitive,
             std::optional<তার্কিক> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

তার্কিক JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using নামাকাশ llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  বাবে (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    যদি (*IParent != *IChild)
      ঘূৰাই মিছা;
  }
  // Have we exhausted the parent path?
  ঘূৰাই IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  প্রতিজ্ঞা(!Parent.empty());
  প্রতিজ্ঞা(containedIn(Parent, Path));
  ঘূৰাই Path.substr(Parent.size() + 1);
}

শূন্য JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_স্তূপ.empty() ? Path : containedPart(Dir_স্তূপ.back(), Path);
  Dir_স্তূপ.ঠেলো_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

শূন্য JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_স্তূপ.বেৰ_কৰো_back();
}

শূন্য JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

শূন্য JSONWriter::লিখ(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<তার্কিক> UseExternalNames,
                       std::optional<তার্কিক> IsCaseSensitive,
                       std::optional<তার্কিক> IsOverlayRelative,
                       StringRef OverlayDir) {
  using নামাকাশ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  যদি (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  যদি (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  তার্কিক UseOverlayRelative = মিছা;
  যদি (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  যদি (!Entries.empty()) {
    ধ্রুৱ YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    যদি (UseOverlayRelative) {
      প্রতিজ্ঞা(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    তার্কিক IsCurrentDirEmpty = সঁচা;
    যদি (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = মিছা;
    }

    বাবে (ধ্রুৱ auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      যদি (Dir == Dir_স্তূপ.back()) {
        যদি (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } অন্যথা {
        তার্কিক Is_Dir_Popped_From_স্তূপ = মিছা;
        যেতিয়ালৈ (!Dir_স্তূপ.empty() && !containedIn(Dir_স্তূপ.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_স্তূপ = সঁচা;
        }
        যদি (Is_Dir_Popped_From_স্তূপ || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = সঁচা;
      }
      StringRef RPath = Entry.RPath;
      যদি (UseOverlayRelative) {
        প্রতিজ্ঞা(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      যদি (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = মিছা;
      }
    }

    যেতিয়ালৈ (!Dir_স্তূপ.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

শূন্য YAMLVFSWriter::লিখ(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](ধ্রুৱ YAMLVFSEntry &LHS, ধ্রুৱ YAMLVFSEntry &RHS) {
    ঘূৰাই LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).লিখ(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ধ্রুৱ Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  যদি (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->স্তূপ.ঠেলো_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  প্রতিজ্ঞা(FS && State && !State->স্তূপ.empty() && "incrementing past end");
  প্রতিজ্ঞা(!State->স্তূপ.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  যদি (State->Has_No_ঠেলো_Request)
    State->Has_No_ঠেলো_Request = মিছা;
  অন্যথা {
    যদি (State->স্তূপ.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->স্তূপ.back()->path(), EC);
      যদি (I != End) {
        State->স্তূপ.ঠেলো_back(I);
        ঘূৰাই *এতত্;
      }
    }
  }

  যেতিয়ালৈ (!State->স্তূপ.empty() && State->স্তূপ.back().increment(EC) == End)
    State->স্তূপ.বেৰ_কৰো_back();

  যদি (State->স্তূপ.empty())
    State.reset(); // end iterator

  ঘূৰাই *এতত্;
}

ধ্রুৱ ৱর্ণ FileSystem::ID = 0;
ধ্রুৱ ৱর্ণ OverlayFileSystem::ID = 0;
ধ্রুৱ ৱর্ণ ProxyFileSystem::ID = 0;
ধ্রুৱ ৱর্ণ InMemoryFileSystem::ID = 0;
ধ্রুৱ ৱর্ণ RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_গুনতি(
    ধ্রুৱ FileSystem *FS) {
  ঘূৰাই FS->Use_গুনতি();
}

শূন্য ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

শূন্য ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
