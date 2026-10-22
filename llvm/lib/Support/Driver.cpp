//===-- Driver.cpp -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/Driver.h"

#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/InitLLVM.h"
#अन्तर्भाव "llvm/Support/Path.h"

#अन्तर्भाव <cassert>
#अन्तर्भाव <string>
#अन्तर्भाव <system_error>
#अन्तर्भाव <utility>
#अन्तर्भाव <vector>

using नामाकाश llvm;

नामाकाश {

तार्किक matchesToolName(StringRef RegisteredName, StringRef InvokedName) {
  StringRef Stem = sys::path::stem(InvokedName);
  StringRef Filename = sys::path::filename(InvokedName);
  स्वतः Matches = [RegisteredName](StringRef Candidate) {
    माप_प्रकार Position = Candidate.rfind_insensitive(RegisteredName);
    परत Position != StringRef::npos &&
           (Position + RegisteredName.size() == Candidate.size() ||
            !llvm::isAlnum(Candidate[Position + RegisteredName.size()]));
  };
  परत Matches(Stem) || Matches(Filename);
}

तार्किक isMulticallName(StringRef Name) { परत matchesToolName("llvm", Name); }

} // namespace

संरचना ToolSession::Impl {
  InitLLVM Initialization;
  std::string ExecutablePath;
  std::vector<std::pair<std::string, ToolMainFn>> Tools;

  Impl(पूर्णांक &Argc, अक्षर **&Argv, ArrayRef<CallableTool> RegisteredTools,
       तार्किक InstallPipeSignalExitHandler, तार्किक NeedsPOSIXUtilitySignalHandling)
      : Initialization(Argc, Argv, InstallPipeSignalExitHandler,
                       NeedsPOSIXUtilitySignalHandling),
        ExecutablePath(Argv[0]) {
    Tools.reserve(RegisteredTools.size());
    साठी (स्थिरांक CallableTool &Tool : RegisteredTools)
      Tools.emplace_back(Tool.Name.str(), Tool.Main);
  }
};

ToolSession::ToolSession(पूर्णांक &Argc, अक्षर **&Argv, ArrayRef<CallableTool> Tools,
                         तार्किक InstallPipeSignalExitHandler,
                         तार्किक NeedsPOSIXUtilitySignalHandling) {
  प्रतिज्ञा(Argc > 0 && Argv && Argv[0] && "ToolSession requires a valid argv[0]");
  PImpl =
      std::make_unique<Impl>(Argc, Argv, Tools, InstallPipeSignalExitHandler,
                             NeedsPOSIXUtilitySignalHandling);
}

ToolSession::~ToolSession() = पूर्वनिर्धारित;

ErrorOr<CallableTool> ToolSession::findTool(StringRef Name) स्थिरांक {
  StringRef Stem = sys::path::stem(Name);
  StringRef Filename = sys::path::filename(Name);
  साठी (स्थिरांक स्वतः &[RegisteredName, Main] : PImpl->Tools)
    जर (Stem.equals_insensitive(RegisteredName) ||
        Filename.equals_insensitive(RegisteredName))
      परत CallableTool{RegisteredName, Main};

  साठी (स्थिरांक स्वतः &[RegisteredName, Main] : PImpl->Tools)
    जर (matchesToolName(RegisteredName, Name))
      परत CallableTool{RegisteredName, Main};
  परत make_error_code(std::errc::no_such_file_or_directory);
}

ToolContext ToolSession::makeContext(StringRef RegisteredName,
                                     स्थिरांक अक्षर *PrependArg) {
  ErrorOr<CallableTool> ExecutableTool = findTool(PImpl->ExecutablePath);
  तार्किक NeedsPrependArg =
      !ExecutableTool ||
      !ExecutableTool->Name.equals_insensitive(RegisteredName);
  ToolContext Context(PImpl->ExecutablePath.c_str(), PrependArg,
                      NeedsPrependArg);
  Context.Session = हे;
  परत Context;
}

ErrorOr<पूर्णांक> ToolSession::callTool(ArrayRef<स्थिरांक अक्षर *> Args) {
  जर (Args.empty())
    परत make_error_code(std::errc::invalid_argument);

  StringRef InvokedName = Args.front();
  ErrorOr<CallableTool> Tool = findTool(InvokedName);
  जर (!Tool) {
    जर (InvokedName != PImpl->ExecutablePath && !isMulticallName(InvokedName))
      परत make_error_code(std::errc::no_such_file_or_directory);
    Args = Args.drop_front();
    जर (Args.empty())
      परत make_error_code(std::errc::invalid_argument);
    InvokedName = Args.front();
    Tool = findTool(InvokedName);
  }

  जर (!Tool)
    परत Tool.getError();

  std::string PrependArg = sys::path::stem(InvokedName).str();
  ToolContext Context = makeContext(Tool->Name, PrependArg.c_str());
  SmallVector<अक्षर *, 16> MutableArgs;
  MutableArgs.reserve(Args.size() + 1);
  साठी (स्थिरांक अक्षर *Arg : Args)
    MutableArgs.ढकला_back(const_cast<अक्षर *>(Arg));
  MutableArgs.ढकला_back(nullptr);
  परत Tool->Main(Args.size(), MutableArgs.data(), Context);
}

ErrorOr<CallableTool> ToolContext::getCallableTool(StringRef Name) स्थिरांक {
  जर (!Session)
    परत make_error_code(std::errc::operation_not_permitted);
  परत Session->findTool(Name);
}

ErrorOr<पूर्णांक> ToolContext::callTool(ArrayRef<स्थिरांक अक्षर *> Args) स्थिरांक {
  जर (!Session)
    परत make_error_code(std::errc::operation_not_permitted);
  परत Session->callTool(Args);
}
