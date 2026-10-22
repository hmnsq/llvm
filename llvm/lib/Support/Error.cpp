//===----- lib/Support/Error.cpp - Error and associated utilities ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव <system_error>

using नामाकाश llvm;

नामाकाश {

गणना वर्ग ErrorErrorCode : पूर्णाङ्क {
  MultipleErrors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
वर्ग ErrorErrorCategory : सार्वजनिक std::error_category {
सार्वजनिक:
  अचर वर्ण *name() अचर noexcept override { फर्काऊ "Error"; }

  std::string message(पूर्णाङ्क condition) अचर override {
    विकल्प (static_cast<ErrorErrorCode>(condition)) {
    पक्ष ErrorErrorCode::MultipleErrors:
      फर्काऊ "Multiple errors";
    पक्ष ErrorErrorCode::InconvertibleError:
      फर्काऊ "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    पक्ष ErrorErrorCode::FileError:
      फर्काऊ "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

स्थिर ErrorErrorCategory &getErrorErrorCat() {
  स्थिर ErrorErrorCategory ErrorErrorCat;
  फर्काऊ ErrorErrorCat;
}

शून्य ErrorInfoBase::anchor() {}
वर्ण ErrorInfoBase::ID = 0;
वर्ण ErrorList::ID = 0;
शून्य ECError::anchor() {}
वर्ण ECError::ID = 0;
वर्ण StringError::ID = 0;
वर्ण FileError::ID = 0;

शून्य llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  यदि (!E)
    फर्काऊ;
  OS << ErrorBanner;
  handleAllErrors(std::move(E), [&](अचर ErrorInfoBase &EI) {
    EI.log(OS);
    OS << "\n";
  });
}

/// Write all error messages (if any) in E to a string. The newline character
/// is used to separate error messages.
std::string llvm::toString(Error E) {
  SmallVector<std::string, 2> Errors;
  handleAllErrors(std::move(E), [&Errors](अचर ErrorInfoBase &EI) {
    Errors.थप_back(EI.message());
  });
  फर्काऊ join(Errors.begin(), Errors.end(), "\n");
}

std::string llvm::toStringWithoutConsuming(अचर Error &E) {
  SmallVector<std::string, 2> Errors;
  visitErrors(E, [&Errors](अचर ErrorInfoBase &EI) {
    Errors.थप_back(EI.message());
  });
  फर्काऊ join(Errors.begin(), Errors.end(), "\n");
}

std::error_code ErrorList::convertToErrorCode() अचर {
  फर्काऊ std::error_code(static_cast<पूर्णाङ्क>(ErrorErrorCode::MultipleErrors),
                         getErrorErrorCat());
}

std::error_code llvm::inconvertibleErrorCode() {
  फर्काऊ std::error_code(static_cast<पूर्णाङ्क>(ErrorErrorCode::InconvertibleError),
                         getErrorErrorCat());
}

std::error_code FileError::convertToErrorCode() अचर {
  std::error_code NestedEC = Err->convertToErrorCode();
  यदि (NestedEC == inconvertibleErrorCode())
    फर्काऊ std::error_code(static_cast<पूर्णाङ्क>(ErrorErrorCode::FileError),
                           getErrorErrorCat());
  फर्काऊ NestedEC;
}

Error llvm::errorCodeToError(std::error_code EC) {
  यदि (!EC)
    फर्काऊ Error::success();
  फर्काऊ Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_code llvm::errorToErrorCode(Error Err) {
  std::error_code EC;
  handleAllErrors(std::move(Err), [&](अचर ErrorInfoBase &EI) {
    EC = EI.convertToErrorCode();
  });
  यदि (EC == inconvertibleErrorCode())
    report_fatal_error(Twine(EC.message()));
  फर्काऊ EC;
}

#यदि LLVM_ENABLE_ABI_BREAKING_CHECKS
शून्य Error::fatalUncheckedError() अचर {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  यदि (getPtr()) {
    getPtr()->log(dbgs());
    dbgs() << "\n";
  }अन्यथा
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  पात();
}
#endif

StringError::StringError(std::error_code EC, अचर Twine &S)
    : सन्देश_2(S.str()), EC(EC) {}

StringError::StringError(अचर Twine &S, std::error_code EC)
    : सन्देश_2(S.str()), EC(EC), Print_सन्देश_Only(सत्य) {}

StringError::StringError(std::string &&S, std::error_code EC, तार्किक Print_सन्देश_Only)
    : सन्देश_2(std::move(S)), EC(EC), Print_सन्देश_Only(Print_सन्देश_Only) {}

शून्य StringError::log(raw_ostream &OS) अचर {
  यदि (Print_सन्देश_Only) {
    OS << सन्देश_2;
  } अन्यथा {
    OS << EC.message();
    यदि (!सन्देश_2.empty())
      OS << (" " + सन्देश_2);
  }
}

std::error_code StringError::convertToErrorCode() अचर {
  फर्काऊ EC;
}

Error llvm::createStringError(std::string &&सन्देश_2, std::error_code EC) {
  फर्काऊ make_error<StringError>(std::move(सन्देश_2), EC, सत्य);
}

शून्य llvm::report_fatal_error(Error Err, तार्किक GenCrashDiag) {
  प्रतिज्ञा(Err && "report_fatal_error called with success value");
  std::string Err_सन्देश;
  {
    raw_string_ostream ErrStream(Err_सन्देश);
    logAllUnhandledErrors(std::move(Err), ErrStream);
  }
  report_fatal_error(Twine(Err_सन्देश), GenCrashDiag);
}

शून्य llvm::reportFatalInternalError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/सत्य);
}

शून्य llvm::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/असत्य);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  फर्काऊ reinterpret_cast<ErrorInfoBase *>(Err)->dynamicClassID();
}

शून्य LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

शून्य LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

वर्ण *LLVMGetErrorMessage(LLVMErrorRef Err) {
  std::string Tmp = toString(unwrap(Err));
  वर्ण *Err_सन्देश = नूतन वर्ण[Tmp.size() + 1];
  स_नकल(Err_सन्देश, Tmp.data(), Tmp.size());
  Err_सन्देश[Tmp.size()] = '\0';
  फर्काऊ Err_सन्देश;
}

शून्य LLVMDisposeErrorMessage(वर्ण *Err_सन्देश) { delete[] Err_सन्देश; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  फर्काऊ reinterpret_cast<शून्य *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(अचर वर्ण *Err_सन्देश) {
  फर्काऊ wrap(make_error<StringError>(Err_सन्देश, inconvertibleErrorCode()));
}
