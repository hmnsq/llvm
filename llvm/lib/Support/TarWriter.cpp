//===-- TarWriter.cpp - Tar archive file creator --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// TarWriter class provides a feature to create a tar archive file.
//
// I put emphasis on simplicity over comprehensiveness when implementing this
// class because we don't need a full-fledged archive file generator in LLVM
// at the moment.
//
// The filename field in the Unix V7 tar header is 100 bytes. Longer filenames
// are stored using the PAX extension. The PAX header is standardized in
// POSIX.1-2001.
//
// The struct definition of UstarHeader is copied from
// https://www.freebsd.org/cgi/man.cgi?query=tar&sektion=5
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/TarWriter.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/Path.h"

using فضاینام llvm;

// Each file in an archive must be aligned to this block size.
ایستا ثابت صحیح BlockSize = 512;

ساختار UstarHeader {
  نویسه Name[100];
  نویسه Mode[8];
  نویسه Uid[8];
  نویسه Gid[8];
  نویسه Size[12];
  نویسه Mtime[12];
  نویسه Checksum[8];
  نویسه TypeFlag;
  نویسه Linkname[100];
  نویسه Magic[6];
  نویسه Version[2];
  نویسه Uname[32];
  نویسه Gname[32];
  نویسه DevMajor[8];
  نویسه DevMinor[8];
  نویسه Prefix[155];
  نویسه Pad[12];
};
static_assert(اندازه(UstarHeader) == BlockSize, "invalid Ustar header");

ایستا UstarHeader makeUstarHeader() {
  UstarHeader Hdr = {};
  memcpy(Hdr.Magic, "ustar", 5); // Ustar magic
  memcpy(Hdr.Version, "00", 2);  // Ustar version
  بازگشت Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
ایستا std::string formatPax(StringRef کلید, StringRef Val) {
  صحیح Len = کلید.size() + Val.size() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  صحیح Total = Len + Twine(Len).str().size();
  Total = Len + Twine(Total).str().size();
  بازگشت (Twine(Total) + " " + کلید + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
ایستا تهی pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(alignTo(Pos, BlockSize));
}

// Computes a checksum for a tar header.
ایستا تهی computeChecksum(UstarHeader &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', اندازه(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  unsigned Chksum = 0;
  برای (size_t I = 0; I < اندازه(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, اندازه(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
ایستا تهی writePaxHeader(raw_fd_ostream &OS, StringRef Path) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", Path);

  // Create a 512-byte header.
  UstarHeader Hdr = makeUstarHeader();
  snprintf(Hdr.Size, اندازه(Hdr.Size), "%011zo", PaxAttr.size());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<نویسه *>(&Hdr), اندازه(Hdr));
  OS << PaxAttr;
  pad(OS);
}

// Path fits in a Ustar header if
//
// - Path is less than 100 characters long, or
// - Path is in the form of "<prefix>/<name>" where <prefix> is less
//   than or equal to 155 characters long and <name> is less than 100
//   characters long. Both <prefix> and <name> can contain extra '/'.
//
// If Path fits in a Ustar header, updates Prefix and Name and returns true.
// Otherwise, returns false.
ایستا منطقی splitUstar(StringRef Path, StringRef &Prefix, StringRef &Name) {
  اگر (Path.size() < اندازه(UstarHeader::Name)) {
    Prefix = "";
    Name = Path;
    بازگشت درست;
  }

  // tar 1.13 and earlier unconditionally look at the tar header interpreted
  // as an 'oldgnu_header', which has an 'isextended' byte at offset 482 in the
  // header, corresponding to offset 137 in the prefix. That's the version of
  // tar in gnuwin, so only use 137 of the 155 bytes in the prefix. This means
  // we'll need a pax header after 237 bytes of path instead of after 255,
  // but in return paths up to 237 bytes work with gnuwin, instead of just
  // 137 bytes of directory + 100 bytes of basename previously.
  // (tar-1.13 also doesn't support pax headers, but in practice all paths in
  // llvm's test suite are short enough for that to not matter.)
  ثابت صحیح MaxPrefix = 137;
  size_t Sep = Path.rfind('/', MaxPrefix + 1);
  اگر (Sep == StringRef::npos)
    بازگشت نادرست;
  اگر (Path.size() - Sep - 1 >= اندازه(UstarHeader::Name))
    بازگشت نادرست;

  Prefix = Path.substr(0, Sep);
  Name = Path.substr(Sep + 1);
  بازگشت درست;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
ایستا تهی writeUstarHeader(raw_fd_ostream &OS, StringRef Prefix,
                             StringRef Name, size_t Size) {
  UstarHeader Hdr = makeUstarHeader();
  memcpy(Hdr.Name, Name.data(), Name.size());
  memcpy(Hdr.Mode, "0000664", 8);
  snprintf(Hdr.Size, اندازه(Hdr.Size), "%011zo", Size);
  memcpy(Hdr.Prefix, Prefix.data(), Prefix.size());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<نویسه *>(&Hdr), اندازه(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef OutputPath,
                                                       StringRef BaseDir) {
  using فضاینام sys::fs;
  صحیح FD;
  اگر (std::error_code EC =
          openFileForWrite(OutputPath, FD, CD_CreateAlways, OF_None))
    بازگشت make_error<StringError>("cannot open " + OutputPath, EC);
  بازگشت std::unique_ptr<TarWriter>(جدید TarWriter(FD, BaseDir));
}

TarWriter::TarWriter(صحیح FD, StringRef BaseDir)
    : OS(FD, /*shouldClose=*/درست, /*unbuffered=*/نادرست),
      BaseDir(std::string(BaseDir)) {}

// Append a given file to an archive.
تهی TarWriter::append(StringRef Path, StringRef Data) {
  // Write Path and Data.
  std::string Fullpath = BaseDir + "/" + sys::path::convert_to_slash(Path);

  // We do not want to include the same file more than once.
  اگر (!Files.درج(Fullpath).second)
    بازگشت;

  StringRef Prefix;
  StringRef Name;
  اگر (splitUstar(Fullpath, Prefix, Name)) {
    writeUstarHeader(OS, Prefix, Name, Data.size());
  } وگرنه {
    writePaxHeader(OS, Fullpath);
    writeUstarHeader(OS, "", "", Data.size());
  }

  OS << Data;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(BlockSize * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
