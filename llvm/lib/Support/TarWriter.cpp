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

#ಅನ್ತರ್ಭಾವ "llvm/Support/TarWriter.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"

using ನಾಮಾಕಾಶ llvm;

// Each file in an archive must be aligned to this block size.
ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ BlockSize = 512;

ಸಂರಚನಾ UstarHeader {
  ವರ್ಣ Name[100];
  ವರ್ಣ Mode[8];
  ವರ್ಣ Uid[8];
  ವರ್ಣ Gid[8];
  ವರ್ಣ Size[12];
  ವರ್ಣ Mtime[12];
  ವರ್ಣ Checksum[8];
  ವರ್ಣ TypeFlag;
  ವರ್ಣ Linkname[100];
  ವರ್ಣ Magic[6];
  ವರ್ಣ Version[2];
  ವರ್ಣ Uname[32];
  ವರ್ಣ Gname[32];
  ವರ್ಣ DevMajor[8];
  ವರ್ಣ DevMinor[8];
  ವರ್ಣ Prefix[155];
  ವರ್ಣ Pad[12];
};
static_assert(ಪರಿಮಾಣ(UstarHeader) == BlockSize, "invalid Ustar header");

ಸ್ಥಿರ UstarHeader makeUstarHeader() {
  UstarHeader Hdr = {};
  memcpy(Hdr.Magic, "ustar", 5); // Ustar magic
  memcpy(Hdr.Version, "00", 2);  // Ustar version
  ಪ್ರತ್ಯಾಗಮ Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
ಸ್ಥಿರ std::string formatPax(StringRef ಕೀಲಿ, StringRef Val) {
  ಪೂರ್ಣಾಙ್ಕ Len = ಕೀಲಿ.size() + Val.size() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  ಪೂರ್ಣಾಙ್ಕ Total = Len + Twine(Len).str().size();
  Total = Len + Twine(Total).str().size();
  ಪ್ರತ್ಯಾಗಮ (Twine(Total) + " " + ಕೀಲಿ + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
ಸ್ಥಿರ ಶೂನ್ಯ pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(alignTo(Pos, BlockSize));
}

// Computes a checksum for a tar header.
ಸ್ಥಿರ ಶೂನ್ಯ computeChecksum(UstarHeader &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', ಪರಿಮಾಣ(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  unsigned Chksum = 0;
  ಪ್ರತಿ (size_t I = 0; I < ಪರಿಮಾಣ(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, ಪರಿಮಾಣ(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
ಸ್ಥಿರ ಶೂನ್ಯ writePaxHeader(raw_fd_ostream &OS, StringRef Path) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", Path);

  // Create a 512-byte header.
  UstarHeader Hdr = makeUstarHeader();
  snprintf(Hdr.Size, ಪರಿಮಾಣ(Hdr.Size), "%011zo", PaxAttr.size());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<ವರ್ಣ *>(&Hdr), ಪರಿಮಾಣ(Hdr));
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
ಸ್ಥಿರ ತಾರ್ಕಿಕ splitUstar(StringRef Path, StringRef &Prefix, StringRef &Name) {
  ಯದಿ (Path.size() < ಪರಿಮಾಣ(UstarHeader::Name)) {
    Prefix = "";
    Name = Path;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
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
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ MaxPrefix = 137;
  size_t Sep = Path.rfind('/', MaxPrefix + 1);
  ಯದಿ (Sep == StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (Path.size() - Sep - 1 >= ಪರಿಮಾಣ(UstarHeader::Name))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  Prefix = Path.substr(0, Sep);
  Name = Path.substr(Sep + 1);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
ಸ್ಥಿರ ಶೂನ್ಯ writeUstarHeader(raw_fd_ostream &OS, StringRef Prefix,
                             StringRef Name, size_t Size) {
  UstarHeader Hdr = makeUstarHeader();
  memcpy(Hdr.Name, Name.data(), Name.size());
  memcpy(Hdr.Mode, "0000664", 8);
  snprintf(Hdr.Size, ಪರಿಮಾಣ(Hdr.Size), "%011zo", Size);
  memcpy(Hdr.Prefix, Prefix.data(), Prefix.size());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<ವರ್ಣ *>(&Hdr), ಪರಿಮಾಣ(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef OutputPath,
                                                       StringRef BaseDir) {
  using ನಾಮಾಕಾಶ sys::fs;
  ಪೂರ್ಣಾಙ್ಕ FD;
  ಯದಿ (std::error_code EC =
          openFileForWrite(OutputPath, FD, CD_CreateAlways, OF_None))
    ಪ್ರತ್ಯಾಗಮ make_error<StringError>("cannot open " + OutputPath, EC);
  ಪ್ರತ್ಯಾಗಮ std::unique_ptr<TarWriter>(ನೂತನ TarWriter(FD, BaseDir));
}

TarWriter::TarWriter(ಪೂರ್ಣಾಙ್ಕ FD, StringRef BaseDir)
    : OS(FD, /*shouldClose=*/ಸತ್ಯ, /*unbuffered=*/ಅಸತ್ಯ),
      BaseDir(std::string(BaseDir)) {}

// Append a given file to an archive.
ಶೂನ್ಯ TarWriter::append(StringRef Path, StringRef Data) {
  // Write Path and Data.
  std::string Fullpath = BaseDir + "/" + sys::path::convert_to_slash(Path);

  // We do not want to include the same file more than once.
  ಯದಿ (!Files.ಸೇರಿಸು(Fullpath).second)
    ಪ್ರತ್ಯಾಗಮ;

  StringRef Prefix;
  StringRef Name;
  ಯದಿ (splitUstar(Fullpath, Prefix, Name)) {
    writeUstarHeader(OS, Prefix, Name, Data.size());
  } ಅನ್ಯಥಾ {
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
