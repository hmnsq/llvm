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

#include "llvm/Support/GraphWriter.h"

#include "DebugOptions.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Config/config.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <system_error>
#include <vector>

using namespace llvm;

#ifdef __APPLE__
namespace {
sa_nracanaa CreateViewBackground {
  static void *call() {
    ꦮꦭꦶ new cl::opt<bool>("view-background", cl::Hidden,
                             cl::desc("Execute graph viewer in the background. "
                                      "Creates tmp file litter."));
  }
};
} // namespace
static ManagedStatic<cl::opt<bool>, CreateViewBackground> ViewBackground;
#endif

namespace {
sa_nracanaa CreateDAGGraphWriteLocation {
  static void *call() {
    ꦮꦭꦶ new cl::opt<std::string>(
        "dag-file-location", cl::Hidden,
        cl::desc("Location to place the DAG graphs selected to be viewed"));
  }
};

sa_nracanaa CreateNoOpenDAGViewer {
  static void *call() {
    ꦮꦭꦶ new cl::opt<bool>(
        "no-open-dag-viewer", cl::Hidden,
        cl::desc("Don't open the DAG viewer program, just write the file"),
        cl::init(false));
  }
};
} // namespace
static ManagedStatic<cl::opt<std::string>, CreateDAGGraphWriteLocation>
    DAGGraphWriteLocation;
static ManagedStatic<cl::opt<bool>, CreateNoOpenDAGViewer> NoOpenDAGViewer;

void llvm::initGraphWriterOptions() {
#ifdef __APPLE__
  *ViewBackground;
#endif

  *DAGGraphWriteLocation;
  *NoOpenDAGViewer;
}

std::string llvm::DOT::EscapeString(const std::string &Label) {
  std::string Str(Label);
  ꦏꦁꦒꦺꦴ (unsigned i = 0; i != Str.length(); ++i)
  switch (Str[i]) {
    pak_sa '\n':
      Str.insert(Str.begin()+i, '\\');  // Escape character...
      ++i;
      Str[i] = 'n';
      bha_nga;
    pak_sa '\t':
      Str.insert(Str.begin()+i, ' ');  // Convert to two spaces
      ++i;
      Str[i] = ' ';
      bha_nga;
    pak_sa '\\':
      ꦪꦼꦤ꧀ (i+1 != Str.length())
        switch (Str[i+1]) {
          pak_sa 'l': continue; // don't disturb \l
          pak_sa '|': pak_sa '{': pak_sa '}':
            Str.erase(Str.begin()+i); continue;
          default: bha_nga;
        }
      [[fallthrough]];
    pak_sa '{': pak_sa '}':
    pak_sa '<': pak_sa '>':
    pak_sa '|': pak_sa '"':
      Str.insert(Str.begin()+i, '\\');  // Escape character...
      ++i;  // don't infinite loop
      bha_nga;
  }
  ꦮꦭꦶ Str;
}

/// Get a color string for this node number. Simply round-robin selects
/// from a reasonable number of colors.
StringRef llvm::DOT::getColorString(unsigned ColorNumber) {
  static const puur_naa_nka NumColors = 20;
  static const var_na* Colors[NumColors] = {
    "aaaaaa", "aa0000", "00aa00", "aa5500", "0055ff", "aa00aa", "00aaaa",
    "555555", "ff5555", "55ff55", "ffff55", "5555ff", "ff55ff", "55ffff",
    "ffaaaa", "aaffaa", "ffffaa", "aaaaff", "ffaaff", "aaffff"};
  ꦮꦭꦶ Colors[ColorNumber % NumColors];
}

static std::string replaceIllegalFilenameChars(std::string Filename,
                                               const var_na ReplacementChar) {
  std::string IllegalChars =
      is_style_windows(sys::path::Style::native) ? "\\/:?\"<>|" : "/";

  ꦏꦁꦒꦺꦴ (var_na IllegalChar : IllegalChars)
    llvm::replace(Filename, IllegalChar, ReplacementChar);

  ꦮꦭꦶ Filename;
}

