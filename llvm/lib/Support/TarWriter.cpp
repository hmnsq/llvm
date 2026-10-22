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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/TarWriter.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

// Each file in an archive must be aligned to this block size.
ਸਥਿਰ ਅਚਲ ਪੂਰਨ ਖੰਡ_ਮਾਪ = 512;

ਬਣਤਰ Ustar_ਸਿਰਲੇਖ {
  ਅੱਖਰ Name[100];
  ਅੱਖਰ ਬਹੁਲਕ[8];
  ਅੱਖਰ Uid[8];
  ਅੱਖਰ Gid[8];
  ਅੱਖਰ ਮਾਪ[12];
  ਅੱਖਰ Mtime[12];
  ਅੱਖਰ Checksum[8];
  ਅੱਖਰ TypeFlag;
  ਅੱਖਰ Linkname[100];
  ਅੱਖਰ Magic[6];
  ਅੱਖਰ ਸੰਸਕਰਣ[2];
  ਅੱਖਰ Uname[32];
  ਅੱਖਰ Gname[32];
  ਅੱਖਰ DevMajor[8];
  ਅੱਖਰ DevMinor[8];
  ਅੱਖਰ ਅਗੇਤਰ[155];
  ਅੱਖਰ Pad[12];
};
static_assert(ਆਕਾਰ(Ustar_ਸਿਰਲੇਖ) == ਖੰਡ_ਮਾਪ, "invalid Ustar header");

