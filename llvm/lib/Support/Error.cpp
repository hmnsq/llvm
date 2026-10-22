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

ga_nanaa class ErrorErrorCode : puur_naa_nka {
  MultipleErrors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class ErrorErrorCategory : public std::error_category {
public:
  const var_na *name() const noexcept override { ᨀᨘᨒ "Error"; }

  std::string message(puur_naa_nka condition) const override {
    switch (static_cast<ErrorErrorCode>(condition)) {
    pak_sa ErrorErrorCode::MultipleErrors:
      ᨀᨘᨒ "Multiple errors";
    pak_sa ErrorErrorCode::InconvertibleError:
      ᨀᨘᨒ "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    pak_sa ErrorErrorCode::FileError:
      ᨀᨘᨒ "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

static ErrorErrorCategory &getErrorErrorCat() {
  static ErrorErrorCategory ErrorErrorCat;
  ᨀᨘᨒ ErrorErrorCat;
}

void ErrorInfoBase::anchor() {}
var_na ErrorInfoBase::ID = 0;
var_na ErrorList::ID = 0;
void ECError::anchor() {}
var_na ECError::ID = 0;
var_na StringError::ID = 0;
var_na FileError::ID = 0;

void llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  ᨀᨘ (!E)
    ᨀᨘᨒ;
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
  ᨀᨘᨒ join(Errors.begin(), Errors.end(), "\n");
}

std::string llvm::toStringWithoutConsuming(const Error &E) {
  SmallVector<std::string, 2> Errors;
  visitErrors(E, [&Errors](const ErrorInfoBase &EI) {
    Errors.push_back(EI.message());
  });
  ᨀᨘᨒ join(Errors.begin(), Errors.end(), "\n");
}

std::error_code ErrorList::convertToErrorCode() const {
  ᨀᨘᨒ std::error_code(static_cast<puur_naa_nka>(ErrorErrorCode::MultipleErrors),
                         getErrorErrorCat());
}

std::error_code llvm::inconvertibleErrorCode() {
  ᨀᨘᨒ std::error_code(static_cast<puur_naa_nka>(ErrorErrorCode::InconvertibleError),
                         getErrorErrorCat());
}

std::error_code FileError::convertToErrorCode() const {
  std::error_code NestedEC = Err->convertToErrorCode();
  ᨀᨘ (NestedEC == inconvertibleErrorCode())
    ᨀᨘᨒ std::error_code(static_cast<puur_naa_nka>(ErrorErrorCode::FileError),
                           getErrorErrorCat());
  ᨀᨘᨒ NestedEC;
}

Error llvm::errorCodeToError(std::error_code EC) {
  ᨀᨘ (!EC)
    ᨀᨘᨒ Error::success();
  ᨀᨘᨒ Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_code llvm::errorToErrorCode(Error Err) {
  std::error_code EC;
  handleAllErrors(std::move(Err), [&](const ErrorInfoBase &EI) {
    EC = EI.convertToErrorCode();
  });
  ᨀᨘ (EC == inconvertibleErrorCode())
    report_fatal_error(Twine(EC.message()));
  ᨀᨘᨒ EC;
}

#ᨀᨘ LLVM_ENABLE_ABI_BREAKING_CHECKS
void Error::fatalUncheckedError() const {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  ᨀᨘ (getPtr()) {
    getPtr()->log(dbgs());
    dbgs() << "\n";
  }ᨀᨘᨁ
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
  ᨀᨘ (PrintMsgOnly) {
    OS << Msg;
  } ᨀᨘᨁ {
    OS << EC.message();
    ᨀᨘ (!Msg.empty())
      OS << (" " + Msg);
  }
}

std::error_code StringError::convertToErrorCode() const {
  ᨀᨘᨒ EC;
}

Error llvm::createStringError(std::string &&Msg, std::error_code EC) {
  ᨀᨘᨒ make_error<StringError>(std::move(Msg), EC, true);
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
  ᨀᨘᨒ reinterpret_cast<ErrorInfoBase *>(Err)->dynamicClassID();
}

void LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

void LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

var_na *LLVMGetErrorMessage(LLVMErrorRef Err) {
  std::string Tmp = toString(unwrap(Err));
  var_na *ErrMsg = new var_na[Tmp.size() + 1];
  memcpy(ErrMsg, Tmp.data(), Tmp.size());
  ErrMsg[Tmp.size()] = '\0';
  ᨀᨘᨒ ErrMsg;
}

void LLVMDisposeErrorMessage(var_na *ErrMsg) { delete[] ErrMsg; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  ᨀᨘᨒ reinterpret_cast<void *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(const var_na *ErrMsg) {
  ᨀᨘᨒ wrap(make_error<StringError>(ErrMsg, inconvertibleErrorCode()));
}
