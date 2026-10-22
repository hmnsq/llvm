//===-- Driver.cpp -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/Driver.h"

#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/InitLLVM.h"
#અન્તર્ભાવ "llvm/Support/Path.h"

#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <string>
#અન્તર્ભાવ <system_error>
#અન્તર્ભાવ <utility>
#અન્તર્ભાવ <vector>

using નામાકાશ llvm;

નામાકાશ {

તાર્કિક matchesToolName(StringRef RegisteredName, StringRef InvokedName) {
  StringRef Stem = sys::path::stem(InvokedName);
  StringRef Filename = sys::path::filename(InvokedName);
  auto Matches = [RegisteredName](StringRef Candidate) {
    size_t Position = Candidate.rfind_insensitive(RegisteredName);
    પાછા Position != StringRef::npos &&
           (Position + RegisteredName.size() == Candidate.size() ||
            !llvm::isAlnum(Candidate[Position + RegisteredName.size()]));
  };
  પાછા Matches(Stem) || Matches(Filename);
}

તાર્કિક isMulticallName(StringRef Name) { પાછા matchesToolName("llvm", Name); }

} // namespace

સંરચના ToolSession::Impl {
  InitLLVM Initialization;
  std::string ExecutablePath;
  std::vector<std::pair<std::string, ToolMainFn>> Tools;

  Impl(પૂર્ણાંક &Argc, અક્ષર **&Argv, ArrayRef<CallableTool> RegisteredTools,
       તાર્કિક InstallPipeSignalExitHandler, તાર્કિક NeedsPOSIXUtilitySignalHandling)
      : Initialization(Argc, Argv, InstallPipeSignalExitHandler,
                       NeedsPOSIXUtilitySignalHandling),
        ExecutablePath(Argv[0]) {
    Tools.reserve(RegisteredTools.size());
    માટે (અચલ CallableTool &Tool : RegisteredTools)
      Tools.emplace_back(Tool.Name.str(), Tool.Main);
  }
};

ToolSession::ToolSession(પૂર્ણાંક &Argc, અક્ષર **&Argv, ArrayRef<CallableTool> Tools,
                         તાર્કિક InstallPipeSignalExitHandler,
                         તાર્કિક NeedsPOSIXUtilitySignalHandling) {
  પ્રતિજ્ઞા(Argc > 0 && Argv && Argv[0] && "ToolSession requires a valid argv[0]");
  PImpl =
      std::make_unique<Impl>(Argc, Argv, Tools, InstallPipeSignalExitHandler,
                             NeedsPOSIXUtilitySignalHandling);
}

ToolSession::~ToolSession() = મૂળભૂત;

ErrorOr<CallableTool> ToolSession::findTool(StringRef Name) અચલ {
  StringRef Stem = sys::path::stem(Name);
  StringRef Filename = sys::path::filename(Name);
  માટે (અચલ auto &[RegisteredName, Main] : PImpl->Tools)
    જો (Stem.equals_insensitive(RegisteredName) ||
        Filename.equals_insensitive(RegisteredName))
      પાછા CallableTool{RegisteredName, Main};

  માટે (અચલ auto &[RegisteredName, Main] : PImpl->Tools)
    જો (matchesToolName(RegisteredName, Name))
      પાછા CallableTool{RegisteredName, Main};
  પાછા make_error_code(std::errc::no_such_file_or_directory);
}

ToolContext ToolSession::makeContext(StringRef RegisteredName,
                                     અચલ અક્ષર *PrependArg) {
  ErrorOr<CallableTool> ExecutableTool = findTool(PImpl->ExecutablePath);
  તાર્કિક NeedsPrependArg =
      !ExecutableTool ||
      !ExecutableTool->Name.equals_insensitive(RegisteredName);
  ToolContext Context(PImpl->ExecutablePath.c_str(), PrependArg,
                      NeedsPrependArg);
  Context.Session = આ;
  પાછા Context;
}

ErrorOr<પૂર્ણાંક> ToolSession::callTool(ArrayRef<અચલ અક્ષર *> Args) {
  જો (Args.empty())
    પાછા make_error_code(std::errc::invalid_argument);

  StringRef InvokedName = Args.front();
  ErrorOr<CallableTool> Tool = findTool(InvokedName);
  જો (!Tool) {
    જો (InvokedName != PImpl->ExecutablePath && !isMulticallName(InvokedName))
      પાછા make_error_code(std::errc::no_such_file_or_directory);
    Args = Args.drop_front();
    જો (Args.empty())
      પાછા make_error_code(std::errc::invalid_argument);
    InvokedName = Args.front();
    Tool = findTool(InvokedName);
  }

  જો (!Tool)
    પાછા Tool.getError();

  std::string PrependArg = sys::path::stem(InvokedName).str();
  ToolContext Context = makeContext(Tool->Name, PrependArg.c_str());
  SmallVector<અક્ષર *, 16> MutableArgs;
  MutableArgs.reserve(Args.size() + 1);
  માટે (અચલ અક્ષર *Arg : Args)
    MutableArgs.ધકેલો_back(const_cast<અક્ષર *>(Arg));
  MutableArgs.ધકેલો_back(nullptr);
  પાછા Tool->Main(Args.size(), MutableArgs.data(), Context);
}

ErrorOr<CallableTool> ToolContext::getCallableTool(StringRef Name) અચલ {
  જો (!Session)
    પાછા make_error_code(std::errc::operation_not_permitted);
  પાછા Session->findTool(Name);
}

ErrorOr<પૂર્ણાંક> ToolContext::callTool(ArrayRef<અચલ અક્ષર *> Args) અચલ {
  જો (!Session)
    પાછા make_error_code(std::errc::operation_not_permitted);
  પાછા Session->callTool(Args);
}