ਸਥਿਰ Ustar_ਸਿਰਲੇਖ make_Ustar_ਸਿਰਲੇਖ() {
  Ustar_ਸਿਰਲੇਖ Hdr = {};
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(Hdr.Magic, "ustar", 5); // Ustar magic
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(Hdr.ਸੰਸਕਰਣ, "00", 2);  // Ustar version
  ਵਾਪਸ Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
ਸਥਿਰ std::string formatPax(StringRef ਕੁੰਜੀ, StringRef Val) {
  ਪੂਰਨ Len = ਕੁੰਜੀ.ਮਾਪ_2() + Val.ਮਾਪ_2() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  ਪੂਰਨ ਕੁੱਲ = Len + Twine(Len).str().ਮਾਪ_2();
  ਕੁੱਲ = Len + Twine(ਕੁੱਲ).str().ਮਾਪ_2();
  ਵਾਪਸ (Twine(ਕੁੱਲ) + " " + ਕੁੰਜੀ + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
ਸਥਿਰ ਖਾਲੀ pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(align_ਮੰਜ਼ਿਲ(Pos, ਖੰਡ_ਮਾਪ));
}

// Computes a checksum for a tar header.
ਸਥਿਰ ਖਾਲੀ computeChecksum(Ustar_ਸਿਰਲੇਖ &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', ਆਕਾਰ(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Chksum = 0;
  ਲਈ (size_t I = 0; I < ਆਕਾਰ(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, ਆਕਾਰ(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
ਸਥਿਰ ਖਾਲੀ write_Pax_ਸਿਰਲੇਖ(raw_fd_ostream &OS, StringRef ਰਸਤਾ) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", ਰਸਤਾ);

  // Create a 512-byte header.
  Ustar_ਸਿਰਲੇਖ Hdr = make_Ustar_ਸਿਰਲੇਖ();
  snprintf(Hdr.ਮਾਪ, ਆਕਾਰ(Hdr.ਮਾਪ), "%011zo", PaxAttr.ਮਾਪ_2());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<ਅੱਖਰ *>(&Hdr), ਆਕਾਰ(Hdr));
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
ਸਥਿਰ ਤਾਰਕਿਕ ਵੰਡੋ_Ustar(StringRef ਰਸਤਾ, StringRef &ਅਗੇਤਰ, StringRef &Name) {
  ਜੇ (ਰਸਤਾ.ਮਾਪ_2() < ਆਕਾਰ(Ustar_ਸਿਰਲੇਖ::Name)) {
    ਅਗੇਤਰ = "";
    Name = ਰਸਤਾ;
    ਵਾਪਸ ਸੱਚ;
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
  ਅਚਲ ਪੂਰਨ Max_ਅਗੇਤਰ = 137;
  size_t Sep = ਰਸਤਾ.rfind('/', Max_ਅਗੇਤਰ + 1);
  ਜੇ (Sep == StringRef::npos)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (ਰਸਤਾ.ਮਾਪ_2() - Sep - 1 >= ਆਕਾਰ(Ustar_ਸਿਰਲੇਖ::Name))
    ਵਾਪਸ ਝੂਠ;

  ਅਗੇਤਰ = ਰਸਤਾ.substr(0, Sep);
  Name = ਰਸਤਾ.substr(Sep + 1);
  ਵਾਪਸ ਸੱਚ;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
ਸਥਿਰ ਖਾਲੀ write_Ustar_ਸਿਰਲੇਖ(raw_fd_ostream &OS, StringRef ਅਗੇਤਰ,
                             StringRef Name, size_t ਮਾਪ) {
  Ustar_ਸਿਰਲੇਖ Hdr = make_Ustar_ਸਿਰਲੇਖ();
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(Hdr.Name, Name.ਅੰਕੜੇ(), Name.ਮਾਪ_2());
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(Hdr.ਬਹੁਲਕ, "0000664", 8);
  snprintf(Hdr.ਮਾਪ, ਆਕਾਰ(Hdr.ਮਾਪ), "%011zo", ਮਾਪ);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(Hdr.ਅਗੇਤਰ, ਅਗੇਤਰ.ਅੰਕੜੇ(), ਅਗੇਤਰ.ਮਾਪ_2());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<ਅੱਖਰ *>(&Hdr), ਆਕਾਰ(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef Output_ਰਸਤਾ,
                                                       StringRef ਅਧਾਰ_Dir) {
  ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ sys::fs;
  ਪੂਰਨ FD;
  ਜੇ (std::error_ਕੋਡ EC =
          openFileForWrite(Output_ਰਸਤਾ, FD, CD_CreateAlways, ਦਾ_None))
    ਵਾਪਸ make_error<StringError>("cannot open " + Output_ਰਸਤਾ, EC);
  ਵਾਪਸ std::unique_ptr<TarWriter>(ਨਵਾਂ TarWriter(FD, ਅਧਾਰ_Dir));
}

TarWriter::TarWriter(ਪੂਰਨ FD, StringRef ਅਧਾਰ_Dir)
    : OS(FD, /*shouldClose=*/ਸੱਚ, /*unbuffered=*/ਝੂਠ),
      ਅਧਾਰ_Dir(std::string(ਅਧਾਰ_Dir)) {}

// Append a given file to an archive.
ਖਾਲੀ TarWriter::append(StringRef ਰਸਤਾ, StringRef ਅੰਕੜੇ_2) {
  // Write Path and Data.
  std::string Fullpath = ਅਧਾਰ_Dir + "/" + sys::ਰਸਤਾ_2::convert_ਮੰਜ਼ਿਲ_slash(ਰਸਤਾ);

  // We do not want to include the same file more than once.
  ਜੇ (!Files.ਪਾਓ(Fullpath).ਦੂਜਾ_2)
    ਵਾਪਸ;

  StringRef ਅਗੇਤਰ;
  StringRef Name;
  ਜੇ (ਵੰਡੋ_Ustar(Fullpath, ਅਗੇਤਰ, Name)) {
    write_Ustar_ਸਿਰਲੇਖ(OS, ਅਗੇਤਰ, Name, ਅੰਕੜੇ_2.ਮਾਪ_2());
  } ਨਹੀਂਤਾਂ {
    write_Pax_ਸਿਰਲੇਖ(OS, Fullpath);
    write_Ustar_ਸਿਰਲੇਖ(OS, "", "", ਅੰਕੜੇ_2.ਮਾਪ_2());
  }

  OS << ਅੰਕੜੇ_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(ਖੰਡ_ਮਾਪ * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
