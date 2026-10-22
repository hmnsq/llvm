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
    फर्काऊ Position != StringRef::npos &&
           (Position + RegisteredName.size() == Candidate.size() ||
            !llvm::isAlnum(Candidate[Position + RegisteredName.size()]));
  };
  फर्काऊ Matches(Stem) || Matches(Filename);
}

तार्किक isMulticallName(StringRef Name) { फर्काऊ matchesToolName("llvm", Name); }

} // namespace

संरचना ToolSession::Impl {
  InitLLVM Initialization;
  std::string ExecutablePath;
  std::vector<std::pair<std::string, ToolMainFn>> Tools;

  Impl(पूर्णाङ्क &Argc, वर्ण **&Argv, ArrayRef<CallableTool> RegisteredTools,
       तार्किक InstallPipeSignalExitHandler, तार्किक NeedsPOSIXUtilitySignalHandling)
      : Initialization(Argc, Argv, InstallPipeSignalExitHandler,
                       NeedsPOSIXUtilitySignalHandling),
        ExecutablePath(Argv[0]) {
    Tools.reserve(RegisteredTools.size());
    लागि (अचर CallableTool &Tool : RegisteredTools)
      Tools.emplace_back(Tool.Name.str(), Tool.Main);
  }
};

ToolSession::ToolSession(पूर्णाङ्क &Argc, वर्ण **&Argv, ArrayRef<CallableTool> Tools,
                         तार्किक InstallPipeSignalExitHandler,
                         तार्किक NeedsPOSIXUtilitySignalHandling) {
  प्रतिज्ञा(Argc > 0 && Argv && Argv[0] && "ToolSession requires a valid argv[0]");
  PImpl =
      std::make_unique<Impl>(Argc, Argv, Tools, InstallPipeSignalExitHandler,
                             NeedsPOSIXUtilitySignalHandling);
}

ToolSession::~ToolSession() = स्वभाव;

ErrorOr<CallableTool> ToolSession::findTool(StringRef Name) अचर {
  StringRef Stem = sys::path::stem(Name);
  StringRef Filename = sys::path::filename(Name);
  लागि (अचर स्वतः &[RegisteredName, Main] : PImpl->Tools)
    यदि (Stem.equals_insensitive(RegisteredName) ||
        Filename.equals_insensitive(RegisteredName))
      फर्काऊ CallableTool{RegisteredName, Main};

  लागि (अचर स्वतः &[RegisteredName, Main] : PImpl->Tools)
    यदि (matchesToolName(RegisteredName, Name))
      फर्काऊ CallableTool{RegisteredName, Main};
  फर्काऊ make_error_code(std::errc::no_such_file_or_directory);
}

ToolContext ToolSession::makeContext(StringRef RegisteredName,
                                     अचर वर्ण *PrependArg) {
  ErrorOr<CallableTool> ExecutableTool = findTool(PImpl->ExecutablePath);
  तार्किक NeedsPrependArg =
      !ExecutableTool ||
      !ExecutableTool->Name.equals_insensitive(RegisteredName);
  ToolContext Context(PImpl->ExecutablePath.c_str(), PrependArg,
                      NeedsPrependArg);
  Context.Session = एतत्;
  फर्काऊ Context;
}

ErrorOr<पूर्णाङ्क> ToolSession::callTool(ArrayRef<अचर वर्ण *> Args) {
  यदि (Args.empty())
    फर्काऊ make_error_code(std::errc::invalid_argument);

  StringRef InvokedName = Args.front();
  ErrorOr<CallableTool> Tool = findTool(InvokedName);
  यदि (!Tool) {
    यदि (InvokedName != PImpl->ExecutablePath && !isMulticallName(InvokedName))
      फर्काऊ make_error_code(std::errc::no_such_file_or_directory);
    Args = Args.drop_front();
    यदि (Args.empty())
      फर्काऊ make_error_code(std::errc::invalid_argument);
    InvokedName = Args.front();
    Tool = findTool(InvokedName);
  }

  यदि (!Tool)
    फर्काऊ Tool.getError();

  std::string PrependArg = sys::path::stem(InvokedName).str();
  ToolContext Context = makeContext(Tool->Name, PrependArg.c_str());
  SmallVector<वर्ण *, 16> MutableArgs;
  MutableArgs.reserve(Args.size() + 1);
  लागि (अचर वर्ण *Arg : Args)
    MutableArgs.थप_back(const_cast<वर्ण *>(Arg));
  MutableArgs.थप_back(nullptr);
  फर्काऊ Tool->Main(Args.size(), MutableArgs.data(), Context);
}

ErrorOr<CallableTool> ToolContext::getCallableTool(StringRef Name) अचर {
  यदि (!Session)
    फर्काऊ make_error_code(std::errc::operation_not_permitted);
  फर्काऊ Session->findTool(Name);
}

ErrorOr<पूर्णाङ्क> ToolContext::callTool(ArrayRef<अचर वर्ण *> Args) अचर {
  यदि (!Session)
    फर्काऊ make_error_code(std::errc::operation_not_permitted);
  फर्काऊ Session->callTool(Args);
}
