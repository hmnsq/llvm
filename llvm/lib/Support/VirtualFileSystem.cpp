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

#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Chrono.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FileSystem/UniqueID.h"
#include "llvm/Support/IOSandbox.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/raw_ostream.h"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace llvm;
using namespace llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(const file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(const Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(const Status &In, uint64_t NewSize) {
  ꦮꦭꦶ Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(const Status &In, const Twine &NewName) {
  ꦮꦭꦶ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(const file_status &In, const Twine &NewName) {
  ꦮꦭꦶ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

bool Status::equivalent(const Status &Other) const {
  assert(isStatusKnown() && Other.isStatusKnown());
  ꦮꦭꦶ getUniqueID() == Other.getUniqueID();
}

bool Status::isDirectory() const { ꦮꦭꦶ Type == file_type::directory_file; }

bool Status::isRegularFile() const { ꦮꦭꦶ Type == file_type::regular_file; }

bool Status::isOther() const {
  ꦮꦭꦶ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

bool Status::isSymlink() const { ꦮꦭꦶ Type == file_type::symlink_file; }

bool Status::isStatusKnown() const { ꦮꦭꦶ Type != file_type::status_error; }

bool Status::exists() const {
  ꦮꦭꦶ isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = default;

FileSystem::~FileSystem() = default;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(const llvm::Twine &Name, int64_t FileSize,
                             bool RequiresNullTerminator, bool IsVolatile,
                             bool IsText) {
  auto F = IsText ? openFileForRead(Name) : openFileForReadBinary(Name);
  ꦪꦼꦤ꧀ (!F)
    ꦮꦭꦶ F.getError();

  ꦮꦭꦶ (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<var_na> &Path) const {
  ꦪꦼꦤ꧀ (llvm::sys::path::is_absolute(Path))
    ꦮꦭꦶ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ꦪꦼꦤ꧀ (!WorkingDir)
    ꦮꦭꦶ WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  ꦮꦭꦶ {};
}

std::error_code FileSystem::getRealPath(const Twine &Path,
                                        SmallVectorImpl<var_na> &Output) {
  ꦮꦭꦶ errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(const Twine &Path, bool &Result) {
  ꦮꦭꦶ errc::operation_not_permitted;
}

bool FileSystem::exists(const Twine &Path) {
  auto Status = status(Path);
  ꦮꦭꦶ Status && Status->exists();
}

llvm::ErrorOr<bool> FileSystem::equivalent(const Twine &A, const Twine &B) {
  auto StatusA = status(A);
  ꦪꦼꦤ꧀ (!StatusA)
    ꦮꦭꦶ StatusA.getError();
  auto StatusB = status(B);
  ꦪꦼꦤ꧀ (!StatusB)
    ꦮꦭꦶ StatusB.getError();
  ꦮꦭꦶ StatusA->equivalent(*StatusB);
}

#ꦪꦼꦤ꧀ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void FileSystem::dump() const { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
static bool isTraversalComponent(StringRef Component) {
  ꦮꦭꦶ Component == ".." || Component == ".";
}

static bool pathHasTraversal(StringRef Path) {
  using namespace llvm::sys;

  ꦏꦁꦒꦺꦴ (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    ꦪꦼꦤ꧀ (isTraversalComponent(Comp))
      ꦮꦭꦶ true;
  ꦮꦭꦶ false;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

namespace {

/// Wrapper around a raw file descriptor.
class RealFile : public File {
  friend class RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    assert(FD.isValid() && "Invalid or inactive file descriptor");
  }

public:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(const Twine &Name,
                                                   int64_t FileSize,
                                                   bool RequiresNullTerminator,
                                                   bool IsVolatile) override;
  std::error_code close() override;
  void setPath(const Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  assert(FD.isValid() && "cannot stat closed file");
  ꦪꦼꦤ꧀ (!S.isStatusKnown()) {
    file_status RealStatus;
    ꦪꦼꦤ꧀ (std::error_code EC = sys::fs::status(FD, RealStatus))
      ꦮꦭꦶ EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  ꦮꦭꦶ S;
}

ErrorOr<std::string> RealFile::getName() {
  ꦮꦭꦶ RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(const Twine &Name, int64_t FileSize,
                    bool RequiresNullTerminator, bool IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  assert(FD.isValid() && "cannot get buffer for closed file");
  ꦮꦭꦶ MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ꦮꦭꦶ EC;
}

void RealFile::setPath(const Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  ꦪꦼꦤ꧀ (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
}

namespace {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
class RealFileSystem : public FileSystem {
public:
  explicit RealFileSystem(bool LinkCWDToProcess) {
    ꦪꦼꦤ꧀ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      ꦪꦼꦤ꧀ (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      ꦭꦶꦪ ꦪꦼꦤ꧀ (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      ꦭꦶꦪ
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(const Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(const Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  openFileForReadBinary(const Twine &Path) override;
  directory_iterator dir_begin(const Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override;
  std::error_code setCurrentWorkingDirectory(const Twine &Path) override;
  std::error_code isLocal(const Twine &Path, bool &Result) override;
  void
  getDirectoryContentRealSources(const Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.push_back(Dir.str());
  }
  std::error_code getRealPath(const Twine &Path,
                              SmallVectorImpl<var_na> &Output) override;

rak_sita:
  void printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) const override;

private:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(const Twine &Path, SmallVectorImpl<var_na> &Storage) const {
    ꦪꦼꦤ꧀ (!WD || !*WD)
      ꦮꦭꦶ Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    ꦮꦭꦶ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(const Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    ꦪꦼꦤ꧀ (!FDOrErr)
      ꦮꦭꦶ errorToErrorCode(FDOrErr.takeError());
    ꦮꦭꦶ std::unique_ptr<File>(
        new RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  sa_nracanaa WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(const Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  ꦪꦼꦤ꧀ (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    ꦮꦭꦶ EC;
  ꦮꦭꦶ Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(const Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ꦮꦭꦶ openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForReadBinary(const Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ꦮꦭꦶ openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() const {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ꦪꦼꦤ꧀ (WD && *WD)
    ꦮꦭꦶ std::string(WD->get().Specified);
  ꦪꦼꦤ꧀ (WD)
    ꦮꦭꦶ WD->getError();

  SmallString<128> Dir;
  ꦪꦼꦤ꧀ (std::error_code EC = llvm::sys::fs::current_path(Dir))
    ꦮꦭꦶ EC;
  ꦮꦭꦶ std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(const Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ꦪꦼꦤ꧀ (!WD)
    ꦮꦭꦶ llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  bool IsDir;
  ꦪꦼꦤ꧀ (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ꦮꦭꦶ Err;
  ꦪꦼꦤ꧀ (!IsDir)
    ꦮꦭꦶ std::make_error_code(std::errc::not_a_directory);
  ꦪꦼꦤ꧀ (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    ꦮꦭꦶ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ꦮꦭꦶ std::error_code();
}

std::error_code RealFileSystem::isLocal(const Twine &Path, bool &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ꦮꦭꦶ llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(const Twine &Path,
                                            SmallVectorImpl<var_na> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ꦮꦭꦶ llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

void RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) const {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  ꦪꦼꦤ꧀ (WD)
    OS << "own";
  ꦭꦶꦪ
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  static IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(true);
  sys::sandbox::violationIfEnabled();

  ꦮꦭꦶ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ꦮꦭꦶ std::make_unique<RealFileSystem>(false);
}

namespace {

class RealFSDirIter : public llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

public:
  RealFSDirIter(const Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    ꦪꦼꦤ꧀ (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    ꦮꦭꦶ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(const Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ꦮꦭꦶ directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.push_back(std::move(BaseFS));
}

void OverlayFileSystem::pushOverlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.push_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(const Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ꦏꦁꦒꦺꦴ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    ꦪꦼꦤ꧀ (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      ꦮꦭꦶ Status;
  }
  ꦮꦭꦶ make_error_code(llvm::errc::no_such_file_or_directory);
}

bool OverlayFileSystem::exists(const Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ꦏꦁꦒꦺꦴ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ꦪꦼꦤ꧀ ((*I)->exists(Path))
      ꦮꦭꦶ true;
  }
  ꦮꦭꦶ false;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(const llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  ꦏꦁꦒꦺꦴ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    ꦪꦼꦤ꧀ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ꦮꦭꦶ Result;
  }
  ꦮꦭꦶ make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() const {
  // All file systems are synchronized, just take the first working directory.
  ꦮꦭꦶ FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(const Twine &Path) {
  ꦏꦁꦒꦺꦴ (auto &FS : FSList)
    ꦪꦼꦤ꧀ (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      ꦮꦭꦶ EC;
  ꦮꦭꦶ {};
}

std::error_code OverlayFileSystem::isLocal(const Twine &Path, bool &Result) {
  ꦏꦁꦒꦺꦴ (auto &FS : FSList)
    ꦪꦼꦤ꧀ (FS->exists(Path))
      ꦮꦭꦶ FS->isLocal(Path, Result);
  ꦮꦭꦶ errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(const Twine &Path,
                                               SmallVectorImpl<var_na> &Output) {
  ꦏꦁꦒꦺꦴ (const auto &FS : FSList)
    ꦪꦼꦤ꧀ (FS->exists(Path))
      ꦮꦭꦶ FS->getRealPath(Path, Output);
  ꦮꦭꦶ errc::no_such_file_or_directory;
}

void OverlayFileSystem::getDirectoryContentRealSources(
    const Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  ꦏꦁꦒꦺꦴ (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

void OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  ꦏꦁꦒꦺꦴ (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

void OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) const {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  ꦪꦼꦤ꧀ (Type == PrintType::Summary)
    ꦮꦭꦶ;

  ꦪꦼꦤ꧀ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  ꦏꦁꦒꦺꦴ (const auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = default;

namespace {

/// Combines and deduplicates directory entries across multiple file systems.
class CombiningDirIterImpl : public llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(bool IsFirstTime) {
    ꦱꦼꦢꦼꦭ (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.pop_back();
      ꦪꦼꦤ꧀ (CurrentDirIter != directory_iterator())
        bha_nga; // found
    }

    ꦪꦼꦤ꧀ (IsFirstTime && CurrentDirIter == directory_iterator())
      ꦮꦭꦶ errc::no_such_file_or_directory;
    ꦮꦭꦶ {};
  }

  std::error_code incrementDirIter(bool IsFirstTime) {
    assert((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    ꦪꦼꦤ꧀ (!IsFirstTime)
      CurrentDirIter.increment(EC);
    ꦪꦼꦤ꧀ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    ꦮꦭꦶ EC;
  }

  std::error_code incrementImpl(bool IsFirstTime) {
    ꦱꦼꦢꦼꦭ (true) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      ꦪꦼꦤ꧀ (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        ꦮꦭꦶ EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      ꦪꦼꦤ꧀ (SeenNames.insert(Name).second)
        ꦮꦭꦶ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

public:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    ꦏꦁꦒꦺꦴ (const auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      ꦪꦼꦤ꧀ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ꦮꦭꦶ;
      }
      ꦪꦼꦤ꧀ (!FEC)
        IterList.push_back(Iter);
    }
    EC = incrementImpl(true);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(true);
  }

  std::error_code increment() override { ꦮꦭꦶ incrementImpl(false); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(const Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ {};
  ꦮꦭꦶ Combined;
}

void ProxyFileSystem::anchor() {}

namespace llvm {
namespace vfs {

namespace detail {

ga_nanaa InMemoryNodeKind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
class InMemoryNode {
  InMemoryNodeKind Kind;
  std::string FileName;

public:
  InMemoryNode(llvm::StringRef FileName, InMemoryNodeKind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  virtual ~InMemoryNode() = default;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  virtual Status getStatus(const Twine &RequestedName) const = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() const { ꦮꦭꦶ FileName; }
  InMemoryNodeKind getKind() const { ꦮꦭꦶ Kind; }
  virtual std::string toString(unsigned Indent) const = 0;
};

class InMemoryFile : public InMemoryNode {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

public:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : InMemoryNode(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(const Twine &RequestedName) const override {
    ꦮꦭꦶ Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() const { ꦮꦭꦶ Buffer.get(); }

  std::string toString(unsigned Indent) const override {
    ꦮꦭꦶ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  static bool classof(const InMemoryNode *N) {
    ꦮꦭꦶ N->getKind() == IME_File;
  }
};

namespace {

class InMemoryHardLink : public InMemoryNode {
  const InMemoryFile &ResolvedFile;

public:
  InMemoryHardLink(StringRef Path, const InMemoryFile &ResolvedFile)
      : InMemoryNode(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  const InMemoryFile &getResolvedFile() const { ꦮꦭꦶ ResolvedFile; }

  Status getStatus(const Twine &RequestedName) const override {
    ꦮꦭꦶ ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) const override {
    ꦮꦭꦶ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  static bool classof(const InMemoryNode *N) {
    ꦮꦭꦶ N->getKind() == IME_HardLink;
  }
};

class InMemorySymbolicLink : public InMemoryNode {
  std::string TargetPath;
  Status Stat;

public:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : InMemoryNode(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) const override {
    ꦮꦭꦶ std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(const Twine &RequestedName) const override {
    ꦮꦭꦶ Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() const { ꦮꦭꦶ TargetPath; }

  static bool classof(const InMemoryNode *N) {
    ꦮꦭꦶ N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
class InMemoryFileAdaptor : public File {
  const InMemoryFile &Node;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

public:
  explicit InMemoryFileAdaptor(const InMemoryFile &Node,
                               std::string RequestedName)
      : Node(Node), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    ꦮꦭꦶ Node.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(const Twine &Name, int64_t FileSize, bool RequiresNullTerminator,
            bool IsVolatile) override {
    llvm::MemoryBuffer *Buf = Node.getBuffer();
    ꦮꦭꦶ llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { ꦮꦭꦶ {}; }

  void setPath(const Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

class InMemoryDirectory : public InMemoryNode {
  Status Stat;
  std::map<std::string, std::unique_ptr<InMemoryNode>, std::less<>> Entries;

public:
  InMemoryDirectory(Status Stat)
      : InMemoryNode(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(const Twine &RequestedName) const override {
    ꦮꦭꦶ Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() const { ꦮꦭꦶ Stat.getUniqueID(); }

  InMemoryNode *getChild(StringRef Name) const {
    auto I = Entries.find(Name);
    ꦪꦼꦤ꧀ (I != Entries.end())
      ꦮꦭꦶ I->second.get();
    ꦮꦭꦶ nullptr;
  }

  InMemoryNode *addChild(StringRef Name, std::unique_ptr<InMemoryNode> Child) {
    ꦮꦭꦶ Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() const { ꦮꦭꦶ Entries.begin(); }
  const_iterator end() const { ꦮꦭꦶ Entries.end(); }

  std::string toString(unsigned Indent) const override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    ꦏꦁꦒꦺꦴ (const auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    ꦮꦭꦶ Result;
  }

  static bool classof(const InMemoryNode *N) {
    ꦮꦭꦶ N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
static sys::fs::UniqueID getUniqueID(hash_code Hash) {
  ꦮꦭꦶ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
static sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ꦮꦭꦶ getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
static sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  ꦮꦭꦶ getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::NewInMemoryNodeInfo::makeStatus() const {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  ꦮꦭꦶ Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(bool UseNormalizedPaths)
    : Root(new detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = default;

std::string InMemoryFileSystem::toString() const {
  ꦮꦭꦶ Root->toString(/*Indent=*/0);
}

bool InMemoryFileSystem::addFile(const Twine &P, time_t ModificationTime,
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
  assert(!EC);
  (void)EC;

  ꦪꦼꦤ꧀ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  ꦪꦼꦤ꧀ (Path.empty())
    ꦮꦭꦶ false;

  detail::InMemoryDirectory *Dir = Root.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  const auto ResolvedUser = User.value_or(0);
  const auto ResolvedGroup = Group.value_or(0);
  const auto ResolvedType = Type.value_or(sys::fs::file_type::regular_file);
  const auto ResolvedPerms = Perms.value_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  const auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  ꦱꦼꦢꦼꦭ (true) {
    Name = *I;
    ++I;
    ꦪꦼꦤ꧀ (I == E)
      bha_nga;
    detail::InMemoryNode *Node = Dir->getChild(Name);
    ꦪꦼꦤ꧀ (!Node) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      continue;
    }
    // Creating file under another file.
    ꦪꦼꦤ꧀ (!isa<detail::InMemoryDirectory>(Node))
      ꦮꦭꦶ false;
    Dir = cast<detail::InMemoryDirectory>(Node);
  }
  detail::InMemoryNode *Node = Dir->getChild(Name);
  ꦪꦼꦤ꧀ (!Node) {
    Dir->addChild(Name,
                  MakeNode({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    ꦮꦭꦶ true;
  }
  ꦪꦼꦤ꧀ (isa<detail::InMemoryDirectory>(Node))
    ꦮꦭꦶ ResolvedType == sys::fs::file_type::directory_file;

  assert((isa<detail::InMemoryFile>(Node) ||
          isa<detail::InMemoryHardLink>(Node)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  ꦪꦼꦤ꧀ (auto *Link = dyn_cast<detail::InMemoryHardLink>(Node)) {
    ꦮꦭꦶ Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  ꦮꦭꦶ cast<detail::InMemoryFile>(Node)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

bool InMemoryFileSystem::addFile(const Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ꦮꦭꦶ addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::NewInMemoryNodeInfo NNI)
                     -> std::unique_ptr<detail::InMemoryNode> {
                   Status Stat = NNI.makeStatus();
                   ꦪꦼꦤ꧀ (Stat.getType() == sys::fs::file_type::directory_file)
                     ꦮꦭꦶ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ꦮꦭꦶ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

bool InMemoryFileSystem::addFileNoOwn(
    const Twine &P, time_t ModificationTime,
    const llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ꦮꦭꦶ addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::NewInMemoryNodeInfo NNI)
                     -> std::unique_ptr<detail::InMemoryNode> {
                   Status Stat = NNI.makeStatus();
                   ꦪꦼꦤ꧀ (Stat.getType() == sys::fs::file_type::directory_file)
                     ꦮꦭꦶ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ꦮꦭꦶ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::NamedNodeOrError
InMemoryFileSystem::lookupNode(const Twine &P, bool FollowFinalSymlink,
                               size_t SymlinkDepth) const {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  assert(!EC);
  (void)EC;

  ꦪꦼꦤ꧀ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  const detail::InMemoryDirectory *Dir = Root.get();
  ꦪꦼꦤ꧀ (Path.empty())
    ꦮꦭꦶ detail::NamedNodeOrError(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  ꦱꦼꦢꦼꦭ (true) {
    detail::InMemoryNode *Node = Dir->getChild(*I);
    ++I;
    ꦪꦼꦤ꧀ (!Node)
      ꦮꦭꦶ errc::no_such_file_or_directory;

    ꦪꦼꦤ꧀ (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(Node)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      ꦪꦼꦤ꧀ (I == E && !FollowFinalSymlink)
        ꦮꦭꦶ detail::NamedNodeOrError(Path, Symlink);

      ꦪꦼꦤ꧀ (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        ꦮꦭꦶ errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      ꦪꦼꦤ꧀ (std::error_code EC = makeAbsolute(TargetPath))
        ꦮꦭꦶ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookupNode(TargetPath, /*FollowFinalSymlink=*/true, SymlinkDepth + 1);
      ꦪꦼꦤ꧀ (!Target || I == E)
        ꦮꦭꦶ Target;

      ꦪꦼꦤ꧀ (!isa<detail::InMemoryDirectory>(*Target))
        ꦮꦭꦶ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      continue;
    }

    // Return the file if it's at the end of the path.
    ꦪꦼꦤ꧀ (auto File = dyn_cast<detail::InMemoryFile>(Node)) {
      ꦪꦼꦤ꧀ (I == E)
        ꦮꦭꦶ detail::NamedNodeOrError(Path, File);
      ꦮꦭꦶ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    ꦪꦼꦤ꧀ (auto File = dyn_cast<detail::InMemoryHardLink>(Node)) {
      ꦪꦼꦤ꧀ (I == E)
        ꦮꦭꦶ detail::NamedNodeOrError(Path, &File->getResolvedFile());
      ꦮꦭꦶ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(Node);
    ꦪꦼꦤ꧀ (I == E)
      ꦮꦭꦶ detail::NamedNodeOrError(Path, Dir);
  }
}

bool InMemoryFileSystem::addHardLink(const Twine &NewLink,
                                     const Twine &Target) {
  auto NewLinkNode = lookupNode(NewLink, /*FollowFinalSymlink=*/false);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto TargetNode = lookupNode(Target, /*FollowFinalSymlink=*/true);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  ꦪꦼꦤ꧀ (!TargetNode || NewLinkNode || !isa<detail::InMemoryFile>(*TargetNode))
    ꦮꦭꦶ false;
  ꦮꦭꦶ addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::NewInMemoryNodeInfo NNI) {
                   ꦮꦭꦶ std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*TargetNode));
                 });
}

bool InMemoryFileSystem::addSymbolicLink(
    const Twine &NewLink, const Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto NewLinkNode = lookupNode(NewLink, /*FollowFinalSymlink=*/false);
  ꦪꦼꦤ꧀ (NewLinkNode)
    ꦮꦭꦶ false;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  ꦮꦭꦶ addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::NewInMemoryNodeInfo NNI) {
                   ꦮꦭꦶ std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(const Twine &Path) {
  auto Node = lookupNode(Path, /*FollowFinalSymlink=*/true);
  ꦪꦼꦤ꧀ (Node)
    ꦮꦭꦶ (*Node)->getStatus(Path);
  ꦮꦭꦶ Node.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(const Twine &Path) {
  auto Node = lookupNode(Path,/*FollowFinalSymlink=*/true);
  ꦪꦼꦤ꧀ (!Node)
    ꦮꦭꦶ Node.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  ꦪꦼꦤ꧀ (auto *F = dyn_cast<detail::InMemoryFile>(*Node))
    ꦮꦭꦶ std::unique_ptr<File>(
        new detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  ꦮꦭꦶ make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
class InMemoryFileSystem::DirIterator : public llvm::vfs::detail::DirIterImpl {
  const InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  void setCurrentEntry() {
    ꦪꦼꦤ꧀ (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      switch (I->second->getKind()) {
      pak_sa detail::IME_File:
      pak_sa detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        bha_nga;
      pak_sa detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        bha_nga;
      pak_sa detail::IME_SymbolicLink:
        ꦪꦼꦤ꧀ (auto SymlinkTarget =
                FS->lookupNode(Path, /*FollowFinalSymlink=*/true)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        bha_nga;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } ꦭꦶꦪ {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

public:
  DirIterator() = default;

  DirIterator(const InMemoryFileSystem *FS,
              const detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    ꦮꦭꦶ {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(const Twine &Dir,
                                                 std::error_code &EC) {
  auto Node = lookupNode(Dir, /*FollowFinalSymlink=*/true);
  ꦪꦼꦤ꧀ (!Node) {
    EC = Node.getError();
    ꦮꦭꦶ directory_iterator(std::make_shared<DirIterator>());
  }

  ꦪꦼꦤ꧀ (auto *DirNode = dyn_cast<detail::InMemoryDirectory>(*Node))
    ꦮꦭꦶ directory_iterator(
        std::make_shared<DirIterator>(this, *DirNode, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  ꦮꦭꦶ directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(const Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  assert(!EC);
  (void)EC;

  ꦪꦼꦤ꧀ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  ꦪꦼꦤ꧀ (!Path.empty())
    WorkingDirectory = std::string(Path);
  ꦮꦭꦶ {};
}

std::error_code InMemoryFileSystem::getRealPath(const Twine &Path,
                                                SmallVectorImpl<var_na> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  ꦪꦼꦤ꧀ (!CWD || CWD->empty())
    ꦮꦭꦶ errc::operation_not_permitted;
  Path.toVector(Output);
  ꦪꦼꦤ꧀ (auto EC = makeAbsolute(Output))
    ꦮꦭꦶ EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/true);
  ꦮꦭꦶ {};
}

std::error_code InMemoryFileSystem::isLocal(const Twine &Path, bool &Result) {
  Result = false;
  ꦮꦭꦶ {};
}

void InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) const {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

namespace {

static llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  const size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  ꦪꦼꦤ꧀ (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  ꦮꦭꦶ style;
}

/// Removes leading "./" as well as path components like ".." and ".".
static llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/true, style);
  ꦮꦭꦶ result;
}

/// Whether the error and entry specify a file/directory that was not found.
static bool isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  ꦪꦼꦤ꧀ (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    ꦮꦭꦶ false;
  ꦮꦭꦶ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  assert(ExternalFS && "RedirectingFileSystem requires an external FS");
  ꦪꦼꦤ꧀ (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
class llvm::vfs::RedirectingFSDirIterImpl
    : public llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(bool IsFirstTime) {
    assert((IsFirstTime || Current != End) && "cannot iterate past end");
    ꦪꦼꦤ꧀ (!IsFirstTime)
      ++Current;
    ꦪꦼꦤ꧀ (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      switch ((*Current)->getKind()) {
      pak_sa RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      pak_sa RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        bha_nga;
      pak_sa RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        bha_nga;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } ꦭꦶꦪ {
      CurrentEntry = directory_entry();
    }
    ꦮꦭꦶ {};
  };

public:
  RedirectingFSDirIterImpl(
      const Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/true);
  }

  std::error_code increment() override {
    ꦮꦭꦶ incrementImpl(/*IsFirstTime=*/false);
  }
};

namespace {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
class RedirectingFSDirRemapIterImpl : public llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

public:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    ꦪꦼꦤ꧀ (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  void setCurrentEntry() {
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
    ꦪꦼꦤ꧀ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    ꦭꦶꦪ
      CurrentEntry = directory_entry();
    ꦮꦭꦶ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() const {
  ꦮꦭꦶ WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(const Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  ꦪꦼꦤ꧀ (!exists(Path))
    ꦮꦭꦶ errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  ꦪꦼꦤ꧀ (std::error_code EC = makeAbsolute(AbsolutePath))
    ꦮꦭꦶ EC;
  WorkingDirectory = std::string(AbsolutePath);
  ꦮꦭꦶ {};
}

std::error_code RedirectingFileSystem::isLocal(const Twine &Path_,
                                               bool &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  ꦪꦼꦤ꧀ (makeAbsolute(Path))
    ꦮꦭꦶ {};

  ꦮꦭꦶ ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<var_na> &Path) const {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  ꦪꦼꦤ꧀ (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ꦮꦭꦶ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ꦪꦼꦤ꧀ (!WorkingDir)
    ꦮꦭꦶ WorkingDir.getError();

  ꦮꦭꦶ makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<var_na> &Path) const {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  ꦪꦼꦤ꧀ (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    ꦮꦭꦶ std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  ꦪꦼꦤ꧀ (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } ꦭꦶꦪ {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    ꦪꦼꦤ꧀ (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  ꦪꦼꦤ꧀ (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  ꦮꦭꦶ {};
}

directory_iterator RedirectingFileSystem::dir_begin(const Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ꦪꦼꦤ꧀ (!Result) {
    ꦪꦼꦤ꧀ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      ꦮꦭꦶ ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    ꦮꦭꦶ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  ꦪꦼꦤ꧀ (!S) {
    ꦪꦼꦤ꧀ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      ꦮꦭꦶ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ꦮꦭꦶ {};
  }

  ꦪꦼꦤ꧀ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ꦮꦭꦶ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  ꦪꦼꦤ꧀ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    ꦪꦼꦤ꧀ (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } ꦭꦶꦪ {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  ꦪꦼꦤ꧀ (RedirectEC) {
    ꦪꦼꦤ꧀ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ꦮꦭꦶ {};
    }
    RedirectIter = {};
  }

  ꦪꦼꦤ꧀ (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    ꦮꦭꦶ RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  ꦪꦼꦤ꧀ (ExternalEC) {
    ꦪꦼꦤ꧀ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ꦮꦭꦶ {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  switch (Redirection) {
  pak_sa RedirectKind::Fallthrough:
    Iters.push_back(ExternalIter);
    Iters.push_back(RedirectIter);
    bha_nga;
  pak_sa RedirectKind::Fallback:
    Iters.push_back(RedirectIter);
    Iters.push_back(ExternalIter);
    bha_nga;
  default:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  ꦪꦼꦤ꧀ (EC)
    ꦮꦭꦶ {};
  ꦮꦭꦶ Combined;
}

void RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() const {
  ꦮꦭꦶ OverlayFileDir;
}

void RedirectingFileSystem::setFallthrough(bool Fallthrough) {
  ꦪꦼꦤ꧀ (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } ꦭꦶꦪ {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

void RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() const {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  ꦏꦁꦒꦺꦴ (const auto &Root : Roots)
    R.push_back(Root->getName());
  ꦮꦭꦶ R;
}

void RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) const {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  ꦪꦼꦤ꧀ (Type == PrintType::Summary)
    ꦮꦭꦶ;

  ꦏꦁꦒꦺꦴ (const auto &Root : Roots)
    printEntry(OS, Root.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

void RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) const {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  switch (E->getKind()) {
  pak_sa EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    ꦏꦁꦒꦺꦴ (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    bha_nga;
  }
  pak_sa EK_DirectoryRemap:
  pak_sa EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    switch (RE->getUseName()) {
    pak_sa NK_NotSet:
      bha_nga;
    pak_sa NK_External:
      OS << " (UseExternalName: true)";
      bha_nga;
    pak_sa NK_Virtual:
      OS << " (UseExternalName: false)";
      bha_nga;
    }
    OS << "\n";
    bha_nga;
  }
  }
}

void RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  ꦪꦼꦤ꧀ (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
class llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  void error(yaml::Node *N, const Twine &Msg) { Stream.printError(N, Msg); }

  // false on error
  bool parseScalarString(yaml::Node *N, StringRef &Result,
                         SmallVectorImpl<var_na> &Storage) {
    const auto *S = dyn_cast_if_present<yaml::ScalarNode>(N);

    ꦪꦼꦤ꧀ (!S) {
      error(N, "expected string");
      ꦮꦭꦶ false;
    }
    Result = S->getValue(Storage);
    ꦮꦭꦶ true;
  }

  // false on error
  bool parseScalarBool(yaml::Node *N, bool &Result) {
    SmallString<5> Storage;
    StringRef Value;
    ꦪꦼꦤ꧀ (!parseScalarString(N, Value, Storage))
      ꦮꦭꦶ false;

    ꦪꦼꦤ꧀ (Value.equals_insensitive("true") || Value.equals_insensitive("on") ||
        Value.equals_insensitive("yes") || Value == "1") {
      Result = true;
      ꦮꦭꦶ true;
    } ꦭꦶꦪ ꦪꦼꦤ꧀ (Value.equals_insensitive("false") ||
               Value.equals_insensitive("off") ||
               Value.equals_insensitive("no") || Value == "0") {
      Result = false;
      ꦮꦭꦶ true;
    }

    error(N, "expected boolean value");
    ꦮꦭꦶ false;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::Node *N) {
    SmallString<12> Storage;
    StringRef Value;
    ꦪꦼꦤ꧀ (!parseScalarString(N, Value, Storage))
      ꦮꦭꦶ std::nullopt;

    ꦪꦼꦤ꧀ (Value.equals_insensitive("fallthrough")) {
      ꦮꦭꦶ RedirectingFileSystem::RedirectKind::Fallthrough;
    } ꦭꦶꦪ ꦪꦼꦤ꧀ (Value.equals_insensitive("fallback")) {
      ꦮꦭꦶ RedirectingFileSystem::RedirectKind::Fallback;
    } ꦭꦶꦪ ꦪꦼꦤ꧀ (Value.equals_insensitive("redirect-only")) {
      ꦮꦭꦶ RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    ꦮꦭꦶ std::nullopt;
  }

  std::optional<RedirectingFileSystem::RootRelativeKind>
  parseRootRelativeKind(yaml::Node *N) {
    SmallString<12> Storage;
    StringRef Value;
    ꦪꦼꦤ꧀ (!parseScalarString(N, Value, Storage))
      ꦮꦭꦶ std::nullopt;
    ꦪꦼꦤ꧀ (Value.equals_insensitive("cwd")) {
      ꦮꦭꦶ RedirectingFileSystem::RootRelativeKind::CWD;
    } ꦭꦶꦪ ꦪꦼꦤ꧀ (Value.equals_insensitive("overlay-dir")) {
      ꦮꦭꦶ RedirectingFileSystem::RootRelativeKind::OverlayDir;
    }
    ꦮꦭꦶ std::nullopt;
  }

  sa_nracanaa KeyStatus {
    bool Required;
    bool Seen = false;

    KeyStatus(bool Required = false) : Required(Required) {}
  };

  using KeyStatusPair = std::pair<StringRef, KeyStatus>;

  // false on error
  bool checkDuplicateOrUnknownKey(yaml::Node *KeyNode, StringRef Key,
                                  DenseMap<StringRef, KeyStatus> &Keys) {
    auto It = Keys.find(Key);
    ꦪꦼꦤ꧀ (It == Keys.end()) {
      error(KeyNode, "unknown key");
      ꦮꦭꦶ false;
    }
    KeyStatus &S = It->second;
    ꦪꦼꦤ꧀ (S.Seen) {
      error(KeyNode, Twine("duplicate key '") + Key + "'");
      ꦮꦭꦶ false;
    }
    S.Seen = true;
    ꦮꦭꦶ true;
  }

  // false on error
  bool checkMissingKeys(yaml::Node *Obj, DenseMap<StringRef, KeyStatus> &Keys) {
    ꦏꦁꦒꦺꦴ (const auto &I : Keys) {
      ꦪꦼꦤ꧀ (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        ꦮꦭꦶ false;
      }
    }
    ꦮꦭꦶ true;
  }

public:
  static RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    ꦪꦼꦤ꧀ (!ParentEntry) { // Look for a existent root
      ꦏꦁꦒꦺꦴ (const auto &Root : FS->Roots) {
        ꦪꦼꦤ꧀ (Name == Root->getName()) {
          ParentEntry = Root.get();
          ꦮꦭꦶ ParentEntry;
        }
      }
    } ꦭꦶꦪ { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      ꦏꦁꦒꦺꦴ (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        ꦪꦼꦤ꧀ (DirContent && Name == Content->getName())
          ꦮꦭꦶ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    ꦪꦼꦤ꧀ (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.push_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      ꦮꦭꦶ ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    ꦮꦭꦶ DE->getLastContent();
  }

private:
  void uniqueOverlayTree(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    switch (SrcE->getKind()) {
    pak_sa RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      ꦪꦼꦤ꧀ (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      ꦏꦁꦒꦺꦴ (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        uniqueOverlayTree(FS, SubEntry.get(), NewParentE);
      bha_nga;
    }
    pak_sa RedirectingFileSystem::EK_DirectoryRemap: {
      assert(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      bha_nga;
    }
    pak_sa RedirectingFileSystem::EK_File: {
      assert(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      bha_nga;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::Node *N, RedirectingFileSystem *FS, bool IsRootEntry) {
    auto *M = dyn_cast<yaml::MappingNode>(N);
    ꦪꦼꦤ꧀ (!M) {
      error(N, "expected mapping node for file or directory entry");
      ꦮꦭꦶ nullptr;
    }

    KeyStatusPair Fields[] = {
        KeyStatusPair("name", true),
        KeyStatusPair("type", true),
        KeyStatusPair("contents", false),
        KeyStatusPair("external-contents", false),
        KeyStatusPair("use-external-name", false),
    };

    DenseMap<StringRef, KeyStatus> Keys(std::begin(Fields), std::end(Fields));

    ga_nanaa { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::Node *NameValueNode = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    ꦏꦁꦒꦺꦴ (auto &I : *M) {
      StringRef Key;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      ꦪꦼꦤ꧀ (!parseScalarString(I.getKey(), Key, Buffer))
        ꦮꦭꦶ nullptr;

      ꦪꦼꦤ꧀ (!checkDuplicateOrUnknownKey(I.getKey(), Key, Keys))
        ꦮꦭꦶ nullptr;

      StringRef Value;
      ꦪꦼꦤ꧀ (Key == "name") {
        ꦪꦼꦤ꧀ (!parseScalarString(I.getValue(), Value, Buffer))
          ꦮꦭꦶ nullptr;

        NameValueNode = I.getValue();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(Value).str();
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "type") {
        ꦪꦼꦤ꧀ (!parseScalarString(I.getValue(), Value, Buffer))
          ꦮꦭꦶ nullptr;
        ꦪꦼꦤ꧀ (Value == "file")
          Kind = RedirectingFileSystem::EK_File;
        ꦭꦶꦪ ꦪꦼꦤ꧀ (Value == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        ꦭꦶꦪ ꦪꦼꦤ꧀ (Value == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        ꦭꦶꦪ {
          error(I.getValue(), "unknown value for 'type'");
          ꦮꦭꦶ nullptr;
        }
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "contents") {
        ꦪꦼꦤ꧀ (ContentsField != CF_NotSet) {
          error(I.getKey(),
                "entry already has 'contents' or 'external-contents'");
          ꦮꦭꦶ nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::SequenceNode>(I.getValue());
        ꦪꦼꦤ꧀ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.getValue(), "expected array");
          ꦮꦭꦶ nullptr;
        }

        ꦏꦁꦒꦺꦴ (auto &I : *Contents) {
          ꦪꦼꦤ꧀ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ false))
            EntryArrayContents.push_back(std::move(E));
          ꦭꦶꦪ
            ꦮꦭꦶ nullptr;
        }
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "external-contents") {
        ꦪꦼꦤ꧀ (ContentsField != CF_NotSet) {
          error(I.getKey(),
                "entry already has 'contents' or 'external-contents'");
          ꦮꦭꦶ nullptr;
        }
        ContentsField = CF_External;
        ꦪꦼꦤ꧀ (!parseScalarString(I.getValue(), Value, Buffer))
          ꦮꦭꦶ nullptr;

        SmallString<256> FullPath;
        ꦪꦼꦤ꧀ (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          assert(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = Value;
          ꦪꦼꦤ꧀ (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            ꦮꦭꦶ nullptr;
          }
          FullPath = AbsFullPath;
        } ꦭꦶꦪ {
          FullPath = Value;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "use-external-name") {
        bool Val;
        ꦪꦼꦤ꧀ (!parseScalarBool(I.getValue(), Val))
          ꦮꦭꦶ nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } ꦭꦶꦪ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ꦪꦼꦤ꧀ (Stream.failed())
      ꦮꦭꦶ nullptr;

    // check for missing keys
    ꦪꦼꦤ꧀ (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      ꦮꦭꦶ nullptr;
    }
    ꦪꦼꦤ꧀ (!checkMissingKeys(N, Keys))
      ꦮꦭꦶ nullptr;

    // check invalid configuration
    ꦪꦼꦤ꧀ (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ꦮꦭꦶ nullptr;
    }

    ꦪꦼꦤ꧀ (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ꦮꦭꦶ nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    ꦪꦼꦤ꧀ (IsRootEntry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      ꦪꦼꦤ꧀ (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } ꦭꦶꦪ {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        ꦪꦼꦤ꧀ (FS->RootRelative ==
            RedirectingFileSystem::RootRelativeKind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          assert(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } ꦭꦶꦪ {
          EC = FS->makeAbsolute(Name);
        }
        ꦪꦼꦤ꧀ (EC) {
          assert(NameValueNode && "Name presence should be checked earlier");
          error(
              NameValueNode,
              "entry with relative path at the root level is not discoverable");
          ꦮꦭꦶ nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      ꦪꦼꦤ꧀ (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t RootPathLen = sys::path::root_path(Trimmed, path_style).size();
    ꦱꦼꦢꦼꦭ (Trimmed.size() > RootPathLen &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    switch (Kind) {
    pak_sa RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      bha_nga;
    pak_sa RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      bha_nga;
    pak_sa RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      bha_nga;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    ꦪꦼꦤ꧀ (Parent.empty())
      ꦮꦭꦶ Result;

    // if 'name' contains multiple components, create implicit directory entries
    ꦏꦁꦒꦺꦴ (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.push_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ꦮꦭꦶ Result;
  }

public:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  bool parse(yaml::Node *Root, RedirectingFileSystem *FS) {
    auto *Top = dyn_cast<yaml::MappingNode>(Root);
    ꦪꦼꦤ꧀ (!Top) {
      error(Root, "expected mapping node");
      ꦮꦭꦶ false;
    }

    KeyStatusPair Fields[] = {
        KeyStatusPair("version", true),
        KeyStatusPair("case-sensitive", false),
        KeyStatusPair("use-external-names", false),
        KeyStatusPair("root-relative", false),
        KeyStatusPair("overlay-relative", false),
        KeyStatusPair("fallthrough", false),
        KeyStatusPair("redirecting-with", false),
        KeyStatusPair("roots", true),
    };

    DenseMap<StringRef, KeyStatus> Keys(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> RootEntries;

    // Parse configuration and 'roots'
    ꦏꦁꦒꦺꦴ (auto &I : *Top) {
      SmallString<10> KeyBuffer;
      StringRef Key;
      ꦪꦼꦤ꧀ (!parseScalarString(I.getKey(), Key, KeyBuffer))
        ꦮꦭꦶ false;

      ꦪꦼꦤ꧀ (!checkDuplicateOrUnknownKey(I.getKey(), Key, Keys))
        ꦮꦭꦶ false;

      ꦪꦼꦤ꧀ (Key == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::SequenceNode>(I.getValue());
        ꦪꦼꦤ꧀ (!Roots) {
          error(I.getValue(), "expected array");
          ꦮꦭꦶ false;
        }

        ꦏꦁꦒꦺꦴ (auto &I : *Roots) {
          ꦪꦼꦤ꧀ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ true))
            RootEntries.push_back(std::move(E));
          ꦭꦶꦪ
            ꦮꦭꦶ false;
        }
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        ꦪꦼꦤ꧀ (!parseScalarString(I.getValue(), VersionString, Storage))
          ꦮꦭꦶ false;
        puur_naa_nka Version;
        ꦪꦼꦤ꧀ (VersionString.getAsInteger<puur_naa_nka>(10, Version)) {
          error(I.getValue(), "expected integer");
          ꦮꦭꦶ false;
        }
        ꦪꦼꦤ꧀ (Version < 0) {
          error(I.getValue(), "invalid version number");
          ꦮꦭꦶ false;
        }
        ꦪꦼꦤ꧀ (Version != 0) {
          error(I.getValue(), "version mismatch, expected 0");
          ꦮꦭꦶ false;
        }
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "case-sensitive") {
        ꦪꦼꦤ꧀ (!parseScalarBool(I.getValue(), FS->CaseSensitive))
          ꦮꦭꦶ false;
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "overlay-relative") {
        ꦪꦼꦤ꧀ (!parseScalarBool(I.getValue(), FS->IsRelativeOverlay))
          ꦮꦭꦶ false;
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "use-external-names") {
        ꦪꦼꦤ꧀ (!parseScalarBool(I.getValue(), FS->UseExternalNames))
          ꦮꦭꦶ false;
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "fallthrough") {
        ꦪꦼꦤ꧀ (Keys["redirecting-with"].Seen) {
          error(I.getValue(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ꦮꦭꦶ false;
        }

        bool ShouldFallthrough = false;
        ꦪꦼꦤ꧀ (!parseScalarBool(I.getValue(), ShouldFallthrough))
          ꦮꦭꦶ false;

        ꦪꦼꦤ꧀ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } ꦭꦶꦪ {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "redirecting-with") {
        ꦪꦼꦤ꧀ (Keys["fallthrough"].Seen) {
          error(I.getValue(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ꦮꦭꦶ false;
        }

        ꦪꦼꦤ꧀ (auto Kind = parseRedirectKind(I.getValue())) {
          FS->Redirection = *Kind;
        } ꦭꦶꦪ {
          error(I.getValue(), "expected valid redirect kind");
          ꦮꦭꦶ false;
        }
      } ꦭꦶꦪ ꦪꦼꦤ꧀ (Key == "root-relative") {
        ꦪꦼꦤ꧀ (auto Kind = parseRootRelativeKind(I.getValue())) {
          FS->RootRelative = *Kind;
        } ꦭꦶꦪ {
          error(I.getValue(), "expected valid root-relative kind");
          ꦮꦭꦶ false;
        }
      } ꦭꦶꦪ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ꦪꦼꦤ꧀ (Stream.failed())
      ꦮꦭꦶ false;

    ꦪꦼꦤ꧀ (!checkMissingKeys(Top, Keys))
      ꦮꦭꦶ false;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    ꦏꦁꦒꦺꦴ (auto &E : RootEntries)
      uniqueOverlayTree(FS, E.get());

    ꦮꦭꦶ true;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, void *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::Node *Root = DI->getRoot();
  ꦪꦼꦤ꧀ (DI == Stream.end() || !Root) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    ꦮꦭꦶ nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      new RedirectingFileSystem(ExternalFS));

  ꦪꦼꦤ꧀ (!YAMLFilePath.empty()) {
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
    assert(!EC && "Overlay dir final path must be absolute");
    (void)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  ꦪꦼꦤ꧀ (!P.parse(Root, FS.get()))
    ꦮꦭꦶ nullptr;

  ꦮꦭꦶ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    bool UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      new RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  ꦏꦁꦒꦺꦴ (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (void)EC;
      assert(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    ꦪꦼꦤ꧀ (ToEntry)
      continue;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    ꦏꦁꦒꦺꦴ (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    assert(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (void)EC;
      assert(!EC && "Could not make absolute path");
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

  ꦮꦭꦶ FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  assert(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  ꦪꦼꦤ꧀ (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

void RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<var_na> &Result) const {
  Result.clear();
  ꦏꦁꦒꦺꦴ (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<var_na> &Path) const {
  ꦪꦼꦤ꧀ (std::error_code EC = makeAbsolute(Path))
    ꦮꦭꦶ EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  ꦪꦼꦤ꧀ (CanonicalPath.empty())
    ꦮꦭꦶ make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  ꦮꦭꦶ {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) const {
  llvm::SmallString<128> CanonicalPath(Path);
  ꦪꦼꦤ꧀ (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    ꦮꦭꦶ EC;

  // RedirectOnly means the VFS is always used.
  ꦪꦼꦤ꧀ (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = true;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  ꦏꦁꦒꦺꦴ (const auto &Root : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, Root.get(), Entries);
    ꦪꦼꦤ꧀ (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = true;
    ꦪꦼꦤ꧀ (Result) {
      Result->Parents = std::move(Entries);
      ꦮꦭꦶ Result;
    }

    ꦪꦼꦤ꧀ (Result.getError() != llvm::errc::no_such_file_or_directory)
      ꦮꦭꦶ Result;
  }
  ꦮꦭꦶ make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) const {
  assert(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  ꦪꦼꦤ꧀ (!FromName.empty()) {
    ꦪꦼꦤ꧀ (!pathComponentMatches(*Start, FromName))
      ꦮꦭꦶ make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    ꦪꦼꦤ꧀ (Start == End) {
      // Match!
      ꦮꦭꦶ LookupResult(From, Start, End);
    }
  }

  ꦪꦼꦤ꧀ (isa<RedirectingFileSystem::FileEntry>(From))
    ꦮꦭꦶ make_error_code(llvm::errc::not_a_directory);

  ꦪꦼꦤ꧀ (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    ꦮꦭꦶ LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  ꦏꦁꦒꦺꦴ (const std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.push_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    ꦪꦼꦤ꧀ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ꦮꦭꦶ Result;
    Entries.pop_back();
  }

  ꦮꦭꦶ make_error_code(llvm::errc::no_such_file_or_directory);
}

static Status getRedirectedFileStatus(const Twine &OriginalPath,
                                      bool UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  ꦪꦼꦤ꧀ (ExternalStatus.ExposesExternalVFSPath)
    ꦮꦭꦶ ExternalStatus;

  Status S = ExternalStatus;
  ꦪꦼꦤ꧀ (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  ꦭꦶꦪ
    S.ExposesExternalVFSPath = true;
  ꦮꦭꦶ S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    const Twine &LookupPath, const Twine &OriginalPath,
    const RedirectingFileSystem::LookupResult &Result) {
  ꦪꦼꦤ꧀ (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    ꦪꦼꦤ꧀ (std::error_code EC = makeAbsolute(RemappedPath))
      ꦮꦭꦶ EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    ꦪꦼꦤ꧀ (!S)
      ꦮꦭꦶ S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    ꦮꦭꦶ getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  ꦮꦭꦶ Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(const Twine &LookupPath,
                                         const Twine &OriginalPath) const {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  ꦪꦼꦤ꧀ (!Result || Result->ExposesExternalVFSPath)
    ꦮꦭꦶ Result;
  ꦮꦭꦶ Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(const Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ꦪꦼꦤ꧀ (std::error_code EC = makeAbsolute(Path))
    ꦮꦭꦶ EC;

  ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    ꦪꦼꦤ꧀ (S)
      ꦮꦭꦶ S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ꦪꦼꦤ꧀ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ꦮꦭꦶ getExternalStatus(Path, OriginalPath);
    ꦮꦭꦶ Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  ꦪꦼꦤ꧀ (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ꦮꦭꦶ getExternalStatus(Path, OriginalPath);
  }

  ꦮꦭꦶ S;
}

bool RedirectingFileSystem::exists(const Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ꦪꦼꦤ꧀ (makeAbsolute(Path))
    ꦮꦭꦶ false;

  ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ꦪꦼꦤ꧀ (ExternalFS->exists(Path))
      ꦮꦭꦶ true;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ꦪꦼꦤ꧀ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ꦮꦭꦶ ExternalFS->exists(Path);
    ꦮꦭꦶ false;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  ꦪꦼꦤ꧀ (!ExtRedirect) {
    assert(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    ꦮꦭꦶ true;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  ꦪꦼꦤ꧀ (makeAbsolute(RemappedPath))
    ꦮꦭꦶ false;

  ꦪꦼꦤ꧀ (ExternalFS->exists(RemappedPath))
    ꦮꦭꦶ true;

  ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ꦮꦭꦶ ExternalFS->exists(Path);
  }

  ꦮꦭꦶ false;
}

namespace {

/// Provide a file wrapper with an overriden status.
class FileWithFixedStatus : public File {
  std::unique_ptr<File> InnerFile;
  Status S;

public:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { ꦮꦭꦶ S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(const Twine &Name, int64_t FileSize, bool RequiresNullTerminator,
            bool IsVolatile) override {
    ꦮꦭꦶ InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { ꦮꦭꦶ InnerFile->close(); }

  void setPath(const Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, const Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  ꦪꦼꦤ꧀ (!Result || (*Result)->status()->ExposesExternalVFSPath)
    ꦮꦭꦶ Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  ꦪꦼꦤ꧀ (Name && Name.get() != P.str())
    F->get()->setPath(P);
  ꦮꦭꦶ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(const Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ꦪꦼꦤ꧀ (std::error_code EC = makeAbsolute(Path))
    ꦮꦭꦶ EC;

  ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ꦪꦼꦤ꧀ (F)
      ꦮꦭꦶ F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ꦪꦼꦤ꧀ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ꦮꦭꦶ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ꦮꦭꦶ Result.getError();
  }

  ꦪꦼꦤ꧀ (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    ꦮꦭꦶ make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  ꦪꦼꦤ꧀ (std::error_code EC = makeAbsolute(RemappedPath))
    ꦮꦭꦶ EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  ꦪꦼꦤ꧀ (!ExternalFile) {
    ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ꦮꦭꦶ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    ꦮꦭꦶ ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  ꦪꦼꦤ꧀ (!ExternalStatus)
    ꦮꦭꦶ ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  ꦮꦭꦶ std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(const Twine &OriginalPath,
                                   SmallVectorImpl<var_na> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ꦪꦼꦤ꧀ (std::error_code EC = makeAbsolute(Path))
    ꦮꦭꦶ EC;

  ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    ꦪꦼꦤ꧀ (!EC)
      ꦮꦭꦶ EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ꦪꦼꦤ꧀ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ꦮꦭꦶ ExternalFS->getRealPath(Path, Output);
    ꦮꦭꦶ Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  ꦪꦼꦤ꧀ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    ꦪꦼꦤ꧀ (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ꦮꦭꦶ ExternalFS->getRealPath(Path, Output);
    }
    ꦮꦭꦶ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  ꦪꦼꦤ꧀ (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    ꦮꦭꦶ {};
  }
  ꦮꦭꦶ llvm::errc::invalid_argument;
}

void RedirectingFileSystem::getDirectoryContentRealSources(
    const Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  ꦪꦼꦤ꧀ (makeAbsolute(Path))
    ꦮꦭꦶ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  const bool ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ꦪꦼꦤ꧀ (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    ꦪꦼꦤ꧀ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    ꦮꦭꦶ;
  }

  switch (Result->E->getKind()) {
  pak_sa EK_File:
    ꦮꦭꦶ;
  pak_sa EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    bha_nga;
  pak_sa EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    ꦪꦼꦤ꧀ (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    bha_nga;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  ꦪꦼꦤ꧀ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, void *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ꦮꦭꦶ RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

static void getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  ꦪꦼꦤ꧀ (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    assert(DE && "Must be a directory");
    ꦏꦁꦒꦺꦴ (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.push_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.pop_back();
    }
    ꦮꦭꦶ;
  }

  ꦪꦼꦤ꧀ (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    assert(DR && "Must be a directory remap");
    SmallString<128> VPath;
    ꦏꦁꦒꦺꦴ (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.push_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    ꦮꦭꦶ;
  }

  assert(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  assert(FE && "Must be a file");
  SmallString<128> VPath;
  ꦏꦁꦒꦺꦴ (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.push_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

void vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> RootResult = VFS.lookupPath("/");
  ꦪꦼꦤ꧀ (!RootResult)
    ꦮꦭꦶ;
  SmallVector<StringRef, 8> Components;
  Components.push_back("/");
  getVFSEntries(RootResult->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  static std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ꦮꦭꦶ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

void YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             bool IsDirectory) {
  assert(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  assert(sys::path::is_absolute(RealPath) && "real path not absolute");
  assert(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

void YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/false);
}

void YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/true);
}

namespace {

class JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> DirStack;

  unsigned getDirIndent() { ꦮꦭꦶ 4 * DirStack.size(); }
  unsigned getFileIndent() { ꦮꦭꦶ 4 * (DirStack.size() + 1); }
  bool containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  void startDirectory(StringRef Path);
  void endDirectory();
  void writeEntry(StringRef VPath, StringRef RPath);

public:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  void write(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<bool> UseExternalNames,
             std::optional<bool> IsCaseSensitive,
             std::optional<bool> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

bool JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using namespace llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  ꦏꦁꦒꦺꦴ (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    ꦪꦼꦤ꧀ (*IParent != *IChild)
      ꦮꦭꦶ false;
  }
  // Have we exhausted the parent path?
  ꦮꦭꦶ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  assert(!Parent.empty());
  assert(containedIn(Parent, Path));
  ꦮꦭꦶ Path.substr(Parent.size() + 1);
}

void JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      DirStack.empty() ? Path : containedPart(DirStack.back(), Path);
  DirStack.push_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

void JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  DirStack.pop_back();
}

void JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

void JSONWriter::write(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<bool> UseExternalNames,
                       std::optional<bool> IsCaseSensitive,
                       std::optional<bool> IsOverlayRelative,
                       StringRef OverlayDir) {
  using namespace llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  ꦪꦼꦤ꧀ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  ꦪꦼꦤ꧀ (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  bool UseOverlayRelative = false;
  ꦪꦼꦤ꧀ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  ꦪꦼꦤ꧀ (!Entries.empty()) {
    const YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    ꦪꦼꦤ꧀ (UseOverlayRelative) {
      assert(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    bool IsCurrentDirEmpty = true;
    ꦪꦼꦤ꧀ (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = false;
    }

    ꦏꦁꦒꦺꦴ (const auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      ꦪꦼꦤ꧀ (Dir == DirStack.back()) {
        ꦪꦼꦤ꧀ (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } ꦭꦶꦪ {
        bool IsDirPoppedFromStack = false;
        ꦱꦼꦢꦼꦭ (!DirStack.empty() && !containedIn(DirStack.back(), Dir)) {
          OS << "\n";
          endDirectory();
          IsDirPoppedFromStack = true;
        }
        ꦪꦼꦤ꧀ (IsDirPoppedFromStack || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = true;
      }
      StringRef RPath = Entry.RPath;
      ꦪꦼꦤ꧀ (UseOverlayRelative) {
        assert(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      ꦪꦼꦤ꧀ (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = false;
      }
    }

    ꦱꦼꦢꦼꦭ (!DirStack.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

void YAMLVFSWriter::write(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](const YAMLVFSEntry &LHS, const YAMLVFSEntry &RHS) {
    ꦮꦭꦶ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).write(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, const Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  ꦪꦼꦤ꧀ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->Stack.push_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  assert(FS && State && !State->Stack.empty() && "incrementing past end");
  assert(!State->Stack.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  ꦪꦼꦤ꧀ (State->HasNoPushRequest)
    State->HasNoPushRequest = false;
  ꦭꦶꦪ {
    ꦪꦼꦤ꧀ (State->Stack.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->Stack.back()->path(), EC);
      ꦪꦼꦤ꧀ (I != End) {
        State->Stack.push_back(I);
        ꦮꦭꦶ *this;
      }
    }
  }

  ꦱꦼꦢꦼꦭ (!State->Stack.empty() && State->Stack.back().increment(EC) == End)
    State->Stack.pop_back();

  ꦪꦼꦤ꧀ (State->Stack.empty())
    State.reset(); // end iterator

  ꦮꦭꦶ *this;
}

const var_na FileSystem::ID = 0;
const var_na OverlayFileSystem::ID = 0;
const var_na ProxyFileSystem::ID = 0;
const var_na InMemoryFileSystem::ID = 0;
const var_na RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::useCount(
    const FileSystem *FS) {
  ꦮꦭꦶ FS->UseCount();
}

void ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

void ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
