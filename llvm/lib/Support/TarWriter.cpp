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

#অন্তর্ভাব "llvm/Support/TarWriter.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/Path.h"

using নামাকাশ llvm;

// Each file in an archive must be aligned to this block size.
স্থির ধ্রুবক পূর্ণসংখ্যা Block_আকার = 512;

কাঠামো UstarHeader {
  অক্ষর Name[100];
  অক্ষর Mode[8];
  অক্ষর Uid[8];
  অক্ষর Gid[8];
  অক্ষর আকার_2[12];
  অক্ষর Mtime[12];
  অক্ষর Checksum[8];
  অক্ষর TypeFlag;
  অক্ষর Linkname[100];
  অক্ষর Magic[6];
  অক্ষর Version[2];
  অক্ষর Uname[32];
  অক্ষর Gname[32];
  অক্ষর DevMajor[8];
  অক্ষর DevMinor[8];
  অক্ষর Prefix[155];
  অক্ষর Pad[12];
};
static_assert(আকার(UstarHeader) == Block_আকার, "invalid Ustar header");

স্থির UstarHeader makeUstarHeader() {
  UstarHeader Hdr = {};
  memcpy(Hdr.Magic, "ustar", 5); // Ustar magic
  memcpy(Hdr.Version, "00", 2);  // Ustar version
  ফেরত Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
স্থির std::string formatPax(StringRef চাবি, StringRef Val) {
  পূর্ণসংখ্যা Len = চাবি.আকার_3() + Val.আকার_3() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  পূর্ণসংখ্যা যোগ_2 = Len + Twine(Len).str().আকার_3();
  যোগ_2 = Len + Twine(যোগ_2).str().আকার_3();
  ফেরত (Twine(যোগ_2) + " " + চাবি + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
স্থির শূন্য pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(alignTo(Pos, Block_আকার));
}

// Computes a checksum for a tar header.
স্থির শূন্য computeChecksum(UstarHeader &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', আকার(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  unsigned Chksum = 0;
  জন্য (size_t I = 0; I < আকার(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, আকার(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
স্থির শূন্য writePaxHeader(raw_fd_ostream &OS, StringRef Path) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", Path);

  // Create a 512-byte header.
  UstarHeader Hdr = makeUstarHeader();
  snprintf(Hdr.আকার_2, আকার(Hdr.আকার_2), "%011zo", PaxAttr.আকার_3());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<অক্ষর *>(&Hdr), আকার(Hdr));
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
স্থির যৌক্তিক splitUstar(StringRef Path, StringRef &Prefix, StringRef &Name) {
  যদি (Path.আকার_3() < আকার(UstarHeader::Name)) {
    Prefix = "";
    Name = Path;
    ফেরত সত্য;
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
  ধ্রুবক পূর্ণসংখ্যা MaxPrefix = 137;
  size_t Sep = Path.rfind('/', MaxPrefix + 1);
  যদি (Sep == StringRef::npos)
    ফেরত মিথ্যা;
  যদি (Path.আকার_3() - Sep - 1 >= আকার(UstarHeader::Name))
    ফেরত মিথ্যা;

  Prefix = Path.substr(0, Sep);
  Name = Path.substr(Sep + 1);
  ফেরত সত্য;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
স্থির শূন্য writeUstarHeader(raw_fd_ostream &OS, StringRef Prefix,
                             StringRef Name, size_t আকার_2) {
  UstarHeader Hdr = makeUstarHeader();
  memcpy(Hdr.Name, Name.তথ্য(), Name.আকার_3());
  memcpy(Hdr.Mode, "0000664", 8);
  snprintf(Hdr.আকার_2, আকার(Hdr.আকার_2), "%011zo", আকার_2);
  memcpy(Hdr.Prefix, Prefix.তথ্য(), Prefix.আকার_3());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<অক্ষর *>(&Hdr), আকার(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef OutputPath,
                                                       StringRef BaseDir) {
  using নামাকাশ sys::fs;
  পূর্ণসংখ্যা FD;
  যদি (std::error_code EC =
          openFileForWrite(OutputPath, FD, CD_CreateAlways, OF_None))
    ফেরত make_error<StringError>("cannot open " + OutputPath, EC);
  ফেরত std::unique_ptr<TarWriter>(নতুন TarWriter(FD, BaseDir));
}

TarWriter::TarWriter(পূর্ণসংখ্যা FD, StringRef BaseDir)
    : OS(FD, /*shouldClose=*/সত্য, /*unbuffered=*/মিথ্যা),
      BaseDir(std::string(BaseDir)) {}

// Append a given file to an archive.
শূন্য TarWriter::append(StringRef Path, StringRef তথ্য_2) {
  // Write Path and Data.
  std::string Fullpath = BaseDir + "/" + sys::path::convert_to_slash(Path);

  // We do not want to include the same file more than once.
  যদি (!Files.ঢোকাও(Fullpath).second)
    ফেরত;

  StringRef Prefix;
  StringRef Name;
  যদি (splitUstar(Fullpath, Prefix, Name)) {
    writeUstarHeader(OS, Prefix, Name, তথ্য_2.আকার_3());
  } অন্যথা {
    writePaxHeader(OS, Fullpath);
    writeUstarHeader(OS, "", "", তথ্য_2.আকার_3());
  }

  OS << তথ্য_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(Block_আকার * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
