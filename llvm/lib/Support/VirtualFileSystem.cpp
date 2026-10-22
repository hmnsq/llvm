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

using نطاق llvm;
using نطاق llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(ثابت file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(ثابت Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(ثابت Status &In, uint64_t NewSize) {
  ܦܢܐ Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ثابت Status &In, ثابت Twine &NewName) {
  ܦܢܐ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ثابت file_status &In, ثابت Twine &NewName) {
  ܦܢܐ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

منطقي Status::equivalent(ثابت Status &Other) ثابت {
  أكد(isStatusKnown() && Other.isStatusKnown());
  ܦܢܐ getUniqueID() == Other.getUniqueID();
}

منطقي Status::isDirectory() ثابت { ܦܢܐ Type == file_type::directory_file; }

منطقي Status::isRegularFile() ثابت { ܦܢܐ Type == file_type::regular_file; }

منطقي Status::isOther() ثابت {
  ܦܢܐ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

منطقي Status::isSymlink() ثابت { ܦܢܐ Type == file_type::symlink_file; }

منطقي Status::isStatusKnown() ثابت { ܦܢܐ Type != file_type::status_error; }

منطقي Status::exists() ثابت {
  ܦܢܐ isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = افتراضي;

FileSystem::~FileSystem() = افتراضي;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(ثابت llvm::Twine &Name, int64_t FileSize,
                             منطقي RequiresNullTerminator, منطقي IsVolatile,
                             منطقي IsText) {
  auto F = IsText ? openFileForRead(Name) : openFileForReadBinary(Name);
  ܐܢ (!F)
    ܦܢܐ F.getError();

  ܦܢܐ (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<حرف> &Path) ثابت {
  ܐܢ (llvm::sys::path::is_absolute(Path))
    ܦܢܐ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ܐܢ (!WorkingDir)
    ܦܢܐ WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  ܦܢܐ {};
}

std::error_code FileSystem::getRealPath(ثابت Twine &Path,
                                        SmallVectorImpl<حرف> &Output) {
  ܦܢܐ errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(ثابت Twine &Path, منطقي &Result) {
  ܦܢܐ errc::operation_not_permitted;
}

منطقي FileSystem::exists(ثابت Twine &Path) {
  auto Status = status(Path);
  ܦܢܐ Status && Status->exists();
}

llvm::ErrorOr<منطقي> FileSystem::equivalent(ثابت Twine &A, ثابت Twine &B) {
  auto StatusA = status(A);
  ܐܢ (!StatusA)
    ܦܢܐ StatusA.getError();
  auto StatusB = status(B);
  ܐܢ (!StatusB)
    ܦܢܐ StatusB.getError();
  ܦܢܐ StatusA->equivalent(*StatusB);
}

#ܐܢ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
فراغ FileSystem::dump() ثابت { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
ساكن منطقي isTraversalComponent(StringRef Component) {
  ܦܢܐ Component == ".." || Component == ".";
}

ساكن منطقي pathHasTraversal(StringRef Path) {
  using نطاق llvm::sys;

  لكل (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    ܐܢ (isTraversalComponent(Comp))
      ܦܢܐ ܫܪܪܐ;
  ܦܢܐ ܕܓܠܐ;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

/// Wrapper around a raw file descriptor.
صنف RealFile : عام File {
  friend صنف RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    أكد(FD.isValid() && "Invalid or inactive file descriptor");
  }

عام:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(ثابت Twine &Name,
                                                   int64_t FileSize,
                                                   منطقي RequiresNullTerminator,
                                                   منطقي IsVolatile) override;
  std::error_code close() override;
  فراغ setPath(ثابت Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot stat closed file");
  ܐܢ (!S.isStatusKnown()) {
    file_status RealStatus;
    ܐܢ (std::error_code EC = sys::fs::status(FD, RealStatus))
      ܦܢܐ EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  ܦܢܐ S;
}

ErrorOr<std::string> RealFile::getName() {
  ܦܢܐ RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(ثابت Twine &Name, int64_t FileSize,
                    منطقي RequiresNullTerminator, منطقي IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot get buffer for closed file");
  ܦܢܐ MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ܦܢܐ EC;
}

فراغ RealFile::setPath(ثابت Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  ܐܢ (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
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
  explicit RealFileSystem(منطقي LinkCWDToProcess) {
    ܐܢ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      ܐܢ (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      ܐܚܪܢܐ ܐܢ (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      ܐܚܪܢܐ
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(ثابت Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ثابت Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  openFileForReadBinary(ثابت Twine &Path) override;
  directory_iterator dir_begin(ثابت Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override;
  std::error_code setCurrentWorkingDirectory(ثابت Twine &Path) override;
  std::error_code isLocal(ثابت Twine &Path, منطقي &Result) override;
  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.push_back(Dir.str());
  }
  std::error_code getRealPath(ثابت Twine &Path,
                              SmallVectorImpl<حرف> &Output) override;

محمي:
  فراغ printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) ثابت override;

خاص:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(ثابت Twine &Path, SmallVectorImpl<حرف> &Storage) ثابت {
    ܐܢ (!WD || !*WD)
      ܦܢܐ Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    ܦܢܐ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ثابت Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    ܐܢ (!FDOrErr)
      ܦܢܐ errorToErrorCode(FDOrErr.takeError());
    ܦܢܐ std::unique_ptr<File>(
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

ErrorOr<Status> RealFileSystem::status(ثابت Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  ܐܢ (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    ܦܢܐ EC;
  ܦܢܐ Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ثابت Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ܦܢܐ openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForReadBinary(ثابت Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ܦܢܐ openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ثابت {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ܐܢ (WD && *WD)
    ܦܢܐ std::string(WD->get().Specified);
  ܐܢ (WD)
    ܦܢܐ WD->getError();

  SmallString<128> Dir;
  ܐܢ (std::error_code EC = llvm::sys::fs::current_path(Dir))
    ܦܢܐ EC;
  ܦܢܐ std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ܐܢ (!WD)
    ܦܢܐ llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  منطقي IsDir;
  ܐܢ (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ܦܢܐ Err;
  ܐܢ (!IsDir)
    ܦܢܐ std::make_error_code(std::errc::not_a_directory);
  ܐܢ (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    ܦܢܐ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ܦܢܐ std::error_code();
}

std::error_code RealFileSystem::isLocal(ثابت Twine &Path, منطقي &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ܦܢܐ llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(ثابت Twine &Path,
                                            SmallVectorImpl<حرف> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ܦܢܐ llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

فراغ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  ܐܢ (WD)
    OS << "own";
  ܐܚܪܢܐ
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  ساكن IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(ܫܪܪܐ);
  sys::sandbox::violationIfEnabled();

  ܦܢܐ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ܦܢܐ std::make_unique<RealFileSystem>(ܕܓܠܐ);
}

نطاق {

صنف RealFSDirIter : عام llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

عام:
  RealFSDirIter(ثابت Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    ܐܢ (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    ܦܢܐ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ثابت Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ܦܢܐ directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.push_back(std::move(BaseFS));
}

فراغ OverlayFileSystem::pushOverlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.push_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(ثابت Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    ܐܢ (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      ܦܢܐ Status;
  }
  ܦܢܐ make_error_code(llvm::errc::no_such_file_or_directory);
}

منطقي OverlayFileSystem::exists(ثابت Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ܐܢ ((*I)->exists(Path))
      ܦܢܐ ܫܪܪܐ;
  }
  ܦܢܐ ܕܓܠܐ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ثابت llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    ܐܢ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ܦܢܐ Result;
  }
  ܦܢܐ make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ثابت {
  // All file systems are synchronized, just take the first working directory.
  ܦܢܐ FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  لكل (auto &FS : FSList)
    ܐܢ (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      ܦܢܐ EC;
  ܦܢܐ {};
}

std::error_code OverlayFileSystem::isLocal(ثابت Twine &Path, منطقي &Result) {
  لكل (auto &FS : FSList)
    ܐܢ (FS->exists(Path))
      ܦܢܐ FS->isLocal(Path, Result);
  ܦܢܐ errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(ثابت Twine &Path,
                                               SmallVectorImpl<حرف> &Output) {
  لكل (ثابت auto &FS : FSList)
    ܐܢ (FS->exists(Path))
      ܦܢܐ FS->getRealPath(Path, Output);
  ܦܢܐ errc::no_such_file_or_directory;
}

فراغ OverlayFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  لكل (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

فراغ OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  لكل (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

فراغ OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  ܐܢ (Type == PrintType::Summary)
    ܦܢܐ;

  ܐܢ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  لكل (ثابت auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = افتراضي;

نطاق {

/// Combines and deduplicates directory entries across multiple file systems.
صنف CombiningDirIterImpl : عام llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(منطقي IsFirstTime) {
    ܟܡܐ (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.pop_back();
      ܐܢ (CurrentDirIter != directory_iterator())
        اكسر; // found
    }

    ܐܢ (IsFirstTime && CurrentDirIter == directory_iterator())
      ܦܢܐ errc::no_such_file_or_directory;
    ܦܢܐ {};
  }

  std::error_code incrementDirIter(منطقي IsFirstTime) {
    أكد((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    ܐܢ (!IsFirstTime)
      CurrentDirIter.increment(EC);
    ܐܢ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    ܦܢܐ EC;
  }

  std::error_code incrementImpl(منطقي IsFirstTime) {
    ܟܡܐ (ܫܪܪܐ) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      ܐܢ (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        ܦܢܐ EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      ܐܢ (SeenNames.insert(Name).second)
        ܦܢܐ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

عام:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    لكل (ثابت auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      ܐܢ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ܦܢܐ;
      }
      ܐܢ (!FEC)
        IterList.push_back(Iter);
    }
    EC = incrementImpl(ܫܪܪܐ);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(ܫܪܪܐ);
  }

  std::error_code increment() override { ܦܢܐ incrementImpl(ܕܓܠܐ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ثابت Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  ܐܢ (EC)
    ܦܢܐ {};
  ܦܢܐ Combined;
}

فراغ ProxyFileSystem::anchor() {}

نطاق llvm {
نطاق vfs {

نطاق detail {

تعداد InMemoryNodeKind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
صنف InMemoryNode {
  InMemoryNodeKind Kind;
  std::string FileName;

عام:
  InMemoryNode(llvm::StringRef FileName, InMemoryNodeKind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  virtual ~InMemoryNode() = افتراضي;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  virtual Status getStatus(ثابت Twine &RequestedName) ثابت = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ثابت { ܦܢܐ FileName; }
  InMemoryNodeKind getKind() ثابت { ܦܢܐ Kind; }
  virtual std::string toString(unsigned Indent) ثابت = 0;
};

صنف InMemoryFile : عام InMemoryNode {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

عام:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : InMemoryNode(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    ܦܢܐ Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() ثابت { ܦܢܐ Buffer.get(); }

  std::string toString(unsigned Indent) ثابت override {
    ܦܢܐ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ساكن منطقي classof(ثابت InMemoryNode *N) {
    ܦܢܐ N->getKind() == IME_File;
  }
};

نطاق {

صنف InMemoryHardLink : عام InMemoryNode {
  ثابت InMemoryFile &ResolvedFile;

عام:
  InMemoryHardLink(StringRef Path, ثابت InMemoryFile &ResolvedFile)
      : InMemoryNode(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ثابت InMemoryFile &getResolvedFile() ثابت { ܦܢܐ ResolvedFile; }

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    ܦܢܐ ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) ثابت override {
    ܦܢܐ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  ساكن منطقي classof(ثابت InMemoryNode *N) {
    ܦܢܐ N->getKind() == IME_HardLink;
  }
};

صنف InMemorySymbolicLink : عام InMemoryNode {
  std::string TargetPath;
  Status Stat;

عام:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : InMemoryNode(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) ثابت override {
    ܦܢܐ std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    ܦܢܐ Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() ثابت { ܦܢܐ TargetPath; }

  ساكن منطقي classof(ثابت InMemoryNode *N) {
    ܦܢܐ N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
صنف InMemoryFileAdaptor : عام File {
  ثابت InMemoryFile &Node;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

عام:
  explicit InMemoryFileAdaptor(ثابت InMemoryFile &Node,
                               std::string RequestedName)
      : Node(Node), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    ܦܢܐ Node.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(ثابت Twine &Name, int64_t FileSize, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    llvm::MemoryBuffer *Buf = Node.getBuffer();
    ܦܢܐ llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { ܦܢܐ {}; }

  فراغ setPath(ثابت Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

صنف InMemoryDirectory : عام InMemoryNode {
  Status Stat;
  std::map<std::string, std::unique_ptr<InMemoryNode>, std::less<>> Entries;

عام:
  InMemoryDirectory(Status Stat)
      : InMemoryNode(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    ܦܢܐ Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ثابت { ܦܢܐ Stat.getUniqueID(); }

  InMemoryNode *getChild(StringRef Name) ثابت {
    auto I = Entries.find(Name);
    ܐܢ (I != Entries.end())
      ܦܢܐ I->second.get();
    ܦܢܐ nullptr;
  }

  InMemoryNode *addChild(StringRef Name, std::unique_ptr<InMemoryNode> Child) {
    ܦܢܐ Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ثابت { ܦܢܐ Entries.begin(); }
  const_iterator end() ثابت { ܦܢܐ Entries.end(); }

  std::string toString(unsigned Indent) ثابت override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    لكل (ثابت auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    ܦܢܐ Result;
  }

  ساكن منطقي classof(ثابت InMemoryNode *N) {
    ܦܢܐ N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ساكن sys::fs::UniqueID getUniqueID(hash_code Hash) {
  ܦܢܐ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ساكن sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ܦܢܐ getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
ساكن sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  ܦܢܐ getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::NewInMemoryNodeInfo::makeStatus() ثابت {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  ܦܢܐ Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(منطقي UseNormalizedPaths)
    : Root(جديد detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = افتراضي;

std::string InMemoryFileSystem::toString() ثابت {
  ܦܢܐ Root->toString(/*Indent=*/0);
}

منطقي InMemoryFileSystem::addFile(ثابت Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 MakeNodeFn MakeNode) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (فراغ)EC;

  ܐܢ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ܫܪܪܐ);

  ܐܢ (Path.empty())
    ܦܢܐ ܕܓܠܐ;

  detail::InMemoryDirectory *Dir = Root.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  ثابت auto ResolvedUser = User.value_or(0);
  ثابت auto ResolvedGroup = Group.value_or(0);
  ثابت auto ResolvedType = Type.value_or(sys::fs::file_type::regular_file);
  ثابت auto ResolvedPerms = Perms.value_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ثابت auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  ܟܡܐ (ܫܪܪܐ) {
    Name = *I;
    ++I;
    ܐܢ (I == E)
      اكسر;
    detail::InMemoryNode *Node = Dir->getChild(Name);
    ܐܢ (!Node) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      استمر;
    }
    // Creating file under another file.
    ܐܢ (!isa<detail::InMemoryDirectory>(Node))
      ܦܢܐ ܕܓܠܐ;
    Dir = cast<detail::InMemoryDirectory>(Node);
  }
  detail::InMemoryNode *Node = Dir->getChild(Name);
  ܐܢ (!Node) {
    Dir->addChild(Name,
                  MakeNode({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    ܦܢܐ ܫܪܪܐ;
  }
  ܐܢ (isa<detail::InMemoryDirectory>(Node))
    ܦܢܐ ResolvedType == sys::fs::file_type::directory_file;

  أكد((isa<detail::InMemoryFile>(Node) ||
          isa<detail::InMemoryHardLink>(Node)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  ܐܢ (auto *Link = dyn_cast<detail::InMemoryHardLink>(Node)) {
    ܦܢܐ Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  ܦܢܐ cast<detail::InMemoryFile>(Node)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

منطقي InMemoryFileSystem::addFile(ثابت Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ܦܢܐ addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::NewInMemoryNodeInfo NNI)
                     -> std::unique_ptr<detail::InMemoryNode> {
                   Status Stat = NNI.makeStatus();
                   ܐܢ (Stat.getType() == sys::fs::file_type::directory_file)
                     ܦܢܐ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ܦܢܐ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

منطقي InMemoryFileSystem::addFileNoOwn(
    ثابت Twine &P, time_t ModificationTime,
    ثابت llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ܦܢܐ addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::NewInMemoryNodeInfo NNI)
                     -> std::unique_ptr<detail::InMemoryNode> {
                   Status Stat = NNI.makeStatus();
                   ܐܢ (Stat.getType() == sys::fs::file_type::directory_file)
                     ܦܢܐ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ܦܢܐ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::NamedNodeOrError
InMemoryFileSystem::lookupNode(ثابت Twine &P, منطقي FollowFinalSymlink,
                               size_t SymlinkDepth) ثابت {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (فراغ)EC;

  ܐܢ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ܫܪܪܐ);

  ثابت detail::InMemoryDirectory *Dir = Root.get();
  ܐܢ (Path.empty())
    ܦܢܐ detail::NamedNodeOrError(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  ܟܡܐ (ܫܪܪܐ) {
    detail::InMemoryNode *Node = Dir->getChild(*I);
    ++I;
    ܐܢ (!Node)
      ܦܢܐ errc::no_such_file_or_directory;

    ܐܢ (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(Node)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      ܐܢ (I == E && !FollowFinalSymlink)
        ܦܢܐ detail::NamedNodeOrError(Path, Symlink);

      ܐܢ (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        ܦܢܐ errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      ܐܢ (std::error_code EC = makeAbsolute(TargetPath))
        ܦܢܐ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookupNode(TargetPath, /*FollowFinalSymlink=*/ܫܪܪܐ, SymlinkDepth + 1);
      ܐܢ (!Target || I == E)
        ܦܢܐ Target;

      ܐܢ (!isa<detail::InMemoryDirectory>(*Target))
        ܦܢܐ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      استمر;
    }

    // Return the file if it's at the end of the path.
    ܐܢ (auto File = dyn_cast<detail::InMemoryFile>(Node)) {
      ܐܢ (I == E)
        ܦܢܐ detail::NamedNodeOrError(Path, File);
      ܦܢܐ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    ܐܢ (auto File = dyn_cast<detail::InMemoryHardLink>(Node)) {
      ܐܢ (I == E)
        ܦܢܐ detail::NamedNodeOrError(Path, &File->getResolvedFile());
      ܦܢܐ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(Node);
    ܐܢ (I == E)
      ܦܢܐ detail::NamedNodeOrError(Path, Dir);
  }
}

منطقي InMemoryFileSystem::addHardLink(ثابت Twine &NewLink,
                                     ثابت Twine &Target) {
  auto NewLinkNode = lookupNode(NewLink, /*FollowFinalSymlink=*/ܕܓܠܐ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto TargetNode = lookupNode(Target, /*FollowFinalSymlink=*/ܫܪܪܐ);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  ܐܢ (!TargetNode || NewLinkNode || !isa<detail::InMemoryFile>(*TargetNode))
    ܦܢܐ ܕܓܠܐ;
  ܦܢܐ addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::NewInMemoryNodeInfo NNI) {
                   ܦܢܐ std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*TargetNode));
                 });
}

منطقي InMemoryFileSystem::addSymbolicLink(
    ثابت Twine &NewLink, ثابت Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto NewLinkNode = lookupNode(NewLink, /*FollowFinalSymlink=*/ܕܓܠܐ);
  ܐܢ (NewLinkNode)
    ܦܢܐ ܕܓܠܐ;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  ܦܢܐ addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::NewInMemoryNodeInfo NNI) {
                   ܦܢܐ std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(ثابت Twine &Path) {
  auto Node = lookupNode(Path, /*FollowFinalSymlink=*/ܫܪܪܐ);
  ܐܢ (Node)
    ܦܢܐ (*Node)->getStatus(Path);
  ܦܢܐ Node.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(ثابت Twine &Path) {
  auto Node = lookupNode(Path,/*FollowFinalSymlink=*/ܫܪܪܐ);
  ܐܢ (!Node)
    ܦܢܐ Node.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  ܐܢ (auto *F = dyn_cast<detail::InMemoryFile>(*Node))
    ܦܢܐ std::unique_ptr<File>(
        جديد detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  ܦܢܐ make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
صنف InMemoryFileSystem::DirIterator : عام llvm::vfs::detail::DirIterImpl {
  ثابت InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  فراغ setCurrentEntry() {
    ܐܢ (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل (I->second->getKind()) {
      حالة detail::IME_File:
      حالة detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      حالة detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة detail::IME_SymbolicLink:
        ܐܢ (auto SymlinkTarget =
                FS->lookupNode(Path, /*FollowFinalSymlink=*/ܫܪܪܐ)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        اكسر;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } ܐܚܪܢܐ {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

عام:
  DirIterator() = افتراضي;

  DirIterator(ثابت InMemoryFileSystem *FS,
              ثابت detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    ܦܢܐ {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(ثابت Twine &Dir,
                                                 std::error_code &EC) {
  auto Node = lookupNode(Dir, /*FollowFinalSymlink=*/ܫܪܪܐ);
  ܐܢ (!Node) {
    EC = Node.getError();
    ܦܢܐ directory_iterator(std::make_shared<DirIterator>());
  }

  ܐܢ (auto *DirNode = dyn_cast<detail::InMemoryDirectory>(*Node))
    ܦܢܐ directory_iterator(
        std::make_shared<DirIterator>(هذا, *DirNode, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  ܦܢܐ directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(ثابت Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (فراغ)EC;

  ܐܢ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/ܫܪܪܐ);

  ܐܢ (!Path.empty())
    WorkingDirectory = std::string(Path);
  ܦܢܐ {};
}

std::error_code InMemoryFileSystem::getRealPath(ثابت Twine &Path,
                                                SmallVectorImpl<حرف> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  ܐܢ (!CWD || CWD->empty())
    ܦܢܐ errc::operation_not_permitted;
  Path.toVector(Output);
  ܐܢ (auto EC = makeAbsolute(Output))
    ܦܢܐ EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/ܫܪܪܐ);
  ܦܢܐ {};
}

std::error_code InMemoryFileSystem::isLocal(ثابت Twine &Path, منطقي &Result) {
  Result = ܕܓܠܐ;
  ܦܢܐ {};
}

فراغ InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

نطاق {

ساكن llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  ثابت size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  ܐܢ (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  ܦܢܐ style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ساكن llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/ܫܪܪܐ, style);
  ܦܢܐ result;
}

/// Whether the error and entry specify a file/directory that was not found.
ساكن منطقي isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  ܐܢ (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    ܦܢܐ ܕܓܠܐ;
  ܦܢܐ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  أكد(ExternalFS && "RedirectingFileSystem requires an external FS");
  ܐܢ (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
صنف llvm::vfs::RedirectingFSDirIterImpl
    : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(منطقي IsFirstTime) {
    أكد((IsFirstTime || Current != End) && "cannot iterate past end");
    ܐܢ (!IsFirstTime)
      ++Current;
    ܐܢ (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      بدل ((*Current)->getKind()) {
      حالة RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      حالة RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        اكسر;
      حالة RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        اكسر;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } ܐܚܪܢܐ {
      CurrentEntry = directory_entry();
    }
    ܦܢܐ {};
  };

عام:
  RedirectingFSDirIterImpl(
      ثابت Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/ܫܪܪܐ);
  }

  std::error_code increment() override {
    ܦܢܐ incrementImpl(/*IsFirstTime=*/ܕܓܠܐ);
  }
};

نطاق {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
صنف RedirectingFSDirRemapIterImpl : عام llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

عام:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    ܐܢ (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  فراغ setCurrentEntry() {
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
    ܐܢ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    ܐܚܪܢܐ
      CurrentEntry = directory_entry();
    ܦܢܐ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ثابت {
  ܦܢܐ WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  ܐܢ (!exists(Path))
    ܦܢܐ errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  ܐܢ (std::error_code EC = makeAbsolute(AbsolutePath))
    ܦܢܐ EC;
  WorkingDirectory = std::string(AbsolutePath);
  ܦܢܐ {};
}

std::error_code RedirectingFileSystem::isLocal(ثابت Twine &Path_,
                                               منطقي &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  ܐܢ (makeAbsolute(Path))
    ܦܢܐ {};

  ܦܢܐ ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<حرف> &Path) ثابت {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  ܐܢ (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ܦܢܐ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ܐܢ (!WorkingDir)
    ܦܢܐ WorkingDir.getError();

  ܦܢܐ makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<حرف> &Path) ثابت {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  ܐܢ (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    ܦܢܐ std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  ܐܢ (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } ܐܚܪܢܐ {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    ܐܢ (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  ܐܢ (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  ܦܢܐ {};
}

directory_iterator RedirectingFileSystem::dir_begin(ثابت Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  ܐܢ (EC)
    ܦܢܐ {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ܐܢ (!Result) {
    ܐܢ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      ܦܢܐ ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    ܦܢܐ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  ܐܢ (!S) {
    ܐܢ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      ܦܢܐ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ܦܢܐ {};
  }

  ܐܢ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ܦܢܐ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  ܐܢ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    ܐܢ (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } ܐܚܪܢܐ {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  ܐܢ (RedirectEC) {
    ܐܢ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ܦܢܐ {};
    }
    RedirectIter = {};
  }

  ܐܢ (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    ܦܢܐ RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  ܐܢ (ExternalEC) {
    ܐܢ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ܦܢܐ {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  بدل (Redirection) {
  حالة RedirectKind::Fallthrough:
    Iters.push_back(ExternalIter);
    Iters.push_back(RedirectIter);
    اكسر;
  حالة RedirectKind::Fallback:
    Iters.push_back(RedirectIter);
    Iters.push_back(ExternalIter);
    اكسر;
  افتراضي:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  ܐܢ (EC)
    ܦܢܐ {};
  ܦܢܐ Combined;
}

فراغ RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ثابت {
  ܦܢܐ OverlayFileDir;
}

فراغ RedirectingFileSystem::setFallthrough(منطقي Fallthrough) {
  ܐܢ (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } ܐܚܪܢܐ {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

فراغ RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() ثابت {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  لكل (ثابت auto &Root : Roots)
    R.push_back(Root->getName());
  ܦܢܐ R;
}

فراغ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  ܐܢ (Type == PrintType::Summary)
    ܦܢܐ;

  لكل (ثابت auto &Root : Roots)
    printEntry(OS, Root.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

فراغ RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  بدل (E->getKind()) {
  حالة EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    لكل (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    اكسر;
  }
  حالة EK_DirectoryRemap:
  حالة EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    بدل (RE->getUseName()) {
    حالة NK_NotSet:
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

فراغ RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  ܐܢ (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
صنف llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  فراغ error(yaml::Node *N, ثابت Twine &Msg) { Stream.printError(N, Msg); }

  // false on error
  منطقي parseScalarString(yaml::Node *N, StringRef &Result,
                         SmallVectorImpl<حرف> &Storage) {
    ثابت auto *S = dyn_cast_if_present<yaml::ScalarNode>(N);

    ܐܢ (!S) {
      error(N, "expected string");
      ܦܢܐ ܕܓܠܐ;
    }
    Result = S->getValue(Storage);
    ܦܢܐ ܫܪܪܐ;
  }

  // false on error
  منطقي parseScalarBool(yaml::Node *N, منطقي &Result) {
    SmallString<5> Storage;
    StringRef Value;
    ܐܢ (!parseScalarString(N, Value, Storage))
      ܦܢܐ ܕܓܠܐ;

    ܐܢ (Value.equals_insensitive("true") || Value.equals_insensitive("on") ||
        Value.equals_insensitive("yes") || Value == "1") {
      Result = ܫܪܪܐ;
      ܦܢܐ ܫܪܪܐ;
    } ܐܚܪܢܐ ܐܢ (Value.equals_insensitive("false") ||
               Value.equals_insensitive("off") ||
               Value.equals_insensitive("no") || Value == "0") {
      Result = ܕܓܠܐ;
      ܦܢܐ ܫܪܪܐ;
    }

    error(N, "expected boolean value");
    ܦܢܐ ܕܓܠܐ;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::Node *N) {
    SmallString<12> Storage;
    StringRef Value;
    ܐܢ (!parseScalarString(N, Value, Storage))
      ܦܢܐ std::nullopt;

    ܐܢ (Value.equals_insensitive("fallthrough")) {
      ܦܢܐ RedirectingFileSystem::RedirectKind::Fallthrough;
    } ܐܚܪܢܐ ܐܢ (Value.equals_insensitive("fallback")) {
      ܦܢܐ RedirectingFileSystem::RedirectKind::Fallback;
    } ܐܚܪܢܐ ܐܢ (Value.equals_insensitive("redirect-only")) {
      ܦܢܐ RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    ܦܢܐ std::nullopt;
  }

  std::optional<RedirectingFileSystem::RootRelativeKind>
  parseRootRelativeKind(yaml::Node *N) {
    SmallString<12> Storage;
    StringRef Value;
    ܐܢ (!parseScalarString(N, Value, Storage))
      ܦܢܐ std::nullopt;
    ܐܢ (Value.equals_insensitive("cwd")) {
      ܦܢܐ RedirectingFileSystem::RootRelativeKind::CWD;
    } ܐܚܪܢܐ ܐܢ (Value.equals_insensitive("overlay-dir")) {
      ܦܢܐ RedirectingFileSystem::RootRelativeKind::OverlayDir;
    }
    ܦܢܐ std::nullopt;
  }

  بنية KeyStatus {
    منطقي Required;
    منطقي Seen = ܕܓܠܐ;

    KeyStatus(منطقي Required = ܕܓܠܐ) : Required(Required) {}
  };

  using KeyStatusPair = std::pair<StringRef, KeyStatus>;

  // false on error
  منطقي checkDuplicateOrUnknownKey(yaml::Node *KeyNode, StringRef Key,
                                  DenseMap<StringRef, KeyStatus> &Keys) {
    auto It = Keys.find(Key);
    ܐܢ (It == Keys.end()) {
      error(KeyNode, "unknown key");
      ܦܢܐ ܕܓܠܐ;
    }
    KeyStatus &S = It->second;
    ܐܢ (S.Seen) {
      error(KeyNode, Twine("duplicate key '") + Key + "'");
      ܦܢܐ ܕܓܠܐ;
    }
    S.Seen = ܫܪܪܐ;
    ܦܢܐ ܫܪܪܐ;
  }

  // false on error
  منطقي checkMissingKeys(yaml::Node *Obj, DenseMap<StringRef, KeyStatus> &Keys) {
    لكل (ثابت auto &I : Keys) {
      ܐܢ (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        ܦܢܐ ܕܓܠܐ;
      }
    }
    ܦܢܐ ܫܪܪܐ;
  }

عام:
  ساكن RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    ܐܢ (!ParentEntry) { // Look for a existent root
      لكل (ثابت auto &Root : FS->Roots) {
        ܐܢ (Name == Root->getName()) {
          ParentEntry = Root.get();
          ܦܢܐ ParentEntry;
        }
      }
    } ܐܚܪܢܐ { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      لكل (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        ܐܢ (DirContent && Name == Content->getName())
          ܦܢܐ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    ܐܢ (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.push_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      ܦܢܐ ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    ܦܢܐ DE->getLastContent();
  }

خاص:
  فراغ uniqueOverlayTree(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    بدل (SrcE->getKind()) {
    حالة RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      ܐܢ (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      لكل (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        uniqueOverlayTree(FS, SubEntry.get(), NewParentE);
      اكسر;
    }
    حالة RedirectingFileSystem::EK_DirectoryRemap: {
      أكد(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      اكسر;
    }
    حالة RedirectingFileSystem::EK_File: {
      أكد(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      اكسر;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::Node *N, RedirectingFileSystem *FS, منطقي IsRootEntry) {
    auto *M = dyn_cast<yaml::MappingNode>(N);
    ܐܢ (!M) {
      error(N, "expected mapping node for file or directory entry");
      ܦܢܐ nullptr;
    }

    KeyStatusPair Fields[] = {
        KeyStatusPair("name", ܫܪܪܐ),
        KeyStatusPair("type", ܫܪܪܐ),
        KeyStatusPair("contents", ܕܓܠܐ),
        KeyStatusPair("external-contents", ܕܓܠܐ),
        KeyStatusPair("use-external-name", ܕܓܠܐ),
    };

    DenseMap<StringRef, KeyStatus> Keys(std::begin(Fields), std::end(Fields));

    تعداد { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::Node *NameValueNode = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    لكل (auto &I : *M) {
      StringRef Key;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      ܐܢ (!parseScalarString(I.getKey(), Key, Buffer))
        ܦܢܐ nullptr;

      ܐܢ (!checkDuplicateOrUnknownKey(I.getKey(), Key, Keys))
        ܦܢܐ nullptr;

      StringRef Value;
      ܐܢ (Key == "name") {
        ܐܢ (!parseScalarString(I.getValue(), Value, Buffer))
          ܦܢܐ nullptr;

        NameValueNode = I.getValue();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(Value).str();
      } ܐܚܪܢܐ ܐܢ (Key == "type") {
        ܐܢ (!parseScalarString(I.getValue(), Value, Buffer))
          ܦܢܐ nullptr;
        ܐܢ (Value == "file")
          Kind = RedirectingFileSystem::EK_File;
        ܐܚܪܢܐ ܐܢ (Value == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        ܐܚܪܢܐ ܐܢ (Value == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        ܐܚܪܢܐ {
          error(I.getValue(), "unknown value for 'type'");
          ܦܢܐ nullptr;
        }
      } ܐܚܪܢܐ ܐܢ (Key == "contents") {
        ܐܢ (ContentsField != CF_NotSet) {
          error(I.getKey(),
                "entry already has 'contents' or 'external-contents'");
          ܦܢܐ nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::SequenceNode>(I.getValue());
        ܐܢ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.getValue(), "expected array");
          ܦܢܐ nullptr;
        }

        لكل (auto &I : *Contents) {
          ܐܢ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ ܕܓܠܐ))
            EntryArrayContents.push_back(std::move(E));
          ܐܚܪܢܐ
            ܦܢܐ nullptr;
        }
      } ܐܚܪܢܐ ܐܢ (Key == "external-contents") {
        ܐܢ (ContentsField != CF_NotSet) {
          error(I.getKey(),
                "entry already has 'contents' or 'external-contents'");
          ܦܢܐ nullptr;
        }
        ContentsField = CF_External;
        ܐܢ (!parseScalarString(I.getValue(), Value, Buffer))
          ܦܢܐ nullptr;

        SmallString<256> FullPath;
        ܐܢ (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          أكد(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = Value;
          ܐܢ (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            ܦܢܐ nullptr;
          }
          FullPath = AbsFullPath;
        } ܐܚܪܢܐ {
          FullPath = Value;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } ܐܚܪܢܐ ܐܢ (Key == "use-external-name") {
        منطقي Val;
        ܐܢ (!parseScalarBool(I.getValue(), Val))
          ܦܢܐ nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } ܐܚܪܢܐ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ܐܢ (Stream.failed())
      ܦܢܐ nullptr;

    // check for missing keys
    ܐܢ (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      ܦܢܐ nullptr;
    }
    ܐܢ (!checkMissingKeys(N, Keys))
      ܦܢܐ nullptr;

    // check invalid configuration
    ܐܢ (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ܦܢܐ nullptr;
    }

    ܐܢ (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ܦܢܐ nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    ܐܢ (IsRootEntry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      ܐܢ (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } ܐܚܪܢܐ ܐܢ (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } ܐܚܪܢܐ {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        ܐܢ (FS->RootRelative ==
            RedirectingFileSystem::RootRelativeKind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          أكد(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } ܐܚܪܢܐ {
          EC = FS->makeAbsolute(Name);
        }
        ܐܢ (EC) {
          أكد(NameValueNode && "Name presence should be checked earlier");
          error(
              NameValueNode,
              "entry with relative path at the root level is not discoverable");
          ܦܢܐ nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      ܐܢ (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t RootPathLen = sys::path::root_path(Trimmed, path_style).size();
    ܟܡܐ (Trimmed.size() > RootPathLen &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    بدل (Kind) {
    حالة RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      اكسر;
    حالة RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      اكسر;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    ܐܢ (Parent.empty())
      ܦܢܐ Result;

    // if 'name' contains multiple components, create implicit directory entries
    لكل (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.push_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ܦܢܐ Result;
  }

عام:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  منطقي parse(yaml::Node *Root, RedirectingFileSystem *FS) {
    auto *Top = dyn_cast<yaml::MappingNode>(Root);
    ܐܢ (!Top) {
      error(Root, "expected mapping node");
      ܦܢܐ ܕܓܠܐ;
    }

    KeyStatusPair Fields[] = {
        KeyStatusPair("version", ܫܪܪܐ),
        KeyStatusPair("case-sensitive", ܕܓܠܐ),
        KeyStatusPair("use-external-names", ܕܓܠܐ),
        KeyStatusPair("root-relative", ܕܓܠܐ),
        KeyStatusPair("overlay-relative", ܕܓܠܐ),
        KeyStatusPair("fallthrough", ܕܓܠܐ),
        KeyStatusPair("redirecting-with", ܕܓܠܐ),
        KeyStatusPair("roots", ܫܪܪܐ),
    };

    DenseMap<StringRef, KeyStatus> Keys(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> RootEntries;

    // Parse configuration and 'roots'
    لكل (auto &I : *Top) {
      SmallString<10> KeyBuffer;
      StringRef Key;
      ܐܢ (!parseScalarString(I.getKey(), Key, KeyBuffer))
        ܦܢܐ ܕܓܠܐ;

      ܐܢ (!checkDuplicateOrUnknownKey(I.getKey(), Key, Keys))
        ܦܢܐ ܕܓܠܐ;

      ܐܢ (Key == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::SequenceNode>(I.getValue());
        ܐܢ (!Roots) {
          error(I.getValue(), "expected array");
          ܦܢܐ ܕܓܠܐ;
        }

        لكل (auto &I : *Roots) {
          ܐܢ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ ܫܪܪܐ))
            RootEntries.push_back(std::move(E));
          ܐܚܪܢܐ
            ܦܢܐ ܕܓܠܐ;
        }
      } ܐܚܪܢܐ ܐܢ (Key == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        ܐܢ (!parseScalarString(I.getValue(), VersionString, Storage))
          ܦܢܐ ܕܓܠܐ;
        صحيح Version;
        ܐܢ (VersionString.getAsInteger<صحيح>(10, Version)) {
          error(I.getValue(), "expected integer");
          ܦܢܐ ܕܓܠܐ;
        }
        ܐܢ (Version < 0) {
          error(I.getValue(), "invalid version number");
          ܦܢܐ ܕܓܠܐ;
        }
        ܐܢ (Version != 0) {
          error(I.getValue(), "version mismatch, expected 0");
          ܦܢܐ ܕܓܠܐ;
        }
      } ܐܚܪܢܐ ܐܢ (Key == "case-sensitive") {
        ܐܢ (!parseScalarBool(I.getValue(), FS->CaseSensitive))
          ܦܢܐ ܕܓܠܐ;
      } ܐܚܪܢܐ ܐܢ (Key == "overlay-relative") {
        ܐܢ (!parseScalarBool(I.getValue(), FS->IsRelativeOverlay))
          ܦܢܐ ܕܓܠܐ;
      } ܐܚܪܢܐ ܐܢ (Key == "use-external-names") {
        ܐܢ (!parseScalarBool(I.getValue(), FS->UseExternalNames))
          ܦܢܐ ܕܓܠܐ;
      } ܐܚܪܢܐ ܐܢ (Key == "fallthrough") {
        ܐܢ (Keys["redirecting-with"].Seen) {
          error(I.getValue(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ܦܢܐ ܕܓܠܐ;
        }

        منطقي ShouldFallthrough = ܕܓܠܐ;
        ܐܢ (!parseScalarBool(I.getValue(), ShouldFallthrough))
          ܦܢܐ ܕܓܠܐ;

        ܐܢ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } ܐܚܪܢܐ {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } ܐܚܪܢܐ ܐܢ (Key == "redirecting-with") {
        ܐܢ (Keys["fallthrough"].Seen) {
          error(I.getValue(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ܦܢܐ ܕܓܠܐ;
        }

        ܐܢ (auto Kind = parseRedirectKind(I.getValue())) {
          FS->Redirection = *Kind;
        } ܐܚܪܢܐ {
          error(I.getValue(), "expected valid redirect kind");
          ܦܢܐ ܕܓܠܐ;
        }
      } ܐܚܪܢܐ ܐܢ (Key == "root-relative") {
        ܐܢ (auto Kind = parseRootRelativeKind(I.getValue())) {
          FS->RootRelative = *Kind;
        } ܐܚܪܢܐ {
          error(I.getValue(), "expected valid root-relative kind");
          ܦܢܐ ܕܓܠܐ;
        }
      } ܐܚܪܢܐ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ܐܢ (Stream.failed())
      ܦܢܐ ܕܓܠܐ;

    ܐܢ (!checkMissingKeys(Top, Keys))
      ܦܢܐ ܕܓܠܐ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    لكل (auto &E : RootEntries)
      uniqueOverlayTree(FS, E.get());

    ܦܢܐ ܫܪܪܐ;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, فراغ *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::Node *Root = DI->getRoot();
  ܐܢ (DI == Stream.end() || !Root) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    ܦܢܐ nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));

  ܐܢ (!YAMLFilePath.empty()) {
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
    أكد(!EC && "Overlay dir final path must be absolute");
    (فراغ)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  ܐܢ (!P.parse(Root, FS.get()))
    ܦܢܐ nullptr;

  ܦܢܐ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    منطقي UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  لكل (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    ܐܢ (ToEntry)
      استمر;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    لكل (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    أكد(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (فراغ)EC;
      أكد(!EC && "Could not make absolute path");
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

  ܦܢܐ FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  أكد(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  ܐܢ (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

فراغ RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<حرف> &Result) ثابت {
  Result.clear();
  لكل (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<حرف> &Path) ثابت {
  ܐܢ (std::error_code EC = makeAbsolute(Path))
    ܦܢܐ EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  ܐܢ (CanonicalPath.empty())
    ܦܢܐ make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  ܦܢܐ {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) ثابت {
  llvm::SmallString<128> CanonicalPath(Path);
  ܐܢ (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    ܦܢܐ EC;

  // RedirectOnly means the VFS is always used.
  ܐܢ (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = ܫܪܪܐ;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  لكل (ثابت auto &Root : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, Root.get(), Entries);
    ܐܢ (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = ܫܪܪܐ;
    ܐܢ (Result) {
      Result->Parents = std::move(Entries);
      ܦܢܐ Result;
    }

    ܐܢ (Result.getError() != llvm::errc::no_such_file_or_directory)
      ܦܢܐ Result;
  }
  ܦܢܐ make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) ثابت {
  أكد(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  ܐܢ (!FromName.empty()) {
    ܐܢ (!pathComponentMatches(*Start, FromName))
      ܦܢܐ make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    ܐܢ (Start == End) {
      // Match!
      ܦܢܐ LookupResult(From, Start, End);
    }
  }

  ܐܢ (isa<RedirectingFileSystem::FileEntry>(From))
    ܦܢܐ make_error_code(llvm::errc::not_a_directory);

  ܐܢ (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    ܦܢܐ LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  لكل (ثابت std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.push_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    ܐܢ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ܦܢܐ Result;
    Entries.pop_back();
  }

  ܦܢܐ make_error_code(llvm::errc::no_such_file_or_directory);
}

ساكن Status getRedirectedFileStatus(ثابت Twine &OriginalPath,
                                      منطقي UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  ܐܢ (ExternalStatus.ExposesExternalVFSPath)
    ܦܢܐ ExternalStatus;

  Status S = ExternalStatus;
  ܐܢ (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  ܐܚܪܢܐ
    S.ExposesExternalVFSPath = ܫܪܪܐ;
  ܦܢܐ S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    ثابت Twine &LookupPath, ثابت Twine &OriginalPath,
    ثابت RedirectingFileSystem::LookupResult &Result) {
  ܐܢ (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    ܐܢ (std::error_code EC = makeAbsolute(RemappedPath))
      ܦܢܐ EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    ܐܢ (!S)
      ܦܢܐ S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    ܦܢܐ getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  ܦܢܐ Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(ثابت Twine &LookupPath,
                                         ثابت Twine &OriginalPath) ثابت {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  ܐܢ (!Result || Result->ExposesExternalVFSPath)
    ܦܢܐ Result;
  ܦܢܐ Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ܐܢ (std::error_code EC = makeAbsolute(Path))
    ܦܢܐ EC;

  ܐܢ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    ܐܢ (S)
      ܦܢܐ S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ܐܢ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ܐܢ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ܦܢܐ getExternalStatus(Path, OriginalPath);
    ܦܢܐ Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  ܐܢ (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ܦܢܐ getExternalStatus(Path, OriginalPath);
  }

  ܦܢܐ S;
}

منطقي RedirectingFileSystem::exists(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ܐܢ (makeAbsolute(Path))
    ܦܢܐ ܕܓܠܐ;

  ܐܢ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ܐܢ (ExternalFS->exists(Path))
      ܦܢܐ ܫܪܪܐ;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ܐܢ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ܐܢ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ܦܢܐ ExternalFS->exists(Path);
    ܦܢܐ ܕܓܠܐ;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  ܐܢ (!ExtRedirect) {
    أكد(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    ܦܢܐ ܫܪܪܐ;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  ܐܢ (makeAbsolute(RemappedPath))
    ܦܢܐ ܕܓܠܐ;

  ܐܢ (ExternalFS->exists(RemappedPath))
    ܦܢܐ ܫܪܪܐ;

  ܐܢ (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ܦܢܐ ExternalFS->exists(Path);
  }

  ܦܢܐ ܕܓܠܐ;
}

نطاق {

/// Provide a file wrapper with an overriden status.
صنف FileWithFixedStatus : عام File {
  std::unique_ptr<File> InnerFile;
  Status S;

عام:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { ܦܢܐ S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(ثابت Twine &Name, int64_t FileSize, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    ܦܢܐ InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { ܦܢܐ InnerFile->close(); }

  فراغ setPath(ثابت Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, ثابت Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  ܐܢ (!Result || (*Result)->status()->ExposesExternalVFSPath)
    ܦܢܐ Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  ܐܢ (Name && Name.get() != P.str())
    F->get()->setPath(P);
  ܦܢܐ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ܐܢ (std::error_code EC = makeAbsolute(Path))
    ܦܢܐ EC;

  ܐܢ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ܐܢ (F)
      ܦܢܐ F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ܐܢ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ܐܢ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ܦܢܐ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ܦܢܐ Result.getError();
  }

  ܐܢ (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    ܦܢܐ make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  ܐܢ (std::error_code EC = makeAbsolute(RemappedPath))
    ܦܢܐ EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  ܐܢ (!ExternalFile) {
    ܐܢ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ܦܢܐ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    ܦܢܐ ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  ܐܢ (!ExternalStatus)
    ܦܢܐ ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  ܦܢܐ std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(ثابت Twine &OriginalPath,
                                   SmallVectorImpl<حرف> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ܐܢ (std::error_code EC = makeAbsolute(Path))
    ܦܢܐ EC;

  ܐܢ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    ܐܢ (!EC)
      ܦܢܐ EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ܐܢ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ܐܢ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ܦܢܐ ExternalFS->getRealPath(Path, Output);
    ܦܢܐ Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  ܐܢ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    ܐܢ (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ܦܢܐ ExternalFS->getRealPath(Path, Output);
    }
    ܦܢܐ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  ܐܢ (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    ܦܢܐ {};
  }
  ܦܢܐ llvm::errc::invalid_argument;
}

فراغ RedirectingFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  ܐܢ (makeAbsolute(Path))
    ܦܢܐ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ثابت منطقي ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ܐܢ (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    ܐܢ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    ܦܢܐ;
  }

  بدل (Result->E->getKind()) {
  حالة EK_File:
    ܦܢܐ;
  حالة EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    اكسر;
  حالة EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    ܐܢ (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    اكسر;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  ܐܢ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, فراغ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ܦܢܐ RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

ساكن فراغ getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  ܐܢ (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    أكد(DE && "Must be a directory");
    لكل (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.push_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.pop_back();
    }
    ܦܢܐ;
  }

  ܐܢ (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    أكد(DR && "Must be a directory remap");
    SmallString<128> VPath;
    لكل (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.push_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    ܦܢܐ;
  }

  أكد(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  أكد(FE && "Must be a file");
  SmallString<128> VPath;
  لكل (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.push_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

فراغ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> RootResult = VFS.lookupPath("/");
  ܐܢ (!RootResult)
    ܦܢܐ;
  SmallVector<StringRef, 8> Components;
  Components.push_back("/");
  getVFSEntries(RootResult->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  ساكن std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ܦܢܐ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

فراغ YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             منطقي IsDirectory) {
  أكد(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  أكد(sys::path::is_absolute(RealPath) && "real path not absolute");
  أكد(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

فراغ YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/ܕܓܠܐ);
}

فراغ YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/ܫܪܪܐ);
}

نطاق {

صنف JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> DirStack;

  unsigned getDirIndent() { ܦܢܐ 4 * DirStack.size(); }
  unsigned getFileIndent() { ܦܢܐ 4 * (DirStack.size() + 1); }
  منطقي containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  فراغ startDirectory(StringRef Path);
  فراغ endDirectory();
  فراغ writeEntry(StringRef VPath, StringRef RPath);

عام:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  فراغ اكتب(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<منطقي> UseExternalNames,
             std::optional<منطقي> IsCaseSensitive,
             std::optional<منطقي> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

منطقي JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using نطاق llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  لكل (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    ܐܢ (*IParent != *IChild)
      ܦܢܐ ܕܓܠܐ;
  }
  // Have we exhausted the parent path?
  ܦܢܐ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  أكد(!Parent.empty());
  أكد(containedIn(Parent, Path));
  ܦܢܐ Path.substr(Parent.size() + 1);
}

فراغ JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      DirStack.empty() ? Path : containedPart(DirStack.back(), Path);
  DirStack.push_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

فراغ JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  DirStack.pop_back();
}

فراغ JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

فراغ JSONWriter::اكتب(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<منطقي> UseExternalNames,
                       std::optional<منطقي> IsCaseSensitive,
                       std::optional<منطقي> IsOverlayRelative,
                       StringRef OverlayDir) {
  using نطاق llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  ܐܢ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  ܐܢ (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  منطقي UseOverlayRelative = ܕܓܠܐ;
  ܐܢ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  ܐܢ (!Entries.empty()) {
    ثابت YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    ܐܢ (UseOverlayRelative) {
      أكد(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    منطقي IsCurrentDirEmpty = ܫܪܪܐ;
    ܐܢ (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = ܕܓܠܐ;
    }

    لكل (ثابت auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      ܐܢ (Dir == DirStack.back()) {
        ܐܢ (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } ܐܚܪܢܐ {
        منطقي IsDirPoppedFromStack = ܕܓܠܐ;
        ܟܡܐ (!DirStack.empty() && !containedIn(DirStack.back(), Dir)) {
          OS << "\n";
          endDirectory();
          IsDirPoppedFromStack = ܫܪܪܐ;
        }
        ܐܢ (IsDirPoppedFromStack || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = ܫܪܪܐ;
      }
      StringRef RPath = Entry.RPath;
      ܐܢ (UseOverlayRelative) {
        أكد(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      ܐܢ (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = ܕܓܠܐ;
      }
    }

    ܟܡܐ (!DirStack.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

فراغ YAMLVFSWriter::اكتب(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](ثابت YAMLVFSEntry &LHS, ثابت YAMLVFSEntry &RHS) {
    ܦܢܐ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).اكتب(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ثابت Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  ܐܢ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->Stack.push_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  أكد(FS && State && !State->Stack.empty() && "incrementing past end");
  أكد(!State->Stack.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  ܐܢ (State->HasNoPushRequest)
    State->HasNoPushRequest = ܕܓܠܐ;
  ܐܚܪܢܐ {
    ܐܢ (State->Stack.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->Stack.back()->path(), EC);
      ܐܢ (I != End) {
        State->Stack.push_back(I);
        ܦܢܐ *هذا;
      }
    }
  }

  ܟܡܐ (!State->Stack.empty() && State->Stack.back().increment(EC) == End)
    State->Stack.pop_back();

  ܐܢ (State->Stack.empty())
    State.reset(); // end iterator

  ܦܢܐ *هذا;
}

ثابت حرف FileSystem::ID = 0;
ثابت حرف OverlayFileSystem::ID = 0;
ثابت حرف ProxyFileSystem::ID = 0;
ثابت حرف InMemoryFileSystem::ID = 0;
ثابت حرف RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::useCount(
    ثابت FileSystem *FS) {
  ܦܢܐ FS->UseCount();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
