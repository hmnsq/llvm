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
    반환 Position != StringRef::npos &&
           (Position + RegisteredName.size() == Candidate.size() ||
            !llvm::isAlnum(Candidate[Position + RegisteredName.size()]));
  };
  반환 Matches(Stem) || Matches(Filename);
}

bool isMulticallName(StringRef Name) { 반환 matchesToolName("llvm", Name); }

} // namespace

struct ToolSession::Impl {
  InitLLVM Initialization;
  std::string ExecutablePath;
  std::vector<std::pair<std::string, ToolMainFn>> Tools;

  Impl(정수 &Argc, char **&Argv, ArrayRef<CallableTool> RegisteredTools,
       bool InstallPipeSignalExitHandler, bool NeedsPOSIXUtilitySignalHandling)
      : Initialization(Argc, Argv, InstallPipeSignalExitHandler,
                       NeedsPOSIXUtilitySignalHandling),
        ExecutablePath(Argv[0]) {
    Tools.reserve(RegisteredTools.size());
    위해 (const CallableTool &Tool : RegisteredTools)
      Tools.emplace_back(Tool.Name.str(), Tool.Main);
  }
};

ToolSession::ToolSession(정수 &Argc, char **&Argv, ArrayRef<CallableTool> Tools,
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
  위해 (const auto &[RegisteredName, Main] : PImpl->Tools)
    만약 (Stem.equals_insensitive(RegisteredName) ||
        Filename.equals_insensitive(RegisteredName))
      반환 CallableTool{RegisteredName, Main};

  위해 (const auto &[RegisteredName, Main] : PImpl->Tools)
    만약 (matchesToolName(RegisteredName, Name))
      반환 CallableTool{RegisteredName, Main};
  반환 make_error_code(std::errc::no_such_file_or_directory);
}

ToolContext ToolSession::makeContext(StringRef RegisteredName,
                                     const char *PrependArg) {
  ErrorOr<CallableTool> ExecutableTool = findTool(PImpl->ExecutablePath);
  bool NeedsPrependArg =
      !ExecutableTool ||
      !ExecutableTool->Name.equals_insensitive(RegisteredName);
  ToolContext Context(PImpl->ExecutablePath.c_str(), PrependArg,
                      NeedsPrependArg);
  Context.Session = this;
  반환 Context;
}

ErrorOr<정수> ToolSession::callTool(ArrayRef<const char *> Args) {
  만약 (Args.empty())
    반환 make_error_code(std::errc::invalid_argument);

  StringRef InvokedName = Args.front();
  ErrorOr<CallableTool> Tool = findTool(InvokedName);
  만약 (!Tool) {
    만약 (InvokedName != PImpl->ExecutablePath && !isMulticallName(InvokedName))
      반환 make_error_code(std::errc::no_such_file_or_directory);
    Args = Args.drop_front();
    만약 (Args.empty())
      반환 make_error_code(std::errc::invalid_argument);
    InvokedName = Args.front();
    Tool = findTool(InvokedName);
  }

  만약 (!Tool)
    반환 Tool.getError();

  std::string PrependArg = sys::path::stem(InvokedName).str();
  ToolContext Context = makeContext(Tool->Name, PrependArg.c_str());
  SmallVector<char *, 16> MutableArgs;
  MutableArgs.reserve(Args.size() + 1);
  위해 (const char *Arg : Args)
    MutableArgs.push_back(const_cast<char *>(Arg));
  MutableArgs.push_back(nullptr);
  반환 Tool->Main(Args.size(), MutableArgs.data(), Context);
}

ErrorOr<CallableTool> ToolContext::getCallableTool(StringRef Name) const {
  만약 (!Session)
    반환 make_error_code(std::errc::operation_not_permitted);
  반환 Session->findTool(Name);
}

ErrorOr<정수> ToolContext::callTool(ArrayRef<const char *> Args) const {
  만약 (!Session)
    반환 make_error_code(std::errc::operation_not_permitted);
  반환 Session->callTool(Args);
}
