//===----- lib/Support/Error.cpp - Error and associated utilities ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Error.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل <system_error>

using فضاینام llvm;

فضاینام {

شمارش رده ErrorErrorCode : صحیح {
  MultipleErrors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
رده ErrorErrorCategory : عمومی std::error_category {
عمومی:
  ثابت نویسه *name() ثابت noexcept override { بازگشت "Error"; }

  std::string message(صحیح condition) ثابت override {
    انتخاب (static_cast<ErrorErrorCode>(condition)) {
    حالت ErrorErrorCode::MultipleErrors:
      بازگشت "Multiple errors";
    حالت ErrorErrorCode::InconvertibleError:
      بازگشت "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    حالت ErrorErrorCode::FileError:
      بازگشت "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

ایستا ErrorErrorCategory &getErrorErrorCat() {
  ایستا ErrorErrorCategory ErrorErrorCat;
  بازگشت ErrorErrorCat;
}

تهی ErrorInfoBase::anchor() {}
نویسه ErrorInfoBase::ID = 0;
نویسه ErrorList::ID = 0;
تهی ECError::anchor() {}
نویسه ECError::ID = 0;
نویسه StringError::ID = 0;
نویسه FileError::ID = 0;

تهی llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  اگر (!E)
    بازگشت;
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
    Errors.افزودن_back(EI.message());
  });
  بازگشت join(Errors.begin(), Errors.end(), "\n");
}

std::string llvm::toStringWithoutConsuming(ثابت Error &E) {
  SmallVector<std::string, 2> Errors;
  visitErrors(E, [&Errors](ثابت ErrorInfoBase &EI) {
    Errors.افزودن_back(EI.message());
  });
  بازگشت join(Errors.begin(), Errors.end(), "\n");
}

std::error_code ErrorList::convertToErrorCode() ثابت {
  بازگشت std::error_code(static_cast<صحیح>(ErrorErrorCode::MultipleErrors),
                         getErrorErrorCat());
}

std::error_code llvm::inconvertibleErrorCode() {
  بازگشت std::error_code(static_cast<صحیح>(ErrorErrorCode::InconvertibleError),
                         getErrorErrorCat());
}

std::error_code FileError::convertToErrorCode() ثابت {
  std::error_code NestedEC = Err->convertToErrorCode();
  اگر (NestedEC == inconvertibleErrorCode())
    بازگشت std::error_code(static_cast<صحیح>(ErrorErrorCode::FileError),
                           getErrorErrorCat());
  بازگشت NestedEC;
}

Error llvm::errorCodeToError(std::error_code EC) {
  اگر (!EC)
    بازگشت Error::success();
  بازگشت Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_code llvm::errorToErrorCode(Error Err) {
  std::error_code EC;
  handleAllErrors(std::move(Err), [&](ثابت ErrorInfoBase &EI) {
    EC = EI.convertToErrorCode();
  });
  اگر (EC == inconvertibleErrorCode())
    report_fatal_error(Twine(EC.message()));
  بازگشت EC;
}

#اگر LLVM_ENABLE_ABI_BREAKING_CHECKS
تهی Error::fatalUncheckedError() ثابت {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  اگر (getPtr()) {
    getPtr()->log(dbgs());
    dbgs() << "\n";
  }وگرنه
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  abort();
}
#endif

StringError::StringError(std::error_code EC, ثابت Twine &S)
    : پیام_2(S.str()), EC(EC) {}

StringError::StringError(ثابت Twine &S, std::error_code EC)
    : پیام_2(S.str()), EC(EC), Print_پیام_Only(درست) {}

StringError::StringError(std::string &&S, std::error_code EC, منطقی Print_پیام_Only)
    : پیام_2(std::move(S)), EC(EC), Print_پیام_Only(Print_پیام_Only) {}

تهی StringError::log(raw_ostream &OS) ثابت {
  اگر (Print_پیام_Only) {
    OS << پیام_2;
  } وگرنه {
    OS << EC.message();
    اگر (!پیام_2.empty())
      OS << (" " + پیام_2);
  }
}

std::error_code StringError::convertToErrorCode() ثابت {
  بازگشت EC;
}

Error llvm::createStringError(std::string &&پیام_2, std::error_code EC) {
  بازگشت make_error<StringError>(std::move(پیام_2), EC, درست);
}

تهی llvm::report_fatal_error(Error Err, منطقی GenCrashDiag) {
  أكد(Err && "report_fatal_error called with success value");
  std::string Err_پیام;
  {
    raw_string_ostream ErrStream(Err_پیام);
    logAllUnhandledErrors(std::move(Err), ErrStream);
  }
  report_fatal_error(Twine(Err_پیام), GenCrashDiag);
}

تهی llvm::reportFatalInternalError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/درست);
}

تهی llvm::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/نادرست);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  بازگشت reinterpret_cast<ErrorInfoBase *>(Err)->dynamicClassID();
}

تهی LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

تهی LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

نویسه *LLVMGetErrorMessage(LLVMErrorRef Err) {
  std::string Tmp = toString(unwrap(Err));
  نویسه *Err_پیام = جدید نویسه[Tmp.size() + 1];
  memcpy(Err_پیام, Tmp.data(), Tmp.size());
  Err_پیام[Tmp.size()] = '\0';
  بازگشت Err_پیام;
}

تهی LLVMDisposeErrorMessage(نویسه *Err_پیام) { delete[] Err_پیام; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  بازگشت reinterpret_cast<تهی *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(ثابت نویسه *Err_پیام) {
  بازگشت wrap(make_error<StringError>(Err_پیام, inconvertibleErrorCode()));
}
