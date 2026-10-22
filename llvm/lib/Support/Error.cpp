//===----- lib/Support/Error.cpp - Error and associated utilities ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/Error.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن <system_error>

using نطاق llvm;

نطاق {

تعداد صنف ErrorErrorCode : صحيح {
  MultipleErrors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
صنف ErrorErrorCategory : عام std::error_category {
عام:
  ثابت حرف *name() ثابت noexcept override { أرجع "Error"; }

  std::string message(صحيح condition) ثابت override {
    بدل (static_cast<ErrorErrorCode>(condition)) {
    حالة ErrorErrorCode::MultipleErrors:
      أرجع "Multiple errors";
    حالة ErrorErrorCode::InconvertibleError:
      أرجع "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    حالة ErrorErrorCode::FileError:
      أرجع "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

ساكن ErrorErrorCategory &getErrorErrorCat() {
  ساكن ErrorErrorCategory ErrorErrorCat;
  أرجع ErrorErrorCat;
}

فراغ ErrorInfoBase::anchor() {}
حرف ErrorInfoBase::ID = 0;
حرف ErrorList::ID = 0;
فراغ ECError::anchor() {}
حرف ECError::ID = 0;
حرف StringError::ID = 0;
حرف FileError::ID = 0;

فراغ llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  𐤀𐤌 (!E)
    أرجع;
  OS << ErrorBanner;
  handleAllErrors(std::move(E), [&](ثابت ErrorInfoBase &EI) {
    EI.log(OS);
    OS << "\n";
  });
}

/// Write all error messages (if any) in E to a string. The newline character
/// is used to separate error messages.
std::string llvm::toString(Error E) {
  SmallVector<std::string, 2> Errors;
  handleAllErrors(std::move(E), [&Errors](ثابت ErrorInfoBase &EI) {
    Errors.push_back(EI.message());
  });
  أرجع join(Errors.begin(), Errors.end(), "\n");
}

std::string llvm::toStringWithoutConsuming(ثابت Error &E) {
  SmallVector<std::string, 2> Errors;
  visitErrors(E, [&Errors](ثابت ErrorInfoBase &EI) {
    Errors.push_back(EI.message());
  });
  أرجع join(Errors.begin(), Errors.end(), "\n");
}

std::error_code ErrorList::convertToErrorCode() ثابت {
  أرجع std::error_code(static_cast<صحيح>(ErrorErrorCode::MultipleErrors),
                         getErrorErrorCat());
}

std::error_code llvm::inconvertibleErrorCode() {
  أرجع std::error_code(static_cast<صحيح>(ErrorErrorCode::InconvertibleError),
                         getErrorErrorCat());
}

std::error_code FileError::convertToErrorCode() ثابت {
  std::error_code NestedEC = Err->convertToErrorCode();
  𐤀𐤌 (NestedEC == inconvertibleErrorCode())
    أرجع std::error_code(static_cast<صحيح>(ErrorErrorCode::FileError),
                           getErrorErrorCat());
  أرجع NestedEC;
}

Error llvm::errorCodeToError(std::error_code EC) {
  𐤀𐤌 (!EC)
    أرجع Error::success();
  أرجع Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_code llvm::errorToErrorCode(Error Err) {
  std::error_code EC;
  handleAllErrors(std::move(Err), [&](ثابت ErrorInfoBase &EI) {
    EC = EI.convertToErrorCode();
  });
  𐤀𐤌 (EC == inconvertibleErrorCode())
    report_fatal_error(Twine(EC.message()));
  أرجع EC;
}

#𐤀𐤌 LLVM_ENABLE_ABI_BREAKING_CHECKS
فراغ Error::fatalUncheckedError() ثابت {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  𐤀𐤌 (getPtr()) {
    getPtr()->log(dbgs());
    dbgs() << "\n";
  }وإلا
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  abort();
}
#endif

StringError::StringError(std::error_code EC, ثابت Twine &S)
    : Msg(S.str()), EC(EC) {}

StringError::StringError(ثابت Twine &S, std::error_code EC)
    : Msg(S.str()), EC(EC), PrintMsgOnly(true) {}

StringError::StringError(std::string &&S, std::error_code EC, منطقي PrintMsgOnly)
    : Msg(std::move(S)), EC(EC), PrintMsgOnly(PrintMsgOnly) {}

فراغ StringError::log(raw_ostream &OS) ثابت {
  𐤀𐤌 (PrintMsgOnly) {
    OS << Msg;
  } وإلا {
    OS << EC.message();
    𐤀𐤌 (!Msg.empty())
      OS << (" " + Msg);
  }
}

std::error_code StringError::convertToErrorCode() ثابت {
  أرجع EC;
}

Error llvm::createStringError(std::string &&Msg, std::error_code EC) {
  أرجع make_error<StringError>(std::move(Msg), EC, true);
}

فراغ llvm::report_fatal_error(Error Err, منطقي GenCrashDiag) {
  أكد(Err && "report_fatal_error called with success value");
  std::string ErrMsg;
  {
    raw_string_ostream ErrStream(ErrMsg);
    logAllUnhandledErrors(std::move(Err), ErrStream);
  }
  report_fatal_error(Twine(ErrMsg), GenCrashDiag);
}

فراغ llvm::reportFatalInternalError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/true);
}

فراغ llvm::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/خطأ);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  أرجع reinterpret_cast<ErrorInfoBase *>(Err)->dynamicClassID();
}

فراغ LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

فراغ LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

حرف *LLVMGetErrorMessage(LLVMErrorRef Err) {
  std::string Tmp = toString(unwrap(Err));
  حرف *ErrMsg = جديد حرف[Tmp.size() + 1];
  memcpy(ErrMsg, Tmp.data(), Tmp.size());
  ErrMsg[Tmp.size()] = '\0';
  أرجع ErrMsg;
}

فراغ LLVMDisposeErrorMessage(حرف *ErrMsg) { delete[] ErrMsg; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  أرجع reinterpret_cast<فراغ *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(ثابت حرف *ErrMsg) {
  أرجع wrap(make_error<StringError>(ErrMsg, inconvertibleErrorCode()));
}
