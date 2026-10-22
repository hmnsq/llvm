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

Status::Status(अचर file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(अचर Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      Size(Size), Type(Type), Perms(Perms) {}

Status Status::copyWithNewSize(अचर Status &In, uint64_t NewSize) {
  फर्काऊ Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), NewSize, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(अचर Status &In, अचर Twine &NewName) {
  फर्काऊ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(अचर file_status &In, अचर Twine &NewName) {
  फर्काऊ Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

तार्किक Status::equivalent(अचर Status &Other) अचर {
  प्रतिज्ञा(isStatusKnown() && Other.isStatusKnown());
  फर्काऊ getUniqueID() == Other.getUniqueID();
}

तार्किक Status::isDirectory() अचर { फर्काऊ Type == file_type::directory_file; }

तार्किक Status::isRegularFile() अचर { फर्काऊ Type == file_type::regular_file; }

तार्किक Status::isOther() अचर {
  फर्काऊ exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

तार्किक Status::isSymlink() अचर { फर्काऊ Type == file_type::symlink_file; }

तार्किक Status::isStatusKnown() अचर { फर्काऊ Type != file_type::status_error; }

तार्किक Status::exists() अचर {
  फर्काऊ isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = स्वभाव;

FileSystem::~FileSystem() = स्वभाव;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(अचर llvm::Twine &Name, int64_t FileSize,
                             तार्किक RequiresNullTerminator, तार्किक IsVolatile,
                             तार्किक IsText) {
  स्वतः F = IsText ? openFileForRead(Name) : open_File_For_Read_द्विआधारी(Name);
  यदि (!F)
    फर्काऊ F.getError();

  फर्काऊ (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<वर्ण> &Path) अचर {
  यदि (llvm::sys::path::is_absolute(Path))
    फर्काऊ {};

  स्वतः WorkingDir = getCurrentWorkingDirectory();
  यदि (!WorkingDir)
    फर्काऊ WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.get(), Path);
  फर्काऊ {};
}

std::error_code FileSystem::getRealPath(अचर Twine &Path,
                                        SmallVectorImpl<वर्ण> &Output) {
  फर्काऊ errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(अचर Twine &Path, तार्किक &Result) {
  फर्काऊ errc::operation_not_permitted;
}

तार्किक FileSystem::exists(अचर Twine &Path) {
  स्वतः Status = status(Path);
  फर्काऊ Status && Status->exists();
}

llvm::ErrorOr<तार्किक> FileSystem::equivalent(अचर Twine &A, अचर Twine &B) {
  स्वतः StatusA = status(A);
  यदि (!StatusA)
    फर्काऊ StatusA.getError();
  स्वतः StatusB = status(B);
  यदि (!StatusB)
    फर्काऊ StatusB.getError();
  फर्काऊ StatusA->equivalent(*StatusB);
}

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
शून्य FileSystem::dump() अचर { print(dbgs(), PrintType::RecursiveContents); }
#endif

#ifndef न_संशोधन
स्थिर तार्किक isTraversalComponent(StringRef Component) {
  फर्काऊ Component == ".." || Component == ".";
}

स्थिर तार्किक pathHasTraversal(StringRef Path) {
  using नामाकाश llvm::sys;

  लागि (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    यदि (isTraversalComponent(Comp))
      फर्काऊ सत्य;
  फर्काऊ असत्य;
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
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(अचर Twine &Name,
                                                   int64_t FileSize,
                                                   तार्किक RequiresNullTerminator,
                                                   तार्किक IsVolatile) override;
  std::error_code बंद() override;
  शून्य setPath(अचर Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { बंद(); }

ErrorOr<Status> RealFile::status() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(FD.isValid() && "cannot stat closed file");
  यदि (!S.isStatusKnown()) {
    file_status RealStatus;
    यदि (std::error_code EC = sys::fs::status(FD, RealStatus))
      फर्काऊ EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  फर्काऊ S;
}

ErrorOr<std::string> RealFile::getName() {
  फर्काऊ RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(अचर Twine &Name, int64_t FileSize,
                    तार्किक RequiresNullTerminator, तार्किक IsVolatile) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(FD.isValid() && "cannot get buffer for closed file");
  फर्काऊ MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::बंद() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  फर्काऊ EC;
}

शून्य RealFile::setPath(अचर Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  यदि (स्वतः Status = status())
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
    यदि (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      यदि (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      अन्यथा यदि (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      अन्यथा
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(अचर Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(अचर Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_द्विआधारी(अचर Twine &Path) override;
  directory_iterator dir_begin(अचर Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() अचर override;
  std::error_code setCurrentWorkingDirectory(अचर Twine &Path) override;
  std::error_code isLocal(अचर Twine &Path, तार्किक &Result) override;
  शून्य
  getDirectoryContentRealSources(अचर Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.थप_back(Dir.str());
  }
  std::error_code getRealPath(अचर Twine &Path,
                              SmallVectorImpl<वर्ण> &Output) override;

रक्षित:
  शून्य printImpl(raw_ostream &OS, PrintType Type,
                 अचिन्हित IndentLevel) अचर override;

गुप्त:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(अचर Twine &Path, SmallVectorImpl<वर्ण> &Storage) अचर {
    यदि (!WD || !*WD)
      फर्काऊ Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->get().Resolved, Storage);
    फर्काऊ Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(अचर Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    यदि (!FDOrErr)
      फर्काऊ errorToErrorCode(FDOrErr.takeError());
    फर्काऊ std::unique_ptr<File>(
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

ErrorOr<Status> RealFileSystem::status(अचर Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  यदि (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    फर्काऊ EC;
  फर्काऊ Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(अचर Twine &Name) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  फर्काऊ openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_द्विआधारी(अचर Twine &Name) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  फर्काऊ openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() अचर {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  यदि (WD && *WD)
    फर्काऊ std::string(WD->get().Specified);
  यदि (WD)
    फर्काऊ WD->getError();

  SmallString<128> Dir;
  यदि (std::error_code EC = llvm::sys::fs::current_path(Dir))
    फर्काऊ EC;
  फर्काऊ std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(अचर Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  यदि (!WD)
    फर्काऊ llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  तार्किक IsDir;
  यदि (स्वतः Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    फर्काऊ Err;
  यदि (!IsDir)
    फर्काऊ std::make_error_code(std::errc::not_a_directory);
  यदि (स्वतः Err = llvm::sys::fs::real_path(Absolute, Resolved))
    फर्काऊ Err;
  WD = WorkingDirectory{Absolute, Resolved};
  फर्काऊ std::error_code();
}

std::error_code RealFileSystem::isLocal(अचर Twine &Path, तार्किक &Result) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  फर्काऊ llvm::sys::fs::is_local(adjustPath(Path, Storage), Result);
}

std::error_code RealFileSystem::getRealPath(अचर Twine &Path,
                                            SmallVectorImpl<वर्ण> &Output) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  फर्काऊ llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

शून्य RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               अचिन्हित IndentLevel) अचर {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  यदि (WD)
    OS << "own";
  अन्यथा
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  स्थिर IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(सत्य);
  sys::sandbox::violationIfEnabled();

  फर्काऊ FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  फर्काऊ std::make_unique<RealFileSystem>(असत्य);
}

नामाकाश {

वर्ग RealFSDirIter : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

सार्वजनिक:
  RealFSDirIter(अचर Twine &Path, std::error_code &EC) {
    स्वतः BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    यदि (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    स्वतः BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    फर्काऊ EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(अचर Twine &Dir,
                                             std::error_code &EC) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  फर्काऊ directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.थप_back(std::move(BaseFS));
}

शून्य OverlayFileSystem::थप_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.थप_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status> OverlayFileSystem::status(अचर Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  लागि (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    यदि (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      फर्काऊ Status;
  }
  फर्काऊ make_error_code(llvm::errc::no_such_file_or_directory);
}

तार्किक OverlayFileSystem::exists(अचर Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  लागि (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    यदि ((*I)->exists(Path))
      फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(अचर llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  लागि (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    स्वतः Result = (*I)->openFileForRead(Path);
    यदि (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      फर्काऊ Result;
  }
  फर्काऊ make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() अचर {
  // All file systems are synchronized, just take the first working directory.
  फर्काऊ FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(अचर Twine &Path) {
  लागि (स्वतः &FS : FSList)
    यदि (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      फर्काऊ EC;
  फर्काऊ {};
}

std::error_code OverlayFileSystem::isLocal(अचर Twine &Path, तार्किक &Result) {
  लागि (स्वतः &FS : FSList)
    यदि (FS->exists(Path))
      फर्काऊ FS->isLocal(Path, Result);
  फर्काऊ errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(अचर Twine &Path,
                                               SmallVectorImpl<वर्ण> &Output) {
  लागि (अचर स्वतः &FS : FSList)
    यदि (FS->exists(Path))
      फर्काऊ FS->getRealPath(Path, Output);
  फर्काऊ errc::no_such_file_or_directory;
}

शून्य OverlayFileSystem::getDirectoryContentRealSources(
    अचर Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  लागि (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

शून्य OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  लागि (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

शून्य OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  अचिन्हित IndentLevel) अचर {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  यदि (Type == PrintType::Summary)
    फर्काऊ;

  यदि (Type == PrintType::Contents)
    Type = PrintType::Summary;
  लागि (अचर स्वतः &FS : overlays_range())
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
    जबसम्म (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.झिक_back();
      यदि (CurrentDirIter != directory_iterator())
        रोक; // found
    }

    यदि (IsFirstTime && CurrentDirIter == directory_iterator())
      फर्काऊ errc::no_such_file_or_directory;
    फर्काऊ {};
  }

  std::error_code incrementDirIter(तार्किक IsFirstTime) {
    प्रतिज्ञा((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    यदि (!IsFirstTime)
      CurrentDirIter.increment(EC);
    यदि (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    फर्काऊ EC;
  }

  std::error_code incrementImpl(तार्किक IsFirstTime) {
    जबसम्म (सत्य) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      यदि (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        फर्काऊ EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      यदि (SeenNames.घुसाउ(Name).second)
        फर्काऊ EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

सार्वजनिक:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    लागि (अचर स्वतः &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      यदि (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        फर्काऊ;
      }
      यदि (!FEC)
        IterList.थप_back(Iter);
    }
    EC = incrementImpl(सत्य);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(सत्य);
  }

  std::error_code increment() override { फर्काऊ incrementImpl(असत्य); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(अचर Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  यदि (EC)
    फर्काऊ {};
  फर्काऊ Combined;
}

शून्य ProxyFileSystem::anchor() {}

नामाकाश llvm {
नामाकाश vfs {

नामाकाश detail {

गणना In_Memory_गाँठो_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
वर्ग In_Memory_गाँठो {
  In_Memory_गाँठो_Kind Kind;
  std::string FileName;

सार्वजनिक:
  In_Memory_गाँठो(llvm::StringRef FileName, In_Memory_गाँठो_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  आभास ~In_Memory_गाँठो() = स्वभाव;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  आभास Status getStatus(अचर Twine &RequestedName) अचर = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() अचर { फर्काऊ FileName; }
  In_Memory_गाँठो_Kind getKind() अचर { फर्काऊ Kind; }
  आभास std::string toString(अचिन्हित Indent) अचर = 0;
};

वर्ग InMemoryFile : सार्वजनिक In_Memory_गाँठो {
  Status Stat;
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

सार्वजनिक:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
      : In_Memory_गाँठो(Stat.getName(), IME_File), Stat(std::move(Stat)),
        Buffer(std::move(Buffer)) {}

  Status getStatus(अचर Twine &RequestedName) अचर override {
    फर्काऊ Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::MemoryBuffer *getBuffer() अचर { फर्काऊ Buffer.get(); }

  std::string toString(अचिन्हित Indent) अचर override {
    फर्काऊ (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  स्थिर तार्किक classof(अचर In_Memory_गाँठो *N) {
    फर्काऊ N->getKind() == IME_File;
  }
};

नामाकाश {

वर्ग InMemoryHardLink : सार्वजनिक In_Memory_गाँठो {
  अचर InMemoryFile &ResolvedFile;

सार्वजनिक:
  InMemoryHardLink(StringRef Path, अचर InMemoryFile &ResolvedFile)
      : In_Memory_गाँठो(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  अचर InMemoryFile &getResolvedFile() अचर { फर्काऊ ResolvedFile; }

  Status getStatus(अचर Twine &RequestedName) अचर override {
    फर्काऊ ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(अचिन्हित Indent) अचर override {
    फर्काऊ std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  स्थिर तार्किक classof(अचर In_Memory_गाँठो *N) {
    फर्काऊ N->getKind() == IME_HardLink;
  }
};

वर्ग InMemorySymbolicLink : सार्वजनिक In_Memory_गाँठो {
  std::string TargetPath;
  Status Stat;

सार्वजनिक:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_गाँठो(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(अचिन्हित Indent) अचर override {
    फर्काऊ std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(अचर Twine &RequestedName) अचर override {
    फर्काऊ Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() अचर { फर्काऊ TargetPath; }

  स्थिर तार्किक classof(अचर In_Memory_गाँठो *N) {
    फर्काऊ N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
वर्ग InMemoryFileAdaptor : सार्वजनिक File {
  अचर InMemoryFile &गाँठो;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

सार्वजनिक:
  explicit InMemoryFileAdaptor(अचर InMemoryFile &गाँठो,
                               std::string RequestedName)
      : गाँठो(गाँठो), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    फर्काऊ गाँठो.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(अचर Twine &Name, int64_t FileSize, तार्किक RequiresNullTerminator,
            तार्किक IsVolatile) override {
    llvm::MemoryBuffer *Buf = गाँठो.getBuffer();
    फर्काऊ llvm::MemoryBuffer::getMemBuffer(
        Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code बंद() override { फर्काऊ {}; }

  शून्य setPath(अचर Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

वर्ग InMemoryDirectory : सार्वजनिक In_Memory_गाँठो {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_गाँठो>, std::less<>> Entries;

सार्वजनिक:
  InMemoryDirectory(Status Stat)
      : In_Memory_गाँठो(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(अचर Twine &RequestedName) अचर override {
    फर्काऊ Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() अचर { फर्काऊ Stat.getUniqueID(); }

  In_Memory_गाँठो *getChild(StringRef Name) अचर {
    स्वतः I = Entries.find(Name);
    यदि (I != Entries.end())
      फर्काऊ I->second.get();
    फर्काऊ nullptr;
  }

  In_Memory_गाँठो *addChild(StringRef Name, std::unique_ptr<In_Memory_गाँठो> Child) {
    फर्काऊ Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() अचर { फर्काऊ Entries.begin(); }
  const_iterator end() अचर { फर्काऊ Entries.end(); }

  std::string toString(अचिन्हित Indent) अचर override {
    std::string Result =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    लागि (अचर स्वतः &Entry : Entries)
      Result += Entry.second->toString(Indent + 2);
    फर्काऊ Result;
  }

  स्थिर तार्किक classof(अचर In_Memory_गाँठो *N) {
    फर्काऊ N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
स्थिर sys::fs::UniqueID getUniqueID(hash_code Hash) {
  फर्काऊ sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(माप_प्रकार(Hash)));
}
स्थिर sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  फर्काऊ getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
स्थिर sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  फर्काऊ getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_गाँठो_Info::makeStatus() अचर {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, Buffer ? Buffer->getBuffer() : "");

  फर्काऊ Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, Buffer ? Buffer->getBufferSize() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(तार्किक UseNormalizedPaths)
    : जरा_2(नूतन detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = स्वभाव;

std::string InMemoryFileSystem::toString() अचर {
  फर्काऊ जरा_2->toString(/*Indent=*/0);
}

तार्किक InMemoryFileSystem::addFile(अचर Twine &P, समय_प्रकार ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_गाँठो_Fn Make_गाँठो) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  यदि (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सत्य);

  यदि (Path.empty())
    फर्काऊ असत्य;

  detail::InMemoryDirectory *Dir = जरा_2.get();
  स्वतः I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  अचर स्वतः ResolvedUser = User.मान_or(0);
  अचर स्वतः ResolvedGroup = Group.मान_or(0);
  अचर स्वतः ResolvedType = Type.मान_or(sys::fs::file_type::regular_file);
  अचर स्वतः ResolvedPerms = Perms.मान_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  अचर स्वतः NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  जबसम्म (सत्य) {
    Name = *I;
    ++I;
    यदि (I == E)
      रोक;
    detail::In_Memory_गाँठो *गाँठो = Dir->getChild(Name);
    यदि (!गाँठो) {
      // This isn't the last element, so we create a new directory.
      Status Stat(
          StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::toTimePoint(ModificationTime), ResolvedUser, ResolvedGroup,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::InMemoryDirectory>(Dir->addChild(
          Name, std::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      जारी;
    }
    // Creating file under another file.
    यदि (!isa<detail::InMemoryDirectory>(गाँठो))
      फर्काऊ असत्य;
    Dir = cast<detail::InMemoryDirectory>(गाँठो);
  }
  detail::In_Memory_गाँठो *गाँठो = Dir->getChild(Name);
  यदि (!गाँठो) {
    Dir->addChild(Name,
                  Make_गाँठो({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(Buffer), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    फर्काऊ सत्य;
  }
  यदि (isa<detail::InMemoryDirectory>(गाँठो))
    फर्काऊ ResolvedType == sys::fs::file_type::directory_file;

  प्रतिज्ञा((isa<detail::InMemoryFile>(गाँठो) ||
          isa<detail::InMemoryHardLink>(गाँठो)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  यदि (स्वतः *Link = dyn_cast<detail::InMemoryHardLink>(गाँठो)) {
    फर्काऊ Link->getResolvedFile().getBuffer()->getBuffer() ==
           Buffer->getBuffer();
  }
  फर्काऊ cast<detail::InMemoryFile>(गाँठो)->getBuffer()->getBuffer() ==
         Buffer->getBuffer();
}

तार्किक InMemoryFileSystem::addFile(अचर Twine &P, समय_प्रकार ModificationTime,
                                 std::unique_ptr<llvm::MemoryBuffer> Buffer,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  फर्काऊ addFile(P, ModificationTime, std::move(Buffer), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_गाँठो_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_गाँठो> {
                   Status Stat = NNI.makeStatus();
                   यदि (Stat.getType() == sys::fs::file_type::directory_file)
                     फर्काऊ std::make_unique<detail::InMemoryDirectory>(Stat);
                   फर्काऊ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

तार्किक InMemoryFileSystem::addFileNoOwn(
    अचर Twine &P, समय_प्रकार ModificationTime,
    अचर llvm::MemoryBufferRef &Buffer, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  फर्काऊ addFile(P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_गाँठो_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_गाँठो> {
                   Status Stat = NNI.makeStatus();
                   यदि (Stat.getType() == sys::fs::file_type::directory_file)
                     फर्काऊ std::make_unique<detail::InMemoryDirectory>(Stat);
                   फर्काऊ std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.Buffer));
                 });
}

detail::Named_गाँठो_Or_Error
InMemoryFileSystem::lookup_गाँठो(अचर Twine &P, तार्किक FollowFinalSymlink,
                               माप_प्रकार SymlinkDepth) अचर {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  यदि (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सत्य);

  अचर detail::InMemoryDirectory *Dir = जरा_2.get();
  यदि (Path.empty())
    फर्काऊ detail::Named_गाँठो_Or_Error(Path, Dir);

  स्वतः I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  जबसम्म (सत्य) {
    detail::In_Memory_गाँठो *गाँठो = Dir->getChild(*I);
    ++I;
    यदि (!गाँठो)
      फर्काऊ errc::no_such_file_or_directory;

    यदि (स्वतः Symlink = dyn_cast<detail::InMemorySymbolicLink>(गाँठो)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      यदि (I == E && !FollowFinalSymlink)
        फर्काऊ detail::Named_गाँठो_Or_Error(Path, Symlink);

      यदि (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        फर्काऊ errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      यदि (std::error_code EC = makeAbsolute(TargetPath))
        फर्काऊ EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      स्वतः Target =
          lookup_गाँठो(TargetPath, /*FollowFinalSymlink=*/सत्य, SymlinkDepth + 1);
      यदि (!Target || I == E)
        फर्काऊ Target;

      यदि (!isa<detail::InMemoryDirectory>(*Target))
        फर्काऊ errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      जारी;
    }

    // Return the file if it's at the end of the path.
    यदि (स्वतः File = dyn_cast<detail::InMemoryFile>(गाँठो)) {
      यदि (I == E)
        फर्काऊ detail::Named_गाँठो_Or_Error(Path, File);
      फर्काऊ errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    यदि (स्वतः File = dyn_cast<detail::InMemoryHardLink>(गाँठो)) {
      यदि (I == E)
        फर्काऊ detail::Named_गाँठो_Or_Error(Path, &File->getResolvedFile());
      फर्काऊ errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(गाँठो);
    यदि (I == E)
      फर्काऊ detail::Named_गाँठो_Or_Error(Path, Dir);
  }
}

तार्किक InMemoryFileSystem::addHardLink(अचर Twine &NewLink,
                                     अचर Twine &Target) {
  स्वतः New_Link_गाँठो = lookup_गाँठो(NewLink, /*FollowFinalSymlink=*/असत्य);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  स्वतः Target_गाँठो = lookup_गाँठो(Target, /*FollowFinalSymlink=*/सत्य);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  यदि (!Target_गाँठो || New_Link_गाँठो || !isa<detail::InMemoryFile>(*Target_गाँठो))
    फर्काऊ असत्य;
  फर्काऊ addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_गाँठो_Info NNI) {
                   फर्काऊ std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_गाँठो));
                 });
}

तार्किक InMemoryFileSystem::addSymbolicLink(
    अचर Twine &NewLink, अचर Twine &Target, समय_प्रकार ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  स्वतः New_Link_गाँठो = lookup_गाँठो(NewLink, /*FollowFinalSymlink=*/असत्य);
  यदि (New_Link_गाँठो)
    फर्काऊ असत्य;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  फर्काऊ addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_गाँठो_Info NNI) {
                   फर्काऊ std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(अचर Twine &Path) {
  स्वतः गाँठो = lookup_गाँठो(Path, /*FollowFinalSymlink=*/सत्य);
  यदि (गाँठो)
    फर्काऊ (*गाँठो)->getStatus(Path);
  फर्काऊ गाँठो.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(अचर Twine &Path) {
  स्वतः गाँठो = lookup_गाँठो(Path,/*FollowFinalSymlink=*/सत्य);
  यदि (!गाँठो)
    फर्काऊ गाँठो.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  यदि (स्वतः *F = dyn_cast<detail::InMemoryFile>(*गाँठो))
    फर्काऊ std::unique_ptr<File>(
        नूतन detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  फर्काऊ make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
वर्ग InMemoryFileSystem::DirIterator : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  अचर InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  शून्य setCurrentEntry() {
    यदि (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      विकल्प (I->second->getKind()) {
      पक्ष detail::IME_File:
      पक्ष detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        रोक;
      पक्ष detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        रोक;
      पक्ष detail::IME_SymbolicLink:
        यदि (स्वतः SymlinkTarget =
                FS->lookup_गाँठो(Path, /*FollowFinalSymlink=*/सत्य)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        रोक;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } अन्यथा {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

सार्वजनिक:
  DirIterator() = स्वभाव;

  DirIterator(अचर InMemoryFileSystem *FS,
              अचर detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    फर्काऊ {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(अचर Twine &Dir,
                                                 std::error_code &EC) {
  स्वतः गाँठो = lookup_गाँठो(Dir, /*FollowFinalSymlink=*/सत्य);
  यदि (!गाँठो) {
    EC = गाँठो.getError();
    फर्काऊ directory_iterator(std::make_shared<DirIterator>());
  }

  यदि (स्वतः *Dir_गाँठो = dyn_cast<detail::InMemoryDirectory>(*गाँठो))
    फर्काऊ directory_iterator(
        std::make_shared<DirIterator>(एतत्, *Dir_गाँठो, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  फर्काऊ directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(अचर Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  यदि (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/सत्य);

  यदि (!Path.empty())
    WorkingDirectory = std::string(Path);
  फर्काऊ {};
}

std::error_code InMemoryFileSystem::getRealPath(अचर Twine &Path,
                                                SmallVectorImpl<वर्ण> &Output) {
  स्वतः CWD = getCurrentWorkingDirectory();
  यदि (!CWD || CWD->empty())
    फर्काऊ errc::operation_not_permitted;
  Path.toVector(Output);
  यदि (स्वतः EC = makeAbsolute(Output))
    फर्काऊ EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/सत्य);
  फर्काऊ {};
}

std::error_code InMemoryFileSystem::isLocal(अचर Twine &Path, तार्किक &Result) {
  Result = असत्य;
  फर्काऊ {};
}

शून्य InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   अचिन्हित IndentLevel) अचर {
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
  अचर माप_प्रकार n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  यदि (n != static_cast<माप_प्रकार>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  फर्काऊ style;
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
  फर्काऊ result;
}

/// Whether the error and entry specify a file/directory that was not found.
स्थिर तार्किक isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  यदि (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    फर्काऊ असत्य;
  फर्काऊ EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  प्रतिज्ञा(ExternalFS && "RedirectingFileSystem requires an external FS");
  यदि (स्वतः ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
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
    यदि (!IsFirstTime)
      ++Current;
    यदि (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      विकल्प ((*Current)->getKind()) {
      पक्ष RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      पक्ष RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        रोक;
      पक्ष RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        रोक;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } अन्यथा {
      CurrentEntry = directory_entry();
    }
    फर्काऊ {};
  };

सार्वजनिक:
  RedirectingFSDirIterImpl(
      अचर Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/सत्य);
  }

  std::error_code increment() override {
    फर्काऊ incrementImpl(/*IsFirstTime=*/असत्य);
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
    यदि (ExternalIter != llvm::vfs::directory_iterator())
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
    यदि (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    अन्यथा
      CurrentEntry = directory_entry();
    फर्काऊ EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() अचर {
  फर्काऊ WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(अचर Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  यदि (!exists(Path))
    फर्काऊ errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  यदि (std::error_code EC = makeAbsolute(AbsolutePath))
    फर्काऊ EC;
  WorkingDirectory = std::string(AbsolutePath);
  फर्काऊ {};
}

std::error_code RedirectingFileSystem::isLocal(अचर Twine &Path_,
                                               तार्किक &Result) {
  SmallString<256> Path;
  Path_.toVector(Path);

  यदि (makeAbsolute(Path))
    फर्काऊ {};

  फर्काऊ ExternalFS->isLocal(Path, Result);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<वर्ण> &Path) अचर {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  यदि (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    फर्काऊ {};

  स्वतः WorkingDir = getCurrentWorkingDirectory();
  यदि (!WorkingDir)
    फर्काऊ WorkingDir.getError();

  फर्काऊ makeAbsolute(WorkingDir.get(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<वर्ण> &Path) अचर {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  यदि (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    फर्काऊ std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  यदि (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } अन्यथा {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    यदि (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string Result = std::string(WorkingDir);
  StringRef Dir(Result);
  यदि (!Dir.ends_with(sys::path::get_separator(style))) {
    Result += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  Result.append(Path.data(), Path.size());
  Path.assign(Result.begin(), Result.end());

  फर्काऊ {};
}

directory_iterator RedirectingFileSystem::dir_begin(अचर Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  यदि (EC)
    फर्काऊ {};

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  यदि (!Result) {
    यदि (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(Result.getError()))
      फर्काऊ ExternalFS->dir_begin(Path, EC);

    EC = Result.getError();
    फर्काऊ {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *Result);
  यदि (!S) {
    यदि (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), Result->E))
      फर्काऊ ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    फर्काऊ {};
  }

  यदि (!S->isDirectory()) {
    EC = errc::not_a_directory;
    फर्काऊ {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  यदि (स्वतः ExtRedirect = Result->getExternalRedirect()) {
    स्वतः RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    यदि (!RE->useExternalName(UseExternalNames)) {
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

  यदि (RedirectEC) {
    यदि (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      फर्काऊ {};
    }
    RedirectIter = {};
  }

  यदि (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    फर्काऊ RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  यदि (ExternalEC) {
    यदि (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      फर्काऊ {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  विकल्प (Redirection) {
  पक्ष RedirectKind::Fallthrough:
    Iters.थप_back(ExternalIter);
    Iters.थप_back(RedirectIter);
    रोक;
  पक्ष RedirectKind::Fallback:
    Iters.थप_back(RedirectIter);
    Iters.थप_back(ExternalIter);
    रोक;
  स्वभाव:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  यदि (EC)
    फर्काऊ {};
  फर्काऊ Combined;
}

शून्य RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() अचर {
  फर्काऊ OverlayFileDir;
}

शून्य RedirectingFileSystem::setFallthrough(तार्किक Fallthrough) {
  यदि (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } अन्यथा {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

शून्य RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() अचर {
  std::vector<StringRef> R;
  R.reserve(Roots.size());
  लागि (अचर स्वतः &जरा_2 : Roots)
    R.थप_back(जरा_2->getName());
  फर्काऊ R;
}

शून्य RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      अचिन्हित IndentLevel) अचर {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  यदि (Type == PrintType::Summary)
    फर्काऊ;

  लागि (अचर स्वतः &जरा_2 : Roots)
    printEntry(OS, जरा_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

शून्य RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       अचिन्हित IndentLevel) अचर {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  विकल्प (E->getKind()) {
  पक्ष EK_Directory: {
    स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    लागि (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.get(), IndentLevel + 1);
    रोक;
  }
  पक्ष EK_DirectoryRemap:
  पक्ष EK_File: {
    स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    विकल्प (RE->getUseName()) {
    पक्ष NK_NotSet:
      रोक;
    पक्ष NK_External:
      OS << " (UseExternalName: true)";
      रोक;
    पक्ष NK_Virtual:
      OS << " (UseExternalName: false)";
      रोक;
    }
    OS << "\n";
    रोक;
  }
  }
}

शून्य RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  यदि (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
वर्ग llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  शून्य error(yaml::गाँठो *N, अचर Twine &सन्देश_2) { Stream.printError(N, सन्देश_2); }

  // false on error
  तार्किक parseScalarString(yaml::गाँठो *N, StringRef &Result,
                         SmallVectorImpl<वर्ण> &Storage) {
    अचर स्वतः *S = dyn_cast_if_present<yaml::Scalar_गाँठो>(N);

    यदि (!S) {
      error(N, "expected string");
      फर्काऊ असत्य;
    }
    Result = S->get_मान(Storage);
    फर्काऊ सत्य;
  }

  // false on error
  तार्किक parseScalarBool(yaml::गाँठो *N, तार्किक &Result) {
    SmallString<5> Storage;
    StringRef मान;
    यदि (!parseScalarString(N, मान, Storage))
      फर्काऊ असत्य;

    यदि (मान.equals_insensitive("true") || मान.equals_insensitive("on") ||
        मान.equals_insensitive("yes") || मान == "1") {
      Result = सत्य;
      फर्काऊ सत्य;
    } अन्यथा यदि (मान.equals_insensitive("false") ||
               मान.equals_insensitive("off") ||
               मान.equals_insensitive("no") || मान == "0") {
      Result = असत्य;
      फर्काऊ सत्य;
    }

    error(N, "expected boolean value");
    फर्काऊ असत्य;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::गाँठो *N) {
    SmallString<12> Storage;
    StringRef मान;
    यदि (!parseScalarString(N, मान, Storage))
      फर्काऊ std::nullopt;

    यदि (मान.equals_insensitive("fallthrough")) {
      फर्काऊ RedirectingFileSystem::RedirectKind::Fallthrough;
    } अन्यथा यदि (मान.equals_insensitive("fallback")) {
      फर्काऊ RedirectingFileSystem::RedirectKind::Fallback;
    } अन्यथा यदि (मान.equals_insensitive("redirect-only")) {
      फर्काऊ RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    फर्काऊ std::nullopt;
  }

  std::optional<RedirectingFileSystem::जरा_Relative_Kind>
  parse_जरा_Relative_Kind(yaml::गाँठो *N) {
    SmallString<12> Storage;
    StringRef मान;
    यदि (!parseScalarString(N, मान, Storage))
      फर्काऊ std::nullopt;
    यदि (मान.equals_insensitive("cwd")) {
      फर्काऊ RedirectingFileSystem::जरा_Relative_Kind::CWD;
    } अन्यथा यदि (मान.equals_insensitive("overlay-dir")) {
      फर्काऊ RedirectingFileSystem::जरा_Relative_Kind::OverlayDir;
    }
    फर्काऊ std::nullopt;
  }

  संरचना साँचो_Status {
    तार्किक Required;
    तार्किक Seen = असत्य;

    साँचो_Status(तार्किक Required = असत्य) : Required(Required) {}
  };

  using साँचो_Status_Pair = std::pair<StringRef, साँचो_Status>;

  // false on error
  तार्किक check_Duplicate_Or_Unknown_साँचो(yaml::गाँठो *साँचो_गाँठो, StringRef साँचो,
                                  DenseMap<StringRef, साँचो_Status> &साँचोहरू_2) {
    स्वतः It = साँचोहरू_2.find(साँचो);
    यदि (It == साँचोहरू_2.end()) {
      error(साँचो_गाँठो, "unknown key");
      फर्काऊ असत्य;
    }
    साँचो_Status &S = It->second;
    यदि (S.Seen) {
      error(साँचो_गाँठो, Twine("duplicate key '") + साँचो + "'");
      फर्काऊ असत्य;
    }
    S.Seen = सत्य;
    फर्काऊ सत्य;
  }

  // false on error
  तार्किक check_Missing_साँचोहरू(yaml::गाँठो *Obj, DenseMap<StringRef, साँचो_Status> &साँचोहरू_2) {
    लागि (अचर स्वतः &I : साँचोहरू_2) {
      यदि (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        फर्काऊ असत्य;
      }
    }
    फर्काऊ सत्य;
  }

सार्वजनिक:
  स्थिर RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    यदि (!ParentEntry) { // Look for a existent root
      लागि (अचर स्वतः &जरा_2 : FS->Roots) {
        यदि (Name == जरा_2->getName()) {
          ParentEntry = जरा_2.get();
          फर्काऊ ParentEntry;
        }
      }
    } अन्यथा { // Advance to the next component
      स्वतः *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      लागि (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        स्वतः *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.get());
        यदि (DirContent && Name == Content->getName())
          फर्काऊ DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", getNextVirtualUniqueID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    यदि (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.थप_back(std::move(E));
      ParentEntry = FS->Roots.back().get();
      फर्काऊ ParentEntry;
    }

    स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    फर्काऊ DE->getLastContent();
  }

गुप्त:
  शून्य unique_Overlay_रूख(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    विकल्प (SrcE->getKind()) {
    पक्ष RedirectingFileSystem::EK_Directory: {
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      यदि (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      लागि (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_रूख(FS, SubEntry.get(), NewParentE);
      रोक;
    }
    पक्ष RedirectingFileSystem::EK_DirectoryRemap: {
      प्रतिज्ञा(NewParentE && "Parent entry must exist");
      स्वतः *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      रोक;
    }
    पक्ष RedirectingFileSystem::EK_File: {
      प्रतिज्ञा(NewParentE && "Parent entry must exist");
      स्वतः *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      रोक;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::गाँठो *N, RedirectingFileSystem *FS, तार्किक Is_जरा_Entry) {
    स्वतः *M = dyn_cast<yaml::Mapping_गाँठो>(N);
    यदि (!M) {
      error(N, "expected mapping node for file or directory entry");
      फर्काऊ nullptr;
    }

    साँचो_Status_Pair Fields[] = {
        साँचो_Status_Pair("name", सत्य),
        साँचो_Status_Pair("type", सत्य),
        साँचो_Status_Pair("contents", असत्य),
        साँचो_Status_Pair("external-contents", असत्य),
        साँचो_Status_Pair("use-external-name", असत्य),
    };

    DenseMap<StringRef, साँचो_Status> साँचोहरू_2(std::begin(Fields), std::end(Fields));

    गणना { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::गाँठो *Name_मान_गाँठो = nullptr;
    स्वतः UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    लागि (स्वतः &I : *M) {
      StringRef साँचो;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> Buffer;
      यदि (!parseScalarString(I.get_साँचो(), साँचो, Buffer))
        फर्काऊ nullptr;

      यदि (!check_Duplicate_Or_Unknown_साँचो(I.get_साँचो(), साँचो, साँचोहरू_2))
        फर्काऊ nullptr;

      StringRef मान;
      यदि (साँचो == "name") {
        यदि (!parseScalarString(I.get_मान(), मान, Buffer))
          फर्काऊ nullptr;

        Name_मान_गाँठो = I.get_मान();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(मान).str();
      } अन्यथा यदि (साँचो == "type") {
        यदि (!parseScalarString(I.get_मान(), मान, Buffer))
          फर्काऊ nullptr;
        यदि (मान == "file")
          Kind = RedirectingFileSystem::EK_File;
        अन्यथा यदि (मान == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        अन्यथा यदि (मान == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        अन्यथा {
          error(I.get_मान(), "unknown value for 'type'");
          फर्काऊ nullptr;
        }
      } अन्यथा यदि (साँचो == "contents") {
        यदि (ContentsField != CF_NotSet) {
          error(I.get_साँचो(),
                "entry already has 'contents' or 'external-contents'");
          फर्काऊ nullptr;
        }
        ContentsField = CF_List;
        स्वतः *Contents = dyn_cast_if_present<yaml::Sequence_गाँठो>(I.get_मान());
        यदि (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_मान(), "expected array");
          फर्काऊ nullptr;
        }

        लागि (स्वतः &I : *Contents) {
          यदि (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ असत्य))
            EntryArrayContents.थप_back(std::move(E));
          अन्यथा
            फर्काऊ nullptr;
        }
      } अन्यथा यदि (साँचो == "external-contents") {
        यदि (ContentsField != CF_NotSet) {
          error(I.get_साँचो(),
                "entry already has 'contents' or 'external-contents'");
          फर्काऊ nullptr;
        }
        ContentsField = CF_External;
        यदि (!parseScalarString(I.get_मान(), मान, Buffer))
          फर्काऊ nullptr;

        SmallString<256> FullPath;
        यदि (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          प्रतिज्ञा(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = मान;
          यदि (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            फर्काऊ nullptr;
          }
          FullPath = AbsFullPath;
        } अन्यथा {
          FullPath = मान;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } अन्यथा यदि (साँचो == "use-external-name") {
        तार्किक Val;
        यदि (!parseScalarBool(I.get_मान(), Val))
          फर्काऊ nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } अन्यथा {
        llvm_unreachable("key missing from Keys");
      }
    }

    यदि (Stream.failed())
      फर्काऊ nullptr;

    // check for missing keys
    यदि (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      फर्काऊ nullptr;
    }
    यदि (!check_Missing_साँचोहरू(N, साँचोहरू_2))
      फर्काऊ nullptr;

    // check invalid configuration
    यदि (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      फर्काऊ nullptr;
    }

    यदि (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      फर्काऊ nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    यदि (Is_जरा_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      यदि (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } अन्यथा यदि (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } अन्यथा {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        यदि (FS->जरा_Relative ==
            RedirectingFileSystem::जरा_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          प्रतिज्ञा(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } अन्यथा {
          EC = FS->makeAbsolute(Name);
        }
        यदि (EC) {
          प्रतिज्ञा(Name_मान_गाँठो && "Name presence should be checked earlier");
          error(
              Name_मान_गाँठो,
              "entry with relative path at the root level is not discoverable");
          फर्काऊ nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      यदि (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    माप_प्रकार जरा_Path_Len = sys::path::जरा_path(Trimmed, path_style).size();
    जबसम्म (Trimmed.size() > जरा_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.size() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> Result;
    विकल्प (Kind) {
    पक्ष RedirectingFileSystem::EK_File:
      Result = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      रोक;
    पक्ष RedirectingFileSystem::EK_DirectoryRemap:
      Result = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      रोक;
    पक्ष RedirectingFileSystem::EK_Directory:
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      रोक;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    यदि (Parent.empty())
      फर्काऊ Result;

    // if 'name' contains multiple components, create implicit directory entries
    लागि (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.थप_back(std::move(Result));
      Result = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", getNextVirtualUniqueID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    फर्काऊ Result;
  }

सार्वजनिक:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  तार्किक parse(yaml::गाँठो *जरा_2, RedirectingFileSystem *FS) {
    स्वतः *शीर्ष = dyn_cast<yaml::Mapping_गाँठो>(जरा_2);
    यदि (!शीर्ष) {
      error(जरा_2, "expected mapping node");
      फर्काऊ असत्य;
    }

    साँचो_Status_Pair Fields[] = {
        साँचो_Status_Pair("version", सत्य),
        साँचो_Status_Pair("case-sensitive", असत्य),
        साँचो_Status_Pair("use-external-names", असत्य),
        साँचो_Status_Pair("root-relative", असत्य),
        साँचो_Status_Pair("overlay-relative", असत्य),
        साँचो_Status_Pair("fallthrough", असत्य),
        साँचो_Status_Pair("redirecting-with", असत्य),
        साँचो_Status_Pair("roots", सत्य),
    };

    DenseMap<StringRef, साँचो_Status> साँचोहरू_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> जरा_Entries;

    // Parse configuration and 'roots'
    लागि (स्वतः &I : *शीर्ष) {
      SmallString<10> साँचो_Buffer;
      StringRef साँचो;
      यदि (!parseScalarString(I.get_साँचो(), साँचो, साँचो_Buffer))
        फर्काऊ असत्य;

      यदि (!check_Duplicate_Or_Unknown_साँचो(I.get_साँचो(), साँचो, साँचोहरू_2))
        फर्काऊ असत्य;

      यदि (साँचो == "roots") {
        स्वतः *Roots = dyn_cast_if_present<yaml::Sequence_गाँठो>(I.get_मान());
        यदि (!Roots) {
          error(I.get_मान(), "expected array");
          फर्काऊ असत्य;
        }

        लागि (स्वतः &I : *Roots) {
          यदि (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ सत्य))
            जरा_Entries.थप_back(std::move(E));
          अन्यथा
            फर्काऊ असत्य;
        }
      } अन्यथा यदि (साँचो == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        यदि (!parseScalarString(I.get_मान(), VersionString, Storage))
          फर्काऊ असत्य;
        पूर्णाङ्क Version;
        यदि (VersionString.getAsInteger<पूर्णाङ्क>(10, Version)) {
          error(I.get_मान(), "expected integer");
          फर्काऊ असत्य;
        }
        यदि (Version < 0) {
          error(I.get_मान(), "invalid version number");
          फर्काऊ असत्य;
        }
        यदि (Version != 0) {
          error(I.get_मान(), "version mismatch, expected 0");
          फर्काऊ असत्य;
        }
      } अन्यथा यदि (साँचो == "case-sensitive") {
        यदि (!parseScalarBool(I.get_मान(), FS->CaseSensitive))
          फर्काऊ असत्य;
      } अन्यथा यदि (साँचो == "overlay-relative") {
        यदि (!parseScalarBool(I.get_मान(), FS->IsRelativeOverlay))
          फर्काऊ असत्य;
      } अन्यथा यदि (साँचो == "use-external-names") {
        यदि (!parseScalarBool(I.get_मान(), FS->UseExternalNames))
          फर्काऊ असत्य;
      } अन्यथा यदि (साँचो == "fallthrough") {
        यदि (साँचोहरू_2["redirecting-with"].Seen) {
          error(I.get_मान(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          फर्काऊ असत्य;
        }

        तार्किक ShouldFallthrough = असत्य;
        यदि (!parseScalarBool(I.get_मान(), ShouldFallthrough))
          फर्काऊ असत्य;

        यदि (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } अन्यथा {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } अन्यथा यदि (साँचो == "redirecting-with") {
        यदि (साँचोहरू_2["fallthrough"].Seen) {
          error(I.get_मान(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          फर्काऊ असत्य;
        }

        यदि (स्वतः Kind = parseRedirectKind(I.get_मान())) {
          FS->Redirection = *Kind;
        } अन्यथा {
          error(I.get_मान(), "expected valid redirect kind");
          फर्काऊ असत्य;
        }
      } अन्यथा यदि (साँचो == "root-relative") {
        यदि (स्वतः Kind = parse_जरा_Relative_Kind(I.get_मान())) {
          FS->जरा_Relative = *Kind;
        } अन्यथा {
          error(I.get_मान(), "expected valid root-relative kind");
          फर्काऊ असत्य;
        }
      } अन्यथा {
        llvm_unreachable("key missing from Keys");
      }
    }

    यदि (Stream.failed())
      फर्काऊ असत्य;

    यदि (!check_Missing_साँचोहरू(शीर्ष, साँचोहरू_2))
      फर्काऊ असत्य;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    लागि (स्वतः &E : जरा_Entries)
      unique_Overlay_रूख(FS, E.get());

    फर्काऊ सत्य;
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
  yaml::गाँठो *जरा_2 = DI->get_जरा();
  यदि (DI == Stream.end() || !जरा_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    फर्काऊ nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      नूतन RedirectingFileSystem(ExternalFS));

  यदि (!YAMLFilePath.empty()) {
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

  यदि (!P.parse(जरा_2, FS.get()))
    फर्काऊ nullptr;

  फर्काऊ FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    तार्किक UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      नूतन RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  लागि (स्वतः &Mapping : llvm::reverse(RemappedFiles)) {
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
    यदि (ToEntry)
      जारी;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    लागि (स्वतः I = llvm::sys::path::begin(FromDirectory),
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

  फर्काऊ FS;
}

RedirectingFileSystem::LookupResult::LookupResult(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  प्रतिज्ञा(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  यदि (स्वतः *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

शून्य RedirectingFileSystem::LookupResult::getPath(
    llvm::SmallVectorImpl<वर्ण> &Result) अचर {
  Result.clear();
  लागि (Entry *Parent : Parents)
    llvm::sys::path::append(Result, Parent->getName());
  llvm::sys::path::append(Result, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<वर्ण> &Path) अचर {
  यदि (std::error_code EC = makeAbsolute(Path))
    फर्काऊ EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.data(), Path.size()));
  यदि (CanonicalPath.empty())
    फर्काऊ make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  फर्काऊ {};
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPath(StringRef Path) अचर {
  llvm::SmallString<128> CanonicalPath(Path);
  यदि (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    फर्काऊ EC;

  // RedirectOnly means the VFS is always used.
  यदि (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = सत्य;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  लागि (अचर स्वतः &जरा_2 : Roots) {
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, जरा_2.get(), Entries);
    यदि (UsageTrackingActive && Result && isa<RemapEntry>(Result->E))
      HasBeenUsed = सत्य;
    यदि (Result) {
      Result->Parents = std::move(Entries);
      फर्काऊ Result;
    }

    यदि (Result.getError() != llvm::errc::no_such_file_or_directory)
      फर्काऊ Result;
  }
  फर्काऊ make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::LookupResult>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) अचर {
  प्रतिज्ञा(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  यदि (!FromName.empty()) {
    यदि (!pathComponentMatches(*Start, FromName))
      फर्काऊ make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    यदि (Start == End) {
      // Match!
      फर्काऊ LookupResult(From, Start, End);
    }
  }

  यदि (isa<RedirectingFileSystem::FileEntry>(From))
    फर्काऊ make_error_code(llvm::errc::not_a_directory);

  यदि (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    फर्काऊ LookupResult(From, Start, End);

  स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  लागि (अचर std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.थप_back(From);
    ErrorOr<RedirectingFileSystem::LookupResult> Result =
        lookupPathImpl(Start, End, DirEntry.get(), Entries);
    यदि (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      फर्काऊ Result;
    Entries.झिक_back();
  }

  फर्काऊ make_error_code(llvm::errc::no_such_file_or_directory);
}

स्थिर Status getRedirectedFileStatus(अचर Twine &OriginalPath,
                                      तार्किक UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  यदि (ExternalStatus.ExposesExternalVFSPath)
    फर्काऊ ExternalStatus;

  Status S = ExternalStatus;
  यदि (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  अन्यथा
    S.ExposesExternalVFSPath = सत्य;
  फर्काऊ S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    अचर Twine &LookupPath, अचर Twine &OriginalPath,
    अचर RedirectingFileSystem::LookupResult &Result) {
  यदि (std::optional<StringRef> ExtRedirect = Result.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    यदि (std::error_code EC = makeAbsolute(RemappedPath))
      फर्काऊ EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    यदि (!S)
      फर्काऊ S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(Result.E);
    फर्काऊ getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(Result.E);
  फर्काऊ Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(अचर Twine &LookupPath,
                                         अचर Twine &OriginalPath) अचर {
  स्वतः Result = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  यदि (!Result || Result->ExposesExternalVFSPath)
    फर्काऊ Result;
  फर्काऊ Status::copyWithNewName(Result.get(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(अचर Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  यदि (std::error_code EC = makeAbsolute(Path))
    फर्काऊ EC;

  यदि (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    यदि (S)
      फर्काऊ S;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  यदि (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    यदि (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      फर्काऊ getExternalStatus(Path, OriginalPath);
    फर्काऊ Result.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *Result);
  यदि (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), Result->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    फर्काऊ getExternalStatus(Path, OriginalPath);
  }

  फर्काऊ S;
}

तार्किक RedirectingFileSystem::exists(अचर Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  यदि (makeAbsolute(Path))
    फर्काऊ असत्य;

  यदि (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    यदि (ExternalFS->exists(Path))
      फर्काऊ सत्य;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  यदि (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    यदि (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      फर्काऊ ExternalFS->exists(Path);
    फर्काऊ असत्य;
  }

  std::optional<StringRef> ExtRedirect = Result->getExternalRedirect();
  यदि (!ExtRedirect) {
    प्रतिज्ञा(isa<RedirectingFileSystem::DirectoryEntry>(Result->E));
    फर्काऊ सत्य;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  यदि (makeAbsolute(RemappedPath))
    फर्काऊ असत्य;

  यदि (ExternalFS->exists(RemappedPath))
    फर्काऊ सत्य;

  यदि (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    फर्काऊ ExternalFS->exists(Path);
  }

  फर्काऊ असत्य;
}

नामाकाश {

/// Provide a file wrapper with an overriden status.
वर्ग FileWithFixedStatus : सार्वजनिक File {
  std::unique_ptr<File> InnerFile;
  Status S;

सार्वजनिक:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { फर्काऊ S; }
  ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>

  getBuffer(अचर Twine &Name, int64_t FileSize, तार्किक RequiresNullTerminator,
            तार्किक IsVolatile) override {
    फर्काऊ InnerFile->getBuffer(Name, FileSize, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code बंद() override { फर्काऊ InnerFile->बंद(); }

  शून्य setPath(अचर Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> Result, अचर Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  यदि (!Result || (*Result)->status()->ExposesExternalVFSPath)
    फर्काऊ Result;

  ErrorOr<std::unique_ptr<File>> F = std::move(*Result);
  स्वतः Name = F->get()->getName();
  यदि (Name && Name.get() != P.str())
    F->get()->setPath(P);
  फर्काऊ F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(अचर Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  यदि (std::error_code EC = makeAbsolute(Path))
    फर्काऊ EC;

  यदि (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    स्वतः F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    यदि (F)
      फर्काऊ F;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  यदि (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    यदि (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      फर्काऊ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    फर्काऊ Result.getError();
  }

  यदि (!Result->getExternalRedirect()) // FIXME: errc::not_a_file?
    फर्काऊ make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *Result->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  यदि (std::error_code EC = makeAbsolute(RemappedPath))
    फर्काऊ EC;

  स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(Result->E);

  स्वतः ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  यदि (!ExternalFile) {
    यदि (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      फर्काऊ File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    फर्काऊ ExternalFile;
  }

  स्वतः ExternalStatus = (*ExternalFile)->status();
  यदि (!ExternalStatus)
    फर्काऊ ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  फर्काऊ std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(अचर Twine &OriginalPath,
                                   SmallVectorImpl<वर्ण> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  यदि (std::error_code EC = makeAbsolute(Path))
    फर्काऊ EC;

  यदि (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    यदि (!EC)
      फर्काऊ EC;
  }

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  यदि (!Result) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    यदि (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(Result.getError()))
      फर्काऊ ExternalFS->getRealPath(Path, Output);
    फर्काऊ Result.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  यदि (स्वतः ExtRedirect = Result->getExternalRedirect()) {
    स्वतः P = ExternalFS->getRealPath(*ExtRedirect, Output);
    यदि (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, Result->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      फर्काऊ ExternalFS->getRealPath(Path, Output);
    }
    फर्काऊ P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  यदि (Redirection == RedirectKind::Fallthrough) {
    Result->getPath(Output);
    फर्काऊ {};
  }
  फर्काऊ llvm::errc::invalid_argument;
}

शून्य RedirectingFileSystem::getDirectoryContentRealSources(
    अचर Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  यदि (makeAbsolute(Path))
    फर्काऊ;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  अचर तार्किक ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::LookupResult> Result = lookupPath(Path);
  यदि (!Result) {
    // dir_begin() delegates entirely to ExternalFS.
    यदि (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    फर्काऊ;
  }

  विकल्प (Result->E->getKind()) {
  पक्ष EK_File:
    फर्काऊ;
  पक्ष EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    रोक;
  पक्ष EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*Result->getExternalRedirect());
    यदि (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    रोक;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  यदि (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<MemoryBuffer> Buffer,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, शून्य *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  फर्काऊ RedirectingFileSystem::create(std::move(Buffer), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

स्थिर शून्य getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  स्वतः Kind = SrcE->getKind();
  यदि (Kind == RedirectingFileSystem::EK_Directory) {
    स्वतः *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    प्रतिज्ञा(DE && "Must be a directory");
    लागि (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.थप_back(SubEntry->getName());
      getVFSEntries(SubEntry.get(), Path, Entries);
      Path.झिक_back();
    }
    फर्काऊ;
  }

  यदि (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    स्वतः *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    प्रतिज्ञा(DR && "Must be a directory remap");
    SmallString<128> VPath;
    लागि (स्वतः &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.थप_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    फर्काऊ;
  }

  प्रतिज्ञा(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  स्वतः *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  प्रतिज्ञा(FE && "Must be a file");
  SmallString<128> VPath;
  लागि (स्वतः &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.थप_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

शून्य vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::LookupResult> जरा_Result = VFS.lookupPath("/");
  यदि (!जरा_Result)
    फर्काऊ;
  SmallVector<StringRef, 8> Components;
  Components.थप_back("/");
  getVFSEntries(जरा_Result->E, Components, CollectedEntries);
}

UniqueID vfs::getNextVirtualUniqueID() {
  स्थिर std::atomic<अचिन्हित> UID;
  अचिन्हित ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  फर्काऊ UniqueID(std::numeric_limits<uint64_t>::max(), ID);
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
  SmallVector<StringRef, 16> Dir_थाक;

  अचिन्हित getDirIndent() { फर्काऊ 4 * Dir_थाक.size(); }
  अचिन्हित getFileIndent() { फर्काऊ 4 * (Dir_थाक.size() + 1); }
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
  लागि (स्वतः IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    यदि (*IParent != *IChild)
      फर्काऊ असत्य;
  }
  // Have we exhausted the parent path?
  फर्काऊ IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  प्रतिज्ञा(!Parent.empty());
  प्रतिज्ञा(containedIn(Parent, Path));
  फर्काऊ Path.substr(Parent.size() + 1);
}

शून्य JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_थाक.empty() ? Path : containedPart(Dir_थाक.back(), Path);
  Dir_थाक.थप_back(Path);
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

  Dir_थाक.झिक_back();
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
  यदि (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  यदि (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  तार्किक UseOverlayRelative = असत्य;
  यदि (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  यदि (!Entries.empty()) {
    अचर YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    यदि (UseOverlayRelative) {
      प्रतिज्ञा(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.size());
    }

    तार्किक IsCurrentDirEmpty = सत्य;
    यदि (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = असत्य;
    }

    लागि (अचर स्वतः &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      यदि (Dir == Dir_थाक.back()) {
        यदि (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } अन्यथा {
        तार्किक Is_Dir_Popped_From_थाक = असत्य;
        जबसम्म (!Dir_थाक.empty() && !containedIn(Dir_थाक.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_थाक = सत्य;
        }
        यदि (Is_Dir_Popped_From_थाक || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = सत्य;
      }
      StringRef RPath = Entry.RPath;
      यदि (UseOverlayRelative) {
        प्रतिज्ञा(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.size());
      }
      यदि (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = असत्य;
      }
    }

    जबसम्म (!Dir_थाक.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

शून्य YAMLVFSWriter::लिख(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](अचर YAMLVFSEntry &LHS, अचर YAMLVFSEntry &RHS) {
    फर्काऊ LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).लिख(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, अचर Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  यदि (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->थाक.थप_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  प्रतिज्ञा(FS && State && !State->थाक.empty() && "incrementing past end");
  प्रतिज्ञा(!State->थाक.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  यदि (State->Has_No_थप_Request)
    State->Has_No_थप_Request = असत्य;
  अन्यथा {
    यदि (State->थाक.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->थाक.back()->path(), EC);
      यदि (I != End) {
        State->थाक.थप_back(I);
        फर्काऊ *एतत्;
      }
    }
  }

  जबसम्म (!State->थाक.empty() && State->थाक.back().increment(EC) == End)
    State->थाक.झिक_back();

  यदि (State->थाक.empty())
    State.reset(); // end iterator

  फर्काऊ *एतत्;
}

अचर वर्ण FileSystem::ID = 0;
अचर वर्ण OverlayFileSystem::ID = 0;
अचर वर्ण ProxyFileSystem::ID = 0;
अचर वर्ण InMemoryFileSystem::ID = 0;
अचर वर्ण RedirectingFileSystem::ID = 0;

अचिन्हित ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_गन्ती(
    अचर FileSystem *FS) {
  फर्काऊ FS->Use_गन्ती();
}

शून्य ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

शून्य ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
