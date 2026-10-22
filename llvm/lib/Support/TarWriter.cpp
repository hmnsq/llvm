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

#అన్తర్భావ "llvm/Support/TarWriter.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/Path.h"

ఉపయోగించి నామాకాశ llvm;

// Each file in an archive must be aligned to this block size.
స్థిర స్థిరరాశి పూర్ణాంకం ఖండం_కొలత = 512;

నిర్మాణం Ustar_శీర్షిక {
  అక్షరం Name[100];
  అక్షరం బహుళకం[8];
  అక్షరం Uid[8];
  అక్షరం Gid[8];
  అక్షరం కొలత[12];
  అక్షరం Mtime[12];
  అక్షరం Checksum[8];
  అక్షరం TypeFlag;
  అక్షరం Linkname[100];
  అక్షరం Magic[6];
  అక్షరం సంస్కరణ[2];
  అక్షరం Uname[32];
  అక్షరం Gname[32];
  అక్షరం DevMajor[8];
  అక్షరం DevMinor[8];
  అక్షరం ఉపసర్గ[155];
  అక్షరం Pad[12];
};
static_assert(పరిమాణం(Ustar_శీర్షిక) == ఖండం_కొలత, "invalid Ustar header");

స్థిర Ustar_శీర్షిక make_Ustar_శీర్షిక() {
  Ustar_శీర్షిక Hdr = {};
  జ్ఞాపకం_నకలు(Hdr.Magic, "ustar", 5); // Ustar magic
  జ్ఞాపకం_నకలు(Hdr.సంస్కరణ, "00", 2);  // Ustar version
  తిరిగి Hdr;
}

// A PAX attribute is in the form of "<length> <key>=<value>\n"
// where <length> is the length of the entire string including
// the length field itself. An example string is this.
//
//   25 ctime=1084839148.1212\n
//
// This function create such string.
స్థిర std::string formatPax(StringRef కీలకం, StringRef Val) {
  పూర్ణాంకం Len = కీలకం.కొలత_2() + Val.కొలత_2() + 3; // +3 for " ", "=" and "\n"

  // We need to compute total size twice because appending
  // a length field could change total size by one.
  పూర్ణాంకం మొత్తం = Len + Twine(Len).str().కొలత_2();
  మొత్తం = Len + Twine(మొత్తం).str().కొలత_2();
  తిరిగి (Twine(మొత్తం) + " " + కీలకం + "=" + Val + "\n").str();
}

// Headers in tar files must be aligned to 512 byte boundaries.
// This function forwards the current file position to the next boundary.
స్థిర శూన్య pad(raw_fd_ostream &OS) {
  uint64_t Pos = OS.tell();
  OS.seek(align_గమ్యం(Pos, ఖండం_కొలత));
}

// Computes a checksum for a tar header.
స్థిర శూన్య computeChecksum(Ustar_శీర్షిక &Hdr) {
  // Before computing a checksum, checksum field must be
  // filled with space characters.
  memset(Hdr.Checksum, ' ', పరిమాణం(Hdr.Checksum));

  // Compute a checksum and set it to the checksum field.
  గుర్తులేని Chksum = 0;
  కొరకు (size_t I = 0; I < పరిమాణం(Hdr); ++I)
    Chksum += reinterpret_cast<uint8_t *>(&Hdr)[I];
  snprintf(Hdr.Checksum, పరిమాణం(Hdr.Checksum), "%06o", Chksum);
}

