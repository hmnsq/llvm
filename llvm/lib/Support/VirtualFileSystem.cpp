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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualFileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/DenseMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/IntrusiveRefCntPtr.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringSet.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Twine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/iterator_range.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Casting.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Chrono.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorOr.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem/UniqueID.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/IOSandbox.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SMLoc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SourceMgr.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/YAMLParser.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <atomic>
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstdint>
#ଅନ୍ତର୍ଭାଵ <iterator>
#ଅନ୍ତର୍ଭାଵ <limits>
#ଅନ୍ତର୍ଭାଵ <map>
#ଅନ୍ତର୍ଭାଵ <memory>
#ଅନ୍ତର୍ଭାଵ <optional>
#ଅନ୍ତର୍ଭାଵ <string>
#ଅନ୍ତର୍ଭାଵ <system_error>
#ଅନ୍ତର୍ଭାଵ <utility>
#ଅନ୍ତର୍ଭାଵ <vector>

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(ଅଚଳ file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(ଅଚଳ Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(ଅଚଳ Status &In, uint64_t NewSize) {
  ଫେରନ୍ତୁ Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ଅଚଳ Status &In, ଅଚଳ Twine &NewName) {
  ଫେରନ୍ତୁ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ଅଚଳ file_status &In, ଅଚଳ Twine &NewName) {
  ଫେରନ୍ତୁ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

ତାର୍କିକ Status::equivalent(ଅଚଳ Status &Other) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(isStatusKnown() && Other.isStatusKnown());
  ଫେରନ୍ତୁ getUniqueID() == Other.getUniqueID();
}

ତାର୍କିକ Status::isDirectory() ଅଚଳ { ଫେରନ୍ତୁ Type == file_type::directory_file; }

ତାର୍କିକ Status::isRegularFile() ଅଚଳ { ଫେରନ୍ତୁ Type == file_type::regular_file; }

ତାର୍କିକ Status::isOther() ଅଚଳ {
  ଫେରନ୍ତୁ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

ତାର୍କିକ Status::isSymlink() ଅଚଳ { ଫେରନ୍ତୁ Type == file_type::symlink_file; }

ତାର୍କିକ Status::isStatusKnown() ଅଚଳ { ଫେରନ୍ତୁ Type != file_type::status_error; }

ତାର୍କିକ Status::exists() ଅଚଳ {
  ଫେରନ୍ତୁ isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

FileSystem::~FileSystem() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(ଅଚଳ llvm::Twine &Name, int64_t FileSize,
                             ତାର୍କିକ RequiresNullTerminator, ତାର୍କିକ IsVolatile,
                             ତାର୍କିକ IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_ଦ୍ୱିମିକ(Name);
  ଯଦି (!F)
    ଫେରନ୍ତୁ F.getError();

  ଫେରନ୍ତୁ (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<ଅକ୍ଷର> &Path) ଅଚଳ {
  ଯଦି (llvm::sys::path::is_absolute(Path))
    ଫେରନ୍ତୁ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ଯଦି (!WorkingDir)
    ଫେରନ୍ତୁ WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  ଫେରନ୍ତୁ {};
}

std::error_code FileSystem::getRealPath(ଅଚଳ Twine &Path,
                                        SmallVectorImpl<ଅକ୍ଷର> &Output) {
  ଫେରନ୍ତୁ errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(ଅଚଳ Twine &Path, ତାର୍କିକ &Result) {
  ଫେରନ୍ତୁ errc::operation_not_permitted;
}

ତାର୍କିକ FileSystem::exists(ଅଚଳ Twine &Path) {
  auto Status = status(Path);
  ଫେରନ୍ତୁ Status && Status->exists();
}

llvm::ErrorOr<ତାର୍କିକ> FileSystem::equivalent(ଅଚଳ Twine &A, ଅଚଳ Twine &B) {
  auto StatusA = status(A);
  ଯଦି (!StatusA)
    ଫେରନ୍ତୁ StatusA.getError();
  auto StatusB = status(B);
  ଯଦି (!StatusB)
    ଫେରନ୍ତୁ StatusB.getError();
  ଫେରନ୍ତୁ StatusA->equivalent(*StatusB);
}

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
ଶୂନ୍ୟ FileSystem::dump() ଅଚଳ { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
ସ୍ଥିର ତାର୍କିକ isTraversalComponent(StringRef Component) {
  ଫେରନ୍ତୁ Component == ".." || Component == ".";
}

ସ୍ଥିର ତାର୍କିକ pathHasTraversal(StringRef Path) {
  using ନାମାକାଶ llvm::sys;

  ପାଇଁ (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    ଯଦି (isTraversalComponent(Comp))
      ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

ନାମାକାଶ {

/// Wrapper around a raw file descriptor.
ଶ୍ରେଣୀ RealFile : ସର୍ବସାଧାରଣ File {
  friend ଶ୍ରେଣୀ RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    ପ୍ରତିଜ୍ଞା(FD.isValid() && "Invalid or inactive file descriptor");
  }

ସର୍ବସାଧାରଣ:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(ଅଚଳ Twine &Name,
                                                   int64_t FileSize,
                                                   ତାର୍କିକ RequiresNullTerminator,
                                                   ତାର୍କିକ IsVolatile) override;
  std::error_code close() override;
  ଶୂନ୍ୟ setPath(ଅଚଳ Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ପ୍ରତିଜ୍ଞା(FD.isValid() && "cannot stat closed file");
  ଯଦି (!S.isStatusKnown()) {
    file_status RealStatus;
    ଯଦି (std::error_code EC = sys::fs::status(FD, RealStatus))
      ଫେରନ୍ତୁ EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  ଫେରନ୍ତୁ S;
}

ErrorOr<std::string> RealFile::getName() {
  ଫେରନ୍ତୁ RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(ଅଚଳ Twine &Name, int64_t FileSize,
                    ତାର୍କିକ RequiresNullTerminator, ତାର୍କିକ IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ପ୍ରତିଜ୍ଞା(FD.isValid() && "cannot get buffer for closed file");
  ଫେରନ୍ତୁ MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ଫେରନ୍ତୁ EC;
}

ଶୂନ୍ୟ RealFile::setPath(ଅଚଳ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  ଯଦି (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
}

ନାମାକାଶ {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
ଶ୍ରେଣୀ RealFileSystem : ସର୍ବସାଧାରଣ FileSystem {
ସର୍ବସାଧାରଣ:
  explicit RealFileSystem(ତାର୍କିକ LinkCWDToProcess) {
    ଯଦି (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      ଯଦି (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      ଅନ୍ୟଥା ଯଦି (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      ଅନ୍ୟଥା
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(ଅଚଳ Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ଅଚଳ Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ଦ୍ୱିମିକ(ଅଚଳ Twine &Path) override;
  directory_iterator dir_begin(ଅଚଳ Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ଅଚଳ override;
  std::error_code setCurrentWorkingDirectory(ଅଚଳ Twine &Path) override;
  std::error_code isLocal(ଅଚଳ Twine &Path, ତାର୍କିକ &Result) override;
  ଶୂନ୍ୟ
  getDirectoryContentRealSources(ଅଚଳ Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.ଠେଲ_back(Dir.str());
  }
  std::error_code getRealPath(ଅଚଳ Twine &Path,
                              SmallVectorImpl<ଅକ୍ଷର> &Output) override;

ସୁରକ୍ଷିତ:
  ଶୂନ୍ୟ printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) ଅଚଳ override;

ବ୍ୟକ୍ତିଗତ:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(ଅଚଳ Twine &Path, SmallVectorImpl<ଅକ୍ଷର> &Storage) ଅଚଳ {
    ଯଦି (!WD || !*WD)
      ଫେରନ୍ତୁ Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    ଫେରନ୍ତୁ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ଅଚଳ Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    ଯଦି (!FDOrErr)
      ଫେରନ୍ତୁ errorToErrorCode(FDOrErr.takeError());
    ଫେରନ୍ତୁ std::unique_ptr<File>(
        ନୂତନ RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  ସଂରଚନା WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(ଅଚଳ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  ଯଦି (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    ଫେରନ୍ତୁ EC;
  ଫେରନ୍ତୁ Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ଅଚଳ Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ଫେରନ୍ତୁ openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ଦ୍ୱିମିକ(ଅଚଳ Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ଫେରନ୍ତୁ openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ଅଚଳ {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ଯଦି (WD && *WD)
    ଫେରନ୍ତୁ std::string(WD->get().Specified);
  ଯଦି (WD)
    ଫେରନ୍ତୁ WD->getError();

  SmallString<128> Dir;
  ଯଦି (std::error_code EC = llvm::sys::fs::current_path(Dir))
    ଫେରନ୍ତୁ EC;
  ଫେରନ୍ତୁ std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(ଅଚଳ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ଯଦି (!WD)
    ଫେରନ୍ତୁ llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  ତାର୍କିକ IsDir;
  ଯଦି (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ଫେରନ୍ତୁ Err;
  ଯଦି (!IsDir)
    ଫେରନ୍ତୁ std::make_error_code(std::errc::not_a_directory);
  ଯଦି (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    ଫେରନ୍ତୁ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ଫେରନ୍ତୁ std::error_code();
}

std::error_code RealFileSystem::isLocal(ଅଚଳ Twine &Path, ତାର୍କିକ &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ଫେରନ୍ତୁ llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(ଅଚଳ Twine &Path,
                                            SmallVectorImpl<ଅକ୍ଷର> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ଫେରନ୍ତୁ llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

ଶୂନ୍ୟ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) ଅଚଳ {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  ଯଦି (WD)
    OS << "own";
  ଅନ୍ୟଥା
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  ସ୍ଥିର IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(ସତ୍ୟ);
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ଫେରନ୍ତୁ std::make_unique<RealFileSystem>(ମିଥ୍ୟା);
}

ନାମାକାଶ {

ଶ୍ରେଣୀ RealFSDirIter : ସର୍ବସାଧାରଣ llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

ସର୍ବସାଧାରଣ:
  RealFSDirIter(ଅଚଳ Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    ଯଦି (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    ଫେରନ୍ତୁ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ଅଚଳ Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ଫେରନ୍ତୁ directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.ଠେଲ_back(std::move(BaseFS));
}

ଶୂନ୍ୟ OverlayFileSystem::ଠେଲ_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ଠେଲ_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(ଅଚଳ Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ପାଇଁ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    ଯଦି (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      ଫେରନ୍ତୁ Status;
  }
  ଫେରନ୍ତୁ make_error_code(llvm::errc::no_such_file_or_directory);
}

ତାର୍କିକ OverlayFileSystem::exists(ଅଚଳ Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ପାଇଁ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ଯଦି ((*I)->exists(Path))
      ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ଅଚଳ llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ପାଇଁ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    ଯଦି (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ଫେରନ୍ତୁ Result;
  }
  ଫେରନ୍ତୁ make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ଅଚଳ {
  // All file systems are synchronized, just take the first working directory.
  ଫେରନ୍ତୁ FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(ଅଚଳ Twine &Path) {
  ପାଇଁ (auto &FS : FSList)
    ଯଦି (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      ଫେରନ୍ତୁ EC;
  ଫେରନ୍ତୁ {};
}

std::error_code OverlayFileSystem::isLocal(ଅଚଳ Twine &Path, ତାର୍କିକ &Result) {
  ପାଇଁ (auto &FS : FSList)
    ଯଦି (FS->exists(Path))
      ଫେରନ୍ତୁ FS->isLocal(Path, Result);
  ଫେରନ୍ତୁ errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(ଅଚଳ Twine &Path,
                                               SmallVectorImpl<ଅକ୍ଷର> &Output) {
  ପାଇଁ (ଅଚଳ auto &FS : FSList)
    ଯଦି (FS->exists(Path))
      ଫେରନ୍ତୁ FS->getRealPath(Path, Output);
  ଫେରନ୍ତୁ errc::no_such_file_or_directory;
}

ଶୂନ୍ୟ OverlayFileSystem::getDirectoryContentRealSources(
    ଅଚଳ Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  ପାଇଁ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

ଶୂନ୍ୟ OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  ପାଇଁ (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

ଶୂନ୍ୟ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) ଅଚଳ {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  ଯଦି (Type == PrintType::Summary)
    ଫେରନ୍ତୁ;

  ଯଦି (Type == PrintType::Contents)
    Type = PrintType::Summary;
  ପାଇଁ (ଅଚଳ auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ନାମାକାଶ {

/// Combines and deduplicates directory entries across multiple file systems.
ଶ୍ରେଣୀ CombiningDirIterImpl : ସର୍ବସାଧାରଣ llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(ତାର୍କିକ IsFirstTime) {
    ଯେତେବେଳେ (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.କାଢ଼_back();
      ଯଦି (CurrentDirIter != directory_iterator())
        ଭାଙ୍ଗ; // found
    }

    ଯଦି (IsFirstTime && CurrentDirIter == directory_iterator())
      ଫେରନ୍ତୁ errc::no_such_file_or_directory;
    ଫେରନ୍ତୁ {};
  }

  std::error_code incrementDirIter(ତାର୍କିକ IsFirstTime) {
    ପ୍ରତିଜ୍ଞା((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    ଯଦି (!IsFirstTime)
      CurrentDirIter.increment(EC);
    ଯଦି (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    ଫେରନ୍ତୁ EC;
  }

  std::error_code incrementImpl(ତାର୍କିକ IsFirstTime) {
    ଯେତେବେଳେ (ସତ୍ୟ) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      ଯଦି (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        ଫେରନ୍ତୁ EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      ଯଦି (SeenNames.ଭର୍ତ୍ତି(Name).second)
        ଫେରନ୍ତୁ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

ସର୍ବସାଧାରଣ:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    ପାଇଁ (ଅଚଳ auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      ଯଦି (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ଫେରନ୍ତୁ;
      }
      ଯଦି (!FEC)
        IterList.ଠେଲ_back(Iter);
    }
    EC = incrementImpl(ସତ୍ୟ);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(ସତ୍ୟ);
  }

  std::error_code increment() override { ଫେରନ୍ତୁ incrementImpl(ମିଥ୍ୟା); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ଅଚଳ Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  ଯଦି (EC)
    ଫେରନ୍ତୁ {};
  ଫେରନ୍ତୁ Combined;
}

ଶୂନ୍ୟ ProxyFileSystem::anchor() {}

ନାମାକାଶ llvm {
ନାମାକାଶ vfs {

ନାମାକାଶ detail {

ଗଣନା In_Memory_ଗଣ୍ଠି_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
ଶ୍ରେଣୀ In_Memory_ଗଣ୍ଠି {
  In_Memory_ଗଣ୍ଠି_Kind Kind;
  std::string FileName;

ସର୍ବସାଧାରଣ:
  In_Memory_ଗଣ୍ଠି(llvm::StringRef FileName, In_Memory_ଗଣ୍ଠି_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  ଆଭାସୀ ~In_Memory_ଗଣ୍ଠି() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  ଆଭାସୀ Status getStatus(ଅଚଳ Twine &RequestedName) ଅଚଳ = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ଅଚଳ { ଫେରନ୍ତୁ FileName; }
  In_Memory_ଗଣ୍ଠି_Kind getKind() ଅଚଳ { ଫେରନ୍ତୁ Kind; }
  ଆଭାସୀ std::string toString(unsigned Indent) ଅଚଳ = 0;
};

ଶ୍ରେଣୀ InMemoryFile : ସର୍ବସାଧାରଣ In_Memory_ଗଣ୍ଠି {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

ସର୍ବସାଧାରଣ:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_ଗଣ୍ଠି(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(ଅଚଳ Twine &RequestedName) ଅଚଳ override {
    ଫେରନ୍ତୁ Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() ଅଚଳ { ଫେରନ୍ତୁ Buffer.get(); }

  std::string toString(unsigned Indent) ଅଚଳ override {
    ଫେରନ୍ତୁ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ In_Memory_ଗଣ୍ଠି *N) {
    ଫେରନ୍ତୁ N->getKind() == IME_File;
  }
};

ନାମାକାଶ {

ଶ୍ରେଣୀ InMemoryHardLink : ସର୍ବସାଧାରଣ In_Memory_ଗଣ୍ଠି {
  ଅଚଳ InMemoryFile &ResolvedFile;

ସର୍ବସାଧାରଣ:
  InMemoryHardLink(StringRef Path, ଅଚଳ InMemoryFile &ResolvedFile)
      : In_Memory_ଗଣ୍ଠି(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ଅଚଳ InMemoryFile &getResolvedFile() ଅଚଳ { ଫେରନ୍ତୁ ResolvedFile; }

  Status getStatus(ଅଚଳ Twine &RequestedName) ଅଚଳ override {
    ଫେରନ୍ତୁ ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) ଅଚଳ override {
    ଫେରନ୍ତୁ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ In_Memory_ଗଣ୍ଠି *N) {
    ଫେରନ୍ତୁ N->getKind() == IME_HardLink;
  }
};

ଶ୍ରେଣୀ InMemorySymbolicLink : ସର୍ବସାଧାରଣ In_Memory_ଗଣ୍ଠି {
  std::string TargetPath;
  Status Stat;

ସର୍ବସାଧାରଣ:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_ଗଣ୍ଠି(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) ଅଚଳ override {
    ଫେରନ୍ତୁ std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(ଅଚଳ Twine &RequestedName) ଅଚଳ override {
    ଫେରନ୍ତୁ Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() ଅଚଳ { ଫେରନ୍ତୁ TargetPath; }

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ In_Memory_ଗଣ୍ଠି *N) {
    ଫେରନ୍ତୁ N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
ଶ୍ରେଣୀ InMemoryFileAdaptor : ସର୍ବସାଧାରଣ File {
  ଅଚଳ InMemoryFile &ଗଣ୍ଠି;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

ସର୍ବସାଧାରଣ:
  explicit InMemoryFileAdaptor(ଅଚଳ InMemoryFile &ଗଣ୍ଠି,
                               std::string RequestedName)
      : ଗଣ୍ଠି(ଗଣ୍ଠି), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    ଫେରନ୍ତୁ ଗଣ୍ଠି.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(ଅଚଳ Twine &Name, int64_t FileSize, ତାର୍କିକ RequiresNullTerminator,
            ତାର୍କିକ IsVolatile) override {
    llvm::MemoryBuffer *Buf = ଗଣ୍ଠି.getBuffer();
    ଫେରନ୍ତୁ llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { ଫେରନ୍ତୁ {}; }

  ଶୂନ୍ୟ setPath(ଅଚଳ Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

ଶ୍ରେଣୀ InMemoryDirectory : ସର୍ବସାଧାରଣ In_Memory_ଗଣ୍ଠି {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_ଗଣ୍ଠି>, std::less<>> Entries;

ସର୍ବସାଧାରଣ:
  InMemoryDirectory(Status Stat)
      : In_Memory_ଗଣ୍ଠି(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(ଅଚଳ Twine &RequestedName) ଅଚଳ override {
    ଫେରନ୍ତୁ Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ଅଚଳ { ଫେରନ୍ତୁ Stat.getUniqueID(); }

  In_Memory_ଗଣ୍ଠି *getChild(StringRef Name) ଅଚଳ {
    auto I = Entries.find(Name);
    ଯଦି (I != Entries.end())
      ଫେରନ୍ତୁ I->second.get();
    ଫେରନ୍ତୁ nullptr;
  }

  In_Memory_ଗଣ୍ଠି *addChild(StringRef Name, std::unique_ptr<In_Memory_ଗଣ୍ଠି> Child) {
    ଫେରନ୍ତୁ Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ଅଚଳ { ଫେରନ୍ତୁ Entries.begin(); }
  const_iterator end() ଅଚଳ { ଫେରନ୍ତୁ Entries.end(); }

  std::string toString(unsigned Indent) ଅଚଳ override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    ପାଇଁ (ଅଚଳ auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    ଫେରନ୍ତୁ Result;
  }

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ In_Memory_ଗଣ୍ଠି *N) {
    ଫେରନ୍ତୁ N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ସ୍ଥିର sys::fs::UniqueID getUniqueID(hash_code Hash) {
  ଫେରନ୍ତୁ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ସ୍ଥିର sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ଫେରନ୍ତୁ getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
ସ୍ଥିର sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  ଫେରନ୍ତୁ getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_ଗଣ୍ଠି_Info::makeStatus() ଅଚଳ {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  ଫେରନ୍ତୁ Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(ତାର୍କିକ UseNormalizedPaths)
    : ମୂଳ_2(ନୂତନ detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

std::string InMemoryFileSystem::toString() ଅଚଳ {
  ଫେରନ୍ତୁ ମୂଳ_2->toString(/*Indent=*/0);
}

ତାର୍କିକ InMemoryFileSystem::addFile(ଅଚଳ Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_ଗଣ୍ଠି_Fn Make_ଗଣ୍ଠି) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ପ୍ରତିଜ୍ଞା(!EC);
  (ଶୂନ୍ୟ)EC;

  ଯଦି (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ସତ୍ୟ);

  ଯଦି (Path.empty())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  detail::InMemoryDirectory *Dir = ମୂଳ_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  ଅଚଳ auto ResolvedUser = User.ମୂଲ୍ୟ_or(0);
  ଅଚଳ auto ResolvedGroup = Group.ମୂଲ୍ୟ_or(0);
  ଅଚଳ auto ResolvedType = Type.ମୂଲ୍ୟ_or(sys::fs::file_type::regular_file);
  ଅଚଳ auto ResolvedPerms = Perms.ମୂଲ୍ୟ_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ଅଚଳ auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  ଯେତେବେଳେ (ସତ୍ୟ) {
    Name = *I;
    ++I;
    ଯଦି (I == E)
      ଭାଙ୍ଗ;
    detail::In_Memory_ଗଣ୍ଠି *ଗଣ୍ଠି = Dir->getChild(Name);
    ଯଦି (!ଗଣ୍ଠି) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      ଜାରି;
    }
    // Creating file under another file.
    ଯଦି (!isa<detail::InMemoryDirectory>(ଗଣ୍ଠି))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    Dir = cast<detail::InMemoryDirectory>(ଗଣ୍ଠି);
  }
  detail::In_Memory_ଗଣ୍ଠି *ଗଣ୍ଠି = Dir->getChild(Name);
  ଯଦି (!ଗଣ୍ଠି) {
    Dir->addChild(Name,
                  Make_ଗଣ୍ଠି({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଯଦି (isa<detail::InMemoryDirectory>(ଗଣ୍ଠି))
    ଫେରନ୍ତୁ ResolvedType == sys::fs::file_type::directory_file;

  ପ୍ରତିଜ୍ଞା((isa<detail::InMemoryFile>(ଗଣ୍ଠି) ||
          isa<detail::InMemoryHardLink>(ଗଣ୍ଠି)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  ଯଦି (auto *Link = dyn_cast<detail::InMemoryHardLink>(ଗଣ୍ଠି)) {
    ଫେରନ୍ତୁ Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  ଫେରନ୍ତୁ cast<detail::InMemoryFile>(ଗଣ୍ଠି)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

ତାର୍କିକ InMemoryFileSystem::addFile(ଅଚଳ Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ଫେରନ୍ତୁ addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_ଗଣ୍ଠି_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_ଗଣ୍ଠି> {
                   Status Stat = NNI.makeStatus();
                   ଯଦି (Stat.getType() == sys::fs::file_type::directory_file)
                     ଫେରନ୍ତୁ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ଫେରନ୍ତୁ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

ତାର୍କିକ InMemoryFileSystem::addFileNoOwn(
    ଅଚଳ Twine &P, time_t ModificationTime,
    ଅଚଳ llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ଫେରନ୍ତୁ addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_ଗଣ୍ଠି_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_ଗଣ୍ଠି> {
                   Status Stat = NNI.makeStatus();
                   ଯଦି (Stat.getType() == sys::fs::file_type::directory_file)
                     ଫେରନ୍ତୁ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ଫେରନ୍ତୁ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_ଗଣ୍ଠି_Or_Error
InMemoryFileSystem::lookup_ଗଣ୍ଠି(ଅଚଳ Twine &P, ତାର୍କିକ FollowFinalSymlink,
                               size_t SymlinkDepth) ଅଚଳ {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ପ୍ରତିଜ୍ଞା(!EC);
  (ଶୂନ୍ୟ)EC;

  ଯଦି (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ସତ୍ୟ);

  ଅଚଳ detail::InMemoryDirectory *Dir = ମୂଳ_2.get();
  ଯଦି (Path.empty())
    ଫେରନ୍ତୁ detail::Named_ଗଣ୍ଠି_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  ଯେତେବେଳେ (ସତ୍ୟ) {
    detail::In_Memory_ଗଣ୍ଠି *ଗଣ୍ଠି = Dir->getChild(*I);
    ++I;
    ଯଦି (!ଗଣ୍ଠି)
      ଫେରନ୍ତୁ errc::no_such_file_or_directory;

    ଯଦି (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(ଗଣ୍ଠି)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      ଯଦି (I == E && !FollowFinalSymlink)
        ଫେରନ୍ତୁ detail::Named_ଗଣ୍ଠି_Or_Error(Path, Symlink);

      ଯଦି (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        ଫେରନ୍ତୁ errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      ଯଦି (std::error_code EC = makeAbsolute(TargetPath))
        ଫେରନ୍ତୁ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_ଗଣ୍ଠି(TargetPath, /*FollowFinalSymlink=*/ସତ୍ୟ, SymlinkDepth + 1);
      ଯଦି (!Target || I == E)
        ଫେରନ୍ତୁ Target;

      ଯଦି (!isa<detail::InMemoryDirectory>(*Target))
        ଫେରନ୍ତୁ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      ଜାରି;
    }

    // Return the file if it's at the end of the path.
    ଯଦି (auto File = dyn_cast<detail::InMemoryFile>(ଗଣ୍ଠି)) {
      ଯଦି (I == E)
        ଫେରନ୍ତୁ detail::Named_ଗଣ୍ଠି_Or_Error(Path, File);
      ଫେରନ୍ତୁ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    ଯଦି (auto File = dyn_cast<detail::InMemoryHardLink>(ଗଣ୍ଠି)) {
      ଯଦି (I == E)
        ଫେରନ୍ତୁ detail::Named_ଗଣ୍ଠି_Or_Error(Path, &File->getResolvedFile());
      ଫେରନ୍ତୁ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(ଗଣ୍ଠି);
    ଯଦି (I == E)
      ଫେରନ୍ତୁ detail::Named_ଗଣ୍ଠି_Or_Error(Path, Dir);
  }
}

ତାର୍କିକ InMemoryFileSystem::addHardLink(ଅଚଳ Twine &NewLink,
                                     ଅଚଳ Twine &Target) {
  auto New_Link_ଗଣ୍ଠି = lookup_ଗଣ୍ଠି(NewLink, /*FollowFinalSymlink=*/ମିଥ୍ୟା);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_ଗଣ୍ଠି = lookup_ଗଣ୍ଠି(Target, /*FollowFinalSymlink=*/ସତ୍ୟ);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  ଯଦି (!Target_ଗଣ୍ଠି || New_Link_ଗଣ୍ଠି || !isa<detail::InMemoryFile>(*Target_ଗଣ୍ଠି))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_ଗଣ୍ଠି_Info NNI) {
                   ଫେରନ୍ତୁ std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_ଗଣ୍ଠି));
                 });
}

ତାର୍କିକ InMemoryFileSystem::addSymbolicLink(
    ଅଚଳ Twine &NewLink, ଅଚଳ Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_ଗଣ୍ଠି = lookup_ଗଣ୍ଠି(NewLink, /*FollowFinalSymlink=*/ମିଥ୍ୟା);
  ଯଦି (New_Link_ଗଣ୍ଠି)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  ଫେରନ୍ତୁ addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_ଗଣ୍ଠି_Info NNI) {
                   ଫେରନ୍ତୁ std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(ଅଚଳ Twine &Path) {
  auto ଗଣ୍ଠି = lookup_ଗଣ୍ଠି(Path, /*FollowFinalSymlink=*/ସତ୍ୟ);
  ଯଦି (ଗଣ୍ଠି)
    ଫେରନ୍ତୁ (*ଗଣ୍ଠି)->getStatus(Path);
  ଫେରନ୍ତୁ ଗଣ୍ଠି.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(ଅଚଳ Twine &Path) {
  auto ଗଣ୍ଠି = lookup_ଗଣ୍ଠି(Path,/*FollowFinalSymlink=*/ସତ୍ୟ);
  ଯଦି (!ଗଣ୍ଠି)
    ଫେରନ୍ତୁ ଗଣ୍ଠି.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  ଯଦି (auto *F = dyn_cast<detail::InMemoryFile>(*ଗଣ୍ଠି))
    ଫେରନ୍ତୁ std::unique_ptr<File>(
        ନୂତନ detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  ଫେରନ୍ତୁ make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
ଶ୍ରେଣୀ InMemoryFileSystem::DirIterator : ସର୍ବସାଧାରଣ llvm::vfs::detail::DirIterImpl {
  ଅଚଳ InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  ଶୂନ୍ୟ setCurrentEntry() {
    ଯଦି (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ବାଛ (I->second->getKind()) {
      ସ୍ଥିତି detail::IME_File:
      ସ୍ଥିତି detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି detail::IME_SymbolicLink:
        ଯଦି (auto SymlinkTarget =
                FS->lookup_ଗଣ୍ଠି(Path, /*FollowFinalSymlink=*/ସତ୍ୟ)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        ଭାଙ୍ଗ;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } ଅନ୍ୟଥା {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

ସର୍ବସାଧାରଣ:
  DirIterator() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

  DirIterator(ଅଚଳ InMemoryFileSystem *FS,
              ଅଚଳ detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    ଫେରନ୍ତୁ {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(ଅଚଳ Twine &Dir,
                                                 std::error_code &EC) {
  auto ଗଣ୍ଠି = lookup_ଗଣ୍ଠି(Dir, /*FollowFinalSymlink=*/ସତ୍ୟ);
  ଯଦି (!ଗଣ୍ଠି) {
    EC = ଗଣ୍ଠି.getError();
    ଫେରନ୍ତୁ directory_iterator(std::make_shared<DirIterator>());
  }

  ଯଦି (auto *Dir_ଗଣ୍ଠି = dyn_cast<detail::InMemoryDirectory>(*ଗଣ୍ଠି))
    ଫେରନ୍ତୁ directory_iterator(
        std::make_shared<DirIterator>(ଏହି, *Dir_ଗଣ୍ଠି, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  ଫେରନ୍ତୁ directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(ଅଚଳ Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ପ୍ରତିଜ୍ଞା(!EC);
  (ଶୂନ୍ୟ)EC;

  ଯଦି (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ସତ୍ୟ);

  ଯଦି (!Path.empty())
    WorkingDirectory = std::string(Path);
  ଫେରନ୍ତୁ {};
}

std::error_code InMemoryFileSystem::getRealPath(ଅଚଳ Twine &Path,
                                                SmallVectorImpl<ଅକ୍ଷର> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  ଯଦି (!CWD || CWD->empty())
    ଫେରନ୍ତୁ errc::operation_not_permitted;
  Path.toVector(Output);
  ଯଦି (auto EC = makeAbsolute(Output))
    ଫେରନ୍ତୁ EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/ସତ୍ୟ);
  ଫେରନ୍ତୁ {};
}

std::error_code InMemoryFileSystem::isLocal(ଅଚଳ Twine &Path, ତାର୍କିକ &Result) {
  Result = ମିଥ୍ୟା;
  ଫେରନ୍ତୁ {};
}

ଶୂନ୍ୟ InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) ଅଚଳ {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

ନାମାକାଶ {

ସ୍ଥିର llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  ଅଚଳ size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  ଯଦି (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  ଫେରନ୍ତୁ style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ସ୍ଥିର llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/ସତ୍ୟ, style);
  ଫେରନ୍ତୁ result;
}

/// Whether the error and entry specify a file/directory that was not found.
ସ୍ଥିର ତାର୍କିକ isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  ଯଦି (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  ପ୍ରତିଜ୍ଞା(ExternalFS && "RedirectingFileSystem requires an external FS");
  ଯଦି (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
ଶ୍ରେଣୀ llvm::vfs::RedirectingFSDirIterImpl
    : ସର୍ବସାଧାରଣ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(ତାର୍କିକ IsFirstTime) {
    ପ୍ରତିଜ୍ଞା((IsFirstTime || Current != End) && "cannot iterate past end");
    ଯଦି (!IsFirstTime)
      ++Current;
    ଯଦି (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ବାଛ ((*Current)->getKind()) {
      ସ୍ଥିତି RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      ସ୍ଥିତି RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        ଭାଙ୍ଗ;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } ଅନ୍ୟଥା {
      CurrentEntry = directory_entry();
    }
    ଫେରନ୍ତୁ {};
  };

ସର୍ବସାଧାରଣ:
  RedirectingFSDirIterImpl(
      ଅଚଳ Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/ସତ୍ୟ);
  }

  std::error_code increment() override {
    ଫେରନ୍ତୁ incrementImpl(/*IsFirstTime=*/ମିଥ୍ୟା);
  }
};

ନାମାକାଶ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
ଶ୍ରେଣୀ RedirectingFSDirRemapIterImpl : ସର୍ବସାଧାରଣ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

ସର୍ବସାଧାରଣ:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    ଯଦି (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  ଶୂନ୍ୟ setCurrentEntry() {
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
    ଯଦି (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    ଅନ୍ୟଥା
      CurrentEntry = directory_entry();
    ଫେରନ୍ତୁ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ଅଚଳ {
  ଫେରନ୍ତୁ WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(ଅଚଳ Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  ଯଦି (!exists(Path))
    ଫେରନ୍ତୁ errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  ଯଦି (std::error_code EC = makeAbsolute(AbsolutePath))
    ଫେରନ୍ତୁ EC;
  WorkingDirectory = std::string(AbsolutePath);
  ଫେରନ୍ତୁ {};
}

std::error_code RedirectingFileSystem::isLocal(ଅଚଳ Twine &Path_,
                                               ତାର୍କିକ &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  ଯଦି (makeAbsolute(Path))
    ଫେରନ୍ତୁ {};

  ଫେରନ୍ତୁ ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<ଅକ୍ଷର> &Path) ଅଚଳ {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  ଯଦି (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ଫେରନ୍ତୁ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ଯଦି (!WorkingDir)
    ଫେରନ୍ତୁ WorkingDir.getError();

  ଫେରନ୍ତୁ makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<ଅକ୍ଷର> &Path) ଅଚଳ {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  ଯଦି (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    ଫେରନ୍ତୁ std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  ଯଦି (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } ଅନ୍ୟଥା {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    ଯଦି (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  ଯଦି (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  ଫେରନ୍ତୁ {};
}

directory_iterator RedirectingFileSystem::dir_begin(ଅଚଳ Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  ଯଦି (EC)
    ଫେରନ୍ତୁ {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ଯଦି (!Result) {
    ଯଦି (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      ଫେରନ୍ତୁ ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    ଫେରନ୍ତୁ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  ଯଦି (!S) {
    ଯଦି (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      ଫେରନ୍ତୁ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ଫେରନ୍ତୁ {};
  }

  ଯଦି (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ଫେରନ୍ତୁ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  ଯଦି (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    ଯଦି (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } ଅନ୍ୟଥା {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  ଯଦି (RedirectEC) {
    ଯଦି (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ଫେରନ୍ତୁ {};
    }
    RedirectIter = {};
  }

  ଯଦି (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    ଫେରନ୍ତୁ RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  ଯଦି (ExternalEC) {
    ଯଦି (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ଫେରନ୍ତୁ {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  ବାଛ (Redirection) {
  ସ୍ଥିତି RedirectKind::Fallthrough:
    Iters.ଠେଲ_back(ExternalIter);
    Iters.ଠେଲ_back(RedirectIter);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି RedirectKind::Fallback:
    Iters.ଠେଲ_back(RedirectIter);
    Iters.ଠେଲ_back(ExternalIter);
    ଭାଙ୍ଗ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  ଯଦି (EC)
    ଫେରନ୍ତୁ {};
  ଫେରନ୍ତୁ Combined;
}

ଶୂନ୍ୟ RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ଅଚଳ {
  ଫେରନ୍ତୁ OverlayFileDir;
}

ଶୂନ୍ୟ RedirectingFileSystem::setFallthrough(ତାର୍କିକ Fallthrough) {
  ଯଦି (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } ଅନ୍ୟଥା {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

ଶୂନ୍ୟ RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() ଅଚଳ {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  ପାଇଁ (ଅଚଳ auto &ମୂଳ_2 : Roots)
    R.ଠେଲ_back(ମୂଳ_2->getName());
  ଫେରନ୍ତୁ R;
}

ଶୂନ୍ୟ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) ଅଚଳ {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  ଯଦି (Type == PrintType::Summary)
    ଫେରନ୍ତୁ;

  ପାଇଁ (ଅଚଳ auto &ମୂଳ_2 : Roots)
    printEntry(OS, ମୂଳ_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

ଶୂନ୍ୟ RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) ଅଚଳ {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  ବାଛ (E->getKind()) {
  ସ୍ଥିତି EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    ପାଇଁ (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    ଭାଙ୍ଗ;
  }
  ସ୍ଥିତି EK_DirectoryRemap:
  ସ୍ଥିତି EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    ବାଛ (RE->getUseName()) {
    ସ୍ଥିତି NK_NotSet:
      ଭାଙ୍ଗ;
    ସ୍ଥିତି NK_External:
      OS << " (UseExternalName: true)";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି NK_Virtual:
      OS << " (UseExternalName: false)";
      ଭାଙ୍ଗ;
    }
    OS << "\n";
    ଭାଙ୍ଗ;
  }
  }
}

ଶୂନ୍ୟ RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  ଯଦି (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
ଶ୍ରେଣୀ llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  ଶୂନ୍ୟ error(yaml::ଗଣ୍ଠି *N, ଅଚଳ Twine &ସନ୍ଦେଶ_2) { Stream.printError(N, ସନ୍ଦେଶ_2); }

  // false on error
  ତାର୍କିକ parseScalarString(yaml::ଗଣ୍ଠି *N, StringRef &Result,
                         SmallVectorImpl<ଅକ୍ଷର> &Storage) {
    ଅଚଳ auto *S = dyn_cast_if_present<yaml::Scalar_ଗଣ୍ଠି>(N);

    ଯଦି (!S) {
      error(N, "expected string");
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
    Result = S->get_ମୂଲ୍ୟ(Storage);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  // false on error
  ତାର୍କିକ parseScalarBool(yaml::ଗଣ୍ଠି *N, ତାର୍କିକ &Result) {
    SmallString<5> Storage;
    StringRef ମୂଲ୍ୟ;
    ଯଦି (!parseScalarString(N, ମୂଲ୍ୟ, Storage))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    ଯଦି (ମୂଲ୍ୟ.equals_insensitive("true") || ମୂଲ୍ୟ.equals_insensitive("on") ||
        ମୂଲ୍ୟ.equals_insensitive("yes") || ମୂଲ୍ୟ == "1") {
      Result = ସତ୍ୟ;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    } ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ.equals_insensitive("false") ||
               ମୂଲ୍ୟ.equals_insensitive("off") ||
               ମୂଲ୍ୟ.equals_insensitive("no") || ମୂଲ୍ୟ == "0") {
      Result = ମିଥ୍ୟା;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }

    error(N, "expected boolean value");
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::ଗଣ୍ଠି *N) {
    SmallString<12> Storage;
    StringRef ମୂଲ୍ୟ;
    ଯଦି (!parseScalarString(N, ମୂଲ୍ୟ, Storage))
      ଫେରନ୍ତୁ std::nullopt;

    ଯଦି (ମୂଲ୍ୟ.equals_insensitive("fallthrough")) {
      ଫେରନ୍ତୁ RedirectingFileSystem::RedirectKind::Fallthrough;
    } ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ.equals_insensitive("fallback")) {
      ଫେରନ୍ତୁ RedirectingFileSystem::RedirectKind::Fallback;
    } ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ.equals_insensitive("redirect-only")) {
      ଫେରନ୍ତୁ RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    ଫେରନ୍ତୁ std::nullopt;
  }

  std::optional<RedirectingFileSystem::ମୂଳ_Relative_Kind>
  parse_ମୂଳ_Relative_Kind(yaml::ଗଣ୍ଠି *N) {
    SmallString<12> Storage;
    StringRef ମୂଲ୍ୟ;
    ଯଦି (!parseScalarString(N, ମୂଲ୍ୟ, Storage))
      ଫେରନ୍ତୁ std::nullopt;
    ଯଦି (ମୂଲ୍ୟ.equals_insensitive("cwd")) {
      ଫେରନ୍ତୁ RedirectingFileSystem::ମୂଳ_Relative_Kind::CWD;
    } ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ.equals_insensitive("overlay-dir")) {
      ଫେରନ୍ତୁ RedirectingFileSystem::ମୂଳ_Relative_Kind::OverlayDir;
    }
    ଫେରନ୍ତୁ std::nullopt;
  }

  ସଂରଚନା ଚାବି_Status {
    ତାର୍କିକ Required;
    ତାର୍କିକ Seen = ମିଥ୍ୟା;

    ଚାବି_Status(ତାର୍କିକ Required = ମିଥ୍ୟା) : Required(Required) {}
  };

  using ଚାବି_Status_Pair = std::pair<StringRef, ଚାବି_Status>;

  // false on error
  ତାର୍କିକ check_Duplicate_Or_Unknown_ଚାବି(yaml::ଗଣ୍ଠି *ଚାବି_ଗଣ୍ଠି, StringRef ଚାବି,
                                  DenseMap<StringRef, ଚାବି_Status> &ଚାବିଗୁଡ଼ିକ_2) {
    auto It = ଚାବିଗୁଡ଼ିକ_2.find(ଚାବି);
    ଯଦି (It == ଚାବିଗୁଡ଼ିକ_2.end()) {
      error(ଚାବି_ଗଣ୍ଠି, "unknown key");
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
    ଚାବି_Status &S = It->second;
    ଯଦି (S.Seen) {
      error(ଚାବି_ଗଣ୍ଠି, Twine("duplicate key '") + ଚାବି + "'");
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
    S.Seen = ସତ୍ୟ;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  // false on error
  ତାର୍କିକ check_Missing_ଚାବିଗୁଡ଼ିକ(yaml::ଗଣ୍ଠି *Obj, DenseMap<StringRef, ଚାବି_Status> &ଚାବିଗୁଡ଼ିକ_2) {
    ପାଇଁ (ଅଚଳ auto &I : ଚାବିଗୁଡ଼ିକ_2) {
      ଯଦି (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      }
    }
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

ସର୍ବସାଧାରଣ:
  ସ୍ଥିର RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    ଯଦି (!ParentEntry) { // Look for a existent root
      ପାଇଁ (ଅଚଳ auto &ମୂଳ_2 : FS->Roots) {
        ଯଦି (Name == ମୂଳ_2->getName()) {
          ParentEntry = ମୂଳ_2.get();
          ଫେରନ୍ତୁ ParentEntry;
        }
      }
    } ଅନ୍ୟଥା { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      ପାଇଁ (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        ଯଦି (DirContent && Name == Content->getName())
          ଫେରନ୍ତୁ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    ଯଦି (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.ଠେଲ_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      ଫେରନ୍ତୁ ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    ଫେରନ୍ତୁ DE->getLastContent();
  }

ବ୍ୟକ୍ତିଗତ:
  ଶୂନ୍ୟ unique_Overlay_ବୃକ୍ଷ(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    ବାଛ (SrcE->getKind()) {
    ସ୍ଥିତି RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      ଯଦି (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      ପାଇଁ (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_ବୃକ୍ଷ(FS, SubEntry.get(), NewParentE);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି RedirectingFileSystem::EK_DirectoryRemap: {
      ପ୍ରତିଜ୍ଞା(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି RedirectingFileSystem::EK_File: {
      ପ୍ରତିଜ୍ଞା(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      ଭାଙ୍ଗ;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::ଗଣ୍ଠି *N, RedirectingFileSystem *FS, ତାର୍କିକ Is_ମୂଳ_Entry) {
    auto *M = dyn_cast<yaml::Mapping_ଗଣ୍ଠି>(N);
    ଯଦି (!M) {
      error(N, "expected mapping node for file or directory entry");
      ଫେରନ୍ତୁ nullptr;
    }

    ଚାବି_Status_Pair Fields[] = {
        ଚାବି_Status_Pair("name", ସତ୍ୟ),
        ଚାବି_Status_Pair("type", ସତ୍ୟ),
        ଚାବି_Status_Pair("contents", ମିଥ୍ୟା),
        ଚାବି_Status_Pair("external-contents", ମିଥ୍ୟା),
        ଚାବି_Status_Pair("use-external-name", ମିଥ୍ୟା),
    };

    DenseMap<StringRef, ଚାବି_Status> ଚାବିଗୁଡ଼ିକ_2(std::begin(Fields), std::end(Fields));

    ଗଣନା { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::ଗଣ୍ଠି *Name_ମୂଲ୍ୟ_ଗଣ୍ଠି = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    ପାଇଁ (auto &I : *M) {
      StringRef ଚାବି;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      ଯଦି (!parseScalarString(I.get_ଚାବି(), ଚାବି, Buffer))
        ଫେରନ୍ତୁ nullptr;

      ଯଦି (!check_Duplicate_Or_Unknown_ଚାବି(I.get_ଚାବି(), ଚାବି, ଚାବିଗୁଡ଼ିକ_2))
        ଫେରନ୍ତୁ nullptr;

      StringRef ମୂଲ୍ୟ;
      ଯଦି (ଚାବି == "name") {
        ଯଦି (!parseScalarString(I.get_ମୂଲ୍ୟ(), ମୂଲ୍ୟ, Buffer))
          ଫେରନ୍ତୁ nullptr;

        Name_ମୂଲ୍ୟ_ଗଣ୍ଠି = I.get_ମୂଲ୍ୟ();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(ମୂଲ୍ୟ).str();
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "type") {
        ଯଦି (!parseScalarString(I.get_ମୂଲ୍ୟ(), ମୂଲ୍ୟ, Buffer))
          ଫେରନ୍ତୁ nullptr;
        ଯଦି (ମୂଲ୍ୟ == "file")
          Kind = RedirectingFileSystem::EK_File;
        ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        ଅନ୍ୟଥା {
          error(I.get_ମୂଲ୍ୟ(), "unknown value for 'type'");
          ଫେରନ୍ତୁ nullptr;
        }
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "contents") {
        ଯଦି (ContentsField != CF_NotSet) {
          error(I.get_ଚାବି(),
                "entry already has 'contents' or 'external-contents'");
          ଫେରନ୍ତୁ nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_ଗଣ୍ଠି>(I.get_ମୂଲ୍ୟ());
        ଯଦି (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_ମୂଲ୍ୟ(), "expected array");
          ଫେରନ୍ତୁ nullptr;
        }

        ପାଇଁ (auto &I : *Contents) {
          ଯଦି (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ ମିଥ୍ୟା))
            EntryArrayContents.ଠେଲ_back(std::move(E));
          ଅନ୍ୟଥା
            ଫେରନ୍ତୁ nullptr;
        }
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "external-contents") {
        ଯଦି (ContentsField != CF_NotSet) {
          error(I.get_ଚାବି(),
                "entry already has 'contents' or 'external-contents'");
          ଫେରନ୍ତୁ nullptr;
        }
        ContentsField = CF_External;
        ଯଦି (!parseScalarString(I.get_ମୂଲ୍ୟ(), ମୂଲ୍ୟ, Buffer))
          ଫେରନ୍ତୁ nullptr;

        SmallString<256> FullPath;
        ଯଦି (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          ପ୍ରତିଜ୍ଞା(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = ମୂଲ୍ୟ;
          ଯଦି (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            ଫେରନ୍ତୁ nullptr;
          }
          FullPath = AbsFullPath;
        } ଅନ୍ୟଥା {
          FullPath = ମୂଲ୍ୟ;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "use-external-name") {
        ତାର୍କିକ Val;
        ଯଦି (!parseScalarBool(I.get_ମୂଲ୍ୟ(), Val))
          ଫେରନ୍ତୁ nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } ଅନ୍ୟଥା {
        llvm_unreachable("key missing from Keys");
      }
    }

    ଯଦି (Stream.failed())
      ଫେରନ୍ତୁ nullptr;

    // check for missing keys
    ଯଦି (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      ଫେରନ୍ତୁ nullptr;
    }
    ଯଦି (!check_Missing_ଚାବିଗୁଡ଼ିକ(N, ଚାବିଗୁଡ଼ିକ_2))
      ଫେରନ୍ତୁ nullptr;

    // check invalid configuration
    ଯଦି (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ଫେରନ୍ତୁ nullptr;
    }

    ଯଦି (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ଫେରନ୍ତୁ nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    ଯଦି (Is_ମୂଳ_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      ଯଦି (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } ଅନ୍ୟଥା ଯଦି (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } ଅନ୍ୟଥା {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        ଯଦି (FS->ମୂଳ_Relative ==
            RedirectingFileSystem::ମୂଳ_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          ପ୍ରତିଜ୍ଞା(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } ଅନ୍ୟଥା {
          EC = FS->makeAbsolute(Name);
        }
        ଯଦି (EC) {
          ପ୍ରତିଜ୍ଞା(Name_ମୂଲ୍ୟ_ଗଣ୍ଠି && "Name presence should be checked earlier");
          error(
              Name_ମୂଲ୍ୟ_ଗଣ୍ଠି,
              "entry with relative path at the root level is not discoverable");
          ଫେରନ୍ତୁ nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      ଯଦି (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t ମୂଳ_Path_Len = sys::path::ମୂଳ_path(Trimmed, path_style).size();
    ଯେତେବେଳେ (Trimmed.size() > ମୂଳ_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    ବାଛ (Kind) {
    ସ୍ଥିତି RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      ଭାଙ୍ଗ;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    ଯଦି (Parent.empty())
      ଫେରନ୍ତୁ Result;

    // if 'name' contains multiple components, create implicit directory entries
    ପାଇଁ (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.ଠେଲ_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ଫେରନ୍ତୁ Result;
  }

ସର୍ବସାଧାରଣ:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  ତାର୍କିକ parse(yaml::ଗଣ୍ଠି *ମୂଳ_2, RedirectingFileSystem *FS) {
    auto *ଶୀର୍ଷ = dyn_cast<yaml::Mapping_ଗଣ୍ଠି>(ମୂଳ_2);
    ଯଦି (!ଶୀର୍ଷ) {
      error(ମୂଳ_2, "expected mapping node");
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }

    ଚାବି_Status_Pair Fields[] = {
        ଚାବି_Status_Pair("version", ସତ୍ୟ),
        ଚାବି_Status_Pair("case-sensitive", ମିଥ୍ୟା),
        ଚାବି_Status_Pair("use-external-names", ମିଥ୍ୟା),
        ଚାବି_Status_Pair("root-relative", ମିଥ୍ୟା),
        ଚାବି_Status_Pair("overlay-relative", ମିଥ୍ୟା),
        ଚାବି_Status_Pair("fallthrough", ମିଥ୍ୟା),
        ଚାବି_Status_Pair("redirecting-with", ମିଥ୍ୟା),
        ଚାବି_Status_Pair("roots", ସତ୍ୟ),
    };

    DenseMap<StringRef, ଚାବି_Status> ଚାବିଗୁଡ଼ିକ_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> ମୂଳ_Entries;

    // Parse configuration and 'roots'
    ପାଇଁ (auto &I : *ଶୀର୍ଷ) {
      SmallString<10> ଚାବି_Buffer;
      StringRef ଚାବି;
      ଯଦି (!parseScalarString(I.get_ଚାବି(), ଚାବି, ଚାବି_Buffer))
        ଫେରନ୍ତୁ ମିଥ୍ୟା;

      ଯଦି (!check_Duplicate_Or_Unknown_ଚାବି(I.get_ଚାବି(), ଚାବି, ଚାବିଗୁଡ଼ିକ_2))
        ଫେରନ୍ତୁ ମିଥ୍ୟା;

      ଯଦି (ଚାବି == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_ଗଣ୍ଠି>(I.get_ମୂଲ୍ୟ());
        ଯଦି (!Roots) {
          error(I.get_ମୂଲ୍ୟ(), "expected array");
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }

        ପାଇଁ (auto &I : *Roots) {
          ଯଦି (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ ସତ୍ୟ))
            ମୂଳ_Entries.ଠେଲ_back(std::move(E));
          ଅନ୍ୟଥା
            ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        ଯଦି (!parseScalarString(I.get_ମୂଲ୍ୟ(), VersionString, Storage))
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା Version;
        ଯଦି (VersionString.getAsInteger<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(10, Version)) {
          error(I.get_ମୂଲ୍ୟ(), "expected integer");
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }
        ଯଦି (Version < 0) {
          error(I.get_ମୂଲ୍ୟ(), "invalid version number");
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }
        ଯଦି (Version != 0) {
          error(I.get_ମୂଲ୍ୟ(), "version mismatch, expected 0");
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "case-sensitive") {
        ଯଦି (!parseScalarBool(I.get_ମୂଲ୍ୟ(), FS->CaseSensitive))
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "overlay-relative") {
        ଯଦି (!parseScalarBool(I.get_ମୂଲ୍ୟ(), FS->IsRelativeOverlay))
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "use-external-names") {
        ଯଦି (!parseScalarBool(I.get_ମୂଲ୍ୟ(), FS->UseExternalNames))
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "fallthrough") {
        ଯଦି (ଚାବିଗୁଡ଼ିକ_2["redirecting-with"].Seen) {
          error(I.get_ମୂଲ୍ୟ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }

        ତାର୍କିକ ShouldFallthrough = ମିଥ୍ୟା;
        ଯଦି (!parseScalarBool(I.get_ମୂଲ୍ୟ(), ShouldFallthrough))
          ଫେରନ୍ତୁ ମିଥ୍ୟା;

        ଯଦି (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } ଅନ୍ୟଥା {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "redirecting-with") {
        ଯଦି (ଚାବିଗୁଡ଼ିକ_2["fallthrough"].Seen) {
          error(I.get_ମୂଲ୍ୟ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }

        ଯଦି (auto Kind = parseRedirectKind(I.get_ମୂଲ୍ୟ())) {
          FS->Redirection = *Kind;
        } ଅନ୍ୟଥା {
          error(I.get_ମୂଲ୍ୟ(), "expected valid redirect kind");
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }
      } ଅନ୍ୟଥା ଯଦି (ଚାବି == "root-relative") {
        ଯଦି (auto Kind = parse_ମୂଳ_Relative_Kind(I.get_ମୂଲ୍ୟ())) {
          FS->ମୂଳ_Relative = *Kind;
        } ଅନ୍ୟଥା {
          error(I.get_ମୂଲ୍ୟ(), "expected valid root-relative kind");
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }
      } ଅନ୍ୟଥା {
        llvm_unreachable("key missing from Keys");
      }
    }

    ଯଦି (Stream.failed())
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    ଯଦି (!check_Missing_ଚାବିଗୁଡ଼ିକ(ଶୀର୍ଷ, ଚାବିଗୁଡ଼ିକ_2))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    ପାଇଁ (auto &E : ମୂଳ_Entries)
      unique_Overlay_ବୃକ୍ଷ(FS, E.get());

    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, ଶୂନ୍ୟ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::ଗଣ୍ଠି *ମୂଳ_2 = DI->get_ମୂଳ();
  ଯଦି (DI == Stream.end() || !ମୂଳ_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    ଫେରନ୍ତୁ nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      ନୂତନ RedirectingFileSystem(ExternalFS));

  ଯଦି (!YAMLFilePath.empty()) {
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
    ପ୍ରତିଜ୍ଞା(!EC && "Overlay dir final path must be absolute");
    (ଶୂନ୍ୟ)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  ଯଦି (!P.parse(ମୂଳ_2, FS.get()))
    ଫେରନ୍ତୁ nullptr;

  ଫେରନ୍ତୁ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    ତାର୍କିକ UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      ନୂତନ RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  ପାଇଁ (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (ଶୂନ୍ୟ)EC;
      ପ୍ରତିଜ୍ଞା(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    ଯଦି (ToEntry)
      ଜାରି;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    ପାଇଁ (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    ପ୍ରତିଜ୍ଞା(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (ଶୂନ୍ୟ)EC;
      ପ୍ରତିଜ୍ଞା(!EC && "Could not make absolute path");
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

  ଫେରନ୍ତୁ FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  ପ୍ରତିଜ୍ଞା(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  ଯଦି (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

ଶୂନ୍ୟ RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<ଅକ୍ଷର> &Result) ଅଚଳ {
  Result.clear();
  ପାଇଁ (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<ଅକ୍ଷର> &Path) ଅଚଳ {
  ଯଦି (std::error_code EC = makeAbsolute(Path))
    ଫେରନ୍ତୁ EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  ଯଦି (CanonicalPath.empty())
    ଫେରନ୍ତୁ make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  ଫେରନ୍ତୁ {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) ଅଚଳ {
  llvm::SmallString<128> CanonicalPath(Path);
  ଯଦି (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    ଫେରନ୍ତୁ EC;

  // RedirectOnly means the VFS is always used.
  ଯଦି (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = ସତ୍ୟ;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  ପାଇଁ (ଅଚଳ auto &ମୂଳ_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, ମୂଳ_2.get(), Entries);
    ଯଦି (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = ସତ୍ୟ;
    ଯଦି (Result) {
      Result->Parents = std::move(Entries);
      ଫେରନ୍ତୁ Result;
    }

    ଯଦି (Result.getError() != llvm::errc::no_such_file_or_directory)
      ଫେରନ୍ତୁ Result;
  }
  ଫେରନ୍ତୁ make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  ଯଦି (!FromName.empty()) {
    ଯଦି (!pathComponentMatches(*Start, FromName))
      ଫେରନ୍ତୁ make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    ଯଦି (Start == End) {
      // Match!
      ଫେରନ୍ତୁ LookupResult(From, Start, End);
    }
  }

  ଯଦି (isa<RedirectingFileSystem::FileEntry>(From))
    ଫେରନ୍ତୁ make_error_code(llvm::errc::not_a_directory);

  ଯଦି (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    ଫେରନ୍ତୁ LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  ପାଇଁ (ଅଚଳ std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.ଠେଲ_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    ଯଦି (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ଫେରନ୍ତୁ Result;
    Entries.କାଢ଼_back();
  }

  ଫେରନ୍ତୁ make_error_code(llvm::errc::no_such_file_or_directory);
}

ସ୍ଥିର Status getRedirectedFileStatus(ଅଚଳ Twine &OriginalPath,
                                      ତାର୍କିକ UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  ଯଦି (ExternalStatus.ExposesExternalVFSPath)
    ଫେରନ୍ତୁ ExternalStatus;

  Status S = ExternalStatus;
  ଯଦି (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  ଅନ୍ୟଥା
    S.ExposesExternalVFSPath = ସତ୍ୟ;
  ଫେରନ୍ତୁ S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    ଅଚଳ Twine &LookupPath, ଅଚଳ Twine &OriginalPath,
    ଅଚଳ RedirectingFileSystem::LookupResult &Result) {
  ଯଦି (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    ଯଦି (std::error_code EC = makeAbsolute(RemappedPath))
      ଫେରନ୍ତୁ EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    ଯଦି (!S)
      ଫେରନ୍ତୁ S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    ଫେରନ୍ତୁ getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  ଫେରନ୍ତୁ Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(ଅଚଳ Twine &LookupPath,
                                         ଅଚଳ Twine &OriginalPath) ଅଚଳ {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  ଯଦି (!Result || Result->ExposesExternalVFSPath)
    ଫେରନ୍ତୁ Result;
  ଫେରନ୍ତୁ Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(ଅଚଳ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ଯଦି (std::error_code EC = makeAbsolute(Path))
    ଫେରନ୍ତୁ EC;

  ଯଦି (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    ଯଦି (S)
      ଫେରନ୍ତୁ S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ଯଦି (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ଯଦି (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ଫେରନ୍ତୁ getExternalStatus(Path, OriginalPath);
    ଫେରନ୍ତୁ Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  ଯଦି (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ଫେରନ୍ତୁ getExternalStatus(Path, OriginalPath);
  }

  ଫେରନ୍ତୁ S;
}

ତାର୍କିକ RedirectingFileSystem::exists(ଅଚଳ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ଯଦି (makeAbsolute(Path))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଯଦି (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ଯଦି (ExternalFS->exists(Path))
      ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ଯଦି (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ଯଦି (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ଫେରନ୍ତୁ ExternalFS->exists(Path);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  ଯଦି (!ExtRedirect) {
    ପ୍ରତିଜ୍ଞା(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  ଯଦି (makeAbsolute(RemappedPath))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଯଦି (ExternalFS->exists(RemappedPath))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ଫେରନ୍ତୁ ExternalFS->exists(Path);
  }

  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ନାମାକାଶ {

/// Provide a file wrapper with an overriden status.
ଶ୍ରେଣୀ FileWithFixedStatus : ସର୍ବସାଧାରଣ File {
  std::unique_ptr<File> InnerFile;
  Status S;

ସର୍ବସାଧାରଣ:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { ଫେରନ୍ତୁ S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(ଅଚଳ Twine &Name, int64_t FileSize, ତାର୍କିକ RequiresNullTerminator,
            ତାର୍କିକ IsVolatile) override {
    ଫେରନ୍ତୁ InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { ଫେରନ୍ତୁ InnerFile->close(); }

  ଶୂନ୍ୟ setPath(ଅଚଳ Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, ଅଚଳ Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  ଯଦି (!Result || (*Result)->status()->ExposesExternalVFSPath)
    ଫେରନ୍ତୁ Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  ଯଦି (Name && Name.get() != P.str())
    F->get()->setPath(P);
  ଫେରନ୍ତୁ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ଅଚଳ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ଯଦି (std::error_code EC = makeAbsolute(Path))
    ଫେରନ୍ତୁ EC;

  ଯଦି (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ଯଦି (F)
      ଫେରନ୍ତୁ F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ଯଦି (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ଯଦି (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ଫେରନ୍ତୁ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ଫେରନ୍ତୁ Result.getError();
  }

  ଯଦି (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    ଫେରନ୍ତୁ make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  ଯଦି (std::error_code EC = makeAbsolute(RemappedPath))
    ଫେରନ୍ତୁ EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  ଯଦି (!ExternalFile) {
    ଯଦି (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ଫେରନ୍ତୁ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    ଫେରନ୍ତୁ ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  ଯଦି (!ExternalStatus)
    ଫେରନ୍ତୁ ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  ଫେରନ୍ତୁ std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(ଅଚଳ Twine &OriginalPath,
                                   SmallVectorImpl<ଅକ୍ଷର> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ଯଦି (std::error_code EC = makeAbsolute(Path))
    ଫେରନ୍ତୁ EC;

  ଯଦି (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    ଯଦି (!EC)
      ଫେରନ୍ତୁ EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ଯଦି (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ଯଦି (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ଫେରନ୍ତୁ ExternalFS->getRealPath(Path, Output);
    ଫେରନ୍ତୁ Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  ଯଦି (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    ଯଦି (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ଫେରନ୍ତୁ ExternalFS->getRealPath(Path, Output);
    }
    ଫେରନ୍ତୁ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  ଯଦି (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    ଫେରନ୍ତୁ {};
  }
  ଫେରନ୍ତୁ llvm::errc::invalid_argument;
}

ଶୂନ୍ୟ RedirectingFileSystem::getDirectoryContentRealSources(
    ଅଚଳ Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  ଯଦି (makeAbsolute(Path))
    ଫେରନ୍ତୁ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ଅଚଳ ତାର୍କିକ ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ଯଦି (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    ଯଦି (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    ଫେରନ୍ତୁ;
  }

  ବାଛ (Result->E->getKind()) {
  ସ୍ଥିତି EK_File:
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    ଭାଙ୍ଗ;
  ସ୍ଥିତି EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    ଯଦି (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    ଭାଙ୍ଗ;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  ଯଦି (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, ଶୂନ୍ୟ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ଫେରନ୍ତୁ RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

ସ୍ଥିର ଶୂନ୍ୟ getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  ଯଦି (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    ପ୍ରତିଜ୍ଞା(DE && "Must be a directory");
    ପାଇଁ (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.ଠେଲ_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.କାଢ଼_back();
    }
    ଫେରନ୍ତୁ;
  }

  ଯଦି (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    ପ୍ରତିଜ୍ଞା(DR && "Must be a directory remap");
    SmallString<128> VPath;
    ପାଇଁ (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.ଠେଲ_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    ଫେରନ୍ତୁ;
  }

  ପ୍ରତିଜ୍ଞା(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  ପ୍ରତିଜ୍ଞା(FE && "Must be a file");
  SmallString<128> VPath;
  ପାଇଁ (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.ଠେଲ_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

ଶୂନ୍ୟ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> ମୂଳ_Result = VFS.lookupPath("/");
  ଯଦି (!ମୂଳ_Result)
    ଫେରନ୍ତୁ;
  SmallVector<StringRef, 8> Components;
  Components.ଠେଲ_back("/");
  getVFSEntries(ମୂଳ_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  ସ୍ଥିର std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ଫେରନ୍ତୁ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

ଶୂନ୍ୟ YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             ତାର୍କିକ IsDirectory) {
  ପ୍ରତିଜ୍ଞା(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  ପ୍ରତିଜ୍ଞା(sys::path::is_absolute(RealPath) && "real path not absolute");
  ପ୍ରତିଜ୍ଞା(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

ଶୂନ୍ୟ YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/ମିଥ୍ୟା);
}

ଶୂନ୍ୟ YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/ସତ୍ୟ);
}

ନାମାକାଶ {

ଶ୍ରେଣୀ JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_ଗଦା;

  unsigned getDirIndent() { ଫେରନ୍ତୁ 4 * Dir_ଗଦା.size(); }
  unsigned getFileIndent() { ଫେରନ୍ତୁ 4 * (Dir_ଗଦା.size() + 1); }
  ତାର୍କିକ containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  ଶୂନ୍ୟ startDirectory(StringRef Path);
  ଶୂନ୍ୟ endDirectory();
  ଶୂନ୍ୟ writeEntry(StringRef VPath, StringRef RPath);

ସର୍ବସାଧାରଣ:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  ଶୂନ୍ୟ ଲିଖ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<ତାର୍କିକ> UseExternalNames,
             std::optional<ତାର୍କିକ> IsCaseSensitive,
             std::optional<ତାର୍କିକ> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

ତାର୍କିକ JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using ନାମାକାଶ llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  ପାଇଁ (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    ଯଦି (*IParent != *IChild)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  // Have we exhausted the parent path?
  ଫେରନ୍ତୁ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  ପ୍ରତିଜ୍ଞା(!Parent.empty());
  ପ୍ରତିଜ୍ଞା(containedIn(Parent, Path));
  ଫେରନ୍ତୁ Path.substr(Parent.size() + 1);
}

ଶୂନ୍ୟ JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_ଗଦା.empty() ? Path : containedPart(Dir_ଗଦା.back(), Path);
  Dir_ଗଦା.ଠେଲ_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

ଶୂନ୍ୟ JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ଗଦା.କାଢ଼_back();
}

ଶୂନ୍ୟ JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

ଶୂନ୍ୟ JSONWriter::ଲିଖ(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<ତାର୍କିକ> UseExternalNames,
                       std::optional<ତାର୍କିକ> IsCaseSensitive,
                       std::optional<ତାର୍କିକ> IsOverlayRelative,
                       StringRef OverlayDir) {
  using ନାମାକାଶ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  ଯଦି (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  ଯଦି (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  ତାର୍କିକ UseOverlayRelative = ମିଥ୍ୟା;
  ଯଦି (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  ଯଦି (!Entries.empty()) {
    ଅଚଳ YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    ଯଦି (UseOverlayRelative) {
      ପ୍ରତିଜ୍ଞା(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    ତାର୍କିକ IsCurrentDirEmpty = ସତ୍ୟ;
    ଯଦି (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = ମିଥ୍ୟା;
    }

    ପାଇଁ (ଅଚଳ auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      ଯଦି (Dir == Dir_ଗଦା.back()) {
        ଯଦି (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } ଅନ୍ୟଥା {
        ତାର୍କିକ Is_Dir_Popped_From_ଗଦା = ମିଥ୍ୟା;
        ଯେତେବେଳେ (!Dir_ଗଦା.empty() && !containedIn(Dir_ଗଦା.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_ଗଦା = ସତ୍ୟ;
        }
        ଯଦି (Is_Dir_Popped_From_ଗଦା || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = ସତ୍ୟ;
      }
      StringRef RPath = Entry.RPath;
      ଯଦି (UseOverlayRelative) {
        ପ୍ରତିଜ୍ଞା(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      ଯଦି (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = ମିଥ୍ୟା;
      }
    }

    ଯେତେବେଳେ (!Dir_ଗଦା.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

ଶୂନ୍ୟ YAMLVFSWriter::ଲିଖ(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](ଅଚଳ YAMLVFSEntry &LHS, ଅଚଳ YAMLVFSEntry &RHS) {
    ଫେରନ୍ତୁ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).ଲିଖ(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ଅଚଳ Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  ଯଦି (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ଗଦା.ଠେଲ_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  ପ୍ରତିଜ୍ଞା(FS && State && !State->ଗଦା.empty() && "incrementing past end");
  ପ୍ରତିଜ୍ଞା(!State->ଗଦା.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  ଯଦି (State->Has_No_ଠେଲ_Request)
    State->Has_No_ଠେଲ_Request = ମିଥ୍ୟା;
  ଅନ୍ୟଥା {
    ଯଦି (State->ଗଦା.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ଗଦା.back()->path(), EC);
      ଯଦି (I != End) {
        State->ଗଦା.ଠେଲ_back(I);
        ଫେରନ୍ତୁ *ଏହି;
      }
    }
  }

  ଯେତେବେଳେ (!State->ଗଦା.empty() && State->ଗଦା.back().increment(EC) == End)
    State->ଗଦା.କାଢ଼_back();

  ଯଦି (State->ଗଦା.empty())
    State.reset(); // end iterator

  ଫେରନ୍ତୁ *ଏହି;
}

ଅଚଳ ଅକ୍ଷର FileSystem::ID = 0;
ଅଚଳ ଅକ୍ଷର OverlayFileSystem::ID = 0;
ଅଚଳ ଅକ୍ଷର ProxyFileSystem::ID = 0;
ଅଚଳ ଅକ୍ଷର InMemoryFileSystem::ID = 0;
ଅଚଳ ଅକ୍ଷର RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_ଗଣନା(
    ଅଚଳ FileSystem *FS) {
  ଫେରନ୍ତୁ FS->Use_ଗଣନା();
}

ଶୂନ୍ୟ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

ଶୂନ୍ୟ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
