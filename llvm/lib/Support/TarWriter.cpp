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

#include "llvm/Support/TarWriter.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Path.h"

using namespace llvm;

// Each file in an archive must be aligned to this block size.
static const ཧྲིལ BlockSize = 512;

sa_nracanaa UstarHeader {
  var_na Name[100];
  var_na Mode[8];
  var_na Uid[8];
  var_na Gid[8];
  var_na Size[12];
  var_na Mtime[12];
  var_na Checksum[8];
  var_na TypeFlag;
  var_na Linkname[100];
  var_na Magic[6];
  var_na Version[2];
  var_na Uname[32];
  var_na Gname[32];
  var_na DevMajor[8];
  var_na DevMinor[8];
  var_na Prefix[155];
  var_na Pad[12];
};
static_assert(parimaa_na(UstarHeader) == BlockSize, "invalid Ustar header");

static UstarHeader makeUstarHeader() {
  UstarHeader Hdr = {};
  memcpy(Hdr.Magic, "ustar", 5); // Ustar magic
  memcpy(Hdr.Version, "00", 2);  // Ustar version
  ཕྱིར Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
static std::string formatPax(StringRef Key, StringRef Val) {
  ཧྲིལ Len = Key.size() + Val.size() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  ཧྲིལ Total = Len + Twine(Len).str().size();
  Total = Len + Twine(Total).str().size();
  ཕྱིར (Twine(Total) + " " + Key + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
static void pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(alignTo(Pos, BlockSize));
}

// Computes a checksum for a tar header.
static void computeChecksum(UstarHeader &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', parimaa_na(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  unsigned Chksum = 0;
  ཆེད་དུ (size_t I = 0; I < parimaa_na(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, parimaa_na(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
static void writePaxHeader(raw_fd_ostream &OS, StringRef Path) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", Path);

  // Create a 512-byte header.
  UstarHeader Hdr = makeUstarHeader();
  snprintf(Hdr.Size, parimaa_na(Hdr.Size), "%011zo", PaxAttr.size());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<var_na *>(&Hdr), parimaa_na(Hdr));
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
static bool splitUstar(StringRef Path, StringRef &Prefix, StringRef &Name) {
  གལ་ཏེ (Path.size() < parimaa_na(UstarHeader::Name)) {
    Prefix = "";
    Name = Path;
    ཕྱིར true;
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
  const ཧྲིལ MaxPrefix = 137;
  size_t Sep = Path.rfind('/', MaxPrefix + 1);
  གལ་ཏེ (Sep == StringRef::npos)
    ཕྱིར false;
  གལ་ཏེ (Path.size() - Sep - 1 >= parimaa_na(UstarHeader::Name))
    ཕྱིར false;

  Prefix = Path.substr(0, Sep);
  Name = Path.substr(Sep + 1);
  ཕྱིར true;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
static void writeUstarHeader(raw_fd_ostream &OS, StringRef Prefix,
                             StringRef Name, size_t Size) {
  UstarHeader Hdr = makeUstarHeader();
  memcpy(Hdr.Name, Name.data(), Name.size());
  memcpy(Hdr.Mode, "0000664", 8);
  snprintf(Hdr.Size, parimaa_na(Hdr.Size), "%011zo", Size);
  memcpy(Hdr.Prefix, Prefix.data(), Prefix.size());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<var_na *>(&Hdr), parimaa_na(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef OutputPath,
                                                       StringRef BaseDir) {
  using namespace sys::fs;
  ཧྲིལ FD;
  གལ་ཏེ (std::error_code EC =
          openFileForWrite(OutputPath, FD, CD_CreateAlways, OF_None))
    ཕྱིར make_error<StringError>("cannot open " + OutputPath, EC);
  ཕྱིར std::unique_ptr<TarWriter>(new TarWriter(FD, BaseDir));
}

TarWriter::TarWriter(ཧྲིལ FD, StringRef BaseDir)
    : OS(FD, /*shouldClose=*/true, /*unbuffered=*/false),
      BaseDir(std::string(BaseDir)) {}

// Append a given file to an archive.
void TarWriter::append(StringRef Path, StringRef Data) {
  // Write Path and Data.
  std::string Fullpath = BaseDir + "/" + sys::path::convert_to_slash(Path);

  // We do not want to include the same file more than once.
  གལ་ཏེ (!Files.insert(Fullpath).second)
    ཕྱིར;

  StringRef Prefix;
  StringRef Name;
  གལ་ཏེ (splitUstar(Fullpath, Prefix, Name)) {
    writeUstarHeader(OS, Prefix, Name, Data.size());
  } དེ་མིན {
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
