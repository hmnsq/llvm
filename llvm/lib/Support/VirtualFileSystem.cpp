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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::vfs;

उपयुज्य llvm::sys::fs::file_स्थितिः;
उपयुज्य llvm::sys::fs::file_t;
उपयुज्य llvm::sys::fs::file_type;
उपयुज्य llvm::sys::fs::perms;
उपयुज्य llvm::sys::fs::UniqueID;

स्थितिः::स्थितिः(ध्रुव file_स्थितिः &स्थितिः)
    : UID(स्थितिः.getUniqueID()), MTime(स्थितिः.get_चरमः_Modification_Time()),
      User(स्थितिः.getUser()), गणः(स्थितिः.get_गणः()), आकार(स्थितिः.get_आकार()),
      Type(स्थितिः.type()), Perms(स्थितिः.permissions()) {}

स्थितिः::स्थितिः(ध्रुव Twine &Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t गणः, uint64_t आकार, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), गणः(गणः),
      आकार(आकार), Type(Type), Perms(Perms) {}

स्थितिः स्थितिः::copy_With_New_आकार(ध्रुव स्थितिः &अन्तर्_2, uint64_t New_आकार) {
  प्रत्यागम स्थितिः(अन्तर्_2.getName(), अन्तर्_2.getUniqueID(), अन्तर्_2.get_चरमः_Modification_Time(),
                अन्तर्_2.getUser(), अन्तर्_2.get_गणः(), New_आकार, अन्तर्_2.getType(),
                अन्तर्_2.getPermissions());
}

स्थितिः स्थितिः::copyWithNewName(ध्रुव स्थितिः &अन्तर्_2, ध्रुव Twine &NewName) {
  प्रत्यागम स्थितिः(NewName, अन्तर्_2.getUniqueID(), अन्तर्_2.get_चरमः_Modification_Time(),
                अन्तर्_2.getUser(), अन्तर्_2.get_गणः(), अन्तर्_2.get_आकार(), अन्तर्_2.getType(),
                अन्तर्_2.getPermissions());
}

स्थितिः स्थितिः::copyWithNewName(ध्रुव file_स्थितिः &अन्तर्_2, ध्रुव Twine &NewName) {
  प्रत्यागम स्थितिः(NewName, अन्तर्_2.getUniqueID(), अन्तर्_2.get_चरमः_Modification_Time(),
                अन्तर्_2.getUser(), अन्तर्_2.get_गणः(), अन्तर्_2.get_आकार(), अन्तर्_2.type(),
                अन्तर्_2.permissions());
}

तार्किक स्थितिः::equivalent(ध्रुव स्थितिः &Other) ध्रुव {
  प्रतिज्ञा(is_स्थितिः_ज्ञातम्() && Other.is_स्थितिः_ज्ञातम्());
  प्रत्यागम getUniqueID() == Other.getUniqueID();
}

तार्किक स्थितिः::isDirectory() ध्रुव { प्रत्यागम Type == file_type::directory_file; }

तार्किक स्थितिः::isRegularFile() ध्रुव { प्रत्यागम Type == file_type::regular_file; }