// Create a tar header and write it to a given output stream.
స్థిర శూన్య write_Pax_శీర్షిక(raw_fd_ostream &OS, StringRef మార్గం) {
  // A PAX header consists of a 512-byte header followed
  // by key-value strings. First, create key-value strings.
  std::string PaxAttr = formatPax("path", మార్గం);

  // Create a 512-byte header.
  Ustar_శీర్షిక Hdr = make_Ustar_శీర్షిక();
  snprintf(Hdr.కొలత, పరిమాణం(Hdr.కొలత), "%011zo", PaxAttr.కొలత_2());
  Hdr.TypeFlag = 'x'; // PAX magic
  computeChecksum(Hdr);

  // Write them down.
  OS << StringRef(reinterpret_cast<అక్షరం *>(&Hdr), పరిమాణం(Hdr));
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
స్థిర తార్కిక విభజించు_Ustar(StringRef మార్గం, StringRef &ఉపసర్గ, StringRef &Name) {
  ఒకవేళ (మార్గం.కొలత_2() < పరిమాణం(Ustar_శీర్షిక::Name)) {
    ఉపసర్గ = "";
    Name = మార్గం;
    తిరిగి నిజం;
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
  స్థిరరాశి పూర్ణాంకం Max_ఉపసర్గ = 137;
  size_t Sep = మార్గం.rfind('/', Max_ఉపసర్గ + 1);
  ఒకవేళ (Sep == StringRef::npos)
    తిరిగి అబద్ధం;
  ఒకవేళ (మార్గం.కొలత_2() - Sep - 1 >= పరిమాణం(Ustar_శీర్షిక::Name))
    తిరిగి అబద్ధం;

  ఉపసర్గ = మార్గం.substr(0, Sep);
  Name = మార్గం.substr(Sep + 1);
  తిరిగి నిజం;
}

// The PAX header is an extended format, so a PAX header needs
// to be followed by a "real" header.
స్థిర శూన్య write_Ustar_శీర్షిక(raw_fd_ostream &OS, StringRef ఉపసర్గ,
                             StringRef Name, size_t కొలత) {
  Ustar_శీర్షిక Hdr = make_Ustar_శీర్షిక();
  జ్ఞాపకం_నకలు(Hdr.Name, Name.దత్తాంశం(), Name.కొలత_2());
  జ్ఞాపకం_నకలు(Hdr.బహుళకం, "0000664", 8);
  snprintf(Hdr.కొలత, పరిమాణం(Hdr.కొలత), "%011zo", కొలత);
  జ్ఞాపకం_నకలు(Hdr.ఉపసర్గ, ఉపసర్గ.దత్తాంశం(), ఉపసర్గ.కొలత_2());
  computeChecksum(Hdr);
  OS << StringRef(reinterpret_cast<అక్షరం *>(&Hdr), పరిమాణం(Hdr));
}

// Creates a TarWriter instance and returns it.
Expected<std::unique_ptr<TarWriter>> TarWriter::create(StringRef Output_మార్గం,
                                                       StringRef పునాది_Dir) {
  ఉపయోగించి నామాకాశ sys::fs;
  పూర్ణాంకం FD;
  ఒకవేళ (std::error_సంకేతం EC =
          openFileForWrite(Output_మార్గం, FD, CD_CreateAlways, యొక్క_None))
    తిరిగి make_error<StringError>("cannot open " + Output_మార్గం, EC);
  తిరిగి std::unique_ptr<TarWriter>(కొత్త TarWriter(FD, పునాది_Dir));
}

TarWriter::TarWriter(పూర్ణాంకం FD, StringRef పునాది_Dir)
    : OS(FD, /*shouldClose=*/నిజం, /*unbuffered=*/అబద్ధం),
      పునాది_Dir(std::string(పునాది_Dir)) {}

// Append a given file to an archive.
శూన్య TarWriter::append(StringRef మార్గం, StringRef దత్తాంశం_2) {
  // Write Path and Data.
  std::string Fullpath = పునాది_Dir + "/" + sys::మార్గం_2::convert_గమ్యం_slash(మార్గం);

  // We do not want to include the same file more than once.
  ఒకవేళ (!Files.చొప్పించు(Fullpath).రెండవ_2)
    తిరిగి;

  StringRef ఉపసర్గ;
  StringRef Name;
  ఒకవేళ (విభజించు_Ustar(Fullpath, ఉపసర్గ, Name)) {
    write_Ustar_శీర్షిక(OS, ఉపసర్గ, Name, దత్తాంశం_2.కొలత_2());
  } లేకపోతే {
    write_Pax_శీర్షిక(OS, Fullpath);
    write_Ustar_శీర్షిక(OS, "", "", దత్తాంశం_2.కొలత_2());
  }

  OS << దత్తాంశం_2;
  pad(OS);

  // POSIX requires tar archives end with two null blocks.
  // Here, we write the terminator and then seek back, so that
  // the file being output is terminated correctly at any moment.
  uint64_t Pos = OS.tell();
  OS << std::string(ఖండం_కొలత * 2, '\0');
  OS.seek(Pos);
  OS.flush();
}
