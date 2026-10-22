//===----- lib/Support/Error.cpp - Error and associated utilities ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Error.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/ErrorHandling.h"
#include <system_error>

using namespace llvm;

namespace {

enum class ErrorErrorCode : số_nguyên {
  MultipleErrors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class ErrorErrorCategory : public std::error_category {
public:
  const char *name() const noexcept override { trả_về "Error"; }

  std::string message(số_nguyên condition) const override {
    switch (static_cast<ErrorErrorCode>(condition)) {
    case ErrorErrorCode::MultipleErrors:
      trả_về "Multiple errors";
    case ErrorErrorCode::InconvertibleError:
      trả_về "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    case ErrorErrorCode::FileError:
      trả_về "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

static ErrorErrorCategory &getErrorErrorCat() {
  static ErrorErrorCategory ErrorErrorCat;
  trả_về ErrorErrorCat;
}

void ErrorInfoBase::anchor() {}
char ErrorInfoBase::ID = 0;
char ErrorList::ID = 0;
void ECError::anchor() {}
char ECError::ID = 0;
char StringError::ID = 0;
char FileError::ID = 0;

void llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  nếu (!E)
    trả_về;
  OS << ErrorBanner;
  handleAllErrors(std::move(E), [&](const ErrorInfoBase &EI) {
    EI.log(OS);
    OS << "\n";
  });
}

/// Write all error messages (if any) in E to a string. The newline character
/// is used to separate error messages.
std::string llvm::toString(Error E) {
  SmallVector<std::string, 2> Errors;
  handleAllErrors(std::move(E), [&Errors](const ErrorInfoBase &EI) {
    Errors.push_back(EI.message());
  });
  trả_về join(Errors.begin(), Errors.end(), "\n");
}

std::string llvm::toStringWithoutConsuming(const Error &E) {
  SmallVector<std::string, 2> Errors;
  visitErrors(E, [&Errors](const ErrorInfoBase &EI) {
    Errors.push_back(EI.message());
  });
  trả_về join(Errors.begin(), Errors.end(), "\n");
}

std::error_code ErrorList::convertToErrorCode() const {
  trả_về std::error_code(static_cast<số_nguyên>(ErrorErrorCode::MultipleErrors),
                         getErrorErrorCat());
}

std::error_code llvm::inconvertibleErrorCode() {
  trả_về std::error_code(static_cast<số_nguyên>(ErrorErrorCode::InconvertibleError),
                         getErrorErrorCat());
}

std::error_code FileError::convertToErrorCode() const {
  std::error_code NestedEC = Err->convertToErrorCode();
  nếu (NestedEC == inconvertibleErrorCode())
    trả_về std::error_code(static_cast<số_nguyên>(ErrorErrorCode::FileError),
                           getErrorErrorCat());
  trả_về NestedEC;
}

Error llvm::errorCodeToError(std::error_code EC) {
  nếu (!EC)
    trả_về Error::success();
  trả_về Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_code llvm::errorToErrorCode(Error Err) {
  std::error_code EC;
  handleAllErrors(std::move(Err), [&](const ErrorInfoBase &EI) {
    EC = EI.convertToErrorCode();
  });
  nếu (EC == inconvertibleErrorCode())
    report_fatal_error(Twine(EC.message()));
  trả_về EC;
}

#nếu LLVM_ENABLE_ABI_BREAKING_CHECKS
void Error::fatalUncheckedError() const {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  nếu (getPtr()) {
    getPtr()->log(dbgs());
    dbgs() << "\n";
  }không_thì
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  abort();
}
#endif

StringError::StringError(std::error_code EC, const Twine &S)
    : Msg(S.str()), EC(EC) {}

StringError::StringError(const Twine &S, std::error_code EC)
    : Msg(S.str()), EC(EC), PrintMsgOnly(true) {}

StringError::StringError(std::string &&S, std::error_code EC, bool PrintMsgOnly)
    : Msg(std::move(S)), EC(EC), PrintMsgOnly(PrintMsgOnly) {}

void StringError::log(raw_ostream &OS) const {
  nếu (PrintMsgOnly) {
    OS << Msg;
  } không_thì {
    OS << EC.message();
    nếu (!Msg.empty())
      OS << (" " + Msg);
  }
}

std::error_code StringError::convertToErrorCode() const {
  trả_về EC;
}

Error llvm::createStringError(std::string &&Msg, std::error_code EC) {
  trả_về make_error<StringError>(std::move(Msg), EC, true);
}

void llvm::report_fatal_error(Error Err, bool GenCrashDiag) {
  assert(Err && "report_fatal_error called with success value");
  std::string ErrMsg;
  {
    raw_string_ostream ErrStream(ErrMsg);
    logAllUnhandledErrors(std::move(Err), ErrStream);
  }
  report_fatal_error(Twine(ErrMsg), GenCrashDiag);
}

void llvm::reportFatalInternalError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/true);
}

void llvm::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/false);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  trả_về reinterpret_cast<ErrorInfoBase *>(Err)->dynamicClassID();
}

void LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

void LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

char *LLVMGetErrorMessage(LLVMErrorRef Err) {
  std::string Tmp = toString(unwrap(Err));
  char *ErrMsg = new char[Tmp.size() + 1];
  memcpy(ErrMsg, Tmp.data(), Tmp.size());
  ErrMsg[Tmp.size()] = '\0';
  trả_về ErrMsg;
}

void LLVMDisposeErrorMessage(char *ErrMsg) { delete[] ErrMsg; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  trả_về reinterpret_cast<void *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(const char *ErrMsg) {
  trả_về wrap(make_error<StringError>(ErrMsg, inconvertibleErrorCode()));
}
