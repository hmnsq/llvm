//===- GraphWriter.cpp - Implements GraphWriter support routines ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements misc. GraphWriter support routines.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/GraphWriter.h"

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorOr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <system_error>
#ಅನ್ತರ್ಭಾವ <vector>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
ನಾಮಾಕಾಶ {
ರಚನೆ CreateViewBackground {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<ತಾರ್ಕಿಕ>("view-background", cl::Hidden,
                             cl::desc("Execute graph viewer in the background. "
                                      "Creates tmp file litter."));
  }
};
} // namespace
ಸ್ಥಿರ ManagedStatic<cl::opt<ತಾರ್ಕಿಕ>, CreateViewBackground> ViewBackground;
#ಆದರೆ_ಅಂತ್ಯ

ನಾಮಾಕಾಶ {
ರಚನೆ CreateDAGGraphWriteLocation {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<std::string>(
        "dag-file-location", cl::Hidden,
        cl::desc("Location to place the DAG graphs selected to be viewed"));
  }
};

ರಚನೆ CreateNoOpenDAGViewer {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<ತಾರ್ಕಿಕ>(
        "no-open-dag-viewer", cl::Hidden,
        cl::desc("Don't open the DAG viewer program, just write the file"),
        cl::init(ಸುಳ್ಳು));
  }
};
} // namespace
ಸ್ಥಿರ ManagedStatic<cl::opt<std::string>, CreateDAGGraphWriteLocation>
    DAGGraphWriteLocation;
ಸ್ಥಿರ ManagedStatic<cl::opt<ತಾರ್ಕಿಕ>, CreateNoOpenDAGViewer> NoOpenDAGViewer;

ಶೂನ್ಯ llvm::init_ನಕ್ಷೆ_Writer_Options() {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
  *ViewBackground;
#ಆದರೆ_ಅಂತ್ಯ

  *DAGGraphWriteLocation;
  *NoOpenDAGViewer;
}

std::string llvm::DOT::EscapeString(ಸ್ಥಿರಾಂಕ std::string &Label) {
  std::string Str(Label);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != Str.ಉದ್ದಳತೆ_2(); ++i)
  ಆಯ್ಕೆ (Str[i]) {
    ಸ್ಥಿತಿ '\n':
      Str.ಸೇರಿಸು(Str.begin()+i, '\\');  // Escape character...
      ++i;
      Str[i] = 'n';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\t':
      Str.ಸೇರಿಸು(Str.begin()+i, ' ');  // Convert to two spaces
      ++i;
      Str[i] = ' ';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\\':
      ಆದರೆ (i+1 != Str.ಉದ್ದಳತೆ_2())
        ಆಯ್ಕೆ (Str[i+1]) {
          ಸ್ಥಿತಿ 'l': ಮುಂದುವರಿ; // don't disturb \l
          ಸ್ಥಿತಿ '|': ಸ್ಥಿತಿ '{': ಸ್ಥಿತಿ '}':
            Str.erase(Str.begin()+i); ಮುಂದುವರಿ;
          ಪೂರ್ವನಿಯೋಜಿತ: ನಿಲ್ಲಿಸು;
        }
      [[fallthrough]];
    ಸ್ಥಿತಿ '{': ಸ್ಥಿತಿ '}':
    ಸ್ಥಿತಿ '<': ಸ್ಥಿತಿ '>':
    ಸ್ಥಿತಿ '|': ಸ್ಥಿತಿ '"':
      Str.ಸೇರಿಸು(Str.begin()+i, '\\');  // Escape character...
      ++i;  // don't infinite loop
      ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ Str;
}

/// Get a color string for this node number. Simply round-robin selects
/// from a reasonable number of colors.
StringRef llvm::DOT::getColorString(ಚಿಹ್ನೆರಹಿತ Color_ಸಂಖ್ಯೆ) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_Colors = 20;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* Colors[ಅಂಕೆ_Colors] = {
    "aaaaaa", "aa0000", "00aa00", "aa5500", "0055ff", "aa00aa", "00aaaa",
    "555555", "ff5555", "55ff55", "ffff55", "5555ff", "ff55ff", "55ffff",
    "ffaaaa", "aaffaa", "ffffaa", "aaaaff", "ffaaff", "aaffff"};
  ಹಿಂತಿರುಗಿ Colors[Color_ಸಂಖ್ಯೆ % ಅಂಕೆ_Colors];
}

