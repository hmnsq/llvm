//===-- Driver.cpp -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/Driver.h"

#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/InitLLVM.h"
#ضمن "llvm/Support/Path.h"

#ضمن <cassert>
#ضمن <string>
#ضمن <system_error>
#ضمن <utility>
#ضمن <vector>

using نطاق llvm;

نطاق {

منطقي matchesToolName(StringRef RegisteredName, StringRef InvokedName) {
  StringRef Stem = sys::path::stem(InvokedName);
  StringRef Filename = sys::path::filename(InvokedName);
  auto Matches = [RegisteredName](StringRef Candidate) {
    size_t Position = Candidate.rfind_insensitive(RegisteredName);
    أرجع Position != StringRef::npos &&
           (Position + RegisteredName.size() == Candidate.size() ||
            !llvm::isAlnum(Candidate[Position + RegisteredName.size()]));
  };
  أرجع Matches(Stem) || Matches(Filename);
}

منطقي isMulticallName(StringRef Name) { أرجع matchesToolName("llvm", Name); }

} // namespace

بنية ToolSession::Impl {
  InitLLVM Initialization;
  std::string ExecutablePath;
  std::vector<std::pair<std::string, ToolMainFn>> Tools;

  Impl(صحيح &Argc, حرف **&Argv, ArrayRef<CallableTool> RegisteredTools,
       منطقي InstallPipeSignalExitHandler, منطقي NeedsPOSIXUtilitySignalHandling)
      : Initialization(Argc, Argv, InstallPipeSignalExitHandler,
                       NeedsPOSIXUtilitySignalHandling),
        ExecutablePath(Argv[0]) {
    Tools.reserve(RegisteredTools.size());
    لكل (ثابت CallableTool &Tool : RegisteredTools)
      Tools.emplace_back(Tool.Name.str(), Tool.Main);
  }
};

ToolSession::ToolSession(صحيح &Argc, حرف **&Argv, ArrayRef<CallableTool> Tools,
                         منطقي InstallPipeSignalExitHandler,
                         منطقي NeedsPOSIXUtilitySignalHandling) {
  أكد(Argc > 0 && Argv && Argv[0] && "ToolSession requires a valid argv[0]");
  PImpl =
      std::make_unique<Impl>(Argc, Argv, Tools, InstallPipeSignalExitHandler,
                             NeedsPOSIXUtilitySignalHandling);
}

ToolSession::~ToolSession() = افتراضي;

ErrorOr<CallableTool> ToolSession::findTool(StringRef Name) ثابت {
  StringRef Stem = sys::path::stem(Name);
  StringRef Filename = sys::path::filename(Name);
  لكل (ثابت auto &[RegisteredName, Main] : PImpl->Tools)
    إذا (Stem.equals_insensitive(RegisteredName) ||
        Filename.equals_insensitive(RegisteredName))
      أرجع CallableTool{RegisteredName, Main};

  لكل (ثابت auto &[RegisteredName, Main] : PImpl->Tools)
    إذا (matchesToolName(RegisteredName, Name))
      أرجع CallableTool{RegisteredName, Main};
  أرجع make_error_code(std::errc::no_such_file_or_directory);
}

ToolContext ToolSession::makeContext(StringRef RegisteredName,
                                     ثابت حرف *PrependArg) {
  ErrorOr<CallableTool> ExecutableTool = findTool(PImpl->ExecutablePath);
  منطقي NeedsPrependArg =
      !ExecutableTool ||
      !ExecutableTool->Name.equals_insensitive(RegisteredName);
  ToolContext Context(PImpl->ExecutablePath.c_str(), PrependArg,
                      NeedsPrependArg);
  Context.Session = هذا;
  أرجع Context;
}

ErrorOr<صحيح> ToolSession::callTool(ArrayRef<ثابت حرف *> Args) {
  إذا (Args.empty())
    أرجع make_error_code(std::errc::invalid_argument);

  StringRef InvokedName = Args.front();
  ErrorOr<CallableTool> Tool = findTool(InvokedName);
  إذا (!Tool) {
    إذا (InvokedName != PImpl->ExecutablePath && !isMulticallName(InvokedName))
      أرجع make_error_code(std::errc::no_such_file_or_directory);
    Args = Args.drop_front();
    إذا (Args.empty())
      أرجع make_error_code(std::errc::invalid_argument);
    InvokedName = Args.front();
    Tool = findTool(InvokedName);
  }

  إذا (!Tool)
    أرجع Tool.getError();

  std::string PrependArg = sys::path::stem(InvokedName).str();
  ToolContext Context = makeContext(Tool->Name, PrependArg.c_str());
  SmallVector<حرف *, 16> MutableArgs;
  MutableArgs.reserve(Args.size() + 1);
  لكل (ثابت حرف *Arg : Args)
    MutableArgs.ئىتتىرىش_back(const_cast<حرف *>(Arg));
  MutableArgs.ئىتتىرىش_back(nullptr);
  أرجع Tool->Main(Args.size(), MutableArgs.data(), Context);
}

ErrorOr<CallableTool> ToolContext::getCallableTool(StringRef Name) ثابت {
  إذا (!Session)
    أرجع make_error_code(std::errc::operation_not_permitted);
  أرجع Session->findTool(Name);
}

ErrorOr<صحيح> ToolContext::callTool(ArrayRef<ثابت حرف *> Args) ثابت {
  إذا (!Session)
    أرجع make_error_code(std::errc::operation_not_permitted);
  أرجع Session->callTool(Args);
}