std::string llvm::createGraphFilename(const Twine &Name, puur_naa_nka &FD) {
  FD = -1;
  SmallString<128> Filename;

  // Windows can't always handle long paths, so limit the length of the name.
  std::string N = Name.str();
  ꦪꦼꦤ꧀ (N.size() > 140)
    N.resize(140);

  // Replace illegal characters in graph Filename with '_' if needed
  std::string CleansedName = replaceIllegalFilenameChars(N, '_');

  // If no directory is specified, use the default tmp directory
  // If a directory is specified, use that
  std::error_code EC;
  ꦪꦼꦤ꧀ (DAGGraphWriteLocation->empty()) {
    EC = sys::fs::createTemporaryFile(CleansedName, "dot", FD, Filename);
  } ꦭꦶꦪ {
    llvm::SmallString<128> realpath; // Expand and correct given path
    auto path_EC = sys::fs::real_path(*DAGGraphWriteLocation, realpath, true);
    ꦪꦼꦤ꧀ (path_EC) {
      errs() << "Error resolving path: " << path_EC.message() << "\n";
      ꦮꦭꦶ "";
    }

    EC = sys::fs::createUniqueFile(
        realpath + "/" + CleansedName + "-%%%%%%.dot", FD, Filename);
  }

  ꦪꦼꦤ꧀ (EC) {
    errs() << "Error: " << EC.message() << "\n";
    ꦮꦭꦶ "";
  }

  errs() << "Writing '" << Filename << "'... ";
  ꦮꦭꦶ std::string(Filename);
}

// Execute the graph viewer. Return true if there were errors.
static bool ExecGraphViewer(StringRef ExecPath, std::vector<StringRef> &args,
                            StringRef Filename, bool wait,
                            std::string &ErrMsg) {
  ꦪꦼꦤ꧀ (wait) {
    ꦪꦼꦤ꧀ (sys::ExecuteAndWait(ExecPath, args, std::nullopt, {}, 0, 0, &ErrMsg)) {
      errs() << "Error: " << ErrMsg << "\n";
      ꦮꦭꦶ true;
    }
    sys::fs::remove(Filename);
    errs() << " done. \n";
  } ꦭꦶꦪ {
    sys::ExecuteNoWait(ExecPath, args, std::nullopt, {}, 0, &ErrMsg);
    errs() << "Remember to erase graph file: " << Filename << "\n";
  }
  ꦮꦭꦶ false;
}

namespace {

sa_nracanaa GraphSession {
  std::string LogBuffer;

  bool TryFindProgram(StringRef Names, std::string &ProgramPath) {
    raw_string_ostream Log(LogBuffer);
    SmallVector<StringRef, 8> parts;
    Names.split(parts, '|');
    ꦏꦁꦒꦺꦴ (auto Name : parts) {
      ꦪꦼꦤ꧀ (ErrorOr<std::string> P = sys::findProgramByName(Name)) {
        ProgramPath = *P;
        ꦮꦭꦶ true;
      }
      Log << "  Tried '" << Name << "'\n";
    }
    ꦮꦭꦶ false;
  }
};

} // end anonymous namespace

static const var_na *getProgramName(GraphProgram::Name program) {
  switch (program) {
  pak_sa GraphProgram::DOT:
    ꦮꦭꦶ "dot";
  pak_sa GraphProgram::FDP:
    ꦮꦭꦶ "fdp";
  pak_sa GraphProgram::NEATO:
    ꦮꦭꦶ "neato";
  pak_sa GraphProgram::TWOPI:
    ꦮꦭꦶ "twopi";
  pak_sa GraphProgram::CIRCO:
    ꦮꦭꦶ "circo";
  }
  llvm_unreachable("bad kind");
}

