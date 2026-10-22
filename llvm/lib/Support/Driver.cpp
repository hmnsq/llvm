//===-- Driver.cpp -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Driver.h"

#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/InitLLVM.h"
#شامل "llvm/Support/Path.h"

#شامل <cassert>
#شامل <string>
#شامل <system_error>
#شامل <utility>
#شامل <vector>

using فضاینام llvm;

فضاینام {

منطقی matchesToolName(StringRef RegisteredName, StringRef InvokedName) {
  StringRef Stem = sys::path::stem(InvokedName);
  StringRef Filename = sys::path::filename(InvokedName);
  auto Matches = [RegisteredName](StringRef Candidate) {
    size_t Position = Candidate.rfind_insensitive(RegisteredName);
    بازگشت Position != StringRef::npos &&
           (Position + RegisteredName.size() == Candidate.size() ||
            !llvm::isAlnum(Candidate[Position + RegisteredName.size()]));
  };
  بازگشت Matches(Stem) || Matches(Filename);
}

منطقی isMulticallName(StringRef Name) { بازگشت matchesToolName("llvm", Name); }

} // namespace

ساختار ToolSession::Impl {
  InitLLVM Initialization;
  std::string ExecutablePath;
  std::vector<std::pair<std::string, ToolMainFn>> Tools;

  Impl(صحیح &Argc, نویسه **&Argv, ArrayRef<CallableTool> RegisteredTools,
       منطقی InstallPipeSignalExitHandler, منطقی NeedsPOSIXUtilitySignalHandling)
      : Initialization(Argc, Argv, InstallPipeSignalExitHandler,
                       NeedsPOSIXUtilitySignalHandling),
        ExecutablePath(Argv[0]) {
    Tools.reserve(RegisteredTools.size());
    برای (ثابت CallableTool &Tool : RegisteredTools)
      Tools.emplace_back(Tool.Name.str(), Tool.Main);
  }
};

ToolSession::ToolSession(صحیح &Argc, نویسه **&Argv, ArrayRef<CallableTool> Tools,
                         منطقی InstallPipeSignalExitHandler,
                         منطقی NeedsPOSIXUtilitySignalHandling) {
  أكد(Argc > 0 && Argv && Argv[0] && "ToolSession requires a valid argv[0]");
  PImpl =
      std::make_unique<Impl>(Argc, Argv, Tools, InstallPipeSignalExitHandler,
                             NeedsPOSIXUtilitySignalHandling);
}

ToolSession::~ToolSession() = پیشفرض;

ErrorOr<CallableTool> ToolSession::findTool(StringRef Name) ثابت {
  StringRef Stem = sys::path::stem(Name);
  StringRef Filename = sys::path::filename(Name);
  برای (ثابت auto &[RegisteredName, Main] : PImpl->Tools)
    اگر (Stem.equals_insensitive(RegisteredName) ||
        Filename.equals_insensitive(RegisteredName))
      بازگشت CallableTool{RegisteredName, Main};

  برای (ثابت auto &[RegisteredName, Main] : PImpl->Tools)
    اگر (matchesToolName(RegisteredName, Name))
      بازگشت CallableTool{RegisteredName, Main};
  بازگشت make_error_code(std::errc::no_such_file_or_directory);
}

ToolContext ToolSession::makeContext(StringRef RegisteredName,
                                     ثابت نویسه *PrependArg) {
  ErrorOr<CallableTool> ExecutableTool = findTool(PImpl->ExecutablePath);
  منطقی NeedsPrependArg =
      !ExecutableTool ||
      !ExecutableTool->Name.equals_insensitive(RegisteredName);
  ToolContext Context(PImpl->ExecutablePath.c_str(), PrependArg,
                      NeedsPrependArg);
  Context.Session = این;
  بازگشت Context;
}

ErrorOr<صحیح> ToolSession::callTool(ArrayRef<ثابت نویسه *> Args) {
  اگر (Args.empty())
    بازگشت make_error_code(std::errc::invalid_argument);

  StringRef InvokedName = Args.front();
  ErrorOr<CallableTool> Tool = findTool(InvokedName);
  اگر (!Tool) {
    اگر (InvokedName != PImpl->ExecutablePath && !isMulticallName(InvokedName))
      بازگشت make_error_code(std::errc::no_such_file_or_directory);
    Args = Args.drop_front();
    اگر (Args.empty())
      بازگشت make_error_code(std::errc::invalid_argument);
    InvokedName = Args.front();
    Tool = findTool(InvokedName);
  }

  اگر (!Tool)
    بازگشت Tool.getError();

  std::string PrependArg = sys::path::stem(InvokedName).str();
  ToolContext Context = makeContext(Tool->Name, PrependArg.c_str());
  SmallVector<نویسه *, 16> MutableArgs;
  MutableArgs.reserve(Args.size() + 1);
  برای (ثابت نویسه *Arg : Args)
    MutableArgs.افزودن_back(const_cast<نویسه *>(Arg));
  MutableArgs.افزودن_back(nullptr);
  بازگشت Tool->Main(Args.size(), MutableArgs.data(), Context);
}

ErrorOr<CallableTool> ToolContext::getCallableTool(StringRef Name) ثابت {
  اگر (!Session)
    بازگشت make_error_code(std::errc::operation_not_permitted);
  بازگشت Session->findTool(Name);
}

ErrorOr<صحیح> ToolContext::callTool(ArrayRef<ثابت نویسه *> Args) ثابت {
  اگر (!Session)
    بازگشت make_error_code(std::errc::operation_not_permitted);
  بازگشت Session->callTool(Args);
}
