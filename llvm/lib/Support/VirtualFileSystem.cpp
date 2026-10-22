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

using فضاینام llvm;
using فضاینام llvm::vfs;

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
  بازگشت Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ثابت Status &In, ثابت Twine &NewName) {
  بازگشت Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ثابت file_status &In, ثابت Twine &NewName) {
  بازگشت Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

منطقی Status::equivalent(ثابت Status &Other) ثابت {
  أكد(isStatusKnown() && Other.isStatusKnown());
  بازگشت getUniqueID() == Other.getUniqueID();
}

منطقی Status::isDirectory() ثابت { بازگشت Type == file_type::directory_file; }

منطقی Status::isRegularFile() ثابت { بازگشت Type == file_type::regular_file; }

منطقی Status::isOther() ثابت {
  بازگشت exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

منطقی Status::isSymlink() ثابت { بازگشت Type == file_type::symlink_file; }

منطقی Status::isStatusKnown() ثابت { بازگشت Type != file_type::status_error; }

منطقی Status::exists() ثابت {
  بازگشت isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = پیشفرض;

FileSystem::~FileSystem() = پیشفرض;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(ثابت llvm::Twine &Name, int64_t FileSize,
                             منطقی RequiresNullTerminator, منطقی IsVolatile,
                             منطقی IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_دودویی(Name);
  اگر (!F)
    بازگشت F.getError();

  بازگشت (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<نویسه> &Path) ثابت {
  اگر (llvm::sys::path::is_absolute(Path))
    بازگشت {};

  auto WorkingDir = getCurrentWorkingDirectory();
  اگر (!WorkingDir)
    بازگشت WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  بازگشت {};
}

std::error_code FileSystem::getRealPath(ثابت Twine &Path,
                                        SmallVectorImpl<نویسه> &Output) {
  بازگشت errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(ثابت Twine &Path, منطقی &Result) {
  بازگشت errc::operation_not_permitted;
}

منطقی FileSystem::exists(ثابت Twine &Path) {
  auto Status = status(Path);
  بازگشت Status && Status->exists();
}

llvm::ErrorOr<منطقی> FileSystem::equivalent(ثابت Twine &A, ثابت Twine &B) {
  auto StatusA = status(A);
  اگر (!StatusA)
    بازگشت StatusA.getError();
  auto StatusB = status(B);
  اگر (!StatusB)
    بازگشت StatusB.getError();
  بازگشت StatusA->equivalent(*StatusB);
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
تهی FileSystem::dump() ثابت { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
ایستا منطقی isTraversalComponent(StringRef Component) {
  بازگشت Component == ".." || Component == ".";
}

ایستا منطقی pathHasTraversal(StringRef Path) {
  using فضاینام llvm::sys;

  برای (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    اگر (isTraversalComponent(Comp))
      بازگشت درست;
  بازگشت نادرست;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

فضاینام {

/// Wrapper around a raw file descriptor.
رده RealFile : عمومی File {
  friend رده RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    أكد(FD.isValid() && "Invalid or inactive file descriptor");
  }

عمومی:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(ثابت Twine &Name,
                                                   int64_t FileSize,
                                                   منطقی RequiresNullTerminator,
                                                   منطقی IsVolatile) override;
  std::error_code close() override;
  تهی setPath(ثابت Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot stat closed file");
  اگر (!S.isStatusKnown()) {
    file_status RealStatus;
    اگر (std::error_code EC = sys::fs::status(FD, RealStatus))
      بازگشت EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  بازگشت S;
}

ErrorOr<std::string> RealFile::getName() {
  بازگشت RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(ثابت Twine &Name, int64_t FileSize,
                    منطقی RequiresNullTerminator, منطقی IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد(FD.isValid() && "cannot get buffer for closed file");
  بازگشت MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  بازگشت EC;
}

تهی RealFile::setPath(ثابت Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  اگر (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
}

فضاینام {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
رده RealFileSystem : عمومی FileSystem {
عمومی:
  explicit RealFileSystem(منطقی LinkCWDToProcess) {
    اگر (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      اگر (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      وگرنه اگر (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      وگرنه
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(ثابت Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ثابت Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_دودویی(ثابت Twine &Path) override;
  directory_iterator dir_begin(ثابت Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ثابت override;
  std::error_code setCurrentWorkingDirectory(ثابت Twine &Path) override;
  std::error_code isLocal(ثابت Twine &Path, منطقی &Result) override;
  تهی
  getDirectoryContentRealSources(ثابت Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.افزودن_back(Dir.str());
  }
  std::error_code getRealPath(ثابت Twine &Path,
                              SmallVectorImpl<نویسه> &Output) override;

محافظتشده:
  تهی printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) ثابت override;

خصوصی:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(ثابت Twine &Path, SmallVectorImpl<نویسه> &Storage) ثابت {
    اگر (!WD || !*WD)
      بازگشت Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    بازگشت Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ثابت Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    اگر (!FDOrErr)
      بازگشت errorToErrorCode(FDOrErr.takeError());
    بازگشت std::unique_ptr<File>(
        جدید RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  ساختار WorkingDirectory {
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
  اگر (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    بازگشت EC;
  بازگشت Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ثابت Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  بازگشت openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_دودویی(ثابت Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  بازگشت openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ثابت {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  اگر (WD && *WD)
    بازگشت std::string(WD->get().Specified);
  اگر (WD)
    بازگشت WD->getError();

  SmallString<128> Dir;
  اگر (std::error_code EC = llvm::sys::fs::current_path(Dir))
    بازگشت EC;
  بازگشت std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  اگر (!WD)
    بازگشت llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  منطقی IsDir;
  اگر (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    بازگشت Err;
  اگر (!IsDir)
    بازگشت std::make_error_code(std::errc::not_a_directory);
  اگر (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    بازگشت Err;
  WD = WorkingDirectory{Absolute, Resolved};
  بازگشت std::error_code();
}

std::error_code RealFileSystem::isLocal(ثابت Twine &Path, منطقی &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  بازگشت llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(ثابت Twine &Path,
                                            SmallVectorImpl<نویسه> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  بازگشت llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

تهی RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  اگر (WD)
    OS << "own";
  وگرنه
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  ایستا IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(درست);
  sys::sandbox::violationIfEnabled();

  بازگشت FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  بازگشت std::make_unique<RealFileSystem>(نادرست);
}

فضاینام {

رده RealFSDirIter : عمومی llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

عمومی:
  RealFSDirIter(ثابت Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    اگر (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    بازگشت EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ثابت Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  بازگشت directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.افزودن_back(std::move(BaseFS));
}

تهی OverlayFileSystem::افزودن_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.افزودن_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(ثابت Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  برای (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    اگر (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      بازگشت Status;
  }
  بازگشت make_error_code(llvm::errc::no_such_file_or_directory);
}

منطقی OverlayFileSystem::exists(ثابت Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  برای (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    اگر ((*I)->exists(Path))
      بازگشت درست;
  }
  بازگشت نادرست;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ثابت llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  برای (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    اگر (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      بازگشت Result;
  }
  بازگشت make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ثابت {
  // All file systems are synchronized, just take the first working directory.
  بازگشت FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  برای (auto &FS : FSList)
    اگر (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      بازگشت EC;
  بازگشت {};
}

std::error_code OverlayFileSystem::isLocal(ثابت Twine &Path, منطقی &Result) {
  برای (auto &FS : FSList)
    اگر (FS->exists(Path))
      بازگشت FS->isLocal(Path, Result);
  بازگشت errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(ثابت Twine &Path,
                                               SmallVectorImpl<نویسه> &Output) {
  برای (ثابت auto &FS : FSList)
    اگر (FS->exists(Path))
      بازگشت FS->getRealPath(Path, Output);
  بازگشت errc::no_such_file_or_directory;
}

تهی OverlayFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  برای (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

تهی OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  برای (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

تهی OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  اگر (Type == PrintType::Summary)
    بازگشت;

  اگر (Type == PrintType::Contents)
    Type = PrintType::Summary;
  برای (ثابت auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = پیشفرض;

فضاینام {

/// Combines and deduplicates directory entries across multiple file systems.
رده CombiningDirIterImpl : عمومی llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(منطقی IsFirstTime) {
    وقتی (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.برداشت_back();
      اگر (CurrentDirIter != directory_iterator())
        بشکن; // found
    }

    اگر (IsFirstTime && CurrentDirIter == directory_iterator())
      بازگشت errc::no_such_file_or_directory;
    بازگشت {};
  }

  std::error_code incrementDirIter(منطقی IsFirstTime) {
    أكد((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    اگر (!IsFirstTime)
      CurrentDirIter.increment(EC);
    اگر (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    بازگشت EC;
  }

  std::error_code incrementImpl(منطقی IsFirstTime) {
    وقتی (درست) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      اگر (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        بازگشت EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      اگر (SeenNames.درج(Name).second)
        بازگشت EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

عمومی:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    برای (ثابت auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      اگر (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        بازگشت;
      }
      اگر (!FEC)
        IterList.افزودن_back(Iter);
    }
    EC = incrementImpl(درست);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(درست);
  }

  std::error_code increment() override { بازگشت incrementImpl(نادرست); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ثابت Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  اگر (EC)
    بازگشت {};
  بازگشت Combined;
}

تهی ProxyFileSystem::anchor() {}

فضاینام llvm {
فضاینام vfs {

فضاینام detail {

شمارش In_Memory_گره_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
رده In_Memory_گره {
  In_Memory_گره_Kind Kind;
  std::string FileName;

عمومی:
  In_Memory_گره(llvm::StringRef FileName, In_Memory_گره_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  مجازی ~In_Memory_گره() = پیشفرض;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  مجازی Status getStatus(ثابت Twine &RequestedName) ثابت = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ثابت { بازگشت FileName; }
  In_Memory_گره_Kind getKind() ثابت { بازگشت Kind; }
  مجازی std::string toString(unsigned Indent) ثابت = 0;
};

رده InMemoryFile : عمومی In_Memory_گره {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

عمومی:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_گره(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    بازگشت Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() ثابت { بازگشت Buffer.get(); }

  std::string toString(unsigned Indent) ثابت override {
    بازگشت (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  ایستا منطقی classof(ثابت In_Memory_گره *N) {
    بازگشت N->getKind() == IME_File;
  }
};

فضاینام {

رده InMemoryHardLink : عمومی In_Memory_گره {
  ثابت InMemoryFile &ResolvedFile;

عمومی:
  InMemoryHardLink(StringRef Path, ثابت InMemoryFile &ResolvedFile)
      : In_Memory_گره(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ثابت InMemoryFile &getResolvedFile() ثابت { بازگشت ResolvedFile; }

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    بازگشت ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) ثابت override {
    بازگشت std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  ایستا منطقی classof(ثابت In_Memory_گره *N) {
    بازگشت N->getKind() == IME_HardLink;
  }
};

رده InMemorySymbolicLink : عمومی In_Memory_گره {
  std::string TargetPath;
  Status Stat;

عمومی:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_گره(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) ثابت override {
    بازگشت std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    بازگشت Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() ثابت { بازگشت TargetPath; }

  ایستا منطقی classof(ثابت In_Memory_گره *N) {
    بازگشت N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
رده InMemoryFileAdaptor : عمومی File {
  ثابت InMemoryFile &گره;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

عمومی:
  explicit InMemoryFileAdaptor(ثابت InMemoryFile &گره,
                               std::string RequestedName)
      : گره(گره), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    بازگشت گره.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(ثابت Twine &Name, int64_t FileSize, منطقی RequiresNullTerminator,
            منطقی IsVolatile) override {
    llvm::MemoryBuffer *Buf = گره.getBuffer();
    بازگشت llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { بازگشت {}; }

  تهی setPath(ثابت Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

رده InMemoryDirectory : عمومی In_Memory_گره {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_گره>, std::less<>> Entries;

عمومی:
  InMemoryDirectory(Status Stat)
      : In_Memory_گره(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(ثابت Twine &RequestedName) ثابت override {
    بازگشت Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ثابت { بازگشت Stat.getUniqueID(); }

  In_Memory_گره *getChild(StringRef Name) ثابت {
    auto I = Entries.find(Name);
    اگر (I != Entries.end())
      بازگشت I->second.get();
    بازگشت nullptr;
  }

  In_Memory_گره *addChild(StringRef Name, std::unique_ptr<In_Memory_گره> Child) {
    بازگشت Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ثابت { بازگشت Entries.begin(); }
  const_iterator end() ثابت { بازگشت Entries.end(); }

  std::string toString(unsigned Indent) ثابت override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    برای (ثابت auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    بازگشت Result;
  }

  ایستا منطقی classof(ثابت In_Memory_گره *N) {
    بازگشت N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
ایستا sys::fs::UniqueID getUniqueID(hash_code Hash) {
  بازگشت sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
ایستا sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  بازگشت getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
ایستا sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  بازگشت getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_گره_Info::makeStatus() ثابت {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  بازگشت Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(منطقی UseNormalizedPaths)
    : ریشه_2(جدید detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = پیشفرض;

std::string InMemoryFileSystem::toString() ثابت {
  بازگشت ریشه_2->toString(/*Indent=*/0);
}

منطقی InMemoryFileSystem::addFile(ثابت Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_گره_Fn Make_گره) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (تهی)EC;

  اگر (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/درست);

  اگر (Path.empty())
    بازگشت نادرست;

  detail::InMemoryDirectory *Dir = ریشه_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  ثابت auto ResolvedUser = User.مقدار_or(0);
  ثابت auto ResolvedGroup = Group.مقدار_or(0);
  ثابت auto ResolvedType = Type.مقدار_or(sys::fs::file_type::regular_file);
  ثابت auto ResolvedPerms = Perms.مقدار_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ثابت auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  وقتی (درست) {
    Name = *I;
    ++I;
    اگر (I == E)
      بشکن;
    detail::In_Memory_گره *گره = Dir->getChild(Name);
    اگر (!گره) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      ادامه;
    }
    // Creating file under another file.
    اگر (!isa<detail::InMemoryDirectory>(گره))
      بازگشت نادرست;
    Dir = cast<detail::InMemoryDirectory>(گره);
  }
  detail::In_Memory_گره *گره = Dir->getChild(Name);
  اگر (!گره) {
    Dir->addChild(Name,
                  Make_گره({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    بازگشت درست;
  }
  اگر (isa<detail::InMemoryDirectory>(گره))
    بازگشت ResolvedType == sys::fs::file_type::directory_file;

  أكد((isa<detail::InMemoryFile>(گره) ||
          isa<detail::InMemoryHardLink>(گره)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  اگر (auto *Link = dyn_cast<detail::InMemoryHardLink>(گره)) {
    بازگشت Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  بازگشت cast<detail::InMemoryFile>(گره)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

منطقی InMemoryFileSystem::addFile(ثابت Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  بازگشت addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_گره_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_گره> {
                   Status Stat = NNI.makeStatus();
                   اگر (Stat.getType() == sys::fs::file_type::directory_file)
                     بازگشت std::make_unique<detail::InMemoryDirectory>(Stat);
                   بازگشت std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

منطقی InMemoryFileSystem::addFileNoOwn(
    ثابت Twine &P, time_t ModificationTime,
    ثابت llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  بازگشت addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_گره_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_گره> {
                   Status Stat = NNI.makeStatus();
                   اگر (Stat.getType() == sys::fs::file_type::directory_file)
                     بازگشت std::make_unique<detail::InMemoryDirectory>(Stat);
                   بازگشت std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_گره_Or_Error
InMemoryFileSystem::lookup_گره(ثابت Twine &P, منطقی FollowFinalSymlink,
                               size_t SymlinkDepth) ثابت {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (تهی)EC;

  اگر (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/درست);

  ثابت detail::InMemoryDirectory *Dir = ریشه_2.get();
  اگر (Path.empty())
    بازگشت detail::Named_گره_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  وقتی (درست) {
    detail::In_Memory_گره *گره = Dir->getChild(*I);
    ++I;
    اگر (!گره)
      بازگشت errc::no_such_file_or_directory;

    اگر (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(گره)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      اگر (I == E && !FollowFinalSymlink)
        بازگشت detail::Named_گره_Or_Error(Path, Symlink);

      اگر (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        بازگشت errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      اگر (std::error_code EC = makeAbsolute(TargetPath))
        بازگشت EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_گره(TargetPath, /*FollowFinalSymlink=*/درست, SymlinkDepth + 1);
      اگر (!Target || I == E)
        بازگشت Target;

      اگر (!isa<detail::InMemoryDirectory>(*Target))
        بازگشت errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      ادامه;
    }

    // Return the file if it's at the end of the path.
    اگر (auto File = dyn_cast<detail::InMemoryFile>(گره)) {
      اگر (I == E)
        بازگشت detail::Named_گره_Or_Error(Path, File);
      بازگشت errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    اگر (auto File = dyn_cast<detail::InMemoryHardLink>(گره)) {
      اگر (I == E)
        بازگشت detail::Named_گره_Or_Error(Path, &File->getResolvedFile());
      بازگشت errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(گره);
    اگر (I == E)
      بازگشت detail::Named_گره_Or_Error(Path, Dir);
  }
}

منطقی InMemoryFileSystem::addHardLink(ثابت Twine &NewLink,
                                     ثابت Twine &Target) {
  auto New_Link_گره = lookup_گره(NewLink, /*FollowFinalSymlink=*/نادرست);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_گره = lookup_گره(Target, /*FollowFinalSymlink=*/درست);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  اگر (!Target_گره || New_Link_گره || !isa<detail::InMemoryFile>(*Target_گره))
    بازگشت نادرست;
  بازگشت addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_گره_Info NNI) {
                   بازگشت std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_گره));
                 });
}

منطقی InMemoryFileSystem::addSymbolicLink(
    ثابت Twine &NewLink, ثابت Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_گره = lookup_گره(NewLink, /*FollowFinalSymlink=*/نادرست);
  اگر (New_Link_گره)
    بازگشت نادرست;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  بازگشت addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_گره_Info NNI) {
                   بازگشت std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(ثابت Twine &Path) {
  auto گره = lookup_گره(Path, /*FollowFinalSymlink=*/درست);
  اگر (گره)
    بازگشت (*گره)->getStatus(Path);
  بازگشت گره.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(ثابت Twine &Path) {
  auto گره = lookup_گره(Path,/*FollowFinalSymlink=*/درست);
  اگر (!گره)
    بازگشت گره.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  اگر (auto *F = dyn_cast<detail::InMemoryFile>(*گره))
    بازگشت std::unique_ptr<File>(
        جدید detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  بازگشت make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
رده InMemoryFileSystem::DirIterator : عمومی llvm::vfs::detail::DirIterImpl {
  ثابت InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  تهی setCurrentEntry() {
    اگر (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      انتخاب (I->second->getKind()) {
      حالت detail::IME_File:
      حالت detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        بشکن;
      حالت detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        بشکن;
      حالت detail::IME_SymbolicLink:
        اگر (auto SymlinkTarget =
                FS->lookup_گره(Path, /*FollowFinalSymlink=*/درست)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        بشکن;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } وگرنه {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

عمومی:
  DirIterator() = پیشفرض;

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
    بازگشت {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(ثابت Twine &Dir,
                                                 std::error_code &EC) {
  auto گره = lookup_گره(Dir, /*FollowFinalSymlink=*/درست);
  اگر (!گره) {
    EC = گره.getError();
    بازگشت directory_iterator(std::make_shared<DirIterator>());
  }

  اگر (auto *Dir_گره = dyn_cast<detail::InMemoryDirectory>(*گره))
    بازگشت directory_iterator(
        std::make_shared<DirIterator>(این, *Dir_گره, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  بازگشت directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(ثابت Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  أكد(!EC);
  (تهی)EC;

  اگر (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/درست);

  اگر (!Path.empty())
    WorkingDirectory = std::string(Path);
  بازگشت {};
}

std::error_code InMemoryFileSystem::getRealPath(ثابت Twine &Path,
                                                SmallVectorImpl<نویسه> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  اگر (!CWD || CWD->empty())
    بازگشت errc::operation_not_permitted;
  Path.toVector(Output);
  اگر (auto EC = makeAbsolute(Output))
    بازگشت EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/درست);
  بازگشت {};
}

std::error_code InMemoryFileSystem::isLocal(ثابت Twine &Path, منطقی &Result) {
  Result = نادرست;
  بازگشت {};
}

تهی InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

فضاینام {

ایستا llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  ثابت size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  اگر (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  بازگشت style;
}

/// Removes leading "./" as well as path components like ".." and ".".
ایستا llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/درست, style);
  بازگشت result;
}

/// Whether the error and entry specify a file/directory that was not found.
ایستا منطقی isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  اگر (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    بازگشت نادرست;
  بازگشت EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  أكد(ExternalFS && "RedirectingFileSystem requires an external FS");
  اگر (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
رده llvm::vfs::RedirectingFSDirIterImpl
    : عمومی llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(منطقی IsFirstTime) {
    أكد((IsFirstTime || Current != End) && "cannot iterate past end");
    اگر (!IsFirstTime)
      ++Current;
    اگر (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      انتخاب ((*Current)->getKind()) {
      حالت RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      حالت RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        بشکن;
      حالت RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        بشکن;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } وگرنه {
      CurrentEntry = directory_entry();
    }
    بازگشت {};
  };

عمومی:
  RedirectingFSDirIterImpl(
      ثابت Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/درست);
  }

  std::error_code increment() override {
    بازگشت incrementImpl(/*IsFirstTime=*/نادرست);
  }
};

فضاینام {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
رده RedirectingFSDirRemapIterImpl : عمومی llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

عمومی:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    اگر (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  تهی setCurrentEntry() {
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
    اگر (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    وگرنه
      CurrentEntry = directory_entry();
    بازگشت EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ثابت {
  بازگشت WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(ثابت Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  اگر (!exists(Path))
    بازگشت errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  اگر (std::error_code EC = makeAbsolute(AbsolutePath))
    بازگشت EC;
  WorkingDirectory = std::string(AbsolutePath);
  بازگشت {};
}

std::error_code RedirectingFileSystem::isLocal(ثابت Twine &Path_,
                                               منطقی &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  اگر (makeAbsolute(Path))
    بازگشت {};

  بازگشت ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<نویسه> &Path) ثابت {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  اگر (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    بازگشت {};

  auto WorkingDir = getCurrentWorkingDirectory();
  اگر (!WorkingDir)
    بازگشت WorkingDir.getError();

  بازگشت makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<نویسه> &Path) ثابت {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  اگر (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    بازگشت std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  اگر (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } وگرنه {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    اگر (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  اگر (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  بازگشت {};
}

directory_iterator RedirectingFileSystem::dir_begin(ثابت Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  اگر (EC)
    بازگشت {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  اگر (!Result) {
    اگر (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      بازگشت ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    بازگشت {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  اگر (!S) {
    اگر (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      بازگشت ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    بازگشت {};
  }

  اگر (!S->isDirectory()) {
    EC = errc::not_a_directory;
    بازگشت {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  اگر (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    اگر (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } وگرنه {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  اگر (RedirectEC) {
    اگر (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      بازگشت {};
    }
    RedirectIter = {};
  }

  اگر (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    بازگشت RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  اگر (ExternalEC) {
    اگر (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      بازگشت {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  انتخاب (Redirection) {
  حالت RedirectKind::Fallthrough:
    Iters.افزودن_back(ExternalIter);
    Iters.افزودن_back(RedirectIter);
    بشکن;
  حالت RedirectKind::Fallback:
    Iters.افزودن_back(RedirectIter);
    Iters.افزودن_back(ExternalIter);
    بشکن;
  پیشفرض:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  اگر (EC)
    بازگشت {};
  بازگشت Combined;
}

تهی RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ثابت {
  بازگشت OverlayFileDir;
}

تهی RedirectingFileSystem::setFallthrough(منطقی Fallthrough) {
  اگر (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } وگرنه {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

تهی RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() ثابت {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  برای (ثابت auto &ریشه_2 : Roots)
    R.افزودن_back(ریشه_2->getName());
  بازگشت R;
}

تهی RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  اگر (Type == PrintType::Summary)
    بازگشت;

  برای (ثابت auto &ریشه_2 : Roots)
    printEntry(OS, ریشه_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

تهی RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) ثابت {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  انتخاب (E->getKind()) {
  حالت EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    برای (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    بشکن;
  }
  حالت EK_DirectoryRemap:
  حالت EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    انتخاب (RE->getUseName()) {
    حالت NK_NotSet:
      بشکن;
    حالت NK_External:
      OS << " (UseExternalName: true)";
      بشکن;
    حالت NK_Virtual:
      OS << " (UseExternalName: false)";
      بشکن;
    }
    OS << "\n";
    بشکن;
  }
  }
}

تهی RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  اگر (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
رده llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  تهی error(yaml::گره *N, ثابت Twine &پیام_2) { Stream.printError(N, پیام_2); }

  // false on error
  منطقی parseScalarString(yaml::گره *N, StringRef &Result,
                         SmallVectorImpl<نویسه> &Storage) {
    ثابت auto *S = dyn_cast_if_present<yaml::Scalar_گره>(N);

    اگر (!S) {
      error(N, "expected string");
      بازگشت نادرست;
    }
    Result = S->get_مقدار(Storage);
    بازگشت درست;
  }

  // false on error
  منطقی parseScalarBool(yaml::گره *N, منطقی &Result) {
    SmallString<5> Storage;
    StringRef مقدار;
    اگر (!parseScalarString(N, مقدار, Storage))
      بازگشت نادرست;

    اگر (مقدار.equals_insensitive("true") || مقدار.equals_insensitive("on") ||
        مقدار.equals_insensitive("yes") || مقدار == "1") {
      Result = درست;
      بازگشت درست;
    } وگرنه اگر (مقدار.equals_insensitive("false") ||
               مقدار.equals_insensitive("off") ||
               مقدار.equals_insensitive("no") || مقدار == "0") {
      Result = نادرست;
      بازگشت درست;
    }

    error(N, "expected boolean value");
    بازگشت نادرست;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::گره *N) {
    SmallString<12> Storage;
    StringRef مقدار;
    اگر (!parseScalarString(N, مقدار, Storage))
      بازگشت std::nullopt;

    اگر (مقدار.equals_insensitive("fallthrough")) {
      بازگشت RedirectingFileSystem::RedirectKind::Fallthrough;
    } وگرنه اگر (مقدار.equals_insensitive("fallback")) {
      بازگشت RedirectingFileSystem::RedirectKind::Fallback;
    } وگرنه اگر (مقدار.equals_insensitive("redirect-only")) {
      بازگشت RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    بازگشت std::nullopt;
  }

  std::optional<RedirectingFileSystem::ریشه_Relative_Kind>
  parse_ریشه_Relative_Kind(yaml::گره *N) {
    SmallString<12> Storage;
    StringRef مقدار;
    اگر (!parseScalarString(N, مقدار, Storage))
      بازگشت std::nullopt;
    اگر (مقدار.equals_insensitive("cwd")) {
      بازگشت RedirectingFileSystem::ریشه_Relative_Kind::CWD;
    } وگرنه اگر (مقدار.equals_insensitive("overlay-dir")) {
      بازگشت RedirectingFileSystem::ریشه_Relative_Kind::OverlayDir;
    }
    بازگشت std::nullopt;
  }

  ساختار کلید_Status {
    منطقی Required;
    منطقی Seen = نادرست;

    کلید_Status(منطقی Required = نادرست) : Required(Required) {}
  };

  using کلید_Status_Pair = std::pair<StringRef, کلید_Status>;

  // false on error
  منطقی check_Duplicate_Or_Unknown_کلید(yaml::گره *کلید_گره, StringRef کلید,
                                  DenseMap<StringRef, کلید_Status> &کلیدها_2) {
    auto It = کلیدها_2.find(کلید);
    اگر (It == کلیدها_2.end()) {
      error(کلید_گره, "unknown key");
      بازگشت نادرست;
    }
    کلید_Status &S = It->second;
    اگر (S.Seen) {
      error(کلید_گره, Twine("duplicate key '") + کلید + "'");
      بازگشت نادرست;
    }
    S.Seen = درست;
    بازگشت درست;
  }

  // false on error
  منطقی check_Missing_کلیدها(yaml::گره *Obj, DenseMap<StringRef, کلید_Status> &کلیدها_2) {
    برای (ثابت auto &I : کلیدها_2) {
      اگر (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        بازگشت نادرست;
      }
    }
    بازگشت درست;
  }

عمومی:
  ایستا RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    اگر (!ParentEntry) { // Look for a existent root
      برای (ثابت auto &ریشه_2 : FS->Roots) {
        اگر (Name == ریشه_2->getName()) {
          ParentEntry = ریشه_2.get();
          بازگشت ParentEntry;
        }
      }
    } وگرنه { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      برای (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        اگر (DirContent && Name == Content->getName())
          بازگشت DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    اگر (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.افزودن_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      بازگشت ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    بازگشت DE->getLastContent();
  }

خصوصی:
  تهی unique_Overlay_درخت(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    انتخاب (SrcE->getKind()) {
    حالت RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      اگر (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      برای (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_درخت(FS, SubEntry.get(), NewParentE);
      بشکن;
    }
    حالت RedirectingFileSystem::EK_DirectoryRemap: {
      أكد(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      بشکن;
    }
    حالت RedirectingFileSystem::EK_File: {
      أكد(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      بشکن;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::گره *N, RedirectingFileSystem *FS, منطقی Is_ریشه_Entry) {
    auto *M = dyn_cast<yaml::Mapping_گره>(N);
    اگر (!M) {
      error(N, "expected mapping node for file or directory entry");
      بازگشت nullptr;
    }

    کلید_Status_Pair Fields[] = {
        کلید_Status_Pair("name", درست),
        کلید_Status_Pair("type", درست),
        کلید_Status_Pair("contents", نادرست),
        کلید_Status_Pair("external-contents", نادرست),
        کلید_Status_Pair("use-external-name", نادرست),
    };

    DenseMap<StringRef, کلید_Status> کلیدها_2(std::begin(Fields), std::end(Fields));

    شمارش { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::گره *Name_مقدار_گره = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    برای (auto &I : *M) {
      StringRef کلید;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      اگر (!parseScalarString(I.get_کلید(), کلید, Buffer))
        بازگشت nullptr;

      اگر (!check_Duplicate_Or_Unknown_کلید(I.get_کلید(), کلید, کلیدها_2))
        بازگشت nullptr;

      StringRef مقدار;
      اگر (کلید == "name") {
        اگر (!parseScalarString(I.get_مقدار(), مقدار, Buffer))
          بازگشت nullptr;

        Name_مقدار_گره = I.get_مقدار();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(مقدار).str();
      } وگرنه اگر (کلید == "type") {
        اگر (!parseScalarString(I.get_مقدار(), مقدار, Buffer))
          بازگشت nullptr;
        اگر (مقدار == "file")
          Kind = RedirectingFileSystem::EK_File;
        وگرنه اگر (مقدار == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        وگرنه اگر (مقدار == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        وگرنه {
          error(I.get_مقدار(), "unknown value for 'type'");
          بازگشت nullptr;
        }
      } وگرنه اگر (کلید == "contents") {
        اگر (ContentsField != CF_NotSet) {
          error(I.get_کلید(),
                "entry already has 'contents' or 'external-contents'");
          بازگشت nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_گره>(I.get_مقدار());
        اگر (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_مقدار(), "expected array");
          بازگشت nullptr;
        }

        برای (auto &I : *Contents) {
          اگر (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ نادرست))
            EntryArrayContents.افزودن_back(std::move(E));
          وگرنه
            بازگشت nullptr;
        }
      } وگرنه اگر (کلید == "external-contents") {
        اگر (ContentsField != CF_NotSet) {
          error(I.get_کلید(),
                "entry already has 'contents' or 'external-contents'");
          بازگشت nullptr;
        }
        ContentsField = CF_External;
        اگر (!parseScalarString(I.get_مقدار(), مقدار, Buffer))
          بازگشت nullptr;

        SmallString<256> FullPath;
        اگر (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          أكد(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = مقدار;
          اگر (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            بازگشت nullptr;
          }
          FullPath = AbsFullPath;
        } وگرنه {
          FullPath = مقدار;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } وگرنه اگر (کلید == "use-external-name") {
        منطقی Val;
        اگر (!parseScalarBool(I.get_مقدار(), Val))
          بازگشت nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } وگرنه {
        llvm_unreachable("key missing from Keys");
      }
    }

    اگر (Stream.failed())
      بازگشت nullptr;

    // check for missing keys
    اگر (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      بازگشت nullptr;
    }
    اگر (!check_Missing_کلیدها(N, کلیدها_2))
      بازگشت nullptr;

    // check invalid configuration
    اگر (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      بازگشت nullptr;
    }

    اگر (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      بازگشت nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    اگر (Is_ریشه_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      اگر (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } وگرنه اگر (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } وگرنه {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        اگر (FS->ریشه_Relative ==
            RedirectingFileSystem::ریشه_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          أكد(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } وگرنه {
          EC = FS->makeAbsolute(Name);
        }
        اگر (EC) {
          أكد(Name_مقدار_گره && "Name presence should be checked earlier");
          error(
              Name_مقدار_گره,
              "entry with relative path at the root level is not discoverable");
          بازگشت nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      اگر (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t ریشه_Path_Len = sys::path::ریشه_path(Trimmed, path_style).size();
    وقتی (Trimmed.size() > ریشه_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    انتخاب (Kind) {
    حالت RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      بشکن;
    حالت RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      بشکن;
    حالت RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      بشکن;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    اگر (Parent.empty())
      بازگشت Result;

    // if 'name' contains multiple components, create implicit directory entries
    برای (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.افزودن_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    بازگشت Result;
  }

عمومی:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  منطقی parse(yaml::گره *ریشه_2, RedirectingFileSystem *FS) {
    auto *بالاترین = dyn_cast<yaml::Mapping_گره>(ریشه_2);
    اگر (!بالاترین) {
      error(ریشه_2, "expected mapping node");
      بازگشت نادرست;
    }

    کلید_Status_Pair Fields[] = {
        کلید_Status_Pair("version", درست),
        کلید_Status_Pair("case-sensitive", نادرست),
        کلید_Status_Pair("use-external-names", نادرست),
        کلید_Status_Pair("root-relative", نادرست),
        کلید_Status_Pair("overlay-relative", نادرست),
        کلید_Status_Pair("fallthrough", نادرست),
        کلید_Status_Pair("redirecting-with", نادرست),
        کلید_Status_Pair("roots", درست),
    };

    DenseMap<StringRef, کلید_Status> کلیدها_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> ریشه_Entries;

    // Parse configuration and 'roots'
    برای (auto &I : *بالاترین) {
      SmallString<10> کلید_Buffer;
      StringRef کلید;
      اگر (!parseScalarString(I.get_کلید(), کلید, کلید_Buffer))
        بازگشت نادرست;

      اگر (!check_Duplicate_Or_Unknown_کلید(I.get_کلید(), کلید, کلیدها_2))
        بازگشت نادرست;

      اگر (کلید == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_گره>(I.get_مقدار());
        اگر (!Roots) {
          error(I.get_مقدار(), "expected array");
          بازگشت نادرست;
        }

        برای (auto &I : *Roots) {
          اگر (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ درست))
            ریشه_Entries.افزودن_back(std::move(E));
          وگرنه
            بازگشت نادرست;
        }
      } وگرنه اگر (کلید == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        اگر (!parseScalarString(I.get_مقدار(), VersionString, Storage))
          بازگشت نادرست;
        صحیح Version;
        اگر (VersionString.getAsInteger<صحیح>(10, Version)) {
          error(I.get_مقدار(), "expected integer");
          بازگشت نادرست;
        }
        اگر (Version < 0) {
          error(I.get_مقدار(), "invalid version number");
          بازگشت نادرست;
        }
        اگر (Version != 0) {
          error(I.get_مقدار(), "version mismatch, expected 0");
          بازگشت نادرست;
        }
      } وگرنه اگر (کلید == "case-sensitive") {
        اگر (!parseScalarBool(I.get_مقدار(), FS->CaseSensitive))
          بازگشت نادرست;
      } وگرنه اگر (کلید == "overlay-relative") {
        اگر (!parseScalarBool(I.get_مقدار(), FS->IsRelativeOverlay))
          بازگشت نادرست;
      } وگرنه اگر (کلید == "use-external-names") {
        اگر (!parseScalarBool(I.get_مقدار(), FS->UseExternalNames))
          بازگشت نادرست;
      } وگرنه اگر (کلید == "fallthrough") {
        اگر (کلیدها_2["redirecting-with"].Seen) {
          error(I.get_مقدار(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          بازگشت نادرست;
        }

        منطقی ShouldFallthrough = نادرست;
        اگر (!parseScalarBool(I.get_مقدار(), ShouldFallthrough))
          بازگشت نادرست;

        اگر (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } وگرنه {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } وگرنه اگر (کلید == "redirecting-with") {
        اگر (کلیدها_2["fallthrough"].Seen) {
          error(I.get_مقدار(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          بازگشت نادرست;
        }

        اگر (auto Kind = parseRedirectKind(I.get_مقدار())) {
          FS->Redirection = *Kind;
        } وگرنه {
          error(I.get_مقدار(), "expected valid redirect kind");
          بازگشت نادرست;
        }
      } وگرنه اگر (کلید == "root-relative") {
        اگر (auto Kind = parse_ریشه_Relative_Kind(I.get_مقدار())) {
          FS->ریشه_Relative = *Kind;
        } وگرنه {
          error(I.get_مقدار(), "expected valid root-relative kind");
          بازگشت نادرست;
        }
      } وگرنه {
        llvm_unreachable("key missing from Keys");
      }
    }

    اگر (Stream.failed())
      بازگشت نادرست;

    اگر (!check_Missing_کلیدها(بالاترین, کلیدها_2))
      بازگشت نادرست;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    برای (auto &E : ریشه_Entries)
      unique_Overlay_درخت(FS, E.get());

    بازگشت درست;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, تهی *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::گره *ریشه_2 = DI->get_ریشه();
  اگر (DI == Stream.end() || !ریشه_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    بازگشت nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      جدید RedirectingFileSystem(ExternalFS));

  اگر (!YAMLFilePath.empty()) {
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
    (تهی)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  اگر (!P.parse(ریشه_2, FS.get()))
    بازگشت nullptr;

  بازگشت FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    منطقی UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      جدید RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  برای (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (تهی)EC;
      أكد(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    اگر (ToEntry)
      ادامه;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    برای (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    أكد(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (تهی)EC;
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

  بازگشت FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  أكد(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  اگر (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

تهی RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<نویسه> &Result) ثابت {
  Result.clear();
  برای (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<نویسه> &Path) ثابت {
  اگر (std::error_code EC = makeAbsolute(Path))
    بازگشت EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  اگر (CanonicalPath.empty())
    بازگشت make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  بازگشت {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) ثابت {
  llvm::SmallString<128> CanonicalPath(Path);
  اگر (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    بازگشت EC;

  // RedirectOnly means the VFS is always used.
  اگر (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = درست;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  برای (ثابت auto &ریشه_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, ریشه_2.get(), Entries);
    اگر (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = درست;
    اگر (Result) {
      Result->Parents = std::move(Entries);
      بازگشت Result;
    }

    اگر (Result.getError() != llvm::errc::no_such_file_or_directory)
      بازگشت Result;
  }
  بازگشت make_error_code(llvm::errc::no_such_file_or_directory);
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
  اگر (!FromName.empty()) {
    اگر (!pathComponentMatches(*Start, FromName))
      بازگشت make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    اگر (Start == End) {
      // Match!
      بازگشت LookupResult(From, Start, End);
    }
  }

  اگر (isa<RedirectingFileSystem::FileEntry>(From))
    بازگشت make_error_code(llvm::errc::not_a_directory);

  اگر (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    بازگشت LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  برای (ثابت std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.افزودن_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    اگر (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      بازگشت Result;
    Entries.برداشت_back();
  }

  بازگشت make_error_code(llvm::errc::no_such_file_or_directory);
}

ایستا Status getRedirectedFileStatus(ثابت Twine &OriginalPath,
                                      منطقی UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  اگر (ExternalStatus.ExposesExternalVFSPath)
    بازگشت ExternalStatus;

  Status S = ExternalStatus;
  اگر (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  وگرنه
    S.ExposesExternalVFSPath = درست;
  بازگشت S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    ثابت Twine &LookupPath, ثابت Twine &OriginalPath,
    ثابت RedirectingFileSystem::LookupResult &Result) {
  اگر (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    اگر (std::error_code EC = makeAbsolute(RemappedPath))
      بازگشت EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    اگر (!S)
      بازگشت S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    بازگشت getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  بازگشت Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(ثابت Twine &LookupPath,
                                         ثابت Twine &OriginalPath) ثابت {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  اگر (!Result || Result->ExposesExternalVFSPath)
    بازگشت Result;
  بازگشت Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  اگر (std::error_code EC = makeAbsolute(Path))
    بازگشت EC;

  اگر (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    اگر (S)
      بازگشت S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  اگر (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    اگر (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      بازگشت getExternalStatus(Path, OriginalPath);
    بازگشت Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  اگر (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    بازگشت getExternalStatus(Path, OriginalPath);
  }

  بازگشت S;
}

منطقی RedirectingFileSystem::exists(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  اگر (makeAbsolute(Path))
    بازگشت نادرست;

  اگر (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    اگر (ExternalFS->exists(Path))
      بازگشت درست;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  اگر (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    اگر (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      بازگشت ExternalFS->exists(Path);
    بازگشت نادرست;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  اگر (!ExtRedirect) {
    أكد(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    بازگشت درست;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  اگر (makeAbsolute(RemappedPath))
    بازگشت نادرست;

  اگر (ExternalFS->exists(RemappedPath))
    بازگشت درست;

  اگر (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    بازگشت ExternalFS->exists(Path);
  }

  بازگشت نادرست;
}

فضاینام {

/// Provide a file wrapper with an overriden status.
رده FileWithFixedStatus : عمومی File {
  std::unique_ptr<File> InnerFile;
  Status S;

عمومی:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { بازگشت S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(ثابت Twine &Name, int64_t FileSize, منطقی RequiresNullTerminator,
            منطقی IsVolatile) override {
    بازگشت InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { بازگشت InnerFile->close(); }

  تهی setPath(ثابت Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, ثابت Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  اگر (!Result || (*Result)->status()->ExposesExternalVFSPath)
    بازگشت Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  اگر (Name && Name.get() != P.str())
    F->get()->setPath(P);
  بازگشت F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ثابت Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  اگر (std::error_code EC = makeAbsolute(Path))
    بازگشت EC;

  اگر (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    اگر (F)
      بازگشت F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  اگر (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    اگر (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      بازگشت File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    بازگشت Result.getError();
  }

  اگر (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    بازگشت make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  اگر (std::error_code EC = makeAbsolute(RemappedPath))
    بازگشت EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  اگر (!ExternalFile) {
    اگر (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      بازگشت File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    بازگشت ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  اگر (!ExternalStatus)
    بازگشت ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  بازگشت std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(ثابت Twine &OriginalPath,
                                   SmallVectorImpl<نویسه> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  اگر (std::error_code EC = makeAbsolute(Path))
    بازگشت EC;

  اگر (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    اگر (!EC)
      بازگشت EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  اگر (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    اگر (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      بازگشت ExternalFS->getRealPath(Path, Output);
    بازگشت Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  اگر (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    اگر (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      بازگشت ExternalFS->getRealPath(Path, Output);
    }
    بازگشت P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  اگر (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    بازگشت {};
  }
  بازگشت llvm::errc::invalid_argument;
}

تهی RedirectingFileSystem::getDirectoryContentRealSources(
    ثابت Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  اگر (makeAbsolute(Path))
    بازگشت;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ثابت منطقی ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  اگر (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    اگر (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    بازگشت;
  }

  انتخاب (Result->E->getKind()) {
  حالت EK_File:
    بازگشت;
  حالت EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    بشکن;
  حالت EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    اگر (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    بشکن;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  اگر (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, تهی *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  بازگشت RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

ایستا تهی getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  اگر (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    أكد(DE && "Must be a directory");
    برای (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.افزودن_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.برداشت_back();
    }
    بازگشت;
  }

  اگر (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    أكد(DR && "Must be a directory remap");
    SmallString<128> VPath;
    برای (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.افزودن_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    بازگشت;
  }

  أكد(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  أكد(FE && "Must be a file");
  SmallString<128> VPath;
  برای (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.افزودن_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

تهی vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> ریشه_Result = VFS.lookupPath("/");
  اگر (!ریشه_Result)
    بازگشت;
  SmallVector<StringRef, 8> Components;
  Components.افزودن_back("/");
  getVFSEntries(ریشه_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  ایستا std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  بازگشت UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

تهی YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             منطقی IsDirectory) {
  أكد(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  أكد(sys::path::is_absolute(RealPath) && "real path not absolute");
  أكد(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

تهی YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/نادرست);
}

تهی YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/درست);
}

فضاینام {

رده JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_پشته;

  unsigned getDirIndent() { بازگشت 4 * Dir_پشته.size(); }
  unsigned getFileIndent() { بازگشت 4 * (Dir_پشته.size() + 1); }
  منطقی containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  تهی startDirectory(StringRef Path);
  تهی endDirectory();
  تهی writeEntry(StringRef VPath, StringRef RPath);

عمومی:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  تهی اكتب(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<منطقی> UseExternalNames,
             std::optional<منطقی> IsCaseSensitive,
             std::optional<منطقی> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

منطقی JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using فضاینام llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  برای (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    اگر (*IParent != *IChild)
      بازگشت نادرست;
  }
  // Have we exhausted the parent path?
  بازگشت IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  أكد(!Parent.empty());
  أكد(containedIn(Parent, Path));
  بازگشت Path.substr(Parent.size() + 1);
}

تهی JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_پشته.empty() ? Path : containedPart(Dir_پشته.back(), Path);
  Dir_پشته.افزودن_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

تهی JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_پشته.برداشت_back();
}

تهی JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

تهی JSONWriter::اكتب(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<منطقی> UseExternalNames,
                       std::optional<منطقی> IsCaseSensitive,
                       std::optional<منطقی> IsOverlayRelative,
                       StringRef OverlayDir) {
  using فضاینام llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  اگر (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  اگر (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  منطقی UseOverlayRelative = نادرست;
  اگر (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  اگر (!Entries.empty()) {
    ثابت YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    اگر (UseOverlayRelative) {
      أكد(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    منطقی IsCurrentDirEmpty = درست;
    اگر (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = نادرست;
    }

    برای (ثابت auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      اگر (Dir == Dir_پشته.back()) {
        اگر (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } وگرنه {
        منطقی Is_Dir_Popped_From_پشته = نادرست;
        وقتی (!Dir_پشته.empty() && !containedIn(Dir_پشته.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_پشته = درست;
        }
        اگر (Is_Dir_Popped_From_پشته || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = درست;
      }
      StringRef RPath = Entry.RPath;
      اگر (UseOverlayRelative) {
        أكد(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      اگر (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = نادرست;
      }
    }

    وقتی (!Dir_پشته.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

تهی YAMLVFSWriter::اكتب(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](ثابت YAMLVFSEntry &LHS, ثابت YAMLVFSEntry &RHS) {
    بازگشت LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).اكتب(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ثابت Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  اگر (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->پشته.افزودن_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  أكد(FS && State && !State->پشته.empty() && "incrementing past end");
  أكد(!State->پشته.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  اگر (State->Has_No_افزودن_Request)
    State->Has_No_افزودن_Request = نادرست;
  وگرنه {
    اگر (State->پشته.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->پشته.back()->path(), EC);
      اگر (I != End) {
        State->پشته.افزودن_back(I);
        بازگشت *این;
      }
    }
  }

  وقتی (!State->پشته.empty() && State->پشته.back().increment(EC) == End)
    State->پشته.برداشت_back();

  اگر (State->پشته.empty())
    State.reset(); // end iterator

  بازگشت *این;
}

ثابت نویسه FileSystem::ID = 0;
ثابت نویسه OverlayFileSystem::ID = 0;
ثابت نویسه ProxyFileSystem::ID = 0;
ثابت نویسه InMemoryFileSystem::ID = 0;
ثابت نویسه RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_شمار(
    ثابت FileSystem *FS) {
  بازگشت FS->Use_شمار();
}

تهی ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

تهی ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
