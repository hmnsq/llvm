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

#অন্তর্ভাব "llvm/Support/VirtualFileSystem.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/ADT/DenseMap.h"
#অন্তর্ভাব "llvm/ADT/IntrusiveRefCntPtr.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/StringSet.h"
#অন্তর্ভাব "llvm/ADT/Twine.h"
#অন্তর্ভাব "llvm/ADT/iterator_range.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"
#অন্তর্ভাব "llvm/Support/Casting.h"
#অন্তর্ভাব "llvm/Support/Chrono.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/ErrorOr.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/FileSystem/UniqueID.h"
#অন্তর্ভাব "llvm/Support/IOSandbox.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/Path.h"
#অন্তর্ভাব "llvm/Support/SMLoc.h"
#অন্তর্ভাব "llvm/Support/SourceMgr.h"
#অন্তর্ভাব "llvm/Support/YAMLParser.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <atomic>
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstdint>
#অন্তর্ভাব <iterator>
#অন্তর্ভাব <limits>
#অন্তর্ভাব <map>
#অন্তর্ভাব <memory>
#অন্তর্ভাব <optional>
#অন্তর্ভাব <string>
#অন্তর্ভাব <system_error>
#অন্তর্ভাব <utility>
#অন্তর্ভাব <vector>

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(ধ্রুবক file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), আকার_2(Status.get_আকার()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(ধ্রুবক Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t আকার_2, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      আকার_2(আকার_2), Type(Type), Perms(Perms) {}

Status Status::copy_With_New_আকার(ধ্রুবক Status &In, uint64_t New_আকার) {
  ফেরত Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), New_আকার, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ধ্রুবক Status &In, ধ্রুবক Twine &NewName) {
  ফেরত Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.get_আকার(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ধ্রুবক file_status &In, ধ্রুবক Twine &NewName) {
  ফেরত Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.get_আকার(), In.type(),
                In.permissions());
}

যৌক্তিক Status::equivalent(ধ্রুবক Status &Other) ধ্রুবক {
  প্রতিজ্ঞা(isStatusKnown() && Other.isStatusKnown());
  ফেরত getUniqueID() == Other.getUniqueID();
}

যৌক্তিক Status::isDirectory() ধ্রুবক { ফেরত Type == file_type::directory_file; }

যৌক্তিক Status::isRegularFile() ধ্রুবক { ফেরত Type == file_type::regular_file; }

যৌক্তিক Status::isOther() ধ্রুবক {
  ফেরত exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

যৌক্তিক Status::isSymlink() ধ্রুবক { ফেরত Type == file_type::symlink_file; }

যৌক্তিক Status::isStatusKnown() ধ্রুবক { ফেরত Type != file_type::status_error; }

যৌক্তিক Status::exists() ধ্রুবক {
  ফেরত isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = পূর্বনির্ধারিত;

FileSystem::~FileSystem() = পূর্বনির্ধারিত;

ErrorOr<std::unique_ptr<Memory_বাফার>>
FileSystem::get_বাফার_For_File(ধ্রুবক llvm::Twine &Name, int64_t File_আকার,
                             যৌক্তিক RequiresNullTerminator, যৌক্তিক IsVolatile,
                             যৌক্তিক IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_দ্বিমিক(Name);
  যদি (!F)
    ফেরত F.getError();

  ফেরত (*F)->get_বাফার(Name, File_আকার, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<অক্ষর> &Path) ধ্রুবক {
  যদি (llvm::sys::path::is_absolute(Path))
    ফেরত {};

  auto WorkingDir = getCurrentWorkingDirectory();
  যদি (!WorkingDir)
    ফেরত WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  ফেরত {};
}

std::error_code FileSystem::getRealPath(ধ্রুবক Twine &Path,
                                        SmallVectorImpl<অক্ষর> &Output) {
  ফেরত errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(ধ্রুবক Twine &Path, যৌক্তিক &ফলাফল) {
  ফেরত errc::operation_not_permitted;
}

যৌক্তিক FileSystem::exists(ধ্রুবক Twine &Path) {
  auto Status = status(Path);
  ফেরত Status && Status->exists();
}

llvm::ErrorOr<যৌক্তিক> FileSystem::equivalent(ধ্রুবক Twine &A, ধ্রুবক Twine &B) {
  auto StatusA = status(A);
  যদি (!StatusA)
    ফেরত StatusA.getError();
  auto StatusB = status(B);
  যদি (!StatusB)
    ফেরত StatusB.getError();
  ফেরত StatusA->equivalent(*StatusB);
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
শূন্য FileSystem::dump() ধ্রুবক { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
স্থির যৌক্তিক isTraversalComponent(StringRef Component) {
  ফেরত Component == ".." || Component == ".";
}

স্থির যৌক্তিক pathHasTraversal(StringRef Path) {
  using নামাকাশ llvm::sys;

  জন্য (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    যদি (isTraversalComponent(Comp))
      ফেরত সত্য;
  ফেরত মিথ্যা;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

নামাকাশ {

/// Wrapper around a raw file descriptor.
শ্রেণী RealFile : সর্বজনীন File {
  friend শ্রেণী RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    প্রতিজ্ঞা(FD.isValid() && "Invalid or inactive file descriptor");
  }

সর্বজনীন:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<Memory_বাফার>> get_বাফার(ধ্রুবক Twine &Name,
                                                   int64_t File_আকার,
                                                   যৌক্তিক RequiresNullTerminator,
                                                   যৌক্তিক IsVolatile) override;
  std::error_code close() override;
  শূন্য setPath(ধ্রুবক Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  প্রতিজ্ঞা(FD.isValid() && "cannot stat closed file");
  যদি (!S.isStatusKnown()) {
    file_status RealStatus;
    যদি (std::error_code EC = sys::fs::status(FD, RealStatus))
      ফেরত EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  ফেরত S;
}

ErrorOr<std::string> RealFile::getName() {
  ফেরত RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<Memory_বাফার>>
RealFile::get_বাফার(ধ্রুবক Twine &Name, int64_t File_আকার,
                    যৌক্তিক RequiresNullTerminator, যৌক্তিক IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  প্রতিজ্ঞা(FD.isValid() && "cannot get buffer for closed file");
  ফেরত Memory_বাফার::getOpenFile(FD, Name, File_আকার, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  ফেরত EC;
}

শূন্য RealFile::setPath(ধ্রুবক Twine &Path) {
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
শ্রেণী RealFileSystem : সর্বজনীন FileSystem {
সর্বজনীন:
  explicit RealFileSystem(যৌক্তিক LinkCWDToProcess) {
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

  ErrorOr<Status> status(ধ্রুবক Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ধ্রুবক Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_দ্বিমিক(ধ্রুবক Twine &Path) override;
  directory_iterator dir_begin(ধ্রুবক Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ধ্রুবক override;
  std::error_code setCurrentWorkingDirectory(ধ্রুবক Twine &Path) override;
  std::error_code isLocal(ধ্রুবক Twine &Path, যৌক্তিক &ফলাফল) override;
  শূন্য
  getDirectoryContentRealSources(ধ্রুবক Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.ঠেলো_back(Dir.str());
  }
  std::error_code getRealPath(ধ্রুবক Twine &Path,
                              SmallVectorImpl<অক্ষর> &Output) override;

সুরক্ষিত:
  শূন্য printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) ধ্রুবক override;

ব্যক্তিগত:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(ধ্রুবক Twine &Path, SmallVectorImpl<অক্ষর> &Storage) ধ্রুবক {
    যদি (!WD || !*WD)
      ফেরত Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    ফেরত Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ধ্রুবক Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    যদি (!FDOrErr)
      ফেরত errorToErrorCode(FDOrErr.takeError());
    ফেরত std::unique_ptr<File>(
        নতুন RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  কাঠামো WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(ধ্রুবক Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  যদি (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    ফেরত EC;
  ফেরত Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ধ্রুবক Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ফেরত openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_দ্বিমিক(ধ্রুবক Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ফেরত openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ধ্রুবক {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (WD && *WD)
    ফেরত std::string(WD->get().Specified);
  যদি (WD)
    ফেরত WD->getError();

  SmallString<128> Dir;
  যদি (std::error_code EC = llvm::sys::fs::current_path(Dir))
    ফেরত EC;
  ফেরত std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(ধ্রুবক Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (!WD)
    ফেরত llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  যৌক্তিক IsDir;
  যদি (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    ফেরত Err;
  যদি (!IsDir)
    ফেরত std::make_error_code(std::errc::not_a_directory);
  যদি (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    ফেরত Err;
  WD = WorkingDirectory{Absolute, Resolved};
  ফেরত std::error_code();
}

std::error_code RealFileSystem::isLocal(ধ্রুবক Twine &Path, যৌক্তিক &ফলাফল) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ফেরত llvm::sys::fs::is_local(adjustPath(Path, Storage), ফলাফল);
}

std::error_code RealFileSystem::getRealPath(ধ্রুবক Twine &Path,
                                            SmallVectorImpl<অক্ষর> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  ফেরত llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

শূন্য RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) ধ্রুবক {
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
      makeIntrusiveRefCnt<RealFileSystem>(সত্য);
  sys::sandbox::violationIfEnabled();

  ফেরত FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  ফেরত std::make_unique<RealFileSystem>(মিথ্যা);
}

নামাকাশ {

শ্রেণী RealFSDirIter : সর্বজনীন llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

সর্বজনীন:
  RealFSDirIter(ধ্রুবক Twine &Path, std::error_code &EC) {
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
    ফেরত EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ধ্রুবক Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  ফেরত directory_iterator(
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

ErrorOr<Status> OverlayFileSystem::status(ধ্রুবক Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  জন্য (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    যদি (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      ফেরত Status;
  }
  ফেরত make_error_code(llvm::errc::no_such_file_or_directory);
}

যৌক্তিক OverlayFileSystem::exists(ধ্রুবক Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  জন্য (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    যদি ((*I)->exists(Path))
      ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ধ্রুবক llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  জন্য (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto ফলাফল = (*I)->openFileForRead(Path);
    যদি (ফলাফল || ফলাফল.getError() != llvm::errc::no_such_file_or_directory)
      ফেরত ফলাফল;
  }
  ফেরত make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ধ্রুবক {
  // All file systems are synchronized, just take the first working directory.
  ফেরত FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(ধ্রুবক Twine &Path) {
  জন্য (auto &FS : FSList)
    যদি (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      ফেরত EC;
  ফেরত {};
}

std::error_code OverlayFileSystem::isLocal(ধ্রুবক Twine &Path, যৌক্তিক &ফলাফল) {
  জন্য (auto &FS : FSList)
    যদি (FS->exists(Path))
      ফেরত FS->isLocal(Path, ফলাফল);
  ফেরত errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(ধ্রুবক Twine &Path,
                                               SmallVectorImpl<অক্ষর> &Output) {
  জন্য (ধ্রুবক auto &FS : FSList)
    যদি (FS->exists(Path))
      ফেরত FS->getRealPath(Path, Output);
  ফেরত errc::no_such_file_or_directory;
}

শূন্য OverlayFileSystem::getDirectoryContentRealSources(
    ধ্রুবক Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  জন্য (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

শূন্য OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  জন্য (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

শূন্য OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) ধ্রুবক {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  যদি (Type == PrintType::Summary)
    ফেরত;

  যদি (Type == PrintType::Contents)
    Type = PrintType::Summary;
  জন্য (ধ্রুবক auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = পূর্বনির্ধারিত;

নামাকাশ {

/// Combines and deduplicates directory entries across multiple file systems.
শ্রেণী CombiningDirIterImpl : সর্বজনীন llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(যৌক্তিক IsFirstTime) {
    যখন (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.বের_করো_back();
      যদি (CurrentDirIter != directory_iterator())
        বিরতি; // found
    }

    যদি (IsFirstTime && CurrentDirIter == directory_iterator())
      ফেরত errc::no_such_file_or_directory;
    ফেরত {};
  }

  std::error_code incrementDirIter(যৌক্তিক IsFirstTime) {
    প্রতিজ্ঞা((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    যদি (!IsFirstTime)
      CurrentDirIter.increment(EC);
    যদি (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    ফেরত EC;
  }

  std::error_code incrementImpl(যৌক্তিক IsFirstTime) {
    যখন (সত্য) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      যদি (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        ফেরত EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      যদি (SeenNames.ঢোকাও(Name).second)
        ফেরত EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

সর্বজনীন:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    জন্য (ধ্রুবক auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      যদি (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        ফেরত;
      }
      যদি (!FEC)
        IterList.ঠেলো_back(Iter);
    }
    EC = incrementImpl(সত্য);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(সত্য);
  }

  std::error_code increment() override { ফেরত incrementImpl(মিথ্যা); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ধ্রুবক Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  যদি (EC)
    ফেরত {};
  ফেরত Combined;
}

শূন্য ProxyFileSystem::anchor() {}

নামাকাশ llvm {
নামাকাশ vfs {

নামাকাশ detail {

গণনা In_Memory_গ্রন্থি_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
শ্রেণী In_Memory_গ্রন্থি {
  In_Memory_গ্রন্থি_Kind Kind;
  std::string FileName;

সর্বজনীন:
  In_Memory_গ্রন্থি(llvm::StringRef FileName, In_Memory_গ্রন্থি_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  আভাসী ~In_Memory_গ্রন্থি() = পূর্বনির্ধারিত;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  আভাসী Status getStatus(ধ্রুবক Twine &RequestedName) ধ্রুবক = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ধ্রুবক { ফেরত FileName; }
  In_Memory_গ্রন্থি_Kind getKind() ধ্রুবক { ফেরত Kind; }
  আভাসী std::string toString(unsigned Indent) ধ্রুবক = 0;
};

শ্রেণী InMemoryFile : সর্বজনীন In_Memory_গ্রন্থি {
  Status Stat;
  std::unique_ptr<llvm::Memory_বাফার> বাফার;

সর্বজনীন:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::Memory_বাফার> বাফার)
      : In_Memory_গ্রন্থি(Stat.getName(), IME_File), Stat(std::move(Stat)),
        বাফার(std::move(বাফার)) {}

  Status getStatus(ধ্রুবক Twine &RequestedName) ধ্রুবক override {
    ফেরত Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::Memory_বাফার *get_বাফার() ধ্রুবক { ফেরত বাফার.get(); }

  std::string toString(unsigned Indent) ধ্রুবক override {
    ফেরত (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  স্থির যৌক্তিক classof(ধ্রুবক In_Memory_গ্রন্থি *N) {
    ফেরত N->getKind() == IME_File;
  }
};

নামাকাশ {

শ্রেণী InMemoryHardLink : সর্বজনীন In_Memory_গ্রন্থি {
  ধ্রুবক InMemoryFile &ResolvedFile;

সর্বজনীন:
  InMemoryHardLink(StringRef Path, ধ্রুবক InMemoryFile &ResolvedFile)
      : In_Memory_গ্রন্থি(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ধ্রুবক InMemoryFile &getResolvedFile() ধ্রুবক { ফেরত ResolvedFile; }

  Status getStatus(ধ্রুবক Twine &RequestedName) ধ্রুবক override {
    ফেরত ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) ধ্রুবক override {
    ফেরত std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  স্থির যৌক্তিক classof(ধ্রুবক In_Memory_গ্রন্থি *N) {
    ফেরত N->getKind() == IME_HardLink;
  }
};

শ্রেণী InMemorySymbolicLink : সর্বজনীন In_Memory_গ্রন্থি {
  std::string TargetPath;
  Status Stat;

সর্বজনীন:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_গ্রন্থি(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) ধ্রুবক override {
    ফেরত std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(ধ্রুবক Twine &RequestedName) ধ্রুবক override {
    ফেরত Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() ধ্রুবক { ফেরত TargetPath; }

  স্থির যৌক্তিক classof(ধ্রুবক In_Memory_গ্রন্থি *N) {
    ফেরত N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
শ্রেণী InMemoryFileAdaptor : সর্বজনীন File {
  ধ্রুবক InMemoryFile &গ্রন্থি;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

সর্বজনীন:
  explicit InMemoryFileAdaptor(ধ্রুবক InMemoryFile &গ্রন্থি,
                               std::string RequestedName)
      : গ্রন্থি(গ্রন্থি), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    ফেরত গ্রন্থি.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::Memory_বাফার>>
  get_বাফার(ধ্রুবক Twine &Name, int64_t File_আকার, যৌক্তিক RequiresNullTerminator,
            যৌক্তিক IsVolatile) override {
    llvm::Memory_বাফার *Buf = গ্রন্থি.get_বাফার();
    ফেরত llvm::Memory_বাফার::get_Mem_বাফার(
        Buf->get_বাফার(), Buf->get_বাফার_Identifier(), RequiresNullTerminator);
  }

  std::error_code close() override { ফেরত {}; }

  শূন্য setPath(ধ্রুবক Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

শ্রেণী InMemoryDirectory : সর্বজনীন In_Memory_গ্রন্থি {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_গ্রন্থি>, std::less<>> Entries;

সর্বজনীন:
  InMemoryDirectory(Status Stat)
      : In_Memory_গ্রন্থি(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(ধ্রুবক Twine &RequestedName) ধ্রুবক override {
    ফেরত Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ধ্রুবক { ফেরত Stat.getUniqueID(); }

  In_Memory_গ্রন্থি *getChild(StringRef Name) ধ্রুবক {
    auto I = Entries.find(Name);
    যদি (I != Entries.end())
      ফেরত I->second.get();
    ফেরত nullptr;
  }

  In_Memory_গ্রন্থি *addChild(StringRef Name, std::unique_ptr<In_Memory_গ্রন্থি> Child) {
    ফেরত Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ধ্রুবক { ফেরত Entries.begin(); }
  const_iterator end() ধ্রুবক { ফেরত Entries.end(); }

  std::string toString(unsigned Indent) ধ্রুবক override {
    std::string ফলাফল =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    জন্য (ধ্রুবক auto &Entry : Entries)
      ফলাফল += Entry.second->toString(Indent + 2);
    ফেরত ফলাফল;
  }

  স্থির যৌক্তিক classof(ধ্রুবক In_Memory_গ্রন্থি *N) {
    ফেরত N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
স্থির sys::fs::UniqueID getUniqueID(hash_code Hash) {
  ফেরত sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
স্থির sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  ফেরত getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
স্থির sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  ফেরত getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_গ্রন্থি_Info::makeStatus() ধ্রুবক {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, বাফার ? বাফার->get_বাফার() : "");

  ফেরত Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, বাফার ? বাফার->get_বাফার_আকার() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(যৌক্তিক UseNormalizedPaths)
    : মূল_2(নতুন detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = পূর্বনির্ধারিত;

std::string InMemoryFileSystem::toString() ধ্রুবক {
  ফেরত মূল_2->toString(/*Indent=*/0);
}

যৌক্তিক InMemoryFileSystem::addFile(ধ্রুবক Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::Memory_বাফার> বাফার,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_গ্রন্থি_Fn Make_গ্রন্থি) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  প্রতিজ্ঞা(!EC);
  (শূন্য)EC;

  যদি (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/সত্য);

  যদি (Path.empty())
    ফেরত মিথ্যা;

  detail::InMemoryDirectory *Dir = মূল_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  ধ্রুবক auto ResolvedUser = User.মান_or(0);
  ধ্রুবক auto ResolvedGroup = Group.মান_or(0);
  ধ্রুবক auto ResolvedType = Type.মান_or(sys::fs::file_type::regular_file);
  ধ্রুবক auto ResolvedPerms = Perms.মান_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ধ্রুবক auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  যখন (সত্য) {
    Name = *I;
    ++I;
    যদি (I == E)
      বিরতি;
    detail::In_Memory_গ্রন্থি *গ্রন্থি = Dir->getChild(Name);
    যদি (!গ্রন্থি) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      চালু;
    }
    // Creating file under another file.
    যদি (!isa<detail::InMemoryDirectory>(গ্রন্থি))
      ফেরত মিথ্যা;
    Dir = cast<detail::InMemoryDirectory>(গ্রন্থি);
  }
  detail::In_Memory_গ্রন্থি *গ্রন্থি = Dir->getChild(Name);
  যদি (!গ্রন্থি) {
    Dir->addChild(Name,
                  Make_গ্রন্থি({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(বাফার), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    ফেরত সত্য;
  }
  যদি (isa<detail::InMemoryDirectory>(গ্রন্থি))
    ফেরত ResolvedType == sys::fs::file_type::directory_file;

  প্রতিজ্ঞা((isa<detail::InMemoryFile>(গ্রন্থি) ||
          isa<detail::InMemoryHardLink>(গ্রন্থি)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  যদি (auto *Link = dyn_cast<detail::InMemoryHardLink>(গ্রন্থি)) {
    ফেরত Link->getResolvedFile().get_বাফার()->get_বাফার() ==
           বাফার->get_বাফার();
  }
  ফেরত cast<detail::InMemoryFile>(গ্রন্থি)->get_বাফার()->get_বাফার() ==
         বাফার->get_বাফার();
}

যৌক্তিক InMemoryFileSystem::addFile(ধ্রুবক Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::Memory_বাফার> বাফার,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  ফেরত addFile(P, ModificationTime, std::move(বাফার), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_গ্রন্থি_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_গ্রন্থি> {
                   Status Stat = NNI.makeStatus();
                   যদি (Stat.getType() == sys::fs::file_type::directory_file)
                     ফেরত std::make_unique<detail::InMemoryDirectory>(Stat);
                   ফেরত std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.বাফার));
                 });
}

যৌক্তিক InMemoryFileSystem::addFileNoOwn(
    ধ্রুবক Twine &P, time_t ModificationTime,
    ধ্রুবক llvm::Memory_বাফার_Ref &বাফার, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  ফেরত addFile(P, ModificationTime, llvm::Memory_বাফার::get_Mem_বাফার(বাফার),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_গ্রন্থি_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_গ্রন্থি> {
                   Status Stat = NNI.makeStatus();
                   যদি (Stat.getType() == sys::fs::file_type::directory_file)
                     ফেরত std::make_unique<detail::InMemoryDirectory>(Stat);
                   ফেরত std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.বাফার));
                 });
}

detail::Named_গ্রন্থি_Or_Error
InMemoryFileSystem::lookup_গ্রন্থি(ধ্রুবক Twine &P, যৌক্তিক FollowFinalSymlink,
                               size_t SymlinkDepth) ধ্রুবক {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  প্রতিজ্ঞা(!EC);
  (শূন্য)EC;

  যদি (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/সত্য);

  ধ্রুবক detail::InMemoryDirectory *Dir = মূল_2.get();
  যদি (Path.empty())
    ফেরত detail::Named_গ্রন্থি_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  যখন (সত্য) {
    detail::In_Memory_গ্রন্থি *গ্রন্থি = Dir->getChild(*I);
    ++I;
    যদি (!গ্রন্থি)
      ফেরত errc::no_such_file_or_directory;

    যদি (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(গ্রন্থি)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      যদি (I == E && !FollowFinalSymlink)
        ফেরত detail::Named_গ্রন্থি_Or_Error(Path, Symlink);

      যদি (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        ফেরত errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      যদি (std::error_code EC = makeAbsolute(TargetPath))
        ফেরত EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_গ্রন্থি(TargetPath, /*FollowFinalSymlink=*/সত্য, SymlinkDepth + 1);
      যদি (!Target || I == E)
        ফেরত Target;

      যদি (!isa<detail::InMemoryDirectory>(*Target))
        ফেরত errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      চালু;
    }

    // Return the file if it's at the end of the path.
    যদি (auto File = dyn_cast<detail::InMemoryFile>(গ্রন্থি)) {
      যদি (I == E)
        ফেরত detail::Named_গ্রন্থি_Or_Error(Path, File);
      ফেরত errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    যদি (auto File = dyn_cast<detail::InMemoryHardLink>(গ্রন্থি)) {
      যদি (I == E)
        ফেরত detail::Named_গ্রন্থি_Or_Error(Path, &File->getResolvedFile());
      ফেরত errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(গ্রন্থি);
    যদি (I == E)
      ফেরত detail::Named_গ্রন্থি_Or_Error(Path, Dir);
  }
}

যৌক্তিক InMemoryFileSystem::addHardLink(ধ্রুবক Twine &NewLink,
                                     ধ্রুবক Twine &Target) {
  auto New_Link_গ্রন্থি = lookup_গ্রন্থি(NewLink, /*FollowFinalSymlink=*/মিথ্যা);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_গ্রন্থি = lookup_গ্রন্থি(Target, /*FollowFinalSymlink=*/সত্য);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  যদি (!Target_গ্রন্থি || New_Link_গ্রন্থি || !isa<detail::InMemoryFile>(*Target_গ্রন্থি))
    ফেরত মিথ্যা;
  ফেরত addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_গ্রন্থি_Info NNI) {
                   ফেরত std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_গ্রন্থি));
                 });
}

যৌক্তিক InMemoryFileSystem::addSymbolicLink(
    ধ্রুবক Twine &NewLink, ধ্রুবক Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_গ্রন্থি = lookup_গ্রন্থি(NewLink, /*FollowFinalSymlink=*/মিথ্যা);
  যদি (New_Link_গ্রন্থি)
    ফেরত মিথ্যা;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  ফেরত addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_গ্রন্থি_Info NNI) {
                   ফেরত std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(ধ্রুবক Twine &Path) {
  auto গ্রন্থি = lookup_গ্রন্থি(Path, /*FollowFinalSymlink=*/সত্য);
  যদি (গ্রন্থি)
    ফেরত (*গ্রন্থি)->getStatus(Path);
  ফেরত গ্রন্থি.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(ধ্রুবক Twine &Path) {
  auto গ্রন্থি = lookup_গ্রন্থি(Path,/*FollowFinalSymlink=*/সত্য);
  যদি (!গ্রন্থি)
    ফেরত গ্রন্থি.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  যদি (auto *F = dyn_cast<detail::InMemoryFile>(*গ্রন্থি))
    ফেরত std::unique_ptr<File>(
        নতুন detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  ফেরত make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
শ্রেণী InMemoryFileSystem::DirIterator : সর্বজনীন llvm::vfs::detail::DirIterImpl {
  ধ্রুবক InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  শূন্য setCurrentEntry() {
    যদি (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      নির্বাচন (I->second->getKind()) {
      ক্ষেত্র detail::IME_File:
      ক্ষেত্র detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        বিরতি;
      ক্ষেত্র detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        বিরতি;
      ক্ষেত্র detail::IME_SymbolicLink:
        যদি (auto SymlinkTarget =
                FS->lookup_গ্রন্থি(Path, /*FollowFinalSymlink=*/সত্য)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        বিরতি;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } অন্যথা {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

সর্বজনীন:
  DirIterator() = পূর্বনির্ধারিত;

  DirIterator(ধ্রুবক InMemoryFileSystem *FS,
              ধ্রুবক detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    ফেরত {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(ধ্রুবক Twine &Dir,
                                                 std::error_code &EC) {
  auto গ্রন্থি = lookup_গ্রন্থি(Dir, /*FollowFinalSymlink=*/সত্য);
  যদি (!গ্রন্থি) {
    EC = গ্রন্থি.getError();
    ফেরত directory_iterator(std::make_shared<DirIterator>());
  }

  যদি (auto *Dir_গ্রন্থি = dyn_cast<detail::InMemoryDirectory>(*গ্রন্থি))
    ফেরত directory_iterator(
        std::make_shared<DirIterator>(এই, *Dir_গ্রন্থি, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  ফেরত directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(ধ্রুবক Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  প্রতিজ্ঞা(!EC);
  (শূন্য)EC;

  যদি (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/সত্য);

  যদি (!Path.empty())
    WorkingDirectory = std::string(Path);
  ফেরত {};
}

std::error_code InMemoryFileSystem::getRealPath(ধ্রুবক Twine &Path,
                                                SmallVectorImpl<অক্ষর> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  যদি (!CWD || CWD->empty())
    ফেরত errc::operation_not_permitted;
  Path.toVector(Output);
  যদি (auto EC = makeAbsolute(Output))
    ফেরত EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/সত্য);
  ফেরত {};
}

std::error_code InMemoryFileSystem::isLocal(ধ্রুবক Twine &Path, যৌক্তিক &ফলাফল) {
  ফলাফল = মিথ্যা;
  ফেরত {};
}

শূন্য InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) ধ্রুবক {
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
  ধ্রুবক size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  যদি (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  ফেরত style;
}

/// Removes leading "./" as well as path components like ".." and ".".
স্থির llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> ফলাফল_2 =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(ফলাফল_2, /*remove_dot_dot=*/সত্য, style);
  ফেরত ফলাফল_2;
}

/// Whether the error and entry specify a file/directory that was not found.
স্থির যৌক্তিক isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  যদি (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    ফেরত মিথ্যা;
  ফেরত EC == llvm::errc::no_such_file_or_directory;
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
শ্রেণী llvm::vfs::RedirectingFSDirIterImpl
    : সর্বজনীন llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(যৌক্তিক IsFirstTime) {
    প্রতিজ্ঞা((IsFirstTime || Current != End) && "cannot iterate past end");
    যদি (!IsFirstTime)
      ++Current;
    যদি (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      নির্বাচন ((*Current)->getKind()) {
      ক্ষেত্র RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      ক্ষেত্র RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        বিরতি;
      ক্ষেত্র RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        বিরতি;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } অন্যথা {
      CurrentEntry = directory_entry();
    }
    ফেরত {};
  };

সর্বজনীন:
  RedirectingFSDirIterImpl(
      ধ্রুবক Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/সত্য);
  }

  std::error_code increment() override {
    ফেরত incrementImpl(/*IsFirstTime=*/মিথ্যা);
  }
};

নামাকাশ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
শ্রেণী RedirectingFSDirRemapIterImpl : সর্বজনীন llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

সর্বজনীন:
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
    ফেরত EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ধ্রুবক {
  ফেরত WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(ধ্রুবক Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  যদি (!exists(Path))
    ফেরত errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  যদি (std::error_code EC = makeAbsolute(AbsolutePath))
    ফেরত EC;
  WorkingDirectory = std::string(AbsolutePath);
  ফেরত {};
}

std::error_code RedirectingFileSystem::isLocal(ধ্রুবক Twine &Path_,
                                               যৌক্তিক &ফলাফল) {
  SmallString<256> Path;
  Path_.toVector(Path);

  যদি (makeAbsolute(Path))
    ফেরত {};

  ফেরত ExternalFS->isLocal(Path, ফলাফল);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<অক্ষর> &Path) ধ্রুবক {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  যদি (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    ফেরত {};

  auto WorkingDir = getCurrentWorkingDirectory();
  যদি (!WorkingDir)
    ফেরত WorkingDir.getError();

  ফেরত makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<অক্ষর> &Path) ধ্রুবক {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  যদি (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    ফেরত std::error_code();
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

  std::string ফলাফল = std::string(WorkingDir);
  StringRef Dir(ফলাফল);
  যদি (!Dir.ends_with(sys::path::get_separator(style))) {
    ফলাফল += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  ফলাফল.append(Path.তথ্য(), Path.আকার_3());
  Path.assign(ফলাফল.begin(), ফলাফল.end());

  ফেরত {};
}

directory_iterator RedirectingFileSystem::dir_begin(ধ্রুবক Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  যদি (EC)
    ফেরত {};

  ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> ফলাফল = lookupPath(Path);
  যদি (!ফলাফল) {
    যদি (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(ফলাফল.getError()))
      ফেরত ExternalFS->dir_begin(Path, EC);

    EC = ফলাফল.getError();
    ফেরত {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *ফলাফল);
  যদি (!S) {
    যদি (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), ফলাফল->E))
      ফেরত ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    ফেরত {};
  }

  যদি (!S->isDirectory()) {
    EC = errc::not_a_directory;
    ফেরত {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  যদি (auto ExtRedirect = ফলাফল->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(ফলাফল->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    যদি (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } অন্যথা {
    auto DE = cast<DirectoryEntry>(ফলাফল->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  যদি (RedirectEC) {
    যদি (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      ফেরত {};
    }
    RedirectIter = {};
  }

  যদি (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    ফেরত RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  যদি (ExternalEC) {
    যদি (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      ফেরত {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  নির্বাচন (Redirection) {
  ক্ষেত্র RedirectKind::Fallthrough:
    Iters.ঠেলো_back(ExternalIter);
    Iters.ঠেলো_back(RedirectIter);
    বিরতি;
  ক্ষেত্র RedirectKind::Fallback:
    Iters.ঠেলো_back(RedirectIter);
    Iters.ঠেলো_back(ExternalIter);
    বিরতি;
  পূর্বনির্ধারিত:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  যদি (EC)
    ফেরত {};
  ফেরত Combined;
}

শূন্য RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ধ্রুবক {
  ফেরত OverlayFileDir;
}

শূন্য RedirectingFileSystem::setFallthrough(যৌক্তিক Fallthrough) {
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

std::vector<StringRef> RedirectingFileSystem::getRoots() ধ্রুবক {
  std::vector<StringRef> R;
  R.reserve(Roots.আকার_3());
  জন্য (ধ্রুবক auto &মূল_2 : Roots)
    R.ঠেলো_back(মূল_2->getName());
  ফেরত R;
}

শূন্য RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) ধ্রুবক {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  যদি (Type == PrintType::Summary)
    ফেরত;

  জন্য (ধ্রুবক auto &মূল_2 : Roots)
    printEntry(OS, মূল_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

শূন্য RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) ধ্রুবক {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  নির্বাচন (E->getKind()) {
  ক্ষেত্র EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    জন্য (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    বিরতি;
  }
  ক্ষেত্র EK_DirectoryRemap:
  ক্ষেত্র EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    নির্বাচন (RE->getUseName()) {
    ক্ষেত্র NK_NotSet:
      বিরতি;
    ক্ষেত্র NK_External:
      OS << " (UseExternalName: true)";
      বিরতি;
    ক্ষেত্র NK_Virtual:
      OS << " (UseExternalName: false)";
      বিরতি;
    }
    OS << "\n";
    বিরতি;
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
শ্রেণী llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  শূন্য error(yaml::গ্রন্থি *N, ধ্রুবক Twine &বার্তা_2) { Stream.printError(N, বার্তা_2); }

  // false on error
  যৌক্তিক parseScalarString(yaml::গ্রন্থি *N, StringRef &ফলাফল,
                         SmallVectorImpl<অক্ষর> &Storage) {
    ধ্রুবক auto *S = dyn_cast_if_present<yaml::Scalar_গ্রন্থি>(N);

    যদি (!S) {
      error(N, "expected string");
      ফেরত মিথ্যা;
    }
    ফলাফল = S->get_মান(Storage);
    ফেরত সত্য;
  }

  // false on error
  যৌক্তিক parseScalarBool(yaml::গ্রন্থি *N, যৌক্তিক &ফলাফল) {
    SmallString<5> Storage;
    StringRef মান;
    যদি (!parseScalarString(N, মান, Storage))
      ফেরত মিথ্যা;

    যদি (মান.equals_insensitive("true") || মান.equals_insensitive("on") ||
        মান.equals_insensitive("yes") || মান == "1") {
      ফলাফল = সত্য;
      ফেরত সত্য;
    } অন্যথা যদি (মান.equals_insensitive("false") ||
               মান.equals_insensitive("off") ||
               মান.equals_insensitive("no") || মান == "0") {
      ফলাফল = মিথ্যা;
      ফেরত সত্য;
    }

    error(N, "expected boolean value");
    ফেরত মিথ্যা;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::গ্রন্থি *N) {
    SmallString<12> Storage;
    StringRef মান;
    যদি (!parseScalarString(N, মান, Storage))
      ফেরত std::nullopt;

    যদি (মান.equals_insensitive("fallthrough")) {
      ফেরত RedirectingFileSystem::RedirectKind::Fallthrough;
    } অন্যথা যদি (মান.equals_insensitive("fallback")) {
      ফেরত RedirectingFileSystem::RedirectKind::Fallback;
    } অন্যথা যদি (মান.equals_insensitive("redirect-only")) {
      ফেরত RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    ফেরত std::nullopt;
  }

  std::optional<RedirectingFileSystem::মূল_Relative_Kind>
  parse_মূল_Relative_Kind(yaml::গ্রন্থি *N) {
    SmallString<12> Storage;
    StringRef মান;
    যদি (!parseScalarString(N, মান, Storage))
      ফেরত std::nullopt;
    যদি (মান.equals_insensitive("cwd")) {
      ফেরত RedirectingFileSystem::মূল_Relative_Kind::CWD;
    } অন্যথা যদি (মান.equals_insensitive("overlay-dir")) {
      ফেরত RedirectingFileSystem::মূল_Relative_Kind::OverlayDir;
    }
    ফেরত std::nullopt;
  }

  কাঠামো চাবি_Status {
    যৌক্তিক Required;
    যৌক্তিক Seen = মিথ্যা;

    চাবি_Status(যৌক্তিক Required = মিথ্যা) : Required(Required) {}
  };

  using চাবি_Status_Pair = std::pair<StringRef, চাবি_Status>;

  // false on error
  যৌক্তিক check_Duplicate_Or_Unknown_চাবি(yaml::গ্রন্থি *চাবি_গ্রন্থি, StringRef চাবি,
                                  DenseMap<StringRef, চাবি_Status> &চাবিগুলি_2) {
    auto It = চাবিগুলি_2.find(চাবি);
    যদি (It == চাবিগুলি_2.end()) {
      error(চাবি_গ্রন্থি, "unknown key");
      ফেরত মিথ্যা;
    }
    চাবি_Status &S = It->second;
    যদি (S.Seen) {
      error(চাবি_গ্রন্থি, Twine("duplicate key '") + চাবি + "'");
      ফেরত মিথ্যা;
    }
    S.Seen = সত্য;
    ফেরত সত্য;
  }

  // false on error
  যৌক্তিক check_Missing_চাবিগুলি(yaml::গ্রন্থি *Obj, DenseMap<StringRef, চাবি_Status> &চাবিগুলি_2) {
    জন্য (ধ্রুবক auto &I : চাবিগুলি_2) {
      যদি (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        ফেরত মিথ্যা;
      }
    }
    ফেরত সত্য;
  }

সর্বজনীন:
  স্থির RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    যদি (!ParentEntry) { // Look for a existent root
      জন্য (ধ্রুবক auto &মূল_2 : FS->Roots) {
        যদি (Name == মূল_2->getName()) {
          ParentEntry = মূল_2.get();
          ফেরত ParentEntry;
        }
      }
    } অন্যথা { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      জন্য (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        যদি (DirContent && Name == Content->getName())
          ফেরত DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", get_পরবর্তী_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    যদি (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.ঠেলো_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      ফেরত ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    ফেরত DE->getLastContent();
  }

ব্যক্তিগত:
  শূন্য unique_Overlay_বৃক্ষ(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    নির্বাচন (SrcE->getKind()) {
    ক্ষেত্র RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      যদি (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      জন্য (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_বৃক্ষ(FS, SubEntry.get(), NewParentE);
      বিরতি;
    }
    ক্ষেত্র RedirectingFileSystem::EK_DirectoryRemap: {
      প্রতিজ্ঞা(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      বিরতি;
    }
    ক্ষেত্র RedirectingFileSystem::EK_File: {
      প্রতিজ্ঞা(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      বিরতি;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::গ্রন্থি *N, RedirectingFileSystem *FS, যৌক্তিক Is_মূল_Entry) {
    auto *M = dyn_cast<yaml::Mapping_গ্রন্থি>(N);
    যদি (!M) {
      error(N, "expected mapping node for file or directory entry");
      ফেরত nullptr;
    }

    চাবি_Status_Pair Fields[] = {
        চাবি_Status_Pair("name", সত্য),
        চাবি_Status_Pair("type", সত্য),
        চাবি_Status_Pair("contents", মিথ্যা),
        চাবি_Status_Pair("external-contents", মিথ্যা),
        চাবি_Status_Pair("use-external-name", মিথ্যা),
    };

    DenseMap<StringRef, চাবি_Status> চাবিগুলি_2(std::begin(Fields), std::end(Fields));

    গণনা { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::গ্রন্থি *Name_মান_গ্রন্থি = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    জন্য (auto &I : *M) {
      StringRef চাবি;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> বাফার;
      যদি (!parseScalarString(I.get_চাবি(), চাবি, বাফার))
        ফেরত nullptr;

      যদি (!check_Duplicate_Or_Unknown_চাবি(I.get_চাবি(), চাবি, চাবিগুলি_2))
        ফেরত nullptr;

      StringRef মান;
      যদি (চাবি == "name") {
        যদি (!parseScalarString(I.get_মান(), মান, বাফার))
          ফেরত nullptr;

        Name_মান_গ্রন্থি = I.get_মান();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(মান).str();
      } অন্যথা যদি (চাবি == "type") {
        যদি (!parseScalarString(I.get_মান(), মান, বাফার))
          ফেরত nullptr;
        যদি (মান == "file")
          Kind = RedirectingFileSystem::EK_File;
        অন্যথা যদি (মান == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        অন্যথা যদি (মান == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        অন্যথা {
          error(I.get_মান(), "unknown value for 'type'");
          ফেরত nullptr;
        }
      } অন্যথা যদি (চাবি == "contents") {
        যদি (ContentsField != CF_NotSet) {
          error(I.get_চাবি(),
                "entry already has 'contents' or 'external-contents'");
          ফেরত nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_গ্রন্থি>(I.get_মান());
        যদি (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_মান(), "expected array");
          ফেরত nullptr;
        }

        জন্য (auto &I : *Contents) {
          যদি (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ মিথ্যা))
            EntryArrayContents.ঠেলো_back(std::move(E));
          অন্যথা
            ফেরত nullptr;
        }
      } অন্যথা যদি (চাবি == "external-contents") {
        যদি (ContentsField != CF_NotSet) {
          error(I.get_চাবি(),
                "entry already has 'contents' or 'external-contents'");
          ফেরত nullptr;
        }
        ContentsField = CF_External;
        যদি (!parseScalarString(I.get_মান(), মান, বাফার))
          ফেরত nullptr;

        SmallString<256> FullPath;
        যদি (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          প্রতিজ্ঞা(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = মান;
          যদি (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            ফেরত nullptr;
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
        যৌক্তিক Val;
        যদি (!parseScalarBool(I.get_মান(), Val))
          ফেরত nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } অন্যথা {
        llvm_unreachable("key missing from Keys");
      }
    }

    যদি (Stream.failed())
      ফেরত nullptr;

    // check for missing keys
    যদি (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      ফেরত nullptr;
    }
    যদি (!check_Missing_চাবিগুলি(N, চাবিগুলি_2))
      ফেরত nullptr;

    // check invalid configuration
    যদি (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      ফেরত nullptr;
    }

    যদি (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      ফেরত nullptr;
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
          প্রতিজ্ঞা(Name_মান_গ্রন্থি && "Name presence should be checked earlier");
          error(
              Name_মান_গ্রন্থি,
              "entry with relative path at the root level is not discoverable");
          ফেরত nullptr;
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
    size_t মূল_Path_Len = sys::path::মূল_path(Trimmed, path_style).আকার_3();
    যখন (Trimmed.আকার_3() > মূল_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.আকার_3() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> ফলাফল;
    নির্বাচন (Kind) {
    ক্ষেত্র RedirectingFileSystem::EK_File:
      ফলাফল = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      বিরতি;
    ক্ষেত্র RedirectingFileSystem::EK_DirectoryRemap:
      ফলাফল = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      বিরতি;
    ক্ষেত্র RedirectingFileSystem::EK_Directory:
      ফলাফল = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", get_পরবর্তী_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      বিরতি;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    যদি (Parent.empty())
      ফেরত ফলাফল;

    // if 'name' contains multiple components, create implicit directory entries
    জন্য (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.ঠেলো_back(std::move(ফলাফল));
      ফলাফল = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", get_পরবর্তী_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    ফেরত ফলাফল;
  }

সর্বজনীন:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  যৌক্তিক parse(yaml::গ্রন্থি *মূল_2, RedirectingFileSystem *FS) {
    auto *শীর্ষ_3 = dyn_cast<yaml::Mapping_গ্রন্থি>(মূল_2);
    যদি (!শীর্ষ_3) {
      error(মূল_2, "expected mapping node");
      ফেরত মিথ্যা;
    }

    চাবি_Status_Pair Fields[] = {
        চাবি_Status_Pair("version", সত্য),
        চাবি_Status_Pair("case-sensitive", মিথ্যা),
        চাবি_Status_Pair("use-external-names", মিথ্যা),
        চাবি_Status_Pair("root-relative", মিথ্যা),
        চাবি_Status_Pair("overlay-relative", মিথ্যা),
        চাবি_Status_Pair("fallthrough", মিথ্যা),
        চাবি_Status_Pair("redirecting-with", মিথ্যা),
        চাবি_Status_Pair("roots", সত্য),
    };

    DenseMap<StringRef, চাবি_Status> চাবিগুলি_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> মূল_Entries;

    // Parse configuration and 'roots'
    জন্য (auto &I : *শীর্ষ_3) {
      SmallString<10> চাবি_বাফার;
      StringRef চাবি;
      যদি (!parseScalarString(I.get_চাবি(), চাবি, চাবি_বাফার))
        ফেরত মিথ্যা;

      যদি (!check_Duplicate_Or_Unknown_চাবি(I.get_চাবি(), চাবি, চাবিগুলি_2))
        ফেরত মিথ্যা;

      যদি (চাবি == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_গ্রন্থি>(I.get_মান());
        যদি (!Roots) {
          error(I.get_মান(), "expected array");
          ফেরত মিথ্যা;
        }

        জন্য (auto &I : *Roots) {
          যদি (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ সত্য))
            মূল_Entries.ঠেলো_back(std::move(E));
          অন্যথা
            ফেরত মিথ্যা;
        }
      } অন্যথা যদি (চাবি == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        যদি (!parseScalarString(I.get_মান(), VersionString, Storage))
          ফেরত মিথ্যা;
        পূর্ণসংখ্যা Version;
        যদি (VersionString.getAsInteger<পূর্ণসংখ্যা>(10, Version)) {
          error(I.get_মান(), "expected integer");
          ফেরত মিথ্যা;
        }
        যদি (Version < 0) {
          error(I.get_মান(), "invalid version number");
          ফেরত মিথ্যা;
        }
        যদি (Version != 0) {
          error(I.get_মান(), "version mismatch, expected 0");
          ফেরত মিথ্যা;
        }
      } অন্যথা যদি (চাবি == "case-sensitive") {
        যদি (!parseScalarBool(I.get_মান(), FS->CaseSensitive))
          ফেরত মিথ্যা;
      } অন্যথা যদি (চাবি == "overlay-relative") {
        যদি (!parseScalarBool(I.get_মান(), FS->IsRelativeOverlay))
          ফেরত মিথ্যা;
      } অন্যথা যদি (চাবি == "use-external-names") {
        যদি (!parseScalarBool(I.get_মান(), FS->UseExternalNames))
          ফেরত মিথ্যা;
      } অন্যথা যদি (চাবি == "fallthrough") {
        যদি (চাবিগুলি_2["redirecting-with"].Seen) {
          error(I.get_মান(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ফেরত মিথ্যা;
        }

        যৌক্তিক ShouldFallthrough = মিথ্যা;
        যদি (!parseScalarBool(I.get_মান(), ShouldFallthrough))
          ফেরত মিথ্যা;

        যদি (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } অন্যথা {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } অন্যথা যদি (চাবি == "redirecting-with") {
        যদি (চাবিগুলি_2["fallthrough"].Seen) {
          error(I.get_মান(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          ফেরত মিথ্যা;
        }

        যদি (auto Kind = parseRedirectKind(I.get_মান())) {
          FS->Redirection = *Kind;
        } অন্যথা {
          error(I.get_মান(), "expected valid redirect kind");
          ফেরত মিথ্যা;
        }
      } অন্যথা যদি (চাবি == "root-relative") {
        যদি (auto Kind = parse_মূল_Relative_Kind(I.get_মান())) {
          FS->মূল_Relative = *Kind;
        } অন্যথা {
          error(I.get_মান(), "expected valid root-relative kind");
          ফেরত মিথ্যা;
        }
      } অন্যথা {
        llvm_unreachable("key missing from Keys");
      }
    }

    যদি (Stream.failed())
      ফেরত মিথ্যা;

    যদি (!check_Missing_চাবিগুলি(শীর্ষ_3, চাবিগুলি_2))
      ফেরত মিথ্যা;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    জন্য (auto &E : মূল_Entries)
      unique_Overlay_বৃক্ষ(FS, E.get());

    ফেরত সত্য;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<Memory_বাফার> বাফার,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, শূন্য *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(বাফার->get_Mem_বাফার_Ref(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::গ্রন্থি *মূল_2 = DI->get_মূল();
  যদি (DI == Stream.end() || !মূল_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    ফেরত nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      নতুন RedirectingFileSystem(ExternalFS));

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
    ফেরত nullptr;

  ফেরত FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    যৌক্তিক UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      নতুন RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  জন্য (auto &Mapping : llvm::reverse(RemappedFiles)) {
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
      চালু;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    জন্য (auto I = llvm::sys::path::begin(FromDirectory),
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

  ফেরত FS;
}

RedirectingFileSystem::Lookup_ফলাফল::Lookup_ফলাফল(
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

শূন্য RedirectingFileSystem::Lookup_ফলাফল::getPath(
    llvm::SmallVectorImpl<অক্ষর> &ফলাফল) ধ্রুবক {
  ফলাফল.clear();
  জন্য (Entry *Parent : Parents)
    llvm::sys::path::append(ফলাফল, Parent->getName());
  llvm::sys::path::append(ফলাফল, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<অক্ষর> &Path) ধ্রুবক {
  যদি (std::error_code EC = makeAbsolute(Path))
    ফেরত EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.তথ্য(), Path.আকার_3()));
  যদি (CanonicalPath.empty())
    ফেরত make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  ফেরত {};
}

ErrorOr<RedirectingFileSystem::Lookup_ফলাফল>
RedirectingFileSystem::lookupPath(StringRef Path) ধ্রুবক {
  llvm::SmallString<128> CanonicalPath(Path);
  যদি (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    ফেরত EC;

  // RedirectOnly means the VFS is always used.
  যদি (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = সত্য;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  জন্য (ধ্রুবক auto &মূল_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> ফলাফল =
        lookupPathImpl(Start, End, মূল_2.get(), Entries);
    যদি (UsageTrackingActive && ফলাফল && isa<RemapEntry>(ফলাফল->E))
      HasBeenUsed = সত্য;
    যদি (ফলাফল) {
      ফলাফল->Parents = std::move(Entries);
      ফেরত ফলাফল;
    }

    যদি (ফলাফল.getError() != llvm::errc::no_such_file_or_directory)
      ফেরত ফলাফল;
  }
  ফেরত make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_ফলাফল>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) ধ্রুবক {
  প্রতিজ্ঞা(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  যদি (!FromName.empty()) {
    যদি (!pathComponentMatches(*Start, FromName))
      ফেরত make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    যদি (Start == End) {
      // Match!
      ফেরত Lookup_ফলাফল(From, Start, End);
    }
  }

  যদি (isa<RedirectingFileSystem::FileEntry>(From))
    ফেরত make_error_code(llvm::errc::not_a_directory);

  যদি (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    ফেরত Lookup_ফলাফল(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  জন্য (ধ্রুবক std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.ঠেলো_back(From);
    ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> ফলাফল =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    যদি (ফলাফল || ফলাফল.getError() != llvm::errc::no_such_file_or_directory)
      ফেরত ফলাফল;
    Entries.বের_করো_back();
  }

  ফেরত make_error_code(llvm::errc::no_such_file_or_directory);
}

স্থির Status getRedirectedFileStatus(ধ্রুবক Twine &OriginalPath,
                                      যৌক্তিক UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  যদি (ExternalStatus.ExposesExternalVFSPath)
    ফেরত ExternalStatus;

  Status S = ExternalStatus;
  যদি (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  অন্যথা
    S.ExposesExternalVFSPath = সত্য;
  ফেরত S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    ধ্রুবক Twine &LookupPath, ধ্রুবক Twine &OriginalPath,
    ধ্রুবক RedirectingFileSystem::Lookup_ফলাফল &ফলাফল) {
  যদি (std::optional<StringRef> ExtRedirect = ফলাফল.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    যদি (std::error_code EC = makeAbsolute(RemappedPath))
      ফেরত EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    যদি (!S)
      ফেরত S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(ফলাফল.E);
    ফেরত getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ফলাফল.E);
  ফেরত Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(ধ্রুবক Twine &LookupPath,
                                         ধ্রুবক Twine &OriginalPath) ধ্রুবক {
  auto ফলাফল = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  যদি (!ফলাফল || ফলাফল->ExposesExternalVFSPath)
    ফেরত ফলাফল;
  ফেরত Status::copyWithNewName(ফলাফল.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(ধ্রুবক Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  যদি (std::error_code EC = makeAbsolute(Path))
    ফেরত EC;

  যদি (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    যদি (S)
      ফেরত S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> ফলাফল = lookupPath(Path);
  যদি (!ফলাফল) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ফলাফল.getError()))
      ফেরত getExternalStatus(Path, OriginalPath);
    ফেরত ফলাফল.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *ফলাফল);
  যদি (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), ফলাফল->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ফেরত getExternalStatus(Path, OriginalPath);
  }

  ফেরত S;
}

যৌক্তিক RedirectingFileSystem::exists(ধ্রুবক Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  যদি (makeAbsolute(Path))
    ফেরত মিথ্যা;

  যদি (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    যদি (ExternalFS->exists(Path))
      ফেরত সত্য;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> ফলাফল = lookupPath(Path);
  যদি (!ফলাফল) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ফলাফল.getError()))
      ফেরত ExternalFS->exists(Path);
    ফেরত মিথ্যা;
  }

  std::optional<StringRef> ExtRedirect = ফলাফল->getExternalRedirect();
  যদি (!ExtRedirect) {
    প্রতিজ্ঞা(isa<RedirectingFileSystem::DirectoryEntry>(ফলাফল->E));
    ফেরত সত্য;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  যদি (makeAbsolute(RemappedPath))
    ফেরত মিথ্যা;

  যদি (ExternalFS->exists(RemappedPath))
    ফেরত সত্য;

  যদি (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    ফেরত ExternalFS->exists(Path);
  }

  ফেরত মিথ্যা;
}

নামাকাশ {

/// Provide a file wrapper with an overriden status.
শ্রেণী FileWithFixedStatus : সর্বজনীন File {
  std::unique_ptr<File> InnerFile;
  Status S;

সর্বজনীন:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { ফেরত S; }
  ErrorOr<std::unique_ptr<llvm::Memory_বাফার>>

  get_বাফার(ধ্রুবক Twine &Name, int64_t File_আকার, যৌক্তিক RequiresNullTerminator,
            যৌক্তিক IsVolatile) override {
    ফেরত InnerFile->get_বাফার(Name, File_আকার, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { ফেরত InnerFile->close(); }

  শূন্য setPath(ধ্রুবক Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> ফলাফল, ধ্রুবক Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  যদি (!ফলাফল || (*ফলাফল)->status()->ExposesExternalVFSPath)
    ফেরত ফলাফল;

  ErrorOr<std::unique_ptr<File>> F = std::move(*ফলাফল);
  auto Name = F->get()->getName();
  যদি (Name && Name.get() != P.str())
    F->get()->setPath(P);
  ফেরত F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ধ্রুবক Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  যদি (std::error_code EC = makeAbsolute(Path))
    ফেরত EC;

  যদি (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    যদি (F)
      ফেরত F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> ফলাফল = lookupPath(Path);
  যদি (!ফলাফল) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ফলাফল.getError()))
      ফেরত File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    ফেরত ফলাফল.getError();
  }

  যদি (!ফলাফল->getExternalRedirect()) // FIXME: errc::not_a_file?
    ফেরত make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *ফলাফল->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  যদি (std::error_code EC = makeAbsolute(RemappedPath))
    ফেরত EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(ফলাফল->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  যদি (!ExternalFile) {
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), ফলাফল->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ফেরত File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    ফেরত ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  যদি (!ExternalStatus)
    ফেরত ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  ফেরত std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(ধ্রুবক Twine &OriginalPath,
                                   SmallVectorImpl<অক্ষর> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  যদি (std::error_code EC = makeAbsolute(Path))
    ফেরত EC;

  যদি (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    যদি (!EC)
      ফেরত EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> ফলাফল = lookupPath(Path);
  যদি (!ফলাফল) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    যদি (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ফলাফল.getError()))
      ফেরত ExternalFS->getRealPath(Path, Output);
    ফেরত ফলাফল.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  যদি (auto ExtRedirect = ফলাফল->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    যদি (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, ফলাফল->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      ফেরত ExternalFS->getRealPath(Path, Output);
    }
    ফেরত P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  যদি (Redirection == RedirectKind::Fallthrough) {
    ফলাফল->getPath(Output);
    ফেরত {};
  }
  ফেরত llvm::errc::invalid_argument;
}

শূন্য RedirectingFileSystem::getDirectoryContentRealSources(
    ধ্রুবক Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  যদি (makeAbsolute(Path))
    ফেরত;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ধ্রুবক যৌক্তিক ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> ফলাফল = lookupPath(Path);
  যদি (!ফলাফল) {
    // dir_begin() delegates entirely to ExternalFS.
    যদি (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    ফেরত;
  }

  নির্বাচন (ফলাফল->E->getKind()) {
  ক্ষেত্র EK_File:
    ফেরত;
  ক্ষেত্র EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    বিরতি;
  ক্ষেত্র EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*ফলাফল->getExternalRedirect());
    যদি (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    বিরতি;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  যদি (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<Memory_বাফার> বাফার,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, শূন্য *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ফেরত RedirectingFileSystem::create(std::move(বাফার), DiagHandler,
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
    জন্য (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.ঠেলো_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.বের_করো_back();
    }
    ফেরত;
  }

  যদি (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    প্রতিজ্ঞা(DR && "Must be a directory remap");
    SmallString<128> VPath;
    জন্য (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.ঠেলো_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    ফেরত;
  }

  প্রতিজ্ঞা(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  প্রতিজ্ঞা(FE && "Must be a file");
  SmallString<128> VPath;
  জন্য (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.ঠেলো_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

শূন্য vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_ফলাফল> মূল_ফলাফল = VFS.lookupPath("/");
  যদি (!মূল_ফলাফল)
    ফেরত;
  SmallVector<StringRef, 8> Components;
  Components.ঠেলো_back("/");
  getVFSEntries(মূল_ফলাফল->E, Components, CollectedEntries);
}

UniqueID vfs::get_পরবর্তী_Virtual_Unique_ID() {
  স্থির std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  ফেরত UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

শূন্য YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             যৌক্তিক IsDirectory) {
  প্রতিজ্ঞা(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  প্রতিজ্ঞা(sys::path::is_absolute(RealPath) && "real path not absolute");
  প্রতিজ্ঞা(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

শূন্য YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/মিথ্যা);
}

শূন্য YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/সত্য);
}

নামাকাশ {

শ্রেণী JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_স্তূপ;

  unsigned getDirIndent() { ফেরত 4 * Dir_স্তূপ.আকার_3(); }
  unsigned getFileIndent() { ফেরত 4 * (Dir_স্তূপ.আকার_3() + 1); }
  যৌক্তিক containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  শূন্য startDirectory(StringRef Path);
  শূন্য endDirectory();
  শূন্য writeEntry(StringRef VPath, StringRef RPath);

সর্বজনীন:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  শূন্য লিখ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<যৌক্তিক> UseExternalNames,
             std::optional<যৌক্তিক> IsCaseSensitive,
             std::optional<যৌক্তিক> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

যৌক্তিক JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using নামাকাশ llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  জন্য (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    যদি (*IParent != *IChild)
      ফেরত মিথ্যা;
  }
  // Have we exhausted the parent path?
  ফেরত IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  প্রতিজ্ঞা(!Parent.empty());
  প্রতিজ্ঞা(containedIn(Parent, Path));
  ফেরত Path.substr(Parent.আকার_3() + 1);
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

  Dir_স্তূপ.বের_করো_back();
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
                       std::optional<যৌক্তিক> UseExternalNames,
                       std::optional<যৌক্তিক> IsCaseSensitive,
                       std::optional<যৌক্তিক> IsOverlayRelative,
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
  যৌক্তিক UseOverlayRelative = মিথ্যা;
  যদি (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  যদি (!Entries.empty()) {
    ধ্রুবক YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    যদি (UseOverlayRelative) {
      প্রতিজ্ঞা(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.আকার_3());
    }

    যৌক্তিক IsCurrentDirEmpty = সত্য;
    যদি (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = মিথ্যা;
    }

    জন্য (ধ্রুবক auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      যদি (Dir == Dir_স্তূপ.back()) {
        যদি (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } অন্যথা {
        যৌক্তিক Is_Dir_Popped_From_স্তূপ = মিথ্যা;
        যখন (!Dir_স্তূপ.empty() && !containedIn(Dir_স্তূপ.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_স্তূপ = সত্য;
        }
        যদি (Is_Dir_Popped_From_স্তূপ || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = সত্য;
      }
      StringRef RPath = Entry.RPath;
      যদি (UseOverlayRelative) {
        প্রতিজ্ঞা(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.আকার_3());
      }
      যদি (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = মিথ্যা;
      }
    }

    যখন (!Dir_স্তূপ.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

শূন্য YAMLVFSWriter::লিখ(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](ধ্রুবক YAMLVFSEntry &LHS, ধ্রুবক YAMLVFSEntry &RHS) {
    ফেরত LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).লিখ(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ধ্রুবক Twine &Path, std::error_code &EC)
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
    State->Has_No_ঠেলো_Request = মিথ্যা;
  অন্যথা {
    যদি (State->স্তূপ.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->স্তূপ.back()->path(), EC);
      যদি (I != End) {
        State->স্তূপ.ঠেলো_back(I);
        ফেরত *এই;
      }
    }
  }

  যখন (!State->স্তূপ.empty() && State->স্তূপ.back().increment(EC) == End)
    State->স্তূপ.বের_করো_back();

  যদি (State->স্তূপ.empty())
    State.reset(); // end iterator

  ফেরত *এই;
}

ধ্রুবক অক্ষর FileSystem::ID = 0;
ধ্রুবক অক্ষর OverlayFileSystem::ID = 0;
ধ্রুবক অক্ষর ProxyFileSystem::ID = 0;
ধ্রুবক অক্ষর InMemoryFileSystem::ID = 0;
ধ্রুবক অক্ষর RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_গুনতি(
    ধ্রুবক FileSystem *FS) {
  ফেরত FS->Use_গুনতি();
}

শূন্য ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

শূন্য ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
