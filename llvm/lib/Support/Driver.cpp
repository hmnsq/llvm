//===-- Driver.cpp -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/Driver.h"

#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/InitLLVM.h"
#समावेश "llvm/Support/Path.h"

#समावेश <cassert>
#समावेश <string>
#समावेश <system_error>
#समावेश <utility>
#समावेश <vector>

using namespace llvm;

namespace {

bool matchesToolName(StringRef RegisteredName, StringRef InvokedName) {
  StringRef Stem = sys::path::stem(InvokedName);
  StringRef Filename = sys::path::filename(InvokedName);
  स्वतः Matches = [RegisteredName](StringRef Candidate) {
    माप_प्रकार Position = Candidate.rfind_insensitive(RegisteredName);
    वापस Position != StringRef::npos &&
           (Position + RegisteredName.आकार_2() == Candidate.आकार_2() ||
            !llvm::isAlnum(Candidate[Position + RegisteredName.आकार_2()]));
  };
  वापस Matches(Stem) || Matches(Filename);
}

bool isMulticallName(StringRef Name) { वापस matchesToolName("llvm", Name); }

} // namespace

काष्ठा ToolSession::Impl {
  InitLLVM Initialization;
  std::string ExecutablePath;
  std::vector<std::pair<std::string, ToolMainFn>> Tools;

  Impl(पूर्णांक &Argc, अक्षर **&Argv, ArrayRef<CallableTool> RegisteredTools,
       bool InstallPipeSignalExitHandler, bool NeedsPOSIXUtilitySignalHandling)
      : Initialization(Argc, Argv, InstallPipeSignalExitHandler,
                       NeedsPOSIXUtilitySignalHandling),
        ExecutablePath(Argv[0]) {
    Tools.reserve(RegisteredTools.आकार_2());
    क्रम (स्थिर CallableTool &Tool : RegisteredTools)
      Tools.emplace_back(Tool.Name.str(), Tool.Main);
  }
};

ToolSession::ToolSession(पूर्णांक &Argc, अक्षर **&Argv, ArrayRef<CallableTool> Tools,
                         bool InstallPipeSignalExitHandler,
                         bool NeedsPOSIXUtilitySignalHandling) {
  निश्चित(Argc > 0 && Argv && Argv[0] && "ToolSession requires a valid argv[0]");
  PImpl =
      std::make_unique<Impl>(Argc, Argv, Tools, InstallPipeSignalExitHandler,
                             NeedsPOSIXUtilitySignalHandling);
}

ToolSession::~ToolSession() = शेष;

ErrorOr<CallableTool> ToolSession::findTool(StringRef Name) स्थिर {
  StringRef Stem = sys::path::stem(Name);
  StringRef Filename = sys::path::filename(Name);
  क्रम (स्थिर स्वतः &[RegisteredName, Main] : PImpl->Tools)
    अगर (Stem.equals_insensitive(RegisteredName) ||
        Filename.equals_insensitive(RegisteredName))
      वापस CallableTool{RegisteredName, Main};

  क्रम (स्थिर स्वतः &[RegisteredName, Main] : PImpl->Tools)
    अगर (matchesToolName(RegisteredName, Name))
      वापस CallableTool{RegisteredName, Main};
  वापस make_error_code(std::errc::no_such_file_or_directory);
}

ToolContext ToolSession::makeContext(StringRef RegisteredName,
                                     स्थिर अक्षर *PrependArg) {
  ErrorOr<CallableTool> ExecutableTool = findTool(PImpl->ExecutablePath);
  bool NeedsPrependArg =
      !ExecutableTool ||
      !ExecutableTool->Name.equals_insensitive(RegisteredName);
  ToolContext Context(PImpl->ExecutablePath.c_str(), PrependArg,
                      NeedsPrependArg);
  Context.Session = यह;
  वापस Context;
}

ErrorOr<पूर्णांक> ToolSession::callTool(ArrayRef<स्थिर अक्षर *> Args) {
  अगर (Args.empty())
    वापस make_error_code(std::errc::invalid_argument);

  StringRef InvokedName = Args.front();
  ErrorOr<CallableTool> Tool = findTool(InvokedName);
  अगर (!Tool) {
    अगर (InvokedName != PImpl->ExecutablePath && !isMulticallName(InvokedName))
      वापस make_error_code(std::errc::no_such_file_or_directory);
    Args = Args.drop_front();
    अगर (Args.empty())
      वापस make_error_code(std::errc::invalid_argument);
    InvokedName = Args.front();
    Tool = findTool(InvokedName);
  }

  अगर (!Tool)
    वापस Tool.getError();

  std::string PrependArg = sys::path::stem(InvokedName).str();
  ToolContext Context = makeContext(Tool->Name, PrependArg.c_str());
  SmallVector<अक्षर *, 16> MutableArgs;
  MutableArgs.reserve(Args.आकार_2() + 1);
  क्रम (स्थिर अक्षर *Arg : Args)
    MutableArgs.धकेलो_back(const_cast<अक्षर *>(Arg));
  MutableArgs.धकेलो_back(nullptr);
  वापस Tool->Main(Args.आकार_2(), MutableArgs.तथ्य(), Context);
}

ErrorOr<CallableTool> ToolContext::getCallableTool(StringRef Name) स्थिर {
  अगर (!Session)
    वापस make_error_code(std::errc::operation_not_permitted);
  वापस Session->findTool(Name);
}

ErrorOr<पूर्णांक> ToolContext::callTool(ArrayRef<स्थिर अक्षर *> Args) स्थिर {
  अगर (!Session)
    वापस make_error_code(std::errc::operation_not_permitted);
  वापस Session->callTool(Args);
}