तार्किक स्थितिः::isOther() ध्रुव {
  प्रत्यागम exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

तार्किक स्थितिः::isSymlink() ध्रुव { प्रत्यागम Type == file_type::symlink_file; }

तार्किक स्थितिः::is_स्थितिः_ज्ञातम्() ध्रुव { प्रत्यागम Type != file_type::स्थितिः_error; }

तार्किक स्थितिः::exists() ध्रुव {
  प्रत्यागम is_स्थितिः_ज्ञातम्() && Type != file_type::file_not_प्राप्तम्;
}

File::~File() = स्वभाव;

FileSystem::~FileSystem() = स्वभाव;

ErrorOr<std::unique_ptr<स्मृतिः_बफर>>
FileSystem::get_बफर_For_File(ध्रुव llvm::Twine &Name, int64_t File_आकार,
                             तार्किक RequiresNullTerminator, तार्किक IsVolatile,
                             तार्किक IsText) {
  स्वचालित F = IsText ? openFileForRead(Name) : open_File_For_Read_द्विमान(Name);
  यदि (!F)
    प्रत्यागम F.getError();

  प्रत्यागम (*F)->get_बफर(Name, File_आकार, RequiresNullTerminator, IsVolatile);
}

std::error_संहिता FileSystem::makeAbsolute(Small_सदिशम्_Impl<वर्ण> &मार्गः) ध्रुव {
  यदि (llvm::sys::मार्गः_2::is_absolute(मार्गः))
    प्रत्यागम {};

  स्वचालित WorkingDir = getCurrentWorkingDirectory();
  यदि (!WorkingDir)
    प्रत्यागम WorkingDir.getError();

  sys::मार्गः_2::make_absolute(WorkingDir.get(), मार्गः);
  प्रत्यागम {};
}

std::error_संहिता FileSystem::get_Real_मार्गः(ध्रुव Twine &मार्गः,
                                        Small_सदिशम्_Impl<वर्ण> &Output) {
  प्रत्यागम errc::operation_not_permitted;
}

std::error_संहिता FileSystem::isLocal(ध्रुव Twine &मार्गः, तार्किक &परिणाम) {
  प्रत्यागम errc::operation_not_permitted;
}

तार्किक FileSystem::exists(ध्रुव Twine &मार्गः) {
  स्वचालित स्थितिः = स्थितिः_2(मार्गः);
  प्रत्यागम स्थितिः && स्थितिः->exists();
}

llvm::ErrorOr<तार्किक> FileSystem::equivalent(ध्रुव Twine &A, ध्रुव Twine &B) {
  स्वचालित स्थितिः_A = स्थितिः_2(A);
  यदि (!स्थितिः_A)
    प्रत्यागम स्थितिः_A.getError();
  स्वचालित स्थितिः_B = स्थितिः_2(B);
  यदि (!स्थितिः_B)
    प्रत्यागम स्थितिः_B.getError();
  प्रत्यागम स्थितिः_A->equivalent(*स्थितिः_B);
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
शून्य FileSystem::dump() ध्रुव { print(dbgs(), PrintType::RecursiveContents); }
#यदि_अन्तः

#यदि_अपरिभाषितम् NDEBUG
स्थिर तार्किक isTraversalComponent(StringRef Component) {
  प्रत्यागम Component == ".." || Component == ".";
}

स्थिर तार्किक मार्गः_Has_Traversal(StringRef मार्गः) {
  उपयुज्य नामाकाश llvm::sys;

  प्रति (StringRef Comp : llvm::make_range(मार्गः_2::begin(मार्गः), मार्गः_2::अन्तः(मार्गः)))
    यदि (isTraversalComponent(Comp))
      प्रत्यागम सत्य;
  प्रत्यागम असत्य;
}
#यदि_अन्तः

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

नामाकाश {

/// Wrapper around a raw file descriptor.
वर्ग RealFile : सार्वजनिक File {
  मित्रम् वर्ग RealFileSystem;

  file_t FD;
  स्थितिः S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_मार्गः_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::स्थितिः_error, {}),
        RealName(New_Real_मार्गः_Name.str()) {
    प्रतिज्ञा(FD.isValid() && "Invalid or inactive file descriptor");
  }

सार्वजनिक:
  ~RealFile() override;

  ErrorOr<स्थितिः> स्थितिः_2() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<स्मृतिः_बफर>> get_बफर(ध्रुव Twine &Name,
                                                   int64_t File_आकार,
                                                   तार्किक RequiresNullTerminator,
                                                   तार्किक IsVolatile) override;
  std::error_संहिता close() override;
  शून्य समुच्चयः_मार्गः(ध्रुव Twine &मार्गः) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<स्थितिः> RealFile::स्थितिः_2() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(FD.isValid() && "cannot stat closed file");
  यदि (!S.is_स्थितिः_ज्ञातम्()) {
    file_स्थितिः Real_स्थितिः;
    यदि (std::error_संहिता EC = sys::fs::स्थितिः_2(FD, Real_स्थितिः))
      प्रत्यागम EC;
    S = स्थितिः::copyWithNewName(Real_स्थितिः, S.getName());
  }
  प्रत्यागम S;
}

ErrorOr<std::string> RealFile::getName() {
  प्रत्यागम RealName.रिक्तानि() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<स्मृतिः_बफर>>
RealFile::get_बफर(ध्रुव Twine &Name, int64_t File_आकार,
                    तार्किक RequiresNullTerminator, तार्किक IsVolatile) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा(FD.isValid() && "cannot get buffer for closed file");
  प्रत्यागम स्मृतिः_बफर::getOpenFile(FD, Name, File_आकार, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_संहिता RealFile::close() {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  std::error_संहिता EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  प्रत्यागम EC;
}

शून्य RealFile::समुच्चयः_मार्गः(ध्रुव Twine &मार्गः) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  RealName = मार्गः.str();
  यदि (स्वचालित स्थितिः = स्थितिः_2())
    S = स्थितिः.get().copyWithNewName(स्थितिः.get(), मार्गः);
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
  स्पष्ट RealFileSystem(तार्किक LinkCWDToProcess) {
    यदि (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      यदि (std::error_संहिता EC = llvm::sys::fs::current_मार्गः(PWD))
        WD = std::move(EC);
      अन्यथा यदि (llvm::sys::fs::real_मार्गः(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      अन्यथा
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<स्थितिः> स्थितिः_2(ध्रुव Twine &मार्गः) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(ध्रुव Twine &मार्गः) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_द्विमान(ध्रुव Twine &मार्गः) override;
  directory_iterator dir_begin(ध्रुव Twine &Dir, std::error_संहिता &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() ध्रुव override;
  std::error_संहिता समुच्चयः_Current_Working_Directory(ध्रुव Twine &मार्गः) override;
  std::error_संहिता isLocal(ध्रुव Twine &मार्गः, तार्किक &परिणाम) override;
  शून्य
  getDirectoryContentRealSources(ध्रुव Twine &Dir,
                                 Small_सदिशम्_Impl<std::string> &बहिर्गमनम्) override {
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(Dir.str());
  }
  std::error_संहिता get_Real_मार्गः(ध्रुव Twine &मार्गः,
                              Small_सदिशम्_Impl<वर्ण> &Output) override;

रक्षित:
  शून्य printImpl(raw_ostream &OS, PrintType Type,
                 अचिह्नित IndentLevel) ध्रुव override;

गुप्त:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_मार्गः(ध्रुव Twine &मार्गः, Small_सदिशम्_Impl<वर्ण> &Storage) ध्रुव {
    यदि (!WD || !*WD)
      प्रत्यागम मार्गः;
    मार्गः.गन्तव्यम्_सदिशम्(Storage);
    sys::मार्गः_2::make_absolute(WD->get().Resolved, Storage);
    प्रत्यागम Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(ध्रुव Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_मार्गः(Name, Storage), Flags, &RealName);
    यदि (!FDOrErr)
      प्रत्यागम error_गन्तव्यम्_Error_संहिता(FDOrErr.takeError());
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

ErrorOr<स्थितिः> RealFileSystem::स्थितिः_2(ध्रुव Twine &मार्गः) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_स्थितिः Real_स्थितिः;
  यदि (std::error_संहिता EC =
          sys::fs::स्थितिः_2(adjust_मार्गः(मार्गः, Storage), Real_स्थितिः))
    प्रत्यागम EC;
  प्रत्यागम स्थितिः::copyWithNewName(Real_स्थितिः, मार्गः);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(ध्रुव Twine &Name) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  प्रत्यागम openFileForReadWithFlags(Name, sys::fs::सम्बन्धि_Text);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_द्विमान(ध्रुव Twine &Name) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  प्रत्यागम openFileForReadWithFlags(Name, sys::fs::सम्बन्धि_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() ध्रुव {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  यदि (WD && *WD)
    प्रत्यागम std::string(WD->get().Specified);
  यदि (WD)
    प्रत्यागम WD->getError();

  SmallString<128> Dir;
  यदि (std::error_संहिता EC = llvm::sys::fs::current_मार्गः(Dir))
    प्रत्यागम EC;
  प्रत्यागम std::string(Dir);
}

std::error_संहिता RealFileSystem::समुच्चयः_Current_Working_Directory(ध्रुव Twine &मार्गः) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  यदि (!WD)
    प्रत्यागम llvm::sys::fs::समुच्चयः_current_मार्गः(मार्गः);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_मार्गः(मार्गः, Storage).गन्तव्यम्_सदिशम्(Absolute);
  तार्किक IsDir;
  यदि (स्वचालित Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    प्रत्यागम Err;
  यदि (!IsDir)
    प्रत्यागम std::make_error_संहिता(std::errc::not_a_directory);
  यदि (स्वचालित Err = llvm::sys::fs::real_मार्गः(Absolute, Resolved))
    प्रत्यागम Err;
  WD = WorkingDirectory{Absolute, Resolved};
  प्रत्यागम std::error_संहिता();
}

std::error_संहिता RealFileSystem::isLocal(ध्रुव Twine &मार्गः, तार्किक &परिणाम) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  प्रत्यागम llvm::sys::fs::is_local(adjust_मार्गः(मार्गः, Storage), परिणाम);
}

std::error_संहिता RealFileSystem::get_Real_मार्गः(ध्रुव Twine &मार्गः,
                                            Small_सदिशम्_Impl<वर्ण> &Output) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  प्रत्यागम llvm::sys::fs::real_मार्गः(adjust_मार्गः(मार्गः, Storage), Output);
}

शून्य RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               अचिह्नित IndentLevel) ध्रुव {
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

  प्रत्यागम FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  प्रत्यागम std::make_unique<RealFileSystem>(असत्य);
}

नामाकाश {

वर्ग RealFSDirIter : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

सार्वजनिक:
  RealFSDirIter(ध्रुव Twine &मार्गः, std::error_संहिता &EC) {
    स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(मार्गः, EC);
    यदि (Iter != sys::fs::directory_iterator())
      Current_प्रविष्टिः = directory_प्रविष्टिः(Iter->मार्गः_2(), Iter->type());
  }

  std::error_संहिता increment() override {
    स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

    std::error_संहिता EC;
    Iter.increment(EC);
    Current_प्रविष्टिः = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_प्रविष्टिः()
                       : directory_प्रविष्टिः(Iter->मार्गः_2(), Iter->type());
    प्रत्यागम EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(ध्रुव Twine &Dir,
                                             std::error_संहिता &EC) {
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  प्रत्यागम directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_मार्गः(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> आधारः_FS) {
  FSList.प्रक्षिप_पृष्ठभागः(std::move(आधारः_FS));
}

शून्य OverlayFileSystem::प्रक्षिप_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.प्रक्षिप_पृष्ठभागः(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->समुच्चयः_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<स्थितिः> OverlayFileSystem::स्थितिः_2(ध्रुव Twine &मार्गः) {
  // FIXME: handle symlinks that cross file systems
  प्रति (iterator I = overlays_begin(), E = overlays_अन्तः(); I != E; ++I) {
    ErrorOr<स्थितिः> स्थितिः = (*I)->स्थितिः_2(मार्गः);
    यदि (स्थितिः || स्थितिः.getError() != llvm::errc::no_such_file_or_directory)
      प्रत्यागम स्थितिः;
  }
  प्रत्यागम make_error_संहिता(llvm::errc::no_such_file_or_directory);
}

तार्किक OverlayFileSystem::exists(ध्रुव Twine &मार्गः) {
  // FIXME: handle symlinks that cross file systems
  प्रति (iterator I = overlays_begin(), E = overlays_अन्तः(); I != E; ++I) {
    यदि ((*I)->exists(मार्गः))
      प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(ध्रुव llvm::Twine &मार्गः) {
  // FIXME: handle symlinks that cross file systems
  प्रति (iterator I = overlays_begin(), E = overlays_अन्तः(); I != E; ++I) {
    स्वचालित परिणाम = (*I)->openFileForRead(मार्गः);
    यदि (परिणाम || परिणाम.getError() != llvm::errc::no_such_file_or_directory)
      प्रत्यागम परिणाम;
  }
  प्रत्यागम make_error_संहिता(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() ध्रुव {
  // All file systems are synchronized, just take the first working directory.
  प्रत्यागम FSList.front()->getCurrentWorkingDirectory();
}

std::error_संहिता
OverlayFileSystem::समुच्चयः_Current_Working_Directory(ध्रुव Twine &मार्गः) {
  प्रति (स्वचालित &FS : FSList)
    यदि (std::error_संहिता EC = FS->समुच्चयः_Current_Working_Directory(मार्गः))
      प्रत्यागम EC;
  प्रत्यागम {};
}

std::error_संहिता OverlayFileSystem::isLocal(ध्रुव Twine &मार्गः, तार्किक &परिणाम) {
  प्रति (स्वचालित &FS : FSList)
    यदि (FS->exists(मार्गः))
      प्रत्यागम FS->isLocal(मार्गः, परिणाम);
  प्रत्यागम errc::no_such_file_or_directory;
}

std::error_संहिता OverlayFileSystem::get_Real_मार्गः(ध्रुव Twine &मार्गः,
                                               Small_सदिशम्_Impl<वर्ण> &Output) {
  प्रति (ध्रुव स्वचालित &FS : FSList)
    यदि (FS->exists(मार्गः))
      प्रत्यागम FS->get_Real_मार्गः(मार्गः, Output);
  प्रत्यागम errc::no_such_file_or_directory;
}

शून्य OverlayFileSystem::getDirectoryContentRealSources(
    ध्रुव Twine &Dir, Small_सदिशम्_Impl<std::string> &बहिर्गमनम्) {
  // All layers contribute.
  प्रति (iterator I = overlays_begin(), E = overlays_अन्तः(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, बहिर्गमनम्);
}

शून्य OverlayFileSystem::दर्शय_Child_File_Systems(दर्शय_Callback_Ty Callback) {
  प्रति (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->दर्शय_Child_File_Systems(Callback);
  }
}

शून्य OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  अचिह्नित IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  यदि (Type == PrintType::Summary)
    प्रत्यागम;

  यदि (Type == PrintType::Contents)
    Type = PrintType::Summary;
  प्रति (ध्रुव स्वचालित &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = स्वभाव;

नामाकाश {

/// Combines and deduplicates directory entries across multiple file systems.
वर्ग CombiningDirIterImpl : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  उपयुज्य FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_सदिशम्<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_समुच्चयः<> आलोकितम्_नामानि;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_संहिता incrementIter(तार्किक IsFirstTime) {
    यावत् (!IterList.रिक्तानि()) {
      CurrentDirIter = IterList.पृष्ठभागः();
      IterList.निष्कासय_पृष्ठभागः();
      यदि (CurrentDirIter != directory_iterator())
        भङ्ग; //प्राप्तम्
    }

    यदि (IsFirstTime && CurrentDirIter == directory_iterator())
      प्रत्यागम errc::no_such_file_or_directory;
    प्रत्यागम {};
  }

  std::error_संहिता incrementDirIter(तार्किक IsFirstTime) {
    प्रतिज्ञा((IsFirstTime || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_संहिता EC;
    यदि (!IsFirstTime)
      CurrentDirIter.increment(EC);
    यदि (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(IsFirstTime);
    प्रत्यागम EC;
  }

  std::error_संहिता incrementImpl(तार्किक IsFirstTime) {
    यावत् (सत्य) {
      std::error_संहिता EC = incrementDirIter(IsFirstTime);
      यदि (EC || CurrentDirIter == directory_iterator()) {
        Current_प्रविष्टिः = directory_प्रविष्टिः();
        प्रत्यागम EC;
      }
      Current_प्रविष्टिः = *CurrentDirIter;
      StringRef Name = llvm::sys::मार्गः_2::filename(Current_प्रविष्टिः.मार्गः_2());
      यदि (आलोकितम्_नामानि.निवेशय(Name).second)
        प्रत्यागम EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

सार्वजनिक:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_संहिता &EC) {
    प्रति (ध्रुव स्वचालित &FS : FileSystems) {
      std::error_संहिता FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      यदि (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        प्रत्यागम;
      }
      यदि (!FEC)
        IterList.प्रक्षिप_पृष्ठभागः(Iter);
    }
    EC = incrementImpl(सत्य);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_संहिता &EC)
      : IterList(DirIters) {
    EC = incrementImpl(सत्य);
  }

  std::error_संहिता increment() override { प्रत्यागम incrementImpl(असत्य); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(ध्रुव Twine &Dir,
                                                std::error_संहिता &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  यदि (EC)
    प्रत्यागम {};
  प्रत्यागम Combined;
}

शून्य ProxyFileSystem::anchor() {}

नामाकाश llvm {
नामाकाश vfs {

नामाकाश detail {

गणना अन्तर्_स्मृतिः_ग्रन्थिः_विधा {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
वर्ग अन्तर्_स्मृतिः_ग्रन्थिः {
  अन्तर्_स्मृतिः_ग्रन्थिः_विधा विधा_2;
  std::string FileName;

सार्वजनिक:
  अन्तर्_स्मृतिः_ग्रन्थिः(llvm::StringRef FileName, अन्तर्_स्मृतिः_ग्रन्थिः_विधा विधा_2)
      : विधा_2(विधा_2), FileName(std::string(llvm::sys::मार्गः_2::filename(FileName))) {
  }
  आभास ~अन्तर्_स्मृतिः_ग्रन्थिः() = स्वभाव;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  आभास स्थितिः get_स्थितिः(ध्रुव Twine &RequestedName) ध्रुव = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() ध्रुव { प्रत्यागम FileName; }
  अन्तर्_स्मृतिः_ग्रन्थिः_विधा get_विधा() ध्रुव { प्रत्यागम विधा_2; }
  आभास std::string गन्तव्यम्_String(अचिह्नित Indent) ध्रुव = 0;
};

वर्ग अन्तर्_स्मृतिः_File : सार्वजनिक अन्तर्_स्मृतिः_ग्रन्थिः {
  स्थितिः Stat;
  std::unique_ptr<llvm::स्मृतिः_बफर> बफर;

सार्वजनिक:
  अन्तर्_स्मृतिः_File(स्थितिः Stat, std::unique_ptr<llvm::स्मृतिः_बफर> बफर)
      : अन्तर्_स्मृतिः_ग्रन्थिः(Stat.getName(), IME_File), Stat(std::move(Stat)),
        बफर(std::move(बफर)) {}

  स्थितिः get_स्थितिः(ध्रुव Twine &RequestedName) ध्रुव override {
    प्रत्यागम स्थितिः::copyWithNewName(Stat, RequestedName);
  }
  llvm::स्मृतिः_बफर *get_बफर() ध्रुव { प्रत्यागम बफर.get(); }

  std::string गन्तव्यम्_String(अचिह्नित Indent) ध्रुव override {
    प्रत्यागम (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  स्थिर तार्किक classof(ध्रुव अन्तर्_स्मृतिः_ग्रन्थिः *N) {
    प्रत्यागम N->get_विधा() == IME_File;
  }
};

नामाकाश {

वर्ग अन्तर्_स्मृतिः_Hard_Link : सार्वजनिक अन्तर्_स्मृतिः_ग्रन्थिः {
  ध्रुव अन्तर्_स्मृतिः_File &ResolvedFile;

सार्वजनिक:
  अन्तर्_स्मृतिः_Hard_Link(StringRef मार्गः, ध्रुव अन्तर्_स्मृतिः_File &ResolvedFile)
      : अन्तर्_स्मृतिः_ग्रन्थिः(मार्गः, IME_HardLink), ResolvedFile(ResolvedFile) {}
  ध्रुव अन्तर्_स्मृतिः_File &getResolvedFile() ध्रुव { प्रत्यागम ResolvedFile; }

  स्थितिः get_स्थितिः(ध्रुव Twine &RequestedName) ध्रुव override {
    प्रत्यागम ResolvedFile.get_स्थितिः(RequestedName);
  }

  std::string गन्तव्यम्_String(अचिह्नित Indent) ध्रुव override {
    प्रत्यागम std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.गन्तव्यम्_String(0);
  }

  स्थिर तार्किक classof(ध्रुव अन्तर्_स्मृतिः_ग्रन्थिः *N) {
    प्रत्यागम N->get_विधा() == IME_HardLink;
  }
};

वर्ग अन्तर्_स्मृतिः_Symbolic_Link : सार्वजनिक अन्तर्_स्मृतिः_ग्रन्थिः {
  std::string Target_मार्गः;
  स्थितिः Stat;

सार्वजनिक:
  अन्तर्_स्मृतिः_Symbolic_Link(StringRef मार्गः, StringRef Target_मार्गः, स्थितिः Stat)
      : अन्तर्_स्मृतिः_ग्रन्थिः(मार्गः, IME_SymbolicLink), Target_मार्गः(std::move(Target_मार्गः)),
        Stat(Stat) {}

  std::string गन्तव्यम्_String(अचिह्नित Indent) ध्रुव override {
    प्रत्यागम std::string(Indent, ' ') + "SymbolicLink to -> " + Target_मार्गः;
  }

  स्थितिः get_स्थितिः(ध्रुव Twine &RequestedName) ध्रुव override {
    प्रत्यागम स्थितिः::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_Target_मार्गः() ध्रुव { प्रत्यागम Target_मार्गः; }

  स्थिर तार्किक classof(ध्रुव अन्तर्_स्मृतिः_ग्रन्थिः *N) {
    प्रत्यागम N->get_विधा() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
वर्ग अन्तर्_स्मृतिः_File_Adaptor : सार्वजनिक File {
  ध्रुव अन्तर्_स्मृतिः_File &ग्रन्थिः;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

सार्वजनिक:
  स्पष्ट अन्तर्_स्मृतिः_File_Adaptor(ध्रुव अन्तर्_स्मृतिः_File &ग्रन्थिः,
                               std::string RequestedName)
      : ग्रन्थिः(ग्रन्थिः), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<स्थितिः> स्थितिः_2() override {
    प्रत्यागम ग्रन्थिः.get_स्थितिः(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::स्मृतिः_बफर>>
  get_बफर(ध्रुव Twine &Name, int64_t File_आकार, तार्किक RequiresNullTerminator,
            तार्किक IsVolatile) override {
    llvm::स्मृतिः_बफर *Buf = ग्रन्थिः.get_बफर();
    प्रत्यागम llvm::स्मृतिः_बफर::get_Mem_बफर(
        Buf->get_बफर(), Buf->get_बफर_Identifier(), RequiresNullTerminator);
  }

  std::error_संहिता close() override { प्रत्यागम {}; }

  शून्य समुच्चयः_मार्गः(ध्रुव Twine &मार्गः) override { RequestedName = मार्गः.str(); }
};
} // namespace

वर्ग अन्तर्_स्मृतिः_Directory : सार्वजनिक अन्तर्_स्मृतिः_ग्रन्थिः {
  स्थितिः Stat;
  std::map<std::string, std::unique_ptr<अन्तर्_स्मृतिः_ग्रन्थिः>, std::less<>> Entries;

सार्वजनिक:
  अन्तर्_स्मृतिः_Directory(स्थितिः Stat)
      : अन्तर्_स्मृतिः_ग्रन्थिः(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  स्थितिः get_स्थितिः(ध्रुव Twine &RequestedName) ध्रुव override {
    प्रत्यागम स्थितिः::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() ध्रुव { प्रत्यागम Stat.getUniqueID(); }

  अन्तर्_स्मृतिः_ग्रन्थिः *getChild(StringRef Name) ध्रुव {
    स्वचालित I = Entries.find(Name);
    यदि (I != Entries.अन्तः())
      प्रत्यागम I->second.get();
    प्रत्यागम शून्यसूचकः;
  }

  अन्तर्_स्मृतिः_ग्रन्थिः *सङ्कलय_Child(StringRef Name, std::unique_ptr<अन्तर्_स्मृतिः_ग्रन्थिः> Child) {
    प्रत्यागम Entries.emplace(Name, std::move(Child)).first->second.get();
  }

  उपयुज्य const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() ध्रुव { प्रत्यागम Entries.begin(); }
  const_iterator अन्तः() ध्रुव { प्रत्यागम Entries.अन्तः(); }

  std::string गन्तव्यम्_String(अचिह्नित Indent) ध्रुव override {
    std::string परिणाम =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    प्रति (ध्रुव स्वचालित &प्रविष्टिः : Entries)
      परिणाम += प्रविष्टिः.second->गन्तव्यम्_String(Indent + 2);
    प्रत्यागम परिणाम;
  }

  स्थिर तार्किक classof(ध्रुव अन्तर्_स्मृतिः_ग्रन्थिः *N) {
    प्रत्यागम N->get_विधा() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
स्थिर sys::fs::UniqueID getUniqueID(hash_संहिता Hash) {
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

स्थितिः detail::New_अन्तर्_स्मृतिः_ग्रन्थिः_Info::make_स्थितिः() ध्रुव {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, बफर ? बफर->get_बफर() : "");

  प्रत्यागम स्थितिः(मार्गः, UID, llvm::sys::गन्तव्यम्_Time_Point(ModificationTime), User,
                गणः, बफर ? बफर->get_बफर_आकार() : 0, Type, Perms);
}

अन्तर्_स्मृतिः_File_System::अन्तर्_स्मृतिः_File_System(तार्किक UseNormalizedPaths)
    : मूलम्_2(नूतन detail::अन्तर्_स्मृतिः_Directory(
          स्थितिः("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::TimePoint<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

अन्तर्_स्मृतिः_File_System::~अन्तर्_स्मृतिः_File_System() = स्वभाव;

std::string अन्तर्_स्मृतिः_File_System::गन्तव्यम्_String() ध्रुव {
  प्रत्यागम मूलम्_2->गन्तव्यम्_String(/*Indent=*/0);
}

तार्किक अन्तर्_स्मृतिः_File_System::सङ्कलय_File(ध्रुव Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::स्मृतिः_बफर> बफर,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> गणः,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_ग्रन्थिः_Fn Make_ग्रन्थिः) {
  SmallString<128> मार्गः;
  P.गन्तव्यम्_सदिशम्(मार्गः);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_संहिता EC = makeAbsolute(मार्गः);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  यदि (useNormalizedPaths())
    llvm::sys::मार्गः_2::remove_dots(मार्गः, /*remove_dot_dot=*/सत्य);

  यदि (मार्गः.रिक्तानि())
    प्रत्यागम असत्य;

  detail::अन्तर्_स्मृतिः_Directory *Dir = मूलम्_2.get();
  स्वचालित I = llvm::sys::मार्गः_2::begin(मार्गः), E = sys::मार्गः_2::अन्तः(मार्गः);
  ध्रुव स्वचालित ResolvedUser = User.मानम्_or(0);
  ध्रुव स्वचालित Resolved_गणः = गणः.मानम्_or(0);
  ध्रुव स्वचालित ResolvedType = Type.मानम्_or(sys::fs::file_type::regular_file);
  ध्रुव स्वचालित ResolvedPerms = Perms.मानम्_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  ध्रुव स्वचालित NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  यावत् (सत्य) {
    Name = *I;
    ++I;
    यदि (I == E)
      भङ्ग;
    detail::अन्तर्_स्मृतिः_ग्रन्थिः *ग्रन्थिः = Dir->getChild(Name);
    यदि (!ग्रन्थिः) {
      // This isn't the last element, so we create a new directory.
      स्थितिः Stat(
          StringRef(मार्गः.str().begin(), Name.अन्तः() - मार्गः.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::गन्तव्यम्_Time_Point(ModificationTime), ResolvedUser, Resolved_गणः,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::अन्तर्_स्मृतिः_Directory>(Dir->सङ्कलय_Child(
          Name, std::make_unique<detail::अन्तर्_स्मृतिः_Directory>(std::move(Stat))));
      अनुवर्त;
    }
    // Creating file under another file.
    यदि (!isa<detail::अन्तर्_स्मृतिः_Directory>(ग्रन्थिः))
      प्रत्यागम असत्य;
    Dir = cast<detail::अन्तर्_स्मृतिः_Directory>(ग्रन्थिः);
  }
  detail::अन्तर्_स्मृतिः_ग्रन्थिः *ग्रन्थिः = Dir->getChild(Name);
  यदि (!ग्रन्थिः) {
    Dir->सङ्कलय_Child(Name,
                  Make_ग्रन्थिः({Dir->getUniqueID(), मार्गः, Name, ModificationTime,
                            std::move(बफर), ResolvedUser, Resolved_गणः,
                            ResolvedType, ResolvedPerms}));
    प्रत्यागम सत्य;
  }
  यदि (isa<detail::अन्तर्_स्मृतिः_Directory>(ग्रन्थिः))
    प्रत्यागम ResolvedType == sys::fs::file_type::directory_file;

  प्रतिज्ञा((isa<detail::अन्तर्_स्मृतिः_File>(ग्रन्थिः) ||
          isa<detail::अन्तर्_स्मृतिः_Hard_Link>(ग्रन्थिः)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  यदि (स्वचालित *Link = dyn_cast<detail::अन्तर्_स्मृतिः_Hard_Link>(ग्रन्थिः)) {
    प्रत्यागम Link->getResolvedFile().get_बफर()->get_बफर() ==
           बफर->get_बफर();
  }
  प्रत्यागम cast<detail::अन्तर्_स्मृतिः_File>(ग्रन्थिः)->get_बफर()->get_बफर() ==
         बफर->get_बफर();
}

तार्किक अन्तर्_स्मृतिः_File_System::सङ्कलय_File(ध्रुव Twine &P, time_t ModificationTime,
                                 std::unique_ptr<llvm::स्मृतिः_बफर> बफर,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> गणः,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  प्रत्यागम सङ्कलय_File(P, ModificationTime, std::move(बफर), User, गणः, Type,
                 Perms,
                 [](detail::New_अन्तर्_स्मृतिः_ग्रन्थिः_Info NNI)
                     -> std::unique_ptr<detail::अन्तर्_स्मृतिः_ग्रन्थिः> {
                   स्थितिः Stat = NNI.make_स्थितिः();
                   यदि (Stat.getType() == sys::fs::file_type::directory_file)
                     प्रत्यागम std::make_unique<detail::अन्तर्_स्मृतिः_Directory>(Stat);
                   प्रत्यागम std::make_unique<detail::अन्तर्_स्मृतिः_File>(
                       Stat, std::move(NNI.बफर));
                 });
}

तार्किक अन्तर्_स्मृतिः_File_System::सङ्कलय_File_No_Own(
    ध्रुव Twine &P, time_t ModificationTime,
    ध्रुव llvm::स्मृतिः_बफर_Ref &बफर, std::optional<uint32_t> User,
    std::optional<uint32_t> गणः, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  प्रत्यागम सङ्कलय_File(P, ModificationTime, llvm::स्मृतिः_बफर::get_Mem_बफर(बफर),
                 std::move(User), std::move(गणः), std::move(Type),
                 std::move(Perms),
                 [](detail::New_अन्तर्_स्मृतिः_ग्रन्थिः_Info NNI)
                     -> std::unique_ptr<detail::अन्तर्_स्मृतिः_ग्रन्थिः> {
                   स्थितिः Stat = NNI.make_स्थितिः();
                   यदि (Stat.getType() == sys::fs::file_type::directory_file)
                     प्रत्यागम std::make_unique<detail::अन्तर्_स्मृतिः_Directory>(Stat);
                   प्रत्यागम std::make_unique<detail::अन्तर्_स्मृतिः_File>(
                       Stat, std::move(NNI.बफर));
                 });
}

detail::Named_ग्रन्थिः_Or_Error
अन्तर्_स्मृतिः_File_System::lookup_ग्रन्थिः(ध्रुव Twine &P, तार्किक FollowFinalSymlink,
                               size_t Symlink_गभीरता) ध्रुव {
  SmallString<128> मार्गः;
  P.गन्तव्यम्_सदिशम्(मार्गः);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_संहिता EC = makeAbsolute(मार्गः);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  यदि (useNormalizedPaths())
    llvm::sys::मार्गः_2::remove_dots(मार्गः, /*remove_dot_dot=*/सत्य);

  ध्रुव detail::अन्तर्_स्मृतिः_Directory *Dir = मूलम्_2.get();
  यदि (मार्गः.रिक्तानि())
    प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(मार्गः, Dir);

  स्वचालित I = llvm::sys::मार्गः_2::begin(मार्गः), E = llvm::sys::मार्गः_2::अन्तः(मार्गः);
  यावत् (सत्य) {
    detail::अन्तर्_स्मृतिः_ग्रन्थिः *ग्रन्थिः = Dir->getChild(*I);
    ++I;
    यदि (!ग्रन्थिः)
      प्रत्यागम errc::no_such_file_or_directory;

    यदि (स्वचालित Symlink = dyn_cast<detail::अन्तर्_स्मृतिः_Symbolic_Link>(ग्रन्थिः)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      यदि (I == E && !FollowFinalSymlink)
        प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(मार्गः, Symlink);

      यदि (Symlink_गभीरता > अन्तर्_स्मृतिः_File_System::Max_Symlink_गभीरता)
        प्रत्यागम errc::no_such_file_or_directory;

      SmallString<128> Target_मार्गः = Symlink->get_Target_मार्गः();
      यदि (std::error_संहिता EC = makeAbsolute(Target_मार्गः))
        प्रत्यागम EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      स्वचालित Target =
          lookup_ग्रन्थिः(Target_मार्गः, /*FollowFinalSymlink=*/सत्य, Symlink_गभीरता + 1);
      यदि (!Target || I == E)
        प्रत्यागम Target;

      यदि (!isa<detail::अन्तर्_स्मृतिः_Directory>(*Target))
        प्रत्यागम errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::अन्तर्_स्मृतिः_Directory>(*Target);
      अनुवर्त;
    }

    // Return the file if it's at the end of the path.
    यदि (स्वचालित File = dyn_cast<detail::अन्तर्_स्मृतिः_File>(ग्रन्थिः)) {
      यदि (I == E)
        प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(मार्गः, File);
      प्रत्यागम errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    यदि (स्वचालित File = dyn_cast<detail::अन्तर्_स्मृतिः_Hard_Link>(ग्रन्थिः)) {
      यदि (I == E)
        प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(मार्गः, &File->getResolvedFile());
      प्रत्यागम errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::अन्तर्_स्मृतिः_Directory>(ग्रन्थिः);
    यदि (I == E)
      प्रत्यागम detail::Named_ग्रन्थिः_Or_Error(मार्गः, Dir);
  }
}

तार्किक अन्तर्_स्मृतिः_File_System::सङ्कलय_Hard_Link(ध्रुव Twine &NewLink,
                                     ध्रुव Twine &Target) {
  स्वचालित New_Link_ग्रन्थिः = lookup_ग्रन्थिः(NewLink, /*FollowFinalSymlink=*/असत्य);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  स्वचालित Target_ग्रन्थिः = lookup_ग्रन्थिः(Target, /*FollowFinalSymlink=*/सत्य);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  यदि (!Target_ग्रन्थिः || New_Link_ग्रन्थिः || !isa<detail::अन्तर्_स्मृतिः_File>(*Target_ग्रन्थिः))
    प्रत्यागम असत्य;
  प्रत्यागम सङ्कलय_File(NewLink, 0, शून्यसूचकः, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_अन्तर्_स्मृतिः_ग्रन्थिः_Info NNI) {
                   प्रत्यागम std::make_unique<detail::अन्तर्_स्मृतिः_Hard_Link>(
                       NNI.मार्गः.str(),
                       *cast<detail::अन्तर्_स्मृतिः_File>(*Target_ग्रन्थिः));
                 });
}

तार्किक अन्तर्_स्मृतिः_File_System::सङ्कलय_Symbolic_Link(
    ध्रुव Twine &NewLink, ध्रुव Twine &Target, time_t ModificationTime,
    std::optional<uint32_t> User, std::optional<uint32_t> गणः,
    std::optional<llvm::sys::fs::perms> Perms) {
  स्वचालित New_Link_ग्रन्थिः = lookup_ग्रन्थिः(NewLink, /*FollowFinalSymlink=*/असत्य);
  यदि (New_Link_ग्रन्थिः)
    प्रत्यागम असत्य;

  SmallString<128> NewLinkStr, TargetStr;
  NewLink.गन्तव्यम्_सदिशम्(NewLinkStr);
  Target.गन्तव्यम्_सदिशम्(TargetStr);

  प्रत्यागम सङ्कलय_File(NewLinkStr, ModificationTime, शून्यसूचकः, User, गणः,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_अन्तर्_स्मृतिः_ग्रन्थिः_Info NNI) {
                   प्रत्यागम std::make_unique<detail::अन्तर्_स्मृतिः_Symbolic_Link>(
                       NewLinkStr, TargetStr, NNI.make_स्थितिः());
                 });
}

llvm::ErrorOr<स्थितिः> अन्तर्_स्मृतिः_File_System::स्थितिः_2(ध्रुव Twine &मार्गः) {
  स्वचालित ग्रन्थिः = lookup_ग्रन्थिः(मार्गः, /*FollowFinalSymlink=*/सत्य);
  यदि (ग्रन्थिः)
    प्रत्यागम (*ग्रन्थिः)->get_स्थितिः(मार्गः);
  प्रत्यागम ग्रन्थिः.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
अन्तर्_स्मृतिः_File_System::openFileForRead(ध्रुव Twine &मार्गः) {
  स्वचालित ग्रन्थिः = lookup_ग्रन्थिः(मार्गः,/*FollowFinalSymlink=*/सत्य);
  यदि (!ग्रन्थिः)
    प्रत्यागम ग्रन्थिः.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  यदि (स्वचालित *F = dyn_cast<detail::अन्तर्_स्मृतिः_File>(*ग्रन्थिः))
    प्रत्यागम std::unique_ptr<File>(
        नूतन detail::अन्तर्_स्मृतिः_File_Adaptor(*F, मार्गः.str()));

  // FIXME: errc::not_a_file?
  प्रत्यागम make_error_संहिता(llvm::errc::invalid_तर्कः);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
वर्ग अन्तर्_स्मृतिः_File_System::DirIterator : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  ध्रुव अन्तर्_स्मृतिः_File_System *FS;
  detail::अन्तर्_स्मृतिः_Directory::const_iterator I;
  detail::अन्तर्_स्मृतिः_Directory::const_iterator E;
  std::string RequestedDirName;

  शून्य समुच्चयः_Current_प्रविष्टिः() {
    यदि (I != E) {
      SmallString<256> मार्गः(RequestedDirName);
      llvm::sys::मार्गः_2::append(मार्गः, I->second->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      विकल्प (I->second->get_विधा()) {
      पक्ष detail::IME_File:
      पक्ष detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        भङ्ग;
      पक्ष detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        भङ्ग;
      पक्ष detail::IME_SymbolicLink:
        यदि (स्वचालित SymlinkTarget =
                FS->lookup_ग्रन्थिः(मार्गः, /*FollowFinalSymlink=*/सत्य)) {
          मार्गः = SymlinkTarget.getName();
          Type = (*SymlinkTarget)->get_स्थितिः(मार्गः).getType();
        }
        भङ्ग;
      }
      Current_प्रविष्टिः = directory_प्रविष्टिः(std::string(मार्गः), Type);
    } अन्यथा {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_प्रविष्टिः = directory_प्रविष्टिः();
    }
  }

सार्वजनिक:
  DirIterator() = स्वभाव;

  DirIterator(ध्रुव अन्तर्_स्मृतिः_File_System *FS,
              ध्रुव detail::अन्तर्_स्मृतिः_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.अन्तः()),
        RequestedDirName(std::move(RequestedDirName)) {
    समुच्चयः_Current_प्रविष्टिः();
  }

  std::error_संहिता increment() override {
    ++I;
    समुच्चयः_Current_प्रविष्टिः();
    प्रत्यागम {};
  }
};

directory_iterator अन्तर्_स्मृतिः_File_System::dir_begin(ध्रुव Twine &Dir,
                                                 std::error_संहिता &EC) {
  स्वचालित ग्रन्थिः = lookup_ग्रन्थिः(Dir, /*FollowFinalSymlink=*/सत्य);
  यदि (!ग्रन्थिः) {
    EC = ग्रन्थिः.getError();
    प्रत्यागम directory_iterator(std::make_shared<DirIterator>());
  }

  यदि (स्वचालित *Dir_ग्रन्थिः = dyn_cast<detail::अन्तर्_स्मृतिः_Directory>(*ग्रन्थिः))
    प्रत्यागम directory_iterator(
        std::make_shared<DirIterator>(एतत्, *Dir_ग्रन्थिः, Dir.str()));

  EC = make_error_संहिता(llvm::errc::not_a_directory);
  प्रत्यागम directory_iterator(std::make_shared<DirIterator>());
}

std::error_संहिता अन्तर्_स्मृतिः_File_System::समुच्चयः_Current_Working_Directory(ध्रुव Twine &P) {
  SmallString<128> मार्गः;
  P.गन्तव्यम्_सदिशम्(मार्गः);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_संहिता EC = makeAbsolute(मार्गः);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  यदि (useNormalizedPaths())
    llvm::sys::मार्गः_2::remove_dots(मार्गः, /*remove_dot_dot=*/सत्य);

  यदि (!मार्गः.रिक्तानि())
    WorkingDirectory = std::string(मार्गः);
  प्रत्यागम {};
}

std::error_संहिता अन्तर्_स्मृतिः_File_System::get_Real_मार्गः(ध्रुव Twine &मार्गः,
                                                Small_सदिशम्_Impl<वर्ण> &Output) {
  स्वचालित CWD = getCurrentWorkingDirectory();
  यदि (!CWD || CWD->रिक्तानि())
    प्रत्यागम errc::operation_not_permitted;
  मार्गः.गन्तव्यम्_सदिशम्(Output);
  यदि (स्वचालित EC = makeAbsolute(Output))
    प्रत्यागम EC;
  llvm::sys::मार्गः_2::remove_dots(Output, /*remove_dot_dot=*/सत्य);
  प्रत्यागम {};
}

std::error_संहिता अन्तर्_स्मृतिः_File_System::isLocal(ध्रुव Twine &मार्गः, तार्किक &परिणाम) {
  परिणाम = असत्य;
  प्रत्यागम {};
}

शून्य अन्तर्_स्मृतिः_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   अचिह्नित IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

नामाकाश {

स्थिर llvm::sys::मार्गः_2::Style getExistingStyle(llvm::StringRef मार्गः) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::मार्गः_2::Style style = llvm::sys::मार्गः_2::Style::native;
  ध्रुव size_t n = मार्गः.find_first_सम्बन्धि("/\\");
  // Can't distinguish between posix and windows_slash here.
  यदि (n != static_cast<size_t>(-1))
    style = (मार्गः[n] == '/') ? llvm::sys::मार्गः_2::Style::posix
                             : llvm::sys::मार्गः_2::Style::windows_backslash;
  प्रत्यागम style;
}

/// Removes leading "./" as well as path components like ".." and ".".
स्थिर llvm::SmallString<256> canonicalize(llvm::StringRef मार्गः) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::मार्गः_2::Style style = getExistingStyle(मार्गः);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> परिणाम_2 =
      llvm::sys::मार्गः_2::remove_leading_dotslash(मार्गः, style);
  llvm::sys::मार्गः_2::remove_dots(परिणाम_2, /*remove_dot_dot=*/सत्य, style);
  प्रत्यागम परिणाम_2;
}

/// Whether the error and entry specify a file/directory that was not found.
स्थिर तार्किक is_File_Not_प्राप्तम्(std::error_संहिता EC,
                           RedirectingFileSystem::प्रविष्टिः *E = शून्यसूचकः) {
  यदि (E && !isa<RedirectingFileSystem::Directory_Remap_प्रविष्टिः>(E))
    प्रत्यागम असत्य;
  प्रत्यागम EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  प्रतिज्ञा(ExternalFS && "RedirectingFileSystem requires an external FS");
  यदि (स्वचालित ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
वर्ग llvm::vfs::RedirectingFSDirIterImpl
    : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::Directory_प्रविष्टिः::iterator Current, अन्तः_2;

  std::error_संहिता incrementImpl(तार्किक IsFirstTime) {
    प्रतिज्ञा((IsFirstTime || Current != अन्तः_2) && "cannot iterate past end");
    यदि (!IsFirstTime)
      ++Current;
    यदि (Current != अन्तः_2) {
      SmallString<128> मार्गः_Str(Dir);
      llvm::sys::मार्गः_2::append(मार्गः_Str, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      विकल्प ((*Current)->get_विधा()) {
      पक्ष RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      पक्ष RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        भङ्ग;
      पक्ष RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        भङ्ग;
      }
      Current_प्रविष्टिः = directory_प्रविष्टिः(std::string(मार्गः_Str), Type);
    } अन्यथा {
      Current_प्रविष्टिः = directory_प्रविष्टिः();
    }
    प्रत्यागम {};
  };

सार्वजनिक:
  RedirectingFSDirIterImpl(
      ध्रुव Twine &मार्गः, RedirectingFileSystem::Directory_प्रविष्टिः::iterator Begin,
      RedirectingFileSystem::Directory_प्रविष्टिः::iterator अन्तः_2, std::error_संहिता &EC)
      : Dir(मार्गः.str()), Current(Begin), अन्तः_2(अन्तः_2) {
    EC = incrementImpl(/*IsFirstTime=*/सत्य);
  }

  std::error_संहिता increment() override {
    प्रत्यागम incrementImpl(/*IsFirstTime=*/असत्य);
  }
};

नामाकाश {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
वर्ग RedirectingFSDirRemapIterImpl : सार्वजनिक llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::मार्गः_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

सार्वजनिक:
  RedirectingFSDirRemapIterImpl(std::string Dir_मार्गः,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_मार्गः)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    यदि (ExternalIter != llvm::vfs::directory_iterator())
      समुच्चयः_Current_प्रविष्टिः();
  }

  शून्य समुच्चयः_Current_प्रविष्टिः() {
    StringRef External_मार्गः = ExternalIter->मार्गः_2();
    llvm::sys::मार्गः_2::Style ExternalStyle = getExistingStyle(External_मार्गः);
    StringRef File = llvm::sys::मार्गः_2::filename(External_मार्गः, ExternalStyle);

    SmallString<128> New_मार्गः(Dir);
    llvm::sys::मार्गः_2::append(New_मार्गः, DirStyle, File);

    Current_प्रविष्टिः = directory_प्रविष्टिः(std::string(New_मार्गः), ExternalIter->type());
  }

  std::error_संहिता increment() override {
    std::error_संहिता EC;
    ExternalIter.increment(EC);
    यदि (!EC && ExternalIter != llvm::vfs::directory_iterator())
      समुच्चयः_Current_प्रविष्टिः();
    अन्यथा
      Current_प्रविष्टिः = directory_प्रविष्टिः();
    प्रत्यागम EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() ध्रुव {
  प्रत्यागम WorkingDirectory;
}

std::error_संहिता
RedirectingFileSystem::समुच्चयः_Current_Working_Directory(ध्रुव Twine &मार्गः) {
  // Don't change the working directory if the path doesn't exist.
  यदि (!exists(मार्गः))
    प्रत्यागम errc::no_such_file_or_directory;

  SmallString<128> Absolute_मार्गः;
  मार्गः.गन्तव्यम्_सदिशम्(Absolute_मार्गः);
  यदि (std::error_संहिता EC = makeAbsolute(Absolute_मार्गः))
    प्रत्यागम EC;
  WorkingDirectory = std::string(Absolute_मार्गः);
  प्रत्यागम {};
}

std::error_संहिता RedirectingFileSystem::isLocal(ध्रुव Twine &मार्गः_3,
                                               तार्किक &परिणाम) {
  SmallString<256> मार्गः;
  मार्गः_3.गन्तव्यम्_सदिशम्(मार्गः);

  यदि (makeAbsolute(मार्गः))
    प्रत्यागम {};

  प्रत्यागम ExternalFS->isLocal(मार्गः, परिणाम);
}

std::error_संहिता RedirectingFileSystem::makeAbsolute(Small_सदिशम्_Impl<वर्ण> &मार्गः) ध्रुव {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  यदि (llvm::sys::मार्गः_2::is_absolute(मार्गः, llvm::sys::मार्गः_2::Style::posix) ||
      llvm::sys::मार्गः_2::is_absolute(मार्गः,
                                   llvm::sys::मार्गः_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    प्रत्यागम {};

  स्वचालित WorkingDir = getCurrentWorkingDirectory();
  यदि (!WorkingDir)
    प्रत्यागम WorkingDir.getError();

  प्रत्यागम makeAbsolute(WorkingDir.get(), मार्गः);
}

std::error_संहिता
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_सदिशम्_Impl<वर्ण> &मार्गः) ध्रुव {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  यदि (!WorkingDir.रिक्तानि() &&
      !sys::मार्गः_2::is_absolute(WorkingDir, sys::मार्गः_2::Style::posix) &&
      !sys::मार्गः_2::is_absolute(WorkingDir,
                              sys::मार्गः_2::Style::windows_backslash)) {
    प्रत्यागम std::error_संहिता();
  }
  sys::मार्गः_2::Style style = sys::मार्गः_2::Style::windows_backslash;
  यदि (sys::मार्गः_2::is_absolute(WorkingDir, sys::मार्गः_2::Style::posix)) {
    style = sys::मार्गः_2::Style::posix;
  } अन्यथा {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    यदि (getExistingStyle(WorkingDir) != sys::मार्गः_2::Style::windows_backslash)
      style = sys::मार्गः_2::Style::windows_slash;
  }

  std::string परिणाम = std::string(WorkingDir);
  StringRef Dir(परिणाम);
  यदि (!Dir.अन्ताः_with(sys::मार्गः_2::get_separator(style))) {
    परिणाम += sys::मार्गः_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  परिणाम.append(मार्गः.दत्त(), मार्गः.आकार_2());
  मार्गः.नियोजय(परिणाम.begin(), परिणाम.अन्तः());

  प्रत्यागम {};
}

directory_iterator RedirectingFileSystem::dir_begin(ध्रुव Twine &Dir,
                                                    std::error_संहिता &EC) {
  SmallString<256> मार्गः;
  Dir.गन्तव्यम्_सदिशम्(मार्गः);

  EC = makeAbsolute(मार्गः);
  यदि (EC)
    प्रत्यागम {};

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookup_मार्गः(मार्गः);
  यदि (!परिणाम) {
    यदि (Redirection != Redirect_विधा::RedirectOnly &&
        is_File_Not_प्राप्तम्(परिणाम.getError()))
      प्रत्यागम ExternalFS->dir_begin(मार्गः, EC);

    EC = परिणाम.getError();
    प्रत्यागम {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<स्थितिः> S = स्थितिः_2(मार्गः, Dir, *परिणाम);
  यदि (!S) {
    यदि (Redirection != Redirect_विधा::RedirectOnly &&
        is_File_Not_प्राप्तम्(S.getError(), परिणाम->E))
      प्रत्यागम ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    प्रत्यागम {};
  }

  यदि (!S->isDirectory()) {
    EC = errc::not_a_directory;
    प्रत्यागम {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_संहिता RedirectEC;
  यदि (स्वचालित ExtRedirect = परिणाम->getExternalRedirect()) {
    स्वचालित RE = cast<RedirectingFileSystem::Remap_प्रविष्टिः>(परिणाम->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    यदि (!RE->useExternalName(Use_External_नामानि)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(मार्गः), RedirectIter));
    }
  } अन्यथा {
    स्वचालित DE = cast<Directory_प्रविष्टिः>(परिणाम->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            मार्गः, DE->contents_begin(), DE->contents_अन्तः(), RedirectEC));
  }

  यदि (RedirectEC) {
    यदि (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      प्रत्यागम {};
    }
    RedirectIter = {};
  }

  यदि (Redirection == Redirect_विधा::RedirectOnly) {
    EC = RedirectEC;
    प्रत्यागम RedirectIter;
  }

  std::error_संहिता ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(मार्गः, ExternalEC);
  यदि (ExternalEC) {
    यदि (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      प्रत्यागम {};
    }
    ExternalIter = {};
  }

  Small_सदिशम्<directory_iterator, 2> Iters;
  विकल्प (Redirection) {
  पक्ष Redirect_विधा::Fallthrough:
    Iters.प्रक्षिप_पृष्ठभागः(ExternalIter);
    Iters.प्रक्षिप_पृष्ठभागः(RedirectIter);
    भङ्ग;
  पक्ष Redirect_विधा::Fallback:
    Iters.प्रक्षिप_पृष्ठभागः(RedirectIter);
    Iters.प्रक्षिप_पृष्ठभागः(ExternalIter);
    भङ्ग;
  स्वभाव:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  यदि (EC)
    प्रत्यागम {};
  प्रत्यागम Combined;
}

शून्य RedirectingFileSystem::समुच्चयः_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() ध्रुव {
  प्रत्यागम OverlayFileDir;
}

शून्य RedirectingFileSystem::समुच्चयः_Fallthrough(तार्किक Fallthrough) {
  यदि (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_विधा::Fallthrough;
  } अन्यथा {
    Redirection = RedirectingFileSystem::Redirect_विधा::RedirectOnly;
  }
}

शून्य RedirectingFileSystem::समुच्चयः_Redirection(
    RedirectingFileSystem::Redirect_विधा विधा_2) {
  Redirection = विधा_2;
}

std::सदिशम्<StringRef> RedirectingFileSystem::getRoots() ध्रुव {
  std::सदिशम्<StringRef> R;
  R.reserve(Roots.आकार_2());
  प्रति (ध्रुव स्वचालित &मूलम्_2 : Roots)
    R.प्रक्षिप_पृष्ठभागः(मूलम्_2->getName());
  प्रत्यागम R;
}

शून्य RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      अचिह्नित IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_नामानि ? "true" : "false") << ")\n";
  यदि (Type == PrintType::Summary)
    प्रत्यागम;

  प्रति (ध्रुव स्वचालित &मूलम्_2 : Roots)
    print_प्रविष्टिः(OS, मूलम्_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

शून्य RedirectingFileSystem::print_प्रविष्टिः(raw_ostream &OS,
                                       RedirectingFileSystem::प्रविष्टिः *E,
                                       अचिह्नित IndentLevel) ध्रुव {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  विकल्प (E->get_विधा()) {
  पक्ष EK_Directory: {
    स्वचालित *DE = cast<RedirectingFileSystem::Directory_प्रविष्टिः>(E);

    OS << "\n";
    प्रति (std::unique_ptr<प्रविष्टिः> &Sub_प्रविष्टिः :
         llvm::make_range(DE->contents_begin(), DE->contents_अन्तः()))
      print_प्रविष्टिः(OS, Sub_प्रविष्टिः.get(), IndentLevel + 1);
    भङ्ग;
  }
  पक्ष EK_DirectoryRemap:
  पक्ष EK_File: {
    स्वचालित *RE = cast<RedirectingFileSystem::Remap_प्रविष्टिः>(E);
    OS << " -> '" << RE->get_External_Contents_मार्गः() << "'";
    विकल्प (RE->getUseName()) {
    पक्ष NK_Not_समुच्चयः:
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

शून्य RedirectingFileSystem::दर्शय_Child_File_Systems(दर्शय_Callback_Ty Callback) {
  यदि (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->दर्शय_Child_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
वर्ग llvm::vfs::RedirectingFileSystemParser {
  yaml::प्रवाहः &प्रवाहः;

  शून्य error(yaml::ग्रन्थिः *N, ध्रुव Twine &सन्देशः_4) { प्रवाहः.printError(N, सन्देशः_4); }

  // false on error
  तार्किक parseScalarString(yaml::ग्रन्थिः *N, StringRef &परिणाम,
                         Small_सदिशम्_Impl<वर्ण> &Storage) {
    ध्रुव स्वचालित *S = dyn_cast_if_present<yaml::Scalar_ग्रन्थिः>(N);

    यदि (!S) {
      error(N, "expected string");
      प्रत्यागम असत्य;
    }
    परिणाम = S->get_मानम्(Storage);
    प्रत्यागम सत्य;
  }

  // false on error
  तार्किक parseScalarBool(yaml::ग्रन्थिः *N, तार्किक &परिणाम) {
    SmallString<5> Storage;
    StringRef मानम्;
    यदि (!parseScalarString(N, मानम्, Storage))
      प्रत्यागम असत्य;

    यदि (मानम्.equals_insensitive("true") || मानम्.equals_insensitive("on") ||
        मानम्.equals_insensitive("yes") || मानम् == "1") {
      परिणाम = सत्य;
      प्रत्यागम सत्य;
    } अन्यथा यदि (मानम्.equals_insensitive("false") ||
               मानम्.equals_insensitive("off") ||
               मानम्.equals_insensitive("no") || मानम् == "0") {
      परिणाम = असत्य;
      प्रत्यागम सत्य;
    }

    error(N, "expected boolean value");
    प्रत्यागम असत्य;
  }

  std::optional<RedirectingFileSystem::Redirect_विधा>
  parse_Redirect_विधा(yaml::ग्रन्थिः *N) {
    SmallString<12> Storage;
    StringRef मानम्;
    यदि (!parseScalarString(N, मानम्, Storage))
      प्रत्यागम std::nullopt;

    यदि (मानम्.equals_insensitive("fallthrough")) {
      प्रत्यागम RedirectingFileSystem::Redirect_विधा::Fallthrough;
    } अन्यथा यदि (मानम्.equals_insensitive("fallback")) {
      प्रत्यागम RedirectingFileSystem::Redirect_विधा::Fallback;
    } अन्यथा यदि (मानम्.equals_insensitive("redirect-only")) {
      प्रत्यागम RedirectingFileSystem::Redirect_विधा::RedirectOnly;
    }
    प्रत्यागम std::nullopt;
  }

  std::optional<RedirectingFileSystem::मूलम्_Relative_विधा>
  parse_मूलम्_Relative_विधा(yaml::ग्रन्थिः *N) {
    SmallString<12> Storage;
    StringRef मानम्;
    यदि (!parseScalarString(N, मानम्, Storage))
      प्रत्यागम std::nullopt;
    यदि (मानम्.equals_insensitive("cwd")) {
      प्रत्यागम RedirectingFileSystem::मूलम्_Relative_विधा::CWD;
    } अन्यथा यदि (मानम्.equals_insensitive("overlay-dir")) {
      प्रत्यागम RedirectingFileSystem::मूलम्_Relative_विधा::OverlayDir;
    }
    प्रत्यागम std::nullopt;
  }

  संरचना कुञ्चिका_स्थितिः {
    तार्किक Required;
    तार्किक आलोकितम् = असत्य;

    कुञ्चिका_स्थितिः(तार्किक Required = असत्य) : Required(Required) {}
  };

  उपयुज्य कुञ्चिका_स्थितिः_Pair = std::pair<StringRef, कुञ्चिका_स्थितिः>;

  // false on error
  तार्किक check_Duplicate_Or_Unknown_कुञ्चिका(yaml::ग्रन्थिः *कुञ्चिका_ग्रन्थिः, StringRef कुञ्चिका,
                                  DenseMap<StringRef, कुञ्चिका_स्थितिः> &कुञ्चिकाः_2) {
    स्वचालित It = कुञ्चिकाः_2.find(कुञ्चिका);
    यदि (It == कुञ्चिकाः_2.अन्तः()) {
      error(कुञ्चिका_ग्रन्थिः, "unknown key");
      प्रत्यागम असत्य;
    }
    कुञ्चिका_स्थितिः &S = It->second;
    यदि (S.आलोकितम्) {
      error(कुञ्चिका_ग्रन्थिः, Twine("duplicate key '") + कुञ्चिका + "'");
      प्रत्यागम असत्य;
    }
    S.आलोकितम् = सत्य;
    प्रत्यागम सत्य;
  }

  // false on error
  तार्किक check_Missing_कुञ्चिकाः(yaml::ग्रन्थिः *Obj, DenseMap<StringRef, कुञ्चिका_स्थितिः> &कुञ्चिकाः_2) {
    प्रति (ध्रुव स्वचालित &I : कुञ्चिकाः_2) {
      यदि (I.second.Required && !I.second.आलोकितम्) {
        error(Obj, Twine("missing key '") + I.first + "'");
        प्रत्यागम असत्य;
      }
    }
    प्रत्यागम सत्य;
  }

सार्वजनिक:
  स्थिर RedirectingFileSystem::प्रविष्टिः *
  lookup_Or_Create_प्रविष्टिः(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::प्रविष्टिः *Parent_प्रविष्टिः = शून्यसूचकः) {
    यदि (!Parent_प्रविष्टिः) { // Look for a existent root
      प्रति (ध्रुव स्वचालित &मूलम्_2 : FS->Roots) {
        यदि (Name == मूलम्_2->getName()) {
          Parent_प्रविष्टिः = मूलम्_2.get();
          प्रत्यागम Parent_प्रविष्टिः;
        }
      }
    } अन्यथा { // Advance to the next component
      स्वचालित *DE = dyn_cast<RedirectingFileSystem::Directory_प्रविष्टिः>(Parent_प्रविष्टिः);
      प्रति (std::unique_ptr<RedirectingFileSystem::प्रविष्टिः> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_अन्तः())) {
        स्वचालित *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_प्रविष्टिः>(Content.get());
        यदि (DirContent && Name == Content->getName())
          प्रत्यागम DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::प्रविष्टिः> E =
        std::make_unique<RedirectingFileSystem::Directory_प्रविष्टिः>(
            Name, स्थितिः("", get_अग्र_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    यदि (!Parent_प्रविष्टिः) { // Add a new root to the overlay
      FS->Roots.प्रक्षिप_पृष्ठभागः(std::move(E));
      Parent_प्रविष्टिः = FS->Roots.पृष्ठभागः().get();
      प्रत्यागम Parent_प्रविष्टिः;
    }

    स्वचालित *DE = cast<RedirectingFileSystem::Directory_प्रविष्टिः>(Parent_प्रविष्टिः);
    DE->सङ्कलय_Content(std::move(E));
    प्रत्यागम DE->get_चरमः_Content();
  }

गुप्त:
  शून्य unique_Overlay_वृक्षः(RedirectingFileSystem *FS,
                         RedirectingFileSystem::प्रविष्टिः *SrcE,
                         RedirectingFileSystem::प्रविष्टिः *NewParentE = शून्यसूचकः) {
    StringRef Name = SrcE->getName();
    विकल्प (SrcE->get_विधा()) {
    पक्ष RedirectingFileSystem::EK_Directory: {
      स्वचालित *DE = cast<RedirectingFileSystem::Directory_प्रविष्टिः>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      यदि (!Name.रिक्तानि())
        NewParentE = lookup_Or_Create_प्रविष्टिः(FS, Name, NewParentE);
      प्रति (std::unique_ptr<RedirectingFileSystem::प्रविष्टिः> &Sub_प्रविष्टिः :
           llvm::make_range(DE->contents_begin(), DE->contents_अन्तः()))
        unique_Overlay_वृक्षः(FS, Sub_प्रविष्टिः.get(), NewParentE);
      भङ्ग;
    }
    पक्ष RedirectingFileSystem::EK_DirectoryRemap: {
      प्रतिज्ञा(NewParentE && "Parent entry must exist");
      स्वचालित *DR = cast<RedirectingFileSystem::Directory_Remap_प्रविष्टिः>(SrcE);
      स्वचालित *DE = cast<RedirectingFileSystem::Directory_प्रविष्टिः>(NewParentE);
      DE->सङ्कलय_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_प्रविष्टिः>(
              Name, DR->get_External_Contents_मार्गः(), DR->getUseName()));
      भङ्ग;
    }
    पक्ष RedirectingFileSystem::EK_File: {
      प्रतिज्ञा(NewParentE && "Parent entry must exist");
      स्वचालित *FE = cast<RedirectingFileSystem::File_प्रविष्टिः>(SrcE);
      स्वचालित *DE = cast<RedirectingFileSystem::Directory_प्रविष्टिः>(NewParentE);
      DE->सङ्कलय_Content(std::make_unique<RedirectingFileSystem::File_प्रविष्टिः>(
          Name, FE->get_External_Contents_मार्गः(), FE->getUseName()));
      भङ्ग;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::प्रविष्टिः>
  parse_प्रविष्टिः(yaml::ग्रन्थिः *N, RedirectingFileSystem *FS, तार्किक Is_मूलम्_प्रविष्टिः) {
    स्वचालित *M = dyn_cast<yaml::Mapping_ग्रन्थिः>(N);
    यदि (!M) {
      error(N, "expected mapping node for file or directory entry");
      प्रत्यागम शून्यसूचकः;
    }

    कुञ्चिका_स्थितिः_Pair Fields[] = {
        कुञ्चिका_स्थितिः_Pair("name", सत्य),
        कुञ्चिका_स्थितिः_Pair("type", सत्य),
        कुञ्चिका_स्थितिः_Pair("contents", असत्य),
        कुञ्चिका_स्थितिः_Pair("external-contents", असत्य),
        कुञ्चिका_स्थितिः_Pair("use-external-name", असत्य),
    };

    DenseMap<StringRef, कुञ्चिका_स्थितिः> कुञ्चिकाः_2(std::begin(Fields), std::अन्तः(Fields));

    गणना { CF_Not_समुच्चयः, CF_List, CF_External } Contents_क्षेत्रम् = CF_Not_समुच्चयः;
    std::सदिशम्<std::unique_ptr<RedirectingFileSystem::प्रविष्टिः>>
        प्रविष्टिः_Array_Contents;
    SmallString<256> External_Contents_मार्गः;
    SmallString<256> Name;
    yaml::ग्रन्थिः *Name_मानम्_ग्रन्थिः = शून्यसूचकः;
    स्वचालित UseExternalName = RedirectingFileSystem::NK_Not_समुच्चयः;
    RedirectingFileSystem::प्रविष्टिः_विधा विधा_2;

    प्रति (स्वचालित &I : *M) {
      StringRef कुञ्चिका;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> बफर;
      यदि (!parseScalarString(I.get_कुञ्चिका(), कुञ्चिका, बफर))
        प्रत्यागम शून्यसूचकः;

      यदि (!check_Duplicate_Or_Unknown_कुञ्चिका(I.get_कुञ्चिका(), कुञ्चिका, कुञ्चिकाः_2))
        प्रत्यागम शून्यसूचकः;

      StringRef मानम्;
      यदि (कुञ्चिका == "name") {
        यदि (!parseScalarString(I.get_मानम्(), मानम्, बफर))
          प्रत्यागम शून्यसूचकः;

        Name_मानम्_ग्रन्थिः = I.get_मानम्();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(मानम्).str();
      } अन्यथा यदि (कुञ्चिका == "type") {
        यदि (!parseScalarString(I.get_मानम्(), मानम्, बफर))
          प्रत्यागम शून्यसूचकः;
        यदि (मानम् == "file")
          विधा_2 = RedirectingFileSystem::EK_File;
        अन्यथा यदि (मानम् == "directory")
          विधा_2 = RedirectingFileSystem::EK_Directory;
        अन्यथा यदि (मानम् == "directory-remap")
          विधा_2 = RedirectingFileSystem::EK_DirectoryRemap;
        अन्यथा {
          error(I.get_मानम्(), "unknown value for 'type'");
          प्रत्यागम शून्यसूचकः;
        }
      } अन्यथा यदि (कुञ्चिका == "contents") {
        यदि (Contents_क्षेत्रम् != CF_Not_समुच्चयः) {
          error(I.get_कुञ्चिका(),
                "entry already has 'contents' or 'external-contents'");
          प्रत्यागम शून्यसूचकः;
        }
        Contents_क्षेत्रम् = CF_List;
        स्वचालित *Contents = dyn_cast_if_present<yaml::Sequence_ग्रन्थिः>(I.get_मानम्());
        यदि (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_मानम्(), "expected array");
          प्रत्यागम शून्यसूचकः;
        }

        प्रति (स्वचालित &I : *Contents) {
          यदि (std::unique_ptr<RedirectingFileSystem::प्रविष्टिः> E =
                  parse_प्रविष्टिः(&I, FS, /*IsRootEntry*/ असत्य))
            प्रविष्टिः_Array_Contents.प्रक्षिप_पृष्ठभागः(std::move(E));
          अन्यथा
            प्रत्यागम शून्यसूचकः;
        }
      } अन्यथा यदि (कुञ्चिका == "external-contents") {
        यदि (Contents_क्षेत्रम् != CF_Not_समुच्चयः) {
          error(I.get_कुञ्चिका(),
                "entry already has 'contents' or 'external-contents'");
          प्रत्यागम शून्यसूचकः;
        }
        Contents_क्षेत्रम् = CF_External;
        यदि (!parseScalarString(I.get_मानम्(), मानम्, बफर))
          प्रत्यागम शून्यसूचकः;

        SmallString<256> पूर्णम्_मार्गः;
        यदि (FS->IsRelativeOverlay) {
          पूर्णम्_मार्गः = FS->getOverlayFileDir();
          प्रतिज्ञा(!पूर्णम्_मार्गः.रिक्तानि() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_पूर्णम्_मार्गः = मानम्;
          यदि (FS->makeAbsolute(पूर्णम्_मार्गः, Abs_पूर्णम्_मार्गः)) {
            error(N, "failed to make 'external-contents' absolute");
            प्रत्यागम शून्यसूचकः;
          }
          पूर्णम्_मार्गः = Abs_पूर्णम्_मार्गः;
        } अन्यथा {
          पूर्णम्_मार्गः = मानम्;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        पूर्णम्_मार्गः = canonicalize(पूर्णम्_मार्गः);
        External_Contents_मार्गः = पूर्णम्_मार्गः.str();
      } अन्यथा यदि (कुञ्चिका == "use-external-name") {
        तार्किक Val;
        यदि (!parseScalarBool(I.get_मानम्(), Val))
          प्रत्यागम शून्यसूचकः;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } अन्यथा {
        llvm_unreachable("key missing from Keys");
      }
    }

    यदि (प्रवाहः.failed())
      प्रत्यागम शून्यसूचकः;

    // check for missing keys
    यदि (Contents_क्षेत्रम् == CF_Not_समुच्चयः) {
      error(N, "missing key 'contents' or 'external-contents'");
      प्रत्यागम शून्यसूचकः;
    }
    यदि (!check_Missing_कुञ्चिकाः(N, कुञ्चिकाः_2))
      प्रत्यागम शून्यसूचकः;

    // check invalid configuration
    यदि (विधा_2 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_समुच्चयः) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      प्रत्यागम शून्यसूचकः;
    }

    यदि (विधा_2 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_क्षेत्रम् == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      प्रत्यागम शून्यसूचकः;
    }

    sys::मार्गः_2::Style मार्गः_style = sys::मार्गः_2::Style::native;
    यदि (Is_मूलम्_प्रविष्टिः) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      यदि (sys::मार्गः_2::is_absolute(Name, sys::मार्गः_2::Style::posix)) {
        मार्गः_style = sys::मार्गः_2::Style::posix;
      } अन्यथा यदि (sys::मार्गः_2::is_absolute(Name,
                                        sys::मार्गः_2::Style::windows_backslash)) {
        मार्गः_style = sys::मार्गः_2::Style::windows_backslash;
      } अन्यथा {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_संहिता EC;
        यदि (FS->मूलम्_Relative ==
            RedirectingFileSystem::मूलम्_Relative_विधा::OverlayDir) {
          StringRef पूर्णम्_मार्गः = FS->getOverlayFileDir();
          प्रतिज्ञा(!पूर्णम्_मार्गः.रिक्तानि() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(पूर्णम्_मार्गः, Name);
          Name = canonicalize(Name);
        } अन्यथा {
          EC = FS->makeAbsolute(Name);
        }
        यदि (EC) {
          प्रतिज्ञा(Name_मानम्_ग्रन्थिः && "Name presence should be checked earlier");
          error(
              Name_मानम्_ग्रन्थिः,
              "entry with relative path at the root level is not discoverable");
          प्रत्यागम शून्यसूचकः;
        }
        मार्गः_style = sys::मार्गः_2::is_absolute(Name, sys::मार्गः_2::Style::posix)
                         ? sys::मार्गः_2::Style::posix
                         : sys::मार्गः_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      यदि (मार्गः_style == sys::मार्गः_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::मार्गः_2::Style::windows_backslash)
        मार्गः_style = sys::मार्गः_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t मूलम्_मार्गः_Len = sys::मार्गः_2::मूलम्_मार्गः_2(Trimmed, मार्गः_style).आकार_2();
    यावत् (Trimmed.आकार_2() > मूलम्_मार्गः_Len &&
           sys::मार्गः_2::is_separator(Trimmed.पृष्ठभागः(), मार्गः_style))
      Trimmed = Trimmed.खण्डिका(0, Trimmed.आकार_2() - 1);

    // Get the last component
    StringRef चरमः_Component = sys::मार्गः_2::filename(Trimmed, मार्गः_style);

    std::unique_ptr<RedirectingFileSystem::प्रविष्टिः> परिणाम;
    विकल्प (विधा_2) {
    पक्ष RedirectingFileSystem::EK_File:
      परिणाम = std::make_unique<RedirectingFileSystem::File_प्रविष्टिः>(
          चरमः_Component, std::move(External_Contents_मार्गः), UseExternalName);
      भङ्ग;
    पक्ष RedirectingFileSystem::EK_DirectoryRemap:
      परिणाम = std::make_unique<RedirectingFileSystem::Directory_Remap_प्रविष्टिः>(
          चरमः_Component, std::move(External_Contents_मार्गः), UseExternalName);
      भङ्ग;
    पक्ष RedirectingFileSystem::EK_Directory:
      परिणाम = std::make_unique<RedirectingFileSystem::Directory_प्रविष्टिः>(
          चरमः_Component, std::move(प्रविष्टिः_Array_Contents),
          स्थितिः("", get_अग्र_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      भङ्ग;
    }

    StringRef Parent = sys::मार्गः_2::parent_मार्गः(Trimmed, मार्गः_style);
    यदि (Parent.रिक्तानि())
      प्रत्यागम परिणाम;

    // if 'name' contains multiple components, create implicit directory entries
    प्रति (sys::मार्गः_2::reverse_iterator I = sys::मार्गः_2::rbegin(Parent, मार्गः_style),
                                     E = sys::मार्गः_2::rend(Parent);
         I != E; ++I) {
      std::सदिशम्<std::unique_ptr<RedirectingFileSystem::प्रविष्टिः>> Entries;
      Entries.प्रक्षिप_पृष्ठभागः(std::move(परिणाम));
      परिणाम = std::make_unique<RedirectingFileSystem::Directory_प्रविष्टिः>(
          *I, std::move(Entries),
          स्थितिः("", get_अग्र_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    प्रत्यागम परिणाम;
  }

सार्वजनिक:
  RedirectingFileSystemParser(yaml::प्रवाहः &S) : प्रवाहः(S) {}

  // false on error
  तार्किक parse(yaml::ग्रन्थिः *मूलम्_2, RedirectingFileSystem *FS) {
    स्वचालित *शिखरम् = dyn_cast<yaml::Mapping_ग्रन्थिः>(मूलम्_2);
    यदि (!शिखरम्) {
      error(मूलम्_2, "expected mapping node");
      प्रत्यागम असत्य;
    }

    कुञ्चिका_स्थितिः_Pair Fields[] = {
        कुञ्चिका_स्थितिः_Pair("version", सत्य),
        कुञ्चिका_स्थितिः_Pair("case-sensitive", असत्य),
        कुञ्चिका_स्थितिः_Pair("use-external-names", असत्य),
        कुञ्चिका_स्थितिः_Pair("root-relative", असत्य),
        कुञ्चिका_स्थितिः_Pair("overlay-relative", असत्य),
        कुञ्चिका_स्थितिः_Pair("fallthrough", असत्य),
        कुञ्चिका_स्थितिः_Pair("redirecting-with", असत्य),
        कुञ्चिका_स्थितिः_Pair("roots", सत्य),
    };

    DenseMap<StringRef, कुञ्चिका_स्थितिः> कुञ्चिकाः_2(std::begin(Fields), std::अन्तः(Fields));
    std::सदिशम्<std::unique_ptr<RedirectingFileSystem::प्रविष्टिः>> मूलम्_Entries;

    // Parse configuration and 'roots'
    प्रति (स्वचालित &I : *शिखरम्) {
      SmallString<10> कुञ्चिका_बफर;
      StringRef कुञ्चिका;
      यदि (!parseScalarString(I.get_कुञ्चिका(), कुञ्चिका, कुञ्चिका_बफर))
        प्रत्यागम असत्य;

      यदि (!check_Duplicate_Or_Unknown_कुञ्चिका(I.get_कुञ्चिका(), कुञ्चिका, कुञ्चिकाः_2))
        प्रत्यागम असत्य;

      यदि (कुञ्चिका == "roots") {
        स्वचालित *Roots = dyn_cast_if_present<yaml::Sequence_ग्रन्थिः>(I.get_मानम्());
        यदि (!Roots) {
          error(I.get_मानम्(), "expected array");
          प्रत्यागम असत्य;
        }

        प्रति (स्वचालित &I : *Roots) {
          यदि (std::unique_ptr<RedirectingFileSystem::प्रविष्टिः> E =
                  parse_प्रविष्टिः(&I, FS, /*IsRootEntry*/ सत्य))
            मूलम्_Entries.प्रक्षिप_पृष्ठभागः(std::move(E));
          अन्यथा
            प्रत्यागम असत्य;
        }
      } अन्यथा यदि (कुञ्चिका == "version") {
        StringRef संस्करणम्_String;
        SmallString<4> Storage;
        यदि (!parseScalarString(I.get_मानम्(), संस्करणम्_String, Storage))
          प्रत्यागम असत्य;
        पूर्णाङ्क संस्करणम्;
        यदि (संस्करणम्_String.getAsInteger<पूर्णाङ्क>(10, संस्करणम्)) {
          error(I.get_मानम्(), "expected integer");
          प्रत्यागम असत्य;
        }
        यदि (संस्करणम् < 0) {
          error(I.get_मानम्(), "invalid version number");
          प्रत्यागम असत्य;
        }
        यदि (संस्करणम् != 0) {
          error(I.get_मानम्(), "version mismatch, expected 0");
          प्रत्यागम असत्य;
        }
      } अन्यथा यदि (कुञ्चिका == "case-sensitive") {
        यदि (!parseScalarBool(I.get_मानम्(), FS->CaseSensitive))
          प्रत्यागम असत्य;
      } अन्यथा यदि (कुञ्चिका == "overlay-relative") {
        यदि (!parseScalarBool(I.get_मानम्(), FS->IsRelativeOverlay))
          प्रत्यागम असत्य;
      } अन्यथा यदि (कुञ्चिका == "use-external-names") {
        यदि (!parseScalarBool(I.get_मानम्(), FS->Use_External_नामानि))
          प्रत्यागम असत्य;
      } अन्यथा यदि (कुञ्चिका == "fallthrough") {
        यदि (कुञ्चिकाः_2["redirecting-with"].आलोकितम्) {
          error(I.get_मानम्(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          प्रत्यागम असत्य;
        }

        तार्किक ShouldFallthrough = असत्य;
        यदि (!parseScalarBool(I.get_मानम्(), ShouldFallthrough))
          प्रत्यागम असत्य;

        यदि (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::Redirect_विधा::Fallthrough;
        } अन्यथा {
          FS->Redirection = RedirectingFileSystem::Redirect_विधा::RedirectOnly;
        }
      } अन्यथा यदि (कुञ्चिका == "redirecting-with") {
        यदि (कुञ्चिकाः_2["fallthrough"].आलोकितम्) {
          error(I.get_मानम्(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          प्रत्यागम असत्य;
        }

        यदि (स्वचालित विधा_2 = parse_Redirect_विधा(I.get_मानम्())) {
          FS->Redirection = *विधा_2;
        } अन्यथा {
          error(I.get_मानम्(), "expected valid redirect kind");
          प्रत्यागम असत्य;
        }
      } अन्यथा यदि (कुञ्चिका == "root-relative") {
        यदि (स्वचालित विधा_2 = parse_मूलम्_Relative_विधा(I.get_मानम्())) {
          FS->मूलम्_Relative = *विधा_2;
        } अन्यथा {
          error(I.get_मानम्(), "expected valid root-relative kind");
          प्रत्यागम असत्य;
        }
      } अन्यथा {
        llvm_unreachable("key missing from Keys");
      }
    }

    यदि (प्रवाहः.failed())
      प्रत्यागम असत्य;

    यदि (!check_Missing_कुञ्चिकाः(शिखरम्, कुञ्चिकाः_2))
      प्रत्यागम असत्य;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    प्रति (स्वचालित &E : मूलम्_Entries)
      unique_Overlay_वृक्षः(FS, E.get());

    प्रत्यागम सत्य;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<स्मृतिः_बफर> बफर,
                              SourceMgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_मार्गः, शून्य *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  SourceMgr SM;
  yaml::प्रवाहः प्रवाहः(बफर->get_Mem_बफर_Ref(), SM);

  SM.समुच्चयः_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = प्रवाहः.begin();
  yaml::ग्रन्थिः *मूलम्_2 = DI->get_मूलम्();
  यदि (DI == प्रवाहः.अन्तः() || !मूलम्_2) {
    SM.Print_सन्देशः(SMLoc(), SourceMgr::DK_Error, "expected root node");
    प्रत्यागम शून्यसूचकः;
  }

  RedirectingFileSystemParser P(प्रवाहः);

  std::unique_ptr<RedirectingFileSystem> FS(
      नूतन RedirectingFileSystem(ExternalFS));

  यदि (!YAMLFile_मार्गः.रिक्तानि()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::मार्गः_2::parent_मार्गः(YAMLFile_मार्गः);
    std::error_संहिता EC = FS->makeAbsolute(OverlayAbsDir);
    प्रतिज्ञा(!EC && "Overlay dir final path must be absolute");
    (शून्य)EC;
    FS->समुच्चयः_Overlay_File_Dir(OverlayAbsDir);
  }

  यदि (!P.parse(मूलम्_2, FS.get()))
    प्रत्यागम शून्यसूचकः;

  प्रत्यागम FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    तार्किक Use_External_नामानि, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      नूतन RedirectingFileSystem(ExternalFS));
  FS->Use_External_नामानि = Use_External_नामानि;

  StringMap<RedirectingFileSystem::प्रविष्टिः *> Entries;

  प्रति (स्वचालित &Mapping : llvm::reverse(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.first);
    SmallString<128> गन्तव्यम् = StringRef(Mapping.second);
    {
      स्वचालित EC = ExternalFS->makeAbsolute(From);
      (शून्य)EC;
      प्रतिज्ञा(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::प्रविष्टिः *&गन्तव्यम्_प्रविष्टिः = Entries[From];
    यदि (गन्तव्यम्_प्रविष्टिः)
      अनुवर्त;

    // Add parent directories.
    RedirectingFileSystem::प्रविष्टिः *Parent = शून्यसूचकः;
    StringRef FromDirectory = llvm::sys::मार्गः_2::parent_मार्गः(From);
    प्रति (स्वचालित I = llvm::sys::मार्गः_2::begin(FromDirectory),
              E = llvm::sys::मार्गः_2::अन्तः(FromDirectory);
         I != E; ++I) {
      Parent = RedirectingFileSystemParser::lookup_Or_Create_प्रविष्टिः(FS.get(), *I,
                                                                Parent);
    }
    प्रतिज्ञा(Parent && "File without a directory?");
    {
      स्वचालित EC = ExternalFS->makeAbsolute(गन्तव्यम्);
      (शून्य)EC;
      प्रतिज्ञा(!EC && "Could not make absolute path");
    }

    // Add the file.
    स्वचालित NewFile = std::make_unique<RedirectingFileSystem::File_प्रविष्टिः>(
        llvm::sys::मार्गः_2::filename(From), गन्तव्यम्,
        Use_External_नामानि ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    गन्तव्यम्_प्रविष्टिः = NewFile.get();
    cast<RedirectingFileSystem::Directory_प्रविष्टिः>(Parent)->सङ्कलय_Content(
        std::move(NewFile));
  }

  प्रत्यागम FS;
}

RedirectingFileSystem::Lookup_परिणाम::Lookup_परिणाम(
    प्रविष्टिः *E, sys::मार्गः_2::const_iterator Start, sys::मार्गः_2::const_iterator अन्तः_2)
    : E(E) {
  प्रतिज्ञा(E != शून्यसूचकः);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  यदि (स्वचालित *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_प्रविष्टिः>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_मार्गः());
    sys::मार्गः_2::append(Redirect, Start, अन्तः_2,
                      getExistingStyle(DRE->get_External_Contents_मार्गः()));
    ExternalRedirect = std::string(Redirect);
  }
}

शून्य RedirectingFileSystem::Lookup_परिणाम::get_मार्गः(
    llvm::Small_सदिशम्_Impl<वर्ण> &परिणाम) ध्रुव {
  परिणाम.clear();
  प्रति (प्रविष्टिः *Parent : Parents)
    llvm::sys::मार्गः_2::append(परिणाम, Parent->getName());
  llvm::sys::मार्गः_2::append(परिणाम, E->getName());
}

std::error_संहिता RedirectingFileSystem::makeCanonicalForLookup(
    Small_सदिशम्_Impl<वर्ण> &मार्गः) ध्रुव {
  यदि (std::error_संहिता EC = makeAbsolute(मार्गः))
    प्रत्यागम EC;

  llvm::SmallString<256> Canonical_मार्गः =
      canonicalize(StringRef(मार्गः.दत्त(), मार्गः.आकार_2()));
  यदि (Canonical_मार्गः.रिक्तानि())
    प्रत्यागम make_error_संहिता(llvm::errc::invalid_तर्कः);

  मार्गः.नियोजय(Canonical_मार्गः.begin(), Canonical_मार्गः.अन्तः());
  प्रत्यागम {};
}

ErrorOr<RedirectingFileSystem::Lookup_परिणाम>
RedirectingFileSystem::lookup_मार्गः(StringRef मार्गः) ध्रुव {
  llvm::SmallString<128> Canonical_मार्गः(मार्गः);
  यदि (std::error_संहिता EC = makeCanonicalForLookup(Canonical_मार्गः))
    प्रत्यागम EC;

  // RedirectOnly means the VFS is always used.
  यदि (Usage_Tracking_सक्रिय && Redirection == Redirect_विधा::RedirectOnly)
    Has_Been_प्रयुक्तम् = सत्य;

  sys::मार्गः_2::const_iterator Start = sys::मार्गः_2::begin(Canonical_मार्गः);
  sys::मार्गः_2::const_iterator अन्तः_2 = sys::मार्गः_2::अन्तः(Canonical_मार्गः);
  llvm::Small_सदिशम्<प्रविष्टिः *, 32> Entries;
  प्रति (ध्रुव स्वचालित &मूलम्_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम =
        lookup_मार्गः_Impl(Start, अन्तः_2, मूलम्_2.get(), Entries);
    यदि (Usage_Tracking_सक्रिय && परिणाम && isa<Remap_प्रविष्टिः>(परिणाम->E))
      Has_Been_प्रयुक्तम् = सत्य;
    यदि (परिणाम) {
      परिणाम->Parents = std::move(Entries);
      प्रत्यागम परिणाम;
    }

    यदि (परिणाम.getError() != llvm::errc::no_such_file_or_directory)
      प्रत्यागम परिणाम;
  }
  प्रत्यागम make_error_संहिता(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_परिणाम>
RedirectingFileSystem::lookup_मार्गः_Impl(
    sys::मार्गः_2::const_iterator Start, sys::मार्गः_2::const_iterator अन्तः_2,
    RedirectingFileSystem::प्रविष्टिः *From,
    llvm::Small_सदिशम्_Impl<प्रविष्टिः *> &Entries) ध्रुव {
  प्रतिज्ञा(!isTraversalComponent(*Start) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  यदि (!FromName.रिक्तानि()) {
    यदि (!मार्गः_Component_Matches(*Start, FromName))
      प्रत्यागम make_error_संहिता(llvm::errc::no_such_file_or_directory);

    ++Start;

    यदि (Start == अन्तः_2) {
      // Match!
      प्रत्यागम Lookup_परिणाम(From, Start, अन्तः_2);
    }
  }

  यदि (isa<RedirectingFileSystem::File_प्रविष्टिः>(From))
    प्रत्यागम make_error_संहिता(llvm::errc::not_a_directory);

  यदि (isa<RedirectingFileSystem::Directory_Remap_प्रविष्टिः>(From))
    प्रत्यागम Lookup_परिणाम(From, Start, अन्तः_2);

  स्वचालित *DE = cast<RedirectingFileSystem::Directory_प्रविष्टिः>(From);
  प्रति (ध्रुव std::unique_ptr<RedirectingFileSystem::प्रविष्टिः> &Dir_प्रविष्टिः :
       llvm::make_range(DE->contents_begin(), DE->contents_अन्तः())) {
    Entries.प्रक्षिप_पृष्ठभागः(From);
    ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम =
        lookup_मार्गः_Impl(Start, अन्तः_2, Dir_प्रविष्टिः.get(), Entries);
    यदि (परिणाम || परिणाम.getError() != llvm::errc::no_such_file_or_directory)
      प्रत्यागम परिणाम;
    Entries.निष्कासय_पृष्ठभागः();
  }

  प्रत्यागम make_error_संहिता(llvm::errc::no_such_file_or_directory);
}

स्थिर स्थितिः get_Redirected_File_स्थितिः(ध्रुव Twine &Original_मार्गः,
                                      तार्किक Use_External_नामानि,
                                      स्थितिः External_स्थितिः) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  यदि (External_स्थितिः.ExposesExternalVFSPath)
    प्रत्यागम External_स्थितिः;

  स्थितिः S = External_स्थितिः;
  यदि (!Use_External_नामानि)
    S = स्थितिः::copyWithNewName(S, Original_मार्गः);
  अन्यथा
    S.ExposesExternalVFSPath = सत्य;
  प्रत्यागम S;
}

ErrorOr<स्थितिः> RedirectingFileSystem::स्थितिः_2(
    ध्रुव Twine &Lookup_मार्गः, ध्रुव Twine &Original_मार्गः,
    ध्रुव RedirectingFileSystem::Lookup_परिणाम &परिणाम) {
  यदि (std::optional<StringRef> ExtRedirect = परिणाम.getExternalRedirect()) {
    SmallString<256> Remapped_मार्गः((*ExtRedirect).str());
    यदि (std::error_संहिता EC = makeAbsolute(Remapped_मार्गः))
      प्रत्यागम EC;

    ErrorOr<स्थितिः> S = ExternalFS->स्थितिः_2(Remapped_मार्गः);
    यदि (!S)
      प्रत्यागम S;
    S = स्थितिः::copyWithNewName(*S, *ExtRedirect);
    स्वचालित *RE = cast<RedirectingFileSystem::Remap_प्रविष्टिः>(परिणाम.E);
    प्रत्यागम get_Redirected_File_स्थितिः(Original_मार्गः,
                                   RE->useExternalName(Use_External_नामानि), *S);
  }

  स्वचालित *DE = cast<RedirectingFileSystem::Directory_प्रविष्टिः>(परिणाम.E);
  प्रत्यागम स्थितिः::copyWithNewName(DE->get_स्थितिः(), Lookup_मार्गः);
}

ErrorOr<स्थितिः>
RedirectingFileSystem::get_External_स्थितिः(ध्रुव Twine &Lookup_मार्गः,
                                         ध्रुव Twine &Original_मार्गः) ध्रुव {
  स्वचालित परिणाम = ExternalFS->स्थितिः_2(Lookup_मार्गः);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  यदि (!परिणाम || परिणाम->ExposesExternalVFSPath)
    प्रत्यागम परिणाम;
  प्रत्यागम स्थितिः::copyWithNewName(परिणाम.get(), Original_मार्गः);
}

ErrorOr<स्थितिः> RedirectingFileSystem::स्थितिः_2(ध्रुव Twine &Original_मार्गः) {
  SmallString<256> मार्गः;
  Original_मार्गः.गन्तव्यम्_सदिशम्(मार्गः);

  यदि (std::error_संहिता EC = makeAbsolute(मार्गः))
    प्रत्यागम EC;

  यदि (Redirection == Redirect_विधा::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<स्थितिः> S = get_External_स्थितिः(मार्गः, Original_मार्गः);
    यदि (S)
      प्रत्यागम S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookup_मार्गः(मार्गः);
  यदि (!परिणाम) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    यदि (Redirection == Redirect_विधा::Fallthrough &&
        is_File_Not_प्राप्तम्(परिणाम.getError()))
      प्रत्यागम get_External_स्थितिः(मार्गः, Original_मार्गः);
    प्रत्यागम परिणाम.getError();
  }

  ErrorOr<स्थितिः> S = स्थितिः_2(मार्गः, Original_मार्गः, *परिणाम);
  यदि (!S && Redirection == Redirect_विधा::Fallthrough &&
      is_File_Not_प्राप्तम्(S.getError(), परिणाम->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    प्रत्यागम get_External_स्थितिः(मार्गः, Original_मार्गः);
  }

  प्रत्यागम S;
}

तार्किक RedirectingFileSystem::exists(ध्रुव Twine &Original_मार्गः) {
  SmallString<256> मार्गः;
  Original_मार्गः.गन्तव्यम्_सदिशम्(मार्गः);

  यदि (makeAbsolute(मार्गः))
    प्रत्यागम असत्य;

  यदि (Redirection == Redirect_विधा::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    यदि (ExternalFS->exists(मार्गः))
      प्रत्यागम सत्य;
  }

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookup_मार्गः(मार्गः);
  यदि (!परिणाम) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    यदि (Redirection == Redirect_विधा::Fallthrough &&
        is_File_Not_प्राप्तम्(परिणाम.getError()))
      प्रत्यागम ExternalFS->exists(मार्गः);
    प्रत्यागम असत्य;
  }

  std::optional<StringRef> ExtRedirect = परिणाम->getExternalRedirect();
  यदि (!ExtRedirect) {
    प्रतिज्ञा(isa<RedirectingFileSystem::Directory_प्रविष्टिः>(परिणाम->E));
    प्रत्यागम सत्य;
  }

  SmallString<256> Remapped_मार्गः((*ExtRedirect).str());
  यदि (makeAbsolute(Remapped_मार्गः))
    प्रत्यागम असत्य;

  यदि (ExternalFS->exists(Remapped_मार्गः))
    प्रत्यागम सत्य;

  यदि (Redirection == Redirect_विधा::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    प्रत्यागम ExternalFS->exists(मार्गः);
  }

  प्रत्यागम असत्य;
}

नामाकाश {

/// Provide a file wrapper with an overriden status.
वर्ग File_With_Fixed_स्थितिः : सार्वजनिक File {
  std::unique_ptr<File> InnerFile;
  स्थितिः S;

सार्वजनिक:
  File_With_Fixed_स्थितिः(std::unique_ptr<File> InnerFile, स्थितिः S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<स्थितिः> स्थितिः_2() override { प्रत्यागम S; }
  ErrorOr<std::unique_ptr<llvm::स्मृतिः_बफर>>

  get_बफर(ध्रुव Twine &Name, int64_t File_आकार, तार्किक RequiresNullTerminator,
            तार्किक IsVolatile) override {
    प्रत्यागम InnerFile->get_बफर(Name, File_आकार, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_संहिता close() override { प्रत्यागम InnerFile->close(); }

  शून्य समुच्चयः_मार्गः(ध्रुव Twine &मार्गः) override { S = S.copyWithNewName(S, मार्गः); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_मार्गः(ErrorOr<std::unique_ptr<File>> परिणाम, ध्रुव Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  यदि (!परिणाम || (*परिणाम)->स्थितिः_2()->ExposesExternalVFSPath)
    प्रत्यागम परिणाम;

  ErrorOr<std::unique_ptr<File>> F = std::move(*परिणाम);
  स्वचालित Name = F->get()->getName();
  यदि (Name && Name.get() != P.str())
    F->get()->समुच्चयः_मार्गः(P);
  प्रत्यागम F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(ध्रुव Twine &Original_मार्गः) {
  SmallString<256> मार्गः;
  Original_मार्गः.गन्तव्यम्_सदिशम्(मार्गः);

  यदि (std::error_संहिता EC = makeAbsolute(मार्गः))
    प्रत्यागम EC;

  यदि (Redirection == Redirect_विधा::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    स्वचालित F = File::get_With_मार्गः(ExternalFS->openFileForRead(मार्गः), Original_मार्गः);
    यदि (F)
      प्रत्यागम F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookup_मार्गः(मार्गः);
  यदि (!परिणाम) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    यदि (Redirection == Redirect_विधा::Fallthrough &&
        is_File_Not_प्राप्तम्(परिणाम.getError()))
      प्रत्यागम File::get_With_मार्गः(ExternalFS->openFileForRead(मार्गः), Original_मार्गः);
    प्रत्यागम परिणाम.getError();
  }

  यदि (!परिणाम->getExternalRedirect()) // FIXME: errc::not_a_file?
    प्रत्यागम make_error_संहिता(llvm::errc::invalid_तर्कः);

  StringRef ExtRedirect = *परिणाम->getExternalRedirect();
  SmallString<256> Remapped_मार्गः(ExtRedirect.str());
  यदि (std::error_संहिता EC = makeAbsolute(Remapped_मार्गः))
    प्रत्यागम EC;

  स्वचालित *RE = cast<RedirectingFileSystem::Remap_प्रविष्टिः>(परिणाम->E);

  स्वचालित ExternalFile =
      File::get_With_मार्गः(ExternalFS->openFileForRead(Remapped_मार्गः), ExtRedirect);
  यदि (!ExternalFile) {
    यदि (Redirection == Redirect_विधा::Fallthrough &&
        is_File_Not_प्राप्तम्(ExternalFile.getError(), परिणाम->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      प्रत्यागम File::get_With_मार्गः(ExternalFS->openFileForRead(मार्गः), Original_मार्गः);
    }
    प्रत्यागम ExternalFile;
  }

  स्वचालित External_स्थितिः = (*ExternalFile)->स्थितिः_2();
  यदि (!External_स्थितिः)
    प्रत्यागम External_स्थितिः.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  स्थितिः S = get_Redirected_File_स्थितिः(
      Original_मार्गः, RE->useExternalName(Use_External_नामानि), *External_स्थितिः);
  प्रत्यागम std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_स्थितिः>(std::move(*ExternalFile), S));
}

std::error_संहिता
RedirectingFileSystem::get_Real_मार्गः(ध्रुव Twine &Original_मार्गः,
                                   Small_सदिशम्_Impl<वर्ण> &Output) {
  SmallString<256> मार्गः;
  Original_मार्गः.गन्तव्यम्_सदिशम्(मार्गः);

  यदि (std::error_संहिता EC = makeAbsolute(मार्गः))
    प्रत्यागम EC;

  यदि (Redirection == Redirect_विधा::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_संहिता EC = ExternalFS->get_Real_मार्गः(मार्गः, Output);
    यदि (!EC)
      प्रत्यागम EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookup_मार्गः(मार्गः);
  यदि (!परिणाम) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    यदि (Redirection == Redirect_विधा::Fallthrough &&
        is_File_Not_प्राप्तम्(परिणाम.getError()))
      प्रत्यागम ExternalFS->get_Real_मार्गः(मार्गः, Output);
    प्रत्यागम परिणाम.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  यदि (स्वचालित ExtRedirect = परिणाम->getExternalRedirect()) {
    स्वचालित P = ExternalFS->get_Real_मार्गः(*ExtRedirect, Output);
    यदि (P && Redirection == Redirect_विधा::Fallthrough &&
        is_File_Not_प्राप्तम्(P, परिणाम->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      प्रत्यागम ExternalFS->get_Real_मार्गः(मार्गः, Output);
    }
    प्रत्यागम P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  यदि (Redirection == Redirect_विधा::Fallthrough) {
    परिणाम->get_मार्गः(Output);
    प्रत्यागम {};
  }
  प्रत्यागम llvm::errc::invalid_तर्कः;
}

शून्य RedirectingFileSystem::getDirectoryContentRealSources(
    ध्रुव Twine &Dir, Small_सदिशम्_Impl<std::string> &बहिर्गमनम्) {
  SmallString<256> मार्गः;
  Dir.गन्तव्यम्_सदिशम्(मार्गः);

  यदि (makeAbsolute(मार्गः))
    प्रत्यागम;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  ध्रुव तार्किक ConsultsExternalFS = Redirection != Redirect_विधा::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> परिणाम = lookup_मार्गः(मार्गः);
  यदि (!परिणाम) {
    // dir_begin() delegates entirely to ExternalFS.
    यदि (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(मार्गः, बहिर्गमनम्);
    प्रत्यागम;
  }

  विकल्प (परिणाम->E->get_विधा()) {
  पक्ष EK_File:
    प्रत्यागम;
  पक्ष EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    भङ्ग;
  पक्ष EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_मार्गः(*परिणाम->getExternalRedirect());
    यदि (!makeAbsolute(Remapped_मार्गः))
      ExternalFS->getDirectoryContentRealSources(Remapped_मार्गः, बहिर्गमनम्);
    भङ्ग;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  यदि (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(मार्गः, बहिर्गमनम्);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<स्मृतिः_बफर> बफर,
                    SourceMgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_मार्गः, शून्य *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  प्रत्यागम RedirectingFileSystem::create(std::move(बफर), DiagHandler,
                                       YAMLFile_मार्गः, DiagContext,
                                       std::move(ExternalFS));
}

स्थिर शून्य getVFSEntries(RedirectingFileSystem::प्रविष्टिः *SrcE,
                          Small_सदिशम्_Impl<StringRef> &मार्गः,
                          Small_सदिशम्_Impl<YAMLVFSEntry> &Entries) {
  स्वचालित विधा_2 = SrcE->get_विधा();
  यदि (विधा_2 == RedirectingFileSystem::EK_Directory) {
    स्वचालित *DE = dyn_cast<RedirectingFileSystem::Directory_प्रविष्टिः>(SrcE);
    प्रतिज्ञा(DE && "Must be a directory");
    प्रति (std::unique_ptr<RedirectingFileSystem::प्रविष्टिः> &Sub_प्रविष्टिः :
         llvm::make_range(DE->contents_begin(), DE->contents_अन्तः())) {
      मार्गः.प्रक्षिप_पृष्ठभागः(Sub_प्रविष्टिः->getName());
      getVFSEntries(Sub_प्रविष्टिः.get(), मार्गः, Entries);
      मार्गः.निष्कासय_पृष्ठभागः();
    }
    प्रत्यागम;
  }

  यदि (विधा_2 == RedirectingFileSystem::EK_DirectoryRemap) {
    स्वचालित *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_प्रविष्टिः>(SrcE);
    प्रतिज्ञा(DR && "Must be a directory remap");
    SmallString<128> VPath;
    प्रति (स्वचालित &Comp : मार्गः)
      llvm::sys::मार्गः_2::append(VPath, Comp);
    Entries.प्रक्षिप_पृष्ठभागः(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_मार्गः()));
    प्रत्यागम;
  }

  प्रतिज्ञा(विधा_2 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  स्वचालित *FE = dyn_cast<RedirectingFileSystem::File_प्रविष्टिः>(SrcE);
  प्रतिज्ञा(FE && "Must be a file");
  SmallString<128> VPath;
  प्रति (स्वचालित &Comp : मार्गः)
    llvm::sys::मार्गः_2::append(VPath, Comp);
  Entries.प्रक्षिप_पृष्ठभागः(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_मार्गः()));
}

शून्य vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_सदिशम्_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_परिणाम> मूलम्_परिणाम = VFS.lookup_मार्गः("/");
  यदि (!मूलम्_परिणाम)
    प्रत्यागम;
  Small_सदिशम्<StringRef, 8> Components;
  Components.प्रक्षिप_पृष्ठभागः("/");
  getVFSEntries(मूलम्_परिणाम->E, Components, CollectedEntries);
}

UniqueID vfs::get_अग्र_Virtual_Unique_ID() {
  स्थिर std::atomic<अचिह्नित> UID;
  अचिह्नित ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  प्रत्यागम UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

शून्य YAMLVFSWriter::सङ्कलय_प्रविष्टिः(StringRef Virtual_मार्गः, StringRef Real_मार्गः,
                             तार्किक IsDirectory) {
  प्रतिज्ञा(sys::मार्गः_2::is_absolute(Virtual_मार्गः) && "virtual path not absolute");
  प्रतिज्ञा(sys::मार्गः_2::is_absolute(Real_मार्गः) && "real path not absolute");
  प्रतिज्ञा(!मार्गः_Has_Traversal(Virtual_मार्गः) && "path traversal is not supported");
  Mappings.emplace_पृष्ठभागः(Virtual_मार्गः, Real_मार्गः, IsDirectory);
}

शून्य YAMLVFSWriter::सङ्कलय_File_Mapping(StringRef Virtual_मार्गः, StringRef Real_मार्गः) {
  सङ्कलय_प्रविष्टिः(Virtual_मार्गः, Real_मार्गः, /*IsDirectory=*/असत्य);
}

शून्य YAMLVFSWriter::सङ्कलय_Directory_Mapping(StringRef Virtual_मार्गः,
                                        StringRef Real_मार्गः) {
  सङ्कलय_प्रविष्टिः(Virtual_मार्गः, Real_मार्गः, /*IsDirectory=*/सत्य);
}

नामाकाश {

वर्ग JSONWriter {
  llvm::raw_ostream &OS;
  Small_सदिशम्<StringRef, 16> Dir_राशिः;

  अचिह्नित getDirIndent() { प्रत्यागम 4 * Dir_राशिः.आकार_2(); }
  अचिह्नित getFileIndent() { प्रत्यागम 4 * (Dir_राशिः.आकार_2() + 1); }
  तार्किक contained_अन्तर्(StringRef Parent, StringRef मार्गः);
  StringRef containedPart(StringRef Parent, StringRef मार्गः);
  शून्य startDirectory(StringRef मार्गः);
  शून्य अन्तः_Directory();
  शून्य write_प्रविष्टिः(StringRef VPath, StringRef RPath);

सार्वजनिक:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  शून्य लिख(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<तार्किक> Use_External_नामानि,
             std::optional<तार्किक> IsCaseSensitive,
             std::optional<तार्किक> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

तार्किक JSONWriter::contained_अन्तर्(StringRef Parent, StringRef मार्गः) {
  उपयुज्य नामाकाश llvm::sys;

  // Compare each path component.
  स्वचालित IParent = मार्गः_2::begin(Parent), EParent = मार्गः_2::अन्तः(Parent);
  प्रति (स्वचालित IChild = मार्गः_2::begin(मार्गः), EChild = मार्गः_2::अन्तः(मार्गः);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    यदि (*IParent != *IChild)
      प्रत्यागम असत्य;
  }
  // Have we exhausted the parent path?
  प्रत्यागम IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef Parent, StringRef मार्गः) {
  प्रतिज्ञा(!Parent.रिक्तानि());
  प्रतिज्ञा(contained_अन्तर्(Parent, मार्गः));
  प्रत्यागम मार्गः.substr(Parent.आकार_2() + 1);
}

शून्य JSONWriter::startDirectory(StringRef मार्गः) {
  StringRef Name =
      Dir_राशिः.रिक्तानि() ? मार्गः : containedPart(Dir_राशिः.पृष्ठभागः(), मार्गः);
  Dir_राशिः.प्रक्षिप_पृष्ठभागः(मार्गः);
  अचिह्नित Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

शून्य JSONWriter::अन्तः_Directory() {
  अचिह्नित Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_राशिः.निष्कासय_पृष्ठभागः();
}

शून्य JSONWriter::write_प्रविष्टिः(StringRef VPath, StringRef RPath) {
  अचिह्नित Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

शून्य JSONWriter::लिख(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<तार्किक> Use_External_नामानि,
                       std::optional<तार्किक> IsCaseSensitive,
                       std::optional<तार्किक> IsOverlayRelative,
                       StringRef OverlayDir) {
  उपयुज्य नामाकाश llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  यदि (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  यदि (Use_External_नामानि)
    OS << "  'use-external-names': '" << (*Use_External_नामानि ? "true" : "false")
       << "',\n";
  तार्किक UseOverlayRelative = असत्य;
  यदि (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  यदि (!Entries.रिक्तानि()) {
    ध्रुव YAMLVFSEntry &प्रविष्टिः = Entries.front();

    startDirectory(
      प्रविष्टिः.IsDirectory ? प्रविष्टिः.VPath : मार्गः_2::parent_मार्गः(प्रविष्टिः.VPath)
    );

    StringRef RPath = प्रविष्टिः.RPath;
    यदि (UseOverlayRelative) {
      प्रतिज्ञा(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.आकार_2());
    }

    तार्किक Is_Current_Dir_रिक्तानि = सत्य;
    यदि (!प्रविष्टिः.IsDirectory) {
      write_प्रविष्टिः(मार्गः_2::filename(प्रविष्टिः.VPath), RPath);
      Is_Current_Dir_रिक्तानि = असत्य;
    }

    प्रति (ध्रुव स्वचालित &प्रविष्टिः : Entries.खण्डिका(1)) {
      StringRef Dir =
          प्रविष्टिः.IsDirectory ? प्रविष्टिः.VPath : मार्गः_2::parent_मार्गः(प्रविष्टिः.VPath);
      यदि (Dir == Dir_राशिः.पृष्ठभागः()) {
        यदि (!Is_Current_Dir_रिक्तानि) {
          OS << ",\n";
        }
      } अन्यथा {
        तार्किक Is_Dir_Popped_From_राशिः = असत्य;
        यावत् (!Dir_राशिः.रिक्तानि() && !contained_अन्तर्(Dir_राशिः.पृष्ठभागः(), Dir)) {
          OS << "\n";
          अन्तः_Directory();
          Is_Dir_Popped_From_राशिः = सत्य;
        }
        यदि (Is_Dir_Popped_From_राशिः || !Is_Current_Dir_रिक्तानि) {
          OS << ",\n";
        }
        startDirectory(Dir);
        Is_Current_Dir_रिक्तानि = सत्य;
      }
      StringRef RPath = प्रविष्टिः.RPath;
      यदि (UseOverlayRelative) {
        प्रतिज्ञा(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.आकार_2());
      }
      यदि (!प्रविष्टिः.IsDirectory) {
        write_प्रविष्टिः(मार्गः_2::filename(प्रविष्टिः.VPath), RPath);
        Is_Current_Dir_रिक्तानि = असत्य;
      }
    }

    यावत् (!Dir_राशिः.रिक्तानि()) {
      OS << "\n";
      अन्तः_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

शून्य YAMLVFSWriter::लिख(llvm::raw_ostream &OS) {
  llvm::क्रमय(Mappings, [](ध्रुव YAMLVFSEntry &LHS, ध्रुव YAMLVFSEntry &RHS) {
    प्रत्यागम LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).लिख(Mappings, Use_External_नामानि, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, ध्रुव Twine &मार्गः, std::error_संहिता &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(मार्गः, EC);
  यदि (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->राशिः.प्रक्षिप_पृष्ठभागः(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_संहिता &EC) {
  प्रतिज्ञा(FS && State && !State->राशिः.रिक्तानि() && "incrementing past end");
  प्रतिज्ञा(!State->राशिः.पृष्ठभागः()->मार्गः_2().रिक्तानि() && "non-canonical end iterator");
  vfs::directory_iterator अन्तः_2;

  यदि (State->Has_No_प्रक्षिप_Request)
    State->Has_No_प्रक्षिप_Request = असत्य;
  अन्यथा {
    यदि (State->राशिः.पृष्ठभागः()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->राशिः.पृष्ठभागः()->मार्गः_2(), EC);
      यदि (I != अन्तः_2) {
        State->राशिः.प्रक्षिप_पृष्ठभागः(I);
        प्रत्यागम *एतत्;
      }
    }
  }

  यावत् (!State->राशिः.रिक्तानि() && State->राशिः.पृष्ठभागः().increment(EC) == अन्तः_2)
    State->राशिः.निष्कासय_पृष्ठभागः();

  यदि (State->राशिः.रिक्तानि())
    State.reset(); // end iterator

  प्रत्यागम *एतत्;
}

ध्रुव वर्ण FileSystem::ID = 0;
ध्रुव वर्ण OverlayFileSystem::ID = 0;
ध्रुव वर्ण ProxyFileSystem::ID = 0;
ध्रुव वर्ण अन्तर्_स्मृतिः_File_System::ID = 0;
ध्रुव वर्ण RedirectingFileSystem::ID = 0;

अचिह्नित ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_गणनम्(
    ध्रुव FileSystem *FS) {
  प्रत्यागम FS->Use_गणनम्();
}

शून्य ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

शून्य ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
