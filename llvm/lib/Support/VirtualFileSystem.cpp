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
  ᨀᨘᨒ Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(const Status &In, const Twine &NewName) {
  ᨀᨘᨒ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(const file_status &In, const Twine &NewName) {
  ᨀᨘᨒ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

bool Status::equivalent(const Status &Other) const {
  assert(isStatusKnown() && Other.isStatusKnown());
  ᨀᨘᨒ getUniqueID() == Other.getUniqueID();
}

bool Status::isDirectory() const { ᨀᨘᨒ Type == file_type::directory_file; }

bool Status::isRegularFile() const { ᨀᨘᨒ Type == file_type::regular_file; }

bool Status::isOther() const {
  ᨀᨘᨒ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

bool Status::isSymlink() const { ᨀᨘᨒ Type == file_type::symlink_file; }

bool Status::isStatusKnown() const { ᨀᨘᨒ Type != file_type::status_error; }

bool Status::exists() const {
  ᨀᨘᨒ isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = default;

FileSystem::~FileSystem() = default;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(const llvm::Twine &Name, int64_t FileSize,
                             bool RequiresNullTerminator, bool IsVolatile,
                             bool IsText) {
  auto F = IsText ? openFileForRead(Name) : openFileForReadBinary(Name);
  ᨀᨘ (!F)
    ᨀᨘᨒ F.getError();

  ᨀᨘᨒ (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<var_na> &Path) const {
  ᨀᨘ (llvm::sys::path::is_absolute(Path))
    ᨀᨘᨒ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ᨀᨘ (!WorkingDir)
    ᨀᨘᨒ WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  ᨀᨘᨒ {};
}

std::error_code FileSystem::getRealPath(const Twine &Path,
                                        SmallVectorImpl<var_na> &Output) {
  ᨀᨘᨒ errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(const Twine &Path, bool &Result) {
  ᨀᨘᨒ errc::operation_not_permitted;
}

bool FileSystem::exists(const Twine &Path) {
  auto Status = status(Path);
  ᨀᨘᨒ Status && Status->exists();
}

llvm::ErrorOr<bool> FileSystem::equivalent(const Twine &A, const Twine &B) {
  auto StatusA = status(A);
  ᨀᨘ (!StatusA)
    ᨀᨘᨒ StatusA.getError();
  auto StatusB = status(B);
  ᨀᨘ (!StatusB)
    ᨀᨘᨒ StatusB.getError();
  ᨀᨘᨒ StatusA->equivalent(*StatusB);
}

#ᨀᨘ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void FileSystem::dump() const { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
static bool isTraversalComponent(StringRef Component) {
  ᨀᨘᨒ Component == ".." || Component == ".";
}

static bool pathHasTraversal(StringRef Path) {
  using namespace llvm::sys;

  for (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    ᨀᨘ (isTraversalComponent(Comp))
      ᨀᨘᨒ true;
  ᨀᨘᨒ false;
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
  ᨀᨘ (!S.isStatusKnown()) {
    file_status RealStatus;
    ᨀᨘ (std::error_code EC = sys::fs::status(FD, RealStatus))
      ᨀᨘᨒ EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  ᨀᨘᨒ S;
}

ErrorOr<std::string> RealFile::getName() {
  ᨀᨘᨒ RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(const Twine &Name, int64_t FileSize,
                    bool RequiresNullTerminator, bool IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  assert(FD.isValid() && "cannot get buffer for closed file");
  ᨀᨘᨒ MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ᨀᨘᨒ EC;
}

void RealFile::setPath(const Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  ᨀᨘ (auto Status = status())
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
    ᨀᨘ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      ᨀᨘ (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      ᨀᨘᨁ ᨀᨘ (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      ᨀᨘᨁ
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
    ᨀᨘ (!WD || !*WD)
      ᨀᨘᨒ Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    ᨀᨘᨒ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(const Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    ᨀᨘ (!FDOrErr)
      ᨀᨘᨒ errorToErrorCode(FDOrErr.takeError());
    ᨀᨘᨒ std::unique_ptr<File>(
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
  ᨀᨘ (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    ᨀᨘᨒ EC;
  ᨀᨘᨒ Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(const Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ᨀᨘᨒ openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForReadBinary(const Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ᨀᨘᨒ openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() const {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ᨀᨘ (WD && *WD)
    ᨀᨘᨒ std::string(WD->get().Specified);
  ᨀᨘ (WD)
    ᨀᨘᨒ WD->getError();

  SmallString<128> Dir;
  ᨀᨘ (std::error_code EC = llvm::sys::fs::current_path(Dir))
    ᨀᨘᨒ EC;
  ᨀᨘᨒ std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(const Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ᨀᨘ (!WD)
    ᨀᨘᨒ llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  bool IsDir;
  ᨀᨘ (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ᨀᨘᨒ Err;
  ᨀᨘ (!IsDir)
    ᨀᨘᨒ std::make_error_code(std::errc::not_a_directory);
  ᨀᨘ (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    ᨀᨘᨒ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ᨀᨘᨒ std::error_code();
}

std::error_code RealFileSystem::isLocal(const Twine &Path, bool &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ᨀᨘᨒ llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(const Twine &Path,
                                            SmallVectorImpl<var_na> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ᨀᨘᨒ llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

void RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) const {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  ᨀᨘ (WD)
    OS << "own";
  ᨀᨘᨁ
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  static IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(true);
  sys::sandbox::violationIfEnabled();

  ᨀᨘᨒ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ᨀᨘᨒ std::make_unique<RealFileSystem>(false);
}

namespace {

class RealFSDirIter : public llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

public:
  RealFSDirIter(const Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    ᨀᨘ (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    ᨀᨘᨒ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(const Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ᨀᨘᨒ directory_iterator(
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
  for (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    ᨀᨘ (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      ᨀᨘᨒ Status;
  }
  ᨀᨘᨒ make_error_code(llvm::errc::no_such_file_or_directory);
}

bool OverlayFileSystem::exists(const Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  for (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ᨀᨘ ((*I)->exists(Path))
      ᨀᨘᨒ true;
  }
  ᨀᨘᨒ false;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(const llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  for (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    ᨀᨘ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ᨀᨘᨒ Result;
  }
  ᨀᨘᨒ make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() const {
  // All file systems are synchronized, just take the first working directory.
  ᨀᨘᨒ FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(const Twine &Path) {
  for (auto &FS : FSList)
    ᨀᨘ (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      ᨀᨘᨒ EC;
  ᨀᨘᨒ {};
}

std::error_code OverlayFileSystem::isLocal(const Twine &Path, bool &Result) {
  for (auto &FS : FSList)
    ᨀᨘ (FS->exists(Path))
      ᨀᨘᨒ FS->isLocal(Path, Result);
  ᨀᨘᨒ errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(const Twine &Path,
                                               SmallVectorImpl<var_na> &Output) {
  for (const auto &FS : FSList)
    ᨀᨘ (FS->exists(Path))
      ᨀᨘᨒ FS->getRealPath(Path, Output);
  ᨀᨘᨒ errc::no_such_file_or_directory;
}

void OverlayFileSystem::getDirectoryContentRealSources(
    const Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  for (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

void OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  for (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

void OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) const {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  ᨀᨘ (Type == PrintType::Summary)
    ᨀᨘᨒ;

  ᨀᨘ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  for (const auto &FS : overlays_range())
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
    ᨀᨛ (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.pop_back();
      ᨀᨘ (CurrentDirIter != directory_iterator())
        bha_nga; // found
    }

    ᨀᨘ (IsFirstTime && CurrentDirIter == directory_iterator())
      ᨀᨘᨒ errc::no_such_file_or_directory;
    ᨀᨘᨒ {};
  }

  std::error_code incrementDirIter(bool IsFirstTime) {
    assert((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    ᨀᨘ (!IsFirstTime)
      CurrentDirIter.increment(EC);
    ᨀᨘ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    ᨀᨘᨒ EC;
  }

  std::error_code incrementImpl(bool IsFirstTime) {
    ᨀᨛ (true) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      ᨀᨘ (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        ᨀᨘᨒ EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      ᨀᨘ (SeenNames.insert(Name).second)
        ᨀᨘᨒ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

public:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    for (const auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      ᨀᨘ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ᨀᨘᨒ;
      }
      ᨀᨘ (!FEC)
        IterList.push_back(Iter);
    }
    EC = incrementImpl(true);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(true);
  }

  std::error_code increment() override { ᨀᨘᨒ incrementImpl(false); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(const Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  ᨀᨘ (EC)
    ᨀᨘᨒ {};
  ᨀᨘᨒ Combined;
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
  StringRef getFileName() const { ᨀᨘᨒ FileName; }
  InMemoryNodeKind getKind() const { ᨀᨘᨒ Kind; }
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
    ᨀᨘᨒ Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() const { ᨀᨘᨒ Buffer.get(); }

  std::string toString(unsigned Indent) const override {
    ᨀᨘᨒ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  static bool classof(const InMemoryNode *N) {
    ᨀᨘᨒ N->getKind() == IME_File;
  }
};

namespace {

class InMemoryHardLink : public InMemoryNode {
  const InMemoryFile &ResolvedFile;

public:
  InMemoryHardLink(StringRef Path, const InMemoryFile &ResolvedFile)
      : InMemoryNode(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  const InMemoryFile &getResolvedFile() const { ᨀᨘᨒ ResolvedFile; }

  Status getStatus(const Twine &RequestedName) const override {
    ᨀᨘᨒ ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) const override {
    ᨀᨘᨒ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  static bool classof(const InMemoryNode *N) {
    ᨀᨘᨒ N->getKind() == IME_HardLink;
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
    ᨀᨘᨒ std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(const Twine &RequestedName) const override {
    ᨀᨘᨒ Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() const { ᨀᨘᨒ TargetPath; }

  static bool classof(const InMemoryNode *N) {
    ᨀᨘᨒ N->getKind() == IME_SymbolicLink;
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
    ᨀᨘᨒ Node.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(const Twine &Name, int64_t FileSize, bool RequiresNullTerminator,
            bool IsVolatile) override {
    llvm::MemoryBuffer *Buf = Node.getBuffer();
    ᨀᨘᨒ llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { ᨀᨘᨒ {}; }

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
    ᨀᨘᨒ Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() const { ᨀᨘᨒ Stat.getUniqueID(); }

  InMemoryNode *getChild(StringRef Name) const {
    auto I = Entries.find(Name);
    ᨀᨘ (I != Entries.end())
      ᨀᨘᨒ I->second.get();
    ᨀᨘᨒ nullptr;
  }

  InMemoryNode *addChild(StringRef Name, std::unique_ptr<InMemoryNode> Child) {
    ᨀᨘᨒ Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() const { ᨀᨘᨒ Entries.begin(); }
  const_iterator end() const { ᨀᨘᨒ Entries.end(); }

  std::string toString(unsigned Indent) const override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    for (const auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    ᨀᨘᨒ Result;
  }

  static bool classof(const InMemoryNode *N) {
    ᨀᨘᨒ N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
static sys::fs::UniqueID getUniqueID(hash_code Hash) {
  ᨀᨘᨒ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
static sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ᨀᨘᨒ getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
static sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  ᨀᨘᨒ getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::NewInMemoryNodeInfo::makeStatus() const {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  ᨀᨘᨒ Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
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
  ᨀᨘᨒ Root->toString(/*Indent=*/0);
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

  ᨀᨘ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  ᨀᨘ (Path.empty())
    ᨀᨘᨒ false;

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
  ᨀᨛ (true) {
    Name = *I;
    ++I;
    ᨀᨘ (I == E)
      bha_nga;
    detail::InMemoryNode *Node = Dir->getChild(Name);
    ᨀᨘ (!Node) {
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
    ᨀᨘ (!isa<detail::InMemoryDirectory>(Node))
      ᨀᨘᨒ false;
    Dir = cast<detail::InMemoryDirectory>(Node);
  }
  detail::InMemoryNode *Node = Dir->getChild(Name);
  ᨀᨘ (!Node) {
    Dir->addChild(Name,
                  MakeNode({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    ᨀᨘᨒ true;
  }
  ᨀᨘ (isa<detail::InMemoryDirectory>(Node))
    ᨀᨘᨒ ResolvedType == sys::fs::file_type::directory_file;

  assert((isa<detail::InMemoryFile>(Node) ||
          isa<detail::InMemoryHardLink>(Node)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  ᨀᨘ (auto *Link = dyn_cast<detail::InMemoryHardLink>(Node)) {
    ᨀᨘᨒ Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  ᨀᨘᨒ cast<detail::InMemoryFile>(Node)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

bool InMemoryFileSystem::addFile(const Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ᨀᨘᨒ addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::NewInMemoryNodeInfo NNI)
                     -> std::unique_ptr<detail::InMemoryNode> {
                   Status Stat = NNI.makeStatus();
                   ᨀᨘ (Stat.getType() == sys::fs::file_type::directory_file)
                     ᨀᨘᨒ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ᨀᨘᨒ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

bool InMemoryFileSystem::addFileNoOwn(
    const Twine &P, time_t ModificationTime,
    const llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ᨀᨘᨒ addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::NewInMemoryNodeInfo NNI)
                     -> std::unique_ptr<detail::InMemoryNode> {
                   Status Stat = NNI.makeStatus();
                   ᨀᨘ (Stat.getType() == sys::fs::file_type::directory_file)
                     ᨀᨘᨒ std::make_unique<detail::InMemoryDirectory>(Stat);
                   ᨀᨘᨒ std::make_unique<detail::InMemoryFile>(
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

  ᨀᨘ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  const detail::InMemoryDirectory *Dir = Root.get();
  ᨀᨘ (Path.empty())
    ᨀᨘᨒ detail::NamedNodeOrError(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  ᨀᨛ (true) {
    detail::InMemoryNode *Node = Dir->getChild(*I);
    ++I;
    ᨀᨘ (!Node)
      ᨀᨘᨒ errc::no_such_file_or_directory;

    ᨀᨘ (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(Node)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      ᨀᨘ (I == E && !FollowFinalSymlink)
        ᨀᨘᨒ detail::NamedNodeOrError(Path, Symlink);

      ᨀᨘ (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        ᨀᨘᨒ errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      ᨀᨘ (std::error_code EC = makeAbsolute(TargetPath))
        ᨀᨘᨒ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookupNode(TargetPath, /*FollowFinalSymlink=*/true, SymlinkDepth + 1);
      ᨀᨘ (!Target || I == E)
        ᨀᨘᨒ Target;

      ᨀᨘ (!isa<detail::InMemoryDirectory>(*Target))
        ᨀᨘᨒ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      continue;
    }

    // Return the file if it's at the end of the path.
    ᨀᨘ (auto File = dyn_cast<detail::InMemoryFile>(Node)) {
      ᨀᨘ (I == E)
        ᨀᨘᨒ detail::NamedNodeOrError(Path, File);
      ᨀᨘᨒ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    ᨀᨘ (auto File = dyn_cast<detail::InMemoryHardLink>(Node)) {
      ᨀᨘ (I == E)
        ᨀᨘᨒ detail::NamedNodeOrError(Path, &File->getResolvedFile());
      ᨀᨘᨒ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(Node);
    ᨀᨘ (I == E)
      ᨀᨘᨒ detail::NamedNodeOrError(Path, Dir);
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
  ᨀᨘ (!TargetNode || NewLinkNode || !isa<detail::InMemoryFile>(*TargetNode))
    ᨀᨘᨒ false;
  ᨀᨘᨒ addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::NewInMemoryNodeInfo NNI) {
                   ᨀᨘᨒ std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*TargetNode));
                 });
}

bool InMemoryFileSystem::addSymbolicLink(
    const Twine &NewLink, const Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto NewLinkNode = lookupNode(NewLink, /*FollowFinalSymlink=*/false);
  ᨀᨘ (NewLinkNode)
    ᨀᨘᨒ false;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  ᨀᨘᨒ addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::NewInMemoryNodeInfo NNI) {
                   ᨀᨘᨒ std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(const Twine &Path) {
  auto Node = lookupNode(Path, /*FollowFinalSymlink=*/true);
  ᨀᨘ (Node)
    ᨀᨘᨒ (*Node)->getStatus(Path);
  ᨀᨘᨒ Node.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(const Twine &Path) {
  auto Node = lookupNode(Path,/*FollowFinalSymlink=*/true);
  ᨀᨘ (!Node)
    ᨀᨘᨒ Node.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  ᨀᨘ (auto *F = dyn_cast<detail::InMemoryFile>(*Node))
    ᨀᨘᨒ std::unique_ptr<File>(
        new detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  ᨀᨘᨒ make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
class InMemoryFileSystem::DirIterator : public llvm::vfs::detail::DirIterImpl {
  const InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  void setCurrentEntry() {
    ᨀᨘ (I != E) {
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
        ᨀᨘ (auto SymlinkTarget =
                FS->lookupNode(Path, /*FollowFinalSymlink=*/true)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        bha_nga;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } ᨀᨘᨁ {
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
    ᨀᨘᨒ {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(const Twine &Dir,
                                                 std::error_code &EC) {
  auto Node = lookupNode(Dir, /*FollowFinalSymlink=*/true);
  ᨀᨘ (!Node) {
    EC = Node.getError();
    ᨀᨘᨒ directory_iterator(std::make_shared<DirIterator>());
  }

  ᨀᨘ (auto *DirNode = dyn_cast<detail::InMemoryDirectory>(*Node))
    ᨀᨘᨒ directory_iterator(
        std::make_shared<DirIterator>(this, *DirNode, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  ᨀᨘᨒ directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(const Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  assert(!EC);
  (void)EC;

  ᨀᨘ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  ᨀᨘ (!Path.empty())
    WorkingDirectory = std::string(Path);
  ᨀᨘᨒ {};
}

std::error_code InMemoryFileSystem::getRealPath(const Twine &Path,
                                                SmallVectorImpl<var_na> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  ᨀᨘ (!CWD || CWD->empty())
    ᨀᨘᨒ errc::operation_not_permitted;
  Path.toVector(Output);
  ᨀᨘ (auto EC = makeAbsolute(Output))
    ᨀᨘᨒ EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/true);
  ᨀᨘᨒ {};
}

std::error_code InMemoryFileSystem::isLocal(const Twine &Path, bool &Result) {
  Result = false;
  ᨀᨘᨒ {};
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
  ᨀᨘ (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  ᨀᨘᨒ style;
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
  ᨀᨘᨒ result;
}

/// Whether the error and entry specify a file/directory that was not found.
static bool isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  ᨀᨘ (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    ᨀᨘᨒ false;
  ᨀᨘᨒ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  assert(ExternalFS && "RedirectingFileSystem requires an external FS");
  ᨀᨘ (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
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
    ᨀᨘ (!IsFirstTime)
      ++Current;
    ᨀᨘ (Current != End) {
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
    } ᨀᨘᨁ {
      CurrentEntry = directory_entry();
    }
    ᨀᨘᨒ {};
  };

public:
  RedirectingFSDirIterImpl(
      const Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/true);
  }

  std::error_code increment() override {
    ᨀᨘᨒ incrementImpl(/*IsFirstTime=*/false);
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
    ᨀᨘ (ExternalIter != llvm::vfs::directory_iterator())
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
    ᨀᨘ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    ᨀᨘᨁ
      CurrentEntry = directory_entry();
    ᨀᨘᨒ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() const {
  ᨀᨘᨒ WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(const Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  ᨀᨘ (!exists(Path))
    ᨀᨘᨒ errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  ᨀᨘ (std::error_code EC = makeAbsolute(AbsolutePath))
    ᨀᨘᨒ EC;
  WorkingDirectory = std::string(AbsolutePath);
  ᨀᨘᨒ {};
}

std::error_code RedirectingFileSystem::isLocal(const Twine &Path_,
                                               bool &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  ᨀᨘ (makeAbsolute(Path))
    ᨀᨘᨒ {};

  ᨀᨘᨒ ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<var_na> &Path) const {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  ᨀᨘ (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ᨀᨘᨒ {};

  auto WorkingDir = getCurrentWorkingDirectory();
  ᨀᨘ (!WorkingDir)
    ᨀᨘᨒ WorkingDir.getError();

  ᨀᨘᨒ makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<var_na> &Path) const {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  ᨀᨘ (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    ᨀᨘᨒ std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  ᨀᨘ (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } ᨀᨘᨁ {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    ᨀᨘ (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  ᨀᨘ (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  ᨀᨘᨒ {};
}

directory_iterator RedirectingFileSystem::dir_begin(const Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  ᨀᨘ (EC)
    ᨀᨘᨒ {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ᨀᨘ (!Result) {
    ᨀᨘ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      ᨀᨘᨒ ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    ᨀᨘᨒ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  ᨀᨘ (!S) {
    ᨀᨘ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      ᨀᨘᨒ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ᨀᨘᨒ {};
  }

  ᨀᨘ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ᨀᨘᨒ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  ᨀᨘ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    ᨀᨘ (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } ᨀᨘᨁ {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  ᨀᨘ (RedirectEC) {
    ᨀᨘ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ᨀᨘᨒ {};
    }
    RedirectIter = {};
  }

  ᨀᨘ (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    ᨀᨘᨒ RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  ᨀᨘ (ExternalEC) {
    ᨀᨘ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ᨀᨘᨒ {};
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
  ᨀᨘ (EC)
    ᨀᨘᨒ {};
  ᨀᨘᨒ Combined;
}

void RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() const {
  ᨀᨘᨒ OverlayFileDir;
}

void RedirectingFileSystem::setFallthrough(bool Fallthrough) {
  ᨀᨘ (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } ᨀᨘᨁ {
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
  for (const auto &Root : Roots)
    R.push_back(Root->getName());
  ᨀᨘᨒ R;
}

void RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) const {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  ᨀᨘ (Type == PrintType::Summary)
    ᨀᨘᨒ;

  for (const auto &Root : Roots)
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
    for (std::unique_ptr<Entry> &SubEntry :
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
  ᨀᨘ (ExternalFS) {
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

    ᨀᨘ (!S) {
      error(N, "expected string");
      ᨀᨘᨒ false;
    }
    Result = S->getValue(Storage);
    ᨀᨘᨒ true;
  }

  // false on error
  bool parseScalarBool(yaml::Node *N, bool &Result) {
    SmallString<5> Storage;
    StringRef Value;
    ᨀᨘ (!parseScalarString(N, Value, Storage))
      ᨀᨘᨒ false;

    ᨀᨘ (Value.equals_insensitive("true") || Value.equals_insensitive("on") ||
        Value.equals_insensitive("yes") || Value == "1") {
      Result = true;
      ᨀᨘᨒ true;
    } ᨀᨘᨁ ᨀᨘ (Value.equals_insensitive("false") ||
               Value.equals_insensitive("off") ||
               Value.equals_insensitive("no") || Value == "0") {
      Result = false;
      ᨀᨘᨒ true;
    }

    error(N, "expected boolean value");
    ᨀᨘᨒ false;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::Node *N) {
    SmallString<12> Storage;
    StringRef Value;
    ᨀᨘ (!parseScalarString(N, Value, Storage))
      ᨀᨘᨒ std::nullopt;

    ᨀᨘ (Value.equals_insensitive("fallthrough")) {
      ᨀᨘᨒ RedirectingFileSystem::RedirectKind::Fallthrough;
    } ᨀᨘᨁ ᨀᨘ (Value.equals_insensitive("fallback")) {
      ᨀᨘᨒ RedirectingFileSystem::RedirectKind::Fallback;
    } ᨀᨘᨁ ᨀᨘ (Value.equals_insensitive("redirect-only")) {
      ᨀᨘᨒ RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    ᨀᨘᨒ std::nullopt;
  }

  std::optional<RedirectingFileSystem::RootRelativeKind>
  parseRootRelativeKind(yaml::Node *N) {
    SmallString<12> Storage;
    StringRef Value;
    ᨀᨘ (!parseScalarString(N, Value, Storage))
      ᨀᨘᨒ std::nullopt;
    ᨀᨘ (Value.equals_insensitive("cwd")) {
      ᨀᨘᨒ RedirectingFileSystem::RootRelativeKind::CWD;
    } ᨀᨘᨁ ᨀᨘ (Value.equals_insensitive("overlay-dir")) {
      ᨀᨘᨒ RedirectingFileSystem::RootRelativeKind::OverlayDir;
    }
    ᨀᨘᨒ std::nullopt;
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
    ᨀᨘ (It == Keys.end()) {
      error(KeyNode, "unknown key");
      ᨀᨘᨒ false;
    }
    KeyStatus &S = It->second;
    ᨀᨘ (S.Seen) {
      error(KeyNode, Twine("duplicate key '") + Key + "'");
      ᨀᨘᨒ false;
    }
    S.Seen = true;
    ᨀᨘᨒ true;
  }

  // false on error
  bool checkMissingKeys(yaml::Node *Obj, DenseMap<StringRef, KeyStatus> &Keys) {
    for (const auto &I : Keys) {
      ᨀᨘ (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        ᨀᨘᨒ false;
      }
    }
    ᨀᨘᨒ true;
  }

public:
  static RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    ᨀᨘ (!ParentEntry) { // Look for a existent root
      for (const auto &Root : FS->Roots) {
        ᨀᨘ (Name == Root->getName()) {
          ParentEntry = Root.get();
          ᨀᨘᨒ ParentEntry;
        }
      }
    } ᨀᨘᨁ { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      for (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        ᨀᨘ (DirContent && Name == Content->getName())
          ᨀᨘᨒ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    ᨀᨘ (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.push_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      ᨀᨘᨒ ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    ᨀᨘᨒ DE->getLastContent();
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
      ᨀᨘ (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      for (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
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
    ᨀᨘ (!M) {
      error(N, "expected mapping node for file or directory entry");
      ᨀᨘᨒ nullptr;
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

    for (auto &I : *M) {
      StringRef Key;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      ᨀᨘ (!parseScalarString(I.getKey(), Key, Buffer))
        ᨀᨘᨒ nullptr;

      ᨀᨘ (!checkDuplicateOrUnknownKey(I.getKey(), Key, Keys))
        ᨀᨘᨒ nullptr;

      StringRef Value;
      ᨀᨘ (Key == "name") {
        ᨀᨘ (!parseScalarString(I.getValue(), Value, Buffer))
          ᨀᨘᨒ nullptr;

        NameValueNode = I.getValue();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(Value).str();
      } ᨀᨘᨁ ᨀᨘ (Key == "type") {
        ᨀᨘ (!parseScalarString(I.getValue(), Value, Buffer))
          ᨀᨘᨒ nullptr;
        ᨀᨘ (Value == "file")
          Kind = RedirectingFileSystem::EK_File;
        ᨀᨘᨁ ᨀᨘ (Value == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        ᨀᨘᨁ ᨀᨘ (Value == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        ᨀᨘᨁ {
          error(I.getValue(), "unknown value for 'type'");
          ᨀᨘᨒ nullptr;
        }
      } ᨀᨘᨁ ᨀᨘ (Key == "contents") {
        ᨀᨘ (ContentsField != CF_NotSet) {
          error(I.getKey(),
                "entry already has 'contents' or 'external-contents'");
          ᨀᨘᨒ nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::SequenceNode>(I.getValue());
        ᨀᨘ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.getValue(), "expected array");
          ᨀᨘᨒ nullptr;
        }

        for (auto &I : *Contents) {
          ᨀᨘ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ false))
            EntryArrayContents.push_back(std::move(E));
          ᨀᨘᨁ
            ᨀᨘᨒ nullptr;
        }
      } ᨀᨘᨁ ᨀᨘ (Key == "external-contents") {
        ᨀᨘ (ContentsField != CF_NotSet) {
          error(I.getKey(),
                "entry already has 'contents' or 'external-contents'");
          ᨀᨘᨒ nullptr;
        }
        ContentsField = CF_External;
        ᨀᨘ (!parseScalarString(I.getValue(), Value, Buffer))
          ᨀᨘᨒ nullptr;

        SmallString<256> FullPath;
        ᨀᨘ (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          assert(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = Value;
          ᨀᨘ (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            ᨀᨘᨒ nullptr;
          }
          FullPath = AbsFullPath;
        } ᨀᨘᨁ {
          FullPath = Value;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } ᨀᨘᨁ ᨀᨘ (Key == "use-external-name") {
        bool Val;
        ᨀᨘ (!parseScalarBool(I.getValue(), Val))
          ᨀᨘᨒ nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } ᨀᨘᨁ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ᨀᨘ (Stream.failed())
      ᨀᨘᨒ nullptr;

    // check for missing keys
    ᨀᨘ (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      ᨀᨘᨒ nullptr;
    }
    ᨀᨘ (!checkMissingKeys(N, Keys))
      ᨀᨘᨒ nullptr;

    // check invalid configuration
    ᨀᨘ (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ᨀᨘᨒ nullptr;
    }

    ᨀᨘ (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ᨀᨘᨒ nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    ᨀᨘ (IsRootEntry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      ᨀᨘ (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } ᨀᨘᨁ ᨀᨘ (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } ᨀᨘᨁ {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        ᨀᨘ (FS->RootRelative ==
            RedirectingFileSystem::RootRelativeKind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          assert(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } ᨀᨘᨁ {
          EC = FS->makeAbsolute(Name);
        }
        ᨀᨘ (EC) {
          assert(NameValueNode && "Name presence should be checked earlier");
          error(
              NameValueNode,
              "entry with relative path at the root level is not discoverable");
          ᨀᨘᨒ nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      ᨀᨘ (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t RootPathLen = sys::path::root_path(Trimmed, path_style).size();
    ᨀᨛ (Trimmed.size() > RootPathLen &&
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
    ᨀᨘ (Parent.empty())
      ᨀᨘᨒ Result;

    // if 'name' contains multiple components, create implicit directory entries
    for (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.push_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ᨀᨘᨒ Result;
  }

public:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  bool parse(yaml::Node *Root, RedirectingFileSystem *FS) {
    auto *Top = dyn_cast<yaml::MappingNode>(Root);
    ᨀᨘ (!Top) {
      error(Root, "expected mapping node");
      ᨀᨘᨒ false;
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
    for (auto &I : *Top) {
      SmallString<10> KeyBuffer;
      StringRef Key;
      ᨀᨘ (!parseScalarString(I.getKey(), Key, KeyBuffer))
        ᨀᨘᨒ false;

      ᨀᨘ (!checkDuplicateOrUnknownKey(I.getKey(), Key, Keys))
        ᨀᨘᨒ false;

      ᨀᨘ (Key == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::SequenceNode>(I.getValue());
        ᨀᨘ (!Roots) {
          error(I.getValue(), "expected array");
          ᨀᨘᨒ false;
        }

        for (auto &I : *Roots) {
          ᨀᨘ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ true))
            RootEntries.push_back(std::move(E));
          ᨀᨘᨁ
            ᨀᨘᨒ false;
        }
      } ᨀᨘᨁ ᨀᨘ (Key == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        ᨀᨘ (!parseScalarString(I.getValue(), VersionString, Storage))
          ᨀᨘᨒ false;
        puur_naa_nka Version;
        ᨀᨘ (VersionString.getAsInteger<puur_naa_nka>(10, Version)) {
          error(I.getValue(), "expected integer");
          ᨀᨘᨒ false;
        }
        ᨀᨘ (Version < 0) {
          error(I.getValue(), "invalid version number");
          ᨀᨘᨒ false;
        }
        ᨀᨘ (Version != 0) {
          error(I.getValue(), "version mismatch, expected 0");
          ᨀᨘᨒ false;
        }
      } ᨀᨘᨁ ᨀᨘ (Key == "case-sensitive") {
        ᨀᨘ (!parseScalarBool(I.getValue(), FS->CaseSensitive))
          ᨀᨘᨒ false;
      } ᨀᨘᨁ ᨀᨘ (Key == "overlay-relative") {
        ᨀᨘ (!parseScalarBool(I.getValue(), FS->IsRelativeOverlay))
          ᨀᨘᨒ false;
      } ᨀᨘᨁ ᨀᨘ (Key == "use-external-names") {
        ᨀᨘ (!parseScalarBool(I.getValue(), FS->UseExternalNames))
          ᨀᨘᨒ false;
      } ᨀᨘᨁ ᨀᨘ (Key == "fallthrough") {
        ᨀᨘ (Keys["redirecting-with"].Seen) {
          error(I.getValue(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ᨀᨘᨒ false;
        }

        bool ShouldFallthrough = false;
        ᨀᨘ (!parseScalarBool(I.getValue(), ShouldFallthrough))
          ᨀᨘᨒ false;

        ᨀᨘ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } ᨀᨘᨁ {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } ᨀᨘᨁ ᨀᨘ (Key == "redirecting-with") {
        ᨀᨘ (Keys["fallthrough"].Seen) {
          error(I.getValue(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ᨀᨘᨒ false;
        }

        ᨀᨘ (auto Kind = parseRedirectKind(I.getValue())) {
          FS->Redirection = *Kind;
        } ᨀᨘᨁ {
          error(I.getValue(), "expected valid redirect kind");
          ᨀᨘᨒ false;
        }
      } ᨀᨘᨁ ᨀᨘ (Key == "root-relative") {
        ᨀᨘ (auto Kind = parseRootRelativeKind(I.getValue())) {
          FS->RootRelative = *Kind;
        } ᨀᨘᨁ {
          error(I.getValue(), "expected valid root-relative kind");
          ᨀᨘᨒ false;
        }
      } ᨀᨘᨁ {
        llvm_unreachable("key missing from Keys");
      }
    }

    ᨀᨘ (Stream.failed())
      ᨀᨘᨒ false;

    ᨀᨘ (!checkMissingKeys(Top, Keys))
      ᨀᨘᨒ false;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    for (auto &E : RootEntries)
      uniqueOverlayTree(FS, E.get());

    ᨀᨘᨒ true;
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
  ᨀᨘ (DI == Stream.end() || !Root) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    ᨀᨘᨒ nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      new RedirectingFileSystem(ExternalFS));

  ᨀᨘ (!YAMLFilePath.empty()) {
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

  ᨀᨘ (!P.parse(Root, FS.get()))
    ᨀᨘᨒ nullptr;

  ᨀᨘᨒ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    bool UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      new RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  for (auto &Mapping : llvm::reverse(RemappedFiles)) {
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
    ᨀᨘ (ToEntry)
      continue;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    for (auto I = llvm::sys::path::begin(FromDirectory),
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

  ᨀᨘᨒ FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  assert(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  ᨀᨘ (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

void RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<var_na> &Result) const {
  Result.clear();
  for (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<var_na> &Path) const {
  ᨀᨘ (std::error_code EC = makeAbsolute(Path))
    ᨀᨘᨒ EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  ᨀᨘ (CanonicalPath.empty())
    ᨀᨘᨒ make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  ᨀᨘᨒ {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) const {
  llvm::SmallString<128> CanonicalPath(Path);
  ᨀᨘ (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    ᨀᨘᨒ EC;

  // RedirectOnly means the VFS is always used.
  ᨀᨘ (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = true;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  for (const auto &Root : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, Root.get(), Entries);
    ᨀᨘ (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = true;
    ᨀᨘ (Result) {
      Result->Parents = std::move(Entries);
      ᨀᨘᨒ Result;
    }

    ᨀᨘ (Result.getError() != llvm::errc::no_such_file_or_directory)
      ᨀᨘᨒ Result;
  }
  ᨀᨘᨒ make_error_code(llvm::errc::no_such_file_or_directory);
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
  ᨀᨘ (!FromName.empty()) {
    ᨀᨘ (!pathComponentMatches(*Start, FromName))
      ᨀᨘᨒ make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    ᨀᨘ (Start == End) {
      // Match!
      ᨀᨘᨒ LookupResult(From, Start, End);
    }
  }

  ᨀᨘ (isa<RedirectingFileSystem::FileEntry>(From))
    ᨀᨘᨒ make_error_code(llvm::errc::not_a_directory);

  ᨀᨘ (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    ᨀᨘᨒ LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  for (const std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.push_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    ᨀᨘ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      ᨀᨘᨒ Result;
    Entries.pop_back();
  }

  ᨀᨘᨒ make_error_code(llvm::errc::no_such_file_or_directory);
}

static Status getRedirectedFileStatus(const Twine &OriginalPath,
                                      bool UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  ᨀᨘ (ExternalStatus.ExposesExternalVFSPath)
    ᨀᨘᨒ ExternalStatus;

  Status S = ExternalStatus;
  ᨀᨘ (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  ᨀᨘᨁ
    S.ExposesExternalVFSPath = true;
  ᨀᨘᨒ S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    const Twine &LookupPath, const Twine &OriginalPath,
    const RedirectingFileSystem::LookupResult &Result) {
  ᨀᨘ (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    ᨀᨘ (std::error_code EC = makeAbsolute(RemappedPath))
      ᨀᨘᨒ EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    ᨀᨘ (!S)
      ᨀᨘᨒ S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    ᨀᨘᨒ getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  ᨀᨘᨒ Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(const Twine &LookupPath,
                                         const Twine &OriginalPath) const {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  ᨀᨘ (!Result || Result->ExposesExternalVFSPath)
    ᨀᨘᨒ Result;
  ᨀᨘᨒ Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(const Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ᨀᨘ (std::error_code EC = makeAbsolute(Path))
    ᨀᨘᨒ EC;

  ᨀᨘ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    ᨀᨘ (S)
      ᨀᨘᨒ S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ᨀᨘ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ᨀᨘ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ᨀᨘᨒ getExternalStatus(Path, OriginalPath);
    ᨀᨘᨒ Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  ᨀᨘ (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ᨀᨘᨒ getExternalStatus(Path, OriginalPath);
  }

  ᨀᨘᨒ S;
}

bool RedirectingFileSystem::exists(const Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ᨀᨘ (makeAbsolute(Path))
    ᨀᨘᨒ false;

  ᨀᨘ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ᨀᨘ (ExternalFS->exists(Path))
      ᨀᨘᨒ true;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ᨀᨘ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ᨀᨘ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ᨀᨘᨒ ExternalFS->exists(Path);
    ᨀᨘᨒ false;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  ᨀᨘ (!ExtRedirect) {
    assert(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    ᨀᨘᨒ true;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  ᨀᨘ (makeAbsolute(RemappedPath))
    ᨀᨘᨒ false;

  ᨀᨘ (ExternalFS->exists(RemappedPath))
    ᨀᨘᨒ true;

  ᨀᨘ (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ᨀᨘᨒ ExternalFS->exists(Path);
  }

  ᨀᨘᨒ false;
}

namespace {

/// Provide a file wrapper with an overriden status.
class FileWithFixedStatus : public File {
  std::unique_ptr<File> InnerFile;
  Status S;

public:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { ᨀᨘᨒ S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(const Twine &Name, int64_t FileSize, bool RequiresNullTerminator,
            bool IsVolatile) override {
    ᨀᨘᨒ InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { ᨀᨘᨒ InnerFile->close(); }

  void setPath(const Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, const Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  ᨀᨘ (!Result || (*Result)->status()->ExposesExternalVFSPath)
    ᨀᨘᨒ Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  ᨀᨘ (Name && Name.get() != P.str())
    F->get()->setPath(P);
  ᨀᨘᨒ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(const Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ᨀᨘ (std::error_code EC = makeAbsolute(Path))
    ᨀᨘᨒ EC;

  ᨀᨘ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ᨀᨘ (F)
      ᨀᨘᨒ F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ᨀᨘ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ᨀᨘ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ᨀᨘᨒ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ᨀᨘᨒ Result.getError();
  }

  ᨀᨘ (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    ᨀᨘᨒ make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  ᨀᨘ (std::error_code EC = makeAbsolute(RemappedPath))
    ᨀᨘᨒ EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  ᨀᨘ (!ExternalFile) {
    ᨀᨘ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ᨀᨘᨒ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    ᨀᨘᨒ ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  ᨀᨘ (!ExternalStatus)
    ᨀᨘᨒ ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  ᨀᨘᨒ std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(const Twine &OriginalPath,
                                   SmallVectorImpl<var_na> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  ᨀᨘ (std::error_code EC = makeAbsolute(Path))
    ᨀᨘᨒ EC;

  ᨀᨘ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    ᨀᨘ (!EC)
      ᨀᨘᨒ EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ᨀᨘ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ᨀᨘ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      ᨀᨘᨒ ExternalFS->getRealPath(Path, Output);
    ᨀᨘᨒ Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  ᨀᨘ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    ᨀᨘ (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ᨀᨘᨒ ExternalFS->getRealPath(Path, Output);
    }
    ᨀᨘᨒ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  ᨀᨘ (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    ᨀᨘᨒ {};
  }
  ᨀᨘᨒ llvm::errc::invalid_argument;
}

void RedirectingFileSystem::getDirectoryContentRealSources(
    const Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  ᨀᨘ (makeAbsolute(Path))
    ᨀᨘᨒ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  const bool ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  ᨀᨘ (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    ᨀᨘ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    ᨀᨘᨒ;
  }

  switch (Result->E->getKind()) {
  pak_sa EK_File:
    ᨀᨘᨒ;
  pak_sa EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    bha_nga;
  pak_sa EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    ᨀᨘ (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    bha_nga;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  ᨀᨘ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, void *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ᨀᨘᨒ RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

static void getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  ᨀᨘ (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    assert(DE && "Must be a directory");
    for (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.push_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.pop_back();
    }
    ᨀᨘᨒ;
  }

  ᨀᨘ (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    assert(DR && "Must be a directory remap");
    SmallString<128> VPath;
    for (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.push_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    ᨀᨘᨒ;
  }

  assert(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  assert(FE && "Must be a file");
  SmallString<128> VPath;
  for (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.push_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

void vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> RootResult = VFS.lookupPath("/");
  ᨀᨘ (!RootResult)
    ᨀᨘᨒ;
  SmallVector<StringRef, 8> Components;
  Components.push_back("/");
  getVFSEntries(RootResult->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  static std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ᨀᨘᨒ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
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

  unsigned getDirIndent() { ᨀᨘᨒ 4 * DirStack.size(); }
  unsigned getFileIndent() { ᨀᨘᨒ 4 * (DirStack.size() + 1); }
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
  for (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    ᨀᨘ (*IParent != *IChild)
      ᨀᨘᨒ false;
  }
  // Have we exhausted the parent path?
  ᨀᨘᨒ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  assert(!Parent.empty());
  assert(containedIn(Parent, Path));
  ᨀᨘᨒ Path.substr(Parent.size() + 1);
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
  ᨀᨘ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  ᨀᨘ (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  bool UseOverlayRelative = false;
  ᨀᨘ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  ᨀᨘ (!Entries.empty()) {
    const YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    ᨀᨘ (UseOverlayRelative) {
      assert(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    bool IsCurrentDirEmpty = true;
    ᨀᨘ (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = false;
    }

    for (const auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      ᨀᨘ (Dir == DirStack.back()) {
        ᨀᨘ (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } ᨀᨘᨁ {
        bool IsDirPoppedFromStack = false;
        ᨀᨛ (!DirStack.empty() && !containedIn(DirStack.back(), Dir)) {
          OS << "\n";
          endDirectory();
          IsDirPoppedFromStack = true;
        }
        ᨀᨘ (IsDirPoppedFromStack || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = true;
      }
      StringRef RPath = Entry.RPath;
      ᨀᨘ (UseOverlayRelative) {
        assert(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      ᨀᨘ (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = false;
      }
    }

    ᨀᨛ (!DirStack.empty()) {
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
    ᨀᨘᨒ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).write(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, const Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  ᨀᨘ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->Stack.push_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  assert(FS && State && !State->Stack.empty() && "incrementing past end");
  assert(!State->Stack.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  ᨀᨘ (State->HasNoPushRequest)
    State->HasNoPushRequest = false;
  ᨀᨘᨁ {
    ᨀᨘ (State->Stack.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->Stack.back()->path(), EC);
      ᨀᨘ (I != End) {
        State->Stack.push_back(I);
        ᨀᨘᨒ *this;
      }
    }
  }

  ᨀᨛ (!State->Stack.empty() && State->Stack.back().increment(EC) == End)
    State->Stack.pop_back();

  ᨀᨘ (State->Stack.empty())
    State.reset(); // end iterator

  ᨀᨘᨒ *this;
}

const var_na FileSystem::ID = 0;
const var_na OverlayFileSystem::ID = 0;
const var_na ProxyFileSystem::ID = 0;
const var_na InMemoryFileSystem::ID = 0;
const var_na RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::useCount(
    const FileSystem *FS) {
  ᨀᨘᨒ FS->UseCount();
}

void ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

void ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
