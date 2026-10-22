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

استعمال نامخلا llvm;

// Each file in an archive must be aligned to this block size.
ساکن ثابت صحیح بلاک_سائز = 512;

ساخت Ustar_سرخی {
  حرف Name[100];
  حرف منوال[8];
  حرف Uid[8];
  حرف Gid[8];
  حرف سائز[12];
  حرف Mtime[12];
  حرف Checksum[8];
  حرف TypeFlag;
  حرف Linkname[100];
  حرف Magic[6];
  حرف نسخہ[2];
  حرف Uname[32];
  حرف Gname[32];
  حرف DevMajor[8];
  حرف DevMinor[8];
  حرف سابقہ[155];
  حرف Pad[12];
};
static_assert(حجم(Ustar_سرخی) == بلاک_سائز, "invalid Ustar header");

ساکن Ustar_سرخی make_Ustar_سرخی() {
  Ustar_سرخی Hdr = {};
  حافظہ_نقل(Hdr.Magic, "ustar", 5); // Ustar magic
  حافظہ_نقل(Hdr.نسخہ, "00", 2);  // Ustar version
  واپس Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
ساکن std::string formatPax(StringRef کلید, StringRef Val) {
  صحیح Len = کلید.سائز_2() + Val.سائز_2() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  صحیح کل = Len + Twine(Len).str().سائز_2();
  کل = Len + Twine(کل).str().سائز_2();
  واپس (Twine(کل) + " " + کلید + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
ساکن خالی pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(align_مقصد(Pos, بلاک_سائز));
}

// Computes a checksum for a tar header.
ساکن خالی computeChecksum(Ustar_سرخی &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', حجم(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  غیرعلامتی Chksum = 0;
  برائے (size_t I = 0; I < حجم(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, حجم(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
ساکن خالی write_Pax_سرخی(raw_fd_ostream &OS, StringRef راستہ) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", راستہ);

  // Create a 512-byte header.
  Ustar_سرخی Hdr = make_Ustar_سرخی();
  snprintf(Hdr.سائز, حجم(Hdr.سائز), "%011zo", PaxAttr.سائز_2());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<حرف *>(&Hdr), حجم(Hdr));
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
ساکن منطقی بانٹو_Ustar(StringRef راستہ, StringRef &سابقہ, StringRef &Name) {
  اگر (راستہ.سائز_2() < حجم(Ustar_سرخی::Name)) {
    سابقہ = "";
    Name = راستہ;
    واپس سچ;
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
  ثابت صحیح Max_سابقہ = 137;
  size_t Sep = راستہ.rfind('/', Max_سابقہ + 1);
  اگر (Sep == StringRef::npos)
    واپس جھوٹ;
  اگر (راستہ.سائز_2() - Sep - 1 >= حجم(Ustar_سرخی::Name))
    واپس جھوٹ;

  سابقہ = راستہ.substr(0, Sep);
  Name = راستہ.substr(Sep + 1);
  واپس سچ;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
ساکن خالی write_Ustar_سرخی(raw_fd_ostream &OS, StringRef سابقہ,
                             StringRef Name, size_t سائز) {
  Ustar_سرخی Hdr = make_Ustar_سرخی();
  حافظہ_نقل(Hdr.Name, Name.ڈیٹا(), Name.سائز_2());
  حافظہ_نقل(Hdr.منوال, "0000664", 8);
  snprintf(Hdr.سائز, حجم(Hdr.سائز), "%011zo", سائز);
  حافظہ_نقل(Hdr.سابقہ, سابقہ.ڈیٹا(), سابقہ.سائز_2());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<حرف *>(&Hdr), حجم(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef Output_راستہ,
                                                       StringRef بنیاد_Dir) {
  استعمال نامخلا sys::fs;
  صحیح FD;
  اگر (std::error_رمز EC =
          openFileForWrite(Output_راستہ, FD, CD_CreateAlways, کا_None))
    واپس make_error<StringError>("cannot open " + Output_راستہ, EC);
  واپس std::unique_ptr<TarWriter>(نیا TarWriter(FD, بنیاد_Dir));
}

TarWriter::TarWriter(صحیح FD, StringRef بنیاد_Dir)
    : OS(FD, /*shouldClose=*/سچ, /*unbuffered=*/جھوٹ),
      بنیاد_Dir(std::string(بنیاد_Dir)) {}

// Append a given file to an archive.
خالی TarWriter::append(StringRef راستہ, StringRef ڈیٹا_2) {
  // Write Path and Data.
  std::string Fullpath = بنیاد_Dir + "/" + sys::راستہ_2::convert_مقصد_slash(راستہ);

  // We do not want to include the same file more than once.
  اگر (!Files.داخل_کرو(Fullpath).دوسرا_2)
    واپس;

  StringRef سابقہ;
  StringRef Name;
  اگر (بانٹو_Ustar(Fullpath, سابقہ, Name)) {
    write_Ustar_سرخی(OS, سابقہ, Name, ڈیٹا_2.سائز_2());
  } ورنہ {
    write_Pax_سرخی(OS, Fullpath);
    write_Ustar_سرخی(OS, "", "", ڈیٹا_2.سائز_2());
  }

  OS << ڈیٹا_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(بلاک_سائز * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
