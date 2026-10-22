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

#अन्तर्भाव "llvm/Support/VirtualFileSystem.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/DenseMap.h"
#अन्तर्भाव "llvm/ADT/IntrusiveRefCntPtr.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/StringSet.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/ADT/iterator_range.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/Casting.h"
#अन्तर्भाव "llvm/Support/Chrono.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/ErrorOr.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/FileSystem/UniqueID.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/SMLoc.h"
#अन्तर्भाव "llvm/Support/SourceMgr.h"
#अन्तर्भाव "llvm/Support/YAMLParser.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <atomic>
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstdint>
#अन्तर्भाव <iterator>
#अन्तर्भाव <limits>
#अन्तर्भाव <map>
#अन्तर्भाव <memory>
#अन्तर्भाव <optional>
#अन्तर्भाव <string>
#अन्तर्भाव <system_error>
#अन्तर्भाव <utility>
#अन्तर्भाव <vector>

using नामाकाश llvm;
using नामाकाश llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(ध्रुव file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(ध्रुव Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(ध्रुव Status &In, uint64_t NewSize) {
  प्रत्यागम Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ध्रुव Status &In, ध्रुव Twine &NewName) {
  प्रत्यागम Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(ध्रुव file_status &In, ध्रुव Twine &NewName) {
  प्रत्यागम Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

तार्किक Status::equivalent(ध्रुव Status &Other) ध्रुव {
  प्रतिज्ञा(isStatusKnown() && Other.isStatusKnown());
  प्रत्यागम getUniqueID() == Other.getUniqueID();
}

तार्किक Status::isDirectory() ध्रुव { प्रत्यागम Type == file_type::directory_file; }

तार्किक Status::isRegularFile() ध्रुव { प्रत्यागम Type == file_type::regular_file; }

तार्किक Status::isOther() ध्रुव {
  प्रत्यागम exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

तार्किक Status::isSymlink() ध्रुव { प्रत्यागम Type == file_type::symlink_file; }

तार्किक Status::isStatusKnown() ध्रुव { प्रत्यागम Type != file_type::status_error; }

तार्किक Status::exists() ध्रुव {
  प्रत्यागम isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = स्वभाव;

FileSystem::~FileSystem() = स्वभाव;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(ध्रुव llvm::Twine &Name, int64_t FileSize,
                             तार्किक RequiresNullTerminator, तार्किक IsVolatile,
                             तार्किक IsText) {
  auto F = IsText ? openFileForRead(Name) : open_File_For_Read_द्विमान(Name);
  जइ (!F)
    प्रत्यागम F.getError();

  प्रत्यागम (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<वर्ण> &Path) ध्रुव {
  जइ (llvm::sys::path::is_absolute(Path))
    प्रत्यागम {};

  auto WorkingDir = getCurrentWorkingDirectory();
  जइ (!WorkingDir)
    प्रत्यागम WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  प्रत्यागम {};
}

std::error_code FileSystem::getRealPath(ध्रुव Twine &Path,
                                        SmallVectorImpl<वर्ण> &Output) {
  प्रत्यागम errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(ध्रुव Twine &Path, तार्किक &Result) {
  प्रत्यागम errc::operation_not_permitted;
}

तार्किक FileSystem::exists(ध्रुव Twine &Path) {
  auto Status = status(Path);
  प्रत्यागम Status && Status->exists();
}

llvm::ErrorOr<तार्किक> FileSystem::equivalent(ध्रुव Twine &A, ध्रुव Twine &B) {
  auto StatusA = status(A);
  जइ (!StatusA)
    प्रत्यागम StatusA.getError();
  auto StatusB = status(B);
  जइ (!StatusB)
    प्रत्यागम StatusB.getError();
  प्रत्यागम StatusA->equivalent(*StatusB);
}

#जइ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
शून्य FileSystem::dump() ध्रुव { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef NDEBUG
स्थिर तार्किक isTraversalComponent(StringRef Component) {
  प्रत्यागम Component == ".." || Component == ".";
}

स्थिर तार्किक pathHasTraversal(StringRef Path) {
  using नामाकाश llvm::sys;

  प्रति (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    जइ (isTraversalComponent(Comp))
      प्रत्यागम सच्च;
  प्रत्यागम मिच्छ;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

नामाकाश {

/// Wrapper around a raw file descriptor.
वर्ग RealFile : सार्वजनिक File {
  friend वर्ग RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    प्रतिज्ञा(FD.isValid() && "Invalid or inactive file descriptor");
  }

सार्वजनिक:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(ध्रुव Twine &Name,
                                                   int64_t FileSize,
                                                   तार्किक RequiresNullTerminator,
                                                   तार्किक IsVolatile) override;
  std::error_code close() override;
  शून्य setPath(ध्रुव Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(FD.isValid() && "cannot stat closed file");
  जइ (!S.isStatusKnown()) {
    file_status RealStatus;
    जइ (std::error_code EC = sys::fs::status(FD, RealStatus))
      प्रत्यागम EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  प्रत्यागम S;
}

ErrorOr<std::string> RealFile::getName() {
  प्रत्यागम RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(ध्रुव Twine &Name, int64_t FileSize,
                    तार्किक RequiresNullTerminator, तार्किक IsVolatile) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(FD.isValid() && "cannot get buffer for closed file");
  प्रत्यागम MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  प्रत्यागम EC;
}

शून्य RealFile::setPath(ध्रुव Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  जइ (auto Status = status())
    S = Status.get().copyWithNewName(Status.get(), Path);
}

नामाकाश {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
वर्ग RealFileSystem : सार्वजनिक FileSystem {
सार्वजनिक:
  explicit RealFileSystem(तार्किक LinkCWDToProcess) {
    जइ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      जइ (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      अण्णहा जइ (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      अण्णहा
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(ध्रुव Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ध्रुव Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_द्विमान(ध्रुव Twine &Path) override;
  directory_iterator dir_begin(ध्रुव Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ध्रुव override;
  std::error_code setCurrentWorkingDirectory(ध्रुव Twine &Path) override;
  std::error_code isLocal(ध्रुव Twine &Path, तार्किक &Result) override;
  शून्य
  getDirectoryContentRealSources(ध्रुव Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.प्रक्षिप_back(Dir.str());
  }
  std::error_code getRealPath(ध्रुव Twine &Path,
                              SmallVectorImpl<वर्ण> &Output) override;

रक्षित:
  शून्य printImpl(raw_ostream &OS, PrintType Type,
                 unsigned IndentLevel) ध्रुव override;

गुप्त:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(ध्रुव Twine &Path, SmallVectorImpl<वर्ण> &Storage) ध्रुव {
    जइ (!WD || !*WD)
      प्रत्यागम Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    प्रत्यागम Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ध्रुव Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    जइ (!FDOrErr)
      प्रत्यागम errorToErrorCode(FDOrErr.takeError());
    प्रत्यागम std::unique_ptr<File>(
        नूतन RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  संरचना WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(ध्रुव Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  जइ (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    प्रत्यागम EC;
  प्रत्यागम Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ध्रुव Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  प्रत्यागम openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_द्विमान(ध्रुव Twine &Name) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  प्रत्यागम openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ध्रुव {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  जइ (WD && *WD)
    प्रत्यागम std::string(WD->get().Specified);
  जइ (WD)
    प्रत्यागम WD->getError();

  SmallString<128> Dir;
  जइ (std::error_code EC = llvm::sys::fs::current_path(Dir))
    प्रत्यागम EC;
  प्रत्यागम std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(ध्रुव Twine &Path) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  जइ (!WD)
    प्रत्यागम llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  तार्किक IsDir;
  जइ (auto Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    प्रत्यागम Err;
  जइ (!IsDir)
    प्रत्यागम std::make_error_code(std::errc::not_a_directory);
  जइ (auto Err = llvm::sys::fs::real_path(Absolute, Resolved))
    प्रत्यागम Err;
  WD = WorkingDirectory{Absolute, Resolved};
  प्रत्यागम std::error_code();
}

std::error_code RealFileSystem::isLocal(ध्रुव Twine &Path, तार्किक &Result) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  प्रत्यागम llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(ध्रुव Twine &Path,
                                            SmallVectorImpl<वर्ण> &Output) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  प्रत्यागम llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

शून्य RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               unsigned IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  जइ (WD)
    OS << "own";
  अण्णहा
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  स्थिर IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(सच्च);
  sys::sandbox::violationIfEnabled();

  प्रत्यागम FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम std::make_unique<RealFileSystem>(मिच्छ);
}

नामाकाश {

वर्ग RealFSDirIter : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

सार्वजनिक:
  RealFSDirIter(ध्रुव Twine &Path, std::error_code &EC) {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    जइ (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    auto BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    प्रत्यागम EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ध्रुव Twine &Dir,
                                             std::error_code &EC) {
  auto BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  प्रत्यागम directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.प्रक्षिप_back(std::move(BaseFS));
}

शून्य OverlayFileSystem::प्रक्षिप_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.प्रक्षिप_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(ध्रुव Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  प्रति (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    जइ (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      प्रत्यागम Status;
  }
  प्रत्यागम make_error_code(llvm::errc::no_such_file_or_directory);
}

तार्किक OverlayFileSystem::exists(ध्रुव Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  प्रति (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    जइ ((*I)->exists(Path))
      प्रत्यागम सच्च;
  }
  प्रत्यागम मिच्छ;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ध्रुव llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  प्रति (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    auto Result = (*I)->openFileForRead(Path);
    जइ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      प्रत्यागम Result;
  }
  प्रत्यागम make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ध्रुव {
  // All file systems are synchronized, just take the first working directory.
  प्रत्यागम FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(ध्रुव Twine &Path) {
  प्रति (auto &FS : FSList)
    जइ (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      प्रत्यागम EC;
  प्रत्यागम {};
}

std::error_code OverlayFileSystem::isLocal(ध्रुव Twine &Path, तार्किक &Result) {
  प्रति (auto &FS : FSList)
    जइ (FS->exists(Path))
      प्रत्यागम FS->isLocal(Path, Result);
  प्रत्यागम errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(ध्रुव Twine &Path,
                                               SmallVectorImpl<वर्ण> &Output) {
  प्रति (ध्रुव auto &FS : FSList)
    जइ (FS->exists(Path))
      प्रत्यागम FS->getRealPath(Path, Output);
  प्रत्यागम errc::no_such_file_or_directory;
}

शून्य OverlayFileSystem::getDirectoryContentRealSources(
    ध्रुव Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  प्रति (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

शून्य OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  प्रति (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

शून्य OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  unsigned IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  जइ (Type == PrintType::Summary)
    प्रत्यागम;

  जइ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  प्रति (ध्रुव auto &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = स्वभाव;

नामाकाश {

/// Combines and deduplicates directory entries across multiple file systems.
वर्ग CombiningDirIterImpl : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  using FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  SmallVector<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::StringSet<> SeenNames;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_code incrementIter(तार्किक IsFirstTime) {
    जाव (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.निष्कासय_back();
      जइ (CurrentDirIter != directory_iterator())
        भङ्ग; // found
    }

    जइ (IsFirstTime && CurrentDirIter == directory_iterator())
      प्रत्यागम errc::no_such_file_or_directory;
    प्रत्यागम {};
  }

  std::error_code incrementDirIter(तार्किक IsFirstTime) {
    प्रतिज्ञा((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    जइ (!IsFirstTime)
      CurrentDirIter.increment(EC);
    जइ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    प्रत्यागम EC;
  }

  std::error_code incrementImpl(तार्किक IsFirstTime) {
    जाव (सच्च) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      जइ (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        प्रत्यागम EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      जइ (SeenNames.निवेशय(Name).second)
        प्रत्यागम EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

सार्वजनिक:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    प्रति (ध्रुव auto &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      जइ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        प्रत्यागम;
      }
      जइ (!FEC)
        IterList.प्रक्षिप_back(Iter);
    }
    EC = incrementImpl(सच्च);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(सच्च);
  }

  std::error_code increment() override { प्रत्यागम incrementImpl(मिच्छ); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ध्रुव Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  जइ (EC)
    प्रत्यागम {};
  प्रत्यागम Combined;
}

शून्य ProxyFileSystem::anchor() {}

नामाकाश llvm {
नामाकाश vfs {

नामाकाश detail {

गणना In_Memory_ग्रन्थिः_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
वर्ग In_Memory_ग्रन्थिः {
  In_Memory_ग्रन्थिः_Kind Kind;
  std::string FileName;

सार्वजनिक:
  In_Memory_ग्रन्थिः(llvm::StringRef FileName, In_Memory_ग्रन्थिः_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  आभास ~In_Memory_ग्रन्थिः() = स्वभाव;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  आभास Status getStatus(ध्रुव Twine &RequestedName) ध्रुव = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ध्रुव { प्रत्यागम FileName; }
  In_Memory_ग्रन्थिः_Kind getKind() ध्रुव { प्रत्यागम Kind; }
  आभास std::string toString(unsigned Indent) ध्रुव = 0;
};

वर्ग InMemoryFile : सार्वजनिक In_Memory_ग्रन्थिः {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

सार्वजनिक:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_ग्रन्थिः(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(ध्रुव Twine &RequestedName) ध्रुव override {
    प्रत्यागम Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() ध्रुव { प्रत्यागम Buffer.get(); }

  std::string toString(unsigned Indent) ध्रुव override {
    प्रत्यागम (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  स्थिर तार्किक classof(ध्रुव In_Memory_ग्रन्थिः *N) {
    प्रत्यागम N->getKind() == IME_File;
  }
};

नामाकाश {

वर्ग InMemoryHardLink : सार्वजनिक In_Memory_ग्रन्थिः {
  ध्रुव InMemoryFile &ResolvedFile;

सार्वजनिक:
  InMemoryHardLink(StringRef Path, ध्रुव InMemoryFile &ResolvedFile)
      : In_Memory_ग्रन्थिः(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ध्रुव InMemoryFile &getResolvedFile() ध्रुव { प्रत्यागम ResolvedFile; }

  Status getStatus(ध्रुव Twine &RequestedName) ध्रुव override {
    प्रत्यागम ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(unsigned Indent) ध्रुव override {
    प्रत्यागम std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  स्थिर तार्किक classof(ध्रुव In_Memory_ग्रन्थिः *N) {
    प्रत्यागम N->getKind() == IME_HardLink;
  }
};

वर्ग InMemorySymbolicLink : सार्वजनिक In_Memory_ग्रन्थिः {
  std::string TargetPath;
  Status Stat;

सार्वजनिक:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_ग्रन्थिः(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(unsigned Indent) ध्रुव override {
    प्रत्यागम std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(ध्रुव Twine &RequestedName) ध्रुव override {
    प्रत्यागम Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() ध्रुव { प्रत्यागम TargetPath; }

  स्थिर तार्किक classof(ध्रुव In_Memory_ग्रन्थिः *N) {
    प्रत्यागम N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
वर्ग InMemoryFileAdaptor : सार्वजनिक File {
  ध्रुव InMemoryFile &ग्रन्थिः;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

सार्वजनिक:
  explicit InMemoryFileAdaptor(ध्रुव InMemoryFile &ग्रन्थिः,
                               std::string RequestedName)
      : ग्रन्थिः(ग्रन्थिः), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    प्रत्यागम ग्रन्थिः.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(ध्रुव Twine &Name, int64_t FileSize, तार्किक RequiresNullTerminator,
            तार्किक IsVolatile) override {
    llvm::MemoryBuffer *Buf = ग्रन्थिः.getBuffer();
    प्रत्यागम llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { प्रत्यागम {}; }

  शून्य setPath(ध्रुव Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

वर्ग InMemoryDirectory : सार्वजनिक In_Memory_ग्रन्थिः {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_ग्रन्थिः>, std::less<>> Entries;

सार्वजनिक:
  InMemoryDirectory(Status Stat)
      : In_Memory_ग्रन्थिः(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(ध्रुव Twine &RequestedName) ध्रुव override {
    प्रत्यागम Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ध्रुव { प्रत्यागम Stat.getUniqueID(); }

  In_Memory_ग्रन्थिः *getChild(StringRef Name) ध्रुव {
    auto I = Entries.find(Name);
    जइ (I != Entries.end())
      प्रत्यागम I->second.get();
    प्रत्यागम nullptr;
  }

  In_Memory_ग्रन्थिः *addChild(StringRef Name, std::unique_ptr<In_Memory_ग्रन्थिः> Child) {
    प्रत्यागम Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ध्रुव { प्रत्यागम Entries.begin(); }
  const_iterator end() ध्रुव { प्रत्यागम Entries.end(); }

  std::string toString(unsigned Indent) ध्रुव override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    प्रति (ध्रुव auto &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    प्रत्यागम Result;
  }

  स्थिर तार्किक classof(ध्रुव In_Memory_ग्रन्थिः *N) {
    प्रत्यागम N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
स्थिर sys::fs::UniqueID getUniqueID(hash_code Hash) {
  प्रत्यागम sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
स्थिर sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  प्रत्यागम getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
स्थिर sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  प्रत्यागम getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_ग्रन्थिः_Info::makeStatus() ध्रुव {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  प्रत्यागम Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(तार्किक UseNormalizedPaths)
    : मूलम्_2(नूतन detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = स्वभाव;

std::string InMemoryFileSystem::toString() ध्रुव {
  प्रत्यागम मूलम्_2->toString(/*Indent=*/0);
}

तार्किक InMemoryFileSystem::addFile(ध्रुव Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_ग्रन्थिः_Fn Make_ग्रन्थिः) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  जइ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सच्च);

  जइ (Path.empty())
    प्रत्यागम मिच्छ;

  detail::InMemoryDirectory *Dir = मूलम्_2.get();
  auto I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  ध्रुव auto ResolvedUser = User.मानम्_or(0);
  ध्रुव auto ResolvedGroup = Group.मानम्_or(0);
  ध्रुव auto ResolvedType = Type.मानम्_or(sys::fs::file_type::regular_file);
  ध्रुव auto ResolvedPerms = Perms.मानम्_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ध्रुव auto NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  जाव (सच्च) {
    Name = *I;
    ++I;
    जइ (I == E)
      भङ्ग;
    detail::In_Memory_ग्रन्थिः *ग्रन्थिः = Dir->getChild(Name);
    जइ (!ग्रन्थिः) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      अनुवर्त;
    }
    // Creating file under another file.
    जइ (!isa<detail::InMemoryDirectory>(ग्रन्थिः))
      प्रत्यागम मिच्छ;
    Dir = cast<detail::InMemoryDirectory>(ग्रन्थिः);
  }
  detail::In_Memory_ग्रन्थिः *ग्रन्थिः = Dir->getChild(Name);
  जइ (!ग्रन्थिः) {
    Dir->addChild(Name,
                  Make_ग्रन्थिः({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    प्रत्यागम सच्च;
  }
  जइ (isa<detail::InMemoryDirectory>(ग्रन्थिः))
    प्रत्यागम ResolvedType == sys::fs::file_type::directory_file;

  प्रतिज्ञा((isa<detail::InMemoryFile>(ग्रन्थिः) ||
          isa<detail::InMemoryHardLink>(ग्रन्थिः)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  जइ (auto *Link = dyn_cast<detail::InMemoryHardLink>(ग्रन्थिः)) {
    प्रत्यागम Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  प्रत्यागम cast<detail::InMemoryFile>(ग्रन्थिः)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

तार्किक InMemoryFileSystem::addFile(ध्रुव Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  प्रत्यागम addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_ग्रन्थिः_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_ग्रन्थिः> {
                   Status Stat = NNI.makeStatus();
                   जइ (Stat.getType() == sys::fs::file_type::directory_file)
                     प्रत्यागम std::make_unique<detail::InMemoryDirectory>(Stat);
                   प्रत्यागम std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

तार्किक InMemoryFileSystem::addFileNoOwn(
    ध्रुव Twine &P, time_t ModificationTime,
    ध्रुव llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  प्रत्यागम addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_ग्रन्थिः_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_ग्रन्थिः> {
                   Status Stat = NNI.makeStatus();
                   जइ (Stat.getType() == sys::fs::file_type::directory_file)
                     प्रत्यागम std::make_unique<detail::InMemoryDirectory>(Stat);
                   प्रत्यागम std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_ग्रन्थिः_Or_Error
InMemoryFileSystem::lookup_ग्रन्थिः(ध्रुव Twine &P, तार्किक FollowFinalSymlink,
                               size_t SymlinkDepth) ध्रुव {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  जइ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सच्च);

  ध्रुव detail::InMemoryDirectory *Dir = मूलम्_2.get();
  जइ (Path.empty())
    प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(Path, Dir);

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  जाव (सच्च) {
    detail::In_Memory_ग्रन्थिः *ग्रन्थिः = Dir->getChild(*I);
    ++I;
    जइ (!ग्रन्थिः)
      प्रत्यागम errc::no_such_file_or_directory;

    जइ (auto Symlink = dyn_cast<detail::InMemorySymbolicLink>(ग्रन्थिः)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      जइ (I == E && !FollowFinalSymlink)
        प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(Path, Symlink);

      जइ (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        प्रत्यागम errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      जइ (std::error_code EC = makeAbsolute(TargetPath))
        प्रत्यागम EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      auto Target =
          lookup_ग्रन्थिः(TargetPath, /*FollowFinalSymlink=*/सच्च, SymlinkDepth + 1);
      जइ (!Target || I == E)
        प्रत्यागम Target;

      जइ (!isa<detail::InMemoryDirectory>(*Target))
        प्रत्यागम errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      अनुवर्त;
    }

    // Return the file if it's at the end of the path.
    जइ (auto File = dyn_cast<detail::InMemoryFile>(ग्रन्थिः)) {
      जइ (I == E)
        प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(Path, File);
      प्रत्यागम errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    जइ (auto File = dyn_cast<detail::InMemoryHardLink>(ग्रन्थिः)) {
      जइ (I == E)
        प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(Path, &File->getResolvedFile());
      प्रत्यागम errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(ग्रन्थिः);
    जइ (I == E)
      प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(Path, Dir);
  }
}

तार्किक InMemoryFileSystem::addHardLink(ध्रुव Twine &NewLink,
                                     ध्रुव Twine &Target) {
  auto New_Link_ग्रन्थिः = lookup_ग्रन्थिः(NewLink, /*FollowFinalSymlink=*/मिच्छ);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  auto Target_ग्रन्थिः = lookup_ग्रन्थिः(Target, /*FollowFinalSymlink=*/सच्च);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  जइ (!Target_ग्रन्थिः || New_Link_ग्रन्थिः || !isa<detail::InMemoryFile>(*Target_ग्रन्थिः))
    प्रत्यागम मिच्छ;
  प्रत्यागम addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_ग्रन्थिः_Info NNI) {
                   प्रत्यागम std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_ग्रन्थिः));
                 });
}

तार्किक InMemoryFileSystem::addSymbolicLink(
    ध्रुव Twine &NewLink, ध्रुव Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  auto New_Link_ग्रन्थिः = lookup_ग्रन्थिः(NewLink, /*FollowFinalSymlink=*/मिच्छ);
  जइ (New_Link_ग्रन्थिः)
    प्रत्यागम मिच्छ;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  प्रत्यागम addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_ग्रन्थिः_Info NNI) {
                   प्रत्यागम std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(ध्रुव Twine &Path) {
  auto ग्रन्थिः = lookup_ग्रन्थिः(Path, /*FollowFinalSymlink=*/सच्च);
  जइ (ग्रन्थिः)
    प्रत्यागम (*ग्रन्थिः)->getStatus(Path);
  प्रत्यागम ग्रन्थिः.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(ध्रुव Twine &Path) {
  auto ग्रन्थिः = lookup_ग्रन्थिः(Path,/*FollowFinalSymlink=*/सच्च);
  जइ (!ग्रन्थिः)
    प्रत्यागम ग्रन्थिः.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  जइ (auto *F = dyn_cast<detail::InMemoryFile>(*ग्रन्थिः))
    प्रत्यागम std::unique_ptr<File>(
        नूतन detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  प्रत्यागम make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
वर्ग InMemoryFileSystem::DirIterator : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  ध्रुव InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  शून्य setCurrentEntry() {
    जइ (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      विकल्प (I->second->getKind()) {
      पक्ष detail::IME_File:
      पक्ष detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        भङ्ग;
      पक्ष detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        भङ्ग;
      पक्ष detail::IME_SymbolicLink:
        जइ (auto SymlinkTarget =
                FS->lookup_ग्रन्थिः(Path, /*FollowFinalSymlink=*/सच्च)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        भङ्ग;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } अण्णहा {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

सार्वजनिक:
  DirIterator() = स्वभाव;

  DirIterator(ध्रुव InMemoryFileSystem *FS,
              ध्रुव detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    प्रत्यागम {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(ध्रुव Twine &Dir,
                                                 std::error_code &EC) {
  auto ग्रन्थिः = lookup_ग्रन्थिः(Dir, /*FollowFinalSymlink=*/सच्च);
  जइ (!ग्रन्थिः) {
    EC = ग्रन्थिः.getError();
    प्रत्यागम directory_iterator(std::make_shared<DirIterator>());
  }

  जइ (auto *Dir_ग्रन्थिः = dyn_cast<detail::InMemoryDirectory>(*ग्रन्थिः))
    प्रत्यागम directory_iterator(
        std::make_shared<DirIterator>(एतत्, *Dir_ग्रन्थिः, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  प्रत्यागम directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(ध्रुव Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  जइ (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सच्च);

  जइ (!Path.empty())
    WorkingDirectory = std::string(Path);
  प्रत्यागम {};
}

std::error_code InMemoryFileSystem::getRealPath(ध्रुव Twine &Path,
                                                SmallVectorImpl<वर्ण> &Output) {
  auto CWD = getCurrentWorkingDirectory();
  जइ (!CWD || CWD->empty())
    प्रत्यागम errc::operation_not_permitted;
  Path.toVector(Output);
  जइ (auto EC = makeAbsolute(Output))
    प्रत्यागम EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/सच्च);
  प्रत्यागम {};
}

std::error_code InMemoryFileSystem::isLocal(ध्रुव Twine &Path, तार्किक &Result) {
  Result = मिच्छ;
  प्रत्यागम {};
}

शून्य InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   unsigned IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

नामाकाश {

स्थिर llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  ध्रुव size_t n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  जइ (n != static_cast<size_t>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  प्रत्यागम style;
}

/// Removes leading "./" as well as path components like ".." and ".".
स्थिर llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/सच्च, style);
  प्रत्यागम result;
}

/// Whether the error and entry specify a file/directory that was not found.
स्थिर तार्किक isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  जइ (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    प्रत्यागम मिच्छ;
  प्रत्यागम EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  प्रतिज्ञा(ExternalFS && "RedirectingFileSystem requires an external FS");
  जइ (auto ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
वर्ग llvm::vfs::RedirectingFSDirIterImpl
    : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(तार्किक IsFirstTime) {
    प्रतिज्ञा((IsFirstTime || Current != End) && "cannot iterate past end");
    जइ (!IsFirstTime)
      ++Current;
    जइ (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      विकल्प ((*Current)->getKind()) {
      पक्ष RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      पक्ष RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        भङ्ग;
      पक्ष RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        भङ्ग;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } अण्णहा {
      CurrentEntry = directory_entry();
    }
    प्रत्यागम {};
  };

सार्वजनिक:
  RedirectingFSDirIterImpl(
      ध्रुव Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/सच्च);
  }

  std::error_code increment() override {
    प्रत्यागम incrementImpl(/*IsFirstTime=*/मिच्छ);
  }
};

नामाकाश {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
वर्ग RedirectingFSDirRemapIterImpl : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

सार्वजनिक:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    जइ (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  शून्य setCurrentEntry() {
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
    जइ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    अण्णहा
      CurrentEntry = directory_entry();
    प्रत्यागम EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ध्रुव {
  प्रत्यागम WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(ध्रुव Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  जइ (!exists(Path))
    प्रत्यागम errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  जइ (std::error_code EC = makeAbsolute(AbsolutePath))
    प्रत्यागम EC;
  WorkingDirectory = std::string(AbsolutePath);
  प्रत्यागम {};
}

std::error_code RedirectingFileSystem::isLocal(ध्रुव Twine &Path_,
                                               तार्किक &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  जइ (makeAbsolute(Path))
    प्रत्यागम {};

  प्रत्यागम ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<वर्ण> &Path) ध्रुव {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  जइ (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    प्रत्यागम {};

  auto WorkingDir = getCurrentWorkingDirectory();
  जइ (!WorkingDir)
    प्रत्यागम WorkingDir.getError();

  प्रत्यागम makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<वर्ण> &Path) ध्रुव {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  जइ (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    प्रत्यागम std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  जइ (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } अण्णहा {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    जइ (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  जइ (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  प्रत्यागम {};
}

directory_iterator RedirectingFileSystem::dir_begin(ध्रुव Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  जइ (EC)
    प्रत्यागम {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जइ (!Result) {
    जइ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      प्रत्यागम ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    प्रत्यागम {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  जइ (!S) {
    जइ (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      प्रत्यागम ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    प्रत्यागम {};
  }

  जइ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    प्रत्यागम {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  जइ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    जइ (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } अण्णहा {
    auto DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  जइ (RedirectEC) {
    जइ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      प्रत्यागम {};
    }
    RedirectIter = {};
  }

  जइ (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    प्रत्यागम RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  जइ (ExternalEC) {
    जइ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      प्रत्यागम {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  विकल्प (Redirection) {
  पक्ष RedirectKind::Fallthrough:
    Iters.प्रक्षिप_back(ExternalIter);
    Iters.प्रक्षिप_back(RedirectIter);
    भङ्ग;
  पक्ष RedirectKind::Fallback:
    Iters.प्रक्षिप_back(RedirectIter);
    Iters.प्रक्षिप_back(ExternalIter);
    भङ्ग;
  स्वभाव:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  जइ (EC)
    प्रत्यागम {};
  प्रत्यागम Combined;
}

शून्य RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ध्रुव {
  प्रत्यागम OverlayFileDir;
}

शून्य RedirectingFileSystem::setFallthrough(तार्किक Fallthrough) {
  जइ (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } अण्णहा {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

शून्य RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() ध्रुव {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  प्रति (ध्रुव auto &मूलम्_2 : Roots)
    R.प्रक्षिप_back(मूलम्_2->getName());
  प्रत्यागम R;
}

शून्य RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      unsigned IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  जइ (Type == PrintType::Summary)
    प्रत्यागम;

  प्रति (ध्रुव auto &मूलम्_2 : Roots)
    printEntry(OS, मूलम्_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

शून्य RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       unsigned IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  विकल्प (E->getKind()) {
  पक्ष EK_Directory: {
    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    प्रति (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    भङ्ग;
  }
  पक्ष EK_DirectoryRemap:
  पक्ष EK_File: {
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    विकल्प (RE->getUseName()) {
    पक्ष NK_NotSet:
      भङ्ग;
    पक्ष NK_External:
      OS << " (UseExternalName: true)";
      भङ्ग;
    पक्ष NK_Virtual:
      OS << " (UseExternalName: false)";
      भङ्ग;
    }
    OS << "\n";
    भङ्ग;
  }
  }
}

शून्य RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  जइ (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
वर्ग llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  शून्य error(yaml::ग्रन्थिः *N, ध्रुव Twine &सन्देशः_2) { Stream.printError(N, सन्देशः_2); }

  // false on error
  तार्किक parseScalarString(yaml::ग्रन्थिः *N, StringRef &Result,
                         SmallVectorImpl<वर्ण> &Storage) {
    ध्रुव auto *S = dyn_cast_if_present<yaml::Scalar_ग्रन्थिः>(N);

    जइ (!S) {
      error(N, "expected string");
      प्रत्यागम मिच्छ;
    }
    Result = S->get_मानम्(Storage);
    प्रत्यागम सच्च;
  }

  // false on error
  तार्किक parseScalarBool(yaml::ग्रन्थिः *N, तार्किक &Result) {
    SmallString<5> Storage;
    StringRef मानम्;
    जइ (!parseScalarString(N, मानम्, Storage))
      प्रत्यागम मिच्छ;

    जइ (मानम्.equals_insensitive("true") || मानम्.equals_insensitive("on") ||
        मानम्.equals_insensitive("yes") || मानम् == "1") {
      Result = सच्च;
      प्रत्यागम सच्च;
    } अण्णहा जइ (मानम्.equals_insensitive("false") ||
               मानम्.equals_insensitive("off") ||
               मानम्.equals_insensitive("no") || मानम् == "0") {
      Result = मिच्छ;
      प्रत्यागम सच्च;
    }

    error(N, "expected boolean value");
    प्रत्यागम मिच्छ;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::ग्रन्थिः *N) {
    SmallString<12> Storage;
    StringRef मानम्;
    जइ (!parseScalarString(N, मानम्, Storage))
      प्रत्यागम std::nullopt;

    जइ (मानम्.equals_insensitive("fallthrough")) {
      प्रत्यागम RedirectingFileSystem::RedirectKind::Fallthrough;
    } अण्णहा जइ (मानम्.equals_insensitive("fallback")) {
      प्रत्यागम RedirectingFileSystem::RedirectKind::Fallback;
    } अण्णहा जइ (मानम्.equals_insensitive("redirect-only")) {
      प्रत्यागम RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    प्रत्यागम std::nullopt;
  }

  std::optional<RedirectingFileSystem::मूलम्_Relative_Kind>
  parse_मूलम्_Relative_Kind(yaml::ग्रन्थिः *N) {
    SmallString<12> Storage;
    StringRef मानम्;
    जइ (!parseScalarString(N, मानम्, Storage))
      प्रत्यागम std::nullopt;
    जइ (मानम्.equals_insensitive("cwd")) {
      प्रत्यागम RedirectingFileSystem::मूलम्_Relative_Kind::CWD;
    } अण्णहा जइ (मानम्.equals_insensitive("overlay-dir")) {
      प्रत्यागम RedirectingFileSystem::मूलम्_Relative_Kind::OverlayDir;
    }
    प्रत्यागम std::nullopt;
  }

  संरचना कुञ्चिका_Status {
    तार्किक Required;
    तार्किक Seen = मिच्छ;

    कुञ्चिका_Status(तार्किक Required = मिच्छ) : Required(Required) {}
  };

  using कुञ्चिका_Status_Pair = std::pair<StringRef, कुञ्चिका_Status>;

  // false on error
  तार्किक check_Duplicate_Or_Unknown_कुञ्चिका(yaml::ग्रन्थिः *कुञ्चिका_ग्रन्थिः, StringRef कुञ्चिका,
                                  DenseMap<StringRef, कुञ्चिका_Status> &कुञ्चिकाः_2) {
    auto It = कुञ्चिकाः_2.find(कुञ्चिका);
    जइ (It == कुञ्चिकाः_2.end()) {
      error(कुञ्चिका_ग्रन्थिः, "unknown key");
      प्रत्यागम मिच्छ;
    }
    कुञ्चिका_Status &S = It->second;
    जइ (S.Seen) {
      error(कुञ्चिका_ग्रन्थिः, Twine("duplicate key '") + कुञ्चिका + "'");
      प्रत्यागम मिच्छ;
    }
    S.Seen = सच्च;
    प्रत्यागम सच्च;
  }

  // false on error
  तार्किक check_Missing_कुञ्चिकाः(yaml::ग्रन्थिः *Obj, DenseMap<StringRef, कुञ्चिका_Status> &कुञ्चिकाः_2) {
    प्रति (ध्रुव auto &I : कुञ्चिकाः_2) {
      जइ (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        प्रत्यागम मिच्छ;
      }
    }
    प्रत्यागम सच्च;
  }

सार्वजनिक:
  स्थिर RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    जइ (!ParentEntry) { // Look for a existent root
      प्रति (ध्रुव auto &मूलम्_2 : FS->Roots) {
        जइ (Name == मूलम्_2->getName()) {
          ParentEntry = मूलम्_2.get();
          प्रत्यागम ParentEntry;
        }
      }
    } अण्णहा { // Advance to the next component
      auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      प्रति (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        auto *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        जइ (DirContent && Name == Content->getName())
          प्रत्यागम DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    जइ (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.प्रक्षिप_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      प्रत्यागम ParentEntry;
    }

    auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    प्रत्यागम DE->getLastContent();
  }

गुप्त:
  शून्य unique_Overlay_वृक्षः(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    विकल्प (SrcE->getKind()) {
    पक्ष RedirectingFileSystem::EK_Directory: {
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      जइ (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      प्रति (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_वृक्षः(FS, SubEntry.get(), NewParentE);
      भङ्ग;
    }
    पक्ष RedirectingFileSystem::EK_DirectoryRemap: {
      प्रतिज्ञा(NewParentE && "Parent entry must exist");
      auto *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      भङ्ग;
    }
    पक्ष RedirectingFileSystem::EK_File: {
      प्रतिज्ञा(NewParentE && "Parent entry must exist");
      auto *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      भङ्ग;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::ग्रन्थिः *N, RedirectingFileSystem *FS, तार्किक Is_मूलम्_Entry) {
    auto *M = dyn_cast<yaml::Mapping_ग्रन्थिः>(N);
    जइ (!M) {
      error(N, "expected mapping node for file or directory entry");
      प्रत्यागम nullptr;
    }

    कुञ्चिका_Status_Pair Fields[] = {
        कुञ्चिका_Status_Pair("name", सच्च),
        कुञ्चिका_Status_Pair("type", सच्च),
        कुञ्चिका_Status_Pair("contents", मिच्छ),
        कुञ्चिका_Status_Pair("external-contents", मिच्छ),
        कुञ्चिका_Status_Pair("use-external-name", मिच्छ),
    };

    DenseMap<StringRef, कुञ्चिका_Status> कुञ्चिकाः_2(std::begin(Fields), std::end(Fields));

    गणना { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::ग्रन्थिः *Name_मानम्_ग्रन्थिः = nullptr;
    auto UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    प्रति (auto &I : *M) {
      StringRef कुञ्चिका;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      जइ (!parseScalarString(I.get_कुञ्चिका(), कुञ्चिका, Buffer))
        प्रत्यागम nullptr;

      जइ (!check_Duplicate_Or_Unknown_कुञ्चिका(I.get_कुञ्चिका(), कुञ्चिका, कुञ्चिकाः_2))
        प्रत्यागम nullptr;

      StringRef मानम्;
      जइ (कुञ्चिका == "name") {
        जइ (!parseScalarString(I.get_मानम्(), मानम्, Buffer))
          प्रत्यागम nullptr;

        Name_मानम्_ग्रन्थिः = I.get_मानम्();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(मानम्).str();
      } अण्णहा जइ (कुञ्चिका == "type") {
        जइ (!parseScalarString(I.get_मानम्(), मानम्, Buffer))
          प्रत्यागम nullptr;
        जइ (मानम् == "file")
          Kind = RedirectingFileSystem::EK_File;
        अण्णहा जइ (मानम् == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        अण्णहा जइ (मानम् == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        अण्णहा {
          error(I.get_मानम्(), "unknown value for 'type'");
          प्रत्यागम nullptr;
        }
      } अण्णहा जइ (कुञ्चिका == "contents") {
        जइ (ContentsField != CF_NotSet) {
          error(I.get_कुञ्चिका(),
                "entry already has 'contents' or 'external-contents'");
          प्रत्यागम nullptr;
        }
        ContentsField = CF_List;
        auto *Contents = dyn_cast_if_present<yaml::Sequence_ग्रन्थिः>(I.get_मानम्());
        जइ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_मानम्(), "expected array");
          प्रत्यागम nullptr;
        }

        प्रति (auto &I : *Contents) {
          जइ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ मिच्छ))
            EntryArrayContents.प्रक्षिप_back(std::move(E));
          अण्णहा
            प्रत्यागम nullptr;
        }
      } अण्णहा जइ (कुञ्चिका == "external-contents") {
        जइ (ContentsField != CF_NotSet) {
          error(I.get_कुञ्चिका(),
                "entry already has 'contents' or 'external-contents'");
          प्रत्यागम nullptr;
        }
        ContentsField = CF_External;
        जइ (!parseScalarString(I.get_मानम्(), मानम्, Buffer))
          प्रत्यागम nullptr;

        SmallString<256> FullPath;
        जइ (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          प्रतिज्ञा(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = मानम्;
          जइ (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            प्रत्यागम nullptr;
          }
          FullPath = AbsFullPath;
        } अण्णहा {
          FullPath = मानम्;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } अण्णहा जइ (कुञ्चिका == "use-external-name") {
        तार्किक Val;
        जइ (!parseScalarBool(I.get_मानम्(), Val))
          प्रत्यागम nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } अण्णहा {
        llvm_unreachable("key missing from Keys");
      }
    }

    जइ (Stream.failed())
      प्रत्यागम nullptr;

    // check for missing keys
    जइ (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      प्रत्यागम nullptr;
    }
    जइ (!check_Missing_कुञ्चिकाः(N, कुञ्चिकाः_2))
      प्रत्यागम nullptr;

    // check invalid configuration
    जइ (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      प्रत्यागम nullptr;
    }

    जइ (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      प्रत्यागम nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    जइ (Is_मूलम्_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      जइ (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } अण्णहा जइ (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } अण्णहा {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        जइ (FS->मूलम्_Relative ==
            RedirectingFileSystem::मूलम्_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          प्रतिज्ञा(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } अण्णहा {
          EC = FS->makeAbsolute(Name);
        }
        जइ (EC) {
          प्रतिज्ञा(Name_मानम्_ग्रन्थिः && "Name presence should be checked earlier");
          error(
              Name_मानम्_ग्रन्थिः,
              "entry with relative path at the root level is not discoverable");
          प्रत्यागम nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      जइ (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t मूलम्_Path_Len = sys::path::मूलम्_path(Trimmed, path_style).size();
    जाव (Trimmed.size() > मूलम्_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    विकल्प (Kind) {
    पक्ष RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      भङ्ग;
    पक्ष RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      भङ्ग;
    पक्ष RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      भङ्ग;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    जइ (Parent.empty())
      प्रत्यागम Result;

    // if 'name' contains multiple components, create implicit directory entries
    प्रति (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.प्रक्षिप_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    प्रत्यागम Result;
  }

सार्वजनिक:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  तार्किक parse(yaml::ग्रन्थिः *मूलम्_2, RedirectingFileSystem *FS) {
    auto *शिखरम् = dyn_cast<yaml::Mapping_ग्रन्थिः>(मूलम्_2);
    जइ (!शिखरम्) {
      error(मूलम्_2, "expected mapping node");
      प्रत्यागम मिच्छ;
    }

    कुञ्चिका_Status_Pair Fields[] = {
        कुञ्चिका_Status_Pair("version", सच्च),
        कुञ्चिका_Status_Pair("case-sensitive", मिच्छ),
        कुञ्चिका_Status_Pair("use-external-names", मिच्छ),
        कुञ्चिका_Status_Pair("root-relative", मिच्छ),
        कुञ्चिका_Status_Pair("overlay-relative", मिच्छ),
        कुञ्चिका_Status_Pair("fallthrough", मिच्छ),
        कुञ्चिका_Status_Pair("redirecting-with", मिच्छ),
        कुञ्चिका_Status_Pair("roots", सच्च),
    };

    DenseMap<StringRef, कुञ्चिका_Status> कुञ्चिकाः_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> मूलम्_Entries;

    // Parse configuration and 'roots'
    प्रति (auto &I : *शिखरम्) {
      SmallString<10> कुञ्चिका_Buffer;
      StringRef कुञ्चिका;
      जइ (!parseScalarString(I.get_कुञ्चिका(), कुञ्चिका, कुञ्चिका_Buffer))
        प्रत्यागम मिच्छ;

      जइ (!check_Duplicate_Or_Unknown_कुञ्चिका(I.get_कुञ्चिका(), कुञ्चिका, कुञ्चिकाः_2))
        प्रत्यागम मिच्छ;

      जइ (कुञ्चिका == "roots") {
        auto *Roots = dyn_cast_if_present<yaml::Sequence_ग्रन्थिः>(I.get_मानम्());
        जइ (!Roots) {
          error(I.get_मानम्(), "expected array");
          प्रत्यागम मिच्छ;
        }

        प्रति (auto &I : *Roots) {
          जइ (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ सच्च))
            मूलम्_Entries.प्रक्षिप_back(std::move(E));
          अण्णहा
            प्रत्यागम मिच्छ;
        }
      } अण्णहा जइ (कुञ्चिका == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        जइ (!parseScalarString(I.get_मानम्(), VersionString, Storage))
          प्रत्यागम मिच्छ;
        पूर्णाङ्क Version;
        जइ (VersionString.getAsInteger<पूर्णाङ्क>(10, Version)) {
          error(I.get_मानम्(), "expected integer");
          प्रत्यागम मिच्छ;
        }
        जइ (Version < 0) {
          error(I.get_मानम्(), "invalid version number");
          प्रत्यागम मिच्छ;
        }
        जइ (Version != 0) {
          error(I.get_मानम्(), "version mismatch, expected 0");
          प्रत्यागम मिच्छ;
        }
      } अण्णहा जइ (कुञ्चिका == "case-sensitive") {
        जइ (!parseScalarBool(I.get_मानम्(), FS->CaseSensitive))
          प्रत्यागम मिच्छ;
      } अण्णहा जइ (कुञ्चिका == "overlay-relative") {
        जइ (!parseScalarBool(I.get_मानम्(), FS->IsRelativeOverlay))
          प्रत्यागम मिच्छ;
      } अण्णहा जइ (कुञ्चिका == "use-external-names") {
        जइ (!parseScalarBool(I.get_मानम्(), FS->UseExternalNames))
          प्रत्यागम मिच्छ;
      } अण्णहा जइ (कुञ्चिका == "fallthrough") {
        जइ (कुञ्चिकाः_2["redirecting-with"].Seen) {
          error(I.get_मानम्(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          प्रत्यागम मिच्छ;
        }

        तार्किक ShouldFallthrough = मिच्छ;
        जइ (!parseScalarBool(I.get_मानम्(), ShouldFallthrough))
          प्रत्यागम मिच्छ;

        जइ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } अण्णहा {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } अण्णहा जइ (कुञ्चिका == "redirecting-with") {
        जइ (कुञ्चिकाः_2["fallthrough"].Seen) {
          error(I.get_मानम्(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          प्रत्यागम मिच्छ;
        }

        जइ (auto Kind = parseRedirectKind(I.get_मानम्())) {
          FS->Redirection = *Kind;
        } अण्णहा {
          error(I.get_मानम्(), "expected valid redirect kind");
          प्रत्यागम मिच्छ;
        }
      } अण्णहा जइ (कुञ्चिका == "root-relative") {
        जइ (auto Kind = parse_मूलम्_Relative_Kind(I.get_मानम्())) {
          FS->मूलम्_Relative = *Kind;
        } अण्णहा {
          error(I.get_मानम्(), "expected valid root-relative kind");
          प्रत्यागम मिच्छ;
        }
      } अण्णहा {
        llvm_unreachable("key missing from Keys");
      }
    }

    जइ (Stream.failed())
      प्रत्यागम मिच्छ;

    जइ (!check_Missing_कुञ्चिकाः(शिखरम्, कुञ्चिकाः_2))
      प्रत्यागम मिच्छ;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    प्रति (auto &E : मूलम्_Entries)
      unique_Overlay_वृक्षः(FS, E.get());

    प्रत्यागम सच्च;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<MemoryBuffer> Buffer,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, शून्य *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(Buffer->getMemBufferRef(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::ग्रन्थिः *मूलम्_2 = DI->get_मूलम्();
  जइ (DI == Stream.end() || !मूलम्_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    प्रत्यागम nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      नूतन RedirectingFileSystem(ExternalFS));

  जइ (!YAMLFilePath.empty()) {
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
    प्रतिज्ञा(!EC && "Overlay dir final path must be absolute");
    (शून्य)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  जइ (!P.parse(मूलम्_2, FS.get()))
    प्रत्यागम nullptr;

  प्रत्यागम FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    तार्किक UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      नूतन RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  प्रति (auto &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      auto EC = ExternalFS->makeAbsolute(From);
      (शून्य)EC;
      प्रतिज्ञा(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    जइ (ToEntry)
      अनुवर्त;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    प्रति (auto I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    प्रतिज्ञा(Parent && "File without a directory?");
    {
      auto EC = ExternalFS->makeAbsolute(To);
      (शून्य)EC;
      प्रतिज्ञा(!EC && "Could not make absolute path");
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

  प्रत्यागम FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  प्रतिज्ञा(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  जइ (auto *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

शून्य RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<वर्ण> &Result) ध्रुव {
  Result.clear();
  प्रति (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<वर्ण> &Path) ध्रुव {
  जइ (std::error_code EC = makeAbsolute(Path))
    प्रत्यागम EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  जइ (CanonicalPath.empty())
    प्रत्यागम make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  प्रत्यागम {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) ध्रुव {
  llvm::SmallString<128> CanonicalPath(Path);
  जइ (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    प्रत्यागम EC;

  // RedirectOnly means the VFS is always used.
  जइ (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = सच्च;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  प्रति (ध्रुव auto &मूलम्_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, मूलम्_2.get(), Entries);
    जइ (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = सच्च;
    जइ (Result) {
      Result->Parents = std::move(Entries);
      प्रत्यागम Result;
    }

    जइ (Result.getError() != llvm::errc::no_such_file_or_directory)
      प्रत्यागम Result;
  }
  प्रत्यागम make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) ध्रुव {
  प्रतिज्ञा(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  जइ (!FromName.empty()) {
    जइ (!pathComponentMatches(*Start, FromName))
      प्रत्यागम make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    जइ (Start == End) {
      // Match!
      प्रत्यागम LookupResult(From, Start, End);
    }
  }

  जइ (isa<RedirectingFileSystem::FileEntry>(From))
    प्रत्यागम make_error_code(llvm::errc::not_a_directory);

  जइ (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    प्रत्यागम LookupResult(From, Start, End);

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  प्रति (ध्रुव std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.प्रक्षिप_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    जइ (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      प्रत्यागम Result;
    Entries.निष्कासय_back();
  }

  प्रत्यागम make_error_code(llvm::errc::no_such_file_or_directory);
}

स्थिर Status getRedirectedFileStatus(ध्रुव Twine &OriginalPath,
                                      तार्किक UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  जइ (ExternalStatus.ExposesExternalVFSPath)
    प्रत्यागम ExternalStatus;

  Status S = ExternalStatus;
  जइ (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  अण्णहा
    S.ExposesExternalVFSPath = सच्च;
  प्रत्यागम S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    ध्रुव Twine &LookupPath, ध्रुव Twine &OriginalPath,
    ध्रुव RedirectingFileSystem::LookupResult &Result) {
  जइ (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    जइ (std::error_code EC = makeAbsolute(RemappedPath))
      प्रत्यागम EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    जइ (!S)
      प्रत्यागम S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    प्रत्यागम getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  auto *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  प्रत्यागम Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(ध्रुव Twine &LookupPath,
                                         ध्रुव Twine &OriginalPath) ध्रुव {
  auto Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  जइ (!Result || Result->ExposesExternalVFSPath)
    प्रत्यागम Result;
  प्रत्यागम Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(ध्रुव Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  जइ (std::error_code EC = makeAbsolute(Path))
    प्रत्यागम EC;

  जइ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    जइ (S)
      प्रत्यागम S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जइ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    जइ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      प्रत्यागम getExternalStatus(Path, OriginalPath);
    प्रत्यागम Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  जइ (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    प्रत्यागम getExternalStatus(Path, OriginalPath);
  }

  प्रत्यागम S;
}

तार्किक RedirectingFileSystem::exists(ध्रुव Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  जइ (makeAbsolute(Path))
    प्रत्यागम मिच्छ;

  जइ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    जइ (ExternalFS->exists(Path))
      प्रत्यागम सच्च;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जइ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    जइ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      प्रत्यागम ExternalFS->exists(Path);
    प्रत्यागम मिच्छ;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  जइ (!ExtRedirect) {
    प्रतिज्ञा(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    प्रत्यागम सच्च;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  जइ (makeAbsolute(RemappedPath))
    प्रत्यागम मिच्छ;

  जइ (ExternalFS->exists(RemappedPath))
    प्रत्यागम सच्च;

  जइ (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    प्रत्यागम ExternalFS->exists(Path);
  }

  प्रत्यागम मिच्छ;
}

नामाकाश {

/// Provide a file wrapper with an overriden status.
वर्ग FileWithFixedStatus : सार्वजनिक File {
  std::unique_ptr<File> InnerFile;
  Status S;

सार्वजनिक:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { प्रत्यागम S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(ध्रुव Twine &Name, int64_t FileSize, तार्किक RequiresNullTerminator,
            तार्किक IsVolatile) override {
    प्रत्यागम InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code close() override { प्रत्यागम InnerFile->close(); }

  शून्य setPath(ध्रुव Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, ध्रुव Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  जइ (!Result || (*Result)->status()->ExposesExternalVFSPath)
    प्रत्यागम Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  auto Name = F->get()->getName();
  जइ (Name && Name.get() != P.str())
    F->get()->setPath(P);
  प्रत्यागम F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ध्रुव Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  जइ (std::error_code EC = makeAbsolute(Path))
    प्रत्यागम EC;

  जइ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    auto F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    जइ (F)
      प्रत्यागम F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जइ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    जइ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      प्रत्यागम File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    प्रत्यागम Result.getError();
  }

  जइ (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    प्रत्यागम make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  जइ (std::error_code EC = makeAbsolute(RemappedPath))
    प्रत्यागम EC;

  auto *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  auto ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  जइ (!ExternalFile) {
    जइ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      प्रत्यागम File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    प्रत्यागम ExternalFile;
  }

  auto ExternalStatus = (*ExternalFile)->status();
  जइ (!ExternalStatus)
    प्रत्यागम ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  प्रत्यागम std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(ध्रुव Twine &OriginalPath,
                                   SmallVectorImpl<वर्ण> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  जइ (std::error_code EC = makeAbsolute(Path))
    प्रत्यागम EC;

  जइ (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    जइ (!EC)
      प्रत्यागम EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जइ (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    जइ (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      प्रत्यागम ExternalFS->getRealPath(Path, Output);
    प्रत्यागम Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  जइ (auto ExtRedirect = Result->getExternalRedirect()) {
    auto P = ExternalFS->getRealPath(*ExtRedirect, Output);
    जइ (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      प्रत्यागम ExternalFS->getRealPath(Path, Output);
    }
    प्रत्यागम P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  जइ (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    प्रत्यागम {};
  }
  प्रत्यागम llvm::errc::invalid_argument;
}

शून्य RedirectingFileSystem::getDirectoryContentRealSources(
    ध्रुव Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  जइ (makeAbsolute(Path))
    प्रत्यागम;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ध्रुव तार्किक ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जइ (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    जइ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    प्रत्यागम;
  }

  विकल्प (Result->E->getKind()) {
  पक्ष EK_File:
    प्रत्यागम;
  पक्ष EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    भङ्ग;
  पक्ष EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    जइ (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    भङ्ग;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  जइ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, शून्य *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  प्रत्यागम RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

स्थिर शून्य getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  auto Kind = SrcE->getKind();
  जइ (Kind == RedirectingFileSystem::EK_Directory) {
    auto *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    प्रतिज्ञा(DE && "Must be a directory");
    प्रति (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.प्रक्षिप_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.निष्कासय_back();
    }
    प्रत्यागम;
  }

  जइ (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    auto *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    प्रतिज्ञा(DR && "Must be a directory remap");
    SmallString<128> VPath;
    प्रति (auto &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.प्रक्षिप_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    प्रत्यागम;
  }

  प्रतिज्ञा(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  auto *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  प्रतिज्ञा(FE && "Must be a file");
  SmallString<128> VPath;
  प्रति (auto &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.प्रक्षिप_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

शून्य vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> मूलम्_Result = VFS.lookupPath("/");
  जइ (!मूलम्_Result)
    प्रत्यागम;
  SmallVector<StringRef, 8> Components;
  Components.प्रक्षिप_back("/");
  getVFSEntries(मूलम्_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  स्थिर std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  प्रत्यागम UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

शून्य YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             तार्किक IsDirectory) {
  प्रतिज्ञा(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  प्रतिज्ञा(sys::path::is_absolute(RealPath) && "real path not absolute");
  प्रतिज्ञा(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

शून्य YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/मिच्छ);
}

शून्य YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/सच्च);
}

नामाकाश {

वर्ग JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_राशिः;

  unsigned getDirIndent() { प्रत्यागम 4 * Dir_राशिः.size(); }
  unsigned getFileIndent() { प्रत्यागम 4 * (Dir_राशिः.size() + 1); }
  तार्किक containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  शून्य startDirectory(StringRef Path);
  शून्य endDirectory();
  शून्य writeEntry(StringRef VPath, StringRef RPath);

सार्वजनिक:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  शून्य लिख(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<तार्किक> UseExternalNames,
             std::optional<तार्किक> IsCaseSensitive,
             std::optional<तार्किक> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

तार्किक JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using नामाकाश llvm::sys;

  // Compare each path component.
  auto IParent = path::begin(Parent), EParent = path::end(Parent);
  प्रति (auto IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    जइ (*IParent != *IChild)
      प्रत्यागम मिच्छ;
  }
  // Have we exhausted the parent path?
  प्रत्यागम IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  प्रतिज्ञा(!Parent.empty());
  प्रतिज्ञा(containedIn(Parent, Path));
  प्रत्यागम Path.substr(Parent.size() + 1);
}

शून्य JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_राशिः.empty() ? Path : containedPart(Dir_राशिः.back(), Path);
  Dir_राशिः.प्रक्षिप_back(Path);
  unsigned Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

शून्य JSONWriter::endDirectory() {
  unsigned Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_राशिः.निष्कासय_back();
}

शून्य JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  unsigned Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

शून्य JSONWriter::लिख(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<तार्किक> UseExternalNames,
                       std::optional<तार्किक> IsCaseSensitive,
                       std::optional<तार्किक> IsOverlayRelative,
                       StringRef OverlayDir) {
  using नामाकाश llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  जइ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  जइ (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  तार्किक UseOverlayRelative = मिच्छ;
  जइ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  जइ (!Entries.empty()) {
    ध्रुव YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    जइ (UseOverlayRelative) {
      प्रतिज्ञा(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    तार्किक IsCurrentDirEmpty = सच्च;
    जइ (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = मिच्छ;
    }

    प्रति (ध्रुव auto &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      जइ (Dir == Dir_राशिः.back()) {
        जइ (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } अण्णहा {
        तार्किक Is_Dir_Popped_From_राशिः = मिच्छ;
        जाव (!Dir_राशिः.empty() && !containedIn(Dir_राशिः.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_राशिः = सच्च;
        }
        जइ (Is_Dir_Popped_From_राशिः || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = सच्च;
      }
      StringRef RPath = Entry.RPath;
      जइ (UseOverlayRelative) {
        प्रतिज्ञा(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      जइ (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = मिच्छ;
      }
    }

    जाव (!Dir_राशिः.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

शून्य YAMLVFSWriter::लिख(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](ध्रुव YAMLVFSEntry &LHS, ध्रुव YAMLVFSEntry &RHS) {
    प्रत्यागम LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).लिख(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ध्रुव Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  जइ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->राशिः.प्रक्षिप_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  प्रतिज्ञा(FS && State && !State->राशिः.empty() && "incrementing past end");
  प्रतिज्ञा(!State->राशिः.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  जइ (State->Has_No_प्रक्षिप_Request)
    State->Has_No_प्रक्षिप_Request = मिच्छ;
  अण्णहा {
    जइ (State->राशिः.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->राशिः.back()->path(), EC);
      जइ (I != End) {
        State->राशिः.प्रक्षिप_back(I);
        प्रत्यागम *एतत्;
      }
    }
  }

  जाव (!State->राशिः.empty() && State->राशिः.back().increment(EC) == End)
    State->राशिः.निष्कासय_back();

  जइ (State->राशिः.empty())
    State.reset(); // end iterator

  प्रत्यागम *एतत्;
}

ध्रुव वर्ण FileSystem::ID = 0;
ध्रुव वर्ण OverlayFileSystem::ID = 0;
ध्रुव वर्ण ProxyFileSystem::ID = 0;
ध्रुव वर्ण InMemoryFileSystem::ID = 0;
ध्रुव वर्ण RedirectingFileSystem::ID = 0;

unsigned ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_गणनम्(
    ध्रुव FileSystem *FS) {
  प्रत्यागम FS->Use_गणनम्();
}

शून्य ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

शून्य ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
