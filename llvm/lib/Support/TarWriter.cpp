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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

// Each file in an archive must be aligned to this block size.
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಖಂಡ_ಗಾತ್ರ = 512;

ರಚನೆ Ustar_ಶೀರ್ಷಿಕೆ {
  ಅಕ್ಷರ Name[100];
  ಅಕ್ಷರ ಬಹುಲಕ[8];
  ಅಕ್ಷರ Uid[8];
  ಅಕ್ಷರ Gid[8];
  ಅಕ್ಷರ ಗಾತ್ರ_2[12];
  ಅಕ್ಷರ Mtime[12];
  ಅಕ್ಷರ Checksum[8];
  ಅಕ್ಷರ TypeFlag;
  ಅಕ್ಷರ Linkname[100];
  ಅಕ್ಷರ Magic[6];
  ಅಕ್ಷರ ಆವೃತ್ತಿ[2];
  ಅಕ್ಷರ Uname[32];
  ಅಕ್ಷರ Gname[32];
  ಅಕ್ಷರ DevMajor[8];
  ಅಕ್ಷರ DevMinor[8];
  ಅಕ್ಷರ ಪೂರ್ವಪ್ರತ್ಯಯ[155];
  ಅಕ್ಷರ Pad[12];
};
static_assert(ಗಾತ್ರ(Ustar_ಶೀರ್ಷಿಕೆ) == ಖಂಡ_ಗಾತ್ರ, "invalid Ustar header");

