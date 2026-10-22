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

#അന്തര്ഭാവ "llvm/Support/VirtualFileSystem.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/DenseMap.h"
#അന്തര്ഭാവ "llvm/ADT/IntrusiveRefCntPtr.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/StringSet.h"
#അന്തര്ഭാവ "llvm/ADT/Twine.h"
#അന്തര്ഭാവ "llvm/ADT/iterator_range.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/Casting.h"
#അന്തര്ഭാവ "llvm/Support/Chrono.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/ErrorOr.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem/UniqueID.h"
#അന്തര്ഭാവ "llvm/Support/IOSandbox.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/SMLoc.h"
#അന്തര്ഭാവ "llvm/Support/SourceMgr.h"
#അന്തര്ഭാവ "llvm/Support/YAMLParser.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <atomic>
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstdint>
#അന്തര്ഭാവ <iterator>
#അന്തര്ഭാവ <limits>
#അന്തര്ഭാവ <map>
#അന്തര്ഭാവ <memory>
#അന്തര്ഭാവ <optional>
#അന്തര്ഭാവ <string>
#അന്തര്ഭാവ <system_error>
#അന്തര്ഭാവ <utility>
#അന്തര്ഭാവ <vector>

using നാമാകാശ llvm;
using നാമാകാശ llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(സ്ഥിരം file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(സ്ഥിരം Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(സ്ഥിരം Status &In, uint64_t NewSize) {
  തിരികെ Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(സ്ഥിരം Status &In, സ്ഥിരം Twine &NewName) {
  തിരികെ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(സ്ഥിരം file_status &In, സ്ഥിരം Twine &NewName) {
  തിരികെ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

തർക്കം Status::equivalent(സ്ഥിരം Status &Other) സ്ഥിരം {
  പ്രതിജ്ഞാ(isStatusKnown() && Other.isStatusKnown());
  തിരികെ getUniqueID() == Other.getUniqueID();
}

തർക്കം Status::isDirectory() സ്ഥിരം { തിരികെ Type == file_type::directory_file; }

തർക്കം Status::isRegularFile() സ്ഥിരം { തിരികെ Type == file_type::regular_file; }

തർക്കം Status::isOther() സ്ഥിരം {
  തിരികെ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

തർക്കം Status::isSymlink() സ്ഥിരം { തിരികെ Type == file_type::symlink_file; }

തർക്കം Status::isStatusKnown() സ്ഥിരം { തിരികെ Type != file_type::status_error; }

തർക്കം Status::exists() സ്ഥിരം {
  തിരികെ isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = സ്ഥിരസ്ഥിതി;

FileSystem::~FileSystem() = സ്ഥിരസ്ഥിതി;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(സ്ഥിരം llvm::Twine &Name, int64_t FileSize,
                             തർക്കം RequiresNullTerminator, തർക്കം IsVolatile,
                             തർക്കം IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_ദ്വിമാന(Name);
  എങ്കിൽ (!F)
    തിരികെ F.getError();

  തിരികെ (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<അക്ഷരം> &Path) സ്ഥിരം {
  എങ്കിൽ (llvm::sys::path::is_absolute(Path))
    തിരികെ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  എങ്കിൽ (!WorkingDir)
    തിരികെ WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  തിരികെ {};
}

std::error_code FileSystem::getRealPath(സ്ഥിരം Twine &Path,
                                        SmallVectorImpl<അക്ഷരം> &Output) {
  തിരികെ errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(സ്ഥിരം Twine &Path, തർക്കം &Result) {
  തിരികെ errc::operation_not_permitted;
}

തർക്കം FileSystem::exists(സ്ഥിരം Twine &Path) {
  auto Status = status(Path);
  തിരികെ Status && Status->exists();
}

llvm::ErrorOr<തർക്കം> FileSystem::equivalent(സ്ഥിരം Twine &A, സ്ഥിരം Twine &B) {
  auto StatusA = status(A);
  എങ്കിൽ (!StatusA)
    തിരികെ StatusA.getError();
  auto StatusB = status(B);
  എങ്കിൽ (!StatusB)
    തിരികെ StatusB.getError();
  തിരികെ StatusA->equivalent(*StatusB);
}

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
ശൂന്യം FileSystem::dump() സ്ഥിരം { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
സ്ഥിതം തർക്കം isTraversalComponent(StringRef Component) {
  തിരികെ Component == ".." || Component == ".";
}

സ്ഥിതം തർക്കം pathHasTraversal(StringRef Path) {
  using നാമാകാശ llvm::sys;

  വേണ്ടി (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    എങ്കിൽ (isTraversalComponent(Comp))
      തിരികെ സത്യം;
  തിരികെ അസത്യം;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

നാമാകാശ {

/// Wrapper around a raw file descriptor.
വർഗ്ഗം RealFile : പൊതു File {
  friend വർഗ്ഗം RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    പ്രതിജ്ഞാ(FD.isValid() && "Invalid or inactive file descriptor");
  }

പൊതു:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(സ്ഥിരം Twine &Name,
                                                   int64_t FileSize,
                                                   തർക്കം RequiresNullTerminator,
                                                   തർക്കം IsVolatile) override;
  std::error_code close() override;
  ശൂന്യം setPath(സ്ഥിരം Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  പ്രതിജ്ഞാ(FD.isValid() && "cannot stat closed file");
  എങ്കിൽ (!S.isStatusKnown()) {
    file_status RealStatus;
    എങ്കിൽ (std::error_code EC = sys::fs::status(FD, RealStatus))
      തിരികെ EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  തിരികെ S;
}

ErrorOr<std::string> RealFile::getName() {
  തിരികെ RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(സ്ഥിരം Twine &Name, int64_t FileSize,
                    തർക്കം RequiresNullTerminator, തർക്കം IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  പ്രതിജ്ഞാ(FD.isValid() && "cannot get buffer for closed file");
  തിരികെ MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  തിരികെ EC;
}

ശൂന്യം RealFile::setPath(സ്ഥിരം Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  എങ്കിൽ (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
}

നാമാകാശ {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
വർഗ്ഗം RealFileSystem : പൊതു FileSystem {
പൊതു:
  explicit RealFileSystem(തർക്കം LinkCWDToProcess) {
    എങ്കിൽ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      എങ്കിൽ (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      അല്ലെങ്കിൽ എങ്കിൽ (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      അല്ലെങ്കിൽ
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(സ്ഥിരം Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(സ്ഥിരം Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ദ്വിമാന(സ്ഥിരം Twine &Path) override;
  directory_iterator dir_begin(സ്ഥിരം Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() സ്ഥിരം override;
  std::error_code setCurrentWorkingDirectory(സ്ഥിരം Twine &Path) override;
  std::error_code isLocal(സ്ഥിരം Twine &Path, തർക്കം &Result) override;
  ശൂന്യം
  getDirectoryContentRealSources(സ്ഥിരം Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.തള്ളുക_back(Dir.str());
  }
  std::error_code getRealPath(സ്ഥിരം Twine &Path,
                              SmallVectorImpl<അക്ഷരം> &Output) override;

സംരക്ഷിത:
  ശൂന്യം printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) സ്ഥിരം override;

സ്വകാര്യ:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(സ്ഥിരം Twine &Path, SmallVectorImpl<അക്ഷരം> &Storage) സ്ഥിരം {
    എങ്കിൽ (!WD || !*WD)
      തിരികെ Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    തിരികെ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(സ്ഥിരം Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    എങ്കിൽ (!FDOrErr)
      തിരികെ errorToErrorCode(FDOrErr.takeError());
    തിരികെ std::unique_ptr<File>(
        പുതിയ RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  ഘടന WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(സ്ഥിരം Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  എങ്കിൽ (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    തിരികെ EC;
  തിരികെ Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(സ്ഥിരം Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  തിരികെ openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ദ്വിമാന(സ്ഥിരം Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  തിരികെ openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() സ്ഥിരം {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  എങ്കിൽ (WD && *WD)
    തിരികെ std::string(WD->get().Specified);
  എങ്കിൽ (WD)
    തിരികെ WD->getError();

  SmallString<128> Dir;
  എങ്കിൽ (std::error_code EC = llvm::sys::fs::current_path(Dir))
    തിരികെ EC;
  തിരികെ std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(സ്ഥിരം Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  എങ്കിൽ (!WD)
    തിരികെ llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  തർക്കം IsDir;
  എങ്കിൽ (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    തിരികെ Err;
  എങ്കിൽ (!IsDir)
    തിരികെ std::make_error_code(std::errc::not_a_directory);
  എങ്കിൽ (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    തിരികെ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  തിരികെ std::error_code();
}

std::error_code RealFileSystem::isLocal(സ്ഥിരം Twine &Path, തർക്കം &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  തിരികെ llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(സ്ഥിരം Twine &Path,
                                            SmallVectorImpl<അക്ഷരം> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  തിരികെ llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

ശൂന്യം RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) സ്ഥിരം {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  എങ്കിൽ (WD)
    OS << "own";
  അല്ലെങ്കിൽ
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  സ്ഥിതം IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(സത്യം);
  sys::sandbox::violationIfEnabled();

  തിരികെ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  തിരികെ std::make_unique<RealFileSystem>(അസത്യം);
}

നാമാകാശ {

വർഗ്ഗം RealFSDirIter : പൊതു llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

പൊതു:
  RealFSDirIter(സ്ഥിരം Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    എങ്കിൽ (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    തിരികെ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(സ്ഥിരം Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  തിരികെ directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.തള്ളുക_back(std::move(BaseFS));
}

ശൂന്യം OverlayFileSystem::തള്ളുക_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.തള്ളുക_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(സ്ഥിരം Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  വേണ്ടി (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    എങ്കിൽ (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      തിരികെ Status;
  }
  തിരികെ make_error_code(llvm::errc::no_such_file_or_directory);
}

തർക്കം OverlayFileSystem::exists(സ്ഥിരം Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  വേണ്ടി (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    എങ്കിൽ ((*I)->exists(Path))
      തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(സ്ഥിരം llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  വേണ്ടി (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    എങ്കിൽ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      തിരികെ Result;
  }
  തിരികെ make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() സ്ഥിരം {
  // All file systems are synchronized, just take the first working directory.
  തിരികെ FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(സ്ഥിരം Twine &Path) {
  വേണ്ടി (auto &FS : FSList)
    എങ്കിൽ (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      തിരികെ EC;
  തിരികെ {};
}

std::error_code OverlayFileSystem::isLocal(സ്ഥിരം Twine &Path, തർക്കം &Result) {
  വേണ്ടി (auto &FS : FSList)
    എങ്കിൽ (FS->exists(Path))
      തിരികെ FS->isLocal(Path, Result);
  തിരികെ errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(സ്ഥിരം Twine &Path,
                                               SmallVectorImpl<അക്ഷരം> &Output) {
  വേണ്ടി (സ്ഥിരം auto &FS : FSList)
    എങ്കിൽ (FS->exists(Path))
      തിരികെ FS->getRealPath(Path, Output);
  തിരികെ errc::no_such_file_or_directory;
}

ശൂന്യം OverlayFileSystem::getDirectoryContentRealSources(
    സ്ഥിരം Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  വേണ്ടി (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

ശൂന്യം OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  വേണ്ടി (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

ശൂന്യം OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) സ്ഥിരം {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  എങ്കിൽ (Type == PrintType::Summary)
    തിരികെ;

  എങ്കിൽ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  വേണ്ടി (സ്ഥിരം auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = സ്ഥിരസ്ഥിതി;

നാമാകാശ {

/// Combines and deduplicates directory entries across multiple file systems.
വർഗ്ഗം CombiningDirIterImpl : പൊതു llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(തർക്കം IsFirstTime) {
    ആകുമ്പോൾ (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.എടുക്കുക_back();
      എങ്കിൽ (CurrentDirIter != directory_iterator())
        നിർത്തുക; // found
    }

    എങ്കിൽ (IsFirstTime && CurrentDirIter == directory_iterator())
      തിരികെ errc::no_such_file_or_directory;
    തിരികെ {};
  }

  std::error_code incrementDirIter(തർക്കം IsFirstTime) {
    പ്രതിജ്ഞാ((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    എങ്കിൽ (!IsFirstTime)
      CurrentDirIter.increment(EC);
    എങ്കിൽ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    തിരികെ EC;
  }

  std::error_code incrementImpl(തർക്കം IsFirstTime) {
    ആകുമ്പോൾ (സത്യം) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      എങ്കിൽ (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        തിരികെ EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      എങ്കിൽ (SeenNames.തിരുകുക(Name).second)
        തിരികെ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

പൊതു:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    വേണ്ടി (സ്ഥിരം auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      എങ്കിൽ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        തിരികെ;
      }
      എങ്കിൽ (!FEC)
        IterList.തള്ളുക_back(Iter);
    }
    EC = incrementImpl(സത്യം);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(സത്യം);
  }

  std::error_code increment() override { തിരികെ incrementImpl(അസത്യം); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(സ്ഥിരം Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  എങ്കിൽ (EC)
    തിരികെ {};
  തിരികെ Combined;
}

ശൂന്യം ProxyFileSystem::anchor() {}

നാമാകാശ llvm {
നാമാകാശ vfs {

നാമാകാശ detail {

എണ്ണൽ In_Memory_കണ്ണി_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
വർഗ്ഗം In_Memory_കണ്ണി {
  In_Memory_കണ്ണി_Kind Kind;
  std::string FileName;

പൊതു:
  In_Memory_കണ്ണി(llvm::StringRef FileName, In_Memory_കണ്ണി_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  മായാ ~In_Memory_കണ്ണി() = സ്ഥിരസ്ഥിതി;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  മായാ Status getStatus(സ്ഥിരം Twine &RequestedName) സ്ഥിരം = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() സ്ഥിരം { തിരികെ FileName; }
  In_Memory_കണ്ണി_Kind getKind() സ്ഥിരം { തിരികെ Kind; }
  മായാ std::string toString(unsigned Indent) സ്ഥിരം = 0;
};

വർഗ്ഗം InMemoryFile : പൊതു In_Memory_കണ്ണി {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

പൊതു:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_കണ്ണി(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(സ്ഥിരം Twine &RequestedName) സ്ഥിരം override {
    തിരികെ Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() സ്ഥിരം { തിരികെ Buffer.get(); }

  std::string toString(unsigned Indent) സ്ഥിരം override {
    തിരികെ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  സ്ഥിതം തർക്കം classof(സ്ഥിരം In_Memory_കണ്ണി *N) {
    തിരികെ N->getKind() == IME_File;
  }
};

നാമാകാശ {

വർഗ്ഗം InMemoryHardLink : പൊതു In_Memory_കണ്ണി {
  സ്ഥിരം InMemoryFile &ResolvedFile;

പൊതു:
  InMemoryHardLink(StringRef Path, സ്ഥിരം InMemoryFile &ResolvedFile)
      : In_Memory_കണ്ണി(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  സ്ഥിരം InMemoryFile &getResolvedFile() സ്ഥിരം { തിരികെ ResolvedFile; }

  Status getStatus(സ്ഥിരം Twine &RequestedName) സ്ഥിരം override {
    തിരികെ ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) സ്ഥിരം override {
    തിരികെ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  സ്ഥിതം തർക്കം classof(സ്ഥിരം In_Memory_കണ്ണി *N) {
    തിരികെ N->getKind() == IME_HardLink;
  }
};

വർഗ്ഗം InMemorySymbolicLink : പൊതു In_Memory_കണ്ണി {
  std::string TargetPath;
  Status Stat;

പൊതു:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_കണ്ണി(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) സ്ഥിരം override {
    തിരികെ std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(സ്ഥിരം Twine &RequestedName) സ്ഥിരം override {
    തിരികെ Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() സ്ഥിരം { തിരികെ TargetPath; }

  സ്ഥിതം തർക്കം classof(സ്ഥിരം In_Memory_കണ്ണി *N) {
    തിരികെ N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
വർഗ്ഗം InMemoryFileAdaptor : പൊതു File {
  സ്ഥിരം InMemoryFile &കണ്ണി;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

പൊതു:
  explicit InMemoryFileAdaptor(സ്ഥിരം InMemoryFile &കണ്ണി,
                               std::string RequestedName)
      : കണ്ണി(കണ്ണി), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    തിരികെ കണ്ണി.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(സ്ഥിരം Twine &Name, int64_t FileSize, തർക്കം RequiresNullTerminator,
            തർക്കം IsVolatile) override {
    llvm::MemoryBuffer *Buf = കണ്ണി.getBuffer();
    തിരികെ llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { തിരികെ {}; }

  ശൂന്യം setPath(സ്ഥിരം Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

വർഗ്ഗം InMemoryDirectory : പൊതു In_Memory_കണ്ണി {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_കണ്ണി>, std::less<>> Entries;

പൊതു:
  InMemoryDirectory(Status Stat)
      : In_Memory_കണ്ണി(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(സ്ഥിരം Twine &RequestedName) സ്ഥിരം override {
    തിരികെ Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() സ്ഥിരം { തിരികെ Stat.getUniqueID(); }

  In_Memory_കണ്ണി *getChild(StringRef Name) സ്ഥിരം {
    auto I = Entries.find(Name);
    എങ്കിൽ (I != Entries.end())
      തിരികെ I->second.get();
    തിരികെ nullptr;
  }

  In_Memory_കണ്ണി *addChild(StringRef Name, std::unique_ptr<In_Memory_കണ്ണി> Child) {
    തിരികെ Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() സ്ഥിരം { തിരികെ Entries.begin(); }
  const_iterator end() സ്ഥിരം { തിരികെ Entries.end(); }

  std::string toString(unsigned Indent) സ്ഥിരം override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    വേണ്ടി (സ്ഥിരം auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    തിരികെ Result;
  }

  സ്ഥിതം തർക്കം classof(സ്ഥിരം In_Memory_കണ്ണി *N) {
    തിരികെ N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
സ്ഥിതം sys::fs::UniqueID getUniqueID(hash_code Hash) {
  തിരികെ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
സ്ഥിതം sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  തിരികെ getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
സ്ഥിതം sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  തിരികെ getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_കണ്ണി_Info::makeStatus() സ്ഥിരം {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  തിരികെ Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(തർക്കം UseNormalizedPaths)
    : വേര്_2(പുതിയ detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = സ്ഥിരസ്ഥിതി;

std::string InMemoryFileSystem::toString() സ്ഥിരം {
  തിരികെ വേര്_2->toString(/*Indent=*/0);
}

തർക്കം InMemoryFileSystem::addFile(സ്ഥിരം Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_കണ്ണി_Fn Make_കണ്ണി) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  പ്രതിജ്ഞാ(!EC);
  (ശൂന്യം)EC;

  എങ്കിൽ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/സത്യം);

  എങ്കിൽ (Path.empty())
    തിരികെ അസത്യം;

  detail::InMemoryDirectory *Dir = വേര്_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  സ്ഥിരം auto ResolvedUser = User.മൂല്യം_or(0);
  സ്ഥിരം auto ResolvedGroup = Group.മൂല്യം_or(0);
  സ്ഥിരം auto ResolvedType = Type.മൂല്യം_or(sys::fs::file_type::regular_file);
  സ്ഥിരം auto ResolvedPerms = Perms.മൂല്യം_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  സ്ഥിരം auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  ആകുമ്പോൾ (സത്യം) {
    Name = *I;
    ++I;
    എങ്കിൽ (I == E)
      നിർത്തുക;
    detail::In_Memory_കണ്ണി *കണ്ണി = Dir->getChild(Name);
    എങ്കിൽ (!കണ്ണി) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      തുടരുക;
    }
    // Creating file under another file.
    എങ്കിൽ (!isa<detail::InMemoryDirectory>(കണ്ണി))
      തിരികെ അസത്യം;
    Dir = cast<detail::InMemoryDirectory>(കണ്ണി);
  }
  detail::In_Memory_കണ്ണി *കണ്ണി = Dir->getChild(Name);
  എങ്കിൽ (!കണ്ണി) {
    Dir->addChild(Name,
                  Make_കണ്ണി({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    തിരികെ സത്യം;
  }
  എങ്കിൽ (isa<detail::InMemoryDirectory>(കണ്ണി))
    തിരികെ ResolvedType == sys::fs::file_type::directory_file;

  പ്രതിജ്ഞാ((isa<detail::InMemoryFile>(കണ്ണി) ||
          isa<detail::InMemoryHardLink>(കണ്ണി)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  എങ്കിൽ (auto *Link = dyn_cast<detail::InMemoryHardLink>(കണ്ണി)) {
    തിരികെ Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  തിരികെ cast<detail::InMemoryFile>(കണ്ണി)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

തർക്കം InMemoryFileSystem::addFile(സ്ഥിരം Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  തിരികെ addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_കണ്ണി_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_കണ്ണി> {
                   Status Stat = NNI.makeStatus();
                   എങ്കിൽ (Stat.getType() == sys::fs::file_type::directory_file)
                     തിരികെ std::make_unique<detail::InMemoryDirectory>(Stat);
                   തിരികെ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

തർക്കം InMemoryFileSystem::addFileNoOwn(
    സ്ഥിരം Twine &P, time_t ModificationTime,
    സ്ഥിരം llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  തിരികെ addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_കണ്ണി_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_കണ്ണി> {
                   Status Stat = NNI.makeStatus();
                   എങ്കിൽ (Stat.getType() == sys::fs::file_type::directory_file)
                     തിരികെ std::make_unique<detail::InMemoryDirectory>(Stat);
                   തിരികെ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_കണ്ണി_Or_Error
InMemoryFileSystem::lookup_കണ്ണി(സ്ഥിരം Twine &P, തർക്കം FollowFinalSymlink,
                               size_t SymlinkDepth) സ്ഥിരം {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  പ്രതിജ്ഞാ(!EC);
  (ശൂന്യം)EC;

  എങ്കിൽ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/സത്യം);

  സ്ഥിരം detail::InMemoryDirectory *Dir = വേര്_2.get();
  എങ്കിൽ (Path.empty())
    തിരികെ detail::Named_കണ്ണി_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  ആകുമ്പോൾ (സത്യം) {
    detail::In_Memory_കണ്ണി *കണ്ണി = Dir->getChild(*I);
    ++I;
    എങ്കിൽ (!കണ്ണി)
      തിരികെ errc::no_such_file_or_directory;

    എങ്കിൽ (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(കണ്ണി)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      എങ്കിൽ (I == E && !FollowFinalSymlink)
        തിരികെ detail::Named_കണ്ണി_Or_Error(Path, Symlink);

      എങ്കിൽ (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        തിരികെ errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      എങ്കിൽ (std::error_code EC = makeAbsolute(TargetPath))
        തിരികെ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_കണ്ണി(TargetPath, /*FollowFinalSymlink=*/സത്യം, SymlinkDepth + 1);
      എങ്കിൽ (!Target || I == E)
        തിരികെ Target;

      എങ്കിൽ (!isa<detail::InMemoryDirectory>(*Target))
        തിരികെ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      തുടരുക;
    }

    // Return the file if it's at the end of the path.
    എങ്കിൽ (auto File = dyn_cast<detail::InMemoryFile>(കണ്ണി)) {
      എങ്കിൽ (I == E)
        തിരികെ detail::Named_കണ്ണി_Or_Error(Path, File);
      തിരികെ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    എങ്കിൽ (auto File = dyn_cast<detail::InMemoryHardLink>(കണ്ണി)) {
      എങ്കിൽ (I == E)
        തിരികെ detail::Named_കണ്ണി_Or_Error(Path, &File->getResolvedFile());
      തിരികെ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(കണ്ണി);
    എങ്കിൽ (I == E)
      തിരികെ detail::Named_കണ്ണി_Or_Error(Path, Dir);
  }
}

തർക്കം InMemoryFileSystem::addHardLink(സ്ഥിരം Twine &NewLink,
                                     സ്ഥിരം Twine &Target) {
  auto New_Link_കണ്ണി = lookup_കണ്ണി(NewLink, /*FollowFinalSymlink=*/അസത്യം);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_കണ്ണി = lookup_കണ്ണി(Target, /*FollowFinalSymlink=*/സത്യം);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  എങ്കിൽ (!Target_കണ്ണി || New_Link_കണ്ണി || !isa<detail::InMemoryFile>(*Target_കണ്ണി))
    തിരികെ അസത്യം;
  തിരികെ addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_കണ്ണി_Info NNI) {
                   തിരികെ std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_കണ്ണി));
                 });
}

തർക്കം InMemoryFileSystem::addSymbolicLink(
    സ്ഥിരം Twine &NewLink, സ്ഥിരം Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_കണ്ണി = lookup_കണ്ണി(NewLink, /*FollowFinalSymlink=*/അസത്യം);
  എങ്കിൽ (New_Link_കണ്ണി)
    തിരികെ അസത്യം;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  തിരികെ addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_കണ്ണി_Info NNI) {
                   തിരികെ std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(സ്ഥിരം Twine &Path) {
  auto കണ്ണി = lookup_കണ്ണി(Path, /*FollowFinalSymlink=*/സത്യം);
  എങ്കിൽ (കണ്ണി)
    തിരികെ (*കണ്ണി)->getStatus(Path);
  തിരികെ കണ്ണി.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(സ്ഥിരം Twine &Path) {
  auto കണ്ണി = lookup_കണ്ണി(Path,/*FollowFinalSymlink=*/സത്യം);
  എങ്കിൽ (!കണ്ണി)
    തിരികെ കണ്ണി.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  എങ്കിൽ (auto *F = dyn_cast<detail::InMemoryFile>(*കണ്ണി))
    തിരികെ std::unique_ptr<File>(
        പുതിയ detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  തിരികെ make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
വർഗ്ഗം InMemoryFileSystem::DirIterator : പൊതു llvm::vfs::detail::DirIterImpl {
  സ്ഥിരം InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  ശൂന്യം setCurrentEntry() {
    എങ്കിൽ (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      തിരഞ്ഞെടുപ്പ് (I->second->getKind()) {
      അവസ്ഥ detail::IME_File:
      അവസ്ഥ detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        നിർത്തുക;
      അവസ്ഥ detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        നിർത്തുക;
      അവസ്ഥ detail::IME_SymbolicLink:
        എങ്കിൽ (auto SymlinkTarget =
                FS->lookup_കണ്ണി(Path, /*FollowFinalSymlink=*/സത്യം)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        നിർത്തുക;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } അല്ലെങ്കിൽ {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

പൊതു:
  DirIterator() = സ്ഥിരസ്ഥിതി;

  DirIterator(സ്ഥിരം InMemoryFileSystem *FS,
              സ്ഥിരം detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    തിരികെ {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(സ്ഥിരം Twine &Dir,
                                                 std::error_code &EC) {
  auto കണ്ണി = lookup_കണ്ണി(Dir, /*FollowFinalSymlink=*/സത്യം);
  എങ്കിൽ (!കണ്ണി) {
    EC = കണ്ണി.getError();
    തിരികെ directory_iterator(std::make_shared<DirIterator>());
  }

  എങ്കിൽ (auto *Dir_കണ്ണി = dyn_cast<detail::InMemoryDirectory>(*കണ്ണി))
    തിരികെ directory_iterator(
        std::make_shared<DirIterator>(ഇത്, *Dir_കണ്ണി, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  തിരികെ directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(സ്ഥിരം Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  പ്രതിജ്ഞാ(!EC);
  (ശൂന്യം)EC;

  എങ്കിൽ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/സത്യം);

  എങ്കിൽ (!Path.empty())
    WorkingDirectory = std::string(Path);
  തിരികെ {};
}

std::error_code InMemoryFileSystem::getRealPath(സ്ഥിരം Twine &Path,
                                                SmallVectorImpl<അക്ഷരം> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  എങ്കിൽ (!CWD || CWD->empty())
    തിരികെ errc::operation_not_permitted;
  Path.toVector(Output);
  എങ്കിൽ (auto EC = makeAbsolute(Output))
    തിരികെ EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/സത്യം);
  തിരികെ {};
}

std::error_code InMemoryFileSystem::isLocal(സ്ഥിരം Twine &Path, തർക്കം &Result) {
  Result = അസത്യം;
  തിരികെ {};
}

ശൂന്യം InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) സ്ഥിരം {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

നാമാകാശ {

സ്ഥിതം llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  സ്ഥിരം size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  എങ്കിൽ (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  തിരികെ style;
}

/// Removes leading "./" as well as path components like ".." and ".".
സ്ഥിതം llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/സത്യം, style);
  തിരികെ result;
}

/// Whether the error and entry specify a file/directory that was not found.
സ്ഥിതം തർക്കം isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  എങ്കിൽ (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    തിരികെ അസത്യം;
  തിരികെ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  പ്രതിജ്ഞാ(ExternalFS && "RedirectingFileSystem requires an external FS");
  എങ്കിൽ (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
വർഗ്ഗം llvm::vfs::RedirectingFSDirIterImpl
    : പൊതു llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(തർക്കം IsFirstTime) {
    പ്രതിജ്ഞാ((IsFirstTime || Current != End) && "cannot iterate past end");
    എങ്കിൽ (!IsFirstTime)
      ++Current;
    എങ്കിൽ (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      തിരഞ്ഞെടുപ്പ് ((*Current)->getKind()) {
      അവസ്ഥ RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      അവസ്ഥ RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        നിർത്തുക;
      അവസ്ഥ RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        നിർത്തുക;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } അല്ലെങ്കിൽ {
      CurrentEntry = directory_entry();
    }
    തിരികെ {};
  };

പൊതു:
  RedirectingFSDirIterImpl(
      സ്ഥിരം Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/സത്യം);
  }

  std::error_code increment() override {
    തിരികെ incrementImpl(/*IsFirstTime=*/അസത്യം);
  }
};

നാമാകാശ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
വർഗ്ഗം RedirectingFSDirRemapIterImpl : പൊതു llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

പൊതു:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    എങ്കിൽ (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  ശൂന്യം setCurrentEntry() {
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
    എങ്കിൽ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    അല്ലെങ്കിൽ
      CurrentEntry = directory_entry();
    തിരികെ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() സ്ഥിരം {
  തിരികെ WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(സ്ഥിരം Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  എങ്കിൽ (!exists(Path))
    തിരികെ errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  എങ്കിൽ (std::error_code EC = makeAbsolute(AbsolutePath))
    തിരികെ EC;
  WorkingDirectory = std::string(AbsolutePath);
  തിരികെ {};
}

std::error_code RedirectingFileSystem::isLocal(സ്ഥിരം Twine &Path_,
                                               തർക്കം &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  എങ്കിൽ (makeAbsolute(Path))
    തിരികെ {};

  തിരികെ ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<അക്ഷരം> &Path) സ്ഥിരം {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  എങ്കിൽ (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    തിരികെ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  എങ്കിൽ (!WorkingDir)
    തിരികെ WorkingDir.getError();

  തിരികെ makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<അക്ഷരം> &Path) സ്ഥിരം {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  എങ്കിൽ (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    തിരികെ std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  എങ്കിൽ (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } അല്ലെങ്കിൽ {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    എങ്കിൽ (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  എങ്കിൽ (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  തിരികെ {};
}

directory_iterator RedirectingFileSystem::dir_begin(സ്ഥിരം Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  എങ്കിൽ (EC)
    തിരികെ {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  എങ്കിൽ (!Result) {
    എങ്കിൽ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      തിരികെ ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    തിരികെ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  എങ്കിൽ (!S) {
    എങ്കിൽ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      തിരികെ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    തിരികെ {};
  }

  എങ്കിൽ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    തിരികെ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  എങ്കിൽ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    എങ്കിൽ (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } അല്ലെങ്കിൽ {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  എങ്കിൽ (RedirectEC) {
    എങ്കിൽ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      തിരികെ {};
    }
    RedirectIter = {};
  }

  എങ്കിൽ (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    തിരികെ RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  എങ്കിൽ (ExternalEC) {
    എങ്കിൽ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      തിരികെ {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  തിരഞ്ഞെടുപ്പ് (Redirection) {
  അവസ്ഥ RedirectKind::Fallthrough:
    Iters.തള്ളുക_back(ExternalIter);
    Iters.തള്ളുക_back(RedirectIter);
    നിർത്തുക;
  അവസ്ഥ RedirectKind::Fallback:
    Iters.തള്ളുക_back(RedirectIter);
    Iters.തള്ളുക_back(ExternalIter);
    നിർത്തുക;
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  എങ്കിൽ (EC)
    തിരികെ {};
  തിരികെ Combined;
}

ശൂന്യം RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() സ്ഥിരം {
  തിരികെ OverlayFileDir;
}

ശൂന്യം RedirectingFileSystem::setFallthrough(തർക്കം Fallthrough) {
  എങ്കിൽ (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } അല്ലെങ്കിൽ {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

ശൂന്യം RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() സ്ഥിരം {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  വേണ്ടി (സ്ഥിരം auto &വേര്_2 : Roots)
    R.തള്ളുക_back(വേര്_2->getName());
  തിരികെ R;
}

ശൂന്യം RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) സ്ഥിരം {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  എങ്കിൽ (Type == PrintType::Summary)
    തിരികെ;

  വേണ്ടി (സ്ഥിരം auto &വേര്_2 : Roots)
    printEntry(OS, വേര്_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

ശൂന്യം RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) സ്ഥിരം {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  തിരഞ്ഞെടുപ്പ് (E->getKind()) {
  അവസ്ഥ EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    വേണ്ടി (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    നിർത്തുക;
  }
  അവസ്ഥ EK_DirectoryRemap:
  അവസ്ഥ EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    തിരഞ്ഞെടുപ്പ് (RE->getUseName()) {
    അവസ്ഥ NK_NotSet:
      നിർത്തുക;
    അവസ്ഥ NK_External:
      OS << " (UseExternalName: true)";
      നിർത്തുക;
    അവസ്ഥ NK_Virtual:
      OS << " (UseExternalName: false)";
      നിർത്തുക;
    }
    OS << "\n";
    നിർത്തുക;
  }
  }
}

ശൂന്യം RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  എങ്കിൽ (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
വർഗ്ഗം llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  ശൂന്യം error(yaml::കണ്ണി *N, സ്ഥിരം Twine &സന്ദേശം_2) { Stream.printError(N, സന്ദേശം_2); }

  // false on error
  തർക്കം parseScalarString(yaml::കണ്ണി *N, StringRef &Result,
                         SmallVectorImpl<അക്ഷരം> &Storage) {
    സ്ഥിരം auto *S = dyn_cast_if_present<yaml::Scalar_കണ്ണി>(N);

    എങ്കിൽ (!S) {
      error(N, "expected string");
      തിരികെ അസത്യം;
    }
    Result = S->get_മൂല്യം(Storage);
    തിരികെ സത്യം;
  }

  // false on error
  തർക്കം parseScalarBool(yaml::കണ്ണി *N, തർക്കം &Result) {
    SmallString<5> Storage;
    StringRef മൂല്യം;
    എങ്കിൽ (!parseScalarString(N, മൂല്യം, Storage))
      തിരികെ അസത്യം;

    എങ്കിൽ (മൂല്യം.equals_insensitive("true") || മൂല്യം.equals_insensitive("on") ||
        മൂല്യം.equals_insensitive("yes") || മൂല്യം == "1") {
      Result = സത്യം;
      തിരികെ സത്യം;
    } അല്ലെങ്കിൽ എങ്കിൽ (മൂല്യം.equals_insensitive("false") ||
               മൂല്യം.equals_insensitive("off") ||
               മൂല്യം.equals_insensitive("no") || മൂല്യം == "0") {
      Result = അസത്യം;
      തിരികെ സത്യം;
    }

    error(N, "expected boolean value");
    തിരികെ അസത്യം;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::കണ്ണി *N) {
    SmallString<12> Storage;
    StringRef മൂല്യം;
    എങ്കിൽ (!parseScalarString(N, മൂല്യം, Storage))
      തിരികെ std::nullopt;

    എങ്കിൽ (മൂല്യം.equals_insensitive("fallthrough")) {
      തിരികെ RedirectingFileSystem::RedirectKind::Fallthrough;
    } അല്ലെങ്കിൽ എങ്കിൽ (മൂല്യം.equals_insensitive("fallback")) {
      തിരികെ RedirectingFileSystem::RedirectKind::Fallback;
    } അല്ലെങ്കിൽ എങ്കിൽ (മൂല്യം.equals_insensitive("redirect-only")) {
      തിരികെ RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    തിരികെ std::nullopt;
  }

  std::optional<RedirectingFileSystem::വേര്_Relative_Kind>
  parse_വേര്_Relative_Kind(yaml::കണ്ണി *N) {
    SmallString<12> Storage;
    StringRef മൂല്യം;
    എങ്കിൽ (!parseScalarString(N, മൂല്യം, Storage))
      തിരികെ std::nullopt;
    എങ്കിൽ (മൂല്യം.equals_insensitive("cwd")) {
      തിരികെ RedirectingFileSystem::വേര്_Relative_Kind::CWD;
    } അല്ലെങ്കിൽ എങ്കിൽ (മൂല്യം.equals_insensitive("overlay-dir")) {
      തിരികെ RedirectingFileSystem::വേര്_Relative_Kind::OverlayDir;
    }
    തിരികെ std::nullopt;
  }

  ഘടന താക്കോൽ_Status {
    തർക്കം Required;
    തർക്കം Seen = അസത്യം;

    താക്കോൽ_Status(തർക്കം Required = അസത്യം) : Required(Required) {}
  };

  using താക്കോൽ_Status_Pair = std::pair<StringRef, താക്കോൽ_Status>;

  // false on error
  തർക്കം check_Duplicate_Or_Unknown_താക്കോൽ(yaml::കണ്ണി *താക്കോൽ_കണ്ണി, StringRef താക്കോൽ,
                                  DenseMap<StringRef, താക്കോൽ_Status> &താക്കോലുകൾ_2) {
    auto It = താക്കോലുകൾ_2.find(താക്കോൽ);
    എങ്കിൽ (It == താക്കോലുകൾ_2.end()) {
      error(താക്കോൽ_കണ്ണി, "unknown key");
      തിരികെ അസത്യം;
    }
    താക്കോൽ_Status &S = It->second;
    എങ്കിൽ (S.Seen) {
      error(താക്കോൽ_കണ്ണി, Twine("duplicate key '") + താക്കോൽ + "'");
      തിരികെ അസത്യം;
    }
    S.Seen = സത്യം;
    തിരികെ സത്യം;
  }

  // false on error
  തർക്കം check_Missing_താക്കോലുകൾ(yaml::കണ്ണി *Obj, DenseMap<StringRef, താക്കോൽ_Status> &താക്കോലുകൾ_2) {
    വേണ്ടി (സ്ഥിരം auto &I : താക്കോലുകൾ_2) {
      എങ്കിൽ (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        തിരികെ അസത്യം;
      }
    }
    തിരികെ സത്യം;
  }

പൊതു:
  സ്ഥിതം RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    എങ്കിൽ (!ParentEntry) { // Look for a existent root
      വേണ്ടി (സ്ഥിരം auto &വേര്_2 : FS->Roots) {
        എങ്കിൽ (Name == വേര്_2->getName()) {
          ParentEntry = വേര്_2.get();
          തിരികെ ParentEntry;
        }
      }
    } അല്ലെങ്കിൽ { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      വേണ്ടി (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        എങ്കിൽ (DirContent && Name == Content->getName())
          തിരികെ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    എങ്കിൽ (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.തള്ളുക_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      തിരികെ ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    തിരികെ DE->getLastContent();
  }

സ്വകാര്യ:
  ശൂന്യം unique_Overlay_വൃക്ഷം(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    തിരഞ്ഞെടുപ്പ് (SrcE->getKind()) {
    അവസ്ഥ RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      എങ്കിൽ (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      വേണ്ടി (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_വൃക്ഷം(FS, SubEntry.get(), NewParentE);
      നിർത്തുക;
    }
    അവസ്ഥ RedirectingFileSystem::EK_DirectoryRemap: {
      പ്രതിജ്ഞാ(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      നിർത്തുക;
    }
    അവസ്ഥ RedirectingFileSystem::EK_File: {
      പ്രതിജ്ഞാ(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      നിർത്തുക;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::കണ്ണി *N, RedirectingFileSystem *FS, തർക്കം Is_വേര്_Entry) {
    auto *M = dyn_cast<yaml::Mapping_കണ്ണി>(N);
    എങ്കിൽ (!M) {
      error(N, "expected mapping node for file or directory entry");
      തിരികെ nullptr;
    }

    താക്കോൽ_Status_Pair Fields[] = {
        താക്കോൽ_Status_Pair("name", സത്യം),
        താക്കോൽ_Status_Pair("type", സത്യം),
        താക്കോൽ_Status_Pair("contents", അസത്യം),
        താക്കോൽ_Status_Pair("external-contents", അസത്യം),
        താക്കോൽ_Status_Pair("use-external-name", അസത്യം),
    };

    DenseMap<StringRef, താക്കോൽ_Status> താക്കോലുകൾ_2(std::begin(Fields), std::end(Fields));

    എണ്ണൽ { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::കണ്ണി *Name_മൂല്യം_കണ്ണി = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    വേണ്ടി (auto &I : *M) {
      StringRef താക്കോൽ;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      എങ്കിൽ (!parseScalarString(I.get_താക്കോൽ(), താക്കോൽ, Buffer))
        തിരികെ nullptr;

      എങ്കിൽ (!check_Duplicate_Or_Unknown_താക്കോൽ(I.get_താക്കോൽ(), താക്കോൽ, താക്കോലുകൾ_2))
        തിരികെ nullptr;

      StringRef മൂല്യം;
      എങ്കിൽ (താക്കോൽ == "name") {
        എങ്കിൽ (!parseScalarString(I.get_മൂല്യം(), മൂല്യം, Buffer))
          തിരികെ nullptr;

        Name_മൂല്യം_കണ്ണി = I.get_മൂല്യം();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(മൂല്യം).str();
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "type") {
        എങ്കിൽ (!parseScalarString(I.get_മൂല്യം(), മൂല്യം, Buffer))
          തിരികെ nullptr;
        എങ്കിൽ (മൂല്യം == "file")
          Kind = RedirectingFileSystem::EK_File;
        അല്ലെങ്കിൽ എങ്കിൽ (മൂല്യം == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        അല്ലെങ്കിൽ എങ്കിൽ (മൂല്യം == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        അല്ലെങ്കിൽ {
          error(I.get_മൂല്യം(), "unknown value for 'type'");
          തിരികെ nullptr;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "contents") {
        എങ്കിൽ (ContentsField != CF_NotSet) {
          error(I.get_താക്കോൽ(),
                "entry already has 'contents' or 'external-contents'");
          തിരികെ nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_കണ്ണി>(I.get_മൂല്യം());
        എങ്കിൽ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_മൂല്യം(), "expected array");
          തിരികെ nullptr;
        }

        വേണ്ടി (auto &I : *Contents) {
          എങ്കിൽ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ അസത്യം))
            EntryArrayContents.തള്ളുക_back(std::move(E));
          അല്ലെങ്കിൽ
            തിരികെ nullptr;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "external-contents") {
        എങ്കിൽ (ContentsField != CF_NotSet) {
          error(I.get_താക്കോൽ(),
                "entry already has 'contents' or 'external-contents'");
          തിരികെ nullptr;
        }
        ContentsField = CF_External;
        എങ്കിൽ (!parseScalarString(I.get_മൂല്യം(), മൂല്യം, Buffer))
          തിരികെ nullptr;

        SmallString<256> FullPath;
        എങ്കിൽ (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          പ്രതിജ്ഞാ(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = മൂല്യം;
          എങ്കിൽ (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            തിരികെ nullptr;
          }
          FullPath = AbsFullPath;
        } അല്ലെങ്കിൽ {
          FullPath = മൂല്യം;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "use-external-name") {
        തർക്കം Val;
        എങ്കിൽ (!parseScalarBool(I.get_മൂല്യം(), Val))
          തിരികെ nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } അല്ലെങ്കിൽ {
        llvm_unreachable("key missing from Keys");
      }
    }

    എങ്കിൽ (Stream.failed())
      തിരികെ nullptr;

    // check for missing keys
    എങ്കിൽ (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      തിരികെ nullptr;
    }
    എങ്കിൽ (!check_Missing_താക്കോലുകൾ(N, താക്കോലുകൾ_2))
      തിരികെ nullptr;

    // check invalid configuration
    എങ്കിൽ (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      തിരികെ nullptr;
    }

    എങ്കിൽ (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      തിരികെ nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    എങ്കിൽ (Is_വേര്_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      എങ്കിൽ (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } അല്ലെങ്കിൽ എങ്കിൽ (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } അല്ലെങ്കിൽ {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        എങ്കിൽ (FS->വേര്_Relative ==
            RedirectingFileSystem::വേര്_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          പ്രതിജ്ഞാ(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } അല്ലെങ്കിൽ {
          EC = FS->makeAbsolute(Name);
        }
        എങ്കിൽ (EC) {
          പ്രതിജ്ഞാ(Name_മൂല്യം_കണ്ണി && "Name presence should be checked earlier");
          error(
              Name_മൂല്യം_കണ്ണി,
              "entry with relative path at the root level is not discoverable");
          തിരികെ nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      എങ്കിൽ (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t വേര്_Path_Len = sys::path::വേര്_path(Trimmed, path_style).size();
    ആകുമ്പോൾ (Trimmed.size() > വേര്_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    തിരഞ്ഞെടുപ്പ് (Kind) {
    അവസ്ഥ RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      നിർത്തുക;
    അവസ്ഥ RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      നിർത്തുക;
    അവസ്ഥ RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      നിർത്തുക;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    എങ്കിൽ (Parent.empty())
      തിരികെ Result;

    // if 'name' contains multiple components, create implicit directory entries
    വേണ്ടി (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.തള്ളുക_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    തിരികെ Result;
  }

പൊതു:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  തർക്കം parse(yaml::കണ്ണി *വേര്_2, RedirectingFileSystem *FS) {
    auto *മുകൾ = dyn_cast<yaml::Mapping_കണ്ണി>(വേര്_2);
    എങ്കിൽ (!മുകൾ) {
      error(വേര്_2, "expected mapping node");
      തിരികെ അസത്യം;
    }

    താക്കോൽ_Status_Pair Fields[] = {
        താക്കോൽ_Status_Pair("version", സത്യം),
        താക്കോൽ_Status_Pair("case-sensitive", അസത്യം),
        താക്കോൽ_Status_Pair("use-external-names", അസത്യം),
        താക്കോൽ_Status_Pair("root-relative", അസത്യം),
        താക്കോൽ_Status_Pair("overlay-relative", അസത്യം),
        താക്കോൽ_Status_Pair("fallthrough", അസത്യം),
        താക്കോൽ_Status_Pair("redirecting-with", അസത്യം),
        താക്കോൽ_Status_Pair("roots", സത്യം),
    };

    DenseMap<StringRef, താക്കോൽ_Status> താക്കോലുകൾ_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> വേര്_Entries;

    // Parse configuration and 'roots'
    വേണ്ടി (auto &I : *മുകൾ) {
      SmallString<10> താക്കോൽ_Buffer;
      StringRef താക്കോൽ;
      എങ്കിൽ (!parseScalarString(I.get_താക്കോൽ(), താക്കോൽ, താക്കോൽ_Buffer))
        തിരികെ അസത്യം;

      എങ്കിൽ (!check_Duplicate_Or_Unknown_താക്കോൽ(I.get_താക്കോൽ(), താക്കോൽ, താക്കോലുകൾ_2))
        തിരികെ അസത്യം;

      എങ്കിൽ (താക്കോൽ == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_കണ്ണി>(I.get_മൂല്യം());
        എങ്കിൽ (!Roots) {
          error(I.get_മൂല്യം(), "expected array");
          തിരികെ അസത്യം;
        }

        വേണ്ടി (auto &I : *Roots) {
          എങ്കിൽ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ സത്യം))
            വേര്_Entries.തള്ളുക_back(std::move(E));
          അല്ലെങ്കിൽ
            തിരികെ അസത്യം;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        എങ്കിൽ (!parseScalarString(I.get_മൂല്യം(), VersionString, Storage))
          തിരികെ അസത്യം;
        പൂർണ്ണസംഖ്യ Version;
        എങ്കിൽ (VersionString.getAsInteger<പൂർണ്ണസംഖ്യ>(10, Version)) {
          error(I.get_മൂല്യം(), "expected integer");
          തിരികെ അസത്യം;
        }
        എങ്കിൽ (Version < 0) {
          error(I.get_മൂല്യം(), "invalid version number");
          തിരികെ അസത്യം;
        }
        എങ്കിൽ (Version != 0) {
          error(I.get_മൂല്യം(), "version mismatch, expected 0");
          തിരികെ അസത്യം;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "case-sensitive") {
        എങ്കിൽ (!parseScalarBool(I.get_മൂല്യം(), FS->CaseSensitive))
          തിരികെ അസത്യം;
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "overlay-relative") {
        എങ്കിൽ (!parseScalarBool(I.get_മൂല്യം(), FS->IsRelativeOverlay))
          തിരികെ അസത്യം;
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "use-external-names") {
        എങ്കിൽ (!parseScalarBool(I.get_മൂല്യം(), FS->UseExternalNames))
          തിരികെ അസത്യം;
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "fallthrough") {
        എങ്കിൽ (താക്കോലുകൾ_2["redirecting-with"].Seen) {
          error(I.get_മൂല്യം(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          തിരികെ അസത്യം;
        }

        തർക്കം ShouldFallthrough = അസത്യം;
        എങ്കിൽ (!parseScalarBool(I.get_മൂല്യം(), ShouldFallthrough))
          തിരികെ അസത്യം;

        എങ്കിൽ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } അല്ലെങ്കിൽ {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "redirecting-with") {
        എങ്കിൽ (താക്കോലുകൾ_2["fallthrough"].Seen) {
          error(I.get_മൂല്യം(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          തിരികെ അസത്യം;
        }

        എങ്കിൽ (auto Kind = parseRedirectKind(I.get_മൂല്യം())) {
          FS->Redirection = *Kind;
        } അല്ലെങ്കിൽ {
          error(I.get_മൂല്യം(), "expected valid redirect kind");
          തിരികെ അസത്യം;
        }
      } അല്ലെങ്കിൽ എങ്കിൽ (താക്കോൽ == "root-relative") {
        എങ്കിൽ (auto Kind = parse_വേര്_Relative_Kind(I.get_മൂല്യം())) {
          FS->വേര്_Relative = *Kind;
        } അല്ലെങ്കിൽ {
          error(I.get_മൂല്യം(), "expected valid root-relative kind");
          തിരികെ അസത്യം;
        }
      } അല്ലെങ്കിൽ {
        llvm_unreachable("key missing from Keys");
      }
    }

    എങ്കിൽ (Stream.failed())
      തിരികെ അസത്യം;

    എങ്കിൽ (!check_Missing_താക്കോലുകൾ(മുകൾ, താക്കോലുകൾ_2))
      തിരികെ അസത്യം;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    വേണ്ടി (auto &E : വേര്_Entries)
      unique_Overlay_വൃക്ഷം(FS, E.get());

    തിരികെ സത്യം;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, ശൂന്യം *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::കണ്ണി *വേര്_2 = DI->get_വേര്();
  എങ്കിൽ (DI == Stream.end() || !വേര്_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    തിരികെ nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      പുതിയ RedirectingFileSystem(ExternalFS));

  എങ്കിൽ (!YAMLFilePath.empty()) {
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
    പ്രതിജ്ഞാ(!EC && "Overlay dir final path must be absolute");
    (ശൂന്യം)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  എങ്കിൽ (!P.parse(വേര്_2, FS.get()))
    തിരികെ nullptr;

  തിരികെ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    തർക്കം UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      പുതിയ RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  വേണ്ടി (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (ശൂന്യം)EC;
      പ്രതിജ്ഞാ(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    എങ്കിൽ (ToEntry)
      തുടരുക;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    വേണ്ടി (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    പ്രതിജ്ഞാ(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (ശൂന്യം)EC;
      പ്രതിജ്ഞാ(!EC && "Could not make absolute path");
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

  തിരികെ FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  പ്രതിജ്ഞാ(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  എങ്കിൽ (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

ശൂന്യം RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<അക്ഷരം> &Result) സ്ഥിരം {
  Result.clear();
  വേണ്ടി (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<അക്ഷരം> &Path) സ്ഥിരം {
  എങ്കിൽ (std::error_code EC = makeAbsolute(Path))
    തിരികെ EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  എങ്കിൽ (CanonicalPath.empty())
    തിരികെ make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  തിരികെ {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) സ്ഥിരം {
  llvm::SmallString<128> CanonicalPath(Path);
  എങ്കിൽ (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    തിരികെ EC;

  // RedirectOnly means the VFS is always used.
  എങ്കിൽ (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = സത്യം;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  വേണ്ടി (സ്ഥിരം auto &വേര്_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, വേര്_2.get(), Entries);
    എങ്കിൽ (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = സത്യം;
    എങ്കിൽ (Result) {
      Result->Parents = std::move(Entries);
      തിരികെ Result;
    }

    എങ്കിൽ (Result.getError() != llvm::errc::no_such_file_or_directory)
      തിരികെ Result;
  }
  തിരികെ make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) സ്ഥിരം {
  പ്രതിജ്ഞാ(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  എങ്കിൽ (!FromName.empty()) {
    എങ്കിൽ (!pathComponentMatches(*Start, FromName))
      തിരികെ make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    എങ്കിൽ (Start == End) {
      // Match!
      തിരികെ LookupResult(From, Start, End);
    }
  }

  എങ്കിൽ (isa<RedirectingFileSystem::FileEntry>(From))
    തിരികെ make_error_code(llvm::errc::not_a_directory);

  എങ്കിൽ (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    തിരികെ LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  വേണ്ടി (സ്ഥിരം std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.തള്ളുക_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    എങ്കിൽ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      തിരികെ Result;
    Entries.എടുക്കുക_back();
  }

  തിരികെ make_error_code(llvm::errc::no_such_file_or_directory);
}

സ്ഥിതം Status getRedirectedFileStatus(സ്ഥിരം Twine &OriginalPath,
                                      തർക്കം UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  എങ്കിൽ (ExternalStatus.ExposesExternalVFSPath)
    തിരികെ ExternalStatus;

  Status S = ExternalStatus;
  എങ്കിൽ (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  അല്ലെങ്കിൽ
    S.ExposesExternalVFSPath = സത്യം;
  തിരികെ S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    സ്ഥിരം Twine &LookupPath, സ്ഥിരം Twine &OriginalPath,
    സ്ഥിരം RedirectingFileSystem::LookupResult &Result) {
  എങ്കിൽ (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    എങ്കിൽ (std::error_code EC = makeAbsolute(RemappedPath))
      തിരികെ EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    എങ്കിൽ (!S)
      തിരികെ S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    തിരികെ getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  തിരികെ Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(സ്ഥിരം Twine &LookupPath,
                                         സ്ഥിരം Twine &OriginalPath) സ്ഥിരം {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  എങ്കിൽ (!Result || Result->ExposesExternalVFSPath)
    തിരികെ Result;
  തിരികെ Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(സ്ഥിരം Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  എങ്കിൽ (std::error_code EC = makeAbsolute(Path))
    തിരികെ EC;

  എങ്കിൽ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    എങ്കിൽ (S)
      തിരികെ S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  എങ്കിൽ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    എങ്കിൽ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      തിരികെ getExternalStatus(Path, OriginalPath);
    തിരികെ Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  എങ്കിൽ (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    തിരികെ getExternalStatus(Path, OriginalPath);
  }

  തിരികെ S;
}

തർക്കം RedirectingFileSystem::exists(സ്ഥിരം Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  എങ്കിൽ (makeAbsolute(Path))
    തിരികെ അസത്യം;

  എങ്കിൽ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    എങ്കിൽ (ExternalFS->exists(Path))
      തിരികെ സത്യം;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  എങ്കിൽ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    എങ്കിൽ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      തിരികെ ExternalFS->exists(Path);
    തിരികെ അസത്യം;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  എങ്കിൽ (!ExtRedirect) {
    പ്രതിജ്ഞാ(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    തിരികെ സത്യം;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  എങ്കിൽ (makeAbsolute(RemappedPath))
    തിരികെ അസത്യം;

  എങ്കിൽ (ExternalFS->exists(RemappedPath))
    തിരികെ സത്യം;

  എങ്കിൽ (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    തിരികെ ExternalFS->exists(Path);
  }

  തിരികെ അസത്യം;
}

നാമാകാശ {

/// Provide a file wrapper with an overriden status.
വർഗ്ഗം FileWithFixedStatus : പൊതു File {
  std::unique_ptr<File> InnerFile;
  Status S;

പൊതു:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { തിരികെ S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(സ്ഥിരം Twine &Name, int64_t FileSize, തർക്കം RequiresNullTerminator,
            തർക്കം IsVolatile) override {
    തിരികെ InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { തിരികെ InnerFile->close(); }

  ശൂന്യം setPath(സ്ഥിരം Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, സ്ഥിരം Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  എങ്കിൽ (!Result || (*Result)->status()->ExposesExternalVFSPath)
    തിരികെ Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  എങ്കിൽ (Name && Name.get() != P.str())
    F->get()->setPath(P);
  തിരികെ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(സ്ഥിരം Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  എങ്കിൽ (std::error_code EC = makeAbsolute(Path))
    തിരികെ EC;

  എങ്കിൽ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    എങ്കിൽ (F)
      തിരികെ F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  എങ്കിൽ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    എങ്കിൽ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      തിരികെ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    തിരികെ Result.getError();
  }

  എങ്കിൽ (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    തിരികെ make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  എങ്കിൽ (std::error_code EC = makeAbsolute(RemappedPath))
    തിരികെ EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  എങ്കിൽ (!ExternalFile) {
    എങ്കിൽ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      തിരികെ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    തിരികെ ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  എങ്കിൽ (!ExternalStatus)
    തിരികെ ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  തിരികെ std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(സ്ഥിരം Twine &OriginalPath,
                                   SmallVectorImpl<അക്ഷരം> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  എങ്കിൽ (std::error_code EC = makeAbsolute(Path))
    തിരികെ EC;

  എങ്കിൽ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    എങ്കിൽ (!EC)
      തിരികെ EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  എങ്കിൽ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    എങ്കിൽ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      തിരികെ ExternalFS->getRealPath(Path, Output);
    തിരികെ Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  എങ്കിൽ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    എങ്കിൽ (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      തിരികെ ExternalFS->getRealPath(Path, Output);
    }
    തിരികെ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  എങ്കിൽ (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    തിരികെ {};
  }
  തിരികെ llvm::errc::invalid_argument;
}

ശൂന്യം RedirectingFileSystem::getDirectoryContentRealSources(
    സ്ഥിരം Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  എങ്കിൽ (makeAbsolute(Path))
    തിരികെ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  സ്ഥിരം തർക്കം ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  എങ്കിൽ (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    എങ്കിൽ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    തിരികെ;
  }

  തിരഞ്ഞെടുപ്പ് (Result->E->getKind()) {
  അവസ്ഥ EK_File:
    തിരികെ;
  അവസ്ഥ EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    നിർത്തുക;
  അവസ്ഥ EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    എങ്കിൽ (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    നിർത്തുക;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  എങ്കിൽ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, ശൂന്യം *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  തിരികെ RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

സ്ഥിതം ശൂന്യം getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  എങ്കിൽ (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    പ്രതിജ്ഞാ(DE && "Must be a directory");
    വേണ്ടി (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.തള്ളുക_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.എടുക്കുക_back();
    }
    തിരികെ;
  }

  എങ്കിൽ (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    പ്രതിജ്ഞാ(DR && "Must be a directory remap");
    SmallString<128> VPath;
    വേണ്ടി (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.തള്ളുക_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    തിരികെ;
  }

  പ്രതിജ്ഞാ(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  പ്രതിജ്ഞാ(FE && "Must be a file");
  SmallString<128> VPath;
  വേണ്ടി (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.തള്ളുക_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

ശൂന്യം vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> വേര്_Result = VFS.lookupPath("/");
  എങ്കിൽ (!വേര്_Result)
    തിരികെ;
  SmallVector<StringRef, 8> Components;
  Components.തള്ളുക_back("/");
  getVFSEntries(വേര്_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  സ്ഥിതം std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  തിരികെ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

ശൂന്യം YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             തർക്കം IsDirectory) {
  പ്രതിജ്ഞാ(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  പ്രതിജ്ഞാ(sys::path::is_absolute(RealPath) && "real path not absolute");
  പ്രതിജ്ഞാ(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

ശൂന്യം YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/അസത്യം);
}

ശൂന്യം YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/സത്യം);
}

നാമാകാശ {

വർഗ്ഗം JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_അട്ടി;

  unsigned getDirIndent() { തിരികെ 4 * Dir_അട്ടി.size(); }
  unsigned getFileIndent() { തിരികെ 4 * (Dir_അട്ടി.size() + 1); }
  തർക്കം containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  ശൂന്യം startDirectory(StringRef Path);
  ശൂന്യം endDirectory();
  ശൂന്യം writeEntry(StringRef VPath, StringRef RPath);

പൊതു:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  ശൂന്യം ലിഖ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<തർക്കം> UseExternalNames,
             std::optional<തർക്കം> IsCaseSensitive,
             std::optional<തർക്കം> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

തർക്കം JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using നാമാകാശ llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  വേണ്ടി (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    എങ്കിൽ (*IParent != *IChild)
      തിരികെ അസത്യം;
  }
  // Have we exhausted the parent path?
  തിരികെ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  പ്രതിജ്ഞാ(!Parent.empty());
  പ്രതിജ്ഞാ(containedIn(Parent, Path));
  തിരികെ Path.substr(Parent.size() + 1);
}

ശൂന്യം JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_അട്ടി.empty() ? Path : containedPart(Dir_അട്ടി.back(), Path);
  Dir_അട്ടി.തള്ളുക_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

ശൂന്യം JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_അട്ടി.എടുക്കുക_back();
}

ശൂന്യം JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

ശൂന്യം JSONWriter::ലിഖ(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<തർക്കം> UseExternalNames,
                       std::optional<തർക്കം> IsCaseSensitive,
                       std::optional<തർക്കം> IsOverlayRelative,
                       StringRef OverlayDir) {
  using നാമാകാശ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  എങ്കിൽ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  എങ്കിൽ (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  തർക്കം UseOverlayRelative = അസത്യം;
  എങ്കിൽ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  എങ്കിൽ (!Entries.empty()) {
    സ്ഥിരം YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    എങ്കിൽ (UseOverlayRelative) {
      പ്രതിജ്ഞാ(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    തർക്കം IsCurrentDirEmpty = സത്യം;
    എങ്കിൽ (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = അസത്യം;
    }

    വേണ്ടി (സ്ഥിരം auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      എങ്കിൽ (Dir == Dir_അട്ടി.back()) {
        എങ്കിൽ (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } അല്ലെങ്കിൽ {
        തർക്കം Is_Dir_Popped_From_അട്ടി = അസത്യം;
        ആകുമ്പോൾ (!Dir_അട്ടി.empty() && !containedIn(Dir_അട്ടി.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_അട്ടി = സത്യം;
        }
        എങ്കിൽ (Is_Dir_Popped_From_അട്ടി || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = സത്യം;
      }
      StringRef RPath = Entry.RPath;
      എങ്കിൽ (UseOverlayRelative) {
        പ്രതിജ്ഞാ(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      എങ്കിൽ (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = അസത്യം;
      }
    }

    ആകുമ്പോൾ (!Dir_അട്ടി.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

ശൂന്യം YAMLVFSWriter::ലിഖ(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](സ്ഥിരം YAMLVFSEntry &LHS, സ്ഥിരം YAMLVFSEntry &RHS) {
    തിരികെ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).ലിഖ(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, സ്ഥിരം Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  എങ്കിൽ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->അട്ടി.തള്ളുക_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  പ്രതിജ്ഞാ(FS && State && !State->അട്ടി.empty() && "incrementing past end");
  പ്രതിജ്ഞാ(!State->അട്ടി.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  എങ്കിൽ (State->Has_No_തള്ളുക_Request)
    State->Has_No_തള്ളുക_Request = അസത്യം;
  അല്ലെങ്കിൽ {
    എങ്കിൽ (State->അട്ടി.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->അട്ടി.back()->path(), EC);
      എങ്കിൽ (I != End) {
        State->അട്ടി.തള്ളുക_back(I);
        തിരികെ *ഇത്;
      }
    }
  }

  ആകുമ്പോൾ (!State->അട്ടി.empty() && State->അട്ടി.back().increment(EC) == End)
    State->അട്ടി.എടുക്കുക_back();

  എങ്കിൽ (State->അട്ടി.empty())
    State.reset(); // end iterator

  തിരികെ *ഇത്;
}

സ്ഥിരം അക്ഷരം FileSystem::ID = 0;
സ്ഥിരം അക്ഷരം OverlayFileSystem::ID = 0;
സ്ഥിരം അക്ഷരം ProxyFileSystem::ID = 0;
സ്ഥിരം അക്ഷരം InMemoryFileSystem::ID = 0;
സ്ഥിരം അക്ഷരം RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_എണ്ണം(
    സ്ഥിരം FileSystem *FS) {
  തിരികെ FS->Use_എണ്ണം();
}

ശൂന്യം ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

ശൂന്യം ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
