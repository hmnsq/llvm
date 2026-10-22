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

#समावेश "llvm/Support/VirtualFileSystem.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/DenseMap.h"
#समावेश "llvm/ADT/IntrusiveRefCntPtr.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/StringSet.h"
#समावेश "llvm/ADT/Twine.h"
#समावेश "llvm/ADT/iterator_range.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/Casting.h"
#समावेश "llvm/Support/Chrono.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/ErrorOr.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/FileSystem/UniqueID.h"
#समावेश "llvm/Support/IOSandbox.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/SMLoc.h"
#समावेश "llvm/Support/SourceMgr.h"
#समावेश "llvm/Support/YAMLParser.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <atomic>
#समावेश <cassert>
#समावेश <cstdint>
#समावेश <iterator>
#समावेश <limits>
#समावेश <map>
#समावेश <memory>
#समावेश <optional>
#समावेश <string>
#समावेश <system_error>
#समावेश <utility>
#समावेश <vector>

using namespace llvm;
using namespace llvm::vfs;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_t;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(स्थिर file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), आकार(Status.get_आकार()),
      Type(Status.type()), Perms(Status.permissions()) {}

Status::Status(स्थिर Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t आकार, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), Group(Group),
      आकार(आकार), Type(Type), Perms(Perms) {}

Status Status::copy_With_New_आकार(स्थिर Status &In, uint64_t New_आकार) {
  वापस Status(In.getName(), In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), New_आकार, In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(स्थिर Status &In, स्थिर Twine &NewName) {
  वापस Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.get_आकार(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(स्थिर file_status &In, स्थिर Twine &NewName) {
  वापस Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.get_आकार(), In.type(),
                In.permissions());
}

bool Status::equivalent(स्थिर Status &Other) स्थिर {
  निश्चित(isStatusKnown() && Other.isStatusKnown());
  वापस getUniqueID() == Other.getUniqueID();
}

bool Status::isDirectory() स्थिर { वापस Type == file_type::directory_file; }

bool Status::isRegularFile() स्थिर { वापस Type == file_type::regular_file; }

bool Status::isOther() स्थिर {
  वापस exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

bool Status::isSymlink() स्थिर { वापस Type == file_type::symlink_file; }

bool Status::isStatusKnown() स्थिर { वापस Type != file_type::status_error; }

bool Status::exists() स्थिर {
  वापस isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = शेष;

FileSystem::~FileSystem() = शेष;

ErrorOr<std::unique_ptr<Memory_बफर>>
FileSystem::get_बफर_For_File(स्थिर llvm::Twine &Name, int64_t File_आकार,
                             bool RequiresNullTerminator, bool IsVolatile,
                             bool IsText) {
  स्वतः F = IsText ? openFileForRead(Name) : open_File_For_Read_द्विआधारी(Name);
  अगर (!F)
    वापस F.getError();

  वापस (*F)->get_बफर(Name, File_आकार, RequiresNullTerminator, IsVolatile);
}

std::error_code FileSystem::makeAbsolute(SmallVectorImpl<अक्षर> &Path) स्थिर {
  अगर (llvm::sys::path::is_absolute(Path))
    वापस {};

  स्वतः WorkingDir = getCurrentWorkingDirectory();
  अगर (!WorkingDir)
    वापस WorkingDir.getError();

  sys::path::make_absolute(WorkingDir.लो(), Path);
  वापस {};
}

std::error_code FileSystem::getRealPath(स्थिर Twine &Path,
                                        SmallVectorImpl<अक्षर> &Output) {
  वापस errc::operation_not_permitted;
}

std::error_code FileSystem::isLocal(स्थिर Twine &Path, bool &परिणाम) {
  वापस errc::operation_not_permitted;
}

bool FileSystem::exists(स्थिर Twine &Path) {
  स्वतः Status = status(Path);
  वापस Status && Status->exists();
}

llvm::ErrorOr<bool> FileSystem::equivalent(स्थिर Twine &A, स्थिर Twine &B) {
  स्वतः StatusA = status(A);
  अगर (!StatusA)
    वापस StatusA.getError();
  स्वतः StatusB = status(B);
  अगर (!StatusB)
    वापस StatusB.getError();
  वापस StatusA->equivalent(*StatusB);
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
व्योम FileSystem::dump() स्थिर { print(dbgs(), PrintType::RecursiveContents); }
#पूर्ण_अगर

#अगर_अघोषित न_संशोधन
अटल bool isTraversalComponent(StringRef Component) {
  वापस Component == ".." || Component == ".";
}

अटल bool pathHasTraversal(StringRef Path) {
  using namespace llvm::sys;

  क्रम (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    अगर (isTraversalComponent(Comp))
      वापस true;
  वापस false;
}
#पूर्ण_अगर

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

namespace {

/// Wrapper around a raw file descriptor.
श्रेणी RealFile : खुला File {
  मित्र श्रेणी RealFileSystem;

  file_t FD;
  Status S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef NewRealPathName)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    निश्चित(FD.isValid() && "Invalid or inactive file descriptor");
  }

खुला:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<Memory_बफर>> get_बफर(स्थिर Twine &Name,
                                                   int64_t File_आकार,
                                                   bool RequiresNullTerminator,
                                                   bool IsVolatile) override;
  std::error_code बंद() override;
  व्योम setPath(स्थिर Twine &Path) override;
};

} // namespace

RealFile::~RealFile() { बंद(); }

ErrorOr<Status> RealFile::status() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  निश्चित(FD.isValid() && "cannot stat closed file");
  अगर (!S.isStatusKnown()) {
    file_status RealStatus;
    अगर (std::error_code EC = sys::fs::status(FD, RealStatus))
      वापस EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  वापस S;
}

ErrorOr<std::string> RealFile::getName() {
  वापस RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<Memory_बफर>>
RealFile::get_बफर(स्थिर Twine &Name, int64_t File_आकार,
                    bool RequiresNullTerminator, bool IsVolatile) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  निश्चित(FD.isValid() && "cannot get buffer for closed file");
  वापस Memory_बफर::getOpenFile(FD, Name, File_आकार, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::बंद() {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  std::error_code EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  वापस EC;
}

व्योम RealFile::setPath(स्थिर Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  RealName = Path.str();
  अगर (स्वतः Status = status())
    S = Status.लो().copyWithNewName(Status.लो(), Path);
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
श्रेणी RealFileSystem : खुला FileSystem {
खुला:
  explicit RealFileSystem(bool LinkCWDToProcess) {
    अगर (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      अगर (std::error_code EC = llvm::sys::fs::current_path(PWD))
        WD = std::move(EC);
      अन्यथा अगर (llvm::sys::fs::real_path(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      अन्यथा
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<Status> status(स्थिर Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(स्थिर Twine &Path) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_द्विआधारी(स्थिर Twine &Path) override;
  directory_iterator dir_begin(स्थिर Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() स्थिर override;
  std::error_code setCurrentWorkingDirectory(स्थिर Twine &Path) override;
  std::error_code isLocal(स्थिर Twine &Path, bool &परिणाम) override;
  व्योम
  getDirectoryContentRealSources(स्थिर Twine &Dir,
                                 SmallVectorImpl<std::string> &Out) override {
    Out.धकेलो_back(Dir.str());
  }
  std::error_code getRealPath(स्थिर Twine &Path,
                              SmallVectorImpl<अक्षर> &Output) override;

रक्षित:
  व्योम printImpl(raw_ostream &OS, PrintType Type,
                 अचिन्हित IndentLevel) स्थिर override;

निजी:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjustPath(स्थिर Twine &Path, SmallVectorImpl<अक्षर> &Storage) स्थिर {
    अगर (!WD || !*WD)
      वापस Path;
    Path.toVector(Storage);
    sys::path::make_absolute(WD->लो().Resolved, Storage);
    वापस Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(स्थिर Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjustPath(Name, Storage), Flags, &RealName);
    अगर (!FDOrErr)
      वापस errorToErrorCode(FDOrErr.takeError());
    वापस std::unique_ptr<File>(
        नया RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  काष्ठा WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(स्थिर Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_status RealStatus;
  अगर (std::error_code EC =
          sys::fs::status(adjustPath(Path, Storage), RealStatus))
    वापस EC;
  वापस Status::copyWithNewName(RealStatus, Path);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(स्थिर Twine &Name) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  वापस openFileForReadWithFlags(Name, sys::fs::OF_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_द्विआधारी(स्थिर Twine &Name) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  वापस openFileForReadWithFlags(Name, sys::fs::OF_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() स्थिर {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  अगर (WD && *WD)
    वापस std::string(WD->लो().Specified);
  अगर (WD)
    वापस WD->getError();

  SmallString<128> Dir;
  अगर (std::error_code EC = llvm::sys::fs::current_path(Dir))
    वापस EC;
  वापस std::string(Dir);
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(स्थिर Twine &Path) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  अगर (!WD)
    वापस llvm::sys::fs::set_current_path(Path);

  SmallString<128> Absolute, Resolved, Storage;
  adjustPath(Path, Storage).toVector(Absolute);
  bool IsDir;
  अगर (स्वतः Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    वापस Err;
  अगर (!IsDir)
    वापस std::make_error_code(std::errc::not_a_directory);
  अगर (स्वतः Err = llvm::sys::fs::real_path(Absolute, Resolved))
    वापस Err;
  WD = WorkingDirectory{Absolute, Resolved};
  वापस std::error_code();
}

std::error_code RealFileSystem::isLocal(स्थिर Twine &Path, bool &परिणाम) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  वापस llvm::sys::fs::is_local(adjustPath(Path, Storage), परिणाम);
}

std::error_code RealFileSystem::getRealPath(स्थिर Twine &Path,
                                            SmallVectorImpl<अक्षर> &Output) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  वापस llvm::sys::fs::real_path(adjustPath(Path, Storage), Output);
}

व्योम RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               अचिन्हित IndentLevel) स्थिर {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  अगर (WD)
    OS << "own";
  अन्यथा
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  अटल IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(true);
  sys::sandbox::violationIfEnabled();

  वापस FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  वापस std::make_unique<RealFileSystem>(false);
}

namespace {

श्रेणी RealFSDirIter : खुला llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

खुला:
  RealFSDirIter(स्थिर Twine &Path, std::error_code &EC) {
    स्वतः BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(Path, EC);
    अगर (Iter != sys::fs::directory_iterator())
      CurrentEntry = directory_entry(Iter->path(), Iter->type());
  }

  std::error_code increment() override {
    स्वतः BypassSandbox = sys::sandbox::scopedDisable();

    std::error_code EC;
    Iter.increment(EC);
    CurrentEntry = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_entry()
                       : directory_entry(Iter->path(), Iter->type());
    वापस EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(स्थिर Twine &Dir,
                                             std::error_code &EC) {
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  वापस directory_iterator(
      std::make_shared<RealFSDirIter>(adjustPath(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS) {
  FSList.धकेलो_back(std::move(BaseFS));
}

व्योम OverlayFileSystem::धकेलो_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.धकेलो_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().लो());
}

ErrorOr<Status> OverlayFileSystem::status(स्थिर Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  क्रम (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    ErrorOr<Status> Status = (*I)->status(Path);
    अगर (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      वापस Status;
  }
  वापस make_error_code(llvm::errc::no_such_file_or_directory);
}

bool OverlayFileSystem::exists(स्थिर Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  क्रम (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    अगर ((*I)->exists(Path))
      वापस true;
  }
  वापस false;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(स्थिर llvm::Twine &Path) {
  // FIXME: handle symlinks that cross file systems
  क्रम (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I) {
    स्वतः परिणाम = (*I)->openFileForRead(Path);
    अगर (परिणाम || परिणाम.getError() != llvm::errc::no_such_file_or_directory)
      वापस परिणाम;
  }
  वापस make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() स्थिर {
  // All file systems are synchronized, just take the first working directory.
  वापस FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(स्थिर Twine &Path) {
  क्रम (स्वतः &FS : FSList)
    अगर (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      वापस EC;
  वापस {};
}

std::error_code OverlayFileSystem::isLocal(स्थिर Twine &Path, bool &परिणाम) {
  क्रम (स्वतः &FS : FSList)
    अगर (FS->exists(Path))
      वापस FS->isLocal(Path, परिणाम);
  वापस errc::no_such_file_or_directory;
}

std::error_code OverlayFileSystem::getRealPath(स्थिर Twine &Path,
                                               SmallVectorImpl<अक्षर> &Output) {
  क्रम (स्थिर स्वतः &FS : FSList)
    अगर (FS->exists(Path))
      वापस FS->getRealPath(Path, Output);
  वापस errc::no_such_file_or_directory;
}

व्योम OverlayFileSystem::getDirectoryContentRealSources(
    स्थिर Twine &Dir, SmallVectorImpl<std::string> &Out) {
  // All layers contribute.
  क्रम (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, Out);
}

व्योम OverlayFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  क्रम (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->visitChildFileSystems(Callback);
  }
}

व्योम OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  अचिन्हित IndentLevel) स्थिर {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  अगर (Type == PrintType::Summary)
    वापस;

  अगर (Type == PrintType::Contents)
    Type = PrintType::Summary;
  क्रम (स्थिर स्वतः &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = शेष;

namespace {

/// Combines and deduplicates directory entries across multiple file systems.
श्रेणी CombiningDirIterImpl : खुला llvm::vfs::detail::DirIterImpl {
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
    जबतक (!IterList.empty()) {
      CurrentDirIter = IterList.back();
      IterList.निकालो_back();
      अगर (CurrentDirIter != directory_iterator())
        अवरोध; // found
    }

    अगर (IsFirstTime && CurrentDirIter == directory_iterator())
      वापस errc::no_such_file_or_directory;
    वापस {};
  }

  std::error_code incrementDirIter(bool IsFirstTime) {
    निश्चित((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_code EC;
    अगर (!IsFirstTime)
      CurrentDirIter.increment(EC);
    अगर (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    वापस EC;
  }

  std::error_code incrementImpl(bool IsFirstTime) {
    जबतक (true) {
      std::error_code EC = incrementDirIter(IsFirstTime);
      अगर (EC || CurrentDirIter == directory_iterator()) {
        CurrentEntry = directory_entry();
        वापस EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.path());
      अगर (SeenNames.जोड़ो(Name).second)
        वापस EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

खुला:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_code &EC) {
    क्रम (स्थिर स्वतः &FS : FileSystems) {
      std::error_code FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      अगर (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        वापस;
      }
      अगर (!FEC)
        IterList.धकेलो_back(Iter);
    }
    EC = incrementImpl(true);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_code &EC)
      : IterList(DirIters) {
    EC = incrementImpl(true);
  }

  std::error_code increment() override { वापस incrementImpl(false); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(स्थिर Twine &Dir,
                                                std::error_code &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  अगर (EC)
    वापस {};
  वापस Combined;
}

व्योम ProxyFileSystem::anchor() {}

namespace llvm {
namespace vfs {

namespace detail {

क्रमागत In_Memory_गाँठ_Kind {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
श्रेणी In_Memory_गाँठ {
  In_Memory_गाँठ_Kind Kind;
  std::string FileName;

खुला:
  In_Memory_गाँठ(llvm::StringRef FileName, In_Memory_गाँठ_Kind Kind)
      : Kind(Kind), FileName(std::string(llvm::sys::path::filename(FileName))) {
  }
  भव ~In_Memory_गाँठ() = शेष;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  भव Status getStatus(स्थिर Twine &RequestedName) स्थिर = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() स्थिर { वापस FileName; }
  In_Memory_गाँठ_Kind getKind() स्थिर { वापस Kind; }
  भव std::string toString(अचिन्हित Indent) स्थिर = 0;
};

श्रेणी InMemoryFile : खुला In_Memory_गाँठ {
  Status Stat;
  std::unique_ptr<llvm::Memory_बफर> बफर;

खुला:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::Memory_बफर> बफर)
      : In_Memory_गाँठ(Stat.getName(), IME_File), Stat(std::move(Stat)),
        बफर(std::move(बफर)) {}

  Status getStatus(स्थिर Twine &RequestedName) स्थिर override {
    वापस Status::copyWithNewName(Stat, RequestedName);
  }
  llvm::Memory_बफर *get_बफर() स्थिर { वापस बफर.लो(); }

  std::string toString(अचिन्हित Indent) स्थिर override {
    वापस (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  अटल bool classof(स्थिर In_Memory_गाँठ *N) {
    वापस N->getKind() == IME_File;
  }
};

namespace {

श्रेणी InMemoryHardLink : खुला In_Memory_गाँठ {
  स्थिर InMemoryFile &ResolvedFile;

खुला:
  InMemoryHardLink(StringRef Path, स्थिर InMemoryFile &ResolvedFile)
      : In_Memory_गाँठ(Path, IME_HardLink), ResolvedFile(ResolvedFile) {}
  स्थिर InMemoryFile &getResolvedFile() स्थिर { वापस ResolvedFile; }

  Status getStatus(स्थिर Twine &RequestedName) स्थिर override {
    वापस ResolvedFile.getStatus(RequestedName);
  }

  std::string toString(अचिन्हित Indent) स्थिर override {
    वापस std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.toString(0);
  }

  अटल bool classof(स्थिर In_Memory_गाँठ *N) {
    वापस N->getKind() == IME_HardLink;
  }
};

श्रेणी InMemorySymbolicLink : खुला In_Memory_गाँठ {
  std::string TargetPath;
  Status Stat;

खुला:
  InMemorySymbolicLink(StringRef Path, StringRef TargetPath, Status Stat)
      : In_Memory_गाँठ(Path, IME_SymbolicLink), TargetPath(std::move(TargetPath)),
        Stat(Stat) {}

  std::string toString(अचिन्हित Indent) स्थिर override {
    वापस std::string(Indent, ' ') + "SymbolicLink to -> " + TargetPath;
  }

  Status getStatus(स्थिर Twine &RequestedName) स्थिर override {
    वापस Status::copyWithNewName(Stat, RequestedName);
  }

  StringRef getTargetPath() स्थिर { वापस TargetPath; }

  अटल bool classof(स्थिर In_Memory_गाँठ *N) {
    वापस N->getKind() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
श्रेणी InMemoryFileAdaptor : खुला File {
  स्थिर InMemoryFile &गाँठ;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

खुला:
  explicit InMemoryFileAdaptor(स्थिर InMemoryFile &गाँठ,
                               std::string RequestedName)
      : गाँठ(गाँठ), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<Status> status() override {
    वापस गाँठ.getStatus(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::Memory_बफर>>
  get_बफर(स्थिर Twine &Name, int64_t File_आकार, bool RequiresNullTerminator,
            bool IsVolatile) override {
    llvm::Memory_बफर *Buf = गाँठ.get_बफर();
    वापस llvm::Memory_बफर::get_Mem_बफर(
        Buf->get_बफर(), Buf->get_बफर_Identifier(), RequiresNullTerminator);
  }

  std::error_code बंद() override { वापस {}; }

  व्योम setPath(स्थिर Twine &Path) override { RequestedName = Path.str(); }
};
} // namespace

श्रेणी InMemoryDirectory : खुला In_Memory_गाँठ {
  Status Stat;
  std::map<std::string, std::unique_ptr<In_Memory_गाँठ>, std::less<>> Entries;

खुला:
  InMemoryDirectory(Status Stat)
      : In_Memory_गाँठ(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  Status getStatus(स्थिर Twine &RequestedName) स्थिर override {
    वापस Status::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() स्थिर { वापस Stat.getUniqueID(); }

  In_Memory_गाँठ *getChild(StringRef Name) स्थिर {
    स्वतः I = Entries.find(Name);
    अगर (I != Entries.end())
      वापस I->second.लो();
    वापस nullptr;
  }

  In_Memory_गाँठ *addChild(StringRef Name, std::unique_ptr<In_Memory_गाँठ> Child) {
    वापस Entries.emplace(Name, std::move(Child)).first->second.लो();
  }

  using const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() स्थिर { वापस Entries.begin(); }
  const_iterator end() स्थिर { वापस Entries.end(); }

  std::string toString(अचिन्हित Indent) स्थिर override {
    std::string परिणाम =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    क्रम (स्थिर स्वतः &Entry : Entries)
      परिणाम += Entry.second->toString(Indent + 2);
    वापस परिणाम;
  }

  अटल bool classof(स्थिर In_Memory_गाँठ *N) {
    वापस N->getKind() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
अटल sys::fs::UniqueID getUniqueID(hash_code Hash) {
  वापस sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(माप_प्रकार(Hash)));
}
अटल sys::fs::UniqueID getFileID(sys::fs::UniqueID Parent,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  वापस getUniqueID(llvm::hash_combine(Parent.getFile(), Name, Contents));
}
अटल sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID Parent,
                                        llvm::StringRef Name) {
  वापस getUniqueID(llvm::hash_combine(Parent.getFile(), Name));
}

Status detail::New_In_Memory_गाँठ_Info::makeStatus() स्थिर {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, बफर ? बफर->get_बफर() : "");

  वापस Status(Path, UID, llvm::sys::toTimePoint(ModificationTime), User,
                Group, बफर ? बफर->get_बफर_आकार() : 0, Type, Perms);
}

InMemoryFileSystem::InMemoryFileSystem(bool UseNormalizedPaths)
    : मूल_2(नया detail::InMemoryDirectory(
          Status("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

InMemoryFileSystem::~InMemoryFileSystem() = शेष;

std::string InMemoryFileSystem::toString() स्थिर {
  वापस मूल_2->toString(/*Indent=*/0);
}

bool InMemoryFileSystem::addFile(स्थिर Twine &P, समय_प्रकार ModificationTime,
                                 std::unique_ptr<llvm::Memory_बफर> बफर,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_गाँठ_Fn Make_गाँठ) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  निश्चित(!EC);
  (व्योम)EC;

  अगर (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  अगर (Path.empty())
    वापस false;

  detail::InMemoryDirectory *Dir = मूल_2.लो();
  स्वतः I = llvm::sys::path::begin(Path), E = sys::path::end(Path);
  स्थिर स्वतः ResolvedUser = User.मूल्य_or(0);
  स्थिर स्वतः ResolvedGroup = Group.मूल्य_or(0);
  स्थिर स्वतः ResolvedType = Type.मूल्य_or(sys::fs::file_type::regular_file);
  स्थिर स्वतः ResolvedPerms = Perms.मूल्य_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  स्थिर स्वतः NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  जबतक (true) {
    Name = *I;
    ++I;
    अगर (I == E)
      अवरोध;
    detail::In_Memory_गाँठ *गाँठ = Dir->getChild(Name);
    अगर (!गाँठ) {
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
    अगर (!isa<detail::InMemoryDirectory>(गाँठ))
      वापस false;
    Dir = cast<detail::InMemoryDirectory>(गाँठ);
  }
  detail::In_Memory_गाँठ *गाँठ = Dir->getChild(Name);
  अगर (!गाँठ) {
    Dir->addChild(Name,
                  Make_गाँठ({Dir->getUniqueID(), Path, Name, ModificationTime,
                            std::move(बफर), ResolvedUser, ResolvedGroup,
                            ResolvedType, ResolvedPerms}));
    वापस true;
  }
  अगर (isa<detail::InMemoryDirectory>(गाँठ))
    वापस ResolvedType == sys::fs::file_type::directory_file;

  निश्चित((isa<detail::InMemoryFile>(गाँठ) ||
          isa<detail::InMemoryHardLink>(गाँठ)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  अगर (स्वतः *Link = dyn_cast<detail::InMemoryHardLink>(गाँठ)) {
    वापस Link->getResolvedFile().get_बफर()->get_बफर() ==
           बफर->get_बफर();
  }
  वापस cast<detail::InMemoryFile>(गाँठ)->get_बफर()->get_बफर() ==
         बफर->get_बफर();
}

bool InMemoryFileSystem::addFile(स्थिर Twine &P, समय_प्रकार ModificationTime,
                                 std::unique_ptr<llvm::Memory_बफर> बफर,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> Group,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  वापस addFile(P, ModificationTime, std::move(बफर), User, Group, Type,
                 Perms,
                 [](detail::New_In_Memory_गाँठ_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_गाँठ> {
                   Status Stat = NNI.makeStatus();
                   अगर (Stat.getType() == sys::fs::file_type::directory_file)
                     वापस std::make_unique<detail::InMemoryDirectory>(Stat);
                   वापस std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.बफर));
                 });
}

bool InMemoryFileSystem::addFileNoOwn(
    स्थिर Twine &P, समय_प्रकार ModificationTime,
    स्थिर llvm::Memory_बफर_Ref &बफर, std::optional<uint32_t> User,
    std::optional<uint32_t> Group, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  वापस addFile(P, ModificationTime, llvm::Memory_बफर::get_Mem_बफर(बफर),
                 std::move(User), std::move(Group), std::move(Type),
                 std::move(Perms),
                 [](detail::New_In_Memory_गाँठ_Info NNI)
                     -> std::unique_ptr<detail::In_Memory_गाँठ> {
                   Status Stat = NNI.makeStatus();
                   अगर (Stat.getType() == sys::fs::file_type::directory_file)
                     वापस std::make_unique<detail::InMemoryDirectory>(Stat);
                   वापस std::make_unique<detail::InMemoryFile>(
                       Stat, std::move(NNI.बफर));
                 });
}

detail::Named_गाँठ_Or_Error
InMemoryFileSystem::lookup_गाँठ(स्थिर Twine &P, bool FollowFinalSymlink,
                               माप_प्रकार SymlinkDepth) स्थिर {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  निश्चित(!EC);
  (व्योम)EC;

  अगर (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  स्थिर detail::InMemoryDirectory *Dir = मूल_2.लो();
  अगर (Path.empty())
    वापस detail::Named_गाँठ_Or_Error(Path, Dir);

  स्वतः I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  जबतक (true) {
    detail::In_Memory_गाँठ *गाँठ = Dir->getChild(*I);
    ++I;
    अगर (!गाँठ)
      वापस errc::no_such_file_or_directory;

    अगर (स्वतः Symlink = dyn_cast<detail::InMemorySymbolicLink>(गाँठ)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      अगर (I == E && !FollowFinalSymlink)
        वापस detail::Named_गाँठ_Or_Error(Path, Symlink);

      अगर (SymlinkDepth > InMemoryFileSystem::MaxSymlinkDepth)
        वापस errc::no_such_file_or_directory;

      SmallString<128> TargetPath = Symlink->getTargetPath();
      अगर (std::error_code EC = makeAbsolute(TargetPath))
        वापस EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      स्वतः Target =
          lookup_गाँठ(TargetPath, /*FollowFinalSymlink=*/true, SymlinkDepth + 1);
      अगर (!Target || I == E)
        वापस Target;

      अगर (!isa<detail::InMemoryDirectory>(*Target))
        वापस errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::InMemoryDirectory>(*Target);
      जारी;
    }

    // Return the file if it's at the end of the path.
    अगर (स्वतः File = dyn_cast<detail::InMemoryFile>(गाँठ)) {
      अगर (I == E)
        वापस detail::Named_गाँठ_Or_Error(Path, File);
      वापस errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    अगर (स्वतः File = dyn_cast<detail::InMemoryHardLink>(गाँठ)) {
      अगर (I == E)
        वापस detail::Named_गाँठ_Or_Error(Path, &File->getResolvedFile());
      वापस errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(गाँठ);
    अगर (I == E)
      वापस detail::Named_गाँठ_Or_Error(Path, Dir);
  }
}

bool InMemoryFileSystem::addHardLink(स्थिर Twine &NewLink,
                                     स्थिर Twine &Target) {
  स्वतः New_Link_गाँठ = lookup_गाँठ(NewLink, /*FollowFinalSymlink=*/false);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  स्वतः Target_गाँठ = lookup_गाँठ(Target, /*FollowFinalSymlink=*/true);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  अगर (!Target_गाँठ || New_Link_गाँठ || !isa<detail::InMemoryFile>(*Target_गाँठ))
    वापस false;
  वापस addFile(NewLink, 0, nullptr, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_In_Memory_गाँठ_Info NNI) {
                   वापस std::make_unique<detail::InMemoryHardLink>(
                       NNI.Path.str(),
                       *cast<detail::InMemoryFile>(*Target_गाँठ));
                 });
}

bool InMemoryFileSystem::addSymbolicLink(
    स्थिर Twine &NewLink, स्थिर Twine &Target, समय_प्रकार ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> Group,
    std::optional<llvm::sys::fs::perms> Perms) {
  स्वतः New_Link_गाँठ = lookup_गाँठ(NewLink, /*FollowFinalSymlink=*/false);
  अगर (New_Link_गाँठ)
    वापस false;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.toVector(NewLinkStr);
  Target.toVector(TargetStr);

  वापस addFile(NewLinkStr, ModificationTime, nullptr, User, Group,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_In_Memory_गाँठ_Info NNI) {
                   वापस std::make_unique<detail::InMemorySymbolicLink>(
                       NewLinkStr, TargetStr, NNI.makeStatus());
                 });
}

llvm::ErrorOr<Status> InMemoryFileSystem::status(स्थिर Twine &Path) {
  स्वतः गाँठ = lookup_गाँठ(Path, /*FollowFinalSymlink=*/true);
  अगर (गाँठ)
    वापस (*गाँठ)->getStatus(Path);
  वापस गाँठ.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(स्थिर Twine &Path) {
  स्वतः गाँठ = lookup_गाँठ(Path,/*FollowFinalSymlink=*/true);
  अगर (!गाँठ)
    वापस गाँठ.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  अगर (स्वतः *F = dyn_cast<detail::InMemoryFile>(*गाँठ))
    वापस std::unique_ptr<File>(
        नया detail::InMemoryFileAdaptor(*F, Path.str()));

  // FIXME: errc::not_a_file?
  वापस make_error_code(llvm::errc::invalid_argument);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
श्रेणी InMemoryFileSystem::DirIterator : खुला llvm::vfs::detail::DirIterImpl {
  स्थिर InMemoryFileSystem *FS;
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;
  std::string RequestedDirName;

  व्योम setCurrentEntry() {
    अगर (I != E) {
      SmallString<256> Path(RequestedDirName);
      llvm::sys::path::append(Path, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      चयन (I->second->getKind()) {
      हाल detail::IME_File:
      हाल detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        अवरोध;
      हाल detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        अवरोध;
      हाल detail::IME_SymbolicLink:
        अगर (स्वतः SymlinkTarget =
                FS->lookup_गाँठ(Path, /*FollowFinalSymlink=*/true)) {
          Path = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->getStatus(Path).getType();
        }
        अवरोध;
      }
      CurrentEntry = directory_entry(std::string(Path), Type);
    } अन्यथा {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      CurrentEntry = directory_entry();
    }
  }

खुला:
  DirIterator() = शेष;

  DirIterator(स्थिर InMemoryFileSystem *FS,
              स्थिर detail::InMemoryDirectory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.end()),
        RequestedDirName(std::move(RequestedDirName)) {
    setCurrentEntry();
  }

  std::error_code increment() override {
    ++I;
    setCurrentEntry();
    वापस {};
  }
};

directory_iterator InMemoryFileSystem::dir_begin(स्थिर Twine &Dir,
                                                 std::error_code &EC) {
  स्वतः गाँठ = lookup_गाँठ(Dir, /*FollowFinalSymlink=*/true);
  अगर (!गाँठ) {
    EC = गाँठ.getError();
    वापस directory_iterator(std::make_shared<DirIterator>());
  }

  अगर (स्वतः *Dir_गाँठ = dyn_cast<detail::InMemoryDirectory>(*गाँठ))
    वापस directory_iterator(
        std::make_shared<DirIterator>(यह, *Dir_गाँठ, Dir.str()));

  EC = make_error_code(llvm::errc::not_a_directory);
  वापस directory_iterator(std::make_shared<DirIterator>());
}

std::error_code InMemoryFileSystem::setCurrentWorkingDirectory(स्थिर Twine &P) {
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  निश्चित(!EC);
  (व्योम)EC;

  अगर (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  अगर (!Path.empty())
    WorkingDirectory = std::string(Path);
  वापस {};
}

std::error_code InMemoryFileSystem::getRealPath(स्थिर Twine &Path,
                                                SmallVectorImpl<अक्षर> &Output) {
  स्वतः CWD = getCurrentWorkingDirectory();
  अगर (!CWD || CWD->empty())
    वापस errc::operation_not_permitted;
  Path.toVector(Output);
  अगर (स्वतः EC = makeAbsolute(Output))
    वापस EC;
  llvm::sys::path::remove_dots(Output, /*remove_dot_dot=*/true);
  वापस {};
}

std::error_code InMemoryFileSystem::isLocal(स्थिर Twine &Path, bool &परिणाम) {
  परिणाम = false;
  वापस {};
}

व्योम InMemoryFileSystem::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   अचिन्हित IndentLevel) स्थिर {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

namespace {

अटल llvm::sys::path::Style getExistingStyle(llvm::StringRef Path) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = llvm::sys::path::Style::native;
  स्थिर माप_प्रकार n = Path.find_first_of("/\\");
  // Can't distinguish between posix and windows_slash here.
  अगर (n != static_cast<माप_प्रकार>(-1))
    style = (Path[n] == '/') ? llvm::sys::path::Style::posix
                             : llvm::sys::path::Style::windows_backslash;
  वापस style;
}

/// Removes leading "./" as well as path components like ".." and ".".
अटल llvm::SmallString<256> canonicalize(llvm::StringRef Path) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::path::Style style = getExistingStyle(Path);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> परिणाम_2 =
      llvm::sys::path::remove_leading_dotslash(Path, style);
  llvm::sys::path::remove_dots(परिणाम_2, /*remove_dot_dot=*/true, style);
  वापस परिणाम_2;
}

/// Whether the error and entry specify a file/directory that was not found.
अटल bool isFileNotFound(std::error_code EC,
                           RedirectingFileSystem::Entry *E = nullptr) {
  अगर (E && !isa<RedirectingFileSystem::DirectoryRemapEntry>(E))
    वापस false;
  वापस EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  निश्चित(ExternalFS && "RedirectingFileSystem requires an external FS");
  अगर (स्वतः ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
श्रेणी llvm::vfs::RedirectingFSDirIterImpl
    : खुला llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::DirectoryEntry::iterator Current, End;

  std::error_code incrementImpl(bool IsFirstTime) {
    निश्चित((IsFirstTime || Current != End) && "cannot iterate past end");
    अगर (!IsFirstTime)
      ++Current;
    अगर (Current != End) {
      SmallString<128> PathStr(Dir);
      llvm::sys::path::append(PathStr, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      चयन ((*Current)->getKind()) {
      हाल RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      हाल RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        अवरोध;
      हाल RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        अवरोध;
      }
      CurrentEntry = directory_entry(std::string(PathStr), Type);
    } अन्यथा {
      CurrentEntry = directory_entry();
    }
    वापस {};
  };

खुला:
  RedirectingFSDirIterImpl(
      स्थिर Twine &Path, RedirectingFileSystem::DirectoryEntry::iterator Begin,
      RedirectingFileSystem::DirectoryEntry::iterator End, std::error_code &EC)
      : Dir(Path.str()), Current(Begin), End(End) {
    EC = incrementImpl(/*IsFirstTime=*/true);
  }

  std::error_code increment() override {
    वापस incrementImpl(/*IsFirstTime=*/false);
  }
};

namespace {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
श्रेणी RedirectingFSDirRemapIterImpl : खुला llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::path::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

खुला:
  RedirectingFSDirRemapIterImpl(std::string DirPath,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(DirPath)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    अगर (ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
  }

  व्योम setCurrentEntry() {
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
    अगर (!EC && ExternalIter != llvm::vfs::directory_iterator())
      setCurrentEntry();
    अन्यथा
      CurrentEntry = directory_entry();
    वापस EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() स्थिर {
  वापस WorkingDirectory;
}

std::error_code
RedirectingFileSystem::setCurrentWorkingDirectory(स्थिर Twine &Path) {
  // Don't change the working directory if the path doesn't exist.
  अगर (!exists(Path))
    वापस errc::no_such_file_or_directory;

  SmallString<128> AbsolutePath;
  Path.toVector(AbsolutePath);
  अगर (std::error_code EC = makeAbsolute(AbsolutePath))
    वापस EC;
  WorkingDirectory = std::string(AbsolutePath);
  वापस {};
}

std::error_code RedirectingFileSystem::isLocal(स्थिर Twine &Path_,
                                               bool &परिणाम) {
  SmallString<256> Path;
  Path_.toVector(Path);

  अगर (makeAbsolute(Path))
    वापस {};

  वापस ExternalFS->isLocal(Path, परिणाम);
}

std::error_code RedirectingFileSystem::makeAbsolute(SmallVectorImpl<अक्षर> &Path) स्थिर {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  अगर (llvm::sys::path::is_absolute(Path, llvm::sys::path::Style::posix) ||
      llvm::sys::path::is_absolute(Path,
                                   llvm::sys::path::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    वापस {};

  स्वतः WorkingDir = getCurrentWorkingDirectory();
  अगर (!WorkingDir)
    वापस WorkingDir.getError();

  वापस makeAbsolute(WorkingDir.लो(), Path);
}

std::error_code
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    SmallVectorImpl<अक्षर> &Path) स्थिर {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  अगर (!WorkingDir.empty() &&
      !sys::path::is_absolute(WorkingDir, sys::path::Style::posix) &&
      !sys::path::is_absolute(WorkingDir,
                              sys::path::Style::windows_backslash)) {
    वापस std::error_code();
  }
  sys::path::Style style = sys::path::Style::windows_backslash;
  अगर (sys::path::is_absolute(WorkingDir, sys::path::Style::posix)) {
    style = sys::path::Style::posix;
  } अन्यथा {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    अगर (getExistingStyle(WorkingDir) != sys::path::Style::windows_backslash)
      style = sys::path::Style::windows_slash;
  }

  std::string परिणाम = std::string(WorkingDir);
  StringRef Dir(परिणाम);
  अगर (!Dir.ends_with(sys::path::get_separator(style))) {
    परिणाम += sys::path::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  परिणाम.append(Path.तथ्य(), Path.आकार_2());
  Path.assign(परिणाम.begin(), परिणाम.end());

  वापस {};
}

directory_iterator RedirectingFileSystem::dir_begin(स्थिर Twine &Dir,
                                                    std::error_code &EC) {
  SmallString<256> Path;
  Dir.toVector(Path);

  EC = makeAbsolute(Path);
  अगर (EC)
    वापस {};

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookupPath(Path);
  अगर (!परिणाम) {
    अगर (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(परिणाम.getError()))
      वापस ExternalFS->dir_begin(Path, EC);

    EC = परिणाम.getError();
    वापस {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<Status> S = status(Path, Dir, *परिणाम);
  अगर (!S) {
    अगर (Redirection != RedirectKind::RedirectOnly &&
        isFileNotFound(S.getError(), परिणाम->E))
      वापस ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    वापस {};
  }

  अगर (!S->isDirectory()) {
    EC = errc::not_a_directory;
    वापस {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_code RedirectEC;
  अगर (स्वतः ExtRedirect = परिणाम->getExternalRedirect()) {
    स्वतः RE = cast<RedirectingFileSystem::RemapEntry>(परिणाम->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    अगर (!RE->useExternalName(UseExternalNames)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(Path), RedirectIter));
    }
  } अन्यथा {
    स्वतः DE = cast<DirectoryEntry>(परिणाम->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            Path, DE->contents_begin(), DE->contents_end(), RedirectEC));
  }

  अगर (RedirectEC) {
    अगर (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      वापस {};
    }
    RedirectIter = {};
  }

  अगर (Redirection == RedirectKind::RedirectOnly) {
    EC = RedirectEC;
    वापस RedirectIter;
  }

  std::error_code ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(Path, ExternalEC);
  अगर (ExternalEC) {
    अगर (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      वापस {};
    }
    ExternalIter = {};
  }

  SmallVector<directory_iterator, 2> Iters;
  चयन (Redirection) {
  हाल RedirectKind::Fallthrough:
    Iters.धकेलो_back(ExternalIter);
    Iters.धकेलो_back(RedirectIter);
    अवरोध;
  हाल RedirectKind::Fallback:
    Iters.धकेलो_back(RedirectIter);
    Iters.धकेलो_back(ExternalIter);
    अवरोध;
  शेष:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  अगर (EC)
    वापस {};
  वापस Combined;
}

व्योम RedirectingFileSystem::setOverlayFileDir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() स्थिर {
  वापस OverlayFileDir;
}

व्योम RedirectingFileSystem::setFallthrough(bool Fallthrough) {
  अगर (Fallthrough) {
    Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
  } अन्यथा {
    Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
  }
}

व्योम RedirectingFileSystem::setRedirection(
    RedirectingFileSystem::RedirectKind Kind) {
  Redirection = Kind;
}

std::vector<StringRef> RedirectingFileSystem::getRoots() स्थिर {
  std::vector<StringRef> R;
  R.reserve(Roots.आकार_2());
  क्रम (स्थिर स्वतः &मूल_2 : Roots)
    R.धकेलो_back(मूल_2->getName());
  वापस R;
}

व्योम RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      अचिन्हित IndentLevel) स्थिर {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (UseExternalNames ? "true" : "false") << ")\n";
  अगर (Type == PrintType::Summary)
    वापस;

  क्रम (स्थिर स्वतः &मूल_2 : Roots)
    printEntry(OS, मूल_2.लो(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

व्योम RedirectingFileSystem::printEntry(raw_ostream &OS,
                                       RedirectingFileSystem::Entry *E,
                                       अचिन्हित IndentLevel) स्थिर {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  चयन (E->getKind()) {
  हाल EK_Directory: {
    स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(E);

    OS << "\n";
    क्रम (std::unique_ptr<Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end()))
      printEntry(OS, SubEntry.लो(), IndentLevel + 1);
    अवरोध;
  }
  हाल EK_DirectoryRemap:
  हाल EK_File: {
    स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(E);
    OS << " -> '" << RE->getExternalContentsPath() << "'";
    चयन (RE->getUseName()) {
    हाल NK_NotSet:
      अवरोध;
    हाल NK_External:
      OS << " (UseExternalName: true)";
      अवरोध;
    हाल NK_Virtual:
      OS << " (UseExternalName: false)";
      अवरोध;
    }
    OS << "\n";
    अवरोध;
  }
  }
}

व्योम RedirectingFileSystem::visitChildFileSystems(VisitCallbackTy Callback) {
  अगर (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->visitChildFileSystems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
श्रेणी llvm::vfs::RedirectingFileSystemParser {
  yaml::Stream &Stream;

  व्योम error(yaml::गाँठ *N, स्थिर Twine &संदेश_2) { Stream.printError(N, संदेश_2); }

  // false on error
  bool parseScalarString(yaml::गाँठ *N, StringRef &परिणाम,
                         SmallVectorImpl<अक्षर> &Storage) {
    स्थिर स्वतः *S = dyn_cast_if_present<yaml::Scalar_गाँठ>(N);

    अगर (!S) {
      error(N, "expected string");
      वापस false;
    }
    परिणाम = S->get_मूल्य(Storage);
    वापस true;
  }

  // false on error
  bool parseScalarBool(yaml::गाँठ *N, bool &परिणाम) {
    SmallString<5> Storage;
    StringRef मूल्य;
    अगर (!parseScalarString(N, मूल्य, Storage))
      वापस false;

    अगर (मूल्य.equals_insensitive("true") || मूल्य.equals_insensitive("on") ||
        मूल्य.equals_insensitive("yes") || मूल्य == "1") {
      परिणाम = true;
      वापस true;
    } अन्यथा अगर (मूल्य.equals_insensitive("false") ||
               मूल्य.equals_insensitive("off") ||
               मूल्य.equals_insensitive("no") || मूल्य == "0") {
      परिणाम = false;
      वापस true;
    }

    error(N, "expected boolean value");
    वापस false;
  }

  std::optional<RedirectingFileSystem::RedirectKind>
  parseRedirectKind(yaml::गाँठ *N) {
    SmallString<12> Storage;
    StringRef मूल्य;
    अगर (!parseScalarString(N, मूल्य, Storage))
      वापस std::nullopt;

    अगर (मूल्य.equals_insensitive("fallthrough")) {
      वापस RedirectingFileSystem::RedirectKind::Fallthrough;
    } अन्यथा अगर (मूल्य.equals_insensitive("fallback")) {
      वापस RedirectingFileSystem::RedirectKind::Fallback;
    } अन्यथा अगर (मूल्य.equals_insensitive("redirect-only")) {
      वापस RedirectingFileSystem::RedirectKind::RedirectOnly;
    }
    वापस std::nullopt;
  }

  std::optional<RedirectingFileSystem::मूल_Relative_Kind>
  parse_मूल_Relative_Kind(yaml::गाँठ *N) {
    SmallString<12> Storage;
    StringRef मूल्य;
    अगर (!parseScalarString(N, मूल्य, Storage))
      वापस std::nullopt;
    अगर (मूल्य.equals_insensitive("cwd")) {
      वापस RedirectingFileSystem::मूल_Relative_Kind::CWD;
    } अन्यथा अगर (मूल्य.equals_insensitive("overlay-dir")) {
      वापस RedirectingFileSystem::मूल_Relative_Kind::OverlayDir;
    }
    वापस std::nullopt;
  }

  काष्ठा कुंजी_Status {
    bool Required;
    bool Seen = false;

    कुंजी_Status(bool Required = false) : Required(Required) {}
  };

  using कुंजी_Status_Pair = std::pair<StringRef, कुंजी_Status>;

  // false on error
  bool check_Duplicate_Or_Unknown_कुंजी(yaml::गाँठ *कुंजी_गाँठ, StringRef कुंजी,
                                  DenseMap<StringRef, कुंजी_Status> &कुंजियाँ_2) {
    स्वतः It = कुंजियाँ_2.find(कुंजी);
    अगर (It == कुंजियाँ_2.end()) {
      error(कुंजी_गाँठ, "unknown key");
      वापस false;
    }
    कुंजी_Status &S = It->second;
    अगर (S.Seen) {
      error(कुंजी_गाँठ, Twine("duplicate key '") + कुंजी + "'");
      वापस false;
    }
    S.Seen = true;
    वापस true;
  }

  // false on error
  bool check_Missing_कुंजियाँ(yaml::गाँठ *Obj, DenseMap<StringRef, कुंजी_Status> &कुंजियाँ_2) {
    क्रम (स्थिर स्वतः &I : कुंजियाँ_2) {
      अगर (I.second.Required && !I.second.Seen) {
        error(Obj, Twine("missing key '") + I.first + "'");
        वापस false;
      }
    }
    वापस true;
  }

खुला:
  अटल RedirectingFileSystem::Entry *
  lookupOrCreateEntry(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::Entry *ParentEntry = nullptr) {
    अगर (!ParentEntry) { // Look for a existent root
      क्रम (स्थिर स्वतः &मूल_2 : FS->Roots) {
        अगर (Name == मूल_2->getName()) {
          ParentEntry = मूल_2.लो();
          वापस ParentEntry;
        }
      }
    } अन्यथा { // Advance to the next component
      स्वतः *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
      क्रम (std::unique_ptr<RedirectingFileSystem::Entry> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_end())) {
        स्वतः *DirContent =
            dyn_cast<RedirectingFileSystem::DirectoryEntry>(Content.लो());
        अगर (DirContent && Name == Content->getName())
          वापस DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::Entry> E =
        std::make_unique<RedirectingFileSystem::DirectoryEntry>(
            Name, Status("", get_अग्र_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    अगर (!ParentEntry) { // Add a new root to the overlay
      FS->Roots.धकेलो_back(std::move(E));
      ParentEntry = FS->Roots.back().लो();
      वापस ParentEntry;
    }

    स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(ParentEntry);
    DE->addContent(std::move(E));
    वापस DE->getLastContent();
  }

निजी:
  व्योम unique_Overlay_वृक्ष(RedirectingFileSystem *FS,
                         RedirectingFileSystem::Entry *SrcE,
                         RedirectingFileSystem::Entry *NewParentE = nullptr) {
    StringRef Name = SrcE->getName();
    चयन (SrcE->getKind()) {
    हाल RedirectingFileSystem::EK_Directory: {
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      अगर (!Name.empty())
        NewParentE = lookupOrCreateEntry(FS, Name, NewParentE);
      क्रम (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
           llvm::make_range(DE->contents_begin(), DE->contents_end()))
        unique_Overlay_वृक्ष(FS, SubEntry.लो(), NewParentE);
      अवरोध;
    }
    हाल RedirectingFileSystem::EK_DirectoryRemap: {
      निश्चित(NewParentE && "Parent entry must exist");
      स्वतः *DR = cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(
          std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
              Name, DR->getExternalContentsPath(), DR->getUseName()));
      अवरोध;
    }
    हाल RedirectingFileSystem::EK_File: {
      निश्चित(NewParentE && "Parent entry must exist");
      स्वतः *FE = cast<RedirectingFileSystem::FileEntry>(SrcE);
      स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(NewParentE);
      DE->addContent(std::make_unique<RedirectingFileSystem::FileEntry>(
          Name, FE->getExternalContentsPath(), FE->getUseName()));
      अवरोध;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::Entry>
  parseEntry(yaml::गाँठ *N, RedirectingFileSystem *FS, bool Is_मूल_Entry) {
    स्वतः *M = dyn_cast<yaml::Mapping_गाँठ>(N);
    अगर (!M) {
      error(N, "expected mapping node for file or directory entry");
      वापस nullptr;
    }

    कुंजी_Status_Pair Fields[] = {
        कुंजी_Status_Pair("name", true),
        कुंजी_Status_Pair("type", true),
        कुंजी_Status_Pair("contents", false),
        कुंजी_Status_Pair("external-contents", false),
        कुंजी_Status_Pair("use-external-name", false),
    };

    DenseMap<StringRef, कुंजी_Status> कुंजियाँ_2(std::begin(Fields), std::end(Fields));

    क्रमागत { CF_NotSet, CF_List, CF_External } ContentsField = CF_NotSet;
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>>
        EntryArrayContents;
    SmallString<256> ExternalContentsPath;
    SmallString<256> Name;
    yaml::गाँठ *Name_मूल्य_गाँठ = nullptr;
    स्वतः UseExternalName = RedirectingFileSystem::NK_NotSet;
    RedirectingFileSystem::EntryKind Kind;

    क्रम (स्वतः &I : *M) {
      StringRef कुंजी;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> बफर;
      अगर (!parseScalarString(I.get_कुंजी(), कुंजी, बफर))
        वापस nullptr;

      अगर (!check_Duplicate_Or_Unknown_कुंजी(I.get_कुंजी(), कुंजी, कुंजियाँ_2))
        वापस nullptr;

      StringRef मूल्य;
      अगर (कुंजी == "name") {
        अगर (!parseScalarString(I.get_मूल्य(), मूल्य, बफर))
          वापस nullptr;

        Name_मूल्य_गाँठ = I.get_मूल्य();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(मूल्य).str();
      } अन्यथा अगर (कुंजी == "type") {
        अगर (!parseScalarString(I.get_मूल्य(), मूल्य, बफर))
          वापस nullptr;
        अगर (मूल्य == "file")
          Kind = RedirectingFileSystem::EK_File;
        अन्यथा अगर (मूल्य == "directory")
          Kind = RedirectingFileSystem::EK_Directory;
        अन्यथा अगर (मूल्य == "directory-remap")
          Kind = RedirectingFileSystem::EK_DirectoryRemap;
        अन्यथा {
          error(I.get_मूल्य(), "unknown value for 'type'");
          वापस nullptr;
        }
      } अन्यथा अगर (कुंजी == "contents") {
        अगर (ContentsField != CF_NotSet) {
          error(I.get_कुंजी(),
                "entry already has 'contents' or 'external-contents'");
          वापस nullptr;
        }
        ContentsField = CF_List;
        स्वतः *Contents = dyn_cast_if_present<yaml::Sequence_गाँठ>(I.get_मूल्य());
        अगर (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_मूल्य(), "expected array");
          वापस nullptr;
        }

        क्रम (स्वतः &I : *Contents) {
          अगर (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ false))
            EntryArrayContents.धकेलो_back(std::move(E));
          अन्यथा
            वापस nullptr;
        }
      } अन्यथा अगर (कुंजी == "external-contents") {
        अगर (ContentsField != CF_NotSet) {
          error(I.get_कुंजी(),
                "entry already has 'contents' or 'external-contents'");
          वापस nullptr;
        }
        ContentsField = CF_External;
        अगर (!parseScalarString(I.get_मूल्य(), मूल्य, बफर))
          वापस nullptr;

        SmallString<256> FullPath;
        अगर (FS->IsRelativeOverlay) {
          FullPath = FS->getOverlayFileDir();
          निश्चित(!FullPath.empty() &&
                 "External contents prefix directory must exist");
          SmallString<256> AbsFullPath = मूल्य;
          अगर (FS->makeAbsolute(FullPath, AbsFullPath)) {
            error(N, "failed to make 'external-contents' absolute");
            वापस nullptr;
          }
          FullPath = AbsFullPath;
        } अन्यथा {
          FullPath = मूल्य;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        FullPath = canonicalize(FullPath);
        ExternalContentsPath = FullPath.str();
      } अन्यथा अगर (कुंजी == "use-external-name") {
        bool Val;
        अगर (!parseScalarBool(I.get_मूल्य(), Val))
          वापस nullptr;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } अन्यथा {
        llvm_unreachable("key missing from Keys");
      }
    }

    अगर (Stream.failed())
      वापस nullptr;

    // check for missing keys
    अगर (ContentsField == CF_NotSet) {
      error(N, "missing key 'contents' or 'external-contents'");
      वापस nullptr;
    }
    अगर (!check_Missing_कुंजियाँ(N, कुंजियाँ_2))
      वापस nullptr;

    // check invalid configuration
    अगर (Kind == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_NotSet) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      वापस nullptr;
    }

    अगर (Kind == RedirectingFileSystem::EK_DirectoryRemap &&
        ContentsField == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      वापस nullptr;
    }

    sys::path::Style path_style = sys::path::Style::native;
    अगर (Is_मूल_Entry) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      अगर (sys::path::is_absolute(Name, sys::path::Style::posix)) {
        path_style = sys::path::Style::posix;
      } अन्यथा अगर (sys::path::is_absolute(Name,
                                        sys::path::Style::windows_backslash)) {
        path_style = sys::path::Style::windows_backslash;
      } अन्यथा {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_code EC;
        अगर (FS->मूल_Relative ==
            RedirectingFileSystem::मूल_Relative_Kind::OverlayDir) {
          StringRef FullPath = FS->getOverlayFileDir();
          निश्चित(!FullPath.empty() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(FullPath, Name);
          Name = canonicalize(Name);
        } अन्यथा {
          EC = FS->makeAbsolute(Name);
        }
        अगर (EC) {
          निश्चित(Name_मूल्य_गाँठ && "Name presence should be checked earlier");
          error(
              Name_मूल्य_गाँठ,
              "entry with relative path at the root level is not discoverable");
          वापस nullptr;
        }
        path_style = sys::path::is_absolute(Name, sys::path::Style::posix)
                         ? sys::path::Style::posix
                         : sys::path::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      अगर (path_style == sys::path::Style::windows_backslash &&
          getExistingStyle(Name) != sys::path::Style::windows_backslash)
        path_style = sys::path::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    माप_प्रकार मूल_Path_Len = sys::path::मूल_path(Trimmed, path_style).आकार_2();
    जबतक (Trimmed.आकार_2() > मूल_Path_Len &&
           sys::path::is_separator(Trimmed.back(), path_style))
      Trimmed = Trimmed.slice(0, Trimmed.आकार_2() - 1);

    // Get the last component
    StringRef LastComponent = sys::path::filename(Trimmed, path_style);

    std::unique_ptr<RedirectingFileSystem::Entry> परिणाम;
    चयन (Kind) {
    हाल RedirectingFileSystem::EK_File:
      परिणाम = std::make_unique<RedirectingFileSystem::FileEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      अवरोध;
    हाल RedirectingFileSystem::EK_DirectoryRemap:
      परिणाम = std::make_unique<RedirectingFileSystem::DirectoryRemapEntry>(
          LastComponent, std::move(ExternalContentsPath), UseExternalName);
      अवरोध;
    हाल RedirectingFileSystem::EK_Directory:
      परिणाम = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          LastComponent, std::move(EntryArrayContents),
          Status("", get_अग्र_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      अवरोध;
    }

    StringRef Parent = sys::path::parent_path(Trimmed, path_style);
    अगर (Parent.empty())
      वापस परिणाम;

    // if 'name' contains multiple components, create implicit directory entries
    क्रम (sys::path::reverse_iterator I = sys::path::rbegin(Parent, path_style),
                                     E = sys::path::rend(Parent);
         I != E; ++I) {
      std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> Entries;
      Entries.धकेलो_back(std::move(परिणाम));
      परिणाम = std::make_unique<RedirectingFileSystem::DirectoryEntry>(
          *I, std::move(Entries),
          Status("", get_अग्र_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    वापस परिणाम;
  }

खुला:
  RedirectingFileSystemParser(yaml::Stream &S) : Stream(S) {}

  // false on error
  bool parse(yaml::गाँठ *मूल_2, RedirectingFileSystem *FS) {
    स्वतः *शीर्ष_3 = dyn_cast<yaml::Mapping_गाँठ>(मूल_2);
    अगर (!शीर्ष_3) {
      error(मूल_2, "expected mapping node");
      वापस false;
    }

    कुंजी_Status_Pair Fields[] = {
        कुंजी_Status_Pair("version", true),
        कुंजी_Status_Pair("case-sensitive", false),
        कुंजी_Status_Pair("use-external-names", false),
        कुंजी_Status_Pair("root-relative", false),
        कुंजी_Status_Pair("overlay-relative", false),
        कुंजी_Status_Pair("fallthrough", false),
        कुंजी_Status_Pair("redirecting-with", false),
        कुंजी_Status_Pair("roots", true),
    };

    DenseMap<StringRef, कुंजी_Status> कुंजियाँ_2(std::begin(Fields), std::end(Fields));
    std::vector<std::unique_ptr<RedirectingFileSystem::Entry>> मूल_Entries;

    // Parse configuration and 'roots'
    क्रम (स्वतः &I : *शीर्ष_3) {
      SmallString<10> कुंजी_बफर;
      StringRef कुंजी;
      अगर (!parseScalarString(I.get_कुंजी(), कुंजी, कुंजी_बफर))
        वापस false;

      अगर (!check_Duplicate_Or_Unknown_कुंजी(I.get_कुंजी(), कुंजी, कुंजियाँ_2))
        वापस false;

      अगर (कुंजी == "roots") {
        स्वतः *Roots = dyn_cast_if_present<yaml::Sequence_गाँठ>(I.get_मूल्य());
        अगर (!Roots) {
          error(I.get_मूल्य(), "expected array");
          वापस false;
        }

        क्रम (स्वतः &I : *Roots) {
          अगर (std::unique_ptr<RedirectingFileSystem::Entry> E =
                  parseEntry(&I, FS, /*IsRootEntry*/ true))
            मूल_Entries.धकेलो_back(std::move(E));
          अन्यथा
            वापस false;
        }
      } अन्यथा अगर (कुंजी == "version") {
        StringRef VersionString;
        SmallString<4> Storage;
        अगर (!parseScalarString(I.get_मूल्य(), VersionString, Storage))
          वापस false;
        पूर्णांक Version;
        अगर (VersionString.getAsInteger<पूर्णांक>(10, Version)) {
          error(I.get_मूल्य(), "expected integer");
          वापस false;
        }
        अगर (Version < 0) {
          error(I.get_मूल्य(), "invalid version number");
          वापस false;
        }
        अगर (Version != 0) {
          error(I.get_मूल्य(), "version mismatch, expected 0");
          वापस false;
        }
      } अन्यथा अगर (कुंजी == "case-sensitive") {
        अगर (!parseScalarBool(I.get_मूल्य(), FS->CaseSensitive))
          वापस false;
      } अन्यथा अगर (कुंजी == "overlay-relative") {
        अगर (!parseScalarBool(I.get_मूल्य(), FS->IsRelativeOverlay))
          वापस false;
      } अन्यथा अगर (कुंजी == "use-external-names") {
        अगर (!parseScalarBool(I.get_मूल्य(), FS->UseExternalNames))
          वापस false;
      } अन्यथा अगर (कुंजी == "fallthrough") {
        अगर (कुंजियाँ_2["redirecting-with"].Seen) {
          error(I.get_मूल्य(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          वापस false;
        }

        bool ShouldFallthrough = false;
        अगर (!parseScalarBool(I.get_मूल्य(), ShouldFallthrough))
          वापस false;

        अगर (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::RedirectKind::Fallthrough;
        } अन्यथा {
          FS->Redirection = RedirectingFileSystem::RedirectKind::RedirectOnly;
        }
      } अन्यथा अगर (कुंजी == "redirecting-with") {
        अगर (कुंजियाँ_2["fallthrough"].Seen) {
          error(I.get_मूल्य(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          वापस false;
        }

        अगर (स्वतः Kind = parseRedirectKind(I.get_मूल्य())) {
          FS->Redirection = *Kind;
        } अन्यथा {
          error(I.get_मूल्य(), "expected valid redirect kind");
          वापस false;
        }
      } अन्यथा अगर (कुंजी == "root-relative") {
        अगर (स्वतः Kind = parse_मूल_Relative_Kind(I.get_मूल्य())) {
          FS->मूल_Relative = *Kind;
        } अन्यथा {
          error(I.get_मूल्य(), "expected valid root-relative kind");
          वापस false;
        }
      } अन्यथा {
        llvm_unreachable("key missing from Keys");
      }
    }

    अगर (Stream.failed())
      वापस false;

    अगर (!check_Missing_कुंजियाँ(शीर्ष_3, कुंजियाँ_2))
      वापस false;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    क्रम (स्वतः &E : मूल_Entries)
      unique_Overlay_वृक्ष(FS, E.लो());

    वापस true;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<Memory_बफर> बफर,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFilePath, व्योम *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::Stream Stream(बफर->get_Mem_बफर_Ref(), SM);

  SM.setDiagHandler(DiagHandler, DiagContext);
  yaml::document_iterator DI = Stream.begin();
  yaml::गाँठ *मूल_2 = DI->get_मूल();
  अगर (DI == Stream.end() || !मूल_2) {
    SM.PrintMessage(SMLoc(), SourceMgr::DK_Error, "expected root node");
    वापस nullptr;
  }

  RedirectingFileSystemParser P(Stream);

  std::unique_ptr<RedirectingFileSystem> FS(
      नया RedirectingFileSystem(ExternalFS));

  अगर (!YAMLFilePath.empty()) {
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
    निश्चित(!EC && "Overlay dir final path must be absolute");
    (व्योम)EC;
    FS->setOverlayFileDir(OverlayAbsDir);
  }

  अगर (!P.parse(मूल_2, FS.लो()))
    वापस nullptr;

  वापस FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    bool UseExternalNames, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      नया RedirectingFileSystem(ExternalFS));
  FS->UseExternalNames = UseExternalNames;

  StringMap<RedirectingFileSystem::Entry *> Entries;

  क्रम (स्वतः &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> To = StringRef(Mapping.second);
    {
      स्वतः EC = ExternalFS->makeAbsolute(From);
      (व्योम)EC;
      निश्चित(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::Entry *&ToEntry = Entries[From];
    अगर (ToEntry)
      जारी;

    // Add parent directories.
    RedirectingFileSystem::Entry *Parent = nullptr;
    StringRef FromDirectory = llvm::sys::path::parent_path(From);
    क्रम (स्वतः I = llvm::sys::path::begin(FromDirectory),
              E = llvm::sys::path::end(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookupOrCreateEntry(FS.लो(), *I,
                                                                Parent);
    }
    निश्चित(Parent && "File without a directory?");
    {
      स्वतः EC = ExternalFS->makeAbsolute(To);
      (व्योम)EC;
      निश्चित(!EC && "Could not make absolute path");
    }

    // Add the file.
    स्वतः NewFile = std::make_unique<RedirectingFileSystem::FileEntry>(
        llvm::sys::path::filename(From), To,
        UseExternalNames ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    ToEntry = NewFile.लो();
    cast<RedirectingFileSystem::DirectoryEntry>(Parent)->addContent(
        std::move(NewFile));
  }

  वापस FS;
}

RedirectingFileSystem::Lookup_परिणाम::Lookup_परिणाम(
    Entry *E, sys::path::const_iterator Start, sys::path::const_iterator End)
    : E(E) {
  निश्चित(E != nullptr);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  अगर (स्वतः *DRE = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(E)) {
    SmallString<256> Redirect(DRE->getExternalContentsPath());
    sys::path::append(Redirect, Start, End,
                      getExistingStyle(DRE->getExternalContentsPath()));
    ExternalRedirect = std::string(Redirect);
  }
}

व्योम RedirectingFileSystem::Lookup_परिणाम::getPath(
    llvm::SmallVectorImpl<अक्षर> &परिणाम) स्थिर {
  परिणाम.सफा();
  क्रम (Entry *Parent : Parents)
    llvm::sys::path::append(परिणाम, Parent->getName());
  llvm::sys::path::append(परिणाम, E->getName());
}

std::error_code RedirectingFileSystem::makeCanonicalForLookup(
    SmallVectorImpl<अक्षर> &Path) स्थिर {
  अगर (std::error_code EC = makeAbsolute(Path))
    वापस EC;

  llvm::SmallString<256> CanonicalPath =
      canonicalize(StringRef(Path.तथ्य(), Path.आकार_2()));
  अगर (CanonicalPath.empty())
    वापस make_error_code(llvm::errc::invalid_argument);

  Path.assign(CanonicalPath.begin(), CanonicalPath.end());
  वापस {};
}

ErrorOr<RedirectingFileSystem::Lookup_परिणाम>
RedirectingFileSystem::lookupPath(StringRef Path) स्थिर {
  llvm::SmallString<128> CanonicalPath(Path);
  अगर (std::error_code EC = makeCanonicalForLookup(CanonicalPath))
    वापस EC;

  // RedirectOnly means the VFS is always used.
  अगर (UsageTrackingActive && Redirection == RedirectKind::RedirectOnly)
    HasBeenUsed = true;

  sys::path::const_iterator Start = sys::path::begin(CanonicalPath);
  sys::path::const_iterator End = sys::path::end(CanonicalPath);
  llvm::SmallVector<Entry *, 32> Entries;
  क्रम (स्थिर स्वतः &मूल_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम =
        lookupPathImpl(Start, End, मूल_2.लो(), Entries);
    अगर (UsageTrackingActive && परिणाम && isa<RemapEntry>(परिणाम->E))
      HasBeenUsed = true;
    अगर (परिणाम) {
      परिणाम->Parents = std::move(Entries);
      वापस परिणाम;
    }

    अगर (परिणाम.getError() != llvm::errc::no_such_file_or_directory)
      वापस परिणाम;
  }
  वापस make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_परिणाम>
RedirectingFileSystem::lookupPathImpl(
    sys::path::const_iterator Start, sys::path::const_iterator End,
    RedirectingFileSystem::Entry *From,
    llvm::SmallVectorImpl<Entry *> &Entries) स्थिर {
  निश्चित(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  अगर (!FromName.empty()) {
    अगर (!pathComponentMatches(*Start, FromName))
      वापस make_error_code(llvm::errc::no_such_file_or_directory);

    ++Start;

    अगर (Start == End) {
      // Match!
      वापस Lookup_परिणाम(From, Start, End);
    }
  }

  अगर (isa<RedirectingFileSystem::FileEntry>(From))
    वापस make_error_code(llvm::errc::not_a_directory);

  अगर (isa<RedirectingFileSystem::DirectoryRemapEntry>(From))
    वापस Lookup_परिणाम(From, Start, End);

  स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(From);
  क्रम (स्थिर std::unique_ptr<RedirectingFileSystem::Entry> &DirEntry :
       llvm::make_range(DE->contents_begin(), DE->contents_end())) {
    Entries.धकेलो_back(From);
    ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम =
        lookupPathImpl(Start, End, DirEntry.लो(), Entries);
    अगर (परिणाम || परिणाम.getError() != llvm::errc::no_such_file_or_directory)
      वापस परिणाम;
    Entries.निकालो_back();
  }

  वापस make_error_code(llvm::errc::no_such_file_or_directory);
}

अटल Status getRedirectedFileStatus(स्थिर Twine &OriginalPath,
                                      bool UseExternalNames,
                                      Status ExternalStatus) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  अगर (ExternalStatus.ExposesExternalVFSPath)
    वापस ExternalStatus;

  Status S = ExternalStatus;
  अगर (!UseExternalNames)
    S = Status::copyWithNewName(S, OriginalPath);
  अन्यथा
    S.ExposesExternalVFSPath = true;
  वापस S;
}

ErrorOr<Status> RedirectingFileSystem::status(
    स्थिर Twine &LookupPath, स्थिर Twine &OriginalPath,
    स्थिर RedirectingFileSystem::Lookup_परिणाम &परिणाम) {
  अगर (std::optional<StringRef> ExtRedirect = परिणाम.getExternalRedirect()) {
    SmallString<256> RemappedPath((*ExtRedirect).str());
    अगर (std::error_code EC = makeAbsolute(RemappedPath))
      वापस EC;

    ErrorOr<Status> S = ExternalFS->status(RemappedPath);
    अगर (!S)
      वापस S;
    S = Status::copyWithNewName(*S, *ExtRedirect);
    स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(परिणाम.E);
    वापस getRedirectedFileStatus(OriginalPath,
                                   RE->useExternalName(UseExternalNames), *S);
  }

  स्वतः *DE = cast<RedirectingFileSystem::DirectoryEntry>(परिणाम.E);
  वापस Status::copyWithNewName(DE->getStatus(), LookupPath);
}

ErrorOr<Status>
RedirectingFileSystem::getExternalStatus(स्थिर Twine &LookupPath,
                                         स्थिर Twine &OriginalPath) स्थिर {
  स्वतः परिणाम = ExternalFS->status(LookupPath);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  अगर (!परिणाम || परिणाम->ExposesExternalVFSPath)
    वापस परिणाम;
  वापस Status::copyWithNewName(परिणाम.लो(), OriginalPath);
}

ErrorOr<Status> RedirectingFileSystem::status(स्थिर Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  अगर (std::error_code EC = makeAbsolute(Path))
    वापस EC;

  अगर (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<Status> S = getExternalStatus(Path, OriginalPath);
    अगर (S)
      वापस S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookupPath(Path);
  अगर (!परिणाम) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    अगर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(परिणाम.getError()))
      वापस getExternalStatus(Path, OriginalPath);
    वापस परिणाम.getError();
  }

  ErrorOr<Status> S = status(Path, OriginalPath, *परिणाम);
  अगर (!S && Redirection == RedirectKind::Fallthrough &&
      isFileNotFound(S.getError(), परिणाम->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    वापस getExternalStatus(Path, OriginalPath);
  }

  वापस S;
}

bool RedirectingFileSystem::exists(स्थिर Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  अगर (makeAbsolute(Path))
    वापस false;

  अगर (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    अगर (ExternalFS->exists(Path))
      वापस true;
  }

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookupPath(Path);
  अगर (!परिणाम) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    अगर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(परिणाम.getError()))
      वापस ExternalFS->exists(Path);
    वापस false;
  }

  std::optional<StringRef> ExtRedirect = परिणाम->getExternalRedirect();
  अगर (!ExtRedirect) {
    निश्चित(isa<RedirectingFileSystem::DirectoryEntry>(परिणाम->E));
    वापस true;
  }

  SmallString<256> RemappedPath((*ExtRedirect).str());
  अगर (makeAbsolute(RemappedPath))
    वापस false;

  अगर (ExternalFS->exists(RemappedPath))
    वापस true;

  अगर (Redirection == RedirectKind::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    वापस ExternalFS->exists(Path);
  }

  वापस false;
}

namespace {

/// Provide a file wrapper with an overriden status.
श्रेणी FileWithFixedStatus : खुला File {
  std::unique_ptr<File> InnerFile;
  Status S;

खुला:
  FileWithFixedStatus(std::unique_ptr<File> InnerFile, Status S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<Status> status() override { वापस S; }
  ErrorOr<std::unique_ptr<llvm::Memory_बफर>>

  get_बफर(स्थिर Twine &Name, int64_t File_आकार, bool RequiresNullTerminator,
            bool IsVolatile) override {
    वापस InnerFile->get_बफर(Name, File_आकार, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_code बंद() override { वापस InnerFile->बंद(); }

  व्योम setPath(स्थिर Twine &Path) override { S = S.copyWithNewName(S, Path); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::getWithPath(ErrorOr<std::unique_ptr<File>> परिणाम, स्थिर Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  अगर (!परिणाम || (*परिणाम)->status()->ExposesExternalVFSPath)
    वापस परिणाम;

  ErrorOr<std::unique_ptr<File>> F = std::move(*परिणाम);
  स्वतः Name = F->लो()->getName();
  अगर (Name && Name.लो() != P.str())
    F->लो()->setPath(P);
  वापस F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(स्थिर Twine &OriginalPath) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  अगर (std::error_code EC = makeAbsolute(Path))
    वापस EC;

  अगर (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    स्वतः F = File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    अगर (F)
      वापस F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookupPath(Path);
  अगर (!परिणाम) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    अगर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(परिणाम.getError()))
      वापस File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    वापस परिणाम.getError();
  }

  अगर (!परिणाम->getExternalRedirect()) // FIXME: errc::not_a_file?
    वापस make_error_code(llvm::errc::invalid_argument);

  StringRef ExtRedirect = *परिणाम->getExternalRedirect();
  SmallString<256> RemappedPath(ExtRedirect.str());
  अगर (std::error_code EC = makeAbsolute(RemappedPath))
    वापस EC;

  स्वतः *RE = cast<RedirectingFileSystem::RemapEntry>(परिणाम->E);

  स्वतः ExternalFile =
      File::getWithPath(ExternalFS->openFileForRead(RemappedPath), ExtRedirect);
  अगर (!ExternalFile) {
    अगर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(ExternalFile.getError(), परिणाम->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      वापस File::getWithPath(ExternalFS->openFileForRead(Path), OriginalPath);
    }
    वापस ExternalFile;
  }

  स्वतः ExternalStatus = (*ExternalFile)->status();
  अगर (!ExternalStatus)
    वापस ExternalStatus.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  Status S = getRedirectedFileStatus(
      OriginalPath, RE->useExternalName(UseExternalNames), *ExternalStatus);
  वापस std::unique_ptr<File>(
      std::make_unique<FileWithFixedStatus>(std::move(*ExternalFile), S));
}

std::error_code
RedirectingFileSystem::getRealPath(स्थिर Twine &OriginalPath,
                                   SmallVectorImpl<अक्षर> &Output) {
  SmallString<256> Path;
  OriginalPath.toVector(Path);

  अगर (std::error_code EC = makeAbsolute(Path))
    वापस EC;

  अगर (Redirection == RedirectKind::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_code EC = ExternalFS->getRealPath(Path, Output);
    अगर (!EC)
      वापस EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookupPath(Path);
  अगर (!परिणाम) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    अगर (Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(परिणाम.getError()))
      वापस ExternalFS->getRealPath(Path, Output);
    वापस परिणाम.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  अगर (स्वतः ExtRedirect = परिणाम->getExternalRedirect()) {
    स्वतः P = ExternalFS->getRealPath(*ExtRedirect, Output);
    अगर (P && Redirection == RedirectKind::Fallthrough &&
        isFileNotFound(P, परिणाम->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      वापस ExternalFS->getRealPath(Path, Output);
    }
    वापस P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  अगर (Redirection == RedirectKind::Fallthrough) {
    परिणाम->getPath(Output);
    वापस {};
  }
  वापस llvm::errc::invalid_argument;
}

व्योम RedirectingFileSystem::getDirectoryContentRealSources(
    स्थिर Twine &Dir, SmallVectorImpl<std::string> &Out) {
  SmallString<256> Path;
  Dir.toVector(Path);

  अगर (makeAbsolute(Path))
    वापस;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  स्थिर bool ConsultsExternalFS = Redirection != RedirectKind::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookupPath(Path);
  अगर (!परिणाम) {
    // dir_begin() delegates entirely to ExternalFS.
    अगर (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(Path, Out);
    वापस;
  }

  चयन (परिणाम->E->getKind()) {
  हाल EK_File:
    वापस;
  हाल EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    अवरोध;
  हाल EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> RemappedPath(*परिणाम->getExternalRedirect());
    अगर (!makeAbsolute(RemappedPath))
      ExternalFS->getDirectoryContentRealSources(RemappedPath, Out);
    अवरोध;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  अगर (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(Path, Out);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<Memory_बफर> बफर,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFilePath, व्योम *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  वापस RedirectingFileSystem::create(std::move(बफर), DiagHandler,
                                       YAMLFilePath, DiagContext,
                                       std::move(ExternalFS));
}

अटल व्योम getVFSEntries(RedirectingFileSystem::Entry *SrcE,
                          SmallVectorImpl<StringRef> &Path,
                          SmallVectorImpl<YAMLVFSEntry> &Entries) {
  स्वतः Kind = SrcE->getKind();
  अगर (Kind == RedirectingFileSystem::EK_Directory) {
    स्वतः *DE = dyn_cast<RedirectingFileSystem::DirectoryEntry>(SrcE);
    निश्चित(DE && "Must be a directory");
    क्रम (std::unique_ptr<RedirectingFileSystem::Entry> &SubEntry :
         llvm::make_range(DE->contents_begin(), DE->contents_end())) {
      Path.धकेलो_back(SubEntry->getName());
      getVFSEntries(SubEntry.लो(), Path, Entries);
      Path.निकालो_back();
    }
    वापस;
  }

  अगर (Kind == RedirectingFileSystem::EK_DirectoryRemap) {
    स्वतः *DR = dyn_cast<RedirectingFileSystem::DirectoryRemapEntry>(SrcE);
    निश्चित(DR && "Must be a directory remap");
    SmallString<128> VPath;
    क्रम (स्वतः &Comp : Path)
      llvm::sys::path::append(VPath, Comp);
    Entries.धकेलो_back(
        YAMLVFSEntry(VPath.c_str(), DR->getExternalContentsPath()));
    वापस;
  }

  निश्चित(Kind == RedirectingFileSystem::EK_File && "Must be a EK_File");
  स्वतः *FE = dyn_cast<RedirectingFileSystem::FileEntry>(SrcE);
  निश्चित(FE && "Must be a file");
  SmallString<128> VPath;
  क्रम (स्वतः &Comp : Path)
    llvm::sys::path::append(VPath, Comp);
  Entries.धकेलो_back(YAMLVFSEntry(VPath.c_str(), FE->getExternalContentsPath()));
}

व्योम vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            SmallVectorImpl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> मूल_परिणाम = VFS.lookupPath("/");
  अगर (!मूल_परिणाम)
    वापस;
  SmallVector<StringRef, 8> Components;
  Components.धकेलो_back("/");
  getVFSEntries(मूल_परिणाम->E, Components, CollectedEntries);
}

UniqueID vfs::get_अग्र_Virtual_Unique_ID() {
  अटल std::atomic<अचिन्हित> UID;
  अचिन्हित ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  वापस UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

व्योम YAMLVFSWriter::addEntry(StringRef VirtualPath, StringRef RealPath,
                             bool IsDirectory) {
  निश्चित(sys::path::is_absolute(VirtualPath) && "virtual path not absolute");
  निश्चित(sys::path::is_absolute(RealPath) && "real path not absolute");
  निश्चित(!pathHasTraversal(VirtualPath) && "path traversal is not supported");
  Mappings.emplace_back(VirtualPath, RealPath, IsDirectory);
}

व्योम YAMLVFSWriter::addFileMapping(StringRef VirtualPath, StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/false);
}

व्योम YAMLVFSWriter::addDirectoryMapping(StringRef VirtualPath,
                                        StringRef RealPath) {
  addEntry(VirtualPath, RealPath, /*IsDirectory=*/true);
}

namespace {

श्रेणी JSONWriter {
  llvm::raw_ostream &OS;
  SmallVector<StringRef, 16> Dir_ढेर;

  अचिन्हित getDirIndent() { वापस 4 * Dir_ढेर.आकार_2(); }
  अचिन्हित getFileIndent() { वापस 4 * (Dir_ढेर.आकार_2() + 1); }
  bool containedIn(StringRef Parent, StringRef Path);
  StringRef containedPart(StringRef Parent, StringRef Path);
  व्योम startDirectory(StringRef Path);
  व्योम endDirectory();
  व्योम writeEntry(StringRef VPath, StringRef RPath);

खुला:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  व्योम ग_लिखो(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<bool> UseExternalNames,
             std::optional<bool> IsCaseSensitive,
             std::optional<bool> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

bool JSONWriter::containedIn(StringRef Parent, StringRef Path) {
  using namespace llvm::sys;

  // Compare each path component.
  स्वतः IParent = path::begin(Parent), EParent = path::end(Parent);
  क्रम (स्वतः IChild = path::begin(Path), EChild = path::end(Path);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    अगर (*IParent != *IChild)
      वापस false;
  }
  // Have we exhausted the parent path?
  वापस IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef Path) {
  निश्चित(!Parent.empty());
  निश्चित(containedIn(Parent, Path));
  वापस Path.substr(Parent.आकार_2() + 1);
}

व्योम JSONWriter::startDirectory(StringRef Path) {
  StringRef Name =
      Dir_ढेर.empty() ? Path : containedPart(Dir_ढेर.back(), Path);
  Dir_ढेर.धकेलो_back(Path);
  अचिन्हित Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

व्योम JSONWriter::endDirectory() {
  अचिन्हित Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_ढेर.निकालो_back();
}

व्योम JSONWriter::writeEntry(StringRef VPath, StringRef RPath) {
  अचिन्हित Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

व्योम JSONWriter::ग_लिखो(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<bool> UseExternalNames,
                       std::optional<bool> IsCaseSensitive,
                       std::optional<bool> IsOverlayRelative,
                       StringRef OverlayDir) {
  using namespace llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  अगर (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  अगर (UseExternalNames)
    OS << "  'use-external-names': '" << (*UseExternalNames ? "true" : "false")
       << "',\n";
  bool UseOverlayRelative = false;
  अगर (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  अगर (!Entries.empty()) {
    स्थिर YAMLVFSEntry &Entry = Entries.front();

    startDirectory(
      Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath)
    );

    StringRef RPath = Entry.RPath;
    अगर (UseOverlayRelative) {
      निश्चित(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.आकार_2());
    }

    bool IsCurrentDirEmpty = true;
    अगर (!Entry.IsDirectory) {
      writeEntry(path::filename(Entry.VPath), RPath);
      IsCurrentDirEmpty = false;
    }

    क्रम (स्थिर स्वतः &Entry : Entries.slice(1)) {
      StringRef Dir =
          Entry.IsDirectory ? Entry.VPath : path::parent_path(Entry.VPath);
      अगर (Dir == Dir_ढेर.back()) {
        अगर (!IsCurrentDirEmpty) {
          OS << ",\n";
        }
      } अन्यथा {
        bool Is_Dir_Popped_From_ढेर = false;
        जबतक (!Dir_ढेर.empty() && !containedIn(Dir_ढेर.back(), Dir)) {
          OS << "\n";
          endDirectory();
          Is_Dir_Popped_From_ढेर = true;
        }
        अगर (Is_Dir_Popped_From_ढेर || !IsCurrentDirEmpty) {
          OS << ",\n";
        }
        startDirectory(Dir);
        IsCurrentDirEmpty = true;
      }
      StringRef RPath = Entry.RPath;
      अगर (UseOverlayRelative) {
        निश्चित(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.आकार_2());
      }
      अगर (!Entry.IsDirectory) {
        writeEntry(path::filename(Entry.VPath), RPath);
        IsCurrentDirEmpty = false;
      }
    }

    जबतक (!Dir_ढेर.empty()) {
      OS << "\n";
      endDirectory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

व्योम YAMLVFSWriter::ग_लिखो(llvm::raw_ostream &OS) {
  llvm::sort(Mappings, [](स्थिर YAMLVFSEntry &LHS, स्थिर YAMLVFSEntry &RHS) {
    वापस LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).ग_लिखो(Mappings, UseExternalNames, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, स्थिर Twine &Path, std::error_code &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(Path, EC);
  अगर (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->ढेर_2.धकेलो_back(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_code &EC) {
  निश्चित(FS && State && !State->ढेर_2.empty() && "incrementing past end");
  निश्चित(!State->ढेर_2.back()->path().empty() && "non-canonical end iterator");
  vfs::directory_iterator End;

  अगर (State->Has_No_धकेलो_Request)
    State->Has_No_धकेलो_Request = false;
  अन्यथा {
    अगर (State->ढेर_2.back()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->ढेर_2.back()->path(), EC);
      अगर (I != End) {
        State->ढेर_2.धकेलो_back(I);
        वापस *यह;
      }
    }
  }

  जबतक (!State->ढेर_2.empty() && State->ढेर_2.back().increment(EC) == End)
    State->ढेर_2.निकालो_back();

  अगर (State->ढेर_2.empty())
    State.reset(); // end iterator

  वापस *यह;
}

स्थिर अक्षर FileSystem::ID = 0;
स्थिर अक्षर OverlayFileSystem::ID = 0;
स्थिर अक्षर ProxyFileSystem::ID = 0;
स्थिर अक्षर InMemoryFileSystem::ID = 0;
स्थिर अक्षर RedirectingFileSystem::ID = 0;

अचिन्हित ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_गिनती(
    स्थिर FileSystem *FS) {
  वापस FS->Use_गिनती();
}

व्योम ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

व्योम ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
