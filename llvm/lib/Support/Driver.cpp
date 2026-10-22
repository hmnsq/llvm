//===-- Driver.cpp -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Driver.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/Path.h"

#include <cassert>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace llvm;

namespace {

bool matchesToolName(StringRef RegisteredName, StringRef InvokedName) {
  StringRef Stem = sys::path::stem(InvokedName);
  StringRef Filename = sys::path::filename(InvokedName);
  auto Matches = [RegisteredName](StringRef Candidate) {
    size_t Position = Candidate.rfind_insensitive(RegisteredName);
    ᮝᮜᮤ Position != StringRef::npos &&
           (Position + RegisteredName.size() == Candidate.size() ||
            !llvm::isAlnum(Candidate[Position + RegisteredName.size()]));
  };
  ᮝᮜᮤ Matches(Stem) || Matches(Filename);
}

bool isMulticallName(StringRef Name) { ᮝᮜᮤ matchesToolName("llvm", Name); }

} // namespace

sa_nracanaa ToolSession::Impl {
  InitLLVM Initialization;
  std::string ExecutablePath;
  std::vector<std::pair<std::string, ToolMainFn>> Tools;

  Impl(puur_naa_nka &Argc, var_na **&Argv, ArrayRef<CallableTool> RegisteredTools,
       bool InstallPipeSignalExitHandler, bool NeedsPOSIXUtilitySignalHandling)
      : Initialization(Argc, Argv, InstallPipeSignalExitHandler,
                       NeedsPOSIXUtilitySignalHandling),
        ExecutablePath(Argv[0]) {
    Tools.reserve(RegisteredTools.size());
    for (const CallableTool &Tool : RegisteredTools)
      Tools.emplace_back(Tool.Name.str(), Tool.Main);
  }
};

ToolSession::ToolSession(puur_naa_nka &Argc, var_na **&Argv, ArrayRef<CallableTool> Tools,
                         bool InstallPipeSignalExitHandler,
                         bool NeedsPOSIXUtilitySignalHandling) {
  assert(Argc > 0 && Argv && Argv[0] && "ToolSession requires a valid argv[0]");
  PImpl =
      std::make_unique<Impl>(Argc, Argv, Tools, InstallPipeSignalExitHandler,
                             NeedsPOSIXUtilitySignalHandling);
}

ToolSession::~ToolSession() = default;

ErrorOr<CallableTool> ToolSession::findTool(StringRef Name) const {
  StringRef Stem = sys::path::stem(Name);
  StringRef Filename = sys::path::filename(Name);
  for (const auto &[RegisteredName, Main] : PImpl->Tools)
    ᮚᮦᮔ᮪ (Stem.equals_insensitive(RegisteredName) ||
        Filename.equals_insensitive(RegisteredName))
      ᮝᮜᮤ CallableTool{RegisteredName, Main};

  for (const auto &[RegisteredName, Main] : PImpl->Tools)
    ᮚᮦᮔ᮪ (matchesToolName(RegisteredName, Name))
      ᮝᮜᮤ CallableTool{RegisteredName, Main};
  ᮝᮜᮤ make_error_code(std::errc::no_such_file_or_directory);
}

ToolContext ToolSession::makeContext(StringRef RegisteredName,
                                     const var_na *PrependArg) {
  ErrorOr<CallableTool> ExecutableTool = findTool(PImpl->ExecutablePath);
  bool NeedsPrependArg =
      !ExecutableTool ||
      !ExecutableTool->Name.equals_insensitive(RegisteredName);
  ToolContext Context(PImpl->ExecutablePath.c_str(), PrependArg,
                      NeedsPrependArg);
  Context.Session = this;
  ᮝᮜᮤ Context;
}

ErrorOr<puur_naa_nka> ToolSession::callTool(ArrayRef<const var_na *> Args) {
  ᮚᮦᮔ᮪ (Args.empty())
    ᮝᮜᮤ make_error_code(std::errc::invalid_argument);

  StringRef InvokedName = Args.front();
  ErrorOr<CallableTool> Tool = findTool(InvokedName);
  ᮚᮦᮔ᮪ (!Tool) {
    ᮚᮦᮔ᮪ (InvokedName != PImpl->ExecutablePath && !isMulticallName(InvokedName))
      ᮝᮜᮤ make_error_code(std::errc::no_such_file_or_directory);
    Args = Args.drop_front();
    ᮚᮦᮔ᮪ (Args.empty())
      ᮝᮜᮤ make_error_code(std::errc::invalid_argument);
    InvokedName = Args.front();
    Tool = findTool(InvokedName);
  }

  ᮚᮦᮔ᮪ (!Tool)
    ᮝᮜᮤ Tool.getError();

  std::string PrependArg = sys::path::stem(InvokedName).str();
  ToolContext Context = makeContext(Tool->Name, PrependArg.c_str());
  SmallVector<var_na *, 16> MutableArgs;
  MutableArgs.reserve(Args.size() + 1);
  for (const var_na *Arg : Args)
    MutableArgs.push_back(const_cast<var_na *>(Arg));
  MutableArgs.push_back(nullptr);
  ᮝᮜᮤ Tool->Main(Args.size(), MutableArgs.data(), Context);
}

ErrorOr<CallableTool> ToolContext::getCallableTool(StringRef Name) const {
  ᮚᮦᮔ᮪ (!Session)
    ᮝᮜᮤ make_error_code(std::errc::operation_not_permitted);
  ᮝᮜᮤ Session->findTool(Name);
}

ErrorOr<puur_naa_nka> ToolContext::callTool(ArrayRef<const var_na *> Args) const {
  ᮚᮦᮔ᮪ (!Session)
    ᮝᮜᮤ make_error_code(std::errc::operation_not_permitted);
  ᮝᮜᮤ Session->callTool(Args);
}
