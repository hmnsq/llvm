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

#अन्तर्भाव "llvm/Support/TarWriter.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/Path.h"

उपयुज्य नामाकाश llvm;

// Each file in an archive must be aligned to this block size.
स्थिर ध्रुव पूर्णाङ्क खण्डम्_आकार = 512;

संरचना Ustar_शीर्षकम् {
  वर्ण Name[100];
  वर्ण बहुलकम्[8];
  वर्ण Uid[8];
  वर्ण Gid[8];
  वर्ण आकार[12];
  वर्ण Mtime[12];
  वर्ण Checksum[8];
  वर्ण TypeFlag;
  वर्ण Linkname[100];
  वर्ण Magic[6];
  वर्ण संस्करणम्[2];
  वर्ण Uname[32];
  वर्ण Gname[32];
  वर्ण DevMajor[8];
  वर्ण DevMinor[8];
  वर्ण उपसर्गः[155];
  वर्ण Pad[12];
};
static_assert(परिमाण(Ustar_शीर्षकम्) == खण्डम्_आकार, "invalid Ustar header");

स्थिर Ustar_शीर्षकम् make_Ustar_शीर्षकम्() {
  Ustar_शीर्षकम् Hdr = {};
  स्मृतिं_प्रतिलिख(Hdr.Magic, "ustar", 5); // Ustar magic
  स्मृतिं_प्रतिलिख(Hdr.संस्करणम्, "00", 2);  // Ustar version
  प्रत्यागम Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
स्थिर std::string formatPax(StringRef कुञ्चिका, StringRef Val) {
  पूर्णाङ्क Len = कुञ्चिका.आकार_2() + Val.आकार_2() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  पूर्णाङ्क योग_2 = Len + Twine(Len).str().आकार_2();
  योग_2 = Len + Twine(योग_2).str().आकार_2();
  प्रत्यागम (Twine(योग_2) + " " + कुञ्चिका + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
स्थिर शून्य pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(align_गन्तव्यम्(Pos, खण्डम्_आकार));
}

// Computes a checksum for a tar header.
स्थिर शून्य computeChecksum(Ustar_शीर्षकम् &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  स्मृतिं_पूरय(Hdr.Checksum, ' ', परिमाण(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  अचिह्नित Chksum = 0;
  प्रति (size_t I = 0; I < परिमाण(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, परिमाण(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
स्थिर शून्य write_Pax_शीर्षकम्(raw_fd_ostream &OS, StringRef मार्गः) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", मार्गः);

  // Create a 512-byte header.
  Ustar_शीर्षकम् Hdr = make_Ustar_शीर्षकम्();
  snprintf(Hdr.आकार, परिमाण(Hdr.आकार), "%011zo", PaxAttr.आकार_2());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<वर्ण *>(&Hdr), परिमाण(Hdr));
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
स्थिर तार्किक विभज_Ustar(StringRef मार्गः, StringRef &उपसर्गः, StringRef &Name) {
  यदि (मार्गः.आकार_2() < परिमाण(Ustar_शीर्षकम्::Name)) {
    उपसर्गः = "";
    Name = मार्गः;
    प्रत्यागम सत्य;
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
  ध्रुव पूर्णाङ्क Max_उपसर्गः = 137;
  size_t Sep = मार्गः.rfind('/', Max_उपसर्गः + 1);
  यदि (Sep == StringRef::npos)
    प्रत्यागम असत्य;
  यदि (मार्गः.आकार_2() - Sep - 1 >= परिमाण(Ustar_शीर्षकम्::Name))
    प्रत्यागम असत्य;

  उपसर्गः = मार्गः.substr(0, Sep);
  Name = मार्गः.substr(Sep + 1);
  प्रत्यागम सत्य;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
स्थिर शून्य write_Ustar_शीर्षकम्(raw_fd_ostream &OS, StringRef उपसर्गः,
                             StringRef Name, size_t आकार) {
  Ustar_शीर्षकम् Hdr = make_Ustar_शीर्षकम्();
  स्मृतिं_प्रतिलिख(Hdr.Name, Name.दत्त(), Name.आकार_2());
  स्मृतिं_प्रतिलिख(Hdr.बहुलकम्, "0000664", 8);
  snprintf(Hdr.आकार, परिमाण(Hdr.आकार), "%011zo", आकार);
  स्मृतिं_प्रतिलिख(Hdr.उपसर्गः, उपसर्गः.दत्त(), उपसर्गः.आकार_2());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<वर्ण *>(&Hdr), परिमाण(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef Output_मार्गः,
                                                       StringRef आधारः_Dir) {
  उपयुज्य नामाकाश sys::fs;
  पूर्णाङ्क FD;
  यदि (std::error_संहिता EC =
          openFileForWrite(Output_मार्गः, FD, CD_CreateAlways, सम्बन्धि_None))
    प्रत्यागम make_error<StringError>("cannot open " + Output_मार्गः, EC);
  प्रत्यागम std::unique_ptr<TarWriter>(नूतन TarWriter(FD, आधारः_Dir));
}

TarWriter::TarWriter(पूर्णाङ्क FD, StringRef आधारः_Dir)
    : OS(FD, /*shouldClose=*/सत्य, /*unbuffered=*/असत्य),
      आधारः_Dir(std::string(आधारः_Dir)) {}

// Append a given file to an archive.
शून्य TarWriter::append(StringRef मार्गः, StringRef दत्त_2) {
  // Write Path and Data.
  std::string Fullpath = आधारः_Dir + "/" + sys::मार्गः_2::convert_गन्तव्यम्_slash(मार्गः);

  // We do not want to include the same file more than once.
  यदि (!Files.निवेशय(Fullpath).second)
    प्रत्यागम;

  StringRef उपसर्गः;
  StringRef Name;
  यदि (विभज_Ustar(Fullpath, उपसर्गः, Name)) {
    write_Ustar_शीर्षकम्(OS, उपसर्गः, Name, दत्त_2.आकार_2());
  } अन्यथा {
    write_Pax_शीर्षकम्(OS, Fullpath);
    write_Ustar_शीर्षकम्(OS, "", "", दत्त_2.आकार_2());
  }

  OS << दत्त_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(खण्डम्_आकार * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
