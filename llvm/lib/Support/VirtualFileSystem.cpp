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

#అన్తర్భావ "llvm/Support/VirtualFileSystem.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/DenseMap.h"
#అన్తర్భావ "llvm/ADT/IntrusiveRefCntPtr.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/StringSet.h"
#అన్తర్భావ "llvm/ADT/Twine.h"
#అన్తర్భావ "llvm/ADT/iterator_range.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/Casting.h"
#అన్తర్భావ "llvm/Support/Chrono.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/ErrorOr.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/FileSystem/UniqueID.h"
#అన్తర్భావ "llvm/Support/IOSandbox.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/SMLoc.h"
#అన్తర్భావ "llvm/Support/SourceMgr.h"
#అన్తర్భావ "llvm/Support/YAMLParser.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <atomic>
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstdint>
#అన్తర్భావ <iterator>
#అన్తర్భావ <limits>
#అన్తర్భావ <map>
#అన్తర్భావ <memory>
#అన్తర్భావ <optional>
#అన్తర్భావ <string>
#అన్తర్భావ <system_error>
#అన్తర్భావ <utility>
#అన్తర్భావ <vector>

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::vfs;

ఉపయోగించి llvm::sys::fs::file_స్థితి;
ఉపయోగించి llvm::sys::fs::file_t;
ఉపయోగించి llvm::sys::fs::file_type;
ఉపయోగించి llvm::sys::fs::perms;
ఉపయోగించి llvm::sys::fs::UniqueID;

స్థితి_2::స్థితి_2(స్థిరరాశి file_స్థితి &స్థితి_2)
    : UID(స్థితి_2.getUniqueID()), MTime(స్థితి_2.get_చివరి_Modification_సమయం()),
      User(స్థితి_2.getUser()), సమూహం(స్థితి_2.get_సమూహం()), కొలత(స్థితి_2.get_కొలత()),
      Type(స్థితి_2.type()), Perms(స్థితి_2.permissions()) {}

స్థితి_2::స్థితి_2(స్థిరరాశి Twine &Name, UniqueID UID, sys::సమయం_బిందువు<> MTime,
               uint32_t User, uint32_t సమూహం, uint64_t కొలత, file_type Type,
               perms Perms)
    : Name(Name.str()), UID(UID), MTime(MTime), User(User), సమూహం(సమూహం),
      కొలత(కొలత), Type(Type), Perms(Perms) {}

స్థితి_2 స్థితి_2::copy_With_New_కొలత(స్థిరరాశి స్థితి_2 &లోపల_2, uint64_t New_కొలత) {
  తిరిగి స్థితి_2(లోపల_2.getName(), లోపల_2.getUniqueID(), లోపల_2.get_చివరి_Modification_సమయం(),
                లోపల_2.getUser(), లోపల_2.get_సమూహం(), New_కొలత, లోపల_2.getType(),
                లోపల_2.getPermissions());
}

స్థితి_2 స్థితి_2::copyWithNewName(స్థిరరాశి స్థితి_2 &లోపల_2, స్థిరరాశి Twine &NewName) {
  తిరిగి స్థితి_2(NewName, లోపల_2.getUniqueID(), లోపల_2.get_చివరి_Modification_సమయం(),
                లోపల_2.getUser(), లోపల_2.get_సమూహం(), లోపల_2.get_కొలత(), లోపల_2.getType(),
                లోపల_2.getPermissions());
}

స్థితి_2 స్థితి_2::copyWithNewName(స్థిరరాశి file_స్థితి &లోపల_2, స్థిరరాశి Twine &NewName) {
  తిరిగి స్థితి_2(NewName, లోపల_2.getUniqueID(), లోపల_2.get_చివరి_Modification_సమయం(),
                లోపల_2.getUser(), లోపల_2.get_సమూహం(), లోపల_2.get_కొలత(), లోపల_2.type(),
                లోపల_2.permissions());
}

తార్కిక స్థితి_2::equivalent(స్థిరరాశి స్థితి_2 &Other) స్థిరరాశి {
  ప్రతిజ్ఞా(is_స్థితి_తెలిసిన() && Other.is_స్థితి_తెలిసిన());
  తిరిగి getUniqueID() == Other.getUniqueID();
}

తార్కిక స్థితి_2::isDirectory() స్థిరరాశి { తిరిగి Type == file_type::directory_file; }

తార్కిక స్థితి_2::isRegularFile() స్థిరరాశి { తిరిగి Type == file_type::regular_file; }

తార్కిక స్థితి_2::isOther() స్థిరరాశి {
  తిరిగి exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

తార్కిక స్థితి_2::isSymlink() స్థిరరాశి { తిరిగి Type == file_type::symlink_file; }

తార్కిక స్థితి_2::is_స్థితి_తెలిసిన() స్థిరరాశి { తిరిగి Type != file_type::స్థితి_error; }

తార్కిక స్థితి_2::exists() స్థిరరాశి {
  తిరిగి is_స్థితి_తెలిసిన() && Type != file_type::file_not_దొరికింది;
}

File::~File() = అప్రమేయ;

FileSystem::~FileSystem() = అప్రమేయ;

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>>
FileSystem::get_బఫరు_For_File(స్థిరరాశి llvm::Twine &Name, int64_t File_కొలత,
                             తార్కిక RequiresNullTerminator, తార్కిక IsVolatile,
                             తార్కిక Is_పాఠ్యం) {
  స్వయంచాలక F = Is_పాఠ్యం ? openFileForRead(Name) : open_File_For_Read_ద్విమాన(Name);
  ఒకవేళ (!F)
    తిరిగి F.getError();

  తిరిగి (*F)->get_బఫరు(Name, File_కొలత, RequiresNullTerminator, IsVolatile);
}

std::error_సంకేతం FileSystem::makeAbsolute(Small_సదిశ_Impl<అక్షరం> &మార్గం) స్థిరరాశి {
  ఒకవేళ (llvm::sys::మార్గం_2::is_absolute(మార్గం))
    తిరిగి {};

  స్వయంచాలక WorkingDir = getCurrentWorkingDirectory();
  ఒకవేళ (!WorkingDir)
    తిరిగి WorkingDir.getError();

  sys::మార్గం_2::make_absolute(WorkingDir.get(), మార్గం);
  తిరిగి {};
}

std::error_సంకేతం FileSystem::get_Real_మార్గం(స్థిరరాశి Twine &మార్గం,
                                        Small_సదిశ_Impl<అక్షరం> &Output) {
  తిరిగి errc::operation_not_permitted;
}

std::error_సంకేతం FileSystem::isLocal(స్థిరరాశి Twine &మార్గం, తార్కిక &ఫలితం) {
  తిరిగి errc::operation_not_permitted;
}

తార్కిక FileSystem::exists(స్థిరరాశి Twine &మార్గం) {
  స్వయంచాలక స్థితి_2 = స్థితి_3(మార్గం);
  తిరిగి స్థితి_2 && స్థితి_2->exists();
}

llvm::ErrorOr<తార్కిక> FileSystem::equivalent(స్థిరరాశి Twine &A, స్థిరరాశి Twine &B) {
  స్వయంచాలక స్థితి_A = స్థితి_3(A);
  ఒకవేళ (!స్థితి_A)
    తిరిగి స్థితి_A.getError();
  స్వయంచాలక స్థితి_B = స్థితి_3(B);
  ఒకవేళ (!స్థితి_B)
    తిరిగి స్థితి_B.getError();
  తిరిగి స్థితి_A->equivalent(*స్థితి_B);
}

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
శూన్య FileSystem::dump() స్థిరరాశి { print(dbgs(), PrintType::RecursiveContents); }
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే NDEBUG
స్థిర తార్కిక isTraversalComponent(StringRef Component) {
  తిరిగి Component == ".." || Component == ".";
}

స్థిర తార్కిక మార్గం_Has_Traversal(StringRef మార్గం) {
  ఉపయోగించి నామాకాశ llvm::sys;

  కొరకు (StringRef Comp : llvm::make_range(మార్గం_2::begin(మార్గం), మార్గం_2::చివర(మార్గం)))
    ఒకవేళ (isTraversalComponent(Comp))
      తిరిగి నిజం;
  తిరిగి అబద్ధం;
}
#అయితే_ముగింపు

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

నామాకాశ {

/// Wrapper around a raw file descriptor.
తరగతి RealFile : బహిరంగ File {
  మిత్ర తరగతి RealFileSystem;

  file_t FD;
  స్థితి_2 S;
  std::string RealName;

  RealFile(file_t RawFD, StringRef NewName, StringRef New_Real_మార్గం_Name)
      : FD(RawFD), S(NewName, {}, {}, {}, {}, {},
                     llvm::sys::fs::file_type::స్థితి_error, {}),
        RealName(New_Real_మార్గం_Name.str()) {
    ప్రతిజ్ఞా(FD.isValid() && "Invalid or inactive file descriptor");
  }

బహిరంగ:
  ~RealFile() override;

  ErrorOr<స్థితి_2> స్థితి_3() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> get_బఫరు(స్థిరరాశి Twine &Name,
                                                   int64_t File_కొలత,
                                                   తార్కిక RequiresNullTerminator,
                                                   తార్కిక IsVolatile) override;
  std::error_సంకేతం close() override;
  శూన్య సమితి_మార్గం(స్థిరరాశి Twine &మార్గం) override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<స్థితి_2> RealFile::స్థితి_3() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ప్రతిజ్ఞా(FD.isValid() && "cannot stat closed file");
  ఒకవేళ (!S.is_స్థితి_తెలిసిన()) {
    file_స్థితి Real_స్థితి;
    ఒకవేళ (std::error_సంకేతం EC = sys::fs::స్థితి_3(FD, Real_స్థితి))
      తిరిగి EC;
    S = స్థితి_2::copyWithNewName(Real_స్థితి, S.getName());
  }
  తిరిగి S;
}

ErrorOr<std::string> RealFile::getName() {
  తిరిగి RealName.ఖాళీలు() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>>
RealFile::get_బఫరు(స్థిరరాశి Twine &Name, int64_t File_కొలత,
                    తార్కిక RequiresNullTerminator, తార్కిక IsVolatile) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ప్రతిజ్ఞా(FD.isValid() && "cannot get buffer for closed file");
  తిరిగి జ్ఞాపకం_బఫరు::getOpenFile(FD, Name, File_కొలత, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_సంకేతం RealFile::close() {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  std::error_సంకేతం EC = sys::fs::closeFile(FD);
  FD = file_t::Invalid;
  తిరిగి EC;
}

శూన్య RealFile::సమితి_మార్గం(స్థిరరాశి Twine &మార్గం) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  RealName = మార్గం.str();
  ఒకవేళ (స్వయంచాలక స్థితి_2 = స్థితి_3())
    S = స్థితి_2.get().copyWithNewName(స్థితి_2.get(), మార్గం);
}

నామాకాశ {

/// A file system according to your operating system.
/// This may be linked to the process's working directory, or maintain its own.
///
/// Currently, its own working directory is emulated by storing the path and
/// sending absolute paths to llvm::sys::fs:: functions.
/// A more principled approach would be to push this down a level, modelling
/// the working dir as an llvm::sys::fs::WorkingDir or similar.
/// This would enable the use of openat()-style functions on some platforms.
తరగతి RealFileSystem : బహిరంగ FileSystem {
బహిరంగ:
  స్పష్టమైన RealFileSystem(తార్కిక LinkCWDToProcess) {
    ఒకవేళ (!LinkCWDToProcess) {
      SmallString<128> PWD, RealPWD;
      ఒకవేళ (std::error_సంకేతం EC = llvm::sys::fs::current_మార్గం(PWD))
        WD = std::move(EC);
      లేకపోతే ఒకవేళ (llvm::sys::fs::real_మార్గం(PWD, RealPWD))
        WD = WorkingDirectory{PWD, PWD};
      లేకపోతే
        WD = WorkingDirectory{PWD, RealPWD};
    }
  }

  ErrorOr<స్థితి_2> స్థితి_3(స్థిరరాశి Twine &మార్గం) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(స్థిరరాశి Twine &మార్గం) override;
  ErrorOr<std::unique_ptr<File>>
  open_File_For_Read_ద్విమాన(స్థిరరాశి Twine &మార్గం) override;
  directory_iterator dir_begin(స్థిరరాశి Twine &Dir, std::error_సంకేతం &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() స్థిరరాశి override;
  std::error_సంకేతం సమితి_Current_Working_Directory(స్థిరరాశి Twine &మార్గం) override;
  std::error_సంకేతం isLocal(స్థిరరాశి Twine &మార్గం, తార్కిక &ఫలితం) override;
  శూన్య
  getDirectoryContentRealSources(స్థిరరాశి Twine &Dir,
                                 Small_సదిశ_Impl<std::string> &బహిర్గతం) override {
    బహిర్గతం.నెట్టు_వెనుక(Dir.str());
  }
  std::error_సంకేతం get_Real_మార్గం(స్థిరరాశి Twine &మార్గం,
                              Small_సదిశ_Impl<అక్షరం> &Output) override;

రక్షిత:
  శూన్య printImpl(raw_ostream &OS, PrintType Type,
                 గుర్తులేని IndentLevel) స్థిరరాశి override;

అంతరంగ:
  // If this FS has its own working dir, use it to make Path absolute.
  // The returned twine is safe to use as long as both Storage and Path live.
  Twine adjust_మార్గం(స్థిరరాశి Twine &మార్గం, Small_సదిశ_Impl<అక్షరం> &Storage) స్థిరరాశి {
    ఒకవేళ (!WD || !*WD)
      తిరిగి మార్గం;
    మార్గం.గమ్యం_సదిశ(Storage);
    sys::మార్గం_2::make_absolute(WD->get().Resolved, Storage);
    తిరిగి Storage;
  }

  ErrorOr<std::unique_ptr<File>>
  openFileForReadWithFlags(స్థిరరాశి Twine &Name, sys::fs::OpenFlags Flags) {
    SmallString<256> RealName, Storage;
    Expected<file_t> FDOrErr = sys::fs::openNativeFileForRead(
        adjust_మార్గం(Name, Storage), Flags, &RealName);
    ఒకవేళ (!FDOrErr)
      తిరిగి error_గమ్యం_Error_సంకేతం(FDOrErr.takeError());
    తిరిగి std::unique_ptr<File>(
        కొత్త RealFile(*FDOrErr, Name.str(), RealName.str()));
  }

  నిర్మాణం WorkingDirectory {
    // The current working directory, without symlinks resolved. (echo $PWD).
    SmallString<128> Specified;
    // The current working directory, with links resolved. (readlink .).
    SmallString<128> Resolved;
  };
  std::optional<llvm::ErrorOr<WorkingDirectory>> WD;
};

} // namespace

ErrorOr<స్థితి_2> RealFileSystem::స్థితి_3(స్థిరరాశి Twine &మార్గం) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  sys::fs::file_స్థితి Real_స్థితి;
  ఒకవేళ (std::error_సంకేతం EC =
          sys::fs::స్థితి_3(adjust_మార్గం(మార్గం, Storage), Real_స్థితి))
    తిరిగి EC;
  తిరిగి స్థితి_2::copyWithNewName(Real_స్థితి, మార్గం);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(స్థిరరాశి Twine &Name) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  తిరిగి openFileForReadWithFlags(Name, sys::fs::యొక్క_పాఠ్యం);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::open_File_For_Read_ద్విమాన(స్థిరరాశి Twine &Name) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  తిరిగి openFileForReadWithFlags(Name, sys::fs::యొక్క_None);
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() స్థిరరాశి {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ఒకవేళ (WD && *WD)
    తిరిగి std::string(WD->get().Specified);
  ఒకవేళ (WD)
    తిరిగి WD->getError();

  SmallString<128> Dir;
  ఒకవేళ (std::error_సంకేతం EC = llvm::sys::fs::current_మార్గం(Dir))
    తిరిగి EC;
  తిరిగి std::string(Dir);
}

std::error_సంకేతం RealFileSystem::సమితి_Current_Working_Directory(స్థిరరాశి Twine &మార్గం) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ఒకవేళ (!WD)
    తిరిగి llvm::sys::fs::సమితి_current_మార్గం(మార్గం);

  SmallString<128> Absolute, Resolved, Storage;
  adjust_మార్గం(మార్గం, Storage).గమ్యం_సదిశ(Absolute);
  తార్కిక IsDir;
  ఒకవేళ (స్వయంచాలక Err = llvm::sys::fs::is_directory(Absolute, IsDir))
    తిరిగి Err;
  ఒకవేళ (!IsDir)
    తిరిగి std::make_error_సంకేతం(std::errc::not_a_directory);
  ఒకవేళ (స్వయంచాలక Err = llvm::sys::fs::real_మార్గం(Absolute, Resolved))
    తిరిగి Err;
  WD = WorkingDirectory{Absolute, Resolved};
  తిరిగి std::error_సంకేతం();
}

std::error_సంకేతం RealFileSystem::isLocal(స్థిరరాశి Twine &మార్గం, తార్కిక &ఫలితం) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  తిరిగి llvm::sys::fs::is_local(adjust_మార్గం(మార్గం, Storage), ఫలితం);
}

std::error_సంకేతం RealFileSystem::get_Real_మార్గం(స్థిరరాశి Twine &మార్గం,
                                            Small_సదిశ_Impl<అక్షరం> &Output) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<256> Storage;
  తిరిగి llvm::sys::fs::real_మార్గం(adjust_మార్గం(మార్గం, Storage), Output);
}

శూన్య RealFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                               గుర్తులేని IndentLevel) స్థిరరాశి {
  printIndent(OS, IndentLevel);
  OS << "RealFileSystem using ";
  ఒకవేళ (WD)
    OS << "own";
  లేకపోతే
    OS << "process";
  OS << " CWD\n";
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  స్థిర IntrusiveRefCntPtr<FileSystem> FS =
      makeIntrusiveRefCnt<RealFileSystem>(నిజం);
  sys::sandbox::violationIfEnabled();

  తిరిగి FS;
}

std::unique_ptr<FileSystem> vfs::createPhysicalFileSystem() {
  sys::sandbox::violationIfEnabled();

  తిరిగి std::make_unique<RealFileSystem>(అబద్ధం);
}