ಸ್ಥಿರ Ustar_ಶೀರ್ಷಿಕೆ make_Ustar_ಶೀರ್ಷಿಕೆ() {
  Ustar_ಶೀರ್ಷಿಕೆ Hdr = {};
  ಸ್ಮರಣೆ_ನಕಲು(Hdr.Magic, "ustar", 5); // Ustar magic
  ಸ್ಮರಣೆ_ನಕಲು(Hdr.ಆವೃತ್ತಿ, "00", 2);  // Ustar version
  ಹಿಂತಿರುಗಿ Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
ಸ್ಥಿರ std::string formatPax(StringRef ಕೀಲಿ, StringRef Val) {
  ಪೂರ್ಣಾಂಕ Len = ಕೀಲಿ.ಗಾತ್ರ_3() + Val.ಗಾತ್ರ_3() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  ಪೂರ್ಣಾಂಕ ಒಟ್ಟು = Len + Twine(Len).str().ಗಾತ್ರ_3();
  ಒಟ್ಟು = Len + Twine(ಒಟ್ಟು).str().ಗಾತ್ರ_3();
  ಹಿಂತಿರುಗಿ (Twine(ಒಟ್ಟು) + " " + ಕೀಲಿ + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
ಸ್ಥಿರ ಶೂನ್ಯ pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(align_ಗಮ್ಯ(Pos, ಖಂಡ_ಗಾತ್ರ));
}

// Computes a checksum for a tar header.
ಸ್ಥಿರ ಶೂನ್ಯ computeChecksum(Ustar_ಶೀರ್ಷಿಕೆ &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', ಗಾತ್ರ(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  ಚಿಹ್ನೆರಹಿತ Chksum = 0;
  ಗಾಗಿ (size_t I = 0; I < ಗಾತ್ರ(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, ಗಾತ್ರ(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
ಸ್ಥಿರ ಶೂನ್ಯ write_Pax_ಶೀರ್ಷಿಕೆ(raw_fd_ostream &OS, StringRef ಮಾರ್ಗ) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", ಮಾರ್ಗ);

  // Create a 512-byte header.
  Ustar_ಶೀರ್ಷಿಕೆ Hdr = make_Ustar_ಶೀರ್ಷಿಕೆ();
  snprintf(Hdr.ಗಾತ್ರ_2, ಗಾತ್ರ(Hdr.ಗಾತ್ರ_2), "%011zo", PaxAttr.ಗಾತ್ರ_3());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<ಅಕ್ಷರ *>(&Hdr), ಗಾತ್ರ(Hdr));
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
ಸ್ಥಿರ ತಾರ್ಕಿಕ ವಿಭಜಿಸು_Ustar(StringRef ಮಾರ್ಗ, StringRef &ಪೂರ್ವಪ್ರತ್ಯಯ, StringRef &Name) {
  ಆದರೆ (ಮಾರ್ಗ.ಗಾತ್ರ_3() < ಗಾತ್ರ(Ustar_ಶೀರ್ಷಿಕೆ::Name)) {
    ಪೂರ್ವಪ್ರತ್ಯಯ = "";
    Name = ಮಾರ್ಗ;
    ಹಿಂತಿರುಗಿ ನಿಜ;
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
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ Max_ಪೂರ್ವಪ್ರತ್ಯಯ = 137;
  size_t Sep = ಮಾರ್ಗ.rfind('/', Max_ಪೂರ್ವಪ್ರತ್ಯಯ + 1);
  ಆದರೆ (Sep == StringRef::npos)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (ಮಾರ್ಗ.ಗಾತ್ರ_3() - Sep - 1 >= ಗಾತ್ರ(Ustar_ಶೀರ್ಷಿಕೆ::Name))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಪೂರ್ವಪ್ರತ್ಯಯ = ಮಾರ್ಗ.substr(0, Sep);
  Name = ಮಾರ್ಗ.substr(Sep + 1);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
ಸ್ಥಿರ ಶೂನ್ಯ write_Ustar_ಶೀರ್ಷಿಕೆ(raw_fd_ostream &OS, StringRef ಪೂರ್ವಪ್ರತ್ಯಯ,
                             StringRef Name, size_t ಗಾತ್ರ_2) {
  Ustar_ಶೀರ್ಷಿಕೆ Hdr = make_Ustar_ಶೀರ್ಷಿಕೆ();
  ಸ್ಮರಣೆ_ನಕಲು(Hdr.Name, Name.ದತ್ತಾಂಶ(), Name.ಗಾತ್ರ_3());
  ಸ್ಮರಣೆ_ನಕಲು(Hdr.ಬಹುಲಕ, "0000664", 8);
  snprintf(Hdr.ಗಾತ್ರ_2, ಗಾತ್ರ(Hdr.ಗಾತ್ರ_2), "%011zo", ಗಾತ್ರ_2);
  ಸ್ಮರಣೆ_ನಕಲು(Hdr.ಪೂರ್ವಪ್ರತ್ಯಯ, ಪೂರ್ವಪ್ರತ್ಯಯ.ದತ್ತಾಂಶ(), ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<ಅಕ್ಷರ *>(&Hdr), ಗಾತ್ರ(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef Output_ಮಾರ್ಗ,
                                                       StringRef ತಳ_Dir) {
  ಬಳಸಿ ನಾಮಾಕಾಶ sys::fs;
  ಪೂರ್ಣಾಂಕ FD;
  ಆದರೆ (std::error_ಸಂಕೇತ EC =
          openFileForWrite(Output_ಮಾರ್ಗ, FD, CD_CreateAlways, ದ_None))
    ಹಿಂತಿರುಗಿ make_error<StringError>("cannot open " + Output_ಮಾರ್ಗ, EC);
  ಹಿಂತಿರುಗಿ std::unique_ptr<TarWriter>(ಹೊಸ TarWriter(FD, ತಳ_Dir));
}

TarWriter::TarWriter(ಪೂರ್ಣಾಂಕ FD, StringRef ತಳ_Dir)
    : OS(FD, /*shouldClose=*/ನಿಜ, /*unbuffered=*/ಸುಳ್ಳು),
      ತಳ_Dir(std::string(ತಳ_Dir)) {}

// Append a given file to an archive.
ಶೂನ್ಯ TarWriter::append(StringRef ಮಾರ್ಗ, StringRef ದತ್ತಾಂಶ_2) {
  // Write Path and Data.
  std::string Fullpath = ತಳ_Dir + "/" + sys::ಮಾರ್ಗ_2::convert_ಗಮ್ಯ_slash(ಮಾರ್ಗ);

  // We do not want to include the same file more than once.
  ಆದರೆ (!Files.ಸೇರಿಸು(Fullpath).ಎರಡನೇ_2)
    ಹಿಂತಿರುಗಿ;

  StringRef ಪೂರ್ವಪ್ರತ್ಯಯ;
  StringRef Name;
  ಆದರೆ (ವಿಭಜಿಸು_Ustar(Fullpath, ಪೂರ್ವಪ್ರತ್ಯಯ, Name)) {
    write_Ustar_ಶೀರ್ಷಿಕೆ(OS, ಪೂರ್ವಪ್ರತ್ಯಯ, Name, ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3());
  } ಇಲ್ಲದಿದ್ದರೆ {
    write_Pax_ಶೀರ್ಷಿಕೆ(OS, Fullpath);
    write_Ustar_ಶೀರ್ಷಿಕೆ(OS, "", "", ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3());
  }

  OS << ದತ್ತಾಂಶ_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(ಖಂಡ_ಗಾತ್ರ * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
