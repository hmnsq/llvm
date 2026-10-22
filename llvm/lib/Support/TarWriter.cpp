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

#ضمن "llvm/Support/TarWriter.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/Path.h"

استفاده نطاق llvm;

// Each file in an archive must be aligned to this block size.
ساكن ثابت صحيح بلوک_اندازه = 512;

بنية Ustar_سرآیند {
  حرف Name[100];
  حرف نما[8];
  حرف Uid[8];
  حرف Gid[8];
  حرف اندازه[12];
  حرف Mtime[12];
  حرف Checksum[8];
  حرف TypeFlag;
  حرف Linkname[100];
  حرف Magic[6];
  حرف نسخه[2];
  حرف Uname[32];
  حرف Gname[32];
  حرف DevMajor[8];
  حرف DevMinor[8];
  حرف پیشوند[155];
  حرف Pad[12];
};
static_assert(حجم(Ustar_سرآیند) == بلوک_اندازه, "invalid Ustar header");

ساكن Ustar_سرآیند make_Ustar_سرآیند() {
  Ustar_سرآیند Hdr = {};
  کپی_حافظه(Hdr.Magic, "ustar", 5); // Ustar magic
  کپی_حافظه(Hdr.نسخه, "00", 2);  // Ustar version
  أرجع Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
ساكن std::string formatPax(StringRef کلید, StringRef Val) {
  صحيح Len = کلید.اندازه_2() + Val.اندازه_2() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  صحيح مجموع = Len + Twine(Len).str().اندازه_2();
  مجموع = Len + Twine(مجموع).str().اندازه_2();
  أرجع (Twine(مجموع) + " " + کلید + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
ساكن فراغ pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(align_مقصد(Pos, بلوک_اندازه));
}

// Computes a checksum for a tar header.
ساكن فراغ computeChecksum(Ustar_سرآیند &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', حجم(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  بیعلامت Chksum = 0;
  لكل (size_t I = 0; I < حجم(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, حجم(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
ساكن فراغ write_Pax_سرآیند(raw_fd_ostream &OS, StringRef مسیر) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", مسیر);

  // Create a 512-byte header.
  Ustar_سرآیند Hdr = make_Ustar_سرآیند();
  snprintf(Hdr.اندازه, حجم(Hdr.اندازه), "%011zo", PaxAttr.اندازه_2());
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
ساكن منطقي تقسیم_Ustar(StringRef مسیر, StringRef &پیشوند, StringRef &Name) {
  إذا (مسیر.اندازه_2() < حجم(Ustar_سرآیند::Name)) {
    پیشوند = "";
    Name = مسیر;
    أرجع درست;
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
  ثابت صحيح Max_پیشوند = 137;
  size_t Sep = مسیر.rfind('/', Max_پیشوند + 1);
  إذا (Sep == StringRef::npos)
    أرجع خطأ;
  إذا (مسیر.اندازه_2() - Sep - 1 >= حجم(Ustar_سرآیند::Name))
    أرجع خطأ;

  پیشوند = مسیر.substr(0, Sep);
  Name = مسیر.substr(Sep + 1);
  أرجع درست;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
ساكن فراغ write_Ustar_سرآیند(raw_fd_ostream &OS, StringRef پیشوند,
                             StringRef Name, size_t اندازه) {
  Ustar_سرآیند Hdr = make_Ustar_سرآیند();
  کپی_حافظه(Hdr.Name, Name.داده(), Name.اندازه_2());
  کپی_حافظه(Hdr.نما, "0000664", 8);
  snprintf(Hdr.اندازه, حجم(Hdr.اندازه), "%011zo", اندازه);
  کپی_حافظه(Hdr.پیشوند, پیشوند.داده(), پیشوند.اندازه_2());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<حرف *>(&Hdr), حجم(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef Output_مسیر,
                                                       StringRef پایه_Dir) {
  استفاده نطاق sys::fs;
  صحيح FD;
  إذا (std::error_کد EC =
          openFileForWrite(Output_مسیر, FD, CD_CreateAlways, از_None))
    أرجع make_error<StringError>("cannot open " + Output_مسیر, EC);
  أرجع std::unique_ptr<TarWriter>(جديد TarWriter(FD, پایه_Dir));
}

TarWriter::TarWriter(صحيح FD, StringRef پایه_Dir)
    : OS(FD, /*shouldClose=*/درست, /*unbuffered=*/خطأ),
      پایه_Dir(std::string(پایه_Dir)) {}

// Append a given file to an archive.
فراغ TarWriter::append(StringRef مسیر, StringRef داده_2) {
  // Write Path and Data.
  std::string Fullpath = پایه_Dir + "/" + sys::مسیر_2::convert_مقصد_slash(مسیر);

  // We do not want to include the same file more than once.
  إذا (!Files.درج(Fullpath).دومی_2)
    أرجع;

  StringRef پیشوند;
  StringRef Name;
  إذا (تقسیم_Ustar(Fullpath, پیشوند, Name)) {
    write_Ustar_سرآیند(OS, پیشوند, Name, داده_2.اندازه_2());
  } وإلا {
    write_Pax_سرآیند(OS, Fullpath);
    write_Ustar_سرآیند(OS, "", "", داده_2.اندازه_2());
  }

  OS << داده_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(بلوک_اندازه * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