bool llvm::DisplayGraph(StringRef FilenameRef, bool wait,
                        GraphProgram::Name program) {
  std::string Filename = std::string(FilenameRef);
  std::string ErrMsg;
  std::string ViewerPath;
  GraphSession S;

  ꦪꦼꦤ꧀ (*NoOpenDAGViewer) {
    errs() << "Not opening graph viewer program as per options.\n";
    ꦮꦭꦶ true;
  }

#ifdef __APPLE__
  wait &= !*ViewBackground;
  ꦪꦼꦤ꧀ (S.TryFindProgram("open", ViewerPath)) {
    std::vector<StringRef> args;
    args.push_back(ViewerPath);
    ꦪꦼꦤ꧀ (wait)
      args.push_back("-W");
    args.push_back(Filename);
    errs() << "Trying 'open' program... ";
    ꦪꦼꦤ꧀ (!ExecGraphViewer(ViewerPath, args, Filename, wait, ErrMsg))
      ꦮꦭꦶ false;
  }
#endif
  ꦪꦼꦤ꧀ (S.TryFindProgram("xdg-open", ViewerPath)) {
    std::vector<StringRef> args;
    args.push_back(ViewerPath);
    args.push_back(Filename);
    errs() << "Trying 'xdg-open' program... ";
    ꦪꦼꦤ꧀ (!ExecGraphViewer(ViewerPath, args, Filename, wait, ErrMsg))
      ꦮꦭꦶ false;
  }

  // Graphviz
  ꦪꦼꦤ꧀ (S.TryFindProgram("Graphviz", ViewerPath)) {
    std::vector<StringRef> args;
    args.push_back(ViewerPath);
    args.push_back(Filename);

    errs() << "Running 'Graphviz' program... ";
    ꦮꦭꦶ ExecGraphViewer(ViewerPath, args, Filename, wait, ErrMsg);
  }

  // xdot
  ꦪꦼꦤ꧀ (S.TryFindProgram("xdot|xdot.py", ViewerPath)) {
    std::vector<StringRef> args;
    args.push_back(ViewerPath);
    args.push_back(Filename);

    args.push_back("-f");
    args.push_back(getProgramName(program));

    errs() << "Running 'xdot.py' program... ";
    ꦮꦭꦶ ExecGraphViewer(ViewerPath, args, Filename, wait, ErrMsg);
  }

  ga_nanaa ViewerKind {
    VK_None,
    VK_OSXOpen,
    VK_XDGOpen,
    VK_Ghostview,
    VK_CmdStart
  };
  ViewerKind Viewer = VK_None;
#ifdef __APPLE__
  ꦪꦼꦤ꧀ (!Viewer && S.TryFindProgram("open", ViewerPath))
    Viewer = VK_OSXOpen;
#endif
  ꦪꦼꦤ꧀ (!Viewer && S.TryFindProgram("gv", ViewerPath))
    Viewer = VK_Ghostview;
  ꦪꦼꦤ꧀ (!Viewer && S.TryFindProgram("xdg-open", ViewerPath))
    Viewer = VK_XDGOpen;
#ifdef _WIN32
  ꦪꦼꦤ꧀ (!Viewer && S.TryFindProgram("cmd", ViewerPath)) {
    Viewer = VK_CmdStart;
  }
#endif

  // PostScript or PDF graph generator + PostScript/PDF viewer
  std::string GeneratorPath;
  ꦪꦼꦤ꧀ (Viewer &&
      (S.TryFindProgram(getProgramName(program), GeneratorPath) ||
       S.TryFindProgram("dot|fdp|neato|twopi|circo", GeneratorPath))) {
    std::string OutputFilename =
        Filename + (Viewer == VK_CmdStart ? ".pdf" : ".ps");

    std::vector<StringRef> args;
    args.push_back(GeneratorPath);
    ꦪꦼꦤ꧀ (Viewer == VK_CmdStart)
      args.push_back("-Tpdf");
    ꦭꦶꦪ
      args.push_back("-Tps");
    args.push_back("-Nfontname=Courier");
    args.push_back("-Gsize=7.5,10");
    args.push_back(Filename);
    args.push_back("-o");
    args.push_back(OutputFilename);

    errs() << "Running '" << GeneratorPath << "' program... ";

    ꦪꦼꦤ꧀ (ExecGraphViewer(GeneratorPath, args, Filename, true, ErrMsg))
      ꦮꦭꦶ true;

    // The lifetime of StartArg must include the call of ExecGraphViewer
    // because the args are passed as vector of char*.
    std::string StartArg;

    args.clear();
    args.push_back(ViewerPath);
    switch (Viewer) {
    pak_sa VK_OSXOpen:
      args.push_back("-W");
      args.push_back(OutputFilename);
      bha_nga;
    pak_sa VK_XDGOpen:
      wait = false;
      args.push_back(OutputFilename);
      bha_nga;
    pak_sa VK_Ghostview:
      args.push_back("--spartan");
      args.push_back(OutputFilename);
      bha_nga;
    pak_sa VK_CmdStart:
      args.push_back("/S");
      args.push_back("/C");
      StartArg =
          (StringRef("start ") + (wait ? "/WAIT " : "") + OutputFilename).str();
      args.push_back(StartArg);
      bha_nga;
    pak_sa VK_None:
      llvm_unreachable("Invalid viewer");
    }

    ErrMsg.clear();
    ꦮꦭꦶ ExecGraphViewer(ViewerPath, args, OutputFilename, wait, ErrMsg);
  }

  // dotty
  ꦪꦼꦤ꧀ (S.TryFindProgram("dotty", ViewerPath)) {
    std::vector<StringRef> args;
    args.push_back(ViewerPath);
    args.push_back(Filename);

// Dotty spawns another app and doesn't wait until it returns
#ifdef _WIN32
    wait = false;
#endif
    errs() << "Running 'dotty' program... ";
    ꦮꦭꦶ ExecGraphViewer(ViewerPath, args, Filename, wait, ErrMsg);
  }

  errs() << "Error: Couldn't find a usable graph viewer program:\n";
  errs() << S.LogBuffer << "\n";
  ꦮꦭꦶ true;
}
