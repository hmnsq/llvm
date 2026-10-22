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
  ثابت حرف *name() ثابت noexcept override { إرجاع "Error"; }

  std::string message(صحيح condition) ثابت override {
    بدل (static_cast<ErrorErrorCode>(condition)) {
    حالة ErrorErrorCode::MultipleErrors:
      إرجاع "Multiple errors";
    حالة ErrorErrorCode::InconvertibleError:
      إرجاع "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    حالة ErrorErrorCode::FileError:
      إرجاع "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

ساكن ErrorErrorCategory &getErrorErrorCat() {
  ساكن ErrorErrorCategory ErrorErrorCat;
  إرجاع ErrorErrorCat;
}

فراغ ErrorInfoBase::anchor() {}
حرف ErrorInfoBase::ID = 0;
حرف ErrorList::ID = 0;
فراغ ECError::anchor() {}
حرف ECError::ID = 0;
حرف StringError::ID = 0;
حرف FileError::ID = 0;

فراغ llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  إذا (!E)
    إرجاع;
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
    Errors.ادفع_back(EI.message());
  });
  إرجاع join(Errors.begin(), Errors.end(), "\n");
}

std::string llvm::toStringWithoutConsuming(ثابت Error &E) {
  SmallVector<std::string, 2> Errors;
  visitErrors(E, [&Errors](ثابت ErrorInfoBase &EI) {
    Errors.ادفع_back(EI.message());
  });
  إرجاع join(Errors.begin(), Errors.end(), "\n");
}

std::error_code ErrorList::convertToErrorCode() ثابت {
  إرجاع std::error_code(static_cast<صحيح>(ErrorErrorCode::MultipleErrors),
                         getErrorErrorCat());
}

std::error_code llvm::inconvertibleErrorCode() {
  إرجاع std::error_code(static_cast<صحيح>(ErrorErrorCode::InconvertibleError),
                         getErrorErrorCat());
}

std::error_code FileError::convertToErrorCode() ثابت {
  std::error_code NestedEC = Err->convertToErrorCode();
  إذا (NestedEC == inconvertibleErrorCode())
    إرجاع std::error_code(static_cast<صحيح>(ErrorErrorCode::FileError),
                           getErrorErrorCat());
  إرجاع NestedEC;
}

Error llvm::errorCodeToError(std::error_code EC) {
  إذا (!EC)
    إرجاع Error::success();
  إرجاع Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_code llvm::errorToErrorCode(Error Err) {
  std::error_code EC;
  handleAllErrors(std::move(Err), [&](ثابت ErrorInfoBase &EI) {
    EC = EI.convertToErrorCode();
  });
  إذا (EC == inconvertibleErrorCode())
    report_fatal_error(Twine(EC.message()));
  إرجاع EC;
}

#إذا LLVM_ENABLE_ABI_BREAKING_CHECKS
فراغ Error::fatalUncheckedError() ثابت {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  إذا (getPtr()) {
    getPtr()->log(dbgs());
    dbgs() << "\n";
  }وإلا
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  abort();
}
#endif

StringError::StringError(std::error_code EC, ثابت Twine &S)
    : رسالة_2(S.str()), EC(EC) {}

StringError::StringError(ثابت Twine &S, std::error_code EC)
    : رسالة_2(S.str()), EC(EC), Print_رسالة_Only(true) {}

StringError::StringError(std::string &&S, std::error_code EC, منطقي Print_رسالة_Only)
    : رسالة_2(std::move(S)), EC(EC), Print_رسالة_Only(Print_رسالة_Only) {}

فراغ StringError::log(raw_ostream &OS) ثابت {
  إذا (Print_رسالة_Only) {
    OS << رسالة_2;
  } وإلا {
    OS << EC.message();
    إذا (!رسالة_2.empty())
      OS << (" " + رسالة_2);
  }
}

std::error_code StringError::convertToErrorCode() ثابت {
  إرجاع EC;
}

Error llvm::createStringError(std::string &&رسالة_2, std::error_code EC) {
  إرجاع make_error<StringError>(std::move(رسالة_2), EC, true);
}

فراغ llvm::report_fatal_error(Error Err, منطقي GenCrashDiag) {
  أكد(Err && "report_fatal_error called with success value");
  std::string Err_رسالة;
  {
    raw_string_ostream ErrStream(Err_رسالة);
    logAllUnhandledErrors(std::move(Err), ErrStream);
  }
  report_fatal_error(Twine(Err_رسالة), GenCrashDiag);
}

فراغ llvm::reportFatalInternalError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/true);
}

فراغ llvm::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/خطأ);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  إرجاع reinterpret_cast<ErrorInfoBase *>(Err)->dynamicClassID();
}

فراغ LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

فراغ LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

حرف *LLVMGetErrorMessage(LLVMErrorRef Err) {
  std::string Tmp = toString(unwrap(Err));
  حرف *Err_رسالة = جديد حرف[Tmp.size() + 1];
  memcpy(Err_رسالة, Tmp.data(), Tmp.size());
  Err_رسالة[Tmp.size()] = '\0';
  إرجاع Err_رسالة;
}

فراغ LLVMDisposeErrorMessage(حرف *Err_رسالة) { delete[] Err_رسالة; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  إرجاع reinterpret_cast<فراغ *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(ثابت حرف *Err_رسالة) {
  إرجاع wrap(make_error<StringError>(Err_رسالة, inconvertibleErrorCode()));
}