ಸ್ಥಿರ std::string replaceIllegalFilenameChars(std::string Filename,
                                               ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ReplacementChar) {
  std::string IllegalChars =
      is_style_windows(sys::ಮಾರ್ಗ_2::Style::native) ? "\\/:?\"<>|" : "/";

  ಗಾಗಿ (ಅಕ್ಷರ IllegalChar : IllegalChars)
    llvm::replace(Filename, IllegalChar, ReplacementChar);

  ಹಿಂತಿರುಗಿ Filename;
}

std::string llvm::create_ನಕ್ಷೆ_Filename(ಸ್ಥಿರಾಂಕ Twine &Name, ಪೂರ್ಣಾಂಕ &FD) {
  FD = -1;
  SmallString<128> Filename;

  // Windows can't always handle long paths, so limit the length of the name.
  std::string N = Name.str();
  ಆದರೆ (N.ಗಾತ್ರ_3() > 140)
    N.resize(140);

  // Replace illegal characters in graph Filename with '_' if needed
  std::string CleansedName = replaceIllegalFilenameChars(N, '_');

  // If no directory is specified, use the default tmp directory
  // If a directory is specified, use that
  std::error_ಸಂಕೇತ EC;
  ಆದರೆ (DAGGraphWriteLocation->ಖಾಲಿಗಳು()) {
    EC = sys::fs::createTemporaryFile(CleansedName, "dot", FD, Filename);
  } ಇಲ್ಲದಿದ್ದರೆ {
    llvm::SmallString<128> realpath; // Expand and correct given path
    ಸ್ವಯಂಚಾಲಿತ ಮಾರ್ಗ_EC = sys::fs::real_ಮಾರ್ಗ(*DAGGraphWriteLocation, realpath, ನಿಜ);
    ಆದರೆ (ಮಾರ್ಗ_EC) {
      errs() << "Error resolving path: " << ಮಾರ್ಗ_EC.ಸಂದೇಶ_2() << "\n";
      ಹಿಂತಿರುಗಿ "";
    }

    EC = sys::fs::createUniqueFile(
        realpath + "/" + CleansedName + "-%%%%%%.dot", FD, Filename);
  }

  ಆದರೆ (EC) {
    errs() << "Error: " << EC.ಸಂದೇಶ_2() << "\n";
    ಹಿಂತಿರುಗಿ "";
  }

  errs() << "Writing '" << Filename << "'... ";
  ಹಿಂತಿರುಗಿ std::string(Filename);
}