నామాకాశ {

తరగతి RealFSDirIter : బహిరంగ llvm::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

బహిరంగ:
  RealFSDirIter(స్థిరరాశి Twine &మార్గం, std::error_సంకేతం &EC) {
    స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

    Iter = sys::fs::directory_iterator(మార్గం, EC);
    ఒకవేళ (Iter != sys::fs::directory_iterator())
      Current_నమోదు = directory_నమోదు(Iter->మార్గం_2(), Iter->type());
  }

  std::error_సంకేతం increment() override {
    స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

    std::error_సంకేతం EC;
    Iter.increment(EC);
    Current_నమోదు = (Iter == llvm::sys::fs::directory_iterator())
                       ? directory_నమోదు()
                       : directory_నమోదు(Iter->మార్గం_2(), Iter->type());
    తిరిగి EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(స్థిరరాశి Twine &Dir,
                                             std::error_సంకేతం &EC) {
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  SmallString<128> Storage;
  తిరిగి directory_iterator(
      std::make_shared<RealFSDirIter>(adjust_మార్గం(Dir, Storage), EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/

OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> పునాది_FS) {
  FSList.నెట్టు_వెనుక(std::move(పునాది_FS));
}

శూన్య OverlayFileSystem::నెట్టు_Overlay(IntrusiveRefCntPtr<FileSystem> FS) {
  FSList.నెట్టు_వెనుక(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->సమితి_Current_Working_Directory(getCurrentWorkingDirectory().get());
}

ErrorOr<స్థితి_2> OverlayFileSystem::స్థితి_3(స్థిరరాశి Twine &మార్గం) {
  // FIXME: handle symlinks that cross file systems
  కొరకు (iterator I = overlays_begin(), E = overlays_చివర(); I != E; ++I) {
    ErrorOr<స్థితి_2> స్థితి_2 = (*I)->స్థితి_3(మార్గం);
    ఒకవేళ (స్థితి_2 || స్థితి_2.getError() != llvm::errc::no_such_file_or_directory)
      తిరిగి స్థితి_2;
  }
  తిరిగి make_error_సంకేతం(llvm::errc::no_such_file_or_directory);
}

తార్కిక OverlayFileSystem::exists(స్థిరరాశి Twine &మార్గం) {
  // FIXME: handle symlinks that cross file systems
  కొరకు (iterator I = overlays_begin(), E = overlays_చివర(); I != E; ++I) {
    ఒకవేళ ((*I)->exists(మార్గం))
      తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(స్థిరరాశి llvm::Twine &మార్గం) {
  // FIXME: handle symlinks that cross file systems
  కొరకు (iterator I = overlays_begin(), E = overlays_చివర(); I != E; ++I) {
    స్వయంచాలక ఫలితం = (*I)->openFileForRead(మార్గం);
    ఒకవేళ (ఫలితం || ఫలితం.getError() != llvm::errc::no_such_file_or_directory)
      తిరిగి ఫలితం;
  }
  తిరిగి make_error_సంకేతం(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() స్థిరరాశి {
  // All file systems are synchronized, just take the first working directory.
  తిరిగి FSList.front()->getCurrentWorkingDirectory();
}

std::error_సంకేతం
OverlayFileSystem::సమితి_Current_Working_Directory(స్థిరరాశి Twine &మార్గం) {
  కొరకు (స్వయంచాలక &FS : FSList)
    ఒకవేళ (std::error_సంకేతం EC = FS->సమితి_Current_Working_Directory(మార్గం))
      తిరిగి EC;
  తిరిగి {};
}

std::error_సంకేతం OverlayFileSystem::isLocal(స్థిరరాశి Twine &మార్గం, తార్కిక &ఫలితం) {
  కొరకు (స్వయంచాలక &FS : FSList)
    ఒకవేళ (FS->exists(మార్గం))
      తిరిగి FS->isLocal(మార్గం, ఫలితం);
  తిరిగి errc::no_such_file_or_directory;
}

std::error_సంకేతం OverlayFileSystem::get_Real_మార్గం(స్థిరరాశి Twine &మార్గం,
                                               Small_సదిశ_Impl<అక్షరం> &Output) {
  కొరకు (స్థిరరాశి స్వయంచాలక &FS : FSList)
    ఒకవేళ (FS->exists(మార్గం))
      తిరిగి FS->get_Real_మార్గం(మార్గం, Output);
  తిరిగి errc::no_such_file_or_directory;
}

శూన్య OverlayFileSystem::getDirectoryContentRealSources(
    స్థిరరాశి Twine &Dir, Small_సదిశ_Impl<std::string> &బహిర్గతం) {
  // All layers contribute.
  కొరకు (iterator I = overlays_begin(), E = overlays_చివర(); I != E; ++I)
    (*I)->getDirectoryContentRealSources(Dir, బహిర్గతం);
}

శూన్య OverlayFileSystem::సందర్శించు_బిడ్డ_File_Systems(సందర్శించు_Callback_Ty Callback) {
  కొరకు (IntrusiveRefCntPtr<FileSystem> FS : overlays_range()) {
    Callback(*FS);
    FS->సందర్శించు_బిడ్డ_File_Systems(Callback);
  }
}

శూన్య OverlayFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                  గుర్తులేని IndentLevel) స్థిరరాశి {
  printIndent(OS, IndentLevel);
  OS << "OverlayFileSystem\n";
  ఒకవేళ (Type == PrintType::Summary)
    తిరిగి;

  ఒకవేళ (Type == PrintType::Contents)
    Type = PrintType::Summary;
  కొరకు (స్థిరరాశి స్వయంచాలక &FS : overlays_range())
    FS->print(OS, Type, IndentLevel + 1);
}

llvm::vfs::detail::DirIterImpl::~DirIterImpl() = అప్రమేయ;

నామాకాశ {

/// Combines and deduplicates directory entries across multiple file systems.
తరగతి CombiningDirIterImpl : బహిరంగ llvm::vfs::detail::DirIterImpl {
  ఉపయోగించి FileSystemPtr = llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>;

  /// Iterators to combine, processed in reverse order.
  Small_సదిశ<directory_iterator, 8> IterList;
  /// The iterator currently being traversed.
  directory_iterator CurrentDirIter;
  /// The set of names already returned as entries.
  llvm::String_సమితి<> చూసిన_పేర్లు;

  /// Sets \c CurrentDirIter to the next iterator in the list, or leaves it as
  /// is (at its end position) if we've already gone through them all.
  std::error_సంకేతం incrementIter(తార్కిక Is_మొదటి_సమయం) {
    అయితే (!IterList.ఖాళీలు()) {
      CurrentDirIter = IterList.వెనుక();
      IterList.తీయి_వెనుక();
      ఒకవేళ (CurrentDirIter != directory_iterator())
        ఆపు; //దొరికింది
    }

    ఒకవేళ (Is_మొదటి_సమయం && CurrentDirIter == directory_iterator())
      తిరిగి errc::no_such_file_or_directory;
    తిరిగి {};
  }

  std::error_సంకేతం incrementDirIter(తార్కిక Is_మొదటి_సమయం) {
    ప్రతిజ్ఞా((Is_మొదటి_సమయం || CurrentDirIter != directory_iterator()) &&
           "incrementing past end");
    std::error_సంకేతం EC;
    ఒకవేళ (!Is_మొదటి_సమయం)
      CurrentDirIter.increment(EC);
    ఒకవేళ (!EC && CurrentDirIter == directory_iterator())
      EC = incrementIter(Is_మొదటి_సమయం);
    తిరిగి EC;
  }

  std::error_సంకేతం incrementImpl(తార్కిక Is_మొదటి_సమయం) {
    అయితే (నిజం) {
      std::error_సంకేతం EC = incrementDirIter(Is_మొదటి_సమయం);
      ఒకవేళ (EC || CurrentDirIter == directory_iterator()) {
        Current_నమోదు = directory_నమోదు();
        తిరిగి EC;
      }
      Current_నమోదు = *CurrentDirIter;
      StringRef Name = llvm::sys::మార్గం_2::filename(Current_నమోదు.మార్గం_2());
      ఒకవేళ (చూసిన_పేర్లు.చొప్పించు(Name).రెండవ_2)
        తిరిగి EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

బహిరంగ:
  CombiningDirIterImpl(ArrayRef<FileSystemPtr> FileSystems, std::string Dir,
                       std::error_సంకేతం &EC) {
    కొరకు (స్థిరరాశి స్వయంచాలక &FS : FileSystems) {
      std::error_సంకేతం FEC;
      directory_iterator Iter = FS->dir_begin(Dir, FEC);
      ఒకవేళ (FEC && FEC != errc::no_such_file_or_directory) {
        EC = FEC;
        తిరిగి;
      }
      ఒకవేళ (!FEC)
        IterList.నెట్టు_వెనుక(Iter);
    }
    EC = incrementImpl(నిజం);
  }

  CombiningDirIterImpl(ArrayRef<directory_iterator> DirIters,
                       std::error_సంకేతం &EC)
      : IterList(DirIters) {
    EC = incrementImpl(నిజం);
  }

  std::error_సంకేతం increment() override { తిరిగి incrementImpl(అబద్ధం); }
};

} // namespace

directory_iterator OverlayFileSystem::dir_begin(స్థిరరాశి Twine &Dir,
                                                std::error_సంకేతం &EC) {
  directory_iterator Combined = directory_iterator(
      std::make_shared<CombiningDirIterImpl>(FSList, Dir.str(), EC));
  ఒకవేళ (EC)
    తిరిగి {};
  తిరిగి Combined;
}

శూన్య ProxyFileSystem::anchor() {}

నామాకాశ llvm {
నామాకాశ vfs {

నామాకాశ detail {

గణన లోపల_జ్ఞాపకం_కణుపు_రకం {
  IME_File,
  IME_Directory,
  IME_HardLink,
  IME_SymbolicLink,
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file, symlink, hardlink or a directory.
తరగతి లోపల_జ్ఞాపకం_కణుపు {
  లోపల_జ్ఞాపకం_కణుపు_రకం రకం_3;
  std::string FileName;

బహిరంగ:
  లోపల_జ్ఞాపకం_కణుపు(llvm::StringRef FileName, లోపల_జ్ఞాపకం_కణుపు_రకం రకం_3)
      : రకం_3(రకం_3), FileName(std::string(llvm::sys::మార్గం_2::filename(FileName))) {
  }
  మాయా ~లోపల_జ్ఞాపకం_కణుపు() = అప్రమేయ;

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  మాయా స్థితి_2 get_స్థితి(స్థిరరాశి Twine &RequestedName) స్థిరరాశి = 0;

  /// Get the filename of this node (the name without the directory part).
  StringRef getFileName() స్థిరరాశి { తిరిగి FileName; }
  లోపల_జ్ఞాపకం_కణుపు_రకం get_రకం() స్థిరరాశి { తిరిగి రకం_3; }
  మాయా std::string గమ్యం_String(గుర్తులేని Indent) స్థిరరాశి = 0;
};

తరగతి లోపల_జ్ఞాపకం_File : బహిరంగ లోపల_జ్ఞాపకం_కణుపు {
  స్థితి_2 Stat;
  std::unique_ptr<llvm::జ్ఞాపకం_బఫరు> బఫరు;

బహిరంగ:
  లోపల_జ్ఞాపకం_File(స్థితి_2 Stat, std::unique_ptr<llvm::జ్ఞాపకం_బఫరు> బఫరు)
      : లోపల_జ్ఞాపకం_కణుపు(Stat.getName(), IME_File), Stat(std::move(Stat)),
        బఫరు(std::move(బఫరు)) {}

  స్థితి_2 get_స్థితి(స్థిరరాశి Twine &RequestedName) స్థిరరాశి override {
    తిరిగి స్థితి_2::copyWithNewName(Stat, RequestedName);
  }
  llvm::జ్ఞాపకం_బఫరు *get_బఫరు() స్థిరరాశి { తిరిగి బఫరు.get(); }

  std::string గమ్యం_String(గుర్తులేని Indent) స్థిరరాశి override {
    తిరిగి (std::string(Indent, ' ') + Stat.getName() + "\n").str();
  }

  స్థిర తార్కిక classof(స్థిరరాశి లోపల_జ్ఞాపకం_కణుపు *N) {
    తిరిగి N->get_రకం() == IME_File;
  }
};

నామాకాశ {

తరగతి లోపల_జ్ఞాపకం_Hard_Link : బహిరంగ లోపల_జ్ఞాపకం_కణుపు {
  స్థిరరాశి లోపల_జ్ఞాపకం_File &ResolvedFile;

బహిరంగ:
  లోపల_జ్ఞాపకం_Hard_Link(StringRef మార్గం, స్థిరరాశి లోపల_జ్ఞాపకం_File &ResolvedFile)
      : లోపల_జ్ఞాపకం_కణుపు(మార్గం, IME_HardLink), ResolvedFile(ResolvedFile) {}
  స్థిరరాశి లోపల_జ్ఞాపకం_File &getResolvedFile() స్థిరరాశి { తిరిగి ResolvedFile; }

  స్థితి_2 get_స్థితి(స్థిరరాశి Twine &RequestedName) స్థిరరాశి override {
    తిరిగి ResolvedFile.get_స్థితి(RequestedName);
  }

  std::string గమ్యం_String(గుర్తులేని Indent) స్థిరరాశి override {
    తిరిగి std::string(Indent, ' ') + "HardLink to -> " +
           ResolvedFile.గమ్యం_String(0);
  }

  స్థిర తార్కిక classof(స్థిరరాశి లోపల_జ్ఞాపకం_కణుపు *N) {
    తిరిగి N->get_రకం() == IME_HardLink;
  }
};

తరగతి లోపల_జ్ఞాపకం_Symbolic_Link : బహిరంగ లోపల_జ్ఞాపకం_కణుపు {
  std::string లక్ష్యం_మార్గం;
  స్థితి_2 Stat;

బహిరంగ:
  లోపల_జ్ఞాపకం_Symbolic_Link(StringRef మార్గం, StringRef లక్ష్యం_మార్గం, స్థితి_2 Stat)
      : లోపల_జ్ఞాపకం_కణుపు(మార్గం, IME_SymbolicLink), లక్ష్యం_మార్గం(std::move(లక్ష్యం_మార్గం)),
        Stat(Stat) {}

  std::string గమ్యం_String(గుర్తులేని Indent) స్థిరరాశి override {
    తిరిగి std::string(Indent, ' ') + "SymbolicLink to -> " + లక్ష్యం_మార్గం;
  }

  స్థితి_2 get_స్థితి(స్థిరరాశి Twine &RequestedName) స్థిరరాశి override {
    తిరిగి స్థితి_2::copyWithNewName(Stat, RequestedName);
  }

  StringRef get_లక్ష్యం_మార్గం() స్థిరరాశి { తిరిగి లక్ష్యం_మార్గం; }

  స్థిర తార్కిక classof(స్థిరరాశి లోపల_జ్ఞాపకం_కణుపు *N) {
    తిరిగి N->get_రకం() == IME_SymbolicLink;
  }
};

/// Adapt a InMemoryFile for VFS' File interface.  The goal is to make
/// \p InMemoryFileAdaptor mimic as much as possible the behavior of
/// \p RealFile.
తరగతి లోపల_జ్ఞాపకం_File_Adaptor : బహిరంగ File {
  స్థిరరాశి లోపల_జ్ఞాపకం_File &కణుపు;
  /// The name to use when returning a Status for this file.
  std::string RequestedName;

బహిరంగ:
  స్పష్టమైన లోపల_జ్ఞాపకం_File_Adaptor(స్థిరరాశి లోపల_జ్ఞాపకం_File &కణుపు,
                               std::string RequestedName)
      : కణుపు(కణుపు), RequestedName(std::move(RequestedName)) {}

  llvm::ErrorOr<స్థితి_2> స్థితి_3() override {
    తిరిగి కణుపు.get_స్థితి(RequestedName);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::జ్ఞాపకం_బఫరు>>
  get_బఫరు(స్థిరరాశి Twine &Name, int64_t File_కొలత, తార్కిక RequiresNullTerminator,
            తార్కిక IsVolatile) override {
    llvm::జ్ఞాపకం_బఫరు *Buf = కణుపు.get_బఫరు();
    తిరిగి llvm::జ్ఞాపకం_బఫరు::get_Mem_బఫరు(
        Buf->get_బఫరు(), Buf->get_బఫరు_Identifier(), RequiresNullTerminator);
  }

  std::error_సంకేతం close() override { తిరిగి {}; }

  శూన్య సమితి_మార్గం(స్థిరరాశి Twine &మార్గం) override { RequestedName = మార్గం.str(); }
};
} // namespace

తరగతి లోపల_జ్ఞాపకం_Directory : బహిరంగ లోపల_జ్ఞాపకం_కణుపు {
  స్థితి_2 Stat;
  std::map<std::string, std::unique_ptr<లోపల_జ్ఞాపకం_కణుపు>, std::less<>> Entries;

బహిరంగ:
  లోపల_జ్ఞాపకం_Directory(స్థితి_2 Stat)
      : లోపల_జ్ఞాపకం_కణుపు(Stat.getName(), IME_Directory), Stat(std::move(Stat)) {}

  /// Return the \p Status for this node. \p RequestedName should be the name
  /// through which the caller referred to this node. It will override
  /// \p Status::Name in the return value, to mimic the behavior of \p RealFile.
  స్థితి_2 get_స్థితి(స్థిరరాశి Twine &RequestedName) స్థిరరాశి override {
    తిరిగి స్థితి_2::copyWithNewName(Stat, RequestedName);
  }

  UniqueID getUniqueID() స్థిరరాశి { తిరిగి Stat.getUniqueID(); }

  లోపల_జ్ఞాపకం_కణుపు *get_బిడ్డ(StringRef Name) స్థిరరాశి {
    స్వయంచాలక I = Entries.కనుగొను(Name);
    ఒకవేళ (I != Entries.చివర())
      తిరిగి I->రెండవ_2.get();
    తిరిగి శూన్యసూచిక;
  }

  లోపల_జ్ఞాపకం_కణుపు *కూడు_బిడ్డ(StringRef Name, std::unique_ptr<లోపల_జ్ఞాపకం_కణుపు> బిడ్డ) {
    తిరిగి Entries.emplace(Name, std::move(బిడ్డ)).మొదటి_2->రెండవ_2.get();
  }

  ఉపయోగించి const_iterator = decltype(Entries)::const_iterator;

  const_iterator begin() స్థిరరాశి { తిరిగి Entries.begin(); }
  const_iterator చివర() స్థిరరాశి { తిరిగి Entries.చివర(); }

  std::string గమ్యం_String(గుర్తులేని Indent) స్థిరరాశి override {
    std::string ఫలితం =
        (std::string(Indent, ' ') + Stat.getName() + "\n").str();
    కొరకు (స్థిరరాశి స్వయంచాలక &నమోదు_2 : Entries)
      ఫలితం += నమోదు_2.రెండవ_2->గమ్యం_String(Indent + 2);
    తిరిగి ఫలితం;
  }

  స్థిర తార్కిక classof(స్థిరరాశి లోపల_జ్ఞాపకం_కణుపు *N) {
    తిరిగి N->get_రకం() == IME_Directory;
  }
};

} // namespace detail

// The UniqueID of in-memory files is derived from path and content.
// This avoids difficulties in creating exactly equivalent in-memory FSes,
// as often needed in multithreaded programs.
స్థిర sys::fs::UniqueID getUniqueID(hash_సంకేతం Hash) {
  తిరిగి sys::fs::UniqueID(std::numeric_limits<uint64_t>::max(),
                           uint64_t(size_t(Hash)));
}
స్థిర sys::fs::UniqueID getFileID(sys::fs::UniqueID మాతృ_2,
                                   llvm::StringRef Name,
                                   llvm::StringRef Contents) {
  తిరిగి getUniqueID(llvm::hash_combine(మాతృ_2.getFile(), Name, Contents));
}
స్థిర sys::fs::UniqueID getDirectoryID(sys::fs::UniqueID మాతృ_2,
                                        llvm::StringRef Name) {
  తిరిగి getUniqueID(llvm::hash_combine(మాతృ_2.getFile(), Name));
}

స్థితి_2 detail::New_లోపల_జ్ఞాపకం_కణుపు_Info::make_స్థితి() స్థిరరాశి {
  UniqueID UID =
      (Type == sys::fs::file_type::directory_file)
          ? getDirectoryID(DirUID, Name)
          : getFileID(DirUID, Name, బఫరు ? బఫరు->get_బఫరు() : "");

  తిరిగి స్థితి_2(మార్గం, UID, llvm::sys::గమ్యం_సమయం_బిందువు(Modification_సమయం), User,
                సమూహం, బఫరు ? బఫరు->get_బఫరు_కొలత() : 0, Type, Perms);
}

లోపల_జ్ఞాపకం_File_System::లోపల_జ్ఞాపకం_File_System(తార్కిక UseNormalizedPaths)
    : మూలం_2(కొత్త detail::లోపల_జ్ఞాపకం_Directory(
          స్థితి_2("", getDirectoryID(llvm::sys::fs::UniqueID(), ""),
                 llvm::sys::సమయం_బిందువు<>(), 0, 0, 0,
                 llvm::sys::fs::file_type::directory_file,
                 llvm::sys::fs::perms::all_all))),
      UseNormalizedPaths(UseNormalizedPaths) {}

లోపల_జ్ఞాపకం_File_System::~లోపల_జ్ఞాపకం_File_System() = అప్రమేయ;

std::string లోపల_జ్ఞాపకం_File_System::గమ్యం_String() స్థిరరాశి {
  తిరిగి మూలం_2->గమ్యం_String(/*Indent=*/0);
}

తార్కిక లోపల_జ్ఞాపకం_File_System::కూడు_File(స్థిరరాశి Twine &P, time_t Modification_సమయం,
                                 std::unique_ptr<llvm::జ్ఞాపకం_బఫరు> బఫరు,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> సమూహం,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms,
                                 Make_కణుపు_Fn Make_కణుపు) {
  SmallString<128> మార్గం;
  P.గమ్యం_సదిశ(మార్గం);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_సంకేతం EC = makeAbsolute(మార్గం);
  ప్రతిజ్ఞా(!EC);
  (శూన్య)EC;

  ఒకవేళ (useNormalizedPaths())
    llvm::sys::మార్గం_2::remove_dots(మార్గం, /*remove_dot_dot=*/నిజం);

  ఒకవేళ (మార్గం.ఖాళీలు())
    తిరిగి అబద్ధం;

  detail::లోపల_జ్ఞాపకం_Directory *Dir = మూలం_2.get();
  స్వయంచాలక I = llvm::sys::మార్గం_2::begin(మార్గం), E = sys::మార్గం_2::చివర(మార్గం);
  స్థిరరాశి స్వయంచాలక ResolvedUser = User.విలువ_or(0);
  స్థిరరాశి స్వయంచాలక Resolved_సమూహం = సమూహం.విలువ_or(0);
  స్థిరరాశి స్వయంచాలక ResolvedType = Type.విలువ_or(sys::fs::file_type::regular_file);
  స్థిరరాశి స్వయంచాలక ResolvedPerms = Perms.విలువ_or(sys::fs::all_all);
  // Any intermediate directories we create should be accessible by
  // the owner, even if Perms says otherwise for the final path.
  స్థిరరాశి స్వయంచాలక NewDirectoryPerms = ResolvedPerms | sys::fs::owner_all;

  StringRef Name = *I;
  అయితే (నిజం) {
    Name = *I;
    ++I;
    ఒకవేళ (I == E)
      ఆపు;
    detail::లోపల_జ్ఞాపకం_కణుపు *కణుపు = Dir->get_బిడ్డ(Name);
    ఒకవేళ (!కణుపు) {
      // This isn't the last element, so we create a new directory.
      స్థితి_2 Stat(
          StringRef(మార్గం.str().begin(), Name.చివర() - మార్గం.str().begin()),
          getDirectoryID(Dir->getUniqueID(), Name),
          llvm::sys::గమ్యం_సమయం_బిందువు(Modification_సమయం), ResolvedUser, Resolved_సమూహం,
          0, sys::fs::file_type::directory_file, NewDirectoryPerms);
      Dir = cast<detail::లోపల_జ్ఞాపకం_Directory>(Dir->కూడు_బిడ్డ(
          Name, std::make_unique<detail::లోపల_జ్ఞాపకం_Directory>(std::move(Stat))));
      కొనసాగు;
    }
    // Creating file under another file.
    ఒకవేళ (!isa<detail::లోపల_జ్ఞాపకం_Directory>(కణుపు))
      తిరిగి అబద్ధం;
    Dir = cast<detail::లోపల_జ్ఞాపకం_Directory>(కణుపు);
  }
  detail::లోపల_జ్ఞాపకం_కణుపు *కణుపు = Dir->get_బిడ్డ(Name);
  ఒకవేళ (!కణుపు) {
    Dir->కూడు_బిడ్డ(Name,
                  Make_కణుపు({Dir->getUniqueID(), మార్గం, Name, Modification_సమయం,
                            std::move(బఫరు), ResolvedUser, Resolved_సమూహం,
                            ResolvedType, ResolvedPerms}));
    తిరిగి నిజం;
  }
  ఒకవేళ (isa<detail::లోపల_జ్ఞాపకం_Directory>(కణుపు))
    తిరిగి ResolvedType == sys::fs::file_type::directory_file;

  ప్రతిజ్ఞా((isa<detail::లోపల_జ్ఞాపకం_File>(కణుపు) ||
          isa<detail::లోపల_జ్ఞాపకం_Hard_Link>(కణుపు)) &&
         "Must be either file, hardlink or directory!");

  // Return false only if the new file is different from the existing one.
  ఒకవేళ (స్వయంచాలక *Link = dyn_cast<detail::లోపల_జ్ఞాపకం_Hard_Link>(కణుపు)) {
    తిరిగి Link->getResolvedFile().get_బఫరు()->get_బఫరు() ==
           బఫరు->get_బఫరు();
  }
  తిరిగి cast<detail::లోపల_జ్ఞాపకం_File>(కణుపు)->get_బఫరు()->get_బఫరు() ==
         బఫరు->get_బఫరు();
}

తార్కిక లోపల_జ్ఞాపకం_File_System::కూడు_File(స్థిరరాశి Twine &P, time_t Modification_సమయం,
                                 std::unique_ptr<llvm::జ్ఞాపకం_బఫరు> బఫరు,
                                 std::optional<uint32_t> User,
                                 std::optional<uint32_t> సమూహం,
                                 std::optional<llvm::sys::fs::file_type> Type,
                                 std::optional<llvm::sys::fs::perms> Perms) {
  తిరిగి కూడు_File(P, Modification_సమయం, std::move(బఫరు), User, సమూహం, Type,
                 Perms,
                 [](detail::New_లోపల_జ్ఞాపకం_కణుపు_Info NNI)
                     -> std::unique_ptr<detail::లోపల_జ్ఞాపకం_కణుపు> {
                   స్థితి_2 Stat = NNI.make_స్థితి();
                   ఒకవేళ (Stat.getType() == sys::fs::file_type::directory_file)
                     తిరిగి std::make_unique<detail::లోపల_జ్ఞాపకం_Directory>(Stat);
                   తిరిగి std::make_unique<detail::లోపల_జ్ఞాపకం_File>(
                       Stat, std::move(NNI.బఫరు));
                 });
}

తార్కిక లోపల_జ్ఞాపకం_File_System::కూడు_File_No_Own(
    స్థిరరాశి Twine &P, time_t Modification_సమయం,
    స్థిరరాశి llvm::జ్ఞాపకం_బఫరు_Ref &బఫరు, std::optional<uint32_t> User,
    std::optional<uint32_t> సమూహం, std::optional<llvm::sys::fs::file_type> Type,
    std::optional<llvm::sys::fs::perms> Perms) {
  తిరిగి కూడు_File(P, Modification_సమయం, llvm::జ్ఞాపకం_బఫరు::get_Mem_బఫరు(బఫరు),
                 std::move(User), std::move(సమూహం), std::move(Type),
                 std::move(Perms),
                 [](detail::New_లోపల_జ్ఞాపకం_కణుపు_Info NNI)
                     -> std::unique_ptr<detail::లోపల_జ్ఞాపకం_కణుపు> {
                   స్థితి_2 Stat = NNI.make_స్థితి();
                   ఒకవేళ (Stat.getType() == sys::fs::file_type::directory_file)
                     తిరిగి std::make_unique<detail::లోపల_జ్ఞాపకం_Directory>(Stat);
                   తిరిగి std::make_unique<detail::లోపల_జ్ఞాపకం_File>(
                       Stat, std::move(NNI.బఫరు));
                 });
}

detail::Named_కణుపు_Or_Error
లోపల_జ్ఞాపకం_File_System::lookup_కణుపు(స్థిరరాశి Twine &P, తార్కిక FollowFinalSymlink,
                               size_t Symlink_లోతు) స్థిరరాశి {
  SmallString<128> మార్గం;
  P.గమ్యం_సదిశ(మార్గం);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_సంకేతం EC = makeAbsolute(మార్గం);
  ప్రతిజ్ఞా(!EC);
  (శూన్య)EC;

  ఒకవేళ (useNormalizedPaths())
    llvm::sys::మార్గం_2::remove_dots(మార్గం, /*remove_dot_dot=*/నిజం);

  స్థిరరాశి detail::లోపల_జ్ఞాపకం_Directory *Dir = మూలం_2.get();
  ఒకవేళ (మార్గం.ఖాళీలు())
    తిరిగి detail::Named_కణుపు_Or_Error(మార్గం, Dir);

  స్వయంచాలక I = llvm::sys::మార్గం_2::begin(మార్గం), E = llvm::sys::మార్గం_2::చివర(మార్గం);
  అయితే (నిజం) {
    detail::లోపల_జ్ఞాపకం_కణుపు *కణుపు = Dir->get_బిడ్డ(*I);
    ++I;
    ఒకవేళ (!కణుపు)
      తిరిగి errc::no_such_file_or_directory;

    ఒకవేళ (స్వయంచాలక Symlink = dyn_cast<detail::లోపల_జ్ఞాపకం_Symbolic_Link>(కణుపు)) {
      // If we're at the end of the path, and we're not following through
      // terminal symlinks, then we're done.
      ఒకవేళ (I == E && !FollowFinalSymlink)
        తిరిగి detail::Named_కణుపు_Or_Error(మార్గం, Symlink);

      ఒకవేళ (Symlink_లోతు > లోపల_జ్ఞాపకం_File_System::Max_Symlink_లోతు)
        తిరిగి errc::no_such_file_or_directory;

      SmallString<128> లక్ష్యం_మార్గం = Symlink->get_లక్ష్యం_మార్గం();
      ఒకవేళ (std::error_సంకేతం EC = makeAbsolute(లక్ష్యం_మార్గం))
        తిరిగి EC;

      // Keep going with the target. We always want to follow symlinks here
      // because we're either at the end of a path that we want to follow, or
      // not at the end of a path, in which case we need to follow the symlink
      // regardless.
      స్వయంచాలక లక్ష్యం_2 =
          lookup_కణుపు(లక్ష్యం_మార్గం, /*FollowFinalSymlink=*/నిజం, Symlink_లోతు + 1);
      ఒకవేళ (!లక్ష్యం_2 || I == E)
        తిరిగి లక్ష్యం_2;

      ఒకవేళ (!isa<detail::లోపల_జ్ఞాపకం_Directory>(*లక్ష్యం_2))
        తిరిగి errc::no_such_file_or_directory;

      // Otherwise, continue on the search in the symlinked directory.
      Dir = cast<detail::లోపల_జ్ఞాపకం_Directory>(*లక్ష్యం_2);
      కొనసాగు;
    }

    // Return the file if it's at the end of the path.
    ఒకవేళ (స్వయంచాలక File = dyn_cast<detail::లోపల_జ్ఞాపకం_File>(కణుపు)) {
      ఒకవేళ (I == E)
        తిరిగి detail::Named_కణుపు_Or_Error(మార్గం, File);
      తిరిగి errc::no_such_file_or_directory;
    }

    // If Node is HardLink then return the resolved file.
    ఒకవేళ (స్వయంచాలక File = dyn_cast<detail::లోపల_జ్ఞాపకం_Hard_Link>(కణుపు)) {
      ఒకవేళ (I == E)
        తిరిగి detail::Named_కణుపు_Or_Error(మార్గం, &File->getResolvedFile());
      తిరిగి errc::no_such_file_or_directory;
    }
    // Traverse directories.
    Dir = cast<detail::లోపల_జ్ఞాపకం_Directory>(కణుపు);
    ఒకవేళ (I == E)
      తిరిగి detail::Named_కణుపు_Or_Error(మార్గం, Dir);
  }
}

తార్కిక లోపల_జ్ఞాపకం_File_System::కూడు_Hard_Link(స్థిరరాశి Twine &NewLink,
                                     స్థిరరాశి Twine &లక్ష్యం_2) {
  స్వయంచాలక New_Link_కణుపు = lookup_కణుపు(NewLink, /*FollowFinalSymlink=*/అబద్ధం);
  // Whether symlinks in the hardlink target are followed is
  // implementation-defined in POSIX.
  // We're following symlinks here to be consistent with macOS.
  స్వయంచాలక లక్ష్యం_కణుపు = lookup_కణుపు(లక్ష్యం_2, /*FollowFinalSymlink=*/నిజం);
  // FromPath must not have been added before. ToPath must have been added
  // before. Resolved ToPath must be a File.
  ఒకవేళ (!లక్ష్యం_కణుపు || New_Link_కణుపు || !isa<detail::లోపల_జ్ఞాపకం_File>(*లక్ష్యం_కణుపు))
    తిరిగి అబద్ధం;
  తిరిగి కూడు_File(NewLink, 0, శూన్యసూచిక, std::nullopt, std::nullopt, std::nullopt,
                 std::nullopt, [&](detail::New_లోపల_జ్ఞాపకం_కణుపు_Info NNI) {
                   తిరిగి std::make_unique<detail::లోపల_జ్ఞాపకం_Hard_Link>(
                       NNI.మార్గం.str(),
                       *cast<detail::లోపల_జ్ఞాపకం_File>(*లక్ష్యం_కణుపు));
                 });
}

తార్కిక లోపల_జ్ఞాపకం_File_System::కూడు_Symbolic_Link(
    స్థిరరాశి Twine &NewLink, స్థిరరాశి Twine &లక్ష్యం_2, time_t Modification_సమయం,
    std::optional<uint32_t> User, std::optional<uint32_t> సమూహం,
    std::optional<llvm::sys::fs::perms> Perms) {
  స్వయంచాలక New_Link_కణుపు = lookup_కణుపు(NewLink, /*FollowFinalSymlink=*/అబద్ధం);
  ఒకవేళ (New_Link_కణుపు)
    తిరిగి అబద్ధం;

  SmallString<128> NewLinkStr, లక్ష్యం_Str;
  NewLink.గమ్యం_సదిశ(NewLinkStr);
  లక్ష్యం_2.గమ్యం_సదిశ(లక్ష్యం_Str);

  తిరిగి కూడు_File(NewLinkStr, Modification_సమయం, శూన్యసూచిక, User, సమూహం,
                 sys::fs::file_type::symlink_file, Perms,
                 [&](detail::New_లోపల_జ్ఞాపకం_కణుపు_Info NNI) {
                   తిరిగి std::make_unique<detail::లోపల_జ్ఞాపకం_Symbolic_Link>(
                       NewLinkStr, లక్ష్యం_Str, NNI.make_స్థితి());
                 });
}

llvm::ErrorOr<స్థితి_2> లోపల_జ్ఞాపకం_File_System::స్థితి_3(స్థిరరాశి Twine &మార్గం) {
  స్వయంచాలక కణుపు = lookup_కణుపు(మార్గం, /*FollowFinalSymlink=*/నిజం);
  ఒకవేళ (కణుపు)
    తిరిగి (*కణుపు)->get_స్థితి(మార్గం);
  తిరిగి కణుపు.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
లోపల_జ్ఞాపకం_File_System::openFileForRead(స్థిరరాశి Twine &మార్గం) {
  స్వయంచాలక కణుపు = lookup_కణుపు(మార్గం,/*FollowFinalSymlink=*/నిజం);
  ఒకవేళ (!కణుపు)
    తిరిగి కణుపు.getError();

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  ఒకవేళ (స్వయంచాలక *F = dyn_cast<detail::లోపల_జ్ఞాపకం_File>(*కణుపు))
    తిరిగి std::unique_ptr<File>(
        కొత్త detail::లోపల_జ్ఞాపకం_File_Adaptor(*F, మార్గం.str()));

  // FIXME: errc::not_a_file?
  తిరిగి make_error_సంకేతం(llvm::errc::invalid_వాదం);
}

/// Adaptor from InMemoryDir::iterator to directory_iterator.
తరగతి లోపల_జ్ఞాపకం_File_System::DirIterator : బహిరంగ llvm::vfs::detail::DirIterImpl {
  స్థిరరాశి లోపల_జ్ఞాపకం_File_System *FS;
  detail::లోపల_జ్ఞాపకం_Directory::const_iterator I;
  detail::లోపల_జ్ఞాపకం_Directory::const_iterator E;
  std::string RequestedDirName;

  శూన్య సమితి_Current_నమోదు() {
    ఒకవేళ (I != E) {
      SmallString<256> మార్గం(RequestedDirName);
      llvm::sys::మార్గం_2::append(మార్గం, I->రెండవ_2->getFileName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ఎంపిక (I->రెండవ_2->get_రకం()) {
      స్థితి detail::IME_File:
      స్థితి detail::IME_HardLink:
        Type = sys::fs::file_type::regular_file;
        ఆపు;
      స్థితి detail::IME_Directory:
        Type = sys::fs::file_type::directory_file;
        ఆపు;
      స్థితి detail::IME_SymbolicLink:
        ఒకవేళ (స్వయంచాలక Symlink_లక్ష్యం =
                FS->lookup_కణుపు(మార్గం, /*FollowFinalSymlink=*/నిజం)) {
          మార్గం = Symlink_లక్ష్యం.getName();
          Type = (*Symlink_లక్ష్యం)->get_స్థితి(మార్గం).getType();
        }
        ఆపు;
      }
      Current_నమోదు = directory_నమోదు(std::string(మార్గం), Type);
    } లేకపోతే {
      // When we're at the end, make CurrentEntry invalid and DirIterImpl will
      // do the rest.
      Current_నమోదు = directory_నమోదు();
    }
  }

బహిరంగ:
  DirIterator() = అప్రమేయ;

  DirIterator(స్థిరరాశి లోపల_జ్ఞాపకం_File_System *FS,
              స్థిరరాశి detail::లోపల_జ్ఞాపకం_Directory &Dir,
              std::string RequestedDirName)
      : FS(FS), I(Dir.begin()), E(Dir.చివర()),
        RequestedDirName(std::move(RequestedDirName)) {
    సమితి_Current_నమోదు();
  }

  std::error_సంకేతం increment() override {
    ++I;
    సమితి_Current_నమోదు();
    తిరిగి {};
  }
};

directory_iterator లోపల_జ్ఞాపకం_File_System::dir_begin(స్థిరరాశి Twine &Dir,
                                                 std::error_సంకేతం &EC) {
  స్వయంచాలక కణుపు = lookup_కణుపు(Dir, /*FollowFinalSymlink=*/నిజం);
  ఒకవేళ (!కణుపు) {
    EC = కణుపు.getError();
    తిరిగి directory_iterator(std::make_shared<DirIterator>());
  }

  ఒకవేళ (స్వయంచాలక *Dir_కణుపు = dyn_cast<detail::లోపల_జ్ఞాపకం_Directory>(*కణుపు))
    తిరిగి directory_iterator(
        std::make_shared<DirIterator>(ఇది, *Dir_కణుపు, Dir.str()));

  EC = make_error_సంకేతం(llvm::errc::not_a_directory);
  తిరిగి directory_iterator(std::make_shared<DirIterator>());
}

std::error_సంకేతం లోపల_జ్ఞాపకం_File_System::సమితి_Current_Working_Directory(స్థిరరాశి Twine &P) {
  SmallString<128> మార్గం;
  P.గమ్యం_సదిశ(మార్గం);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_సంకేతం EC = makeAbsolute(మార్గం);
  ప్రతిజ్ఞా(!EC);
  (శూన్య)EC;

  ఒకవేళ (useNormalizedPaths())
    llvm::sys::మార్గం_2::remove_dots(మార్గం, /*remove_dot_dot=*/నిజం);

  ఒకవేళ (!మార్గం.ఖాళీలు())
    WorkingDirectory = std::string(మార్గం);
  తిరిగి {};
}

std::error_సంకేతం లోపల_జ్ఞాపకం_File_System::get_Real_మార్గం(స్థిరరాశి Twine &మార్గం,
                                                Small_సదిశ_Impl<అక్షరం> &Output) {
  స్వయంచాలక CWD = getCurrentWorkingDirectory();
  ఒకవేళ (!CWD || CWD->ఖాళీలు())
    తిరిగి errc::operation_not_permitted;
  మార్గం.గమ్యం_సదిశ(Output);
  ఒకవేళ (స్వయంచాలక EC = makeAbsolute(Output))
    తిరిగి EC;
  llvm::sys::మార్గం_2::remove_dots(Output, /*remove_dot_dot=*/నిజం);
  తిరిగి {};
}

std::error_సంకేతం లోపల_జ్ఞాపకం_File_System::isLocal(స్థిరరాశి Twine &మార్గం, తార్కిక &ఫలితం) {
  ఫలితం = అబద్ధం;
  తిరిగి {};
}

శూన్య లోపల_జ్ఞాపకం_File_System::printImpl(raw_ostream &OS, PrintType PrintContents,
                                   గుర్తులేని IndentLevel) స్థిరరాశి {
  printIndent(OS, IndentLevel);
  OS << "InMemoryFileSystem\n";
}

} // namespace vfs
} // namespace llvm

//===-----------------------------------------------------------------------===/
// RedirectingFileSystem implementation
//===-----------------------------------------------------------------------===/

నామాకాశ {

స్థిర llvm::sys::మార్గం_2::Style getExistingStyle(llvm::StringRef మార్గం) {
  // Detect the path style in use by checking the first separator.
  llvm::sys::మార్గం_2::Style style = llvm::sys::మార్గం_2::Style::native;
  స్థిరరాశి size_t n = మార్గం.కనుగొను_మొదటి_యొక్క("/\\");
  // Can't distinguish between posix and windows_slash here.
  ఒకవేళ (n != static_cast<size_t>(-1))
    style = (మార్గం[n] == '/') ? llvm::sys::మార్గం_2::Style::posix
                             : llvm::sys::మార్గం_2::Style::windows_backslash;
  తిరిగి style;
}

/// Removes leading "./" as well as path components like ".." and ".".
స్థిర llvm::SmallString<256> canonicalize(llvm::StringRef మార్గం) {
  // First detect the path style in use by checking the first separator.
  llvm::sys::మార్గం_2::Style style = getExistingStyle(మార్గం);

  // Now remove the dots.  Explicitly specifying the path style prevents the
  // direction of the slashes from changing.
  llvm::SmallString<256> ఫలితం_2 =
      llvm::sys::మార్గం_2::remove_leading_dotslash(మార్గం, style);
  llvm::sys::మార్గం_2::remove_dots(ఫలితం_2, /*remove_dot_dot=*/నిజం, style);
  తిరిగి ఫలితం_2;
}

/// Whether the error and entry specify a file/directory that was not found.
స్థిర తార్కిక is_File_Not_దొరికింది(std::error_సంకేతం EC,
                           RedirectingFileSystem::నమోదు_2 *E = శూన్యసూచిక) {
  ఒకవేళ (E && !isa<RedirectingFileSystem::Directory_Remap_నమోదు>(E))
    తిరిగి అబద్ధం;
  తిరిగి EC == llvm::errc::no_such_file_or_directory;
}

} // anonymous namespace


RedirectingFileSystem::RedirectingFileSystem(IntrusiveRefCntPtr<FileSystem> FS)
    : ExternalFS(std::move(FS)) {
  ప్రతిజ్ఞా(ExternalFS && "RedirectingFileSystem requires an external FS");
  ఒకవేళ (స్వయంచాలక ExternalWorkingDirectory = ExternalFS->getCurrentWorkingDirectory())
    WorkingDirectory = *ExternalWorkingDirectory;
}

/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory entries.
తరగతి llvm::vfs::RedirectingFSDirIterImpl
    : బహిరంగ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  RedirectingFileSystem::Directory_నమోదు::iterator Current, చివర_2;

  std::error_సంకేతం incrementImpl(తార్కిక Is_మొదటి_సమయం) {
    ప్రతిజ్ఞా((Is_మొదటి_సమయం || Current != చివర_2) && "cannot iterate past end");
    ఒకవేళ (!Is_మొదటి_సమయం)
      ++Current;
    ఒకవేళ (Current != చివర_2) {
      SmallString<128> మార్గం_Str(Dir);
      llvm::sys::మార్గం_2::append(మార్గం_Str, (*Current)->getName());
      sys::fs::file_type Type = sys::fs::file_type::type_unknown;
      ఎంపిక ((*Current)->get_రకం()) {
      స్థితి RedirectingFileSystem::EK_Directory:
        [[fallthrough]];
      స్థితి RedirectingFileSystem::EK_DirectoryRemap:
        Type = sys::fs::file_type::directory_file;
        ఆపు;
      స్థితి RedirectingFileSystem::EK_File:
        Type = sys::fs::file_type::regular_file;
        ఆపు;
      }
      Current_నమోదు = directory_నమోదు(std::string(మార్గం_Str), Type);
    } లేకపోతే {
      Current_నమోదు = directory_నమోదు();
    }
    తిరిగి {};
  };

బహిరంగ:
  RedirectingFSDirIterImpl(
      స్థిరరాశి Twine &మార్గం, RedirectingFileSystem::Directory_నమోదు::iterator Begin,
      RedirectingFileSystem::Directory_నమోదు::iterator చివర_2, std::error_సంకేతం &EC)
      : Dir(మార్గం.str()), Current(Begin), చివర_2(చివర_2) {
    EC = incrementImpl(/*IsFirstTime=*/నిజం);
  }

  std::error_సంకేతం increment() override {
    తిరిగి incrementImpl(/*IsFirstTime=*/అబద్ధం);
  }
};

నామాకాశ {
/// Directory iterator implementation for \c RedirectingFileSystem's
/// directory remap entries that maps the paths reported by the external
/// file system's directory iterator back to the virtual directory's path.
తరగతి RedirectingFSDirRemapIterImpl : బహిరంగ llvm::vfs::detail::DirIterImpl {
  std::string Dir;
  llvm::sys::మార్గం_2::Style DirStyle;
  llvm::vfs::directory_iterator ExternalIter;

బహిరంగ:
  RedirectingFSDirRemapIterImpl(std::string Dir_మార్గం,
                                llvm::vfs::directory_iterator ExtIter)
      : Dir(std::move(Dir_మార్గం)), DirStyle(getExistingStyle(Dir)),
        ExternalIter(ExtIter) {
    ఒకవేళ (ExternalIter != llvm::vfs::directory_iterator())
      సమితి_Current_నమోదు();
  }

  శూన్య సమితి_Current_నమోదు() {
    StringRef External_మార్గం = ExternalIter->మార్గం_2();
    llvm::sys::మార్గం_2::Style ExternalStyle = getExistingStyle(External_మార్గం);
    StringRef File = llvm::sys::మార్గం_2::filename(External_మార్గం, ExternalStyle);

    SmallString<128> New_మార్గం(Dir);
    llvm::sys::మార్గం_2::append(New_మార్గం, DirStyle, File);

    Current_నమోదు = directory_నమోదు(std::string(New_మార్గం), ExternalIter->type());
  }

  std::error_సంకేతం increment() override {
    std::error_సంకేతం EC;
    ExternalIter.increment(EC);
    ఒకవేళ (!EC && ExternalIter != llvm::vfs::directory_iterator())
      సమితి_Current_నమోదు();
    లేకపోతే
      Current_నమోదు = directory_నమోదు();
    తిరిగి EC;
  }
};
} // namespace

llvm::ErrorOr<std::string>
RedirectingFileSystem::getCurrentWorkingDirectory() స్థిరరాశి {
  తిరిగి WorkingDirectory;
}

std::error_సంకేతం
RedirectingFileSystem::సమితి_Current_Working_Directory(స్థిరరాశి Twine &మార్గం) {
  // Don't change the working directory if the path doesn't exist.
  ఒకవేళ (!exists(మార్గం))
    తిరిగి errc::no_such_file_or_directory;

  SmallString<128> Absolute_మార్గం;
  మార్గం.గమ్యం_సదిశ(Absolute_మార్గం);
  ఒకవేళ (std::error_సంకేతం EC = makeAbsolute(Absolute_మార్గం))
    తిరిగి EC;
  WorkingDirectory = std::string(Absolute_మార్గం);
  తిరిగి {};
}

std::error_సంకేతం RedirectingFileSystem::isLocal(స్థిరరాశి Twine &మార్గం_3,
                                               తార్కిక &ఫలితం) {
  SmallString<256> మార్గం;
  మార్గం_3.గమ్యం_సదిశ(మార్గం);

  ఒకవేళ (makeAbsolute(మార్గం))
    తిరిగి {};

  తిరిగి ExternalFS->isLocal(మార్గం, ఫలితం);
}

std::error_సంకేతం RedirectingFileSystem::makeAbsolute(Small_సదిశ_Impl<అక్షరం> &మార్గం) స్థిరరాశి {
  // is_absolute(..., Style::windows_*) accepts paths with both slash types.
  ఒకవేళ (llvm::sys::మార్గం_2::is_absolute(మార్గం, llvm::sys::మార్గం_2::Style::posix) ||
      llvm::sys::మార్గం_2::is_absolute(మార్గం,
                                   llvm::sys::మార్గం_2::Style::windows_backslash))
    // This covers windows absolute path with forward slash as well, as the
    // forward slashes are treated as path separation in llvm::path
    // regardless of what path::Style is used.
    తిరిగి {};

  స్వయంచాలక WorkingDir = getCurrentWorkingDirectory();
  ఒకవేళ (!WorkingDir)
    తిరిగి WorkingDir.getError();

  తిరిగి makeAbsolute(WorkingDir.get(), మార్గం);
}

std::error_సంకేతం
RedirectingFileSystem::makeAbsolute(StringRef WorkingDir,
                                    Small_సదిశ_Impl<అక్షరం> &మార్గం) స్థిరరాశి {
  // We can't use sys::fs::make_absolute because that assumes the path style
  // is native and there is no way to override that.  Since we know WorkingDir
  // is absolute, we can use it to determine which style we actually have and
  // append Path ourselves.
  ఒకవేళ (!WorkingDir.ఖాళీలు() &&
      !sys::మార్గం_2::is_absolute(WorkingDir, sys::మార్గం_2::Style::posix) &&
      !sys::మార్గం_2::is_absolute(WorkingDir,
                              sys::మార్గం_2::Style::windows_backslash)) {
    తిరిగి std::error_సంకేతం();
  }
  sys::మార్గం_2::Style style = sys::మార్గం_2::Style::windows_backslash;
  ఒకవేళ (sys::మార్గం_2::is_absolute(WorkingDir, sys::మార్గం_2::Style::posix)) {
    style = sys::మార్గం_2::Style::posix;
  } లేకపోతే {
    // Distinguish between windows_backslash and windows_slash; getExistingStyle
    // returns posix for a path with windows_slash.
    ఒకవేళ (getExistingStyle(WorkingDir) != sys::మార్గం_2::Style::windows_backslash)
      style = sys::మార్గం_2::Style::windows_slash;
  }

  std::string ఫలితం = std::string(WorkingDir);
  StringRef Dir(ఫలితం);
  ఒకవేళ (!Dir.చివర్లు_with(sys::మార్గం_2::get_separator(style))) {
    ఫలితం += sys::మార్గం_2::get_separator(style);
  }
  // backslashes '\' are legit path charactors under POSIX. Windows APIs
  // like CreateFile accepts forward slashes '/' as path
  // separator (even when mixed with backslashes). Therefore,
  // `Path` should be directly appended to `WorkingDir` without converting
  // path separator.
  ఫలితం.append(మార్గం.దత్తాంశం(), మార్గం.కొలత_2());
  మార్గం.కేటాయించు(ఫలితం.begin(), ఫలితం.చివర());

  తిరిగి {};
}

directory_iterator RedirectingFileSystem::dir_begin(స్థిరరాశి Twine &Dir,
                                                    std::error_సంకేతం &EC) {
  SmallString<256> మార్గం;
  Dir.గమ్యం_సదిశ(మార్గం);

  EC = makeAbsolute(మార్గం);
  ఒకవేళ (EC)
    తిరిగి {};

  ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> ఫలితం = lookup_మార్గం(మార్గం);
  ఒకవేళ (!ఫలితం) {
    ఒకవేళ (Redirection != Redirect_రకం::RedirectOnly &&
        is_File_Not_దొరికింది(ఫలితం.getError()))
      తిరిగి ExternalFS->dir_begin(మార్గం, EC);

    EC = ఫలితం.getError();
    తిరిగి {};
  }

  // Use status to make sure the path exists and refers to a directory.
  ErrorOr<స్థితి_2> S = స్థితి_3(మార్గం, Dir, *ఫలితం);
  ఒకవేళ (!S) {
    ఒకవేళ (Redirection != Redirect_రకం::RedirectOnly &&
        is_File_Not_దొరికింది(S.getError(), ఫలితం->E))
      తిరిగి ExternalFS->dir_begin(Dir, EC);

    EC = S.getError();
    తిరిగి {};
  }

  ఒకవేళ (!S->isDirectory()) {
    EC = errc::not_a_directory;
    తిరిగి {};
  }

  // Create the appropriate directory iterator based on whether we found a
  // DirectoryRemapEntry or DirectoryEntry.
  directory_iterator RedirectIter;
  std::error_సంకేతం RedirectEC;
  ఒకవేళ (స్వయంచాలక ExtRedirect = ఫలితం->getExternalRedirect()) {
    స్వయంచాలక RE = cast<RedirectingFileSystem::Remap_నమోదు>(ఫలితం->E);
    RedirectIter = ExternalFS->dir_begin(*ExtRedirect, RedirectEC);

    ఒకవేళ (!RE->useExternalName(Use_External_పేర్లు)) {
      // Update the paths in the results to use the virtual directory's path.
      RedirectIter =
          directory_iterator(std::make_shared<RedirectingFSDirRemapIterImpl>(
              std::string(మార్గం), RedirectIter));
    }
  } లేకపోతే {
    స్వయంచాలక DE = cast<Directory_నమోదు>(ఫలితం->E);
    RedirectIter =
        directory_iterator(std::make_shared<RedirectingFSDirIterImpl>(
            మార్గం, DE->contents_begin(), DE->contents_చివర(), RedirectEC));
  }

  ఒకవేళ (RedirectEC) {
    ఒకవేళ (RedirectEC != errc::no_such_file_or_directory) {
      EC = RedirectEC;
      తిరిగి {};
    }
    RedirectIter = {};
  }

  ఒకవేళ (Redirection == Redirect_రకం::RedirectOnly) {
    EC = RedirectEC;
    తిరిగి RedirectIter;
  }

  std::error_సంకేతం ExternalEC;
  directory_iterator ExternalIter = ExternalFS->dir_begin(మార్గం, ExternalEC);
  ఒకవేళ (ExternalEC) {
    ఒకవేళ (ExternalEC != errc::no_such_file_or_directory) {
      EC = ExternalEC;
      తిరిగి {};
    }
    ExternalIter = {};
  }

  Small_సదిశ<directory_iterator, 2> Iters;
  ఎంపిక (Redirection) {
  స్థితి Redirect_రకం::Fallthrough:
    Iters.నెట్టు_వెనుక(ExternalIter);
    Iters.నెట్టు_వెనుక(RedirectIter);
    ఆపు;
  స్థితి Redirect_రకం::Fallback:
    Iters.నెట్టు_వెనుక(RedirectIter);
    Iters.నెట్టు_వెనుక(ExternalIter);
    ఆపు;
  అప్రమేయ:
    llvm_unreachable("unhandled RedirectKind");
  }

  directory_iterator Combined{
      std::make_shared<CombiningDirIterImpl>(Iters, EC)};
  ఒకవేళ (EC)
    తిరిగి {};
  తిరిగి Combined;
}

శూన్య RedirectingFileSystem::సమితి_Overlay_File_Dir(StringRef Dir) {
  OverlayFileDir = Dir.str();
}

StringRef RedirectingFileSystem::getOverlayFileDir() స్థిరరాశి {
  తిరిగి OverlayFileDir;
}

శూన్య RedirectingFileSystem::సమితి_Fallthrough(తార్కిక Fallthrough) {
  ఒకవేళ (Fallthrough) {
    Redirection = RedirectingFileSystem::Redirect_రకం::Fallthrough;
  } లేకపోతే {
    Redirection = RedirectingFileSystem::Redirect_రకం::RedirectOnly;
  }
}

శూన్య RedirectingFileSystem::సమితి_Redirection(
    RedirectingFileSystem::Redirect_రకం రకం_3) {
  Redirection = రకం_3;
}

std::సదిశ<StringRef> RedirectingFileSystem::getRoots() స్థిరరాశి {
  std::సదిశ<StringRef> R;
  R.reserve(Roots.కొలత_2());
  కొరకు (స్థిరరాశి స్వయంచాలక &మూలం_2 : Roots)
    R.నెట్టు_వెనుక(మూలం_2->getName());
  తిరిగి R;
}

శూన్య RedirectingFileSystem::printImpl(raw_ostream &OS, PrintType Type,
                                      గుర్తులేని IndentLevel) స్థిరరాశి {
  printIndent(OS, IndentLevel);
  OS << "RedirectingFileSystem (UseExternalNames: "
     << (Use_External_పేర్లు ? "true" : "false") << ")\n";
  ఒకవేళ (Type == PrintType::Summary)
    తిరిగి;

  కొరకు (స్థిరరాశి స్వయంచాలక &మూలం_2 : Roots)
    print_నమోదు(OS, మూలం_2.get(), IndentLevel);

  printIndent(OS, IndentLevel);
  OS << "ExternalFS:\n";
  ExternalFS->print(OS, Type == PrintType::Contents ? PrintType::Summary : Type,
                    IndentLevel + 1);
}

శూన్య RedirectingFileSystem::print_నమోదు(raw_ostream &OS,
                                       RedirectingFileSystem::నమోదు_2 *E,
                                       గుర్తులేని IndentLevel) స్థిరరాశి {
  printIndent(OS, IndentLevel);
  OS << "'" << E->getName() << "'";

  ఎంపిక (E->get_రకం()) {
  స్థితి EK_Directory: {
    స్వయంచాలక *DE = cast<RedirectingFileSystem::Directory_నమోదు>(E);

    OS << "\n";
    కొరకు (std::unique_ptr<నమోదు_2> &Sub_నమోదు :
         llvm::make_range(DE->contents_begin(), DE->contents_చివర()))
      print_నమోదు(OS, Sub_నమోదు.get(), IndentLevel + 1);
    ఆపు;
  }
  స్థితి EK_DirectoryRemap:
  స్థితి EK_File: {
    స్వయంచాలక *RE = cast<RedirectingFileSystem::Remap_నమోదు>(E);
    OS << " -> '" << RE->get_External_Contents_మార్గం() << "'";
    ఎంపిక (RE->getUseName()) {
    స్థితి NK_Not_సమితి:
      ఆపు;
    స్థితి NK_External:
      OS << " (UseExternalName: true)";
      ఆపు;
    స్థితి NK_Virtual:
      OS << " (UseExternalName: false)";
      ఆపు;
    }
    OS << "\n";
    ఆపు;
  }
  }
}

శూన్య RedirectingFileSystem::సందర్శించు_బిడ్డ_File_Systems(సందర్శించు_Callback_Ty Callback) {
  ఒకవేళ (ExternalFS) {
    Callback(*ExternalFS);
    ExternalFS->సందర్శించు_బిడ్డ_File_Systems(Callback);
  }
}

/// A helper class to hold the common YAML parsing state.
తరగతి llvm::vfs::RedirectingFileSystemParser {
  yaml::ప్రవాహం &ప్రవాహం;

  శూన్య error(yaml::కణుపు *N, స్థిరరాశి Twine &సందేశం_4) { ప్రవాహం.printError(N, సందేశం_4); }

  // false on error
  తార్కిక parseScalarString(yaml::కణుపు *N, StringRef &ఫలితం,
                         Small_సదిశ_Impl<అక్షరం> &Storage) {
    స్థిరరాశి స్వయంచాలక *S = dyn_cast_if_present<yaml::Scalar_కణుపు>(N);

    ఒకవేళ (!S) {
      error(N, "expected string");
      తిరిగి అబద్ధం;
    }
    ఫలితం = S->get_విలువ(Storage);
    తిరిగి నిజం;
  }

  // false on error
  తార్కిక parseScalarBool(yaml::కణుపు *N, తార్కిక &ఫలితం) {
    SmallString<5> Storage;
    StringRef విలువ;
    ఒకవేళ (!parseScalarString(N, విలువ, Storage))
      తిరిగి అబద్ధం;

    ఒకవేళ (విలువ.equals_insensitive("true") || విలువ.equals_insensitive("on") ||
        విలువ.equals_insensitive("yes") || విలువ == "1") {
      ఫలితం = నిజం;
      తిరిగి నిజం;
    } లేకపోతే ఒకవేళ (విలువ.equals_insensitive("false") ||
               విలువ.equals_insensitive("off") ||
               విలువ.equals_insensitive("no") || విలువ == "0") {
      ఫలితం = అబద్ధం;
      తిరిగి నిజం;
    }

    error(N, "expected boolean value");
    తిరిగి అబద్ధం;
  }

  std::optional<RedirectingFileSystem::Redirect_రకం>
  parse_Redirect_రకం(yaml::కణుపు *N) {
    SmallString<12> Storage;
    StringRef విలువ;
    ఒకవేళ (!parseScalarString(N, విలువ, Storage))
      తిరిగి std::nullopt;

    ఒకవేళ (విలువ.equals_insensitive("fallthrough")) {
      తిరిగి RedirectingFileSystem::Redirect_రకం::Fallthrough;
    } లేకపోతే ఒకవేళ (విలువ.equals_insensitive("fallback")) {
      తిరిగి RedirectingFileSystem::Redirect_రకం::Fallback;
    } లేకపోతే ఒకవేళ (విలువ.equals_insensitive("redirect-only")) {
      తిరిగి RedirectingFileSystem::Redirect_రకం::RedirectOnly;
    }
    తిరిగి std::nullopt;
  }

  std::optional<RedirectingFileSystem::మూలం_Relative_రకం>
  parse_మూలం_Relative_రకం(yaml::కణుపు *N) {
    SmallString<12> Storage;
    StringRef విలువ;
    ఒకవేళ (!parseScalarString(N, విలువ, Storage))
      తిరిగి std::nullopt;
    ఒకవేళ (విలువ.equals_insensitive("cwd")) {
      తిరిగి RedirectingFileSystem::మూలం_Relative_రకం::CWD;
    } లేకపోతే ఒకవేళ (విలువ.equals_insensitive("overlay-dir")) {
      తిరిగి RedirectingFileSystem::మూలం_Relative_రకం::OverlayDir;
    }
    తిరిగి std::nullopt;
  }

  నిర్మాణం కీలకం_స్థితి {
    తార్కిక Required;
    తార్కిక చూసిన = అబద్ధం;

    కీలకం_స్థితి(తార్కిక Required = అబద్ధం) : Required(Required) {}
  };

  ఉపయోగించి కీలకం_స్థితి_Pair = std::pair<StringRef, కీలకం_స్థితి>;

  // false on error
  తార్కిక check_Duplicate_Or_Unknown_కీలకం(yaml::కణుపు *కీలకం_కణుపు, StringRef కీలకం,
                                  DenseMap<StringRef, కీలకం_స్థితి> &కీలకాలు_2) {
    స్వయంచాలక It = కీలకాలు_2.కనుగొను(కీలకం);
    ఒకవేళ (It == కీలకాలు_2.చివర()) {
      error(కీలకం_కణుపు, "unknown key");
      తిరిగి అబద్ధం;
    }
    కీలకం_స్థితి &S = It->రెండవ_2;
    ఒకవేళ (S.చూసిన) {
      error(కీలకం_కణుపు, Twine("duplicate key '") + కీలకం + "'");
      తిరిగి అబద్ధం;
    }
    S.చూసిన = నిజం;
    తిరిగి నిజం;
  }

  // false on error
  తార్కిక check_Missing_కీలకాలు(yaml::కణుపు *Obj, DenseMap<StringRef, కీలకం_స్థితి> &కీలకాలు_2) {
    కొరకు (స్థిరరాశి స్వయంచాలక &I : కీలకాలు_2) {
      ఒకవేళ (I.రెండవ_2.Required && !I.రెండవ_2.చూసిన) {
        error(Obj, Twine("missing key '") + I.మొదటి_2 + "'");
        తిరిగి అబద్ధం;
      }
    }
    తిరిగి నిజం;
  }

బహిరంగ:
  స్థిర RedirectingFileSystem::నమోదు_2 *
  lookup_Or_Create_నమోదు(RedirectingFileSystem *FS, StringRef Name,
                      RedirectingFileSystem::నమోదు_2 *మాతృ_నమోదు = శూన్యసూచిక) {
    ఒకవేళ (!మాతృ_నమోదు) { // Look for a existent root
      కొరకు (స్థిరరాశి స్వయంచాలక &మూలం_2 : FS->Roots) {
        ఒకవేళ (Name == మూలం_2->getName()) {
          మాతృ_నమోదు = మూలం_2.get();
          తిరిగి మాతృ_నమోదు;
        }
      }
    } లేకపోతే { // Advance to the next component
      స్వయంచాలక *DE = dyn_cast<RedirectingFileSystem::Directory_నమోదు>(మాతృ_నమోదు);
      కొరకు (std::unique_ptr<RedirectingFileSystem::నమోదు_2> &Content :
           llvm::make_range(DE->contents_begin(), DE->contents_చివర())) {
        స్వయంచాలక *DirContent =
            dyn_cast<RedirectingFileSystem::Directory_నమోదు>(Content.get());
        ఒకవేళ (DirContent && Name == Content->getName())
          తిరిగి DirContent;
      }
    }

    // ... or create a new one
    std::unique_ptr<RedirectingFileSystem::నమోదు_2> E =
        std::make_unique<RedirectingFileSystem::Directory_నమోదు>(
            Name, స్థితి_2("", get_తదుపరి_Virtual_Unique_ID(),
                         std::chrono::system_clock::now(), 0, 0, 0,
                         file_type::directory_file, sys::fs::all_all));

    ఒకవేళ (!మాతృ_నమోదు) { // Add a new root to the overlay
      FS->Roots.నెట్టు_వెనుక(std::move(E));
      మాతృ_నమోదు = FS->Roots.వెనుక().get();
      తిరిగి మాతృ_నమోదు;
    }

    స్వయంచాలక *DE = cast<RedirectingFileSystem::Directory_నమోదు>(మాతృ_నమోదు);
    DE->కూడు_Content(std::move(E));
    తిరిగి DE->get_చివరి_Content();
  }

అంతరంగ:
  శూన్య unique_Overlay_వృక్షం(RedirectingFileSystem *FS,
                         RedirectingFileSystem::నమోదు_2 *SrcE,
                         RedirectingFileSystem::నమోదు_2 *New_మాతృ_E = శూన్యసూచిక) {
    StringRef Name = SrcE->getName();
    ఎంపిక (SrcE->get_రకం()) {
    స్థితి RedirectingFileSystem::EK_Directory: {
      స్వయంచాలక *DE = cast<RedirectingFileSystem::Directory_నమోదు>(SrcE);
      // Empty directories could be present in the YAML as a way to
      // describe a file for a current directory after some of its subdir
      // is parsed. This only leads to redundant walks, ignore it.
      ఒకవేళ (!Name.ఖాళీలు())
        New_మాతృ_E = lookup_Or_Create_నమోదు(FS, Name, New_మాతృ_E);
      కొరకు (std::unique_ptr<RedirectingFileSystem::నమోదు_2> &Sub_నమోదు :
           llvm::make_range(DE->contents_begin(), DE->contents_చివర()))
        unique_Overlay_వృక్షం(FS, Sub_నమోదు.get(), New_మాతృ_E);
      ఆపు;
    }
    స్థితి RedirectingFileSystem::EK_DirectoryRemap: {
      ప్రతిజ్ఞా(New_మాతృ_E && "Parent entry must exist");
      స్వయంచాలక *DR = cast<RedirectingFileSystem::Directory_Remap_నమోదు>(SrcE);
      స్వయంచాలక *DE = cast<RedirectingFileSystem::Directory_నమోదు>(New_మాతృ_E);
      DE->కూడు_Content(
          std::make_unique<RedirectingFileSystem::Directory_Remap_నమోదు>(
              Name, DR->get_External_Contents_మార్గం(), DR->getUseName()));
      ఆపు;
    }
    స్థితి RedirectingFileSystem::EK_File: {
      ప్రతిజ్ఞా(New_మాతృ_E && "Parent entry must exist");
      స్వయంచాలక *FE = cast<RedirectingFileSystem::File_నమోదు>(SrcE);
      స్వయంచాలక *DE = cast<RedirectingFileSystem::Directory_నమోదు>(New_మాతృ_E);
      DE->కూడు_Content(std::make_unique<RedirectingFileSystem::File_నమోదు>(
          Name, FE->get_External_Contents_మార్గం(), FE->getUseName()));
      ఆపు;
    }
    }
  }

  std::unique_ptr<RedirectingFileSystem::నమోదు_2>
  parse_నమోదు(yaml::కణుపు *N, RedirectingFileSystem *FS, తార్కిక Is_మూలం_నమోదు) {
    స్వయంచాలక *M = dyn_cast<yaml::Mapping_కణుపు>(N);
    ఒకవేళ (!M) {
      error(N, "expected mapping node for file or directory entry");
      తిరిగి శూన్యసూచిక;
    }

    కీలకం_స్థితి_Pair Fields[] = {
        కీలకం_స్థితి_Pair("name", నిజం),
        కీలకం_స్థితి_Pair("type", నిజం),
        కీలకం_స్థితి_Pair("contents", అబద్ధం),
        కీలకం_స్థితి_Pair("external-contents", అబద్ధం),
        కీలకం_స్థితి_Pair("use-external-name", అబద్ధం),
    };

    DenseMap<StringRef, కీలకం_స్థితి> కీలకాలు_2(std::begin(Fields), std::చివర(Fields));

    గణన { CF_Not_సమితి, CF_List, CF_External } Contents_క్షేత్రం = CF_Not_సమితి;
    std::సదిశ<std::unique_ptr<RedirectingFileSystem::నమోదు_2>>
        నమోదు_Array_Contents;
    SmallString<256> External_Contents_మార్గం;
    SmallString<256> Name;
    yaml::కణుపు *Name_విలువ_కణుపు = శూన్యసూచిక;
    స్వయంచాలక UseExternalName = RedirectingFileSystem::NK_Not_సమితి;
    RedirectingFileSystem::నమోదు_రకం రకం_3;

    కొరకు (స్వయంచాలక &I : *M) {
      StringRef కీలకం;
      // Reuse the buffer for key and value, since we don't look at key after
      // parsing value.
      SmallString<256> బఫరు;
      ఒకవేళ (!parseScalarString(I.get_కీలకం(), కీలకం, బఫరు))
        తిరిగి శూన్యసూచిక;

      ఒకవేళ (!check_Duplicate_Or_Unknown_కీలకం(I.get_కీలకం(), కీలకం, కీలకాలు_2))
        తిరిగి శూన్యసూచిక;

      StringRef విలువ;
      ఒకవేళ (కీలకం == "name") {
        ఒకవేళ (!parseScalarString(I.get_విలువ(), విలువ, బఫరు))
          తిరిగి శూన్యసూచిక;

        Name_విలువ_కణుపు = I.get_విలువ();
        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        Name = canonicalize(విలువ).str();
      } లేకపోతే ఒకవేళ (కీలకం == "type") {
        ఒకవేళ (!parseScalarString(I.get_విలువ(), విలువ, బఫరు))
          తిరిగి శూన్యసూచిక;
        ఒకవేళ (విలువ == "file")
          రకం_3 = RedirectingFileSystem::EK_File;
        లేకపోతే ఒకవేళ (విలువ == "directory")
          రకం_3 = RedirectingFileSystem::EK_Directory;
        లేకపోతే ఒకవేళ (విలువ == "directory-remap")
          రకం_3 = RedirectingFileSystem::EK_DirectoryRemap;
        లేకపోతే {
          error(I.get_విలువ(), "unknown value for 'type'");
          తిరిగి శూన్యసూచిక;
        }
      } లేకపోతే ఒకవేళ (కీలకం == "contents") {
        ఒకవేళ (Contents_క్షేత్రం != CF_Not_సమితి) {
          error(I.get_కీలకం(),
                "entry already has 'contents' or 'external-contents'");
          తిరిగి శూన్యసూచిక;
        }
        Contents_క్షేత్రం = CF_List;
        స్వయంచాలక *Contents = dyn_cast_if_present<yaml::Sequence_కణుపు>(I.get_విలువ());
        ఒకవేళ (!Contents) {
          // FIXME: this is only for directories, what about files?
          error(I.get_విలువ(), "expected array");
          తిరిగి శూన్యసూచిక;
        }

        కొరకు (స్వయంచాలక &I : *Contents) {
          ఒకవేళ (std::unique_ptr<RedirectingFileSystem::నమోదు_2> E =
                  parse_నమోదు(&I, FS, /*IsRootEntry*/ అబద్ధం))
            నమోదు_Array_Contents.నెట్టు_వెనుక(std::move(E));
          లేకపోతే
            తిరిగి శూన్యసూచిక;
        }
      } లేకపోతే ఒకవేళ (కీలకం == "external-contents") {
        ఒకవేళ (Contents_క్షేత్రం != CF_Not_సమితి) {
          error(I.get_కీలకం(),
                "entry already has 'contents' or 'external-contents'");
          తిరిగి శూన్యసూచిక;
        }
        Contents_క్షేత్రం = CF_External;
        ఒకవేళ (!parseScalarString(I.get_విలువ(), విలువ, బఫరు))
          తిరిగి శూన్యసూచిక;

        SmallString<256> నిండిన_మార్గం;
        ఒకవేళ (FS->IsRelativeOverlay) {
          నిండిన_మార్గం = FS->getOverlayFileDir();
          ప్రతిజ్ఞా(!నిండిన_మార్గం.ఖాళీలు() &&
                 "External contents prefix directory must exist");
          SmallString<256> Abs_నిండిన_మార్గం = విలువ;
          ఒకవేళ (FS->makeAbsolute(నిండిన_మార్గం, Abs_నిండిన_మార్గం)) {
            error(N, "failed to make 'external-contents' absolute");
            తిరిగి శూన్యసూచిక;
          }
          నిండిన_మార్గం = Abs_నిండిన_మార్గం;
        } లేకపోతే {
          నిండిన_మార్గం = విలువ;
        }

        // Guarantee that old YAML files containing paths with ".." and "."
        // are properly canonicalized before read into the VFS.
        నిండిన_మార్గం = canonicalize(నిండిన_మార్గం);
        External_Contents_మార్గం = నిండిన_మార్గం.str();
      } లేకపోతే ఒకవేళ (కీలకం == "use-external-name") {
        తార్కిక Val;
        ఒకవేళ (!parseScalarBool(I.get_విలువ(), Val))
          తిరిగి శూన్యసూచిక;
        UseExternalName = Val ? RedirectingFileSystem::NK_External
                              : RedirectingFileSystem::NK_Virtual;
      } లేకపోతే {
        llvm_unreachable("key missing from Keys");
      }
    }

    ఒకవేళ (ప్రవాహం.failed())
      తిరిగి శూన్యసూచిక;

    // check for missing keys
    ఒకవేళ (Contents_క్షేత్రం == CF_Not_సమితి) {
      error(N, "missing key 'contents' or 'external-contents'");
      తిరిగి శూన్యసూచిక;
    }
    ఒకవేళ (!check_Missing_కీలకాలు(N, కీలకాలు_2))
      తిరిగి శూన్యసూచిక;

    // check invalid configuration
    ఒకవేళ (రకం_3 == RedirectingFileSystem::EK_Directory &&
        UseExternalName != RedirectingFileSystem::NK_Not_సమితి) {
      error(N, "'use-external-name' is not supported for 'directory' entries");
      తిరిగి శూన్యసూచిక;
    }

    ఒకవేళ (రకం_3 == RedirectingFileSystem::EK_DirectoryRemap &&
        Contents_క్షేత్రం == CF_List) {
      error(N, "'contents' is not supported for 'directory-remap' entries");
      తిరిగి శూన్యసూచిక;
    }

    sys::మార్గం_2::Style మార్గం_style = sys::మార్గం_2::Style::native;
    ఒకవేళ (Is_మూలం_నమోదు) {
      // VFS root entries may be in either Posix or Windows style.  Figure out
      // which style we have, and use it consistently.
      ఒకవేళ (sys::మార్గం_2::is_absolute(Name, sys::మార్గం_2::Style::posix)) {
        మార్గం_style = sys::మార్గం_2::Style::posix;
      } లేకపోతే ఒకవేళ (sys::మార్గం_2::is_absolute(Name,
                                        sys::మార్గం_2::Style::windows_backslash)) {
        మార్గం_style = sys::మార్గం_2::Style::windows_backslash;
      } లేకపోతే {
        // Relative VFS root entries are made absolute to either the overlay
        // directory, or the current working directory, then we can determine
        // the path style from that.
        std::error_సంకేతం EC;
        ఒకవేళ (FS->మూలం_Relative ==
            RedirectingFileSystem::మూలం_Relative_రకం::OverlayDir) {
          StringRef నిండిన_మార్గం = FS->getOverlayFileDir();
          ప్రతిజ్ఞా(!నిండిన_మార్గం.ఖాళీలు() && "Overlay file directory must exist");
          EC = FS->makeAbsolute(నిండిన_మార్గం, Name);
          Name = canonicalize(Name);
        } లేకపోతే {
          EC = FS->makeAbsolute(Name);
        }
        ఒకవేళ (EC) {
          ప్రతిజ్ఞా(Name_విలువ_కణుపు && "Name presence should be checked earlier");
          error(
              Name_విలువ_కణుపు,
              "entry with relative path at the root level is not discoverable");
          తిరిగి శూన్యసూచిక;
        }
        మార్గం_style = sys::మార్గం_2::is_absolute(Name, sys::మార్గం_2::Style::posix)
                         ? sys::మార్గం_2::Style::posix
                         : sys::మార్గం_2::Style::windows_backslash;
      }
      // is::path::is_absolute(Name, sys::path::Style::windows_backslash) will
      // return true even if `Name` is using forward slashes. Distinguish
      // between windows_backslash and windows_slash.
      ఒకవేళ (మార్గం_style == sys::మార్గం_2::Style::windows_backslash &&
          getExistingStyle(Name) != sys::మార్గం_2::Style::windows_backslash)
        మార్గం_style = sys::మార్గం_2::Style::windows_slash;
    }

    // Remove trailing slash(es), being careful not to remove the root path
    StringRef Trimmed = Name;
    size_t మూలం_మార్గం_Len = sys::మార్గం_2::మూలం_మార్గం_2(Trimmed, మార్గం_style).కొలత_2();
    అయితే (Trimmed.కొలత_2() > మూలం_మార్గం_Len &&
           sys::మార్గం_2::is_separator(Trimmed.వెనుక(), మార్గం_style))
      Trimmed = Trimmed.ముక్క(0, Trimmed.కొలత_2() - 1);

    // Get the last component
    StringRef చివరి_Component = sys::మార్గం_2::filename(Trimmed, మార్గం_style);

    std::unique_ptr<RedirectingFileSystem::నమోదు_2> ఫలితం;
    ఎంపిక (రకం_3) {
    స్థితి RedirectingFileSystem::EK_File:
      ఫలితం = std::make_unique<RedirectingFileSystem::File_నమోదు>(
          చివరి_Component, std::move(External_Contents_మార్గం), UseExternalName);
      ఆపు;
    స్థితి RedirectingFileSystem::EK_DirectoryRemap:
      ఫలితం = std::make_unique<RedirectingFileSystem::Directory_Remap_నమోదు>(
          చివరి_Component, std::move(External_Contents_మార్గం), UseExternalName);
      ఆపు;
    స్థితి RedirectingFileSystem::EK_Directory:
      ఫలితం = std::make_unique<RedirectingFileSystem::Directory_నమోదు>(
          చివరి_Component, std::move(నమోదు_Array_Contents),
          స్థితి_2("", get_తదుపరి_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
      ఆపు;
    }

    StringRef మాతృ_2 = sys::మార్గం_2::మాతృ_మార్గం(Trimmed, మార్గం_style);
    ఒకవేళ (మాతృ_2.ఖాళీలు())
      తిరిగి ఫలితం;

    // if 'name' contains multiple components, create implicit directory entries
    కొరకు (sys::మార్గం_2::తిరగేయి_iterator I = sys::మార్గం_2::rbegin(మాతృ_2, మార్గం_style),
                                     E = sys::మార్గం_2::rend(మాతృ_2);
         I != E; ++I) {
      std::సదిశ<std::unique_ptr<RedirectingFileSystem::నమోదు_2>> Entries;
      Entries.నెట్టు_వెనుక(std::move(ఫలితం));
      ఫలితం = std::make_unique<RedirectingFileSystem::Directory_నమోదు>(
          *I, std::move(Entries),
          స్థితి_2("", get_తదుపరి_Virtual_Unique_ID(), std::chrono::system_clock::now(),
                 0, 0, 0, file_type::directory_file, sys::fs::all_all));
    }
    తిరిగి ఫలితం;
  }

బహిరంగ:
  RedirectingFileSystemParser(yaml::ప్రవాహం &S) : ప్రవాహం(S) {}

  // false on error
  తార్కిక parse(yaml::కణుపు *మూలం_2, RedirectingFileSystem *FS) {
    స్వయంచాలక *శిఖరం = dyn_cast<yaml::Mapping_కణుపు>(మూలం_2);
    ఒకవేళ (!శిఖరం) {
      error(మూలం_2, "expected mapping node");
      తిరిగి అబద్ధం;
    }

    కీలకం_స్థితి_Pair Fields[] = {
        కీలకం_స్థితి_Pair("version", నిజం),
        కీలకం_స్థితి_Pair("case-sensitive", అబద్ధం),
        కీలకం_స్థితి_Pair("use-external-names", అబద్ధం),
        కీలకం_స్థితి_Pair("root-relative", అబద్ధం),
        కీలకం_స్థితి_Pair("overlay-relative", అబద్ధం),
        కీలకం_స్థితి_Pair("fallthrough", అబద్ధం),
        కీలకం_స్థితి_Pair("redirecting-with", అబద్ధం),
        కీలకం_స్థితి_Pair("roots", నిజం),
    };

    DenseMap<StringRef, కీలకం_స్థితి> కీలకాలు_2(std::begin(Fields), std::చివర(Fields));
    std::సదిశ<std::unique_ptr<RedirectingFileSystem::నమోదు_2>> మూలం_Entries;

    // Parse configuration and 'roots'
    కొరకు (స్వయంచాలక &I : *శిఖరం) {
      SmallString<10> కీలకం_బఫరు;
      StringRef కీలకం;
      ఒకవేళ (!parseScalarString(I.get_కీలకం(), కీలకం, కీలకం_బఫరు))
        తిరిగి అబద్ధం;

      ఒకవేళ (!check_Duplicate_Or_Unknown_కీలకం(I.get_కీలకం(), కీలకం, కీలకాలు_2))
        తిరిగి అబద్ధం;

      ఒకవేళ (కీలకం == "roots") {
        స్వయంచాలక *Roots = dyn_cast_if_present<yaml::Sequence_కణుపు>(I.get_విలువ());
        ఒకవేళ (!Roots) {
          error(I.get_విలువ(), "expected array");
          తిరిగి అబద్ధం;
        }

        కొరకు (స్వయంచాలక &I : *Roots) {
          ఒకవేళ (std::unique_ptr<RedirectingFileSystem::నమోదు_2> E =
                  parse_నమోదు(&I, FS, /*IsRootEntry*/ నిజం))
            మూలం_Entries.నెట్టు_వెనుక(std::move(E));
          లేకపోతే
            తిరిగి అబద్ధం;
        }
      } లేకపోతే ఒకవేళ (కీలకం == "version") {
        StringRef సంస్కరణ_String;
        SmallString<4> Storage;
        ఒకవేళ (!parseScalarString(I.get_విలువ(), సంస్కరణ_String, Storage))
          తిరిగి అబద్ధం;
        పూర్ణాంకం సంస్కరణ;
        ఒకవేళ (సంస్కరణ_String.getAsInteger<పూర్ణాంకం>(10, సంస్కరణ)) {
          error(I.get_విలువ(), "expected integer");
          తిరిగి అబద్ధం;
        }
        ఒకవేళ (సంస్కరణ < 0) {
          error(I.get_విలువ(), "invalid version number");
          తిరిగి అబద్ధం;
        }
        ఒకవేళ (సంస్కరణ != 0) {
          error(I.get_విలువ(), "version mismatch, expected 0");
          తిరిగి అబద్ధం;
        }
      } లేకపోతే ఒకవేళ (కీలకం == "case-sensitive") {
        ఒకవేళ (!parseScalarBool(I.get_విలువ(), FS->CaseSensitive))
          తిరిగి అబద్ధం;
      } లేకపోతే ఒకవేళ (కీలకం == "overlay-relative") {
        ఒకవేళ (!parseScalarBool(I.get_విలువ(), FS->IsRelativeOverlay))
          తిరిగి అబద్ధం;
      } లేకపోతే ఒకవేళ (కీలకం == "use-external-names") {
        ఒకవేళ (!parseScalarBool(I.get_విలువ(), FS->Use_External_పేర్లు))
          తిరిగి అబద్ధం;
      } లేకపోతే ఒకవేళ (కీలకం == "fallthrough") {
        ఒకవేళ (కీలకాలు_2["redirecting-with"].చూసిన) {
          error(I.get_విలువ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          తిరిగి అబద్ధం;
        }

        తార్కిక ShouldFallthrough = అబద్ధం;
        ఒకవేళ (!parseScalarBool(I.get_విలువ(), ShouldFallthrough))
          తిరిగి అబద్ధం;

        ఒకవేళ (ShouldFallthrough) {
          FS->Redirection = RedirectingFileSystem::Redirect_రకం::Fallthrough;
        } లేకపోతే {
          FS->Redirection = RedirectingFileSystem::Redirect_రకం::RedirectOnly;
        }
      } లేకపోతే ఒకవేళ (కీలకం == "redirecting-with") {
        ఒకవేళ (కీలకాలు_2["fallthrough"].చూసిన) {
          error(I.get_విలువ(),
                "'fallthrough' and 'redirecting-with' are mutually exclusive");
          తిరిగి అబద్ధం;
        }

        ఒకవేళ (స్వయంచాలక రకం_3 = parse_Redirect_రకం(I.get_విలువ())) {
          FS->Redirection = *రకం_3;
        } లేకపోతే {
          error(I.get_విలువ(), "expected valid redirect kind");
          తిరిగి అబద్ధం;
        }
      } లేకపోతే ఒకవేళ (కీలకం == "root-relative") {
        ఒకవేళ (స్వయంచాలక రకం_3 = parse_మూలం_Relative_రకం(I.get_విలువ())) {
          FS->మూలం_Relative = *రకం_3;
        } లేకపోతే {
          error(I.get_విలువ(), "expected valid root-relative kind");
          తిరిగి అబద్ధం;
        }
      } లేకపోతే {
        llvm_unreachable("key missing from Keys");
      }
    }

    ఒకవేళ (ప్రవాహం.failed())
      తిరిగి అబద్ధం;

    ఒకవేళ (!check_Missing_కీలకాలు(శిఖరం, కీలకాలు_2))
      తిరిగి అబద్ధం;

    // Now that we sucessefully parsed the YAML file, canonicalize the internal
    // representation to a proper directory tree so that we can search faster
    // inside the VFS.
    కొరకు (స్వయంచాలక &E : మూలం_Entries)
      unique_Overlay_వృక్షం(FS, E.get());

    తిరిగి నిజం;
  }
};

std::unique_ptr<RedirectingFileSystem>
RedirectingFileSystem::create(std::unique_ptr<జ్ఞాపకం_బఫరు> బఫరు,
                              ఆధారం_Mgr::DiagHandlerTy DiagHandler,
                              StringRef YAMLFile_మార్గం, శూన్య *DiagContext,
                              IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  ఆధారం_Mgr SM;
  yaml::ప్రవాహం ప్రవాహం(బఫరు->get_Mem_బఫరు_Ref(), SM);

  SM.సమితి_Diag_Handler(DiagHandler, DiagContext);
  yaml::document_iterator DI = ప్రవాహం.begin();
  yaml::కణుపు *మూలం_2 = DI->get_మూలం();
  ఒకవేళ (DI == ప్రవాహం.చివర() || !మూలం_2) {
    SM.Print_సందేశం(SMLoc(), ఆధారం_Mgr::DK_Error, "expected root node");
    తిరిగి శూన్యసూచిక;
  }

  RedirectingFileSystemParser P(ప్రవాహం);

  std::unique_ptr<RedirectingFileSystem> FS(
      కొత్త RedirectingFileSystem(ExternalFS));

  ఒకవేళ (!YAMLFile_మార్గం.ఖాళీలు()) {
    // Use the YAML path from -ivfsoverlay to compute the dir to be prefixed
    // to each 'external-contents' path.
    //
    // Example:
    //    -ivfsoverlay dummy.cache/vfs/vfs.yaml
    // yields:
    //  FS->OverlayFileDir => /<absolute_path_to>/dummy.cache/vfs
    //
    SmallString<256> OverlayAbsDir = sys::మార్గం_2::మాతృ_మార్గం(YAMLFile_మార్గం);
    std::error_సంకేతం EC = FS->makeAbsolute(OverlayAbsDir);
    ప్రతిజ్ఞా(!EC && "Overlay dir final path must be absolute");
    (శూన్య)EC;
    FS->సమితి_Overlay_File_Dir(OverlayAbsDir);
  }

  ఒకవేళ (!P.parse(మూలం_2, FS.get()))
    తిరిగి శూన్యసూచిక;

  తిరిగి FS;
}

std::unique_ptr<RedirectingFileSystem> RedirectingFileSystem::create(
    ArrayRef<std::pair<std::string, std::string>> RemappedFiles,
    తార్కిక Use_External_పేర్లు, llvm::IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  std::unique_ptr<RedirectingFileSystem> FS(
      కొత్త RedirectingFileSystem(ExternalFS));
  FS->Use_External_పేర్లు = Use_External_పేర్లు;

  StringMap<RedirectingFileSystem::నమోదు_2 *> Entries;

  కొరకు (స్వయంచాలక &Mapping : llvm::తిరగేయి(RemappedFiles)) {
    SmallString<128> From = StringRef(Mapping.మొదటి_2);
    SmallString<128> గమ్యం = StringRef(Mapping.రెండవ_2);
    {
      స్వయంచాలక EC = ExternalFS->makeAbsolute(From);
      (శూన్య)EC;
      ప్రతిజ్ఞా(!EC && "Could not make absolute path");
    }

    // Check if we've already mapped this file. The first one we see (in the
    // reverse iteration) wins.
    RedirectingFileSystem::నమోదు_2 *&గమ్యం_నమోదు = Entries[From];
    ఒకవేళ (గమ్యం_నమోదు)
      కొనసాగు;

    // Add parent directories.
    RedirectingFileSystem::నమోదు_2 *మాతృ_2 = శూన్యసూచిక;
    StringRef FromDirectory = llvm::sys::మార్గం_2::మాతృ_మార్గం(From);
    కొరకు (స్వయంచాలక I = llvm::sys::మార్గం_2::begin(FromDirectory),
              E = llvm::sys::మార్గం_2::చివర(FromDirectory);
         I != E; ++I) {
      మాతృ_2 = RedirectingFileSystemParser::lookup_Or_Create_నమోదు(FS.get(), *I,
                                                                మాతృ_2);
    }
    ప్రతిజ్ఞా(మాతృ_2 && "File without a directory?");
    {
      స్వయంచాలక EC = ExternalFS->makeAbsolute(గమ్యం);
      (శూన్య)EC;
      ప్రతిజ్ఞా(!EC && "Could not make absolute path");
    }

    // Add the file.
    స్వయంచాలక NewFile = std::make_unique<RedirectingFileSystem::File_నమోదు>(
        llvm::sys::మార్గం_2::filename(From), గమ్యం,
        Use_External_పేర్లు ? RedirectingFileSystem::NK_External
                         : RedirectingFileSystem::NK_Virtual);
    గమ్యం_నమోదు = NewFile.get();
    cast<RedirectingFileSystem::Directory_నమోదు>(మాతృ_2)->కూడు_Content(
        std::move(NewFile));
  }

  తిరిగి FS;
}

RedirectingFileSystem::Lookup_ఫలితం::Lookup_ఫలితం(
    నమోదు_2 *E, sys::మార్గం_2::const_iterator ప్రారంభం, sys::మార్గం_2::const_iterator చివర_2)
    : E(E) {
  ప్రతిజ్ఞా(E != శూన్యసూచిక);
  // If the matched entry is a DirectoryRemapEntry, set ExternalRedirect to the
  // path of the directory it maps to in the external file system plus any
  // remaining path components in the provided iterator.
  ఒకవేళ (స్వయంచాలక *DRE = dyn_cast<RedirectingFileSystem::Directory_Remap_నమోదు>(E)) {
    SmallString<256> Redirect(DRE->get_External_Contents_మార్గం());
    sys::మార్గం_2::append(Redirect, ప్రారంభం, చివర_2,
                      getExistingStyle(DRE->get_External_Contents_మార్గం()));
    ExternalRedirect = std::string(Redirect);
  }
}

శూన్య RedirectingFileSystem::Lookup_ఫలితం::get_మార్గం(
    llvm::Small_సదిశ_Impl<అక్షరం> &ఫలితం) స్థిరరాశి {
  ఫలితం.clear();
  కొరకు (నమోదు_2 *మాతృ_2 : Parents)
    llvm::sys::మార్గం_2::append(ఫలితం, మాతృ_2->getName());
  llvm::sys::మార్గం_2::append(ఫలితం, E->getName());
}

std::error_సంకేతం RedirectingFileSystem::makeCanonicalForLookup(
    Small_సదిశ_Impl<అక్షరం> &మార్గం) స్థిరరాశి {
  ఒకవేళ (std::error_సంకేతం EC = makeAbsolute(మార్గం))
    తిరిగి EC;

  llvm::SmallString<256> Canonical_మార్గం =
      canonicalize(StringRef(మార్గం.దత్తాంశం(), మార్గం.కొలత_2()));
  ఒకవేళ (Canonical_మార్గం.ఖాళీలు())
    తిరిగి make_error_సంకేతం(llvm::errc::invalid_వాదం);

  మార్గం.కేటాయించు(Canonical_మార్గం.begin(), Canonical_మార్గం.చివర());
  తిరిగి {};
}

ErrorOr<RedirectingFileSystem::Lookup_ఫలితం>
RedirectingFileSystem::lookup_మార్గం(StringRef మార్గం) స్థిరరాశి {
  llvm::SmallString<128> Canonical_మార్గం(మార్గం);
  ఒకవేళ (std::error_సంకేతం EC = makeCanonicalForLookup(Canonical_మార్గం))
    తిరిగి EC;

  // RedirectOnly means the VFS is always used.
  ఒకవేళ (Usage_Tracking_చురుకైన && Redirection == Redirect_రకం::RedirectOnly)
    Has_Been_వాడిన = నిజం;

  sys::మార్గం_2::const_iterator ప్రారంభం = sys::మార్గం_2::begin(Canonical_మార్గం);
  sys::మార్గం_2::const_iterator చివర_2 = sys::మార్గం_2::చివర(Canonical_మార్గం);
  llvm::Small_సదిశ<నమోదు_2 *, 32> Entries;
  కొరకు (స్థిరరాశి స్వయంచాలక &మూలం_2 : Roots) {
    ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> ఫలితం =
        lookup_మార్గం_Impl(ప్రారంభం, చివర_2, మూలం_2.get(), Entries);
    ఒకవేళ (Usage_Tracking_చురుకైన && ఫలితం && isa<Remap_నమోదు>(ఫలితం->E))
      Has_Been_వాడిన = నిజం;
    ఒకవేళ (ఫలితం) {
      ఫలితం->Parents = std::move(Entries);
      తిరిగి ఫలితం;
    }

    ఒకవేళ (ఫలితం.getError() != llvm::errc::no_such_file_or_directory)
      తిరిగి ఫలితం;
  }
  తిరిగి make_error_సంకేతం(llvm::errc::no_such_file_or_directory);
}

ErrorOr<RedirectingFileSystem::Lookup_ఫలితం>
RedirectingFileSystem::lookup_మార్గం_Impl(
    sys::మార్గం_2::const_iterator ప్రారంభం, sys::మార్గం_2::const_iterator చివర_2,
    RedirectingFileSystem::నమోదు_2 *From,
    llvm::Small_సదిశ_Impl<నమోదు_2 *> &Entries) స్థిరరాశి {
  ప్రతిజ్ఞా(!isTraversalComponent(*ప్రారంభం) &&
         !isTraversalComponent(From->getName()) &&
         "Paths should not contain traversal components");

  StringRef FromName = From->getName();

  // Forward the search to the next component in case this is an empty one.
  ఒకవేళ (!FromName.ఖాళీలు()) {
    ఒకవేళ (!మార్గం_Component_Matches(*ప్రారంభం, FromName))
      తిరిగి make_error_సంకేతం(llvm::errc::no_such_file_or_directory);

    ++ప్రారంభం;

    ఒకవేళ (ప్రారంభం == చివర_2) {
      // Match!
      తిరిగి Lookup_ఫలితం(From, ప్రారంభం, చివర_2);
    }
  }

  ఒకవేళ (isa<RedirectingFileSystem::File_నమోదు>(From))
    తిరిగి make_error_సంకేతం(llvm::errc::not_a_directory);

  ఒకవేళ (isa<RedirectingFileSystem::Directory_Remap_నమోదు>(From))
    తిరిగి Lookup_ఫలితం(From, ప్రారంభం, చివర_2);

  స్వయంచాలక *DE = cast<RedirectingFileSystem::Directory_నమోదు>(From);
  కొరకు (స్థిరరాశి std::unique_ptr<RedirectingFileSystem::నమోదు_2> &Dir_నమోదు :
       llvm::make_range(DE->contents_begin(), DE->contents_చివర())) {
    Entries.నెట్టు_వెనుక(From);
    ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> ఫలితం =
        lookup_మార్గం_Impl(ప్రారంభం, చివర_2, Dir_నమోదు.get(), Entries);
    ఒకవేళ (ఫలితం || ఫలితం.getError() != llvm::errc::no_such_file_or_directory)
      తిరిగి ఫలితం;
    Entries.తీయి_వెనుక();
  }

  తిరిగి make_error_సంకేతం(llvm::errc::no_such_file_or_directory);
}

స్థిర స్థితి_2 get_Redirected_File_స్థితి(స్థిరరాశి Twine &Original_మార్గం,
                                      తార్కిక Use_External_పేర్లు,
                                      స్థితి_2 External_స్థితి) {
  // The path has been mapped by some nested VFS and exposes an external path,
  // don't override it with the original path.
  ఒకవేళ (External_స్థితి.ExposesExternalVFSPath)
    తిరిగి External_స్థితి;

  స్థితి_2 S = External_స్థితి;
  ఒకవేళ (!Use_External_పేర్లు)
    S = స్థితి_2::copyWithNewName(S, Original_మార్గం);
  లేకపోతే
    S.ExposesExternalVFSPath = నిజం;
  తిరిగి S;
}

ErrorOr<స్థితి_2> RedirectingFileSystem::స్థితి_3(
    స్థిరరాశి Twine &Lookup_మార్గం, స్థిరరాశి Twine &Original_మార్గం,
    స్థిరరాశి RedirectingFileSystem::Lookup_ఫలితం &ఫలితం) {
  ఒకవేళ (std::optional<StringRef> ExtRedirect = ఫలితం.getExternalRedirect()) {
    SmallString<256> Remapped_మార్గం((*ExtRedirect).str());
    ఒకవేళ (std::error_సంకేతం EC = makeAbsolute(Remapped_మార్గం))
      తిరిగి EC;

    ErrorOr<స్థితి_2> S = ExternalFS->స్థితి_3(Remapped_మార్గం);
    ఒకవేళ (!S)
      తిరిగి S;
    S = స్థితి_2::copyWithNewName(*S, *ExtRedirect);
    స్వయంచాలక *RE = cast<RedirectingFileSystem::Remap_నమోదు>(ఫలితం.E);
    తిరిగి get_Redirected_File_స్థితి(Original_మార్గం,
                                   RE->useExternalName(Use_External_పేర్లు), *S);
  }

  స్వయంచాలక *DE = cast<RedirectingFileSystem::Directory_నమోదు>(ఫలితం.E);
  తిరిగి స్థితి_2::copyWithNewName(DE->get_స్థితి(), Lookup_మార్గం);
}

ErrorOr<స్థితి_2>
RedirectingFileSystem::get_External_స్థితి(స్థిరరాశి Twine &Lookup_మార్గం,
                                         స్థిరరాశి Twine &Original_మార్గం) స్థిరరాశి {
  స్వయంచాలక ఫలితం = ExternalFS->స్థితి_3(Lookup_మార్గం);

  // The path has been mapped by some nested VFS, don't override it with the
  // original path.
  ఒకవేళ (!ఫలితం || ఫలితం->ExposesExternalVFSPath)
    తిరిగి ఫలితం;
  తిరిగి స్థితి_2::copyWithNewName(ఫలితం.get(), Original_మార్గం);
}

ErrorOr<స్థితి_2> RedirectingFileSystem::స్థితి_3(స్థిరరాశి Twine &Original_మార్గం) {
  SmallString<256> మార్గం;
  Original_మార్గం.గమ్యం_సదిశ(మార్గం);

  ఒకవేళ (std::error_సంకేతం EC = makeAbsolute(మార్గం))
    తిరిగి EC;

  ఒకవేళ (Redirection == Redirect_రకం::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ErrorOr<స్థితి_2> S = get_External_స్థితి(మార్గం, Original_మార్గం);
    ఒకవేళ (S)
      తిరిగి S;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> ఫలితం = lookup_మార్గం(మార్గం);
  ఒకవేళ (!ఫలితం) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ఒకవేళ (Redirection == Redirect_రకం::Fallthrough &&
        is_File_Not_దొరికింది(ఫలితం.getError()))
      తిరిగి get_External_స్థితి(మార్గం, Original_మార్గం);
    తిరిగి ఫలితం.getError();
  }

  ErrorOr<స్థితి_2> S = స్థితి_3(మార్గం, Original_మార్గం, *ఫలితం);
  ఒకవేళ (!S && Redirection == Redirect_రకం::Fallthrough &&
      is_File_Not_దొరికింది(S.getError(), ఫలితం->E)) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    తిరిగి get_External_స్థితి(మార్గం, Original_మార్గం);
  }

  తిరిగి S;
}

తార్కిక RedirectingFileSystem::exists(స్థిరరాశి Twine &Original_మార్గం) {
  SmallString<256> మార్గం;
  Original_మార్గం.గమ్యం_సదిశ(మార్గం);

  ఒకవేళ (makeAbsolute(మార్గం))
    తిరిగి అబద్ధం;

  ఒకవేళ (Redirection == Redirect_రకం::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    ఒకవేళ (ExternalFS->exists(మార్గం))
      తిరిగి నిజం;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> ఫలితం = lookup_మార్గం(మార్గం);
  ఒకవేళ (!ఫలితం) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ఒకవేళ (Redirection == Redirect_రకం::Fallthrough &&
        is_File_Not_దొరికింది(ఫలితం.getError()))
      తిరిగి ExternalFS->exists(మార్గం);
    తిరిగి అబద్ధం;
  }

  std::optional<StringRef> ExtRedirect = ఫలితం->getExternalRedirect();
  ఒకవేళ (!ExtRedirect) {
    ప్రతిజ్ఞా(isa<RedirectingFileSystem::Directory_నమోదు>(ఫలితం->E));
    తిరిగి నిజం;
  }

  SmallString<256> Remapped_మార్గం((*ExtRedirect).str());
  ఒకవేళ (makeAbsolute(Remapped_మార్గం))
    తిరిగి అబద్ధం;

  ఒకవేళ (ExternalFS->exists(Remapped_మార్గం))
    తిరిగి నిజం;

  ఒకవేళ (Redirection == Redirect_రకం::Fallthrough) {
    // Mapped the file but it wasn't found in the underlying filesystem,
    // fallthrough to using the original path if that was the specified
    // redirection type.
    తిరిగి ExternalFS->exists(మార్గం);
  }

  తిరిగి అబద్ధం;
}

నామాకాశ {

/// Provide a file wrapper with an overriden status.
తరగతి File_With_Fixed_స్థితి : బహిరంగ File {
  std::unique_ptr<File> InnerFile;
  స్థితి_2 S;

బహిరంగ:
  File_With_Fixed_స్థితి(std::unique_ptr<File> InnerFile, స్థితి_2 S)
      : InnerFile(std::move(InnerFile)), S(std::move(S)) {}

  ErrorOr<స్థితి_2> స్థితి_3() override { తిరిగి S; }
  ErrorOr<std::unique_ptr<llvm::జ్ఞాపకం_బఫరు>>

  get_బఫరు(స్థిరరాశి Twine &Name, int64_t File_కొలత, తార్కిక RequiresNullTerminator,
            తార్కిక IsVolatile) override {
    తిరిగి InnerFile->get_బఫరు(Name, File_కొలత, RequiresNullTerminator,
                                IsVolatile);
  }

  std::error_సంకేతం close() override { తిరిగి InnerFile->close(); }

  శూన్య సమితి_మార్గం(స్థిరరాశి Twine &మార్గం) override { S = S.copyWithNewName(S, మార్గం); }
};

} // namespace

ErrorOr<std::unique_ptr<File>>
File::get_With_మార్గం(ErrorOr<std::unique_ptr<File>> ఫలితం, స్థిరరాశి Twine &P) {
  // See \c getRedirectedFileStatus - don't update path if it's exposing an
  // external path.
  ఒకవేళ (!ఫలితం || (*ఫలితం)->స్థితి_3()->ExposesExternalVFSPath)
    తిరిగి ఫలితం;

  ErrorOr<std::unique_ptr<File>> F = std::move(*ఫలితం);
  స్వయంచాలక Name = F->get()->getName();
  ఒకవేళ (Name && Name.get() != P.str())
    F->get()->సమితి_మార్గం(P);
  తిరిగి F;
}

ErrorOr<std::unique_ptr<File>>
RedirectingFileSystem::openFileForRead(స్థిరరాశి Twine &Original_మార్గం) {
  SmallString<256> మార్గం;
  Original_మార్గం.గమ్యం_సదిశ(మార్గం);

  ఒకవేళ (std::error_సంకేతం EC = makeAbsolute(మార్గం))
    తిరిగి EC;

  ఒకవేళ (Redirection == Redirect_రకం::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    స్వయంచాలక F = File::get_With_మార్గం(ExternalFS->openFileForRead(మార్గం), Original_మార్గం);
    ఒకవేళ (F)
      తిరిగి F;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> ఫలితం = lookup_మార్గం(మార్గం);
  ఒకవేళ (!ఫలితం) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ఒకవేళ (Redirection == Redirect_రకం::Fallthrough &&
        is_File_Not_దొరికింది(ఫలితం.getError()))
      తిరిగి File::get_With_మార్గం(ExternalFS->openFileForRead(మార్గం), Original_మార్గం);
    తిరిగి ఫలితం.getError();
  }

  ఒకవేళ (!ఫలితం->getExternalRedirect()) // FIXME: errc::not_a_file?
    తిరిగి make_error_సంకేతం(llvm::errc::invalid_వాదం);

  StringRef ExtRedirect = *ఫలితం->getExternalRedirect();
  SmallString<256> Remapped_మార్గం(ExtRedirect.str());
  ఒకవేళ (std::error_సంకేతం EC = makeAbsolute(Remapped_మార్గం))
    తిరిగి EC;

  స్వయంచాలక *RE = cast<RedirectingFileSystem::Remap_నమోదు>(ఫలితం->E);

  స్వయంచాలక ExternalFile =
      File::get_With_మార్గం(ExternalFS->openFileForRead(Remapped_మార్గం), ExtRedirect);
  ఒకవేళ (!ExternalFile) {
    ఒకవేళ (Redirection == Redirect_రకం::Fallthrough &&
        is_File_Not_దొరికింది(ExternalFile.getError(), ఫలితం->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      తిరిగి File::get_With_మార్గం(ExternalFS->openFileForRead(మార్గం), Original_మార్గం);
    }
    తిరిగి ExternalFile;
  }

  స్వయంచాలక External_స్థితి = (*ExternalFile)->స్థితి_3();
  ఒకవేళ (!External_స్థితి)
    తిరిగి External_స్థితి.getError();

  // Otherwise, the file was successfully remapped. Mark it as such. Also
  // replace the underlying path if the external name is being used.
  స్థితి_2 S = get_Redirected_File_స్థితి(
      Original_మార్గం, RE->useExternalName(Use_External_పేర్లు), *External_స్థితి);
  తిరిగి std::unique_ptr<File>(
      std::make_unique<File_With_Fixed_స్థితి>(std::move(*ExternalFile), S));
}

std::error_సంకేతం
RedirectingFileSystem::get_Real_మార్గం(స్థిరరాశి Twine &Original_మార్గం,
                                   Small_సదిశ_Impl<అక్షరం> &Output) {
  SmallString<256> మార్గం;
  Original_మార్గం.గమ్యం_సదిశ(మార్గం);

  ఒకవేళ (std::error_సంకేతం EC = makeAbsolute(మార్గం))
    తిరిగి EC;

  ఒకవేళ (Redirection == Redirect_రకం::Fallback) {
    // Attempt to find the original file first, only falling back to the
    // mapped file if that fails.
    std::error_సంకేతం EC = ExternalFS->get_Real_మార్గం(మార్గం, Output);
    ఒకవేళ (!EC)
      తిరిగి EC;
  }

  ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> ఫలితం = lookup_మార్గం(మార్గం);
  ఒకవేళ (!ఫలితం) {
    // Was not able to map file, fallthrough to using the original path if
    // that was the specified redirection type.
    ఒకవేళ (Redirection == Redirect_రకం::Fallthrough &&
        is_File_Not_దొరికింది(ఫలితం.getError()))
      తిరిగి ExternalFS->get_Real_మార్గం(మార్గం, Output);
    తిరిగి ఫలితం.getError();
  }

  // If we found FileEntry or DirectoryRemapEntry, look up the mapped
  // path in the external file system.
  ఒకవేళ (స్వయంచాలక ExtRedirect = ఫలితం->getExternalRedirect()) {
    స్వయంచాలక P = ExternalFS->get_Real_మార్గం(*ExtRedirect, Output);
    ఒకవేళ (P && Redirection == Redirect_రకం::Fallthrough &&
        is_File_Not_దొరికింది(P, ఫలితం->E)) {
      // Mapped the file but it wasn't found in the underlying filesystem,
      // fallthrough to using the original path if that was the specified
      // redirection type.
      తిరిగి ExternalFS->get_Real_మార్గం(మార్గం, Output);
    }
    తిరిగి P;
  }

  // We found a DirectoryEntry, which does not have a single external contents
  // path. Use the canonical virtual path.
  ఒకవేళ (Redirection == Redirect_రకం::Fallthrough) {
    ఫలితం->get_మార్గం(Output);
    తిరిగి {};
  }
  తిరిగి llvm::errc::invalid_వాదం;
}

శూన్య RedirectingFileSystem::getDirectoryContentRealSources(
    స్థిరరాశి Twine &Dir, Small_సదిశ_Impl<std::string> &బహిర్గతం) {
  SmallString<256> మార్గం;
  Dir.గమ్యం_సదిశ(మార్గం);

  ఒకవేళ (makeAbsolute(మార్గం))
    తిరిగి;

  // Fallthrough and Fallback both consult ExternalFS, differing only in order.
  స్థిరరాశి తార్కిక ConsultsExternalFS = Redirection != Redirect_రకం::RedirectOnly;

  ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> ఫలితం = lookup_మార్గం(మార్గం);
  ఒకవేళ (!ఫలితం) {
    // dir_begin() delegates entirely to ExternalFS.
    ఒకవేళ (ConsultsExternalFS)
      ExternalFS->getDirectoryContentRealSources(మార్గం, బహిర్గతం);
    తిరిగి;
  }

  ఎంపిక (ఫలితం->E->get_రకం()) {
  స్థితి EK_File:
    తిరిగి;
  స్థితి EK_Directory:
    // The names come from the overlay, which has no location in ExternalFS.
    ఆపు;
  స్థితి EK_DirectoryRemap: {
    // Make the target absolute as status() does, so a relative
    // 'external-contents' is not passed down raw.
    SmallString<256> Remapped_మార్గం(*ఫలితం->getExternalRedirect());
    ఒకవేళ (!makeAbsolute(Remapped_మార్గం))
      ExternalFS->getDirectoryContentRealSources(Remapped_మార్గం, బహిర్గతం);
    ఆపు;
  }
  }

  // dir_begin() also iterates the original path in ExternalFS.
  ఒకవేళ (ConsultsExternalFS)
    ExternalFS->getDirectoryContentRealSources(మార్గం, బహిర్గతం);
}

std::unique_ptr<FileSystem>
vfs::getVFSFromYAML(std::unique_ptr<జ్ఞాపకం_బఫరు> బఫరు,
                    ఆధారం_Mgr::DiagHandlerTy DiagHandler,
                    StringRef YAMLFile_మార్గం, శూన్య *DiagContext,
                    IntrusiveRefCntPtr<FileSystem> ExternalFS) {
  తిరిగి RedirectingFileSystem::create(std::move(బఫరు), DiagHandler,
                                       YAMLFile_మార్గం, DiagContext,
                                       std::move(ExternalFS));
}

స్థిర శూన్య getVFSEntries(RedirectingFileSystem::నమోదు_2 *SrcE,
                          Small_సదిశ_Impl<StringRef> &మార్గం,
                          Small_సదిశ_Impl<YAMLVFSEntry> &Entries) {
  స్వయంచాలక రకం_3 = SrcE->get_రకం();
  ఒకవేళ (రకం_3 == RedirectingFileSystem::EK_Directory) {
    స్వయంచాలక *DE = dyn_cast<RedirectingFileSystem::Directory_నమోదు>(SrcE);
    ప్రతిజ్ఞా(DE && "Must be a directory");
    కొరకు (std::unique_ptr<RedirectingFileSystem::నమోదు_2> &Sub_నమోదు :
         llvm::make_range(DE->contents_begin(), DE->contents_చివర())) {
      మార్గం.నెట్టు_వెనుక(Sub_నమోదు->getName());
      getVFSEntries(Sub_నమోదు.get(), మార్గం, Entries);
      మార్గం.తీయి_వెనుక();
    }
    తిరిగి;
  }

  ఒకవేళ (రకం_3 == RedirectingFileSystem::EK_DirectoryRemap) {
    స్వయంచాలక *DR = dyn_cast<RedirectingFileSystem::Directory_Remap_నమోదు>(SrcE);
    ప్రతిజ్ఞా(DR && "Must be a directory remap");
    SmallString<128> VPath;
    కొరకు (స్వయంచాలక &Comp : మార్గం)
      llvm::sys::మార్గం_2::append(VPath, Comp);
    Entries.నెట్టు_వెనుక(
        YAMLVFSEntry(VPath.c_str(), DR->get_External_Contents_మార్గం()));
    తిరిగి;
  }

  ప్రతిజ్ఞా(రకం_3 == RedirectingFileSystem::EK_File && "Must be a EK_File");
  స్వయంచాలక *FE = dyn_cast<RedirectingFileSystem::File_నమోదు>(SrcE);
  ప్రతిజ్ఞా(FE && "Must be a file");
  SmallString<128> VPath;
  కొరకు (స్వయంచాలక &Comp : మార్గం)
    llvm::sys::మార్గం_2::append(VPath, Comp);
  Entries.నెట్టు_వెనుక(YAMLVFSEntry(VPath.c_str(), FE->get_External_Contents_మార్గం()));
}

శూన్య vfs::collectVFSEntries(RedirectingFileSystem &VFS,
                            Small_సదిశ_Impl<YAMLVFSEntry> &CollectedEntries) {
  ErrorOr<RedirectingFileSystem::Lookup_ఫలితం> మూలం_ఫలితం = VFS.lookup_మార్గం("/");
  ఒకవేళ (!మూలం_ఫలితం)
    తిరిగి;
  Small_సదిశ<StringRef, 8> Components;
  Components.నెట్టు_వెనుక("/");
  getVFSEntries(మూలం_ఫలితం->E, Components, CollectedEntries);
}

UniqueID vfs::get_తదుపరి_Virtual_Unique_ID() {
  స్థిర std::atomic<గుర్తులేని> UID;
  గుర్తులేని ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  తిరిగి UniqueID(std::numeric_limits<uint64_t>::max(), ID);
}

శూన్య YAMLVFSWriter::కూడు_నమోదు(StringRef Virtual_మార్గం, StringRef Real_మార్గం,
                             తార్కిక IsDirectory) {
  ప్రతిజ్ఞా(sys::మార్గం_2::is_absolute(Virtual_మార్గం) && "virtual path not absolute");
  ప్రతిజ్ఞా(sys::మార్గం_2::is_absolute(Real_మార్గం) && "real path not absolute");
  ప్రతిజ్ఞా(!మార్గం_Has_Traversal(Virtual_మార్గం) && "path traversal is not supported");
  Mappings.emplace_వెనుక(Virtual_మార్గం, Real_మార్గం, IsDirectory);
}

శూన్య YAMLVFSWriter::కూడు_File_Mapping(StringRef Virtual_మార్గం, StringRef Real_మార్గం) {
  కూడు_నమోదు(Virtual_మార్గం, Real_మార్గం, /*IsDirectory=*/అబద్ధం);
}

శూన్య YAMLVFSWriter::కూడు_Directory_Mapping(StringRef Virtual_మార్గం,
                                        StringRef Real_మార్గం) {
  కూడు_నమోదు(Virtual_మార్గం, Real_మార్గం, /*IsDirectory=*/నిజం);
}

నామాకాశ {

తరగతి JSONWriter {
  llvm::raw_ostream &OS;
  Small_సదిశ<StringRef, 16> Dir_దొంతర;

  గుర్తులేని getDirIndent() { తిరిగి 4 * Dir_దొంతర.కొలత_2(); }
  గుర్తులేని getFileIndent() { తిరిగి 4 * (Dir_దొంతర.కొలత_2() + 1); }
  తార్కిక contained_లోపల(StringRef మాతృ_2, StringRef మార్గం);
  StringRef containedPart(StringRef మాతృ_2, StringRef మార్గం);
  శూన్య ప్రారంభం_Directory(StringRef మార్గం);
  శూన్య చివర_Directory();
  శూన్య write_నమోదు(StringRef VPath, StringRef RPath);

బహిరంగ:
  JSONWriter(llvm::raw_ostream &OS) : OS(OS) {}

  శూన్య లిఖ(ArrayRef<YAMLVFSEntry> Entries,
             std::optional<తార్కిక> Use_External_పేర్లు,
             std::optional<తార్కిక> IsCaseSensitive,
             std::optional<తార్కిక> IsOverlayRelative, StringRef OverlayDir);
};

} // namespace

తార్కిక JSONWriter::contained_లోపల(StringRef మాతృ_2, StringRef మార్గం) {
  ఉపయోగించి నామాకాశ llvm::sys;

  // Compare each path component.
  స్వయంచాలక IParent = మార్గం_2::begin(మాతృ_2), EParent = మార్గం_2::చివర(మాతృ_2);
  కొరకు (స్వయంచాలక IChild = మార్గం_2::begin(మార్గం), EChild = మార్గం_2::చివర(మార్గం);
       IParent != EParent && IChild != EChild; ++IParent, ++IChild) {
    ఒకవేళ (*IParent != *IChild)
      తిరిగి అబద్ధం;
  }
  // Have we exhausted the parent path?
  తిరిగి IParent == EParent;
}

StringRef JSONWriter::containedPart(StringRef మాతృ_2, StringRef మార్గం) {
  ప్రతిజ్ఞా(!మాతృ_2.ఖాళీలు());
  ప్రతిజ్ఞా(contained_లోపల(మాతృ_2, మార్గం));
  తిరిగి మార్గం.substr(మాతృ_2.కొలత_2() + 1);
}

శూన్య JSONWriter::ప్రారంభం_Directory(StringRef మార్గం) {
  StringRef Name =
      Dir_దొంతర.ఖాళీలు() ? మార్గం : containedPart(Dir_దొంతర.వెనుక(), మార్గం);
  Dir_దొంతర.నెట్టు_వెనుక(మార్గం);
  గుర్తులేని Indent = getDirIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'directory',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(Name) << "\",\n";
  OS.indent(Indent + 2) << "'contents': [\n";
}

శూన్య JSONWriter::చివర_Directory() {
  గుర్తులేని Indent = getDirIndent();
  OS.indent(Indent + 2) << "]\n";
  OS.indent(Indent) << "}";

  Dir_దొంతర.తీయి_వెనుక();
}

శూన్య JSONWriter::write_నమోదు(StringRef VPath, StringRef RPath) {
  గుర్తులేని Indent = getFileIndent();
  OS.indent(Indent) << "{\n";
  OS.indent(Indent + 2) << "'type': 'file',\n";
  OS.indent(Indent + 2) << "'name': \"" << llvm::yaml::escape(VPath) << "\",\n";
  OS.indent(Indent + 2) << "'external-contents': \""
                        << llvm::yaml::escape(RPath) << "\"\n";
  OS.indent(Indent) << "}";
}

శూన్య JSONWriter::లిఖ(ArrayRef<YAMLVFSEntry> Entries,
                       std::optional<తార్కిక> Use_External_పేర్లు,
                       std::optional<తార్కిక> IsCaseSensitive,
                       std::optional<తార్కిక> IsOverlayRelative,
                       StringRef OverlayDir) {
  ఉపయోగించి నామాకాశ llvm::sys;

  OS << "{\n"
        "  'version': 0,\n";
  ఒకవేళ (IsCaseSensitive)
    OS << "  'case-sensitive': '" << (*IsCaseSensitive ? "true" : "false")
       << "',\n";
  ఒకవేళ (Use_External_పేర్లు)
    OS << "  'use-external-names': '" << (*Use_External_పేర్లు ? "true" : "false")
       << "',\n";
  తార్కిక UseOverlayRelative = అబద్ధం;
  ఒకవేళ (IsOverlayRelative) {
    UseOverlayRelative = *IsOverlayRelative;
    OS << "  'overlay-relative': '" << (UseOverlayRelative ? "true" : "false")
       << "',\n";
  }
  OS << "  'roots': [\n";

  ఒకవేళ (!Entries.ఖాళీలు()) {
    స్థిరరాశి YAMLVFSEntry &నమోదు_2 = Entries.front();

    ప్రారంభం_Directory(
      నమోదు_2.IsDirectory ? నమోదు_2.VPath : మార్గం_2::మాతృ_మార్గం(నమోదు_2.VPath)
    );

    StringRef RPath = నమోదు_2.RPath;
    ఒకవేళ (UseOverlayRelative) {
      ప్రతిజ్ఞా(RPath.starts_with(OverlayDir) &&
             "Overlay dir must be contained in RPath");
      RPath = RPath.substr(OverlayDir.కొలత_2());
    }

    తార్కిక Is_Current_Dir_ఖాళీలు = నిజం;
    ఒకవేళ (!నమోదు_2.IsDirectory) {
      write_నమోదు(మార్గం_2::filename(నమోదు_2.VPath), RPath);
      Is_Current_Dir_ఖాళీలు = అబద్ధం;
    }

    కొరకు (స్థిరరాశి స్వయంచాలక &నమోదు_2 : Entries.ముక్క(1)) {
      StringRef Dir =
          నమోదు_2.IsDirectory ? నమోదు_2.VPath : మార్గం_2::మాతృ_మార్గం(నమోదు_2.VPath);
      ఒకవేళ (Dir == Dir_దొంతర.వెనుక()) {
        ఒకవేళ (!Is_Current_Dir_ఖాళీలు) {
          OS << ",\n";
        }
      } లేకపోతే {
        తార్కిక Is_Dir_Popped_From_దొంతర = అబద్ధం;
        అయితే (!Dir_దొంతర.ఖాళీలు() && !contained_లోపల(Dir_దొంతర.వెనుక(), Dir)) {
          OS << "\n";
          చివర_Directory();
          Is_Dir_Popped_From_దొంతర = నిజం;
        }
        ఒకవేళ (Is_Dir_Popped_From_దొంతర || !Is_Current_Dir_ఖాళీలు) {
          OS << ",\n";
        }
        ప్రారంభం_Directory(Dir);
        Is_Current_Dir_ఖాళీలు = నిజం;
      }
      StringRef RPath = నమోదు_2.RPath;
      ఒకవేళ (UseOverlayRelative) {
        ప్రతిజ్ఞా(RPath.starts_with(OverlayDir) &&
               "Overlay dir must be contained in RPath");
        RPath = RPath.substr(OverlayDir.కొలత_2());
      }
      ఒకవేళ (!నమోదు_2.IsDirectory) {
        write_నమోదు(మార్గం_2::filename(నమోదు_2.VPath), RPath);
        Is_Current_Dir_ఖాళీలు = అబద్ధం;
      }
    }

    అయితే (!Dir_దొంతర.ఖాళీలు()) {
      OS << "\n";
      చివర_Directory();
    }
    OS << "\n";
  }

  OS << "  ]\n"
     << "}\n";
}

శూన్య YAMLVFSWriter::లిఖ(llvm::raw_ostream &OS) {
  llvm::క్రమీకరించు(Mappings, [](స్థిరరాశి YAMLVFSEntry &LHS, స్థిరరాశి YAMLVFSEntry &RHS) {
    తిరిగి LHS.VPath < RHS.VPath;
  });

  JSONWriter(OS).లిఖ(Mappings, Use_External_పేర్లు, IsCaseSensitive,
                       IsOverlayRelative, OverlayDir);
}

vfs::recursive_directory_iterator::recursive_directory_iterator(
    FileSystem &FS_, స్థిరరాశి Twine &మార్గం, std::error_సంకేతం &EC)
    : FS(&FS_) {
  directory_iterator I = FS->dir_begin(మార్గం, EC);
  ఒకవేళ (I != directory_iterator()) {
    State = std::make_shared<detail::RecDirIterState>();
    State->దొంతర.నెట్టు_వెనుక(I);
  }
}

vfs::recursive_directory_iterator &
recursive_directory_iterator::increment(std::error_సంకేతం &EC) {
  ప్రతిజ్ఞా(FS && State && !State->దొంతర.ఖాళీలు() && "incrementing past end");
  ప్రతిజ్ఞా(!State->దొంతర.వెనుక()->మార్గం_2().ఖాళీలు() && "non-canonical end iterator");
  vfs::directory_iterator చివర_2;

  ఒకవేళ (State->Has_No_నెట్టు_Request)
    State->Has_No_నెట్టు_Request = అబద్ధం;
  లేకపోతే {
    ఒకవేళ (State->దొంతర.వెనుక()->type() == sys::fs::file_type::directory_file) {
      vfs::directory_iterator I =
          FS->dir_begin(State->దొంతర.వెనుక()->మార్గం_2(), EC);
      ఒకవేళ (I != చివర_2) {
        State->దొంతర.నెట్టు_వెనుక(I);
        తిరిగి *ఇది;
      }
    }
  }

  అయితే (!State->దొంతర.ఖాళీలు() && State->దొంతర.వెనుక().increment(EC) == చివర_2)
    State->దొంతర.తీయి_వెనుక();

  ఒకవేళ (State->దొంతర.ఖాళీలు())
    State.reset(); // end iterator

  తిరిగి *ఇది;
}

స్థిరరాశి అక్షరం FileSystem::ID = 0;
స్థిరరాశి అక్షరం OverlayFileSystem::ID = 0;
స్థిరరాశి అక్షరం ProxyFileSystem::ID = 0;
స్థిరరాశి అక్షరం లోపల_జ్ఞాపకం_File_System::ID = 0;
స్థిరరాశి అక్షరం RedirectingFileSystem::ID = 0;

గుర్తులేని ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::use_లెక్క(
    స్థిరరాశి FileSystem *FS) {
  తిరిగి FS->Use_లెక్క();
}

శూన్య ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::retain(FileSystem *FS) {
  FS->Retain();
}

శూన్య ::llvm::IntrusiveRefCntPtrInfo<FileSystem>::release(FileSystem *FS) {
  FS->Release();
}
