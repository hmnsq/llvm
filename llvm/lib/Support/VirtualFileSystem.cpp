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

#અન્તર્ભાવ "llvm/Support/VirtualFileSystem.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/ADT/DenseMap.h"
#અન્તર્ભાવ "llvm/ADT/IntrusiveRefCntPtr.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/StringSet.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"
#અન્તર્ભાવ "llvm/ADT/iterator_range.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/Casting.h"
#અન્તર્ભાવ "llvm/Support/Chrono.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/ErrorOr.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/FileSystem/UniqueID.h"
#અન્તર્ભાવ "llvm/Support/IOSandbox.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/SMLoc.h"
#અન્તર્ભાવ "llvm/Support/SourceMgr.h"
#અન્તર્ભાવ "llvm/Support/YAMLParser.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <atomic>
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstdint>
#અન્તર્ભાવ <iterator>
#અન્તર્ભાવ <limits>
#અન્તર્ભાવ <map>
#અન્તર્ભાવ <memory>
#અન્તર્ભાવ <optional>
#અન્તર્ભાવ <string>
#અન્તર્ભાવ <system_error>
#અન્તર્ભાવ <utility>
#અન્તર્ભાવ <vector>

using નામાકાશ llvm;
using નામાકાશ llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(અચલ file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(અચલ Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(અચલ Status &In, uint64_t NewSize) {
  પાછા Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(અચલ Status &In, અચલ Twine &NewName) {
  પાછા Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(અચલ file_status &In, અચલ Twine &NewName) {
  પાછા Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

તાર્કિક Status::equivalent(અચલ Status &Other) અચલ {
  પ્રતિજ્ઞા(isStatusKnown() && Other.isStatusKnown());
  પાછા getUniqueID() == Other.getUniqueID();
}

તાર્કિક Status::isDirectory() અચલ { પાછા Type == file_type::directory_file; }

તાર્કિક Status::isRegularFile() અચલ { પાછા Type == file_type::regular_file; }

તાર્કિક Status::isOther() અચલ {
  પાછા exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

તાર્કિક Status::isSymlink() અચલ { પાછા Type == file_type::symlink_file; }

તાર્કિક Status::isStatusKnown() અચલ { પાછા Type != file_type::status_error; }

તાર્કિક Status::exists() અચલ {
  પાછા isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = મૂળભૂત;

FileSystem::~FileSystem() = મૂળભૂત;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(અચલ llvm::Twine &Name, int64_t FileSize,
                             તાર્કિક RequiresNullTerminator, તાર્કિક IsVolatile,
                             તાર્કિક IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_દ્વિઅંકી(Name);
  જો (!F)
    પાછા F.getError();

  પાછા (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<અક્ષર> &Path) અચલ {
  જો (llvm::sys::path::is_absolute(Path))
    પાછા {};

  auto WorkingDir = getCurrentWorkingDirectory();
  જો (!WorkingDir)
    પાછા WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  પાછા {};
}

std::error_code FileSystem::getRealPath(અચલ Twine &Path,
                                        SmallVectorImpl<અક્ષર> &Output) {
  પાછા errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(અચલ Twine &Path, તાર્કિક &Result) {
  પાછા errc::operation_not_permitted;
}

તાર્કિક FileSystem::exists(અચલ Twine &Path) {
  auto Status = status(Path);
  પાછા Status && Status->exists();
}

llvm::ErrorOr<તાર્કિક> FileSystem::equivalent(અચલ Twine &A, અચલ Twine &B) {
  auto StatusA = status(A);
  જો (!StatusA)
    પાછા StatusA.getError();
  auto StatusB = status(B);
  જો (!StatusB)
    પાછા StatusB.getError();
  પાછા StatusA->equivalent(*StatusB);
}

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
શૂન્ય FileSystem::dump() અચલ { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
સ્થિર તાર્કિક isTraversalComponent(StringRef Component) {
  પાછા Component == ".." || Component == ".";
}

સ્થિર તાર્કિક pathHasTraversal(StringRef Path) {
  using નામાકાશ llvm::sys;

  માટે (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    જો (isTraversalComponent(Comp))
      પાછા સાચું;
  પાછા ખોટું;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

નામાકાશ {

/// Wrapper around a raw file descriptor.
વર્ગ RealFile : જાહેર File {
  friend વર્ગ RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    પ્રતિજ્ઞા(FD.isValid() && "Invalid or inactive file descriptor");
  }

જાહેર:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(અચલ Twine &Name,
                                                   int64_t FileSize,
                                                   તાર્કિક RequiresNullTerminator,
                                                   તાર્કિક IsVolatile) override;
  std::error_code close() override;
  શૂન્ય setPath(અચલ Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  પ્રતિજ્ઞા(FD.isValid() && "cannot stat closed file");
  જો (!S.isStatusKnown()) {
    file_status RealStatus;
    જો (std::error_code EC = sys::fs::status(FD, RealStatus))
      પાછા EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  પાછા S;
}

ErrorOr<std::string> RealFile::getName() {
  પાછા RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(અચલ Twine &Name, int64_t FileSize,
                    તાર્કિક RequiresNullTerminator, તાર્કિક IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  પ્રતિજ્ઞા(FD.isValid() && "cannot get buffer for closed file");
  પાછા MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  પાછા EC;
}

શૂન્ય RealFile::setPath(અચલ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  જો (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
}

નામાકાશ {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
વર્ગ RealFileSystem : જાહેર FileSystem {
જાહેર:
  explicit RealFileSystem(તાર્કિક LinkCWDToProcess) {
    જો (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      જો (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      નહીંતર જો (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      નહીંતર
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(અચલ Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(અચલ Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_દ્વિઅંકી(અચલ Twine &Path) override;
  directory_iterator dir_begin(અચલ Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() અચલ override;
  std::error_code setCurrentWorkingDirectory(અચલ Twine &Path) override;
  std::error_code isLocal(અચલ Twine &Path, તાર્કિક &Result) override;
  શૂન્ય
  getDirectoryContentRealSources(અચલ Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.ધકેલો_back(Dir.str());
  }
  std::error_code getRealPath(અચલ Twine &Path,
                              SmallVectorImpl<અક્ષર> &Output) override;

સુરક્ષિત:
  શૂન્ય printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) અચલ override;

ખાનગી:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(અચલ Twine &Path, SmallVectorImpl<અક્ષર> &Storage) અચલ {
    જો (!WD || !*WD)
      પાછા Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    પાછા Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(અચલ Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    જો (!FDOrErr)
      પાછા errorToErrorCode(FDOrErr.takeError());
    પાછા std::unique_ptr<File>(
        નવું RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  સંરચના WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(અચલ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  જો (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    પાછા EC;
  પાછા Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(અચલ Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  પાછા openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_દ્વિઅંકી(અચલ Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  પાછા openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() અચલ {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  જો (WD && *WD)
    પાછા std::string(WD->get().Specified);
  જો (WD)
    પાછા WD->getError();

  SmallString<128> Dir;
  જો (std::error_code EC = llvm::sys::fs::current_path(Dir))
    પાછા EC;
  પાછા std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(અચલ Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  જો (!WD)
    પાછા llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  તાર્કિક IsDir;
  જો (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    પાછા Err;
  જો (!IsDir)
    પાછા std::make_error_code(std::errc::not_a_directory);
  જો (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    પાછા Err;
  WD = WorkingDirectory{Absolute, Resolved};
  પાછા std::error_code();
}

std::error_code RealFileSystem::isLocal(અચલ Twine &Path, તાર્કિક &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  પાછા llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(અચલ Twine &Path,
                                            SmallVectorImpl<અક્ષર> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  પાછા llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

શૂન્ય RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) અચલ {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  જો (WD)
    OS << "own";
  નહીંતર
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  સ્થિર IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(સાચું);
  sys::sandbox::violationIfEnabled();

  પાછા FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  પાછા std::make_unique<RealFileSystem>(ખોટું);
}

નામાકાશ {

વર્ગ RealFSDirIter : જાહેર llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

જાહેર:
  RealFSDirIter(અચલ Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    જો (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    પાછા EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(અચલ Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  પાછા directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.ધકેલો_back(std::move(BaseFS));
}

શૂન્ય OverlayFileSystem::ધકેલો_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ધકેલો_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(અચલ Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  માટે (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    જો (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      પાછા Status;
  }
  પાછા make_error_code(llvm::errc::no_such_file_or_directory);
}

તાર્કિક OverlayFileSystem::exists(અચલ Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  માટે (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    જો ((*I)->exists(Path))
      પાછા સાચું;
  }
  પાછા ખોટું;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(અચલ llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  માટે (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    જો (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      પાછા Result;
  }
  પાછા make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() અચલ {
  // All file systems are synchronized, just take the first working directory.
  પાછા FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(અચલ Twine &Path) {
  માટે (auto &FS : FSList)
    જો (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      પાછા EC;
  પાછા {};
}

std::error_code OverlayFileSystem::isLocal(અચલ Twine &Path, તાર્કિક &Result) {
  માટે (auto &FS : FSList)
    જો (FS->exists(Path))
      પાછા FS->isLocal(Path, Result);
  પાછા errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(અચલ Twine &Path,
                                               SmallVectorImpl<અક્ષર> &Output) {
  માટે (અચલ auto &FS : FSList)
    જો (FS->exists(Path))
      પાછા FS->getRealPath(Path, Output);
  પાછા errc::no_such_file_or_directory;
}

શૂન્ય OverlayFileSystem::getDirectoryContentRealSources(
    અચલ Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  માટે (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

શૂન્ય OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  માટે (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

શૂન્ય OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) અચલ {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  જો (Type == PrintType::Summary)
    પાછા;

  જો (Type == PrintType::Contents)
    Type = PrintType::Summary;
  માટે (અચલ auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = મૂળભૂત;

નામાકાશ {

/// Combines and deduplicates directory entries across multiple file systems.
વર્ગ CombiningDirIterImpl : જાહેર llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(તાર્કિક IsFirstTime) {
    જ્યારે (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.કાઢો_back();
      જો (CurrentDirIter != directory_iterator())
        તોડો; // found
    }

    જો (IsFirstTime && CurrentDirIter == directory_iterator())
      પાછા errc::no_such_file_or_directory;
    પાછા {};
  }

  std::error_code incrementDirIter(તાર્કિક IsFirstTime) {
    પ્રતિજ્ઞા((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    જો (!IsFirstTime)
      CurrentDirIter.increment(EC);
    જો (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    પાછા EC;
  }

  std::error_code incrementImpl(તાર્કિક IsFirstTime) {
    જ્યારે (સાચું) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      જો (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        પાછા EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      જો (SeenNames.ઉમેરો(Name).second)
        પાછા EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

જાહેર:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    માટે (અચલ auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      જો (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        પાછા;
      }
      જો (!FEC)
        IterList.ધકેલો_back(Iter);
    }
    EC = incrementImpl(સાચું);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(સાચું);
  }

  std::error_code increment() override { પાછા incrementImpl(ખોટું); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(અચલ Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  જો (EC)
    પાછા {};
  પાછા Combined;
}

શૂન્ય ProxyFileSystem::anchor() {}

નામાકાશ llvm {
નામાકાશ vfs {

નામાકાશ detail {

ગણના In_Memory_ગાંઠ_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
વર્ગ In_Memory_ગાંઠ {
  In_Memory_ગાંઠ_Kind Kind;
  std::string FileName;

જાહેર:
  In_Memory_ગાંઠ(llvm::StringRef FileName, In_Memory_ગાંઠ_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  આભાસી ~In_Memory_ગાંઠ() = મૂળભૂત;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  આભાસી Status getStatus(અચલ Twine &RequestedName) અચલ = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() અચલ { પાછા FileName; }
  In_Memory_ગાંઠ_Kind getKind() અચલ { પાછા Kind; }
  આભાસી std::string toString(unsigned Indent) અચલ = 0;
};

વર્ગ InMemoryFile : જાહેર In_Memory_ગાંઠ {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

જાહેર:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_ગાંઠ(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(અચલ Twine &RequestedName) અચલ override {
    પાછા Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() અચલ { પાછા Buffer.get(); }

  std::string toString(unsigned Indent) અચલ override {
    પાછા (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  સ્થિર તાર્કિક classof(અચલ In_Memory_ગાંઠ *N) {
    પાછા N->getKind() == IME_File;
  }
};

નામાકાશ {

વર્ગ InMemoryHardLink : જાહેર In_Memory_ગાંઠ {
  અચલ InMemoryFile &ResolvedFile;

જાહેર:
  InMemoryHardLink(StringRef Path, અચલ InMemoryFile &ResolvedFile)
      : In_Memory_ગાંઠ(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  અચલ InMemoryFile &getResolvedFile() અચલ { પાછા ResolvedFile; }

  Status getStatus(અચલ Twine &RequestedName) અચલ override {
    પાછા ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) અચલ override {
    પાછા std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  સ્થિર તાર્કિક classof(અચલ In_Memory_ગાંઠ *N) {
    પાછા N->getKind() == IME_HardLink;
  }
};

વર્ગ InMemorySymbolicLink : જાહેર In_Memory_ગાંઠ {
  std::string TargetPath;
  Status Stat;

જાહેર:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_ગાંઠ(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) અચલ override {
    પાછા std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(અચલ Twine &RequestedName) અચલ override {
    પાછા Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() અચલ { પાછા TargetPath; }

  સ્થિર તાર્કિક classof(અચલ In_Memory_ગાંઠ *N) {
    પાછા N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
વર્ગ InMemoryFileAdaptor : જાહેર File {
  અચલ InMemoryFile &ગાંઠ;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

જાહેર:
  explicit InMemoryFileAdaptor(અચલ InMemoryFile &ગાંઠ,
                               std::string RequestedName)
      : ગાંઠ(ગાંઠ), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    પાછા ગાંઠ.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(અચલ Twine &Name, int64_t FileSize, તાર્કિક RequiresNullTerminator,
            તાર્કિક IsVolatile) override {
    llvm::MemoryBuffer *Buf = ગાંઠ.getBuffer();
    પાછા llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { પાછા {}; }

  શૂન્ય setPath(અચલ Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

વર્ગ InMemoryDirectory : જાહેર In_Memory_ગાંઠ {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_ગાંઠ>, std::less<>> Entries;

જાહેર:
  InMemoryDirectory(Status Stat)
      : In_Memory_ગાંઠ(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(અચલ Twine &RequestedName) અચલ override {
    પાછા Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() અચલ { પાછા Stat.getUniqueID(); }

  In_Memory_ગાંઠ *getChild(StringRef Name) અચલ {
    auto I = Entries.find(Name);
    જો (I != Entries.end())
      પાછા I->second.get();
    પાછા nullptr;
  }

  In_Memory_ગાંઠ *addChild(StringRef Name, std::unique_ptr<In_Memory_ગાંઠ> Child) {
    પાછા Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() અચલ { પાછા Entries.begin(); }
  const_iterator end() અચલ { પાછા Entries.end(); }

  std::string toString(unsigned Indent) અચલ override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    માટે (અચલ auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    પાછા Result;
  }

  સ્થિર તાર્કિક classof(અચલ In_Memory_ગાંઠ *N) {
    પાછા N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
સ્થિર sys::fs::UniqueID getUniqueID(hash_code Hash) {
  પાછા sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
સ્થિર sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  પાછા getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
સ્થિર sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  પાછા getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_ગાંઠ_Info::makeStatus() અચલ {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  પાછા Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(તાર્કિક UseNormalizedPaths)
    : મૂળ_2(નવું detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = મૂળભૂત;

std::string InMemoryFileSystem::toString() અચલ {
  પાછા મૂળ_2->toString(/*Indent=*/0);
}

તાર્કિક InMemoryFileSystem::addFile(અચલ Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_ગાંઠ_Fn Make_ગાંઠ) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  પ્રતિજ્ઞા(!EC);
  (શૂન્ય)EC;

  જો (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/સાચું);

  જો (Path.empty())
    પાછા ખોટું;

  detail::InMemoryDirectory *Dir = મૂળ_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  અચલ auto ResolvedUser = User.કિંમત_or(0);
  અચલ auto ResolvedGroup = Group.કિંમત_or(0);
  અચલ auto ResolvedType = Type.કિંમત_or(sys::fs::file_type::regular_file);
  અચલ auto ResolvedPerms = Perms.કિંમત_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  અચલ auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  જ્યારે (સાચું) {
    Name = *I;
    ++I;
    જો (I == E)
      તોડો;
    detail::In_Memory_ગાંઠ *ગાંઠ = Dir->getChild(Name);
    જો (!ગાંઠ) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      ચાલુ;
    }
    // Creating file under another file.
    જો (!isa<detail::InMemoryDirectory>(ગાંઠ))
      પાછા ખોટું;
    Dir = cast<detail::InMemoryDirectory>(ગાંઠ);
  }
  detail::In_Memory_ગાંઠ *ગાંઠ = Dir->getChild(Name);
  જો (!ગાંઠ) {
    Dir->addChild(Name,
                  Make_ગાંઠ({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    પાછા સાચું;
  }
  જો (isa<detail::InMemoryDirectory>(ગાંઠ))
    પાછા ResolvedType == sys::fs::file_type::directory_file;

  પ્રતિજ્ઞા((isa<detail::InMemoryFile>(ગાંઠ) ||
          isa<detail::InMemoryHardLink>(ગાંઠ)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  જો (auto *Link = dyn_cast<detail::InMemoryHardLink>(ગાંઠ)) {
    પાછા Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  પાછા cast<detail::InMemoryFile>(ગાંઠ)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

તાર્કિક InMemoryFileSystem::addFile(અચલ Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  પાછા addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_ગાંઠ_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_ગાંઠ> {
                   Status Stat = NNI.makeStatus();
                   જો (Stat.getType() == sys::fs::file_type::directory_file)
                     પાછા std::make_unique<detail::InMemoryDirectory>(Stat);
                   પાછા std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

તાર્કિક InMemoryFileSystem::addFileNoOwn(
    અચલ Twine &P, time_t ModificationTime,
    અચલ llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  પાછા addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_ગાંઠ_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_ગાંઠ> {
                   Status Stat = NNI.makeStatus();
                   જો (Stat.getType() == sys::fs::file_type::directory_file)
                     પાછા std::make_unique<detail::InMemoryDirectory>(Stat);
                   પાછા std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_ગાંઠ_Or_Error
InMemoryFileSystem::lookup_ગાંઠ(અચલ Twine &P, તાર્કિક FollowFinalSymlink,
                               size_t SymlinkDepth) અચલ {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  પ્રતિજ્ઞા(!EC);
  (શૂન્ય)EC;

  જો (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/સાચું);

  અચલ detail::InMemoryDirectory *Dir = મૂળ_2.get();
  જો (Path.empty())
    પાછા detail::Named_ગાંઠ_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  જ્યારે (સાચું) {
    detail::In_Memory_ગાંઠ *ગાંઠ = Dir->getChild(*I);
    ++I;
    જો (!ગાંઠ)
      પાછા errc::no_such_file_or_directory;

    જો (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(ગાંઠ)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      જો (I == E && !FollowFinalSymlink)
        પાછા detail::Named_ગાંઠ_Or_Error(Path, Symlink);

      જો (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        પાછા errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      જો (std::error_code EC = makeAbsolute(TargetPath))
        પાછા EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_ગાંઠ(TargetPath, /*FollowFinalSymlink=*/સાચું, SymlinkDepth + 1);
      જો (!Target || I == E)
        પાછા Target;

      જો (!isa<detail::InMemoryDirectory>(*Target))
        પાછા errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      ચાલુ;
    }

    // Return the file if it's at the end of the path.
    જો (auto File = dyn_cast<detail::InMemoryFile>(ગાંઠ)) {
      જો (I == E)
        પાછા detail::Named_ગાંઠ_Or_Error(Path, File);
      પાછા errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    જો (auto File = dyn_cast<detail::InMemoryHardLink>(ગાંઠ)) {
      જો (I == E)
        પાછા detail::Named_ગાંઠ_Or_Error(Path, &File->getResolvedFile());
      પાછા errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(ગાંઠ);
    જો (I == E)
      પાછા detail::Named_ગાંઠ_Or_Error(Path, Dir);
  }
}

તાર્કિક InMemoryFileSystem::addHardLink(અચલ Twine &NewLink,
                                     અચલ Twine &Target) {
  auto New_Link_ગાંઠ = lookup_ગાંઠ(NewLink, /*FollowFinalSymlink=*/ખોટું);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_ગાંઠ = lookup_ગાંઠ(Target, /*FollowFinalSymlink=*/સાચું);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  જો (!Target_ગાંઠ || New_Link_ગાંઠ || !isa<detail::InMemoryFile>(*Target_ગાંઠ))
    પાછા ખોટું;
  પાછા addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_ગાંઠ_Info NNI) {
                   પાછા std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_ગાંઠ));
                 });
}

તાર્કિક InMemoryFileSystem::addSymbolicLink(
    અચલ Twine &NewLink, અચલ Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_ગાંઠ = lookup_ગાંઠ(NewLink, /*FollowFinalSymlink=*/ખોટું);
  જો (New_Link_ગાંઠ)
    પાછા ખોટું;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  પાછા addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_ગાંઠ_Info NNI) {
                   પાછા std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(અચલ Twine &Path) {
  auto ગાંઠ = lookup_ગાંઠ(Path, /*FollowFinalSymlink=*/સાચું);
  જો (ગાંઠ)
    પાછા (*ગાંઠ)->getStatus(Path);
  પાછા ગાંઠ.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(અચલ Twine &Path) {
  auto ગાંઠ = lookup_ગાંઠ(Path,/*FollowFinalSymlink=*/સાચું);
  જો (!ગાંઠ)
    પાછા ગાંઠ.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  જો (auto *F = dyn_cast<detail::InMemoryFile>(*ગાંઠ))
    પાછા std::unique_ptr<File>(
        નવું detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  પાછા make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
વર્ગ InMemoryFileSystem::DirIterator : જાહેર llvm::vfs::detail::DirIterImpl {
  અચલ InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  શૂન્ય setCurrentEntry() {
    જો (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      પસંદગી (I->second->getKind()) {
      સ્થિતિ detail::IME_File:
      સ્થિતિ detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        તોડો;
      સ્થિતિ detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        તોડો;
      સ્થિતિ detail::IME_SymbolicLink:
        જો (auto SymlinkTarget =
                FS->lookup_ગાંઠ(Path, /*FollowFinalSymlink=*/સાચું)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        તોડો;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } નહીંતર {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

જાહેર:
  DirIterator() = મૂળભૂત;

  DirIterator(અચલ InMemoryFileSystem *FS,
              અચલ detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    પાછા {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(અચલ Twine &Dir,
                                                 std::error_code &EC) {
  auto ગાંઠ = lookup_ગાંઠ(Dir, /*FollowFinalSymlink=*/સાચું);
  જો (!ગાંઠ) {
    EC = ગાંઠ.getError();
    પાછા directory_iterator(std::make_shared<DirIterator>());
  }

  જો (auto *Dir_ગાંઠ = dyn_cast<detail::InMemoryDirectory>(*ગાંઠ))
    પાછા directory_iterator(
        std::make_shared<DirIterator>(આ, *Dir_ગાંઠ, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  પાછા directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(અચલ Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  પ્રતિજ્ઞા(!EC);
  (શૂન્ય)EC;

  જો (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/સાચું);

  જો (!Path.empty())
    WorkingDirectory = std::string(Path);
  પાછા {};
}

std::error_code InMemoryFileSystem::getRealPath(અચલ Twine &Path,
                                                SmallVectorImpl<અક્ષર> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  જો (!CWD || CWD->empty())
    પાછા errc::operation_not_permitted;
  Path.toVector(Output);
  જો (auto EC = makeAbsolute(Output))
    પાછા EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/સાચું);
  પાછા {};
}

std::error_code InMemoryFileSystem::isLocal(અચલ Twine &Path, તાર્કિક &Result) {
  Result = ખોટું;
  પાછા {};
}

શૂન્ય InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) અચલ {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

નામાકાશ {

સ્થિર llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  અચલ size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  જો (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  પાછા style;
}

/// Removes leading "./" as well as path components like ".." and ".".
સ્થિર llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/સાચું, style);
  પાછા result;
}

/// Whether the error and entry specify a file/directory that was not found.
સ્થિર તાર્કિક isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  જો (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    પાછા ખોટું;
  પાછા EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  પ્રતિજ્ઞા(ExternalFS && "RedirectingFileSystem requires an external FS");
  જો (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
વર્ગ llvm::vfs::RedirectingFSDirIterImpl
    : જાહેર llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(તાર્કિક IsFirstTime) {
    પ્રતિજ્ઞા((IsFirstTime || Current != End) && "cannot iterate past end");
    જો (!IsFirstTime)
      ++Current;
    જો (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      પસંદગી ((*Current)->getKind()) {
      સ્થિતિ RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      સ્થિતિ RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        તોડો;
      સ્થિતિ RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        તોડો;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } નહીંતર {
      CurrentEntry = directory_entry();
    }
    પાછા {};
  };

જાહેર:
  RedirectingFSDirIterImpl(
      અચલ Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/સાચું);
  }

  std::error_code increment() override {
    પાછા incrementImpl(/*IsFirstTime=*/ખોટું);
  }
};

નામાકાશ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
વર્ગ RedirectingFSDirRemapIterImpl : જાહેર llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

જાહેર:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    જો (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  શૂન્ય setCurrentEntry() {
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
    જો (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    નહીંતર
      CurrentEntry = directory_entry();
    પાછા EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() અચલ {
  પાછા WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(અચલ Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  જો (!exists(Path))
    પાછા errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  જો (std::error_code EC = makeAbsolute(AbsolutePath))
    પાછા EC;
  WorkingDirectory = std::string(AbsolutePath);
  પાછા {};
}

std::error_code RedirectingFileSystem::isLocal(અચલ Twine &Path_,
                                               તાર્કિક &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  જો (makeAbsolute(Path))
    પાછા {};

  પાછા ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<અક્ષર> &Path) અચલ {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  જો (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    પાછા {};

  auto WorkingDir = getCurrentWorkingDirectory();
  જો (!WorkingDir)
    પાછા WorkingDir.getError();

  પાછા makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<અક્ષર> &Path) અચલ {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  જો (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    પાછા std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  જો (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } નહીંતર {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    જો (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  જો (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  પાછા {};
}

directory_iterator RedirectingFileSystem::dir_begin(અચલ Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  જો (EC)
    પાછા {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  જો (!Result) {
    જો (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      પાછા ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    પાછા {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  જો (!S) {
    જો (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      પાછા ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    પાછા {};
  }

  જો (!S->isDirectory()) {
    EC = errc::not_a_directory;
    પાછા {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  જો (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    જો (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } નહીંતર {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  જો (RedirectEC) {
    જો (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      પાછા {};
    }
    RedirectIter = {};
  }

  જો (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    પાછા RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  જો (ExternalEC) {
    જો (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      પાછા {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  પસંદગી (Redirection) {
  સ્થિતિ RedirectKind::Fallthrough:
    Iters.ધકેલો_back(ExternalIter);
    Iters.ધકેલો_back(RedirectIter);
    તોડો;
  સ્થિતિ RedirectKind::Fallback:
    Iters.ધકેલો_back(RedirectIter);
    Iters.ધકેલો_back(ExternalIter);
    તોડો;
  મૂળભૂત:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  જો (EC)
    પાછા {};
  પાછા Combined;
}

શૂન્ય RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() અચલ {
  પાછા OverlayFileDir;
}

શૂન્ય RedirectingFileSystem::setFallthrough(તાર્કિક Fallthrough) {
  જો (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } નહીંતર {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

શૂન્ય RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() અચલ {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  માટે (અચલ auto &મૂળ_2 : Roots)
    R.ધકેલો_back(મૂળ_2->getName());
  પાછા R;
}

શૂન્ય RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) અચલ {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  જો (Type == PrintType::Summary)
    પાછા;

  માટે (અચલ auto &મૂળ_2 : Roots)
    printEntry(OS, મૂળ_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

શૂન્ય RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) અચલ {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  પસંદગી (E->getKind()) {
  સ્થિતિ EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    માટે (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    તોડો;
  }
  સ્થિતિ EK_DirectoryRemap:
  સ્થિતિ EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    પસંદગી (RE->getUseName()) {
    સ્થિતિ NK_NotSet:
      તોડો;
    સ્થિતિ NK_External:
      OS << " (UseExternalName: true)";
      તોડો;
    સ્થિતિ NK_Virtual:
      OS << " (UseExternalName: false)";
      તોડો;
    }
    OS << "\n";
    તોડો;
  }
  }
}

શૂન્ય RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  જો (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
વર્ગ llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  શૂન્ય error(yaml::ગાંઠ *N, અચલ Twine &સંદેશ_2) { Stream.printError(N, સંદેશ_2); }

  // false on error
  તાર્કિક parseScalarString(yaml::ગાંઠ *N, StringRef &Result,
                         SmallVectorImpl<અક્ષર> &Storage) {
    અચલ auto *S = dyn_cast_if_present<yaml::Scalar_ગાંઠ>(N);

    જો (!S) {
      error(N, "expected string");
      પાછા ખોટું;
    }
    Result = S->get_કિંમત(Storage);
    પાછા સાચું;
  }

  // false on error
  તાર્કિક parseScalarBool(yaml::ગાંઠ *N, તાર્કિક &Result) {
    SmallString<5> Storage;
    StringRef કિંમત;
    જો (!parseScalarString(N, કિંમત, Storage))
      પાછા ખોટું;

    જો (કિંમત.equals_insensitive("true") || કિંમત.equals_insensitive("on") ||
        કિંમત.equals_insensitive("yes") || કિંમત == "1") {
      Result = સાચું;
      પાછા સાચું;
    } નહીંતર જો (કિંમત.equals_insensitive("false") ||
               કિંમત.equals_insensitive("off") ||
               કિંમત.equals_insensitive("no") || કિંમત == "0") {
      Result = ખોટું;
      પાછા સાચું;
    }

    error(N, "expected boolean value");
    પાછા ખોટું;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::ગાંઠ *N) {
    SmallString<12> Storage;
    StringRef કિંમત;
    જો (!parseScalarString(N, કિંમત, Storage))
      પાછા std::nullopt;

    જો (કિંમત.equals_insensitive("fallthrough")) {
      પાછા RedirectingFileSystem::RedirectKind::Fallthrough;
    } નહીંતર જો (કિંમત.equals_insensitive("fallback")) {
      પાછા RedirectingFileSystem::RedirectKind::Fallback;
    } નહીંતર જો (કિંમત.equals_insensitive("redirect-only")) {
      પાછા RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    પાછા std::nullopt;
  }

  std::optional<RedirectingFileSystem::મૂળ_Relative_Kind>
  parse_મૂળ_Relative_Kind(yaml::ગાંઠ *N) {
    SmallString<12> Storage;
    StringRef કિંમત;
    જો (!parseScalarString(N, કિંમત, Storage))
      પાછા std::nullopt;
    જો (કિંમત.equals_insensitive("cwd")) {
      પાછા RedirectingFileSystem::મૂળ_Relative_Kind::CWD;
    } નહીંતર જો (કિંમત.equals_insensitive("overlay-dir")) {
      પાછા RedirectingFileSystem::મૂળ_Relative_Kind::OverlayDir;
    }
    પાછા std::nullopt;
  }

  સંરચના ચાવી_Status {
    તાર્કિક Required;
    તાર્કિક Seen = ખોટું;

    ચાવી_Status(તાર્કિક Required = ખોટું) : Required(Required) {}
  };

  using ચાવી_Status_Pair = std::pair<StringRef, ચાવી_Status>;

  // false on error
  તાર્કિક check_Duplicate_Or_Unknown_ચાવી(yaml::ગાંઠ *ચાવી_ગાંઠ, StringRef ચાવી,
                                  DenseMap<StringRef, ચાવી_Status> &ચાવીઓ_2) {
    auto It = ચાવીઓ_2.find(ચાવી);
    જો (It == ચાવીઓ_2.end()) {
      error(ચાવી_ગાંઠ, "unknown key");
      પાછા ખોટું;
    }
    ચાવી_Status &S = It->second;
    જો (S.Seen) {
      error(ચાવી_ગાંઠ, Twine("duplicate key '") + ચાવી + "'");
      પાછા ખોટું;
    }
    S.Seen = સાચું;
    પાછા સાચું;
  }

  // false on error
  તાર્કિક check_Missing_ચાવીઓ(yaml::ગાંઠ *Obj, DenseMap<StringRef, ચાવી_Status> &ચાવીઓ_2) {
    માટે (અચલ auto &I : ચાવીઓ_2) {
      જો (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        પાછા ખોટું;
      }
    }
    પાછા સાચું;
  }

જાહેર:
  સ્થિર RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    જો (!ParentEntry) { // Look for a existent root
      માટે (અચલ auto &મૂળ_2 : FS->Roots) {
        જો (Name == મૂળ_2->getName()) {
          ParentEntry = મૂળ_2.get();
          પાછા ParentEntry;
        }
      }
    } નહીંતર { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      માટે (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        જો (DirContent && Name == Content->getName())
          પાછા DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    જો (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.ધકેલો_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      પાછા ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    પાછા DE->getLastContent();
  }

ખાનગી:
  શૂન્ય unique_Overlay_વૃક્ષ(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    પસંદગી (SrcE->getKind()) {
    સ્થિતિ RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      જો (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      માટે (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_વૃક્ષ(FS, SubEntry.get(), NewParentE);
      તોડો;
    }
    સ્થિતિ RedirectingFileSystem::EK_DirectoryRemap: {
      પ્રતિજ્ઞા(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      તોડો;
    }
    સ્થિતિ RedirectingFileSystem::EK_File: {
      પ્રતિજ્ઞા(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      તોડો;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::ગાંઠ *N, RedirectingFileSystem *FS, તાર્કિક Is_મૂળ_Entry) {
    auto *M = dyn_cast<yaml::Mapping_ગાંઠ>(N);
    જો (!M) {
      error(N, "expected mapping node for file or directory entry");
      પાછા nullptr;
    }

    ચાવી_Status_Pair Fields[] = {
        ચાવી_Status_Pair("name", સાચું),
        ચાવી_Status_Pair("type", સાચું),
        ચાવી_Status_Pair("contents", ખોટું),
        ચાવી_Status_Pair("external-contents", ખોટું),
        ચાવી_Status_Pair("use-external-name", ખોટું),
    };

    DenseMap<StringRef, ચાવી_Status> ચાવીઓ_2(std::begin(Fields), std::end(Fields));

    ગણના { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::ગાંઠ *Name_કિંમત_ગાંઠ = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    માટે (auto &I : *M) {
      StringRef ચાવી;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      જો (!parseScalarString(I.get_ચાવી(), ચાવી, Buffer))
        પાછા nullptr;

      જો (!check_Duplicate_Or_Unknown_ચાવી(I.get_ચાવી(), ચાવી, ચાવીઓ_2))
        પાછા nullptr;

      StringRef કિંમત;
      જો (ચાવી == "name") {
        જો (!parseScalarString(I.get_કિંમત(), કિંમત, Buffer))
          પાછા nullptr;

        Name_કિંમત_ગાંઠ = I.get_કિંમત();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(કિંમત).str();
      } નહીંતર જો (ચાવી == "type") {
        જો (!parseScalarString(I.get_કિંમત(), કિંમત, Buffer))
          પાછા nullptr;
        જો (કિંમત == "file")
          Kind = RedirectingFileSystem::EK_File;
        નહીંતર જો (કિંમત == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        નહીંતર જો (કિંમત == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        નહીંતર {
          error(I.get_કિંમત(), "unknown value for 'type'");
          પાછા nullptr;
        }
      } નહીંતર જો (ચાવી == "contents") {
        જો (ContentsField != CF_NotSet) {
          error(I.get_ચાવી(),
                "entry already has 'contents' or 'external-contents'");
          પાછા nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_ગાંઠ>(I.get_કિંમત());
        જો (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_કિંમત(), "expected array");
          પાછા nullptr;
        }

        માટે (auto &I : *Contents) {
          જો (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ ખોટું))
            EntryArrayContents.ધકેલો_back(std::move(E));
          નહીંતર
            પાછા nullptr;
        }
      } નહીંતર જો (ચાવી == "external-contents") {
        જો (ContentsField != CF_NotSet) {
          error(I.get_ચાવી(),
                "entry already has 'contents' or 'external-contents'");
          પાછા nullptr;
        }
        ContentsField = CF_External;
        જો (!parseScalarString(I.get_કિંમત(), કિંમત, Buffer))
          પાછા nullptr;

        SmallString<256> FullPath;
        જો (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          પ્રતિજ્ઞા(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = કિંમત;
          જો (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            પાછા nullptr;
          }
          FullPath = AbsFullPath;
        } નહીંતર {
          FullPath = કિંમત;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } નહીંતર જો (ચાવી == "use-external-name") {
        તાર્કિક Val;
        જો (!parseScalarBool(I.get_કિંમત(), Val))
          પાછા nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } નહીંતર {
        llvm_unreachable("key missing from Keys");
      }
    }

    જો (Stream.failed())
      પાછા nullptr;

    // check for missing keys
    જો (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      પાછા nullptr;
    }
    જો (!check_Missing_ચાવીઓ(N, ચાવીઓ_2))
      પાછા nullptr;

    // check invalid configuration
    જો (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      પાછા nullptr;
    }

    જો (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      પાછા nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    જો (Is_મૂળ_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      જો (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } નહીંતર જો (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } નહીંતર {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        જો (FS->મૂળ_Relative ==
            RedirectingFileSystem::મૂળ_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          પ્રતિજ્ઞા(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } નહીંતર {
          EC = FS->makeAbsolute(Name);
        }
        જો (EC) {
          પ્રતિજ્ઞા(Name_કિંમત_ગાંઠ && "Name presence should be checked earlier");
          error(
              Name_કિંમત_ગાંઠ,
              "entry with relative path at the root level is not discoverable");
          પાછા nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      જો (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t મૂળ_Path_Len = sys::path::મૂળ_path(Trimmed, path_style).size();
    જ્યારે (Trimmed.size() > મૂળ_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    પસંદગી (Kind) {
    સ્થિતિ RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      તોડો;
    સ્થિતિ RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      તોડો;
    સ્થિતિ RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      તોડો;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    જો (Parent.empty())
      પાછા Result;

    // if 'name' contains multiple components, create implicit directory entries
    માટે (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.ધકેલો_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    પાછા Result;
  }

જાહેર:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  તાર્કિક parse(yaml::ગાંઠ *મૂળ_2, RedirectingFileSystem *FS) {
    auto *ટોચ = dyn_cast<yaml::Mapping_ગાંઠ>(મૂળ_2);
    જો (!ટોચ) {
      error(મૂળ_2, "expected mapping node");
      પાછા ખોટું;
    }

    ચાવી_Status_Pair Fields[] = {
        ચાવી_Status_Pair("version", સાચું),
        ચાવી_Status_Pair("case-sensitive", ખોટું),
        ચાવી_Status_Pair("use-external-names", ખોટું),
        ચાવી_Status_Pair("root-relative", ખોટું),
        ચાવી_Status_Pair("overlay-relative", ખોટું),
        ચાવી_Status_Pair("fallthrough", ખોટું),
        ચાવી_Status_Pair("redirecting-with", ખોટું),
        ચાવી_Status_Pair("roots", સાચું),
    };

    DenseMap<StringRef, ચાવી_Status> ચાવીઓ_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> મૂળ_Entries;

    // Parse configuration and 'roots'
    માટે (auto &I : *ટોચ) {
      SmallString<10> ચાવી_Buffer;
      StringRef ચાવી;
      જો (!parseScalarString(I.get_ચાવી(), ચાવી, ચાવી_Buffer))
        પાછા ખોટું;

      જો (!check_Duplicate_Or_Unknown_ચાવી(I.get_ચાવી(), ચાવી, ચાવીઓ_2))
        પાછા ખોટું;

      જો (ચાવી == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_ગાંઠ>(I.get_કિંમત());
        જો (!Roots) {
          error(I.get_કિંમત(), "expected array");
          પાછા ખોટું;
        }

        માટે (auto &I : *Roots) {
          જો (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ સાચું))
            મૂળ_Entries.ધકેલો_back(std::move(E));
          નહીંતર
            પાછા ખોટું;
        }
      } નહીંતર જો (ચાવી == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        જો (!parseScalarString(I.get_કિંમત(), VersionString, Storage))
          પાછા ખોટું;
        પૂર્ણાંક Version;
        જો (VersionString.getAsInteger<પૂર્ણાંક>(10, Version)) {
          error(I.get_કિંમત(), "expected integer");
          પાછા ખોટું;
        }
        જો (Version < 0) {
          error(I.get_કિંમત(), "invalid version number");
          પાછા ખોટું;
        }
        જો (Version != 0) {
          error(I.get_કિંમત(), "version mismatch, expected 0");
          પાછા ખોટું;
        }
      } નહીંતર જો (ચાવી == "case-sensitive") {
        જો (!parseScalarBool(I.get_કિંમત(), FS->CaseSensitive))
          પાછા ખોટું;
      } નહીંતર જો (ચાવી == "overlay-relative") {
        જો (!parseScalarBool(I.get_કિંમત(), FS->IsRelativeOverlay))
          પાછા ખોટું;
      } નહીંતર જો (ચાવી == "use-external-names") {
        જો (!parseScalarBool(I.get_કિંમત(), FS->UseExternalNames))
          પાછા ખોટું;
      } નહીંતર જો (ચાવી == "fallthrough") {
        જો (ચાવીઓ_2["redirecting-with"].Seen) {
          error(I.get_કિંમત(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          પાછા ખોટું;
        }

        તાર્કિક ShouldFallthrough = ખોટું;
        જો (!parseScalarBool(I.get_કિંમત(), ShouldFallthrough))
          પાછા ખોટું;

        જો (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } નહીંતર {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } નહીંતર જો (ચાવી == "redirecting-with") {
        જો (ચાવીઓ_2["fallthrough"].Seen) {
          error(I.get_કિંમત(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          પાછા ખોટું;
        }

        જો (auto Kind = parseRedirectKind(I.get_કિંમત())) {
          FS->Redirection = *Kind;
        } નહીંતર {
          error(I.get_કિંમત(), "expected valid redirect kind");
          પાછા ખોટું;
        }
      } નહીંતર જો (ચાવી == "root-relative") {
        જો (auto Kind = parse_મૂળ_Relative_Kind(I.get_કિંમત())) {
          FS->મૂળ_Relative = *Kind;
        } નહીંતર {
          error(I.get_કિંમત(), "expected valid root-relative kind");
          પાછા ખોટું;
        }
      } નહીંતર {
        llvm_unreachable("key missing from Keys");
      }
    }

    જો (Stream.failed())
      પાછા ખોટું;

    જો (!check_Missing_ચાવીઓ(ટોચ, ચાવીઓ_2))
      પાછા ખોટું;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    માટે (auto &E : મૂળ_Entries)
      unique_Overlay_વૃક્ષ(FS, E.get());

    પાછા સાચું;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, શૂન્ય *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::ગાંઠ *મૂળ_2 = DI->get_મૂળ();
  જો (DI == Stream.end() || !મૂળ_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    પાછા nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      નવું RedirectingFileSystem(ExternalFS));

  જો (!YAMLFilePath.empty()) {
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
    પ્રતિજ્ઞા(!EC && "Overlay dir final path must be absolute");
    (શૂન્ય)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  જો (!P.parse(મૂળ_2, FS.get()))
    પાછા nullptr;

  પાછા FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    તાર્કિક UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      નવું RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  માટે (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (શૂન્ય)EC;
      પ્રતિજ્ઞા(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    જો (ToEntry)
      ચાલુ;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    માટે (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    પ્રતિજ્ઞા(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (શૂન્ય)EC;
      પ્રતિજ્ઞા(!EC && "Could not make absolute path");
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

  પાછા FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  પ્રતિજ્ઞા(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  જો (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

શૂન્ય RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<અક્ષર> &Result) અચલ {
  Result.clear();
  માટે (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<અક્ષર> &Path) અચલ {
  જો (std::error_code EC = makeAbsolute(Path))
    પાછા EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  જો (CanonicalPath.empty())
    પાછા make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  પાછા {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) અચલ {
  llvm::SmallString<128> CanonicalPath(Path);
  જો (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    પાછા EC;

  // RedirectOnly means the VFS is always used.
  જો (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = સાચું;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  માટે (અચલ auto &મૂળ_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, મૂળ_2.get(), Entries);
    જો (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = સાચું;
    જો (Result) {
      Result->Parents = std::move(Entries);
      પાછા Result;
    }

    જો (Result.getError() != llvm::errc::no_such_file_or_directory)
      પાછા Result;
  }
  પાછા make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) અચલ {
  પ્રતિજ્ઞા(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  જો (!FromName.empty()) {
    જો (!pathComponentMatches(*Start, FromName))
      પાછા make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    જો (Start == End) {
      // Match!
      પાછા LookupResult(From, Start, End);
    }
  }

  જો (isa<RedirectingFileSystem::FileEntry>(From))
    પાછા make_error_code(llvm::errc::not_a_directory);

  જો (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    પાછા LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  માટે (અચલ std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.ધકેલો_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    જો (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      પાછા Result;
    Entries.કાઢો_back();
  }

  પાછા make_error_code(llvm::errc::no_such_file_or_directory);
}

સ્થિર Status getRedirectedFileStatus(અચલ Twine &OriginalPath,
                                      તાર્કિક UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  જો (ExternalStatus.ExposesExternalVFSPath)
    પાછા ExternalStatus;

  Status S = ExternalStatus;
  જો (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  નહીંતર
    S.ExposesExternalVFSPath = સાચું;
  પાછા S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    અચલ Twine &LookupPath, અચલ Twine &OriginalPath,
    અચલ RedirectingFileSystem::LookupResult &Result) {
  જો (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    જો (std::error_code EC = makeAbsolute(RemappedPath))
      પાછા EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    જો (!S)
      પાછા S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    પાછા getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  પાછા Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(અચલ Twine &LookupPath,
                                         અચલ Twine &OriginalPath) અચલ {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  જો (!Result || Result->ExposesExternalVFSPath)
    પાછા Result;
  પાછા Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(અચલ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  જો (std::error_code EC = makeAbsolute(Path))
    પાછા EC;

  જો (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    જો (S)
      પાછા S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  જો (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    જો (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      પાછા getExternalStatus(Path, OriginalPath);
    પાછા Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  જો (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    પાછા getExternalStatus(Path, OriginalPath);
  }

  પાછા S;
}

તાર્કિક RedirectingFileSystem::exists(અચલ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  જો (makeAbsolute(Path))
    પાછા ખોટું;

  જો (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    જો (ExternalFS->exists(Path))
      પાછા સાચું;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  જો (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    જો (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      પાછા ExternalFS->exists(Path);
    પાછા ખોટું;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  જો (!ExtRedirect) {
    પ્રતિજ્ઞા(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    પાછા સાચું;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  જો (makeAbsolute(RemappedPath))
    પાછા ખોટું;

  જો (ExternalFS->exists(RemappedPath))
    પાછા સાચું;

  જો (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    પાછા ExternalFS->exists(Path);
  }

  પાછા ખોટું;
}

નામાકાશ {

/// Provide a file wrapper with an overriden status.
વર્ગ FileWithFixedStatus : જાહેર File {
  std::unique_ptr<File> InnerFile;
  Status S;

જાહેર:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { પાછા S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(અચલ Twine &Name, int64_t FileSize, તાર્કિક RequiresNullTerminator,
            તાર્કિક IsVolatile) override {
    પાછા InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { પાછા InnerFile->close(); }

  શૂન્ય setPath(અચલ Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, અચલ Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  જો (!Result || (*Result)->status()->ExposesExternalVFSPath)
    પાછા Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  જો (Name && Name.get() != P.str())
    F->get()->setPath(P);
  પાછા F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(અચલ Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  જો (std::error_code EC = makeAbsolute(Path))
    પાછા EC;

  જો (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    જો (F)
      પાછા F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  જો (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    જો (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      પાછા File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    પાછા Result.getError();
  }

  જો (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    પાછા make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  જો (std::error_code EC = makeAbsolute(RemappedPath))
    પાછા EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  જો (!ExternalFile) {
    જો (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      પાછા File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    પાછા ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  જો (!ExternalStatus)
    પાછા ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  પાછા std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(અચલ Twine &OriginalPath,
                                   SmallVectorImpl<અક્ષર> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  જો (std::error_code EC = makeAbsolute(Path))
    પાછા EC;

  જો (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    જો (!EC)
      પાછા EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  જો (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    જો (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      પાછા ExternalFS->getRealPath(Path, Output);
    પાછા Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  જો (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    જો (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      પાછા ExternalFS->getRealPath(Path, Output);
    }
    પાછા P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  જો (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    પાછા {};
  }
  પાછા llvm::errc::invalid_argument;
}

શૂન્ય RedirectingFileSystem::getDirectoryContentRealSources(
    અચલ Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  જો (makeAbsolute(Path))
    પાછા;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  અચલ તાર્કિક ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  જો (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    જો (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    પાછા;
  }

  પસંદગી (Result->E->getKind()) {
  સ્થિતિ EK_File:
    પાછા;
  સ્થિતિ EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    તોડો;
  સ્થિતિ EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    જો (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    તોડો;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  જો (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, શૂન્ય *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  પાછા RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

સ્થિર શૂન્ય getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  જો (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    પ્રતિજ્ઞા(DE && "Must be a directory");
    માટે (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.ધકેલો_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.કાઢો_back();
    }
    પાછા;
  }

  જો (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    પ્રતિજ્ઞા(DR && "Must be a directory remap");
    SmallString<128> VPath;
    માટે (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.ધકેલો_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    પાછા;
  }

  પ્રતિજ્ઞા(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  પ્રતિજ્ઞા(FE && "Must be a file");
  SmallString<128> VPath;
  માટે (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.ધકેલો_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

શૂન્ય vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> મૂળ_Result = VFS.lookupPath("/");
  જો (!મૂળ_Result)
    પાછા;
  SmallVector<StringRef, 8> Components;
  Components.ધકેલો_back("/");
  getVFSEntries(મૂળ_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  સ્થિર std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  પાછા UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

શૂન્ય YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             તાર્કિક IsDirectory) {
  પ્રતિજ્ઞા(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  પ્રતિજ્ઞા(sys::path::is_absolute(RealPath) && "real path not absolute");
  પ્રતિજ્ઞા(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

શૂન્ય YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/ખોટું);
}

શૂન્ય YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/સાચું);
}

નામાકાશ {

વર્ગ JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_ઢગલો;

  unsigned getDirIndent() { પાછા 4 * Dir_ઢગલો.size(); }
  unsigned getFileIndent() { પાછા 4 * (Dir_ઢગલો.size() + 1); }
  તાર્કિક containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  શૂન્ય startDirectory(StringRef Path);
  શૂન્ય endDirectory();
  શૂન્ય writeEntry(StringRef VPath, StringRef RPath);

જાહેર:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  શૂન્ય લિખ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<તાર્કિક> UseExternalNames,
             std::optional<તાર્કિક> IsCaseSensitive,
             std::optional<તાર્કિક> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

તાર્કિક JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using નામાકાશ llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  માટે (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    જો (*IParent != *IChild)
      પાછા ખોટું;
  }
  // Have we exhausted the parent path?
  પાછા IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  પ્રતિજ્ઞા(!Parent.empty());
  પ્રતિજ્ઞા(containedIn(Parent, Path));
  પાછા Path.substr(Parent.size() + 1);
}

શૂન્ય JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_ઢગલો.empty() ? Path : containedPart(Dir_ઢગલો.back(), Path);
  Dir_ઢગલો.ધકેલો_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

શૂન્ય JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ઢગલો.કાઢો_back();
}

શૂન્ય JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

શૂન્ય JSONWriter::લિખ(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<તાર્કિક> UseExternalNames,
                       std::optional<તાર્કિક> IsCaseSensitive,
                       std::optional<તાર્કિક> IsOverlayRelative,
                       StringRef OverlayDir) {
  using નામાકાશ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  જો (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  જો (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  તાર્કિક UseOverlayRelative = ખોટું;
  જો (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  જો (!Entries.empty()) {
    અચલ YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    જો (UseOverlayRelative) {
      પ્રતિજ્ઞા(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    તાર્કિક IsCurrentDirEmpty = સાચું;
    જો (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = ખોટું;
    }

    માટે (અચલ auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      જો (Dir == Dir_ઢગલો.back()) {
        જો (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } નહીંતર {
        તાર્કિક Is_Dir_Popped_From_ઢગલો = ખોટું;
        જ્યારે (!Dir_ઢગલો.empty() && !containedIn(Dir_ઢગલો.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_ઢગલો = સાચું;
        }
        જો (Is_Dir_Popped_From_ઢગલો || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = સાચું;
      }
      StringRef RPath = Entry.RPath;
      જો (UseOverlayRelative) {
        પ્રતિજ્ઞા(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      જો (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = ખોટું;
      }
    }

    જ્યારે (!Dir_ઢગલો.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

શૂન્ય YAMLVFSWriter::લિખ(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](અચલ YAMLVFSEntry &LHS, અચલ YAMLVFSEntry &RHS) {
    પાછા LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).લિખ(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, અચલ Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  જો (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ઢગલો.ધકેલો_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  પ્રતિજ્ઞા(FS && State && !State->ઢગલો.empty() && "incrementing past end");
  પ્રતિજ્ઞા(!State->ઢગલો.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  જો (State->Has_No_ધકેલો_Request)
    State->Has_No_ધકેલો_Request = ખોટું;
  નહીંતર {
    જો (State->ઢગલો.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ઢગલો.back()->path(), EC);
      જો (I != End) {
        State->ઢગલો.ધકેલો_back(I);
        પાછા *આ;
      }
    }
  }

  જ્યારે (!State->ઢગલો.empty() && State->ઢગલો.back().increment(EC) == End)
    State->ઢગલો.કાઢો_back();

  જો (State->ઢગલો.empty())
    State.reset(); // end iterator

  પાછા *આ;
}

અચલ અક્ષર FileSystem::ID = 0;
અચલ અક્ષર OverlayFileSystem::ID = 0;
અચલ અક્ષર ProxyFileSystem::ID = 0;
અચલ અક્ષર InMemoryFileSystem::ID = 0;
અચલ અક્ષર RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_ગણતરી(
    અચલ FileSystem *FS) {
  પાછા FS->Use_ગણતરી();
}

શૂન્ય ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

શૂન્ય ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