// Execute the graph viewer. Return true if there were errors.
ಸ್ಥಿರ ತಾರ್ಕಿಕ Exec_ನಕ್ಷೆ_Viewer(StringRef Exec_ಮಾರ್ಗ, std::ಸದಿಶ<StringRef> &ವಾದಗಳು_2,
                            StringRef Filename, ತಾರ್ಕಿಕ wait,
                            std::string &Err_ಸಂದೇಶ) {
  ಆದರೆ (wait) {
    ಆದರೆ (sys::ExecuteAndWait(Exec_ಮಾರ್ಗ, ವಾದಗಳು_2, std::nullopt, {}, 0, 0, &Err_ಸಂದೇಶ)) {
      errs() << "Error: " << Err_ಸಂದೇಶ << "\n";
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    sys::fs::remove(Filename);
    errs() << " done. \n";
  } ಇಲ್ಲದಿದ್ದರೆ {
    sys::ExecuteNoWait(Exec_ಮಾರ್ಗ, ವಾದಗಳು_2, std::nullopt, {}, 0, &Err_ಸಂದೇಶ);
    errs() << "Remember to erase graph file: " << Filename << "\n";
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ನಾಮಾಕಾಶ {

ರಚನೆ ನಕ್ಷೆ_Session {
  std::string Log_ಬಫರ್;

  ತಾರ್ಕಿಕ Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ(StringRef ಹೆಸರುಗಳು, std::string &ಕಾರ್ಯಕ್ರಮ_ಮಾರ್ಗ) {
    raw_string_ostream Log(Log_ಬಫರ್);
    Small_ಸದಿಶ<StringRef, 8> parts;
    ಹೆಸರುಗಳು.ವಿಭಜಿಸು(parts, '|');
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ Name : parts) {
      ಆದರೆ (ErrorOr<std::string> P = sys::ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ_By_Name(Name)) {
        ಕಾರ್ಯಕ್ರಮ_ಮಾರ್ಗ = *P;
        ಹಿಂತಿರುಗಿ ನಿಜ;
      }
      Log << "  Tried '" << Name << "'\n";
    }
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
};

} // end anonymous namespace

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get_ಕಾರ್ಯಕ್ರಮ_Name(ನಕ್ಷೆ_ಕಾರ್ಯಕ್ರಮ::Name ಕಾರ್ಯಕ್ರಮ) {
  ಆಯ್ಕೆ (ಕಾರ್ಯಕ್ರಮ) {
  ಸ್ಥಿತಿ ನಕ್ಷೆ_ಕಾರ್ಯಕ್ರಮ::DOT:
    ಹಿಂತಿರುಗಿ "dot";
  ಸ್ಥಿತಿ ನಕ್ಷೆ_ಕಾರ್ಯಕ್ರಮ::FDP:
    ಹಿಂತಿರುಗಿ "fdp";
  ಸ್ಥಿತಿ ನಕ್ಷೆ_ಕಾರ್ಯಕ್ರಮ::NEATO:
    ಹಿಂತಿರುಗಿ "neato";
  ಸ್ಥಿತಿ ನಕ್ಷೆ_ಕಾರ್ಯಕ್ರಮ::TWOPI:
    ಹಿಂತಿರುಗಿ "twopi";
  ಸ್ಥಿತಿ ನಕ್ಷೆ_ಕಾರ್ಯಕ್ರಮ::CIRCO:
    ಹಿಂತಿರುಗಿ "circo";
  }
  llvm_unreachable("bad kind");
}

ತಾರ್ಕಿಕ llvm::Display_ನಕ್ಷೆ(StringRef FilenameRef, ತಾರ್ಕಿಕ wait,
                        ನಕ್ಷೆ_ಕಾರ್ಯಕ್ರಮ::Name ಕಾರ್ಯಕ್ರಮ) {
  std::string Filename = std::string(FilenameRef);
  std::string Err_ಸಂದೇಶ;
  std::string Viewer_ಮಾರ್ಗ;
  ನಕ್ಷೆ_Session S;

  ಆದರೆ (*NoOpenDAGViewer) {
    errs() << "Not opening graph viewer program as per options.\n";
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
  wait &= !*ViewBackground;
  ಆದರೆ (S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("open", Viewer_ಮಾರ್ಗ)) {
    std::ಸದಿಶ<StringRef> ವಾದಗಳು_2;
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Viewer_ಮಾರ್ಗ);
    ಆದರೆ (wait)
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("-W");
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Filename);
    errs() << "Trying 'open' program... ";
    ಆದರೆ (!Exec_ನಕ್ಷೆ_Viewer(Viewer_ಮಾರ್ಗ, ವಾದಗಳು_2, Filename, wait, Err_ಸಂದೇಶ))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("xdg-open", Viewer_ಮಾರ್ಗ)) {
    std::ಸದಿಶ<StringRef> ವಾದಗಳು_2;
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Viewer_ಮಾರ್ಗ);
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Filename);
    errs() << "Trying 'xdg-open' program... ";
    ಆದರೆ (!Exec_ನಕ್ಷೆ_Viewer(Viewer_ಮಾರ್ಗ, ವಾದಗಳು_2, Filename, wait, Err_ಸಂದೇಶ))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // Graphviz
  ಆದರೆ (S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("Graphviz", Viewer_ಮಾರ್ಗ)) {
    std::ಸದಿಶ<StringRef> ವಾದಗಳು_2;
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Viewer_ಮಾರ್ಗ);
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Filename);

    errs() << "Running 'Graphviz' program... ";
    ಹಿಂತಿರುಗಿ Exec_ನಕ್ಷೆ_Viewer(Viewer_ಮಾರ್ಗ, ವಾದಗಳು_2, Filename, wait, Err_ಸಂದೇಶ);
  }

  // xdot
  ಆದರೆ (S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("xdot|xdot.py", Viewer_ಮಾರ್ಗ)) {
    std::ಸದಿಶ<StringRef> ವಾದಗಳು_2;
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Viewer_ಮಾರ್ಗ);
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Filename);

    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("-f");
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(get_ಕಾರ್ಯಕ್ರಮ_Name(ಕಾರ್ಯಕ್ರಮ));

    errs() << "Running 'xdot.py' program... ";
    ಹಿಂತಿರುಗಿ Exec_ನಕ್ಷೆ_Viewer(Viewer_ಮಾರ್ಗ, ವಾದಗಳು_2, Filename, wait, Err_ಸಂದೇಶ);
  }

  ಎಣಿಕೆ Viewer_ಬಗೆ {
    VK_None,
    VK_OSXOpen,
    VK_XDGOpen,
    VK_Ghostview,
    VK_Cmd_ಆರಂಭ
  };
  Viewer_ಬಗೆ Viewer = VK_None;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
  ಆದರೆ (!Viewer && S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("open", Viewer_ಮಾರ್ಗ))
    Viewer = VK_OSXOpen;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (!Viewer && S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("gv", Viewer_ಮಾರ್ಗ))
    Viewer = VK_Ghostview;
  ಆದರೆ (!Viewer && S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("xdg-open", Viewer_ಮಾರ್ಗ))
    Viewer = VK_XDGOpen;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಆದರೆ (!Viewer && S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("cmd", Viewer_ಮಾರ್ಗ)) {
    Viewer = VK_Cmd_ಆರಂಭ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  // PostScript or PDF graph generator + PostScript/PDF viewer
  std::string Generator_ಮಾರ್ಗ;
  ಆದರೆ (Viewer &&
      (S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ(get_ಕಾರ್ಯಕ್ರಮ_Name(ಕಾರ್ಯಕ್ರಮ), Generator_ಮಾರ್ಗ) ||
       S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("dot|fdp|neato|twopi|circo", Generator_ಮಾರ್ಗ))) {
    std::string OutputFilename =
        Filename + (Viewer == VK_Cmd_ಆರಂಭ ? ".pdf" : ".ps");

    std::ಸದಿಶ<StringRef> ವಾದಗಳು_2;
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Generator_ಮಾರ್ಗ);
    ಆದರೆ (Viewer == VK_Cmd_ಆರಂಭ)
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("-Tpdf");
    ಇಲ್ಲದಿದ್ದರೆ
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("-Tps");
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("-Nfontname=Courier");
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("-Gsize=7.5,10");
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Filename);
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("-o");
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(OutputFilename);

    errs() << "Running '" << Generator_ಮಾರ್ಗ << "' program... ";

    ಆದರೆ (Exec_ನಕ್ಷೆ_Viewer(Generator_ಮಾರ್ಗ, ವಾದಗಳು_2, Filename, ನಿಜ, Err_ಸಂದೇಶ))
      ಹಿಂತಿರುಗಿ ನಿಜ;

    // The lifetime of StartArg must include the call of ExecGraphViewer
    // because the args are passed as vector of char*.
    std::string ಆರಂಭ_Arg;

    ವಾದಗಳು_2.clear();
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Viewer_ಮಾರ್ಗ);
    ಆಯ್ಕೆ (Viewer) {
    ಸ್ಥಿತಿ VK_OSXOpen:
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("-W");
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(OutputFilename);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ VK_XDGOpen:
      wait = ಸುಳ್ಳು;
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(OutputFilename);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ VK_Ghostview:
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("--spartan");
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(OutputFilename);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ VK_Cmd_ಆರಂಭ:
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("/S");
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ("/C");
      ಆರಂಭ_Arg =
          (StringRef("start ") + (wait ? "/WAIT " : "") + OutputFilename).str();
      ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(ಆರಂಭ_Arg);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ VK_None:
      llvm_unreachable("Invalid viewer");
    }

    Err_ಸಂದೇಶ.clear();
    ಹಿಂತಿರುಗಿ Exec_ನಕ್ಷೆ_Viewer(Viewer_ಮಾರ್ಗ, ವಾದಗಳು_2, OutputFilename, wait, Err_ಸಂದೇಶ);
  }

  // dotty
  ಆದರೆ (S.Try_ಕಂಡುಹಿಡಿ_ಕಾರ್ಯಕ್ರಮ("dotty", Viewer_ಮಾರ್ಗ)) {
    std::ಸದಿಶ<StringRef> ವಾದಗಳು_2;
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Viewer_ಮಾರ್ಗ);
    ವಾದಗಳು_2.ತಳ್ಳು_ಹಿಂಭಾಗ(Filename);

// Dotty spawns another app and doesn't wait until it returns
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
    wait = ಸುಳ್ಳು;
#ಆದರೆ_ಅಂತ್ಯ
    errs() << "Running 'dotty' program... ";
    ಹಿಂತಿರುಗಿ Exec_ನಕ್ಷೆ_Viewer(Viewer_ಮಾರ್ಗ, ವಾದಗಳು_2, Filename, wait, Err_ಸಂದೇಶ);
  }

  errs() << "Error: Couldn't find a usable graph viewer program:\n";
  errs() << S.Log_ಬಫರ್ << "\n";
  ಹಿಂತಿರುಗಿ ನಿಜ;
}
