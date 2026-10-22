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
  أرجع Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ثابت Status &In, ثابت Twine &NewName) {
  أرجع Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ثابت file_status &In, ثابت Twine &NewName) {
  أرجع Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

منطقي Status::equivalent(ثابت Status &Other) ثابت {
  أكد(isStatusKnown() && Other.isStatusKnown());
  أرجع getUniqueID() == Other.getUniqueID();
}

منطقي Status::isDirectory() ثابت { أرجع Type == file_type::directory_file; }

منطقي Status::isRegularFile() ثابت { أرجع Type == file_type::regular_file; }

منطقي Status::isOther() ثابت {
  أرجع exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

منطقي Status::isSymlink() ثابت { أرجع Type == file_type::symlink_file; }

منطقي Status::isStatusKnown() ثابت { أرجع Type != file_type::status_error; }

منطقي Status::exists() ثابت {
  أرجع isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = افتراضي;

FileSystem::~FileSystem() = افتراضي;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(ثابت llvm::Twine &Name, int64_t FileSize,
                             منطقي RequiresNullTerminator, منطقي IsVolatile,
                             منطقي IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_بيناري(Name);
  إذا (!F)
    أرجع F.getError();

  أرجع (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<حرف> &Path) ثابت {
  إذا (llvm::sys::path::is_absolute(Path))
    أرجع {};

  auto WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  أرجع {};
}

std::error_code FileSystem::getRealPath(ثابت Twine &Path,
                                        SmallVectorImpl<حرف> &Output) {
  أرجع errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(ثابت Twine &Path, منطقي &Result) {
  أرجع errc::operation_not_permitted;
}

منطقي FileSystem::exists(ثابت Twine &Path) {
  auto Status = status(Path);
  أرجع Status && Status->exists();
}

llvm::ErrorOr<منطقي> FileSystem::equivalent(ثابت Twine &A, ثابت Twine &B) {
  auto StatusA = status(A);
  إذا (!StatusA)
    أرجع StatusA.getError();
  auto StatusB = status(B);
  إذا (!StatusB)
    أرجع StatusB.getError();
  أرجع StatusA->equivalent(*StatusB);
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
فراغ FileSystem::dump() ثابت { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
ساكن منطقي isTraversalComponent(StringRef Component) {
  أرجع Component == ".." || Component == ".";
}

ساكن منطقي pathHasTraversal(StringRef Path) {
  using نطاق llvm::sys;

  لكل (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    إذا (isTraversalComponent(Comp))
      أرجع true;
  أرجع خطأ;
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
  إذا (!S.isStatusKnown()) {
    file_status RealStatus;
    إذا (std::error_code EC = sys::fs::status(FD, RealStatus))
      أرجع EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  أرجع S;
}

ErrorOr<std::string> RealFile::getName() {
  أرجع RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(ثابت Twine &Name, int64_t FileSize,
                    منطقي RequiresNullTerminator, منطقي IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot get buffer for closed file");
  أرجع MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  أرجع EC;
}

فراغ RealFile::setPath(ثابت Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  إذا (auto Status = status())
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
    إذا (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      إذا (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      وإلا إذا (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      وإلا
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(ثابت Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ثابت Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_بيناري(ثابت Twine &Path) override;
  directory_iterator dir_begin(ثابت Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override;
  std::error_code setCurrentWorkingDirectory(ثابت Twine &Path) override;
  std::error_code isLocal(ثابت Twine &Path, منطقي &Result) override;
  فراغ
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.تولق_back(Dir.str());
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
    إذا (!WD || !*WD)
      أرجع Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    أرجع Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ثابت Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    إذا (!FDOrErr)
      أرجع errorToErrorCode(FDOrErr.takeError());
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

ErrorOr<Status> RealFileSystem::status(ثابت Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  إذا (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    أرجع EC;
  أرجع Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ثابت Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_بيناري(ثابت Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أرجع openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ثابت {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  إذا (WD && *WD)
    أرجع std::string(WD->get().Specified);
  إذا (WD)
    أرجع WD->getError();

  SmallString<128> Dir;
  إذا (std::error_code EC = llvm::sys::fs::current_path(Dir))
    أرجع EC;
  أرجع std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  إذا (!WD)
    أرجع llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  منطقي IsDir;
  إذا (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    أرجع Err;
  إذا (!IsDir)
    أرجع std::make_error_code(std::errc::not_a_directory);
  إذا (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    أرجع Err;
  WD = WorkingDirectory{Absolute, Resolved};
  أرجع std::error_code();
}

std::error_code RealFileSystem::isLocal(ثابت Twine &Path, منطقي &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(ثابت Twine &Path,
                                            SmallVectorImpl<حرف> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  أرجع llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

فراغ RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) ثابت {
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
      makeIntrusiveRefCnt<RealFileSystem>(true);
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
  RealFSDirIter(ثابت Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    إذا (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    أرجع EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ثابت Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  أرجع directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.تولق_back(std::move(BaseFS));
}

فراغ OverlayFileSystem::تولق_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.تولق_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(ثابت Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    إذا (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      أرجع Status;
  }
  أرجع make_error_code(llvm::errc::no_such_file_or_directory);
}

منطقي OverlayFileSystem::exists(ثابت Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    إذا ((*I)->exists(Path))
      أرجع true;
  }
  أرجع خطأ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ثابت llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  لكل (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    إذا (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      أرجع Result;
  }
  أرجع make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ثابت {
  // All file systems are synchronized, just take the first working directory.
  أرجع FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  لكل (auto &FS : FSList)
    إذا (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      أرجع EC;
  أرجع {};
}

std::error_code OverlayFileSystem::isLocal(ثابت Twine &Path, منطقي &Result) {
  لكل (auto &FS : FSList)
    إذا (FS->exists(Path))
      أرجع FS->isLocal(Path, Result);
  أرجع errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(ثابت Twine &Path,
                                               SmallVectorImpl<حرف> &Output) {
  لكل (ثابت auto &FS : FSList)
    إذا (FS->exists(Path))
      أرجع FS->getRealPath(Path, Output);
  أرجع errc::no_such_file_or_directory;
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
  إذا (Type == PrintType::Summary)
    أرجع;

  إذا (Type == PrintType::Contents)
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
    طالما (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.کلوار_back();
      إذا (CurrentDirIter != directory_iterator())
        اكسر; // found
    }

    إذا (IsFirstTime && CurrentDirIter == directory_iterator())
      أرجع errc::no_such_file_or_directory;
    أرجع {};
  }

  std::error_code incrementDirIter(منطقي IsFirstTime) {
    أكد((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    إذا (!IsFirstTime)
      CurrentDirIter.increment(EC);
    إذا (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    أرجع EC;
  }

  std::error_code incrementImpl(منطقي IsFirstTime) {
    طالما (true) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      إذا (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        أرجع EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      إذا (SeenNames.سيسيڤ(Name).second)
        أرجع EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

عام:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    لكل (ثابت auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      إذا (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        أرجع;
      }
      إذا (!FEC)
        IterList.تولق_back(Iter);
    }
    EC = incrementImpl(true);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(true);
  }

  std::error_code increment() override { أرجع incrementImpl(خطأ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ثابت Twine &Dir,
                                                std::error_code &EC) {
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

تعداد In_Memory_نود_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
صنف In_Memory_نود {
  In_Memory_نود_Kind Kind;
  std::string FileName;

عام:
  In_Memory_نود(llvm::StringRef FileName, In_Memory_نود_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  virtual ~In_Memory_نود() = افتراضي;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  virtual Status getStatus(ثابت Twine &RequestedName) ثابت = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ثابت { أرجع FileName; }
  In_Memory_نود_Kind getKind() ثابت { أرجع Kind; }
  virtual std::string toString(unsigned Indent) ثابت = 0;
};

صنف InMemoryFile : عام In_Memory_نود {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

عام:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_نود(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    أرجع Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() ثابت { أرجع Buffer.get(); }

  std::string toString(unsigned Indent) ثابت override {
    أرجع (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ساكن منطقي classof(ثابت In_Memory_نود *N) {
    أرجع N->getKind() == IME_File;
  }
};

نطاق {

صنف InMemoryHardLink : عام In_Memory_نود {
  ثابت InMemoryFile &ResolvedFile;

عام:
  InMemoryHardLink(StringRef Path, ثابت InMemoryFile &ResolvedFile)
      : In_Memory_نود(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ثابت InMemoryFile &getResolvedFile() ثابت { أرجع ResolvedFile; }

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    أرجع ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  ساكن منطقي classof(ثابت In_Memory_نود *N) {
    أرجع N->getKind() == IME_HardLink;
  }
};

صنف InMemorySymbolicLink : عام In_Memory_نود {
  std::string TargetPath;
  Status Stat;

عام:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_نود(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) ثابت override {
    أرجع std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    أرجع Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() ثابت { أرجع TargetPath; }

  ساكن منطقي classof(ثابت In_Memory_نود *N) {
    أرجع N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
صنف InMemoryFileAdaptor : عام File {
  ثابت InMemoryFile &نود;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

عام:
  explicit InMemoryFileAdaptor(ثابت InMemoryFile &نود,
                               std::string RequestedName)
      : نود(نود), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    أرجع نود.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(ثابت Twine &Name, int64_t FileSize, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    llvm::MemoryBuffer *Buf = نود.getBuffer();
    أرجع llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { أرجع {}; }

  فراغ setPath(ثابت Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

صنف InMemoryDirectory : عام In_Memory_نود {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_نود>, std::less<>> Entries;

عام:
  InMemoryDirectory(Status Stat)
      : In_Memory_نود(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    أرجع Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ثابت { أرجع Stat.getUniqueID(); }

  In_Memory_نود *getChild(StringRef Name) ثابت {
    auto I = Entries.find(Name);
    إذا (I != Entries.end())
      أرجع I->second.get();
    أرجع nullptr;
  }

  In_Memory_نود *addChild(StringRef Name, std::unique_ptr<In_Memory_نود> Child) {
    أرجع Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ثابت { أرجع Entries.begin(); }
  const_iterator end() ثابت { أرجع Entries.end(); }

  std::string toString(unsigned Indent) ثابت override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    لكل (ثابت auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    أرجع Result;
  }

  ساكن منطقي classof(ثابت In_Memory_نود *N) {
    أرجع N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ساكن sys::fs::UniqueID getUniqueID(hash_code Hash) {
  أرجع sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ساكن sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  أرجع getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
ساكن sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  أرجع getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_نود_Info::makeStatus() ثابت {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  أرجع Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(منطقي UseNormalizedPaths)
    : اکر_2(جديد detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = افتراضي;

std::string InMemoryFileSystem::toString() ثابت {
  أرجع اکر_2->toString(/*Indent=*/0);
}

منطقي InMemoryFileSystem::addFile(ثابت Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_نود_Fn Make_نود) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  إذا (Path.empty())
    أرجع خطأ;

  detail::InMemoryDirectory *Dir = اکر_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  ثابت auto ResolvedUser = User.نيلاي_or(0);
  ثابت auto ResolvedGroup = Group.نيلاي_or(0);
  ثابت auto ResolvedType = Type.نيلاي_or(sys::fs::file_type::regular_file);
  ثابت auto ResolvedPerms = Perms.نيلاي_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ثابت auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  طالما (true) {
    Name = *I;
    ++I;
    إذا (I == E)
      اكسر;
    detail::In_Memory_نود *نود = Dir->getChild(Name);
    إذا (!نود) {
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
    إذا (!isa<detail::InMemoryDirectory>(نود))
      أرجع خطأ;
    Dir = cast<detail::InMemoryDirectory>(نود);
  }
  detail::In_Memory_نود *نود = Dir->getChild(Name);
  إذا (!نود) {
    Dir->addChild(Name,
                  Make_نود({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    أرجع true;
  }
  إذا (isa<detail::InMemoryDirectory>(نود))
    أرجع ResolvedType == sys::fs::file_type::directory_file;

  أكد((isa<detail::InMemoryFile>(نود) ||
          isa<detail::InMemoryHardLink>(نود)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  إذا (auto *Link = dyn_cast<detail::InMemoryHardLink>(نود)) {
    أرجع Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  أرجع cast<detail::InMemoryFile>(نود)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

منطقي InMemoryFileSystem::addFile(ثابت Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  أرجع addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_نود_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_نود> {
                   Status Stat = NNI.makeStatus();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::InMemoryDirectory>(Stat);
                   أرجع std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

منطقي InMemoryFileSystem::addFileNoOwn(
    ثابت Twine &P, time_t ModificationTime,
    ثابت llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  أرجع addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_نود_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_نود> {
                   Status Stat = NNI.makeStatus();
                   إذا (Stat.getType() == sys::fs::file_type::directory_file)
                     أرجع std::make_unique<detail::InMemoryDirectory>(Stat);
                   أرجع std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_نود_Or_Error
InMemoryFileSystem::lookup_نود(ثابت Twine &P, منطقي FollowFinalSymlink,
                               size_t SymlinkDepth) ثابت {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  ثابت detail::InMemoryDirectory *Dir = اکر_2.get();
  إذا (Path.empty())
    أرجع detail::Named_نود_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  طالما (true) {
    detail::In_Memory_نود *نود = Dir->getChild(*I);
    ++I;
    إذا (!نود)
      أرجع errc::no_such_file_or_directory;

    إذا (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(نود)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      إذا (I == E && !FollowFinalSymlink)
        أرجع detail::Named_نود_Or_Error(Path, Symlink);

      إذا (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        أرجع errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      إذا (std::error_code EC = makeAbsolute(TargetPath))
        أرجع EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_نود(TargetPath, /*FollowFinalSymlink=*/true, SymlinkDepth + 1);
      إذا (!Target || I == E)
        أرجع Target;

      إذا (!isa<detail::InMemoryDirectory>(*Target))
        أرجع errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      استمر;
    }

    // Return the file if it's at the end of the path.
    إذا (auto File = dyn_cast<detail::InMemoryFile>(نود)) {
      إذا (I == E)
        أرجع detail::Named_نود_Or_Error(Path, File);
      أرجع errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    إذا (auto File = dyn_cast<detail::InMemoryHardLink>(نود)) {
      إذا (I == E)
        أرجع detail::Named_نود_Or_Error(Path, &File->getResolvedFile());
      أرجع errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(نود);
    إذا (I == E)
      أرجع detail::Named_نود_Or_Error(Path, Dir);
  }
}

منطقي InMemoryFileSystem::addHardLink(ثابت Twine &NewLink,
                                     ثابت Twine &Target) {
  auto New_Link_نود = lookup_نود(NewLink, /*FollowFinalSymlink=*/خطأ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_نود = lookup_نود(Target, /*FollowFinalSymlink=*/true);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  إذا (!Target_نود || New_Link_نود || !isa<detail::InMemoryFile>(*Target_نود))
    أرجع خطأ;
  أرجع addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_نود_Info NNI) {
                   أرجع std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_نود));
                 });
}

منطقي InMemoryFileSystem::addSymbolicLink(
    ثابت Twine &NewLink, ثابت Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_نود = lookup_نود(NewLink, /*FollowFinalSymlink=*/خطأ);
  إذا (New_Link_نود)
    أرجع خطأ;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  أرجع addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_نود_Info NNI) {
                   أرجع std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(ثابت Twine &Path) {
  auto نود = lookup_نود(Path, /*FollowFinalSymlink=*/true);
  إذا (نود)
    أرجع (*نود)->getStatus(Path);
  أرجع نود.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(ثابت Twine &Path) {
  auto نود = lookup_نود(Path,/*FollowFinalSymlink=*/true);
  إذا (!نود)
    أرجع نود.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  إذا (auto *F = dyn_cast<detail::InMemoryFile>(*نود))
    أرجع std::unique_ptr<File>(
        جديد detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  أرجع make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
صنف InMemoryFileSystem::DirIterator : عام llvm::vfs::detail::DirIterImpl {
  ثابت InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  فراغ setCurrentEntry() {
    إذا (I != E) {
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
        إذا (auto SymlinkTarget =
                FS->lookup_نود(Path, /*FollowFinalSymlink=*/true)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        اكسر;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } وإلا {
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
    أرجع {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(ثابت Twine &Dir,
                                                 std::error_code &EC) {
  auto نود = lookup_نود(Dir, /*FollowFinalSymlink=*/true);
  إذا (!نود) {
    EC = نود.getError();
    أرجع directory_iterator(std::make_shared<DirIterator>());
  }

  إذا (auto *Dir_نود = dyn_cast<detail::InMemoryDirectory>(*نود))
    أرجع directory_iterator(
        std::make_shared<DirIterator>(هذا, *Dir_نود, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  أرجع directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(ثابت Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (فراغ)EC;

  إذا (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  إذا (!Path.empty())
    WorkingDirectory = std::string(Path);
  أرجع {};
}

std::error_code InMemoryFileSystem::getRealPath(ثابت Twine &Path,
                                                SmallVectorImpl<حرف> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  إذا (!CWD || CWD->empty())
    أرجع errc::operation_not_permitted;
  Path.toVector(Output);
  إذا (auto EC = makeAbsolute(Output))
    أرجع EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/true);
  أرجع {};
}

std::error_code InMemoryFileSystem::isLocal(ثابت Twine &Path, منطقي &Result) {
  Result = خطأ;
  أرجع {};
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
  إذا (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  أرجع style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ساكن llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/true, style);
  أرجع result;
}

/// Whether the error and entry specify a file/directory that was not found.
ساكن منطقي isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  إذا (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    أرجع خطأ;
  أرجع EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  أكد(ExternalFS && "RedirectingFileSystem requires an external FS");
  إذا (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
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
    إذا (!IsFirstTime)
      ++Current;
    إذا (Current != End) {
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
    } وإلا {
      CurrentEntry = directory_entry();
    }
    أرجع {};
  };

عام:
  RedirectingFSDirIterImpl(
      ثابت Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/true);
  }

  std::error_code increment() override {
    أرجع incrementImpl(/*IsFirstTime=*/خطأ);
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
    إذا (ExternalIter != llvm::vfs::directory_iterator())
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
    إذا (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    وإلا
      CurrentEntry = directory_entry();
    أرجع EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ثابت {
  أرجع WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  إذا (!exists(Path))
    أرجع errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  إذا (std::error_code EC = makeAbsolute(AbsolutePath))
    أرجع EC;
  WorkingDirectory = std::string(AbsolutePath);
  أرجع {};
}

std::error_code RedirectingFileSystem::isLocal(ثابت Twine &Path_,
                                               منطقي &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  إذا (makeAbsolute(Path))
    أرجع {};

  أرجع ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<حرف> &Path) ثابت {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  إذا (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    أرجع {};

  auto WorkingDir = getCurrentWorkingDirectory();
  إذا (!WorkingDir)
    أرجع WorkingDir.getError();

  أرجع makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<حرف> &Path) ثابت {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  إذا (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    أرجع std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  إذا (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } وإلا {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    إذا (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  إذا (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  أرجع {};
}

directory_iterator RedirectingFileSystem::dir_begin(ثابت Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  إذا (EC)
    أرجع {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  إذا (!Result) {
    إذا (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      أرجع ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    أرجع {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  إذا (!S) {
    إذا (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
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
  std::error_code RedirectEC;
  إذا (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    إذا (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } وإلا {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  إذا (RedirectEC) {
    إذا (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      أرجع {};
    }
    RedirectIter = {};
  }

  إذا (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    أرجع RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  إذا (ExternalEC) {
    إذا (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      أرجع {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  بدل (Redirection) {
  حالة RedirectKind::Fallthrough:
    Iters.تولق_back(ExternalIter);
    Iters.تولق_back(RedirectIter);
    اكسر;
  حالة RedirectKind::Fallback:
    Iters.تولق_back(RedirectIter);
    Iters.تولق_back(ExternalIter);
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

فراغ RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ثابت {
  أرجع OverlayFileDir;
}

فراغ RedirectingFileSystem::setFallthrough(منطقي Fallthrough) {
  إذا (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } وإلا {
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
  لكل (ثابت auto &اکر_2 : Roots)
    R.تولق_back(اکر_2->getName());
  أرجع R;
}

فراغ RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  إذا (Type == PrintType::Summary)
    أرجع;

  لكل (ثابت auto &اکر_2 : Roots)
    printEntry(OS, اکر_2.get(), IndentLevel);

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
  إذا (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
صنف llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  فراغ error(yaml::نود *N, ثابت Twine &ڤسنن_2) { Stream.printError(N, ڤسنن_2); }

  // false on error
  منطقي parseScalarString(yaml::نود *N, StringRef &Result,
                         SmallVectorImpl<حرف> &Storage) {
    ثابت auto *S = dyn_cast_if_present<yaml::Scalar_نود>(N);

    إذا (!S) {
      error(N, "expected string");
      أرجع خطأ;
    }
    Result = S->get_نيلاي(Storage);
    أرجع true;
  }

  // false on error
  منطقي parseScalarBool(yaml::نود *N, منطقي &Result) {
    SmallString<5> Storage;
    StringRef نيلاي;
    إذا (!parseScalarString(N, نيلاي, Storage))
      أرجع خطأ;

    إذا (نيلاي.equals_insensitive("true") || نيلاي.equals_insensitive("on") ||
        نيلاي.equals_insensitive("yes") || نيلاي == "1") {
      Result = true;
      أرجع true;
    } وإلا إذا (نيلاي.equals_insensitive("false") ||
               نيلاي.equals_insensitive("off") ||
               نيلاي.equals_insensitive("no") || نيلاي == "0") {
      Result = خطأ;
      أرجع true;
    }

    error(N, "expected boolean value");
    أرجع خطأ;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::نود *N) {
    SmallString<12> Storage;
    StringRef نيلاي;
    إذا (!parseScalarString(N, نيلاي, Storage))
      أرجع std::nullopt;

    إذا (نيلاي.equals_insensitive("fallthrough")) {
      أرجع RedirectingFileSystem::RedirectKind::Fallthrough;
    } وإلا إذا (نيلاي.equals_insensitive("fallback")) {
      أرجع RedirectingFileSystem::RedirectKind::Fallback;
    } وإلا إذا (نيلاي.equals_insensitive("redirect-only")) {
      أرجع RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    أرجع std::nullopt;
  }

  std::optional<RedirectingFileSystem::اکر_Relative_Kind>
  parse_اکر_Relative_Kind(yaml::نود *N) {
    SmallString<12> Storage;
    StringRef نيلاي;
    إذا (!parseScalarString(N, نيلاي, Storage))
      أرجع std::nullopt;
    إذا (نيلاي.equals_insensitive("cwd")) {
      أرجع RedirectingFileSystem::اکر_Relative_Kind::CWD;
    } وإلا إذا (نيلاي.equals_insensitive("overlay-dir")) {
      أرجع RedirectingFileSystem::اکر_Relative_Kind::OverlayDir;
    }
    أرجع std::nullopt;
  }

  بنية کونچي_Status {
    منطقي Required;
    منطقي Seen = خطأ;

    کونچي_Status(منطقي Required = خطأ) : Required(Required) {}
  };

  using کونچي_Status_Pair = std::pair<StringRef, کونچي_Status>;

  // false on error
  منطقي check_Duplicate_Or_Unknown_کونچي(yaml::نود *کونچي_نود, StringRef کونچي,
                                  DenseMap<StringRef, کونچي_Status> &کونچي_سموا_2) {
    auto It = کونچي_سموا_2.find(کونچي);
    إذا (It == کونچي_سموا_2.end()) {
      error(کونچي_نود, "unknown key");
      أرجع خطأ;
    }
    کونچي_Status &S = It->second;
    إذا (S.Seen) {
      error(کونچي_نود, Twine("duplicate key '") + کونچي + "'");
      أرجع خطأ;
    }
    S.Seen = true;
    أرجع true;
  }

  // false on error
  منطقي check_Missing_کونچي_سموا(yaml::نود *Obj, DenseMap<StringRef, کونچي_Status> &کونچي_سموا_2) {
    لكل (ثابت auto &I : کونچي_سموا_2) {
      إذا (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        أرجع خطأ;
      }
    }
    أرجع true;
  }

عام:
  ساكن RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    إذا (!ParentEntry) { // Look for a existent root
      لكل (ثابت auto &اکر_2 : FS->Roots) {
        إذا (Name == اکر_2->getName()) {
          ParentEntry = اکر_2.get();
          أرجع ParentEntry;
        }
      }
    } وإلا { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      لكل (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        إذا (DirContent && Name == Content->getName())
          أرجع DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    إذا (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.تولق_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      أرجع ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    أرجع DE->getLastContent();
  }

خاص:
  فراغ unique_Overlay_ڤوکوق(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    بدل (SrcE->getKind()) {
    حالة RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      إذا (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      لكل (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_ڤوکوق(FS, SubEntry.get(), NewParentE);
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
  parseEntry(yaml::نود *N, RedirectingFileSystem *FS, منطقي Is_اکر_Entry) {
    auto *M = dyn_cast<yaml::Mapping_نود>(N);
    إذا (!M) {
      error(N, "expected mapping node for file or directory entry");
      أرجع nullptr;
    }

    کونچي_Status_Pair Fields[] = {
        کونچي_Status_Pair("name", true),
        کونچي_Status_Pair("type", true),
        کونچي_Status_Pair("contents", خطأ),
        کونچي_Status_Pair("external-contents", خطأ),
        کونچي_Status_Pair("use-external-name", خطأ),
    };

    DenseMap<StringRef, کونچي_Status> کونچي_سموا_2(std::begin(Fields), std::end(Fields));

    تعداد { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::نود *Name_نيلاي_نود = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    لكل (auto &I : *M) {
      StringRef کونچي;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      إذا (!parseScalarString(I.get_کونچي(), کونچي, Buffer))
        أرجع nullptr;

      إذا (!check_Duplicate_Or_Unknown_کونچي(I.get_کونچي(), کونچي, کونچي_سموا_2))
        أرجع nullptr;

      StringRef نيلاي;
      إذا (کونچي == "name") {
        إذا (!parseScalarString(I.get_نيلاي(), نيلاي, Buffer))
          أرجع nullptr;

        Name_نيلاي_نود = I.get_نيلاي();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(نيلاي).str();
      } وإلا إذا (کونچي == "type") {
        إذا (!parseScalarString(I.get_نيلاي(), نيلاي, Buffer))
          أرجع nullptr;
        إذا (نيلاي == "file")
          Kind = RedirectingFileSystem::EK_File;
        وإلا إذا (نيلاي == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        وإلا إذا (نيلاي == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        وإلا {
          error(I.get_نيلاي(), "unknown value for 'type'");
          أرجع nullptr;
        }
      } وإلا إذا (کونچي == "contents") {
        إذا (ContentsField != CF_NotSet) {
          error(I.get_کونچي(),
                "entry already has 'contents' or 'external-contents'");
          أرجع nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_نود>(I.get_نيلاي());
        إذا (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_نيلاي(), "expected array");
          أرجع nullptr;
        }

        لكل (auto &I : *Contents) {
          إذا (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ خطأ))
            EntryArrayContents.تولق_back(std::move(E));
          وإلا
            أرجع nullptr;
        }
      } وإلا إذا (کونچي == "external-contents") {
        إذا (ContentsField != CF_NotSet) {
          error(I.get_کونچي(),
                "entry already has 'contents' or 'external-contents'");
          أرجع nullptr;
        }
        ContentsField = CF_External;
        إذا (!parseScalarString(I.get_نيلاي(), نيلاي, Buffer))
          أرجع nullptr;

        SmallString<256> FullPath;
        إذا (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          أكد(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = نيلاي;
          إذا (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            أرجع nullptr;
          }
          FullPath = AbsFullPath;
        } وإلا {
          FullPath = نيلاي;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } وإلا إذا (کونچي == "use-external-name") {
        منطقي Val;
        إذا (!parseScalarBool(I.get_نيلاي(), Val))
          أرجع nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (Stream.failed())
      أرجع nullptr;

    // check for missing keys
    إذا (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      أرجع nullptr;
    }
    إذا (!check_Missing_کونچي_سموا(N, کونچي_سموا_2))
      أرجع nullptr;

    // check invalid configuration
    إذا (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      أرجع nullptr;
    }

    إذا (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      أرجع nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    إذا (Is_اکر_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      إذا (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } وإلا إذا (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } وإلا {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        إذا (FS->اکر_Relative ==
            RedirectingFileSystem::اکر_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          أكد(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } وإلا {
          EC = FS->makeAbsolute(Name);
        }
        إذا (EC) {
          أكد(Name_نيلاي_نود && "Name presence should be checked earlier");
          error(
              Name_نيلاي_نود,
              "entry with relative path at the root level is not discoverable");
          أرجع nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      إذا (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t اکر_Path_Len = sys::path::اکر_path(Trimmed, path_style).size();
    طالما (Trimmed.size() > اکر_Path_Len &&
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
    إذا (Parent.empty())
      أرجع Result;

    // if 'name' contains multiple components, create implicit directory entries
    لكل (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.تولق_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    أرجع Result;
  }

عام:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  منطقي parse(yaml::نود *اکر_2, RedirectingFileSystem *FS) {
    auto *اتس = dyn_cast<yaml::Mapping_نود>(اکر_2);
    إذا (!اتس) {
      error(اکر_2, "expected mapping node");
      أرجع خطأ;
    }

    کونچي_Status_Pair Fields[] = {
        کونچي_Status_Pair("version", true),
        کونچي_Status_Pair("case-sensitive", خطأ),
        کونچي_Status_Pair("use-external-names", خطأ),
        کونچي_Status_Pair("root-relative", خطأ),
        کونچي_Status_Pair("overlay-relative", خطأ),
        کونچي_Status_Pair("fallthrough", خطأ),
        کونچي_Status_Pair("redirecting-with", خطأ),
        کونچي_Status_Pair("roots", true),
    };

    DenseMap<StringRef, کونچي_Status> کونچي_سموا_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> اکر_Entries;

    // Parse configuration and 'roots'
    لكل (auto &I : *اتس) {
      SmallString<10> کونچي_Buffer;
      StringRef کونچي;
      إذا (!parseScalarString(I.get_کونچي(), کونچي, کونچي_Buffer))
        أرجع خطأ;

      إذا (!check_Duplicate_Or_Unknown_کونچي(I.get_کونچي(), کونچي, کونچي_سموا_2))
        أرجع خطأ;

      إذا (کونچي == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_نود>(I.get_نيلاي());
        إذا (!Roots) {
          error(I.get_نيلاي(), "expected array");
          أرجع خطأ;
        }

        لكل (auto &I : *Roots) {
          إذا (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ true))
            اکر_Entries.تولق_back(std::move(E));
          وإلا
            أرجع خطأ;
        }
      } وإلا إذا (کونچي == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        إذا (!parseScalarString(I.get_نيلاي(), VersionString, Storage))
          أرجع خطأ;
        صحيح Version;
        إذا (VersionString.getAsInteger<صحيح>(10, Version)) {
          error(I.get_نيلاي(), "expected integer");
          أرجع خطأ;
        }
        إذا (Version < 0) {
          error(I.get_نيلاي(), "invalid version number");
          أرجع خطأ;
        }
        إذا (Version != 0) {
          error(I.get_نيلاي(), "version mismatch, expected 0");
          أرجع خطأ;
        }
      } وإلا إذا (کونچي == "case-sensitive") {
        إذا (!parseScalarBool(I.get_نيلاي(), FS->CaseSensitive))
          أرجع خطأ;
      } وإلا إذا (کونچي == "overlay-relative") {
        إذا (!parseScalarBool(I.get_نيلاي(), FS->IsRelativeOverlay))
          أرجع خطأ;
      } وإلا إذا (کونچي == "use-external-names") {
        إذا (!parseScalarBool(I.get_نيلاي(), FS->UseExternalNames))
          أرجع خطأ;
      } وإلا إذا (کونچي == "fallthrough") {
        إذا (کونچي_سموا_2["redirecting-with"].Seen) {
          error(I.get_نيلاي(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        منطقي ShouldFallthrough = خطأ;
        إذا (!parseScalarBool(I.get_نيلاي(), ShouldFallthrough))
          أرجع خطأ;

        إذا (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } وإلا {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } وإلا إذا (کونچي == "redirecting-with") {
        إذا (کونچي_سموا_2["fallthrough"].Seen) {
          error(I.get_نيلاي(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          أرجع خطأ;
        }

        إذا (auto Kind = parseRedirectKind(I.get_نيلاي())) {
          FS->Redirection = *Kind;
        } وإلا {
          error(I.get_نيلاي(), "expected valid redirect kind");
          أرجع خطأ;
        }
      } وإلا إذا (کونچي == "root-relative") {
        إذا (auto Kind = parse_اکر_Relative_Kind(I.get_نيلاي())) {
          FS->اکر_Relative = *Kind;
        } وإلا {
          error(I.get_نيلاي(), "expected valid root-relative kind");
          أرجع خطأ;
        }
      } وإلا {
        llvm_unreachable("key missing from Keys");
      }
    }

    إذا (Stream.failed())
      أرجع خطأ;

    إذا (!check_Missing_کونچي_سموا(اتس, کونچي_سموا_2))
      أرجع خطأ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    لكل (auto &E : اکر_Entries)
      unique_Overlay_ڤوکوق(FS, E.get());

    أرجع true;
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
  yaml::نود *اکر_2 = DI->get_اکر();
  إذا (DI == Stream.end() || !اکر_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    أرجع nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      جديد RedirectingFileSystem(ExternalFS));

  إذا (!YAMLFilePath.empty()) {
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

  إذا (!P.parse(اکر_2, FS.get()))
    أرجع nullptr;

  أرجع FS;
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
    إذا (ToEntry)
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

  أرجع FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  أكد(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  إذا (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
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
  إذا (std::error_code EC = makeAbsolute(Path))
    أرجع EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  إذا (CanonicalPath.empty())
    أرجع make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  أرجع {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) ثابت {
  llvm::SmallString<128> CanonicalPath(Path);
  إذا (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    أرجع EC;

  // RedirectOnly means the VFS is always used.
  إذا (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = true;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  لكل (ثابت auto &اکر_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, اکر_2.get(), Entries);
    إذا (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = true;
    إذا (Result) {
      Result->Parents = std::move(Entries);
      أرجع Result;
    }

    إذا (Result.getError() != llvm::errc::no_such_file_or_directory)
      أرجع Result;
  }
  أرجع make_error_code(llvm::errc::no_such_file_or_directory);
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
  إذا (!FromName.empty()) {
    إذا (!pathComponentMatches(*Start, FromName))
      أرجع make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    إذا (Start == End) {
      // Match!
      أرجع LookupResult(From, Start, End);
    }
  }

  إذا (isa<RedirectingFileSystem::FileEntry>(From))
    أرجع make_error_code(llvm::errc::not_a_directory);

  إذا (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    أرجع LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  لكل (ثابت std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.تولق_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    إذا (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      أرجع Result;
    Entries.کلوار_back();
  }

  أرجع make_error_code(llvm::errc::no_such_file_or_directory);
}

ساكن Status getRedirectedFileStatus(ثابت Twine &OriginalPath,
                                      منطقي UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  إذا (ExternalStatus.ExposesExternalVFSPath)
    أرجع ExternalStatus;

  Status S = ExternalStatus;
  إذا (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  وإلا
    S.ExposesExternalVFSPath = true;
  أرجع S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    ثابت Twine &LookupPath, ثابت Twine &OriginalPath,
    ثابت RedirectingFileSystem::LookupResult &Result) {
  إذا (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    إذا (std::error_code EC = makeAbsolute(RemappedPath))
      أرجع EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    إذا (!S)
      أرجع S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    أرجع getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  أرجع Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(ثابت Twine &LookupPath,
                                         ثابت Twine &OriginalPath) ثابت {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  إذا (!Result || Result->ExposesExternalVFSPath)
    أرجع Result;
  أرجع Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  إذا (std::error_code EC = makeAbsolute(Path))
    أرجع EC;

  إذا (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    إذا (S)
      أرجع S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  إذا (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      أرجع getExternalStatus(Path, OriginalPath);
    أرجع Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  إذا (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع getExternalStatus(Path, OriginalPath);
  }

  أرجع S;
}

منطقي RedirectingFileSystem::exists(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  إذا (makeAbsolute(Path))
    أرجع خطأ;

  إذا (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    إذا (ExternalFS->exists(Path))
      أرجع true;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  إذا (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      أرجع ExternalFS->exists(Path);
    أرجع خطأ;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  إذا (!ExtRedirect) {
    أكد(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    أرجع true;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  إذا (makeAbsolute(RemappedPath))
    أرجع خطأ;

  إذا (ExternalFS->exists(RemappedPath))
    أرجع true;

  إذا (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    أرجع ExternalFS->exists(Path);
  }

  أرجع خطأ;
}

نطاق {

/// Provide a file wrapper with an overriden status.
صنف FileWithFixedStatus : عام File {
  std::unique_ptr<File> InnerFile;
  Status S;

عام:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { أرجع S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(ثابت Twine &Name, int64_t FileSize, منطقي RequiresNullTerminator,
            منطقي IsVolatile) override {
    أرجع InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { أرجع InnerFile->close(); }

  فراغ setPath(ثابت Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, ثابت Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  إذا (!Result || (*Result)->status()->ExposesExternalVFSPath)
    أرجع Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  إذا (Name && Name.get() != P.str())
    F->get()->setPath(P);
  أرجع F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  إذا (std::error_code EC = makeAbsolute(Path))
    أرجع EC;

  إذا (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    إذا (F)
      أرجع F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  إذا (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      أرجع File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    أرجع Result.getError();
  }

  إذا (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    أرجع make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  إذا (std::error_code EC = makeAbsolute(RemappedPath))
    أرجع EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  إذا (!ExternalFile) {
    إذا (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    أرجع ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  إذا (!ExternalStatus)
    أرجع ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  أرجع std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(ثابت Twine &OriginalPath,
                                   SmallVectorImpl<حرف> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  إذا (std::error_code EC = makeAbsolute(Path))
    أرجع EC;

  إذا (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    إذا (!EC)
      أرجع EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  إذا (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    إذا (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      أرجع ExternalFS->getRealPath(Path, Output);
    أرجع Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  إذا (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    إذا (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      أرجع ExternalFS->getRealPath(Path, Output);
    }
    أرجع P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  إذا (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    أرجع {};
  }
  أرجع llvm::errc::invalid_argument;
}

فراغ RedirectingFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  إذا (makeAbsolute(Path))
    أرجع;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ثابت منطقي ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  إذا (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    إذا (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    أرجع;
  }

  بدل (Result->E->getKind()) {
  حالة EK_File:
    أرجع;
  حالة EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    اكسر;
  حالة EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    إذا (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    اكسر;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  إذا (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, فراغ *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  أرجع RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

ساكن فراغ getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  إذا (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    أكد(DE && "Must be a directory");
    لكل (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.تولق_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.کلوار_back();
    }
    أرجع;
  }

  إذا (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    أكد(DR && "Must be a directory remap");
    SmallString<128> VPath;
    لكل (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.تولق_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    أرجع;
  }

  أكد(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  أكد(FE && "Must be a file");
  SmallString<128> VPath;
  لكل (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.تولق_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

فراغ vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> اکر_Result = VFS.lookupPath("/");
  إذا (!اکر_Result)
    أرجع;
  SmallVector<StringRef, 8> Components;
  Components.تولق_back("/");
  getVFSEntries(اکر_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  ساكن std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  أرجع UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

فراغ YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             منطقي IsDirectory) {
  أكد(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  أكد(sys::path::is_absolute(RealPath) && "real path not absolute");
  أكد(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

فراغ YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/خطأ);
}

فراغ YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/true);
}

نطاق {

صنف JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_تيندنن;

  unsigned getDirIndent() { أرجع 4 * Dir_تيندنن.size(); }
  unsigned getFileIndent() { أرجع 4 * (Dir_تيندنن.size() + 1); }
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
    إذا (*IParent != *IChild)
      أرجع خطأ;
  }
  // Have we exhausted the parent path?
  أرجع IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  أكد(!Parent.empty());
  أكد(containedIn(Parent, Path));
  أرجع Path.substr(Parent.size() + 1);
}

فراغ JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_تيندنن.empty() ? Path : containedPart(Dir_تيندنن.back(), Path);
  Dir_تيندنن.تولق_back(Path);
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

  Dir_تيندنن.کلوار_back();
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
  إذا (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  إذا (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  منطقي UseOverlayRelative = خطأ;
  إذا (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  إذا (!Entries.empty()) {
    ثابت YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    إذا (UseOverlayRelative) {
      أكد(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    منطقي IsCurrentDirEmpty = true;
    إذا (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = خطأ;
    }

    لكل (ثابت auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      إذا (Dir == Dir_تيندنن.back()) {
        إذا (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } وإلا {
        منطقي Is_Dir_Popped_From_تيندنن = خطأ;
        طالما (!Dir_تيندنن.empty() && !containedIn(Dir_تيندنن.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_تيندنن = true;
        }
        إذا (Is_Dir_Popped_From_تيندنن || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = true;
      }
      StringRef RPath = Entry.RPath;
      إذا (UseOverlayRelative) {
        أكد(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      إذا (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = خطأ;
      }
    }

    طالما (!Dir_تيندنن.empty()) {
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
    أرجع LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).اكتب(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ثابت Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  إذا (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->تيندنن.تولق_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  أكد(FS && State && !State->تيندنن.empty() && "incrementing past end");
  أكد(!State->تيندنن.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  إذا (State->Has_No_تولق_Request)
    State->Has_No_تولق_Request = خطأ;
  وإلا {
    إذا (State->تيندنن.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->تيندنن.back()->path(), EC);
      إذا (I != End) {
        State->تيندنن.تولق_back(I);
        أرجع *هذا;
      }
    }
  }

  طالما (!State->تيندنن.empty() && State->تيندنن.back().increment(EC) == End)
    State->تيندنن.کلوار_back();

  إذا (State->تيندنن.empty())
    State.reset(); // end iterator

  أرجع *هذا;
}

ثابت حرف FileSystem::ID = 0;
ثابت حرف OverlayFileSystem::ID = 0;
ثابت حرف ProxyFileSystem::ID = 0;
ثابت حرف InMemoryFileSystem::ID = 0;
ثابت حرف RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_کيراءن(
    ثابت FileSystem *FS) {
  أرجع FS->Use_کيراءن();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

فراغ ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
