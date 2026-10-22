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

کارول نطاق llvm;

// Each file in an archive must be aligned to this block size.
ساكن ثابت صحيح بلاک_اندازه = 512;

بنية Ustar_سرلیک {
  حرف Name[100];
  حرف ډېرپېښ[8];
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
  حرف مختاړی[155];
  حرف Pad[12];
};
static_assert(حجم(Ustar_سرلیک) == بلاک_اندازه, "invalid Ustar header");

ساكن Ustar_سرلیک make_Ustar_سرلیک() {
  Ustar_سرلیک Hdr = {};
  حافظه_نقل(Hdr.Magic, "ustar", 5); // Ustar magic
  حافظه_نقل(Hdr.نسخه, "00", 2);  // Ustar version
  أرجع Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
ساكن std::string formatPax(StringRef کلی, StringRef Val) {
  صحيح Len = کلی.اندازه_2() + Val.اندازه_2() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  صحيح ټول = Len + Twine(Len).str().اندازه_2();
  ټول = Len + Twine(ټول).str().اندازه_2();
  أرجع (Twine(ټول) + " " + کلی + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
ساكن فراغ pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(align_منزل(Pos, بلاک_اندازه));
}

// Computes a checksum for a tar header.
ساكن فراغ computeChecksum(Ustar_سرلیک &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', حجم(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  بینښه Chksum = 0;
  لكل (size_t I = 0; I < حجم(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, حجم(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
ساكن فراغ write_Pax_سرلیک(raw_fd_ostream &OS, StringRef لار) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", لار);

  // Create a 512-byte header.
  Ustar_سرلیک Hdr = make_Ustar_سرلیک();
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
ساكن منطقي وېشل_Ustar(StringRef لار, StringRef &مختاړی, StringRef &Name) {
  إذا (لار.اندازه_2() < حجم(Ustar_سرلیک::Name)) {
    مختاړی = "";
    Name = لار;
    أرجع ریښتیا;
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
  ثابت صحيح Max_مختاړی = 137;
  size_t Sep = لار.rfind('/', Max_مختاړی + 1);
  إذا (Sep == StringRef::npos)
    أرجع خطأ;
  إذا (لار.اندازه_2() - Sep - 1 >= حجم(Ustar_سرلیک::Name))
    أرجع خطأ;

  مختاړی = لار.substr(0, Sep);
  Name = لار.substr(Sep + 1);
  أرجع ریښتیا;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
ساكن فراغ write_Ustar_سرلیک(raw_fd_ostream &OS, StringRef مختاړی,
                             StringRef Name, size_t اندازه) {
  Ustar_سرلیک Hdr = make_Ustar_سرلیک();
  حافظه_نقل(Hdr.Name, Name.معلومات(), Name.اندازه_2());
  حافظه_نقل(Hdr.ډېرپېښ, "0000664", 8);
  snprintf(Hdr.اندازه, حجم(Hdr.اندازه), "%011zo", اندازه);
  حافظه_نقل(Hdr.مختاړی, مختاړی.معلومات(), مختاړی.اندازه_2());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<حرف *>(&Hdr), حجم(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef Output_لار,
                                                       StringRef بنسټ_Dir) {
  کارول نطاق sys::fs;
  صحيح FD;
  إذا (std::error_کوډ EC =
          openFileForWrite(Output_لار, FD, CD_CreateAlways, د_None))
    أرجع make_error<StringError>("cannot open " + Output_لار, EC);
  أرجع std::unique_ptr<TarWriter>(جديد TarWriter(FD, بنسټ_Dir));
}

TarWriter::TarWriter(صحيح FD, StringRef بنسټ_Dir)
    : OS(FD, /*shouldClose=*/ریښتیا, /*unbuffered=*/خطأ),
      بنسټ_Dir(std::string(بنسټ_Dir)) {}

// Append a given file to an archive.
فراغ TarWriter::append(StringRef لار, StringRef معلومات_2) {
  // Write Path and Data.
  std::string Fullpath = بنسټ_Dir + "/" + sys::لار_2::convert_منزل_slash(لار);

  // We do not want to include the same file more than once.
  إذا (!Files.ورننباسه(Fullpath).دوهم_2)
    أرجع;

  StringRef مختاړی;
  StringRef Name;
  إذا (وېشل_Ustar(Fullpath, مختاړی, Name)) {
    write_Ustar_سرلیک(OS, مختاړی, Name, معلومات_2.اندازه_2());
  } وإلا {
    write_Pax_سرلیک(OS, Fullpath);
    write_Ustar_سرلیک(OS, "", "", معلومات_2.اندازه_2());
  }

  OS << معلومات_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(بلاک_اندازه * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
