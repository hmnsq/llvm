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

#समावेश "llvm/Support/TarWriter.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/Path.h"

using namespace llvm;

// Each file in an archive must be aligned to this block size.
अटल स्थिर पूर्णांक Block_आकार = 512;

काष्ठा UstarHeader {
  अक्षर Name[100];
  अक्षर Mode[8];
  अक्षर Uid[8];
  अक्षर Gid[8];
  अक्षर आकार[12];
  अक्षर Mtime[12];
  अक्षर Checksum[8];
  अक्षर TypeFlag;
  अक्षर Linkname[100];
  अक्षर Magic[6];
  अक्षर Version[2];
  अक्षर Uname[32];
  अक्षर Gname[32];
  अक्षर DevMajor[8];
  अक्षर DevMinor[8];
  अक्षर Prefix[155];
  अक्षर Pad[12];
};
static_assert(माप(UstarHeader) == Block_आकार, "invalid Ustar header");

अटल UstarHeader makeUstarHeader() {
  UstarHeader Hdr = {};
  स_नकल(Hdr.Magic, "ustar", 5); // Ustar magic
  स_नकल(Hdr.Version, "00", 2);  // Ustar version
  वापस Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
अटल std::string formatPax(StringRef कुंजी, StringRef Val) {
  पूर्णांक Len = कुंजी.आकार_2() + Val.आकार_2() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  पूर्णांक योग_2 = Len + Twine(Len).str().आकार_2();
  योग_2 = Len + Twine(योग_2).str().आकार_2();
  वापस (Twine(योग_2) + " " + कुंजी + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
अटल व्योम pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(alignTo(Pos, Block_आकार));
}

// Computes a checksum for a tar header.
अटल व्योम computeChecksum(UstarHeader &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  स_रखो(Hdr.Checksum, ' ', माप(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  अचिन्हित Chksum = 0;
  क्रम (माप_प्रकार I = 0; I < माप(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, माप(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
अटल व्योम writePaxHeader(raw_fd_ostream &OS, StringRef Path) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", Path);

  // Create a 512-byte header.
  UstarHeader Hdr = makeUstarHeader();
  snprintf(Hdr.आकार, माप(Hdr.आकार), "%011zo", PaxAttr.आकार_2());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<अक्षर *>(&Hdr), माप(Hdr));
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
अटल bool splitUstar(StringRef Path, StringRef &Prefix, StringRef &Name) {
  अगर (Path.आकार_2() < माप(UstarHeader::Name)) {
    Prefix = "";
    Name = Path;
    वापस true;
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
  स्थिर पूर्णांक MaxPrefix = 137;
  माप_प्रकार Sep = Path.rfind('/', MaxPrefix + 1);
  अगर (Sep == StringRef::npos)
    वापस false;
  अगर (Path.आकार_2() - Sep - 1 >= माप(UstarHeader::Name))
    वापस false;

  Prefix = Path.substr(0, Sep);
  Name = Path.substr(Sep + 1);
  वापस true;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
अटल व्योम writeUstarHeader(raw_fd_ostream &OS, StringRef Prefix,
                             StringRef Name, माप_प्रकार आकार) {
  UstarHeader Hdr = makeUstarHeader();
  स_नकल(Hdr.Name, Name.तथ्य(), Name.आकार_2());
  स_नकल(Hdr.Mode, "0000664", 8);
  snprintf(Hdr.आकार, माप(Hdr.आकार), "%011zo", आकार);
  स_नकल(Hdr.Prefix, Prefix.तथ्य(), Prefix.आकार_2());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<अक्षर *>(&Hdr), माप(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef OutputPath,
                                                       StringRef BaseDir) {
  using namespace sys::fs;
  पूर्णांक FD;
  अगर (std::error_code EC =
          openFileForWrite(OutputPath, FD, CD_CreateAlways, OF_None))
    वापस make_error<StringError>("cannot open " + OutputPath, EC);
  वापस std::unique_ptr<TarWriter>(नया TarWriter(FD, BaseDir));
}

TarWriter::TarWriter(पूर्णांक FD, StringRef BaseDir)
    : OS(FD, /*shouldClose=*/true, /*unbuffered=*/false),
      BaseDir(std::string(BaseDir)) {}

// Append a given file to an archive.
व्योम TarWriter::append(StringRef Path, StringRef तथ्य_2) {
  // Write Path and Data.
  std::string Fullpath = BaseDir + "/" + sys::path::convert_to_slash(Path);

  // We do not want to include the same file more than once.
  अगर (!Files.जोड़ो(Fullpath).second)
    वापस;

  StringRef Prefix;
  StringRef Name;
  अगर (splitUstar(Fullpath, Prefix, Name)) {
    writeUstarHeader(OS, Prefix, Name, तथ्य_2.आकार_2());
  } अन्यथा {
    writePaxHeader(OS, Fullpath);
    writeUstarHeader(OS, "", "", तथ्य_2.आकार_2());
  }

  OS << तथ्य_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(Block_आकार * 2, '\0');
  OS.seek(Pos);
  OS.सफाई();
}
