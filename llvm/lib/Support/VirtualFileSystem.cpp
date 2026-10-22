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

Status::Status(स्थिरांक file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(स्थिरांक Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(स्थिरांक Status &In, uint64_t NewSize) {
  परत Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(स्थिरांक Status &In, स्थिरांक Twine &NewName) {
  परत Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(स्थिरांक file_status &In, स्थिरांक Twine &NewName) {
  परत Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

तार्किक Status::equivalent(स्थिरांक Status &Other) स्थिरांक {
  प्रतिज्ञा(isStatusKnown() && Other.isStatusKnown());
  परत getUniqueID() == Other.getUniqueID();
}

तार्किक Status::isDirectory() स्थिरांक { परत Type == file_type::directory_file; }

तार्किक Status::isRegularFile() स्थिरांक { परत Type == file_type::regular_file; }

तार्किक Status::isOther() स्थिरांक {
  परत exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

तार्किक Status::isSymlink() स्थिरांक { परत Type == file_type::symlink_file; }

तार्किक Status::isStatusKnown() स्थिरांक { परत Type != file_type::status_error; }

तार्किक Status::exists() स्थिरांक {
  परत isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = पूर्वनिर्धारित;

FileSystem::~FileSystem() = पूर्वनिर्धारित;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(स्थिरांक llvm::Twine &Name, int64_t FileSize,
                             तार्किक RequiresNullTerminator, तार्किक IsVolatile,
                             तार्किक IsText) {
  स्वतः F = IsText ? openFileForRead(Name) : open_File_For_Read_द्विमान(Name);
  जर (!F)
    परत F.getError();

  परत (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<अक्षर> &Path) स्थिरांक {
  जर (llvm::sys::path::is_absolute(Path))
    परत {};

  स्वतः WorkingDir = getCurrentWorkingDirectory();
  जर (!WorkingDir)
    परत WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  परत {};
}

std::error_code FileSystem::getRealPath(स्थिरांक Twine &Path,
                                        SmallVectorImpl<अक्षर> &Output) {
  परत errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(स्थिरांक Twine &Path, तार्किक &Result) {
  परत errc::operation_not_permitted;
}

तार्किक FileSystem::exists(स्थिरांक Twine &Path) {
  स्वतः Status = status(Path);
  परत Status && Status->exists();
}

llvm::ErrorOr<तार्किक> FileSystem::equivalent(स्थिरांक Twine &A, स्थिरांक Twine &B) {
  स्वतः StatusA = status(A);
  जर (!StatusA)
    परत StatusA.getError();
  स्वतः StatusB = status(B);
  जर (!StatusB)
    परत StatusB.getError();
  परत StatusA->equivalent(*StatusB);
}

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
शून्य FileSystem::dump() स्थिरांक { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef न_संशोधन
स्थिर तार्किक isTraversalComponent(StringRef Component) {
  परत Component == ".." || Component == ".";
}

स्थिर तार्किक pathHasTraversal(StringRef Path) {
  using नामाकाश llvm::sys;

  साठी (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    जर (isTraversalComponent(Comp))
      परत सत्य;
  परत असत्य;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

नामाकाश {

/// Wrapper around a raw file descriptor.
वर्ग RealFile : सार्वजनिक File {
  मित्र वर्ग RealFileSystem;

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
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(स्थिरांक Twine &Name,
                                                   int64_t FileSize,
                                                   तार्किक RequiresNullTerminator,
                                                   तार्किक IsVolatile) override;
  std::error_code बंद() override;
  शून्य setPath(स्थिरांक Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { बंद(); }

ErrorOr<Status> RealFile::status() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(FD.isValid() && "cannot stat closed file");
  जर (!S.isStatusKnown()) {
    file_status RealStatus;
    जर (std::error_code EC = sys::fs::status(FD, RealStatus))
      परत EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  परत S;
}

ErrorOr<std::string> RealFile::getName() {
  परत RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(स्थिरांक Twine &Name, int64_t FileSize,
                    तार्किक RequiresNullTerminator, तार्किक IsVolatile) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(FD.isValid() && "cannot get buffer for closed file");
  परत MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::बंद() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  परत EC;
}

शून्य RealFile::setPath(स्थिरांक Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  जर (स्वतः Status = status())
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
    जर (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      जर (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      अन्यथा जर (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      अन्यथा
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(स्थिरांक Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(स्थिरांक Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_द्विमान(स्थिरांक Twine &Path) override;
  directory_iterator dir_begin(स्थिरांक Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() स्थिरांक override;
  std::error_code setCurrentWorkingDirectory(स्थिरांक Twine &Path) override;
  std::error_code isLocal(स्थिरांक Twine &Path, तार्किक &Result) override;
  शून्य
  getDirectoryContentRealSources(स्थिरांक Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.ढकला_back(Dir.str());
  }
  std::error_code getRealPath(स्थिरांक Twine &Path,
                              SmallVectorImpl<अक्षर> &Output) override;

संरक्षित:
  शून्य printImpl(raw_ostream &OS, PrintType Type,
                 अचिन्हित IndentLevel) स्थिरांक override;

खाजगी:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(स्थिरांक Twine &Path, SmallVectorImpl<अक्षर> &Storage) स्थिरांक {
    जर (!WD || !*WD)
      परत Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    परत Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(स्थिरांक Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    जर (!FDOrErr)
      परत errorToErrorCode(FDOrErr.takeError());
    परत std::unique_ptr<File>(
        नवीन RealFile(*FDOrErr, Name.str(), RealName.str()));
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

ErrorOr<Status> RealFileSystem::status(स्थिरांक Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  जर (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    परत EC;
  परत Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(स्थिरांक Twine &Name) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  परत openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_द्विमान(स्थिरांक Twine &Name) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  परत openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() स्थिरांक {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  जर (WD && *WD)
    परत std::string(WD->get().Specified);
  जर (WD)
    परत WD->getError();

  SmallString<128> Dir;
  जर (std::error_code EC = llvm::sys::fs::current_path(Dir))
    परत EC;
  परत std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(स्थिरांक Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  जर (!WD)
    परत llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  तार्किक IsDir;
  जर (स्वतः Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    परत Err;
  जर (!IsDir)
    परत std::make_error_code(std::errc::not_a_directory);
  जर (स्वतः Err = llvm::sys::fs::real_path(Absolute, Resolved))
    परत Err;
  WD = WorkingDirectory{Absolute, Resolved};
  परत std::error_code();
}

std::error_code RealFileSystem::isLocal(स्थिरांक Twine &Path, तार्किक &Result) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  परत llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(स्थिरांक Twine &Path,
                                            SmallVectorImpl<अक्षर> &Output) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  परत llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

शून्य RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               अचिन्हित IndentLevel) स्थिरांक {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  जर (WD)
    OS << "own";
  अन्यथा
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  स्थिर IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(सत्य);
  sys::sandbox::violationIfEnabled();

  परत FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  परत std::make_unique<RealFileSystem>(असत्य);
}

नामाकाश {

वर्ग RealFSDirIter : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

सार्वजनिक:
  RealFSDirIter(स्थिरांक Twine &Path, std::error_code &EC) {
    स्वतः BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    जर (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    स्वतः BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    परत EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(स्थिरांक Twine &Dir,
                                             std::error_code &EC) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  परत directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.ढकला_back(std::move(BaseFS));
}

शून्य OverlayFileSystem::ढकला_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.ढकला_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(स्थिरांक Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  साठी (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    जर (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      परत Status;
  }
  परत make_error_code(llvm::errc::no_such_file_or_directory);
}

तार्किक OverlayFileSystem::exists(स्थिरांक Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  साठी (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    जर ((*I)->exists(Path))
      परत सत्य;
  }
  परत असत्य;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(स्थिरांक llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  साठी (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    स्वतः Result = (*I)->openFileForRead(Path);
    जर (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      परत Result;
  }
  परत make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() स्थिरांक {
  // All file systems are synchronized, just take the first working directory.
  परत FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(स्थिरांक Twine &Path) {
  साठी (स्वतः &FS : FSList)
    जर (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      परत EC;
  परत {};
}

std::error_code OverlayFileSystem::isLocal(स्थिरांक Twine &Path, तार्किक &Result) {
  साठी (स्वतः &FS : FSList)
    जर (FS->exists(Path))
      परत FS->isLocal(Path, Result);
  परत errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(स्थिरांक Twine &Path,
                                               SmallVectorImpl<अक्षर> &Output) {
  साठी (स्थिरांक स्वतः &FS : FSList)
    जर (FS->exists(Path))
      परत FS->getRealPath(Path, Output);
  परत errc::no_such_file_or_directory;
}

शून्य OverlayFileSystem::getDirectoryContentRealSources(
    स्थिरांक Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  साठी (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

शून्य OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  साठी (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

शून्य OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  अचिन्हित IndentLevel) स्थिरांक {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  जर (Type == PrintType::Summary)
    परत;

  जर (Type == PrintType::Contents)
    Type = PrintType::Summary;
  साठी (स्थिरांक स्वतः &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = पूर्वनिर्धारित;

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
    पर्यंत (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.काढा_back();
      जर (CurrentDirIter != directory_iterator())
        थांबा; // found
    }

    जर (IsFirstTime && CurrentDirIter == directory_iterator())
      परत errc::no_such_file_or_directory;
    परत {};
  }

  std::error_code incrementDirIter(तार्किक IsFirstTime) {
    प्रतिज्ञा((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    जर (!IsFirstTime)
      CurrentDirIter.increment(EC);
    जर (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    परत EC;
  }

  std::error_code incrementImpl(तार्किक IsFirstTime) {
    पर्यंत (सत्य) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      जर (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        परत EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      जर (SeenNames.घाला(Name).second)
        परत EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

सार्वजनिक:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    साठी (स्थिरांक स्वतः &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      जर (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        परत;
      }
      जर (!FEC)
        IterList.ढकला_back(Iter);
    }
    EC = incrementImpl(सत्य);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(सत्य);
  }

  std::error_code increment() override { परत incrementImpl(असत्य); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(स्थिरांक Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  जर (EC)
    परत {};
  परत Combined;
}

शून्य ProxyFileSystem::anchor() {}

नामाकाश llvm {
नामाकाश vfs {

नामाकाश detail {

गणना In_Memory_गाठ_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
वर्ग In_Memory_गाठ {
  In_Memory_गाठ_Kind Kind;
  std::string FileName;

सार्वजनिक:
  In_Memory_गाठ(llvm::StringRef FileName, In_Memory_गाठ_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  आभासी ~In_Memory_गाठ() = पूर्वनिर्धारित;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  आभासी Status getStatus(स्थिरांक Twine &RequestedName) स्थिरांक = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() स्थिरांक { परत FileName; }
  In_Memory_गाठ_Kind getKind() स्थिरांक { परत Kind; }
  आभासी std::string toString(अचिन्हित Indent) स्थिरांक = 0;
};

वर्ग InMemoryFile : सार्वजनिक In_Memory_गाठ {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

सार्वजनिक:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_गाठ(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(स्थिरांक Twine &RequestedName) स्थिरांक override {
    परत Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() स्थिरांक { परत Buffer.get(); }

  std::string toString(अचिन्हित Indent) स्थिरांक override {
    परत (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  स्थिर तार्किक classof(स्थिरांक In_Memory_गाठ *N) {
    परत N->getKind() == IME_File;
  }
};

नामाकाश {

वर्ग InMemoryHardLink : सार्वजनिक In_Memory_गाठ {
  स्थिरांक InMemoryFile &ResolvedFile;

सार्वजनिक:
  InMemoryHardLink(StringRef Path, स्थिरांक InMemoryFile &ResolvedFile)
      : In_Memory_गाठ(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  स्थिरांक InMemoryFile &getResolvedFile() स्थिरांक { परत ResolvedFile; }

  Status getStatus(स्थिरांक Twine &RequestedName) स्थिरांक override {
    परत ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(अचिन्हित Indent) स्थिरांक override {
    परत std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  स्थिर तार्किक classof(स्थिरांक In_Memory_गाठ *N) {
    परत N->getKind() == IME_HardLink;
  }
};

वर्ग InMemorySymbolicLink : सार्वजनिक In_Memory_गाठ {
  std::string TargetPath;
  Status Stat;

सार्वजनिक:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_गाठ(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(अचिन्हित Indent) स्थिरांक override {
    परत std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(स्थिरांक Twine &RequestedName) स्थिरांक override {
    परत Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() स्थिरांक { परत TargetPath; }

  स्थिर तार्किक classof(स्थिरांक In_Memory_गाठ *N) {
    परत N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
वर्ग InMemoryFileAdaptor : सार्वजनिक File {
  स्थिरांक InMemoryFile &गाठ;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

सार्वजनिक:
  explicit InMemoryFileAdaptor(स्थिरांक InMemoryFile &गाठ,
                               std::string RequestedName)
      : गाठ(गाठ), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    परत गाठ.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(स्थिरांक Twine &Name, int64_t FileSize, तार्किक RequiresNullTerminator,
            तार्किक IsVolatile) override {
    llvm::MemoryBuffer *Buf = गाठ.getBuffer();
    परत llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code बंद() override { परत {}; }

  शून्य setPath(स्थिरांक Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

वर्ग InMemoryDirectory : सार्वजनिक In_Memory_गाठ {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_गाठ>, std::less<>> Entries;

सार्वजनिक:
  InMemoryDirectory(Status Stat)
      : In_Memory_गाठ(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(स्थिरांक Twine &RequestedName) स्थिरांक override {
    परत Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() स्थिरांक { परत Stat.getUniqueID(); }

  In_Memory_गाठ *getChild(StringRef Name) स्थिरांक {
    स्वतः I = Entries.find(Name);
    जर (I != Entries.end())
      परत I->second.get();
    परत nullptr;
  }

  In_Memory_गाठ *addChild(StringRef Name, std::unique_ptr<In_Memory_गाठ> Child) {
    परत Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() स्थिरांक { परत Entries.begin(); }
  const_iterator end() स्थिरांक { परत Entries.end(); }

  std::string toString(अचिन्हित Indent) स्थिरांक override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    साठी (स्थिरांक स्वतः &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    परत Result;
  }

  स्थिर तार्किक classof(स्थिरांक In_Memory_गाठ *N) {
    परत N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
स्थिर sys::fs::UniqueID getUniqueID(hash_code Hash) {
  परत sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(माप_प्रकार(Hash)));
}
स्थिर sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  परत getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
स्थिर sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  परत getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_गाठ_Info::makeStatus() स्थिरांक {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  परत Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(तार्किक UseNormalizedPaths)
    : मूळ_2(नवीन detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = पूर्वनिर्धारित;

std::string InMemoryFileSystem::toString() स्थिरांक {
  परत मूळ_2->toString(/*Indent=*/0);
}

तार्किक InMemoryFileSystem::addFile(स्थिरांक Twine &P, समय_प्रकार ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_गाठ_Fn Make_गाठ) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  जर (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सत्य);

  जर (Path.empty())
    परत असत्य;

  detail::InMemoryDirectory *Dir = मूळ_2.get();
  स्वतः I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  स्थिरांक स्वतः ResolvedUser = User.मूल्य_or(0);
  स्थिरांक स्वतः ResolvedGroup = Group.मूल्य_or(0);
  स्थिरांक स्वतः ResolvedType = Type.मूल्य_or(sys::fs::file_type::regular_file);
  स्थिरांक स्वतः ResolvedPerms = Perms.मूल्य_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  स्थिरांक स्वतः NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  पर्यंत (सत्य) {
    Name = *I;
    ++I;
    जर (I == E)
      थांबा;
    detail::In_Memory_गाठ *गाठ = Dir->getChild(Name);
    जर (!गाठ) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      सुरूठेवा;
    }
    // Creating file under another file.
    जर (!isa<detail::InMemoryDirectory>(गाठ))
      परत असत्य;
    Dir = cast<detail::InMemoryDirectory>(गाठ);
  }
  detail::In_Memory_गाठ *गाठ = Dir->getChild(Name);
  जर (!गाठ) {
    Dir->addChild(Name,
                  Make_गाठ({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    परत सत्य;
  }
  जर (isa<detail::InMemoryDirectory>(गाठ))
    परत ResolvedType == sys::fs::file_type::directory_file;

  प्रतिज्ञा((isa<detail::InMemoryFile>(गाठ) ||
          isa<detail::InMemoryHardLink>(गाठ)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  जर (स्वतः *Link = dyn_cast<detail::InMemoryHardLink>(गाठ)) {
    परत Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  परत cast<detail::InMemoryFile>(गाठ)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

तार्किक InMemoryFileSystem::addFile(स्थिरांक Twine &P, समय_प्रकार ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  परत addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_गाठ_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_गाठ> {
                   Status Stat = NNI.makeStatus();
                   जर (Stat.getType() == sys::fs::file_type::directory_file)
                     परत std::make_unique<detail::InMemoryDirectory>(Stat);
                   परत std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

तार्किक InMemoryFileSystem::addFileNoOwn(
    स्थिरांक Twine &P, समय_प्रकार ModificationTime,
    स्थिरांक llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  परत addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_गाठ_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_गाठ> {
                   Status Stat = NNI.makeStatus();
                   जर (Stat.getType() == sys::fs::file_type::directory_file)
                     परत std::make_unique<detail::InMemoryDirectory>(Stat);
                   परत std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_गाठ_Or_Error
InMemoryFileSystem::lookup_गाठ(स्थिरांक Twine &P, तार्किक FollowFinalSymlink,
                               माप_प्रकार SymlinkDepth) स्थिरांक {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  जर (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सत्य);

  स्थिरांक detail::InMemoryDirectory *Dir = मूळ_2.get();
  जर (Path.empty())
    परत detail::Named_गाठ_Or_Error(Path, Dir);

  स्वतः I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  पर्यंत (सत्य) {
    detail::In_Memory_गाठ *गाठ = Dir->getChild(*I);
    ++I;
    जर (!गाठ)
      परत errc::no_such_file_or_directory;

    जर (स्वतः Symlink = dyn_cast<detail::InMemorySymbolicLink>(गाठ)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      जर (I == E && !FollowFinalSymlink)
        परत detail::Named_गाठ_Or_Error(Path, Symlink);

      जर (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        परत errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      जर (std::error_code EC = makeAbsolute(TargetPath))
        परत EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      स्वतः Target =
          lookup_गाठ(TargetPath, /*FollowFinalSymlink=*/सत्य, SymlinkDepth + 1);
      जर (!Target || I == E)
        परत Target;

      जर (!isa<detail::InMemoryDirectory>(*Target))
        परत errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      सुरूठेवा;
    }

    // Return the file if it's at the end of the path.
    जर (स्वतः File = dyn_cast<detail::InMemoryFile>(गाठ)) {
      जर (I == E)
        परत detail::Named_गाठ_Or_Error(Path, File);
      परत errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    जर (स्वतः File = dyn_cast<detail::InMemoryHardLink>(गाठ)) {
      जर (I == E)
        परत detail::Named_गाठ_Or_Error(Path, &File->getResolvedFile());
      परत errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(गाठ);
    जर (I == E)
      परत detail::Named_गाठ_Or_Error(Path, Dir);
  }
}

तार्किक InMemoryFileSystem::addHardLink(स्थिरांक Twine &NewLink,
                                     स्थिरांक Twine &Target) {
  स्वतः New_Link_गाठ = lookup_गाठ(NewLink, /*FollowFinalSymlink=*/असत्य);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  स्वतः Target_गाठ = lookup_गाठ(Target, /*FollowFinalSymlink=*/सत्य);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  जर (!Target_गाठ || New_Link_गाठ || !isa<detail::InMemoryFile>(*Target_गाठ))
    परत असत्य;
  परत addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_गाठ_Info NNI) {
                   परत std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_गाठ));
                 });
}

तार्किक InMemoryFileSystem::addSymbolicLink(
    स्थिरांक Twine &NewLink, स्थिरांक Twine &Target, समय_प्रकार ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  स्वतः New_Link_गाठ = lookup_गाठ(NewLink, /*FollowFinalSymlink=*/असत्य);
  जर (New_Link_गाठ)
    परत असत्य;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  परत addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_गाठ_Info NNI) {
                   परत std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(स्थिरांक Twine &Path) {
  स्वतः गाठ = lookup_गाठ(Path, /*FollowFinalSymlink=*/सत्य);
  जर (गाठ)
    परत (*गाठ)->getStatus(Path);
  परत गाठ.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(स्थिरांक Twine &Path) {
  स्वतः गाठ = lookup_गाठ(Path,/*FollowFinalSymlink=*/सत्य);
  जर (!गाठ)
    परत गाठ.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  जर (स्वतः *F = dyn_cast<detail::InMemoryFile>(*गाठ))
    परत std::unique_ptr<File>(
        नवीन detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  परत make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
वर्ग InMemoryFileSystem::DirIterator : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  स्थिरांक InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  शून्य setCurrentEntry() {
    जर (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      निवड (I->second->getKind()) {
      स्थिती detail::IME_File:
      स्थिती detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        थांबा;
      स्थिती detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        थांबा;
      स्थिती detail::IME_SymbolicLink:
        जर (स्वतः SymlinkTarget =
                FS->lookup_गाठ(Path, /*FollowFinalSymlink=*/सत्य)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        थांबा;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } अन्यथा {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

सार्वजनिक:
  DirIterator() = पूर्वनिर्धारित;

  DirIterator(स्थिरांक InMemoryFileSystem *FS,
              स्थिरांक detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    परत {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(स्थिरांक Twine &Dir,
                                                 std::error_code &EC) {
  स्वतः गाठ = lookup_गाठ(Dir, /*FollowFinalSymlink=*/सत्य);
  जर (!गाठ) {
    EC = गाठ.getError();
    परत directory_iterator(std::make_shared<DirIterator>());
  }

  जर (स्वतः *Dir_गाठ = dyn_cast<detail::InMemoryDirectory>(*गाठ))
    परत directory_iterator(
        std::make_shared<DirIterator>(हे, *Dir_गाठ, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  परत directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(स्थिरांक Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  जर (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सत्य);

  जर (!Path.empty())
    WorkingDirectory = std::string(Path);
  परत {};
}

std::error_code InMemoryFileSystem::getRealPath(स्थिरांक Twine &Path,
                                                SmallVectorImpl<अक्षर> &Output) {
  स्वतः CWD = getCurrentWorkingDirectory();
  जर (!CWD || CWD->empty())
    परत errc::operation_not_permitted;
  Path.toVector(Output);
  जर (स्वतः EC = makeAbsolute(Output))
    परत EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/सत्य);
  परत {};
}

std::error_code InMemoryFileSystem::isLocal(स्थिरांक Twine &Path, तार्किक &Result) {
  Result = असत्य;
  परत {};
}

शून्य InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   अचिन्हित IndentLevel) स्थिरांक {
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
  स्थिरांक माप_प्रकार n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  जर (n != static_cast<माप_प्रकार>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  परत style;
}

/// Removes leading "./" as well as path components like ".." and ".".
स्थिर llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> result =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(result, /*remove_dot_dot=*/सत्य, style);
  परत result;
}

/// Whether the error and entry specify a file/directory that was not found.
स्थिर तार्किक isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  जर (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    परत असत्य;
  परत EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  प्रतिज्ञा(ExternalFS && "RedirectingFileSystem requires an external FS");
  जर (स्वतः ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
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
    जर (!IsFirstTime)
      ++Current;
    जर (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      निवड ((*Current)->getKind()) {
      स्थिती RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      स्थिती RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        थांबा;
      स्थिती RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        थांबा;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } अन्यथा {
      CurrentEntry = directory_entry();
    }
    परत {};
  };

सार्वजनिक:
  RedirectingFSDirIterImpl(
      स्थिरांक Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/सत्य);
  }

  std::error_code increment() override {
    परत incrementImpl(/*IsFirstTime=*/असत्य);
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
    जर (ExternalIter != llvm::vfs::directory_iterator())
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
    जर (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    अन्यथा
      CurrentEntry = directory_entry();
    परत EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() स्थिरांक {
  परत WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(स्थिरांक Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  जर (!exists(Path))
    परत errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  जर (std::error_code EC = makeAbsolute(AbsolutePath))
    परत EC;
  WorkingDirectory = std::string(AbsolutePath);
  परत {};
}

std::error_code RedirectingFileSystem::isLocal(स्थिरांक Twine &Path_,
                                               तार्किक &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  जर (makeAbsolute(Path))
    परत {};

  परत ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<अक्षर> &Path) स्थिरांक {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  जर (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    परत {};

  स्वतः WorkingDir = getCurrentWorkingDirectory();
  जर (!WorkingDir)
    परत WorkingDir.getError();

  परत makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<अक्षर> &Path) स्थिरांक {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  जर (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    परत std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  जर (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } अन्यथा {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    जर (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  जर (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  परत {};
}

directory_iterator RedirectingFileSystem::dir_begin(स्थिरांक Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  जर (EC)
    परत {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जर (!Result) {
    जर (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      परत ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    परत {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  जर (!S) {
    जर (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      परत ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    परत {};
  }

  जर (!S->isDirectory()) {
    EC = errc::not_a_directory;
    परत {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  जर (स्वतः ExtRedirect = Result->getExternalRedirect()) {
    स्वतः RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    जर (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } अन्यथा {
    स्वतः DE = cast<DirectoryEntry>(Result->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  जर (RedirectEC) {
    जर (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      परत {};
    }
    RedirectIter = {};
  }

  जर (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    परत RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  जर (ExternalEC) {
    जर (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      परत {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  निवड (Redirection) {
  स्थिती RedirectKind::Fallthrough:
    Iters.ढकला_back(ExternalIter);
    Iters.ढकला_back(RedirectIter);
    थांबा;
  स्थिती RedirectKind::Fallback:
    Iters.ढकला_back(RedirectIter);
    Iters.ढकला_back(ExternalIter);
    थांबा;
  पूर्वनिर्धारित:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  जर (EC)
    परत {};
  परत Combined;
}

शून्य RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() स्थिरांक {
  परत OverlayFileDir;
}

शून्य RedirectingFileSystem::setFallthrough(तार्किक Fallthrough) {
  जर (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } अन्यथा {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

शून्य RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() स्थिरांक {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  साठी (स्थिरांक स्वतः &मूळ_2 : Roots)
    R.ढकला_back(मूळ_2->getName());
  परत R;
}

शून्य RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      अचिन्हित IndentLevel) स्थिरांक {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  जर (Type == PrintType::Summary)
    परत;

  साठी (स्थिरांक स्वतः &मूळ_2 : Roots)
    printEntry(OS, मूळ_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

शून्य RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       अचिन्हित IndentLevel) स्थिरांक {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  निवड (E->getKind()) {
  स्थिती EK_Directory: {
    स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    साठी (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    थांबा;
  }
  स्थिती EK_DirectoryRemap:
  स्थिती EK_File: {
    स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    निवड (RE->getUseName()) {
    स्थिती NK_NotSet:
      थांबा;
    स्थिती NK_External:
      OS << " (UseExternalName: true)";
      थांबा;
    स्थिती NK_Virtual:
      OS << " (UseExternalName: false)";
      थांबा;
    }
    OS << "\n";
    थांबा;
  }
  }
}

शून्य RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  जर (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
वर्ग llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  शून्य error(yaml::गाठ *N, स्थिरांक Twine &संदेश_2) { Stream.printError(N, संदेश_2); }

  // false on error
  तार्किक parseScalarString(yaml::गाठ *N, StringRef &Result,
                         SmallVectorImpl<अक्षर> &Storage) {
    स्थिरांक स्वतः *S = dyn_cast_if_present<yaml::Scalar_गाठ>(N);

    जर (!S) {
      error(N, "expected string");
      परत असत्य;
    }
    Result = S->get_मूल्य(Storage);
    परत सत्य;
  }

  // false on error
  तार्किक parseScalarBool(yaml::गाठ *N, तार्किक &Result) {
    SmallString<5> Storage;
    StringRef मूल्य;
    जर (!parseScalarString(N, मूल्य, Storage))
      परत असत्य;

    जर (मूल्य.equals_insensitive("true") || मूल्य.equals_insensitive("on") ||
        मूल्य.equals_insensitive("yes") || मूल्य == "1") {
      Result = सत्य;
      परत सत्य;
    } अन्यथा जर (मूल्य.equals_insensitive("false") ||
               मूल्य.equals_insensitive("off") ||
               मूल्य.equals_insensitive("no") || मूल्य == "0") {
      Result = असत्य;
      परत सत्य;
    }

    error(N, "expected boolean value");
    परत असत्य;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::गाठ *N) {
    SmallString<12> Storage;
    StringRef मूल्य;
    जर (!parseScalarString(N, मूल्य, Storage))
      परत std::nullopt;

    जर (मूल्य.equals_insensitive("fallthrough")) {
      परत RedirectingFileSystem::RedirectKind::Fallthrough;
    } अन्यथा जर (मूल्य.equals_insensitive("fallback")) {
      परत RedirectingFileSystem::RedirectKind::Fallback;
    } अन्यथा जर (मूल्य.equals_insensitive("redirect-only")) {
      परत RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    परत std::nullopt;
  }

  std::optional<RedirectingFileSystem::मूळ_Relative_Kind>
  parse_मूळ_Relative_Kind(yaml::गाठ *N) {
    SmallString<12> Storage;
    StringRef मूल्य;
    जर (!parseScalarString(N, मूल्य, Storage))
      परत std::nullopt;
    जर (मूल्य.equals_insensitive("cwd")) {
      परत RedirectingFileSystem::मूळ_Relative_Kind::CWD;
    } अन्यथा जर (मूल्य.equals_insensitive("overlay-dir")) {
      परत RedirectingFileSystem::मूळ_Relative_Kind::OverlayDir;
    }
    परत std::nullopt;
  }

  संरचना किल्ली_Status {
    तार्किक Required;
    तार्किक Seen = असत्य;

    किल्ली_Status(तार्किक Required = असत्य) : Required(Required) {}
  };

  using किल्ली_Status_Pair = std::pair<StringRef, किल्ली_Status>;

  // false on error
  तार्किक check_Duplicate_Or_Unknown_किल्ली(yaml::गाठ *किल्ली_गाठ, StringRef किल्ली,
                                  DenseMap<StringRef, किल्ली_Status> &किल्ल्या_2) {
    स्वतः It = किल्ल्या_2.find(किल्ली);
    जर (It == किल्ल्या_2.end()) {
      error(किल्ली_गाठ, "unknown key");
      परत असत्य;
    }
    किल्ली_Status &S = It->second;
    जर (S.Seen) {
      error(किल्ली_गाठ, Twine("duplicate key '") + किल्ली + "'");
      परत असत्य;
    }
    S.Seen = सत्य;
    परत सत्य;
  }

  // false on error
  तार्किक check_Missing_किल्ल्या(yaml::गाठ *Obj, DenseMap<StringRef, किल्ली_Status> &किल्ल्या_2) {
    साठी (स्थिरांक स्वतः &I : किल्ल्या_2) {
      जर (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        परत असत्य;
      }
    }
    परत सत्य;
  }

सार्वजनिक:
  स्थिर RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    जर (!ParentEntry) { // Look for a existent root
      साठी (स्थिरांक स्वतः &मूळ_2 : FS->Roots) {
        जर (Name == मूळ_2->getName()) {
          ParentEntry = मूळ_2.get();
          परत ParentEntry;
        }
      }
    } अन्यथा { // Advance to the next component
      स्वतः *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      साठी (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        स्वतः *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        जर (DirContent && Name == Content->getName())
          परत DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    जर (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.ढकला_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      परत ParentEntry;
    }

    स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    परत DE->getLastContent();
  }

खाजगी:
  शून्य unique_Overlay_वृक्ष(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    निवड (SrcE->getKind()) {
    स्थिती RedirectingFileSystem::EK_Directory: {
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      जर (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      साठी (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_वृक्ष(FS, SubEntry.get(), NewParentE);
      थांबा;
    }
    स्थिती RedirectingFileSystem::EK_DirectoryRemap: {
      प्रतिज्ञा(NewParentE && "Parent entry must exist");
      स्वतः *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      थांबा;
    }
    स्थिती RedirectingFileSystem::EK_File: {
      प्रतिज्ञा(NewParentE && "Parent entry must exist");
      स्वतः *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      थांबा;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::गाठ *N, RedirectingFileSystem *FS, तार्किक Is_मूळ_Entry) {
    स्वतः *M = dyn_cast<yaml::Mapping_गाठ>(N);
    जर (!M) {
      error(N, "expected mapping node for file or directory entry");
      परत nullptr;
    }

    किल्ली_Status_Pair Fields[] = {
        किल्ली_Status_Pair("name", सत्य),
        किल्ली_Status_Pair("type", सत्य),
        किल्ली_Status_Pair("contents", असत्य),
        किल्ली_Status_Pair("external-contents", असत्य),
        किल्ली_Status_Pair("use-external-name", असत्य),
    };

    DenseMap<StringRef, किल्ली_Status> किल्ल्या_2(std::begin(Fields), std::end(Fields));

    गणना { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::गाठ *Name_मूल्य_गाठ = nullptr;
    स्वतः UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    साठी (स्वतः &I : *M) {
      StringRef किल्ली;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      जर (!parseScalarString(I.get_किल्ली(), किल्ली, Buffer))
        परत nullptr;

      जर (!check_Duplicate_Or_Unknown_किल्ली(I.get_किल्ली(), किल्ली, किल्ल्या_2))
        परत nullptr;

      StringRef मूल्य;
      जर (किल्ली == "name") {
        जर (!parseScalarString(I.get_मूल्य(), मूल्य, Buffer))
          परत nullptr;

        Name_मूल्य_गाठ = I.get_मूल्य();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(मूल्य).str();
      } अन्यथा जर (किल्ली == "type") {
        जर (!parseScalarString(I.get_मूल्य(), मूल्य, Buffer))
          परत nullptr;
        जर (मूल्य == "file")
          Kind = RedirectingFileSystem::EK_File;
        अन्यथा जर (मूल्य == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        अन्यथा जर (मूल्य == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        अन्यथा {
          error(I.get_मूल्य(), "unknown value for 'type'");
          परत nullptr;
        }
      } अन्यथा जर (किल्ली == "contents") {
        जर (ContentsField != CF_NotSet) {
          error(I.get_किल्ली(),
                "entry already has 'contents' or 'external-contents'");
          परत nullptr;
        }
        ContentsField = CF_List;
        स्वतः *Contents = dyn_cast_if_present<yaml::Sequence_गाठ>(I.get_मूल्य());
        जर (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_मूल्य(), "expected array");
          परत nullptr;
        }

        साठी (स्वतः &I : *Contents) {
          जर (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ असत्य))
            EntryArrayContents.ढकला_back(std::move(E));
          अन्यथा
            परत nullptr;
        }
      } अन्यथा जर (किल्ली == "external-contents") {
        जर (ContentsField != CF_NotSet) {
          error(I.get_किल्ली(),
                "entry already has 'contents' or 'external-contents'");
          परत nullptr;
        }
        ContentsField = CF_External;
        जर (!parseScalarString(I.get_मूल्य(), मूल्य, Buffer))
          परत nullptr;

        SmallString<256> FullPath;
        जर (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          प्रतिज्ञा(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = मूल्य;
          जर (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            परत nullptr;
          }
          FullPath = AbsFullPath;
        } अन्यथा {
          FullPath = मूल्य;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } अन्यथा जर (किल्ली == "use-external-name") {
        तार्किक Val;
        जर (!parseScalarBool(I.get_मूल्य(), Val))
          परत nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } अन्यथा {
        llvm_unreachable("key missing from Keys");
      }
    }

    जर (Stream.failed())
      परत nullptr;

    // check for missing keys
    जर (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      परत nullptr;
    }
    जर (!check_Missing_किल्ल्या(N, किल्ल्या_2))
      परत nullptr;

    // check invalid configuration
    जर (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      परत nullptr;
    }

    जर (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      परत nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    जर (Is_मूळ_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      जर (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } अन्यथा जर (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } अन्यथा {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        जर (FS->मूळ_Relative ==
            RedirectingFileSystem::मूळ_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          प्रतिज्ञा(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } अन्यथा {
          EC = FS->makeAbsolute(Name);
        }
        जर (EC) {
          प्रतिज्ञा(Name_मूल्य_गाठ && "Name presence should be checked earlier");
          error(
              Name_मूल्य_गाठ,
              "entry with relative path at the root level is not discoverable");
          परत nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      जर (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    माप_प्रकार मूळ_Path_Len = sys::path::मूळ_path(Trimmed, path_style).size();
    पर्यंत (Trimmed.size() > मूळ_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    निवड (Kind) {
    स्थिती RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      थांबा;
    स्थिती RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      थांबा;
    स्थिती RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      थांबा;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    जर (Parent.empty())
      परत Result;

    // if 'name' contains multiple components, create implicit directory entries
    साठी (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.ढकला_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    परत Result;
  }

सार्वजनिक:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  तार्किक parse(yaml::गाठ *मूळ_2, RedirectingFileSystem *FS) {
    स्वतः *शीर्ष = dyn_cast<yaml::Mapping_गाठ>(मूळ_2);
    जर (!शीर्ष) {
      error(मूळ_2, "expected mapping node");
      परत असत्य;
    }

    किल्ली_Status_Pair Fields[] = {
        किल्ली_Status_Pair("version", सत्य),
        किल्ली_Status_Pair("case-sensitive", असत्य),
        किल्ली_Status_Pair("use-external-names", असत्य),
        किल्ली_Status_Pair("root-relative", असत्य),
        किल्ली_Status_Pair("overlay-relative", असत्य),
        किल्ली_Status_Pair("fallthrough", असत्य),
        किल्ली_Status_Pair("redirecting-with", असत्य),
        किल्ली_Status_Pair("roots", सत्य),
    };

    DenseMap<StringRef, किल्ली_Status> किल्ल्या_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> मूळ_Entries;

    // Parse configuration and 'roots'
    साठी (स्वतः &I : *शीर्ष) {
      SmallString<10> किल्ली_Buffer;
      StringRef किल्ली;
      जर (!parseScalarString(I.get_किल्ली(), किल्ली, किल्ली_Buffer))
        परत असत्य;

      जर (!check_Duplicate_Or_Unknown_किल्ली(I.get_किल्ली(), किल्ली, किल्ल्या_2))
        परत असत्य;

      जर (किल्ली == "roots") {
        स्वतः *Roots = dyn_cast_if_present<yaml::Sequence_गाठ>(I.get_मूल्य());
        जर (!Roots) {
          error(I.get_मूल्य(), "expected array");
          परत असत्य;
        }

        साठी (स्वतः &I : *Roots) {
          जर (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ सत्य))
            मूळ_Entries.ढकला_back(std::move(E));
          अन्यथा
            परत असत्य;
        }
      } अन्यथा जर (किल्ली == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        जर (!parseScalarString(I.get_मूल्य(), VersionString, Storage))
          परत असत्य;
        पूर्णांक Version;
        जर (VersionString.getAsInteger<पूर्णांक>(10, Version)) {
          error(I.get_मूल्य(), "expected integer");
          परत असत्य;
        }
        जर (Version < 0) {
          error(I.get_मूल्य(), "invalid version number");
          परत असत्य;
        }
        जर (Version != 0) {
          error(I.get_मूल्य(), "version mismatch, expected 0");
          परत असत्य;
        }
      } अन्यथा जर (किल्ली == "case-sensitive") {
        जर (!parseScalarBool(I.get_मूल्य(), FS->CaseSensitive))
          परत असत्य;
      } अन्यथा जर (किल्ली == "overlay-relative") {
        जर (!parseScalarBool(I.get_मूल्य(), FS->IsRelativeOverlay))
          परत असत्य;
      } अन्यथा जर (किल्ली == "use-external-names") {
        जर (!parseScalarBool(I.get_मूल्य(), FS->UseExternalNames))
          परत असत्य;
      } अन्यथा जर (किल्ली == "fallthrough") {
        जर (किल्ल्या_2["redirecting-with"].Seen) {
          error(I.get_मूल्य(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          परत असत्य;
        }

        तार्किक ShouldFallthrough = असत्य;
        जर (!parseScalarBool(I.get_मूल्य(), ShouldFallthrough))
          परत असत्य;

        जर (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } अन्यथा {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } अन्यथा जर (किल्ली == "redirecting-with") {
        जर (किल्ल्या_2["fallthrough"].Seen) {
          error(I.get_मूल्य(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          परत असत्य;
        }

        जर (स्वतः Kind = parseRedirectKind(I.get_मूल्य())) {
          FS->Redirection = *Kind;
        } अन्यथा {
          error(I.get_मूल्य(), "expected valid redirect kind");
          परत असत्य;
        }
      } अन्यथा जर (किल्ली == "root-relative") {
        जर (स्वतः Kind = parse_मूळ_Relative_Kind(I.get_मूल्य())) {
          FS->मूळ_Relative = *Kind;
        } अन्यथा {
          error(I.get_मूल्य(), "expected valid root-relative kind");
          परत असत्य;
        }
      } अन्यथा {
        llvm_unreachable("key missing from Keys");
      }
    }

    जर (Stream.failed())
      परत असत्य;

    जर (!check_Missing_किल्ल्या(शीर्ष, किल्ल्या_2))
      परत असत्य;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    साठी (स्वतः &E : मूळ_Entries)
      unique_Overlay_वृक्ष(FS, E.get());

    परत सत्य;
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
  yaml::गाठ *मूळ_2 = DI->get_मूळ();
  जर (DI == Stream.end() || !मूळ_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    परत nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      नवीन RedirectingFileSystem(ExternalFS));

  जर (!YAMLFilePath.empty()) {
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

  जर (!P.parse(मूळ_2, FS.get()))
    परत nullptr;

  परत FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    तार्किक UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      नवीन RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  साठी (स्वतः &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      स्वतः EC = ExternalFS->makeAbsolute(From);
      (शून्य)EC;
      प्रतिज्ञा(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    जर (ToEntry)
      सुरूठेवा;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    साठी (स्वतः I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.get(), *I,
                                                                Parent);
    }
    प्रतिज्ञा(Parent && "File without a directory?");
    {
      स्वतः EC = ExternalFS->makeAbsolute(To);
      (शून्य)EC;
      प्रतिज्ञा(!EC && "Could not make absolute path");
    }

    // Add the file.
    स्वतः NewFile = std::make_unique<RedirectingFileSystem::FileEntry>(
        llvm::sys::path::filename(From), To,
        UseExternalNames ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    ToEntry = NewFile.get();
    cast<RedirectingFileSystem::DirectoryEntry>(Parent)->addContent(
        std::move(NewFile));
  }

  परत FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  प्रतिज्ञा(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  जर (स्वतः *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

शून्य RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<अक्षर> &Result) स्थिरांक {
  Result.clear();
  साठी (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<अक्षर> &Path) स्थिरांक {
  जर (std::error_code EC = makeAbsolute(Path))
    परत EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  जर (CanonicalPath.empty())
    परत make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  परत {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) स्थिरांक {
  llvm::SmallString<128> CanonicalPath(Path);
  जर (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    परत EC;

  // RedirectOnly means the VFS is always used.
  जर (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = सत्य;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  साठी (स्थिरांक स्वतः &मूळ_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, मूळ_2.get(), Entries);
    जर (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = सत्य;
    जर (Result) {
      Result->Parents = std::move(Entries);
      परत Result;
    }

    जर (Result.getError() != llvm::errc::no_such_file_or_directory)
      परत Result;
  }
  परत make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) स्थिरांक {
  प्रतिज्ञा(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  जर (!FromName.empty()) {
    जर (!pathComponentMatches(*Start, FromName))
      परत make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    जर (Start == End) {
      // Match!
      परत LookupResult(From, Start, End);
    }
  }

  जर (isa<RedirectingFileSystem::FileEntry>(From))
    परत make_error_code(llvm::errc::not_a_directory);

  जर (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    परत LookupResult(From, Start, End);

  स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  साठी (स्थिरांक std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.ढकला_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    जर (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      परत Result;
    Entries.काढा_back();
  }

  परत make_error_code(llvm::errc::no_such_file_or_directory);
}

स्थिर Status getRedirectedFileStatus(स्थिरांक Twine &OriginalPath,
                                      तार्किक UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  जर (ExternalStatus.ExposesExternalVFSPath)
    परत ExternalStatus;

  Status S = ExternalStatus;
  जर (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  अन्यथा
    S.ExposesExternalVFSPath = सत्य;
  परत S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    स्थिरांक Twine &LookupPath, स्थिरांक Twine &OriginalPath,
    स्थिरांक RedirectingFileSystem::LookupResult &Result) {
  जर (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    जर (std::error_code EC = makeAbsolute(RemappedPath))
      परत EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    जर (!S)
      परत S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    परत getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  परत Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(स्थिरांक Twine &LookupPath,
                                         स्थिरांक Twine &OriginalPath) स्थिरांक {
  स्वतः Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  जर (!Result || Result->ExposesExternalVFSPath)
    परत Result;
  परत Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(स्थिरांक Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  जर (std::error_code EC = makeAbsolute(Path))
    परत EC;

  जर (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    जर (S)
      परत S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जर (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    जर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      परत getExternalStatus(Path, OriginalPath);
    परत Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  जर (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    परत getExternalStatus(Path, OriginalPath);
  }

  परत S;
}

तार्किक RedirectingFileSystem::exists(स्थिरांक Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  जर (makeAbsolute(Path))
    परत असत्य;

  जर (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    जर (ExternalFS->exists(Path))
      परत सत्य;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जर (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    जर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      परत ExternalFS->exists(Path);
    परत असत्य;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  जर (!ExtRedirect) {
    प्रतिज्ञा(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    परत सत्य;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  जर (makeAbsolute(RemappedPath))
    परत असत्य;

  जर (ExternalFS->exists(RemappedPath))
    परत सत्य;

  जर (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    परत ExternalFS->exists(Path);
  }

  परत असत्य;
}

नामाकाश {

/// Provide a file wrapper with an overriden status.
वर्ग FileWithFixedStatus : सार्वजनिक File {
  std::unique_ptr<File> InnerFile;
  Status S;

सार्वजनिक:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { परत S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(स्थिरांक Twine &Name, int64_t FileSize, तार्किक RequiresNullTerminator,
            तार्किक IsVolatile) override {
    परत InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code बंद() override { परत InnerFile->बंद(); }

  शून्य setPath(स्थिरांक Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, स्थिरांक Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  जर (!Result || (*Result)->status()->ExposesExternalVFSPath)
    परत Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  स्वतः Name = F->get()->getName();
  जर (Name && Name.get() != P.str())
    F->get()->setPath(P);
  परत F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(स्थिरांक Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  जर (std::error_code EC = makeAbsolute(Path))
    परत EC;

  जर (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    स्वतः F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    जर (F)
      परत F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जर (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    जर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      परत File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    परत Result.getError();
  }

  जर (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    परत make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  जर (std::error_code EC = makeAbsolute(RemappedPath))
    परत EC;

  स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  स्वतः ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  जर (!ExternalFile) {
    जर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      परत File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    परत ExternalFile;
  }

  स्वतः ExternalStatus = (*ExternalFile)->status();
  जर (!ExternalStatus)
    परत ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  परत std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(स्थिरांक Twine &OriginalPath,
                                   SmallVectorImpl<अक्षर> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  जर (std::error_code EC = makeAbsolute(Path))
    परत EC;

  जर (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    जर (!EC)
      परत EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जर (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    जर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      परत ExternalFS->getRealPath(Path, Output);
    परत Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  जर (स्वतः ExtRedirect = Result->getExternalRedirect()) {
    स्वतः P = ExternalFS->getRealPath(*ExtRedirect, Output);
    जर (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      परत ExternalFS->getRealPath(Path, Output);
    }
    परत P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  जर (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    परत {};
  }
  परत llvm::errc::invalid_argument;
}

शून्य RedirectingFileSystem::getDirectoryContentRealSources(
    स्थिरांक Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  जर (makeAbsolute(Path))
    परत;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  स्थिरांक तार्किक ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  जर (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    जर (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    परत;
  }

  निवड (Result->E->getKind()) {
  स्थिती EK_File:
    परत;
  स्थिती EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    थांबा;
  स्थिती EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    जर (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    थांबा;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  जर (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, शून्य *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  परत RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

स्थिर शून्य getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  स्वतः Kind = SrcE->getKind();
  जर (Kind == RedirectingFileSystem::EK_Directory) {
    स्वतः *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    प्रतिज्ञा(DE && "Must be a directory");
    साठी (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.ढकला_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.काढा_back();
    }
    परत;
  }

  जर (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    स्वतः *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    प्रतिज्ञा(DR && "Must be a directory remap");
    SmallString<128> VPath;
    साठी (स्वतः &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.ढकला_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    परत;
  }

  प्रतिज्ञा(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  स्वतः *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  प्रतिज्ञा(FE && "Must be a file");
  SmallString<128> VPath;
  साठी (स्वतः &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.ढकला_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

शून्य vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> मूळ_Result = VFS.lookupPath("/");
  जर (!मूळ_Result)
    परत;
  SmallVector<StringRef, 8> Components;
  Components.ढकला_back("/");
  getVFSEntries(मूळ_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  स्थिर std::atomic<अचिन्हित> UID;
  अचिन्हित ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  परत UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

शून्य YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             तार्किक IsDirectory) {
  प्रतिज्ञा(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  प्रतिज्ञा(sys::path::is_absolute(RealPath) && "real path not absolute");
  प्रतिज्ञा(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

शून्य YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/असत्य);
}

शून्य YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/सत्य);
}

नामाकाश {

वर्ग JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_चळत;

  अचिन्हित getDirIndent() { परत 4 * Dir_चळत.size(); }
  अचिन्हित getFileIndent() { परत 4 * (Dir_चळत.size() + 1); }
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
  स्वतः IParent = path::begin(Parent), EParent = path::end(Parent);
  साठी (स्वतः IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    जर (*IParent != *IChild)
      परत असत्य;
  }
  // Have we exhausted the parent path?
  परत IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  प्रतिज्ञा(!Parent.empty());
  प्रतिज्ञा(containedIn(Parent, Path));
  परत Path.substr(Parent.size() + 1);
}

शून्य JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_चळत.empty() ? Path : containedPart(Dir_चळत.back(), Path);
  Dir_चळत.ढकला_back(Path);
  अचिन्हित Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

शून्य JSONWriter::endDirectory() {
  अचिन्हित Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_चळत.काढा_back();
}

शून्य JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  अचिन्हित Indent = getFileIndent();
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
  जर (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  जर (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  तार्किक UseOverlayRelative = असत्य;
  जर (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  जर (!Entries.empty()) {
    स्थिरांक YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    जर (UseOverlayRelative) {
      प्रतिज्ञा(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    तार्किक IsCurrentDirEmpty = सत्य;
    जर (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = असत्य;
    }

    साठी (स्थिरांक स्वतः &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      जर (Dir == Dir_चळत.back()) {
        जर (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } अन्यथा {
        तार्किक Is_Dir_Popped_From_चळत = असत्य;
        पर्यंत (!Dir_चळत.empty() && !containedIn(Dir_चळत.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_चळत = सत्य;
        }
        जर (Is_Dir_Popped_From_चळत || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = सत्य;
      }
      StringRef RPath = Entry.RPath;
      जर (UseOverlayRelative) {
        प्रतिज्ञा(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      जर (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = असत्य;
      }
    }

    पर्यंत (!Dir_चळत.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

शून्य YAMLVFSWriter::लिख(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](स्थिरांक YAMLVFSEntry &LHS, स्थिरांक YAMLVFSEntry &RHS) {
    परत LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).लिख(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, स्थिरांक Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  जर (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->चळत.ढकला_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  प्रतिज्ञा(FS && State && !State->चळत.empty() && "incrementing past end");
  प्रतिज्ञा(!State->चळत.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  जर (State->Has_No_ढकला_Request)
    State->Has_No_ढकला_Request = असत्य;
  अन्यथा {
    जर (State->चळत.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->चळत.back()->path(), EC);
      जर (I != End) {
        State->चळत.ढकला_back(I);
        परत *हे;
      }
    }
  }

  पर्यंत (!State->चळत.empty() && State->चळत.back().increment(EC) == End)
    State->चळत.काढा_back();

  जर (State->चळत.empty())
    State.reset(); // end iterator

  परत *हे;
}

स्थिरांक अक्षर FileSystem::ID = 0;
स्थिरांक अक्षर OverlayFileSystem::ID = 0;
स्थिरांक अक्षर ProxyFileSystem::ID = 0;
स्थिरांक अक्षर InMemoryFileSystem::ID = 0;
स्थिरांक अक्षर RedirectingFileSystem::ID = 0;

अचिन्हित ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_मोजणी(
    स्थिरांक FileSystem *FS) {
  परत FS->Use_मोजणी();
}

शून्य ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

शून्य ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
