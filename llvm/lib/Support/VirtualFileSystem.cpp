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

#அந்தர்பாவ "llvm/Support/VirtualFileSystem.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/DenseMap.h"
#அந்தர்பாவ "llvm/ADT/IntrusiveRefCntPtr.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/StringSet.h"
#அந்தர்பாவ "llvm/ADT/Twine.h"
#அந்தர்பாவ "llvm/ADT/iterator_range.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/Casting.h"
#அந்தர்பாவ "llvm/Support/Chrono.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/ErrorOr.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/FileSystem/UniqueID.h"
#அந்தர்பாவ "llvm/Support/IOSandbox.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/SMLoc.h"
#அந்தர்பாவ "llvm/Support/SourceMgr.h"
#அந்தர்பாவ "llvm/Support/YAMLParser.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <atomic>
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstdint>
#அந்தர்பாவ <iterator>
#அந்தர்பாவ <limits>
#அந்தர்பாவ <map>
#அந்தர்பாவ <memory>
#அந்தர்பாவ <optional>
#அந்தர்பாவ <string>
#அந்தர்பாவ <system_error>
#அந்தர்பாவ <utility>
#அந்தர்பாவ <vector>

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(மாறிலி file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(மாறிலி Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(மாறிலி Status &In, uint64_t NewSize) {
  திரும்பு Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(மாறிலி Status &In, மாறிலி Twine &NewName) {
  திரும்பு Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(மாறிலி file_status &In, மாறிலி Twine &NewName) {
  திரும்பு Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

தருக்க Status::equivalent(மாறிலி Status &Other) மாறிலி {
  ப்ரதிஜ்ஞா(isStatusKnown() && Other.isStatusKnown());
  திரும்பு getUniqueID() == Other.getUniqueID();
}

தருக்க Status::isDirectory() மாறிலி { திரும்பு Type == file_type::directory_file; }

தருக்க Status::isRegularFile() மாறிலி { திரும்பு Type == file_type::regular_file; }

தருக்க Status::isOther() மாறிலி {
  திரும்பு exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

தருக்க Status::isSymlink() மாறிலி { திரும்பு Type == file_type::symlink_file; }

தருக்க Status::isStatusKnown() மாறிலி { திரும்பு Type != file_type::status_error; }

தருக்க Status::exists() மாறிலி {
  திரும்பு isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = இயல்பு;

FileSystem::~FileSystem() = இயல்பு;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(மாறிலி llvm::Twine &Name, int64_t FileSize,
                             தருக்க RequiresNullTerminator, தருக்க IsVolatile,
                             தருக்க IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_இருமம்(Name);
  எனில் (!F)
    திரும்பு F.getError();

  திரும்பு (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<எழுத்து> &Path) மாறிலி {
  எனில் (llvm::sys::path::is_absolute(Path))
    திரும்பு {};

  auto WorkingDir = getCurrentWorkingDirectory();
  எனில் (!WorkingDir)
    திரும்பு WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  திரும்பு {};
}

std::error_code FileSystem::getRealPath(மாறிலி Twine &Path,
                                        SmallVectorImpl<எழுத்து> &Output) {
  திரும்பு errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(மாறிலி Twine &Path, தருக்க &Result) {
  திரும்பு errc::operation_not_permitted;
}

தருக்க FileSystem::exists(மாறிலி Twine &Path) {
  auto Status = status(Path);
  திரும்பு Status && Status->exists();
}

llvm::ErrorOr<தருக்க> FileSystem::equivalent(மாறிலி Twine &A, மாறிலி Twine &B) {
  auto StatusA = status(A);
  எனில் (!StatusA)
    திரும்பு StatusA.getError();
  auto StatusB = status(B);
  எனில் (!StatusB)
    திரும்பு StatusB.getError();
  திரும்பு StatusA->equivalent(*StatusB);
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
வெறுமை FileSystem::dump() மாறிலி { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
நிலையான தருக்க isTraversalComponent(StringRef Component) {
  திரும்பு Component == ".." || Component == ".";
}

நிலையான தருக்க pathHasTraversal(StringRef Path) {
  using நாமாகாஶ llvm::sys;

  க்கு (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    எனில் (isTraversalComponent(Comp))
      திரும்பு உண்மை;
  திரும்பு பொய்;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

நாமாகாஶ {

/// Wrapper around a raw file descriptor.
வகுப்பு RealFile : பொது File {
  friend வகுப்பு RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    ப்ரதிஜ்ஞா(FD.isValid() && "Invalid or inactive file descriptor");
  }

பொது:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(மாறிலி Twine &Name,
                                                   int64_t FileSize,
                                                   தருக்க RequiresNullTerminator,
                                                   தருக்க IsVolatile) override;
  std::error_code close() override;
  வெறுமை setPath(மாறிலி Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ப்ரதிஜ்ஞா(FD.isValid() && "cannot stat closed file");
  எனில் (!S.isStatusKnown()) {
    file_status RealStatus;
    எனில் (std::error_code EC = sys::fs::status(FD, RealStatus))
      திரும்பு EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  திரும்பு S;
}

ErrorOr<std::string> RealFile::getName() {
  திரும்பு RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(மாறிலி Twine &Name, int64_t FileSize,
                    தருக்க RequiresNullTerminator, தருக்க IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ப்ரதிஜ்ஞா(FD.isValid() && "cannot get buffer for closed file");
  திரும்பு MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  திரும்பு EC;
}

வெறுமை RealFile::setPath(மாறிலி Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  எனில் (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
}

நாமாகாஶ {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
வகுப்பு RealFileSystem : பொது FileSystem {
பொது:
  explicit RealFileSystem(தருக்க LinkCWDToProcess) {
    எனில் (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      எனில் (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      இல்லையெனில் எனில் (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      இல்லையெனில்
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(மாறிலி Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(மாறிலி Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_இருமம்(மாறிலி Twine &Path) override;
  directory_iterator dir_begin(மாறிலி Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() மாறிலி override;
  std::error_code setCurrentWorkingDirectory(மாறிலி Twine &Path) override;
  std::error_code isLocal(மாறிலி Twine &Path, தருக்க &Result) override;
  வெறுமை
  getDirectoryContentRealSources(மாறிலி Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.தள்ளு_back(Dir.str());
  }
  std::error_code getRealPath(மாறிலி Twine &Path,
                              SmallVectorImpl<எழுத்து> &Output) override;

பாதுகாக்கப்பட்ட:
  வெறுமை printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) மாறிலி override;

தனிப்பட்ட:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(மாறிலி Twine &Path, SmallVectorImpl<எழுத்து> &Storage) மாறிலி {
    எனில் (!WD || !*WD)
      திரும்பு Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    திரும்பு Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(மாறிலி Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    எனில் (!FDOrErr)
      திரும்பு errorToErrorCode(FDOrErr.takeError());
    திரும்பு std::unique_ptr<File>(
        புதிய RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  கட்டமைப்பு WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(மாறிலி Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  எனில் (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    திரும்பு EC;
  திரும்பு Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(மாறிலி Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  திரும்பு openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_இருமம்(மாறிலி Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  திரும்பு openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() மாறிலி {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  எனில் (WD && *WD)
    திரும்பு std::string(WD->get().Specified);
  எனில் (WD)
    திரும்பு WD->getError();

  SmallString<128> Dir;
  எனில் (std::error_code EC = llvm::sys::fs::current_path(Dir))
    திரும்பு EC;
  திரும்பு std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(மாறிலி Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  எனில் (!WD)
    திரும்பு llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  தருக்க IsDir;
  எனில் (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    திரும்பு Err;
  எனில் (!IsDir)
    திரும்பு std::make_error_code(std::errc::not_a_directory);
  எனில் (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    திரும்பு Err;
  WD = WorkingDirectory{Absolute, Resolved};
  திரும்பு std::error_code();
}

std::error_code RealFileSystem::isLocal(மாறிலி Twine &Path, தருக்க &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  திரும்பு llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(மாறிலி Twine &Path,
                                            SmallVectorImpl<எழுத்து> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  திரும்பு llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

வெறுமை RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) மாறிலி {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  எனில் (WD)
    OS << "own";
  இல்லையெனில்
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  நிலையான IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(உண்மை);
  sys::sandbox::violationIfEnabled();

  திரும்பு FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  திரும்பு std::make_unique<RealFileSystem>(பொய்);
}

நாமாகாஶ {

வகுப்பு RealFSDirIter : பொது llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

பொது:
  RealFSDirIter(மாறிலி Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    எனில் (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    திரும்பு EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(மாறிலி Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  திரும்பு directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.தள்ளு_back(std::move(BaseFS));
}

வெறுமை OverlayFileSystem::தள்ளு_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.தள்ளு_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(மாறிலி Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  க்கு (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    எனில் (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      திரும்பு Status;
  }
  திரும்பு make_error_code(llvm::errc::no_such_file_or_directory);
}

தருக்க OverlayFileSystem::exists(மாறிலி Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  க்கு (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    எனில் ((*I)->exists(Path))
      திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(மாறிலி llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  க்கு (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    எனில் (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      திரும்பு Result;
  }
  திரும்பு make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() மாறிலி {
  // All file systems are synchronized, just take the first working directory.
  திரும்பு FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(மாறிலி Twine &Path) {
  க்கு (auto &FS : FSList)
    எனில் (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      திரும்பு EC;
  திரும்பு {};
}

std::error_code OverlayFileSystem::isLocal(மாறிலி Twine &Path, தருக்க &Result) {
  க்கு (auto &FS : FSList)
    எனில் (FS->exists(Path))
      திரும்பு FS->isLocal(Path, Result);
  திரும்பு errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(மாறிலி Twine &Path,
                                               SmallVectorImpl<எழுத்து> &Output) {
  க்கு (மாறிலி auto &FS : FSList)
    எனில் (FS->exists(Path))
      திரும்பு FS->getRealPath(Path, Output);
  திரும்பு errc::no_such_file_or_directory;
}

வெறுமை OverlayFileSystem::getDirectoryContentRealSources(
    மாறிலி Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  க்கு (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

வெறுமை OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  க்கு (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

வெறுமை OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) மாறிலி {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  எனில் (Type == PrintType::Summary)
    திரும்பு;

  எனில் (Type == PrintType::Contents)
    Type = PrintType::Summary;
  க்கு (மாறிலி auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = இயல்பு;

நாமாகாஶ {

/// Combines and deduplicates directory entries across multiple file systems.
வகுப்பு CombiningDirIterImpl : பொது llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(தருக்க IsFirstTime) {
    போது (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.எடு_back();
      எனில் (CurrentDirIter != directory_iterator())
        நிறுத்து; // found
    }

    எனில் (IsFirstTime && CurrentDirIter == directory_iterator())
      திரும்பு errc::no_such_file_or_directory;
    திரும்பு {};
  }

  std::error_code incrementDirIter(தருக்க IsFirstTime) {
    ப்ரதிஜ்ஞா((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    எனில் (!IsFirstTime)
      CurrentDirIter.increment(EC);
    எனில் (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    திரும்பு EC;
  }

  std::error_code incrementImpl(தருக்க IsFirstTime) {
    போது (உண்மை) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      எனில் (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        திரும்பு EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      எனில் (SeenNames.செருகு(Name).second)
        திரும்பு EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

பொது:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    க்கு (மாறிலி auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      எனில் (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        திரும்பு;
      }
      எனில் (!FEC)
        IterList.தள்ளு_back(Iter);
    }
    EC = incrementImpl(உண்மை);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(உண்மை);
  }

  std::error_code increment() override { திரும்பு incrementImpl(பொய்); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(மாறிலி Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  எனில் (EC)
    திரும்பு {};
  திரும்பு Combined;
}

வெறுமை ProxyFileSystem::anchor() {}

நாமாகாஶ llvm {
நாமாகாஶ vfs {

நாமாகாஶ detail {

எண்ணிக்கை In_Memory_முனை_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
வகுப்பு In_Memory_முனை {
  In_Memory_முனை_Kind Kind;
  std::string FileName;

பொது:
  In_Memory_முனை(llvm::StringRef FileName, In_Memory_முனை_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  மெய்நிகர் ~In_Memory_முனை() = இயல்பு;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  மெய்நிகர் Status getStatus(மாறிலி Twine &RequestedName) மாறிலி = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() மாறிலி { திரும்பு FileName; }
  In_Memory_முனை_Kind getKind() மாறிலி { திரும்பு Kind; }
  மெய்நிகர் std::string toString(unsigned Indent) மாறிலி = 0;
};

வகுப்பு InMemoryFile : பொது In_Memory_முனை {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

பொது:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_முனை(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(மாறிலி Twine &RequestedName) மாறிலி override {
    திரும்பு Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() மாறிலி { திரும்பு Buffer.get(); }

  std::string toString(unsigned Indent) மாறிலி override {
    திரும்பு (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  நிலையான தருக்க classof(மாறிலி In_Memory_முனை *N) {
    திரும்பு N->getKind() == IME_File;
  }
};

நாமாகாஶ {

வகுப்பு InMemoryHardLink : பொது In_Memory_முனை {
  மாறிலி InMemoryFile &ResolvedFile;

பொது:
  InMemoryHardLink(StringRef Path, மாறிலி InMemoryFile &ResolvedFile)
      : In_Memory_முனை(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  மாறிலி InMemoryFile &getResolvedFile() மாறிலி { திரும்பு ResolvedFile; }

  Status getStatus(மாறிலி Twine &RequestedName) மாறிலி override {
    திரும்பு ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) மாறிலி override {
    திரும்பு std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  நிலையான தருக்க classof(மாறிலி In_Memory_முனை *N) {
    திரும்பு N->getKind() == IME_HardLink;
  }
};

வகுப்பு InMemorySymbolicLink : பொது In_Memory_முனை {
  std::string TargetPath;
  Status Stat;

பொது:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_முனை(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) மாறிலி override {
    திரும்பு std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(மாறிலி Twine &RequestedName) மாறிலி override {
    திரும்பு Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() மாறிலி { திரும்பு TargetPath; }

  நிலையான தருக்க classof(மாறிலி In_Memory_முனை *N) {
    திரும்பு N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
வகுப்பு InMemoryFileAdaptor : பொது File {
  மாறிலி InMemoryFile &முனை;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

பொது:
  explicit InMemoryFileAdaptor(மாறிலி InMemoryFile &முனை,
                               std::string RequestedName)
      : முனை(முனை), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    திரும்பு முனை.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(மாறிலி Twine &Name, int64_t FileSize, தருக்க RequiresNullTerminator,
            தருக்க IsVolatile) override {
    llvm::MemoryBuffer *Buf = முனை.getBuffer();
    திரும்பு llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { திரும்பு {}; }

  வெறுமை setPath(மாறிலி Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

வகுப்பு InMemoryDirectory : பொது In_Memory_முனை {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_முனை>, std::less<>> Entries;

பொது:
  InMemoryDirectory(Status Stat)
      : In_Memory_முனை(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(மாறிலி Twine &RequestedName) மாறிலி override {
    திரும்பு Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() மாறிலி { திரும்பு Stat.getUniqueID(); }

  In_Memory_முனை *getChild(StringRef Name) மாறிலி {
    auto I = Entries.find(Name);
    எனில் (I != Entries.end())
      திரும்பு I->second.get();
    திரும்பு nullptr;
  }

  In_Memory_முனை *addChild(StringRef Name, std::unique_ptr<In_Memory_முனை> Child) {
    திரும்பு Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() மாறிலி { திரும்பு Entries.begin(); }
  const_iterator end() மாறிலி { திரும்பு Entries.end(); }

  std::string toString(unsigned Indent) மாறிலி override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    க்கு (மாறிலி auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    திரும்பு Result;
  }

  நிலையான தருக்க classof(மாறிலி In_Memory_முனை *N) {
    திரும்பு N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
நிலையான sys::fs::UniqueID getUniqueID(hash_code Hash) {
  திரும்பு sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
நிலையான sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  திரும்பு getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
நிலையான sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  திரும்பு getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_முனை_Info::makeStatus() மாறிலி {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  திரும்பு Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(தருக்க UseNormalizedPaths)
    : வேர்_2(புதிய detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = இயல்பு;

std::string InMemoryFileSystem::toString() மாறிலி {
  திரும்பு வேர்_2->toString(/*Indent=*/0);
}

தருக்க InMemoryFileSystem::addFile(மாறிலி Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_முனை_Fn Make_முனை) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ப்ரதிஜ்ஞா(!EC);
  (வெறுமை)EC;

  எனில் (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/உண்மை);

  எனில் (Path.empty())
    திரும்பு பொய்;

  detail::InMemoryDirectory *Dir = வேர்_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  மாறிலி auto ResolvedUser = User.மதிப்பு_or(0);
  மாறிலி auto ResolvedGroup = Group.மதிப்பு_or(0);
  மாறிலி auto ResolvedType = Type.மதிப்பு_or(sys::fs::file_type::regular_file);
  மாறிலி auto ResolvedPerms = Perms.மதிப்பு_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  மாறிலி auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  போது (உண்மை) {
    Name = *I;
    ++I;
    எனில் (I == E)
      நிறுத்து;
    detail::In_Memory_முனை *முனை = Dir->getChild(Name);
    எனில் (!முனை) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      தொடர்;
    }
    // Creating file under another file.
    எனில் (!isa<detail::InMemoryDirectory>(முனை))
      திரும்பு பொய்;
    Dir = cast<detail::InMemoryDirectory>(முனை);
  }
  detail::In_Memory_முனை *முனை = Dir->getChild(Name);
  எனில் (!முனை) {
    Dir->addChild(Name,
                  Make_முனை({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    திரும்பு உண்மை;
  }
  எனில் (isa<detail::InMemoryDirectory>(முனை))
    திரும்பு ResolvedType == sys::fs::file_type::directory_file;

  ப்ரதிஜ்ஞா((isa<detail::InMemoryFile>(முனை) ||
          isa<detail::InMemoryHardLink>(முனை)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  எனில் (auto *Link = dyn_cast<detail::InMemoryHardLink>(முனை)) {
    திரும்பு Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  திரும்பு cast<detail::InMemoryFile>(முனை)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

தருக்க InMemoryFileSystem::addFile(மாறிலி Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  திரும்பு addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_முனை_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_முனை> {
                   Status Stat = NNI.makeStatus();
                   எனில் (Stat.getType() == sys::fs::file_type::directory_file)
                     திரும்பு std::make_unique<detail::InMemoryDirectory>(Stat);
                   திரும்பு std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

தருக்க InMemoryFileSystem::addFileNoOwn(
    மாறிலி Twine &P, time_t ModificationTime,
    மாறிலி llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  திரும்பு addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_முனை_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_முனை> {
                   Status Stat = NNI.makeStatus();
                   எனில் (Stat.getType() == sys::fs::file_type::directory_file)
                     திரும்பு std::make_unique<detail::InMemoryDirectory>(Stat);
                   திரும்பு std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_முனை_Or_Error
InMemoryFileSystem::lookup_முனை(மாறிலி Twine &P, தருக்க FollowFinalSymlink,
                               size_t SymlinkDepth) மாறிலி {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ப்ரதிஜ்ஞா(!EC);
  (வெறுமை)EC;

  எனில் (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/உண்மை);

  மாறிலி detail::InMemoryDirectory *Dir = வேர்_2.get();
  எனில் (Path.empty())
    திரும்பு detail::Named_முனை_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  போது (உண்மை) {
    detail::In_Memory_முனை *முனை = Dir->getChild(*I);
    ++I;
    எனில் (!முனை)
      திரும்பு errc::no_such_file_or_directory;

    எனில் (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(முனை)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      எனில் (I == E && !FollowFinalSymlink)
        திரும்பு detail::Named_முனை_Or_Error(Path, Symlink);

      எனில் (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        திரும்பு errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      எனில் (std::error_code EC = makeAbsolute(TargetPath))
        திரும்பு EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_முனை(TargetPath, /*FollowFinalSymlink=*/உண்மை, SymlinkDepth + 1);
      எனில் (!Target || I == E)
        திரும்பு Target;

      எனில் (!isa<detail::InMemoryDirectory>(*Target))
        திரும்பு errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      தொடர்;
    }

    // Return the file if it's at the end of the path.
    எனில் (auto File = dyn_cast<detail::InMemoryFile>(முனை)) {
      எனில் (I == E)
        திரும்பு detail::Named_முனை_Or_Error(Path, File);
      திரும்பு errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    எனில் (auto File = dyn_cast<detail::InMemoryHardLink>(முனை)) {
      எனில் (I == E)
        திரும்பு detail::Named_முனை_Or_Error(Path, &File->getResolvedFile());
      திரும்பு errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(முனை);
    எனில் (I == E)
      திரும்பு detail::Named_முனை_Or_Error(Path, Dir);
  }
}

தருக்க InMemoryFileSystem::addHardLink(மாறிலி Twine &NewLink,
                                     மாறிலி Twine &Target) {
  auto New_Link_முனை = lookup_முனை(NewLink, /*FollowFinalSymlink=*/பொய்);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_முனை = lookup_முனை(Target, /*FollowFinalSymlink=*/உண்மை);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  எனில் (!Target_முனை || New_Link_முனை || !isa<detail::InMemoryFile>(*Target_முனை))
    திரும்பு பொய்;
  திரும்பு addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_முனை_Info NNI) {
                   திரும்பு std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_முனை));
                 });
}

தருக்க InMemoryFileSystem::addSymbolicLink(
    மாறிலி Twine &NewLink, மாறிலி Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_முனை = lookup_முனை(NewLink, /*FollowFinalSymlink=*/பொய்);
  எனில் (New_Link_முனை)
    திரும்பு பொய்;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  திரும்பு addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_முனை_Info NNI) {
                   திரும்பு std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(மாறிலி Twine &Path) {
  auto முனை = lookup_முனை(Path, /*FollowFinalSymlink=*/உண்மை);
  எனில் (முனை)
    திரும்பு (*முனை)->getStatus(Path);
  திரும்பு முனை.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(மாறிலி Twine &Path) {
  auto முனை = lookup_முனை(Path,/*FollowFinalSymlink=*/உண்மை);
  எனில் (!முனை)
    திரும்பு முனை.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  எனில் (auto *F = dyn_cast<detail::InMemoryFile>(*முனை))
    திரும்பு std::unique_ptr<File>(
        புதிய detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  திரும்பு make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
வகுப்பு InMemoryFileSystem::DirIterator : பொது llvm::vfs::detail::DirIterImpl {
  மாறிலி InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  வெறுமை setCurrentEntry() {
    எனில் (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      தேர்வு (I->second->getKind()) {
      நிலை detail::IME_File:
      நிலை detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        நிறுத்து;
      நிலை detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        நிறுத்து;
      நிலை detail::IME_SymbolicLink:
        எனில் (auto SymlinkTarget =
                FS->lookup_முனை(Path, /*FollowFinalSymlink=*/உண்மை)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        நிறுத்து;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } இல்லையெனில் {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

பொது:
  DirIterator() = இயல்பு;

  DirIterator(மாறிலி InMemoryFileSystem *FS,
              மாறிலி detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    திரும்பு {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(மாறிலி Twine &Dir,
                                                 std::error_code &EC) {
  auto முனை = lookup_முனை(Dir, /*FollowFinalSymlink=*/உண்மை);
  எனில் (!முனை) {
    EC = முனை.getError();
    திரும்பு directory_iterator(std::make_shared<DirIterator>());
  }

  எனில் (auto *Dir_முனை = dyn_cast<detail::InMemoryDirectory>(*முனை))
    திரும்பு directory_iterator(
        std::make_shared<DirIterator>(இது, *Dir_முனை, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  திரும்பு directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(மாறிலி Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  ப்ரதிஜ்ஞா(!EC);
  (வெறுமை)EC;

  எனில் (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/உண்மை);

  எனில் (!Path.empty())
    WorkingDirectory = std::string(Path);
  திரும்பு {};
}

std::error_code InMemoryFileSystem::getRealPath(மாறிலி Twine &Path,
                                                SmallVectorImpl<எழுத்து> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  எனில் (!CWD || CWD->empty())
    திரும்பு errc::operation_not_permitted;
  Path.toVector(Output);
  எனில் (auto EC = makeAbsolute(Output))
    திரும்பு EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/உண்மை);
  திரும்பு {};
}

std::error_code InMemoryFileSystem::isLocal(மாறிலி Twine &Path, தருக்க &Result) {
  Result = பொய்;
  திரும்பு {};
}

வெறுமை InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) மாறிலி {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

நாமாகாஶ {

நிலையான llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  மாறிலி size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  எனில் (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  திரும்பு style;
}

/// Removes leading "./" as well as path components like ".." and ".".
நிலையான llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/உண்மை, style);
  திரும்பு result;
}

/// Whether the error and entry specify a file/directory that was not found.
நிலையான தருக்க isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  எனில் (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    திரும்பு பொய்;
  திரும்பு EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  ப்ரதிஜ்ஞா(ExternalFS && "RedirectingFileSystem requires an external FS");
  எனில் (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
வகுப்பு llvm::vfs::RedirectingFSDirIterImpl
    : பொது llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(தருக்க IsFirstTime) {
    ப்ரதிஜ்ஞா((IsFirstTime || Current != End) && "cannot iterate past end");
    எனில் (!IsFirstTime)
      ++Current;
    எனில் (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      தேர்வு ((*Current)->getKind()) {
      நிலை RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      நிலை RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        நிறுத்து;
      நிலை RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        நிறுத்து;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } இல்லையெனில் {
      CurrentEntry = directory_entry();
    }
    திரும்பு {};
  };

பொது:
  RedirectingFSDirIterImpl(
      மாறிலி Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/உண்மை);
  }

  std::error_code increment() override {
    திரும்பு incrementImpl(/*IsFirstTime=*/பொய்);
  }
};

நாமாகாஶ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
வகுப்பு RedirectingFSDirRemapIterImpl : பொது llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

பொது:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    எனில் (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  வெறுமை setCurrentEntry() {
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
    எனில் (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    இல்லையெனில்
      CurrentEntry = directory_entry();
    திரும்பு EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() மாறிலி {
  திரும்பு WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(மாறிலி Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  எனில் (!exists(Path))
    திரும்பு errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  எனில் (std::error_code EC = makeAbsolute(AbsolutePath))
    திரும்பு EC;
  WorkingDirectory = std::string(AbsolutePath);
  திரும்பு {};
}

std::error_code RedirectingFileSystem::isLocal(மாறிலி Twine &Path_,
                                               தருக்க &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  எனில் (makeAbsolute(Path))
    திரும்பு {};

  திரும்பு ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<எழுத்து> &Path) மாறிலி {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  எனில் (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    திரும்பு {};

  auto WorkingDir = getCurrentWorkingDirectory();
  எனில் (!WorkingDir)
    திரும்பு WorkingDir.getError();

  திரும்பு makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<எழுத்து> &Path) மாறிலி {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  எனில் (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    திரும்பு std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  எனில் (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } இல்லையெனில் {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    எனில் (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  எனில் (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  திரும்பு {};
}

directory_iterator RedirectingFileSystem::dir_begin(மாறிலி Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  எனில் (EC)
    திரும்பு {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  எனில் (!Result) {
    எனில் (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      திரும்பு ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    திரும்பு {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  எனில் (!S) {
    எனில் (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      திரும்பு ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    திரும்பு {};
  }

  எனில் (!S->isDirectory()) {
    EC = errc::not_a_directory;
    திரும்பு {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  எனில் (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    எனில் (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } இல்லையெனில் {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  எனில் (RedirectEC) {
    எனில் (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      திரும்பு {};
    }
    RedirectIter = {};
  }

  எனில் (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    திரும்பு RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  எனில் (ExternalEC) {
    எனில் (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      திரும்பு {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  தேர்வு (Redirection) {
  நிலை RedirectKind::Fallthrough:
    Iters.தள்ளு_back(ExternalIter);
    Iters.தள்ளு_back(RedirectIter);
    நிறுத்து;
  நிலை RedirectKind::Fallback:
    Iters.தள்ளு_back(RedirectIter);
    Iters.தள்ளு_back(ExternalIter);
    நிறுத்து;
  இயல்பு:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  எனில் (EC)
    திரும்பு {};
  திரும்பு Combined;
}

வெறுமை RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() மாறிலி {
  திரும்பு OverlayFileDir;
}

வெறுமை RedirectingFileSystem::setFallthrough(தருக்க Fallthrough) {
  எனில் (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } இல்லையெனில் {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

வெறுமை RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() மாறிலி {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  க்கு (மாறிலி auto &வேர்_2 : Roots)
    R.தள்ளு_back(வேர்_2->getName());
  திரும்பு R;
}

வெறுமை RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) மாறிலி {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  எனில் (Type == PrintType::Summary)
    திரும்பு;

  க்கு (மாறிலி auto &வேர்_2 : Roots)
    printEntry(OS, வேர்_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

வெறுமை RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) மாறிலி {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  தேர்வு (E->getKind()) {
  நிலை EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    க்கு (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    நிறுத்து;
  }
  நிலை EK_DirectoryRemap:
  நிலை EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    தேர்வு (RE->getUseName()) {
    நிலை NK_NotSet:
      நிறுத்து;
    நிலை NK_External:
      OS << " (UseExternalName: true)";
      நிறுத்து;
    நிலை NK_Virtual:
      OS << " (UseExternalName: false)";
      நிறுத்து;
    }
    OS << "\n";
    நிறுத்து;
  }
  }
}

வெறுமை RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  எனில் (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
வகுப்பு llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  வெறுமை error(yaml::முனை *N, மாறிலி Twine &செய்தி_2) { Stream.printError(N, செய்தி_2); }

  // false on error
  தருக்க parseScalarString(yaml::முனை *N, StringRef &Result,
                         SmallVectorImpl<எழுத்து> &Storage) {
    மாறிலி auto *S = dyn_cast_if_present<yaml::Scalar_முனை>(N);

    எனில் (!S) {
      error(N, "expected string");
      திரும்பு பொய்;
    }
    Result = S->get_மதிப்பு(Storage);
    திரும்பு உண்மை;
  }

  // false on error
  தருக்க parseScalarBool(yaml::முனை *N, தருக்க &Result) {
    SmallString<5> Storage;
    StringRef மதிப்பு;
    எனில் (!parseScalarString(N, மதிப்பு, Storage))
      திரும்பு பொய்;

    எனில் (மதிப்பு.equals_insensitive("true") || மதிப்பு.equals_insensitive("on") ||
        மதிப்பு.equals_insensitive("yes") || மதிப்பு == "1") {
      Result = உண்மை;
      திரும்பு உண்மை;
    } இல்லையெனில் எனில் (மதிப்பு.equals_insensitive("false") ||
               மதிப்பு.equals_insensitive("off") ||
               மதிப்பு.equals_insensitive("no") || மதிப்பு == "0") {
      Result = பொய்;
      திரும்பு உண்மை;
    }

    error(N, "expected boolean value");
    திரும்பு பொய்;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::முனை *N) {
    SmallString<12> Storage;
    StringRef மதிப்பு;
    எனில் (!parseScalarString(N, மதிப்பு, Storage))
      திரும்பு std::nullopt;

    எனில் (மதிப்பு.equals_insensitive("fallthrough")) {
      திரும்பு RedirectingFileSystem::RedirectKind::Fallthrough;
    } இல்லையெனில் எனில் (மதிப்பு.equals_insensitive("fallback")) {
      திரும்பு RedirectingFileSystem::RedirectKind::Fallback;
    } இல்லையெனில் எனில் (மதிப்பு.equals_insensitive("redirect-only")) {
      திரும்பு RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    திரும்பு std::nullopt;
  }

  std::optional<RedirectingFileSystem::வேர்_Relative_Kind>
  parse_வேர்_Relative_Kind(yaml::முனை *N) {
    SmallString<12> Storage;
    StringRef மதிப்பு;
    எனில் (!parseScalarString(N, மதிப்பு, Storage))
      திரும்பு std::nullopt;
    எனில் (மதிப்பு.equals_insensitive("cwd")) {
      திரும்பு RedirectingFileSystem::வேர்_Relative_Kind::CWD;
    } இல்லையெனில் எனில் (மதிப்பு.equals_insensitive("overlay-dir")) {
      திரும்பு RedirectingFileSystem::வேர்_Relative_Kind::OverlayDir;
    }
    திரும்பு std::nullopt;
  }

  கட்டமைப்பு சாவி_Status {
    தருக்க Required;
    தருக்க Seen = பொய்;

    சாவி_Status(தருக்க Required = பொய்) : Required(Required) {}
  };

  using சாவி_Status_Pair = std::pair<StringRef, சாவி_Status>;

  // false on error
  தருக்க check_Duplicate_Or_Unknown_சாவி(yaml::முனை *சாவி_முனை, StringRef சாவி,
                                  DenseMap<StringRef, சாவி_Status> &சாவிகள்_2) {
    auto It = சாவிகள்_2.find(சாவி);
    எனில் (It == சாவிகள்_2.end()) {
      error(சாவி_முனை, "unknown key");
      திரும்பு பொய்;
    }
    சாவி_Status &S = It->second;
    எனில் (S.Seen) {
      error(சாவி_முனை, Twine("duplicate key '") + சாவி + "'");
      திரும்பு பொய்;
    }
    S.Seen = உண்மை;
    திரும்பு உண்மை;
  }

  // false on error
  தருக்க check_Missing_சாவிகள்(yaml::முனை *Obj, DenseMap<StringRef, சாவி_Status> &சாவிகள்_2) {
    க்கு (மாறிலி auto &I : சாவிகள்_2) {
      எனில் (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        திரும்பு பொய்;
      }
    }
    திரும்பு உண்மை;
  }

பொது:
  நிலையான RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    எனில் (!ParentEntry) { // Look for a existent root
      க்கு (மாறிலி auto &வேர்_2 : FS->Roots) {
        எனில் (Name == வேர்_2->getName()) {
          ParentEntry = வேர்_2.get();
          திரும்பு ParentEntry;
        }
      }
    } இல்லையெனில் { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      க்கு (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        எனில் (DirContent && Name == Content->getName())
          திரும்பு DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    எனில் (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.தள்ளு_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      திரும்பு ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    திரும்பு DE->getLastContent();
  }

தனிப்பட்ட:
  வெறுமை unique_Overlay_மரம்(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    தேர்வு (SrcE->getKind()) {
    நிலை RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      எனில் (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      க்கு (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_மரம்(FS, SubEntry.get(), NewParentE);
      நிறுத்து;
    }
    நிலை RedirectingFileSystem::EK_DirectoryRemap: {
      ப்ரதிஜ்ஞா(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      நிறுத்து;
    }
    நிலை RedirectingFileSystem::EK_File: {
      ப்ரதிஜ்ஞா(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      நிறுத்து;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::முனை *N, RedirectingFileSystem *FS, தருக்க Is_வேர்_Entry) {
    auto *M = dyn_cast<yaml::Mapping_முனை>(N);
    எனில் (!M) {
      error(N, "expected mapping node for file or directory entry");
      திரும்பு nullptr;
    }

    சாவி_Status_Pair Fields[] = {
        சாவி_Status_Pair("name", உண்மை),
        சாவி_Status_Pair("type", உண்மை),
        சாவி_Status_Pair("contents", பொய்),
        சாவி_Status_Pair("external-contents", பொய்),
        சாவி_Status_Pair("use-external-name", பொய்),
    };

    DenseMap<StringRef, சாவி_Status> சாவிகள்_2(std::begin(Fields), std::end(Fields));

    எண்ணிக்கை { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::முனை *Name_மதிப்பு_முனை = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    க்கு (auto &I : *M) {
      StringRef சாவி;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      எனில் (!parseScalarString(I.get_சாவி(), சாவி, Buffer))
        திரும்பு nullptr;

      எனில் (!check_Duplicate_Or_Unknown_சாவி(I.get_சாவி(), சாவி, சாவிகள்_2))
        திரும்பு nullptr;

      StringRef மதிப்பு;
      எனில் (சாவி == "name") {
        எனில் (!parseScalarString(I.get_மதிப்பு(), மதிப்பு, Buffer))
          திரும்பு nullptr;

        Name_மதிப்பு_முனை = I.get_மதிப்பு();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(மதிப்பு).str();
      } இல்லையெனில் எனில் (சாவி == "type") {
        எனில் (!parseScalarString(I.get_மதிப்பு(), மதிப்பு, Buffer))
          திரும்பு nullptr;
        எனில் (மதிப்பு == "file")
          Kind = RedirectingFileSystem::EK_File;
        இல்லையெனில் எனில் (மதிப்பு == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        இல்லையெனில் எனில் (மதிப்பு == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        இல்லையெனில் {
          error(I.get_மதிப்பு(), "unknown value for 'type'");
          திரும்பு nullptr;
        }
      } இல்லையெனில் எனில் (சாவி == "contents") {
        எனில் (ContentsField != CF_NotSet) {
          error(I.get_சாவி(),
                "entry already has 'contents' or 'external-contents'");
          திரும்பு nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_முனை>(I.get_மதிப்பு());
        எனில் (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_மதிப்பு(), "expected array");
          திரும்பு nullptr;
        }

        க்கு (auto &I : *Contents) {
          எனில் (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ பொய்))
            EntryArrayContents.தள்ளு_back(std::move(E));
          இல்லையெனில்
            திரும்பு nullptr;
        }
      } இல்லையெனில் எனில் (சாவி == "external-contents") {
        எனில் (ContentsField != CF_NotSet) {
          error(I.get_சாவி(),
                "entry already has 'contents' or 'external-contents'");
          திரும்பு nullptr;
        }
        ContentsField = CF_External;
        எனில் (!parseScalarString(I.get_மதிப்பு(), மதிப்பு, Buffer))
          திரும்பு nullptr;

        SmallString<256> FullPath;
        எனில் (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          ப்ரதிஜ்ஞா(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = மதிப்பு;
          எனில் (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            திரும்பு nullptr;
          }
          FullPath = AbsFullPath;
        } இல்லையெனில் {
          FullPath = மதிப்பு;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } இல்லையெனில் எனில் (சாவி == "use-external-name") {
        தருக்க Val;
        எனில் (!parseScalarBool(I.get_மதிப்பு(), Val))
          திரும்பு nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } இல்லையெனில் {
        llvm_unreachable("key missing from Keys");
      }
    }

    எனில் (Stream.failed())
      திரும்பு nullptr;

    // check for missing keys
    எனில் (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      திரும்பு nullptr;
    }
    எனில் (!check_Missing_சாவிகள்(N, சாவிகள்_2))
      திரும்பு nullptr;

    // check invalid configuration
    எனில் (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      திரும்பு nullptr;
    }

    எனில் (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      திரும்பு nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    எனில் (Is_வேர்_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      எனில் (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } இல்லையெனில் எனில் (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } இல்லையெனில் {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        எனில் (FS->வேர்_Relative ==
            RedirectingFileSystem::வேர்_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          ப்ரதிஜ்ஞா(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } இல்லையெனில் {
          EC = FS->makeAbsolute(Name);
        }
        எனில் (EC) {
          ப்ரதிஜ்ஞா(Name_மதிப்பு_முனை && "Name presence should be checked earlier");
          error(
              Name_மதிப்பு_முனை,
              "entry with relative path at the root level is not discoverable");
          திரும்பு nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      எனில் (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t வேர்_Path_Len = sys::path::வேர்_path(Trimmed, path_style).size();
    போது (Trimmed.size() > வேர்_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    தேர்வு (Kind) {
    நிலை RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      நிறுத்து;
    நிலை RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      நிறுத்து;
    நிலை RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      நிறுத்து;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    எனில் (Parent.empty())
      திரும்பு Result;

    // if 'name' contains multiple components, create implicit directory entries
    க்கு (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.தள்ளு_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    திரும்பு Result;
  }

பொது:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  தருக்க parse(yaml::முனை *வேர்_2, RedirectingFileSystem *FS) {
    auto *உச்சி = dyn_cast<yaml::Mapping_முனை>(வேர்_2);
    எனில் (!உச்சி) {
      error(வேர்_2, "expected mapping node");
      திரும்பு பொய்;
    }

    சாவி_Status_Pair Fields[] = {
        சாவி_Status_Pair("version", உண்மை),
        சாவி_Status_Pair("case-sensitive", பொய்),
        சாவி_Status_Pair("use-external-names", பொய்),
        சாவி_Status_Pair("root-relative", பொய்),
        சாவி_Status_Pair("overlay-relative", பொய்),
        சாவி_Status_Pair("fallthrough", பொய்),
        சாவி_Status_Pair("redirecting-with", பொய்),
        சாவி_Status_Pair("roots", உண்மை),
    };

    DenseMap<StringRef, சாவி_Status> சாவிகள்_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> வேர்_Entries;

    // Parse configuration and 'roots'
    க்கு (auto &I : *உச்சி) {
      SmallString<10> சாவி_Buffer;
      StringRef சாவி;
      எனில் (!parseScalarString(I.get_சாவி(), சாவி, சாவி_Buffer))
        திரும்பு பொய்;

      எனில் (!check_Duplicate_Or_Unknown_சாவி(I.get_சாவி(), சாவி, சாவிகள்_2))
        திரும்பு பொய்;

      எனில் (சாவி == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_முனை>(I.get_மதிப்பு());
        எனில் (!Roots) {
          error(I.get_மதிப்பு(), "expected array");
          திரும்பு பொய்;
        }

        க்கு (auto &I : *Roots) {
          எனில் (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ உண்மை))
            வேர்_Entries.தள்ளு_back(std::move(E));
          இல்லையெனில்
            திரும்பு பொய்;
        }
      } இல்லையெனில் எனில் (சாவி == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        எனில் (!parseScalarString(I.get_மதிப்பு(), VersionString, Storage))
          திரும்பு பொய்;
        முழு Version;
        எனில் (VersionString.getAsInteger<முழு>(10, Version)) {
          error(I.get_மதிப்பு(), "expected integer");
          திரும்பு பொய்;
        }
        எனில் (Version < 0) {
          error(I.get_மதிப்பு(), "invalid version number");
          திரும்பு பொய்;
        }
        எனில் (Version != 0) {
          error(I.get_மதிப்பு(), "version mismatch, expected 0");
          திரும்பு பொய்;
        }
      } இல்லையெனில் எனில் (சாவி == "case-sensitive") {
        எனில் (!parseScalarBool(I.get_மதிப்பு(), FS->CaseSensitive))
          திரும்பு பொய்;
      } இல்லையெனில் எனில் (சாவி == "overlay-relative") {
        எனில் (!parseScalarBool(I.get_மதிப்பு(), FS->IsRelativeOverlay))
          திரும்பு பொய்;
      } இல்லையெனில் எனில் (சாவி == "use-external-names") {
        எனில் (!parseScalarBool(I.get_மதிப்பு(), FS->UseExternalNames))
          திரும்பு பொய்;
      } இல்லையெனில் எனில் (சாவி == "fallthrough") {
        எனில் (சாவிகள்_2["redirecting-with"].Seen) {
          error(I.get_மதிப்பு(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          திரும்பு பொய்;
        }

        தருக்க ShouldFallthrough = பொய்;
        எனில் (!parseScalarBool(I.get_மதிப்பு(), ShouldFallthrough))
          திரும்பு பொய்;

        எனில் (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } இல்லையெனில் {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } இல்லையெனில் எனில் (சாவி == "redirecting-with") {
        எனில் (சாவிகள்_2["fallthrough"].Seen) {
          error(I.get_மதிப்பு(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          திரும்பு பொய்;
        }

        எனில் (auto Kind = parseRedirectKind(I.get_மதிப்பு())) {
          FS->Redirection = *Kind;
        } இல்லையெனில் {
          error(I.get_மதிப்பு(), "expected valid redirect kind");
          திரும்பு பொய்;
        }
      } இல்லையெனில் எனில் (சாவி == "root-relative") {
        எனில் (auto Kind = parse_வேர்_Relative_Kind(I.get_மதிப்பு())) {
          FS->வேர்_Relative = *Kind;
        } இல்லையெனில் {
          error(I.get_மதிப்பு(), "expected valid root-relative kind");
          திரும்பு பொய்;
        }
      } இல்லையெனில் {
        llvm_unreachable("key missing from Keys");
      }
    }

    எனில் (Stream.failed())
      திரும்பு பொய்;

    எனில் (!check_Missing_சாவிகள்(உச்சி, சாவிகள்_2))
      திரும்பு பொய்;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    க்கு (auto &E : வேர்_Entries)
      unique_Overlay_மரம்(FS, E.get());

    திரும்பு உண்மை;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, வெறுமை *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::முனை *வேர்_2 = DI->get_வேர்();
  எனில் (DI == Stream.end() || !வேர்_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    திரும்பு nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      புதிய RedirectingFileSystem(ExternalFS));

  எனில் (!YAMLFilePath.empty()) {
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
    ப்ரதிஜ்ஞா(!EC && "Overlay dir final path must be absolute");
    (வெறுமை)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  எனில் (!P.parse(வேர்_2, FS.get()))
    திரும்பு nullptr;

  திரும்பு FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    தருக்க UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      புதிய RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  க்கு (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (வெறுமை)EC;
      ப்ரதிஜ்ஞா(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    எனில் (ToEntry)
      தொடர்;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    க்கு (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    ப்ரதிஜ்ஞா(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (வெறுமை)EC;
      ப்ரதிஜ்ஞா(!EC && "Could not make absolute path");
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

  திரும்பு FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  ப்ரதிஜ்ஞா(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  எனில் (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

வெறுமை RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<எழுத்து> &Result) மாறிலி {
  Result.clear();
  க்கு (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<எழுத்து> &Path) மாறிலி {
  எனில் (std::error_code EC = makeAbsolute(Path))
    திரும்பு EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  எனில் (CanonicalPath.empty())
    திரும்பு make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  திரும்பு {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) மாறிலி {
  llvm::SmallString<128> CanonicalPath(Path);
  எனில் (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    திரும்பு EC;

  // RedirectOnly means the VFS is always used.
  எனில் (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = உண்மை;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  க்கு (மாறிலி auto &வேர்_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, வேர்_2.get(), Entries);
    எனில் (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = உண்மை;
    எனில் (Result) {
      Result->Parents = std::move(Entries);
      திரும்பு Result;
    }

    எனில் (Result.getError() != llvm::errc::no_such_file_or_directory)
      திரும்பு Result;
  }
  திரும்பு make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) மாறிலி {
  ப்ரதிஜ்ஞா(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  எனில் (!FromName.empty()) {
    எனில் (!pathComponentMatches(*Start, FromName))
      திரும்பு make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    எனில் (Start == End) {
      // Match!
      திரும்பு LookupResult(From, Start, End);
    }
  }

  எனில் (isa<RedirectingFileSystem::FileEntry>(From))
    திரும்பு make_error_code(llvm::errc::not_a_directory);

  எனில் (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    திரும்பு LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  க்கு (மாறிலி std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.தள்ளு_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    எனில் (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      திரும்பு Result;
    Entries.எடு_back();
  }

  திரும்பு make_error_code(llvm::errc::no_such_file_or_directory);
}

நிலையான Status getRedirectedFileStatus(மாறிலி Twine &OriginalPath,
                                      தருக்க UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  எனில் (ExternalStatus.ExposesExternalVFSPath)
    திரும்பு ExternalStatus;

  Status S = ExternalStatus;
  எனில் (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  இல்லையெனில்
    S.ExposesExternalVFSPath = உண்மை;
  திரும்பு S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    மாறிலி Twine &LookupPath, மாறிலி Twine &OriginalPath,
    மாறிலி RedirectingFileSystem::LookupResult &Result) {
  எனில் (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    எனில் (std::error_code EC = makeAbsolute(RemappedPath))
      திரும்பு EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    எனில் (!S)
      திரும்பு S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    திரும்பு getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  திரும்பு Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(மாறிலி Twine &LookupPath,
                                         மாறிலி Twine &OriginalPath) மாறிலி {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  எனில் (!Result || Result->ExposesExternalVFSPath)
    திரும்பு Result;
  திரும்பு Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(மாறிலி Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  எனில் (std::error_code EC = makeAbsolute(Path))
    திரும்பு EC;

  எனில் (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    எனில் (S)
      திரும்பு S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  எனில் (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    எனில் (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      திரும்பு getExternalStatus(Path, OriginalPath);
    திரும்பு Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  எனில் (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    திரும்பு getExternalStatus(Path, OriginalPath);
  }

  திரும்பு S;
}

தருக்க RedirectingFileSystem::exists(மாறிலி Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  எனில் (makeAbsolute(Path))
    திரும்பு பொய்;

  எனில் (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    எனில் (ExternalFS->exists(Path))
      திரும்பு உண்மை;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  எனில் (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    எனில் (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      திரும்பு ExternalFS->exists(Path);
    திரும்பு பொய்;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  எனில் (!ExtRedirect) {
    ப்ரதிஜ்ஞா(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    திரும்பு உண்மை;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  எனில் (makeAbsolute(RemappedPath))
    திரும்பு பொய்;

  எனில் (ExternalFS->exists(RemappedPath))
    திரும்பு உண்மை;

  எனில் (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    திரும்பு ExternalFS->exists(Path);
  }

  திரும்பு பொய்;
}

நாமாகாஶ {

/// Provide a file wrapper with an overriden status.
வகுப்பு FileWithFixedStatus : பொது File {
  std::unique_ptr<File> InnerFile;
  Status S;

பொது:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { திரும்பு S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(மாறிலி Twine &Name, int64_t FileSize, தருக்க RequiresNullTerminator,
            தருக்க IsVolatile) override {
    திரும்பு InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { திரும்பு InnerFile->close(); }

  வெறுமை setPath(மாறிலி Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, மாறிலி Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  எனில் (!Result || (*Result)->status()->ExposesExternalVFSPath)
    திரும்பு Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  எனில் (Name && Name.get() != P.str())
    F->get()->setPath(P);
  திரும்பு F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(மாறிலி Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  எனில் (std::error_code EC = makeAbsolute(Path))
    திரும்பு EC;

  எனில் (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    எனில் (F)
      திரும்பு F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  எனில் (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    எனில் (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      திரும்பு File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    திரும்பு Result.getError();
  }

  எனில் (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    திரும்பு make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  எனில் (std::error_code EC = makeAbsolute(RemappedPath))
    திரும்பு EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  எனில் (!ExternalFile) {
    எனில் (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      திரும்பு File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    திரும்பு ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  எனில் (!ExternalStatus)
    திரும்பு ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  திரும்பு std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(மாறிலி Twine &OriginalPath,
                                   SmallVectorImpl<எழுத்து> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  எனில் (std::error_code EC = makeAbsolute(Path))
    திரும்பு EC;

  எனில் (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    எனில் (!EC)
      திரும்பு EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  எனில் (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    எனில் (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      திரும்பு ExternalFS->getRealPath(Path, Output);
    திரும்பு Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  எனில் (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    எனில் (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      திரும்பு ExternalFS->getRealPath(Path, Output);
    }
    திரும்பு P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  எனில் (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    திரும்பு {};
  }
  திரும்பு llvm::errc::invalid_argument;
}

வெறுமை RedirectingFileSystem::getDirectoryContentRealSources(
    மாறிலி Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  எனில் (makeAbsolute(Path))
    திரும்பு;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  மாறிலி தருக்க ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  எனில் (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    எனில் (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    திரும்பு;
  }

  தேர்வு (Result->E->getKind()) {
  நிலை EK_File:
    திரும்பு;
  நிலை EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    நிறுத்து;
  நிலை EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    எனில் (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    நிறுத்து;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  எனில் (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, வெறுமை *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  திரும்பு RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

நிலையான வெறுமை getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  எனில் (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    ப்ரதிஜ்ஞா(DE && "Must be a directory");
    க்கு (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.தள்ளு_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.எடு_back();
    }
    திரும்பு;
  }

  எனில் (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    ப்ரதிஜ்ஞா(DR && "Must be a directory remap");
    SmallString<128> VPath;
    க்கு (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.தள்ளு_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    திரும்பு;
  }

  ப்ரதிஜ்ஞா(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  ப்ரதிஜ்ஞா(FE && "Must be a file");
  SmallString<128> VPath;
  க்கு (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.தள்ளு_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

வெறுமை vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> வேர்_Result = VFS.lookupPath("/");
  எனில் (!வேர்_Result)
    திரும்பு;
  SmallVector<StringRef, 8> Components;
  Components.தள்ளு_back("/");
  getVFSEntries(வேர்_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  நிலையான std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  திரும்பு UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

வெறுமை YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             தருக்க IsDirectory) {
  ப்ரதிஜ்ஞா(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  ப்ரதிஜ்ஞா(sys::path::is_absolute(RealPath) && "real path not absolute");
  ப்ரதிஜ்ஞா(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

வெறுமை YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/பொய்);
}

வெறுமை YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/உண்மை);
}

நாமாகாஶ {

வகுப்பு JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_அடுக்கு;

  unsigned getDirIndent() { திரும்பு 4 * Dir_அடுக்கு.size(); }
  unsigned getFileIndent() { திரும்பு 4 * (Dir_அடுக்கு.size() + 1); }
  தருக்க containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  வெறுமை startDirectory(StringRef Path);
  வெறுமை endDirectory();
  வெறுமை writeEntry(StringRef VPath, StringRef RPath);

பொது:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  வெறுமை லிக(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<தருக்க> UseExternalNames,
             std::optional<தருக்க> IsCaseSensitive,
             std::optional<தருக்க> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

தருக்க JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using நாமாகாஶ llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  க்கு (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    எனில் (*IParent != *IChild)
      திரும்பு பொய்;
  }
  // Have we exhausted the parent path?
  திரும்பு IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  ப்ரதிஜ்ஞா(!Parent.empty());
  ப்ரதிஜ்ஞா(containedIn(Parent, Path));
  திரும்பு Path.substr(Parent.size() + 1);
}

வெறுமை JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_அடுக்கு.empty() ? Path : containedPart(Dir_அடுக்கு.back(), Path);
  Dir_அடுக்கு.தள்ளு_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

வெறுமை JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_அடுக்கு.எடு_back();
}

வெறுமை JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

வெறுமை JSONWriter::லிக(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<தருக்க> UseExternalNames,
                       std::optional<தருக்க> IsCaseSensitive,
                       std::optional<தருக்க> IsOverlayRelative,
                       StringRef OverlayDir) {
  using நாமாகாஶ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  எனில் (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  எனில் (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  தருக்க UseOverlayRelative = பொய்;
  எனில் (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  எனில் (!Entries.empty()) {
    மாறிலி YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    எனில் (UseOverlayRelative) {
      ப்ரதிஜ்ஞா(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    தருக்க IsCurrentDirEmpty = உண்மை;
    எனில் (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = பொய்;
    }

    க்கு (மாறிலி auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      எனில் (Dir == Dir_அடுக்கு.back()) {
        எனில் (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } இல்லையெனில் {
        தருக்க Is_Dir_Popped_From_அடுக்கு = பொய்;
        போது (!Dir_அடுக்கு.empty() && !containedIn(Dir_அடுக்கு.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_அடுக்கு = உண்மை;
        }
        எனில் (Is_Dir_Popped_From_அடுக்கு || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = உண்மை;
      }
      StringRef RPath = Entry.RPath;
      எனில் (UseOverlayRelative) {
        ப்ரதிஜ்ஞா(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      எனில் (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = பொய்;
      }
    }

    போது (!Dir_அடுக்கு.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

வெறுமை YAMLVFSWriter::லிக(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](மாறிலி YAMLVFSEntry &LHS, மாறிலி YAMLVFSEntry &RHS) {
    திரும்பு LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).லிக(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, மாறிலி Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  எனில் (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->அடுக்கு.தள்ளு_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  ப்ரதிஜ்ஞா(FS && State && !State->அடுக்கு.empty() && "incrementing past end");
  ப்ரதிஜ்ஞா(!State->அடுக்கு.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  எனில் (State->Has_No_தள்ளு_Request)
    State->Has_No_தள்ளு_Request = பொய்;
  இல்லையெனில் {
    எனில் (State->அடுக்கு.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->அடுக்கு.back()->path(), EC);
      எனில் (I != End) {
        State->அடுக்கு.தள்ளு_back(I);
        திரும்பு *இது;
      }
    }
  }

  போது (!State->அடுக்கு.empty() && State->அடுக்கு.back().increment(EC) == End)
    State->அடுக்கு.எடு_back();

  எனில் (State->அடுக்கு.empty())
    State.reset(); // end iterator

  திரும்பு *இது;
}

மாறிலி எழுத்து FileSystem::ID = 0;
மாறிலி எழுத்து OverlayFileSystem::ID = 0;
மாறிலி எழுத்து ProxyFileSystem::ID = 0;
மாறிலி எழுத்து InMemoryFileSystem::ID = 0;
மாறிலி எழுத்து RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_மொத்தம்(
    மாறிலி FileSystem *FS) {
  திரும்பு FS->Use_மொத்தம்();
}

வெறுமை ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

வெறுமை ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
