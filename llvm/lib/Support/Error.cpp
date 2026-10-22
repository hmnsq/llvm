//===----- lib/Support/Error.cpp - Error and associated utilities ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ {

ਗਿਣਤੀ ਵਰਗ Error_Error_ਕੋਡ : ਪੂਰਨ {
  ਗੁਣਜ_Errors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
ਵਰਗ ErrorErrorCategory : ਜਨਤਕ std::error_category {
ਜਨਤਕ:
  ਅਚਲ ਅੱਖਰ *name() ਅਚਲ noexcept override { ਵਾਪਸ "Error"; }

  std::string ਸੁਨੇਹਾ_2(ਪੂਰਨ condition) ਅਚਲ override {
    ਚੋਣ (static_cast<Error_Error_ਕੋਡ>(condition)) {
    ਹਾਲਤ Error_Error_ਕੋਡ::ਗੁਣਜ_Errors:
      ਵਾਪਸ "Multiple errors";
    ਹਾਲਤ Error_Error_ਕੋਡ::InconvertibleError:
      ਵਾਪਸ "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    ਹਾਲਤ Error_Error_ਕੋਡ::FileError:
      ਵਾਪਸ "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

ਸਥਿਰ ErrorErrorCategory &getErrorErrorCat() {
  ਸਥਿਰ ErrorErrorCategory ErrorErrorCat;
  ਵਾਪਸ ErrorErrorCat;
}

ਖਾਲੀ Error_Info_ਅਧਾਰ::anchor() {}
ਅੱਖਰ Error_Info_ਅਧਾਰ::ID = 0;
ਅੱਖਰ ErrorList::ID = 0;
ਖਾਲੀ ECError::anchor() {}
ਅੱਖਰ ECError::ID = 0;
ਅੱਖਰ StringError::ID = 0;
ਅੱਖਰ FileError::ID = 0;

ਖਾਲੀ llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  ਜੇ (!E)
    ਵਾਪਸ;
  OS << ErrorBanner;
  handleAllErrors(std::move(E), [&](ਅਚਲ Error_Info_ਅਧਾਰ &EI) {
    EI.ਲਘੂਗਣਕ(OS);
    OS << "\n";
  });
}

/// Write all error messages (if any) in E to a string. The newline character
/// is used to separate error messages.
std::string llvm::ਮੰਜ਼ਿਲ_String(Error E) {
  Small_ਸਦਿਸ਼<std::string, 2> Errors;
  handleAllErrors(std::move(E), [&Errors](ਅਚਲ Error_Info_ਅਧਾਰ &EI) {
    Errors.ਧੱਕੋ_ਪਿੱਠ(EI.ਸੁਨੇਹਾ_2());
  });
  ਵਾਪਸ join(Errors.begin(), Errors.ਸਿਰਾ(), "\n");
}

std::string llvm::ਮੰਜ਼ਿਲ_String_Without_Consuming(ਅਚਲ Error &E) {
  Small_ਸਦਿਸ਼<std::string, 2> Errors;
  ਦੇਖੋ_Errors(E, [&Errors](ਅਚਲ Error_Info_ਅਧਾਰ &EI) {
    Errors.ਧੱਕੋ_ਪਿੱਠ(EI.ਸੁਨੇਹਾ_2());
  });
  ਵਾਪਸ join(Errors.begin(), Errors.ਸਿਰਾ(), "\n");
}

std::error_ਕੋਡ ErrorList::convert_ਮੰਜ਼ਿਲ_Error_ਕੋਡ() ਅਚਲ {
  ਵਾਪਸ std::error_ਕੋਡ(static_cast<ਪੂਰਨ>(Error_Error_ਕੋਡ::ਗੁਣਜ_Errors),
                         getErrorErrorCat());
}

std::error_ਕੋਡ llvm::inconvertible_Error_ਕੋਡ() {
  ਵਾਪਸ std::error_ਕੋਡ(static_cast<ਪੂਰਨ>(Error_Error_ਕੋਡ::InconvertibleError),
                         getErrorErrorCat());
}

std::error_ਕੋਡ FileError::convert_ਮੰਜ਼ਿਲ_Error_ਕੋਡ() ਅਚਲ {
  std::error_ਕੋਡ NestedEC = Err->convert_ਮੰਜ਼ਿਲ_Error_ਕੋਡ();
  ਜੇ (NestedEC == inconvertible_Error_ਕੋਡ())
    ਵਾਪਸ std::error_ਕੋਡ(static_cast<ਪੂਰਨ>(Error_Error_ਕੋਡ::FileError),
                           getErrorErrorCat());
  ਵਾਪਸ NestedEC;
}

Error llvm::error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(std::error_ਕੋਡ EC) {
  ਜੇ (!EC)
    ਵਾਪਸ Error::success();
  ਵਾਪਸ Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_ਕੋਡ llvm::error_ਮੰਜ਼ਿਲ_Error_ਕੋਡ(Error Err) {
  std::error_ਕੋਡ EC;
  handleAllErrors(std::move(Err), [&](ਅਚਲ Error_Info_ਅਧਾਰ &EI) {
    EC = EI.convert_ਮੰਜ਼ਿਲ_Error_ਕੋਡ();
  });
  ਜੇ (EC == inconvertible_Error_ਕੋਡ())
    report_fatal_error(Twine(EC.ਸੁਨੇਹਾ_2()));
  ਵਾਪਸ EC;
}

#ਜੇ LLVM_ENABLE_ABI_BREAKING_CHECKS
ਖਾਲੀ Error::fatalUncheckedError() ਅਚਲ {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  ਜੇ (getPtr()) {
    getPtr()->ਲਘੂਗਣਕ(dbgs());
    dbgs() << "\n";
  }ਨਹੀਂਤਾਂ
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  abort();
}
#ਜੇ_ਅੰਤ

StringError::StringError(std::error_ਕੋਡ EC, ਅਚਲ Twine &S)
    : ਸੁਨੇਹਾ_4(S.str()), EC(EC) {}

StringError::StringError(ਅਚਲ Twine &S, std::error_ਕੋਡ EC)
    : ਸੁਨੇਹਾ_4(S.str()), EC(EC), Print_ਸੁਨੇਹਾ_Only(ਸੱਚ) {}

StringError::StringError(std::string &&S, std::error_ਕੋਡ EC, ਤਾਰਕਿਕ Print_ਸੁਨੇਹਾ_Only)
    : ਸੁਨੇਹਾ_4(std::move(S)), EC(EC), Print_ਸੁਨੇਹਾ_Only(Print_ਸੁਨੇਹਾ_Only) {}

ਖਾਲੀ StringError::ਲਘੂਗਣਕ(raw_ostream &OS) ਅਚਲ {
  ਜੇ (Print_ਸੁਨੇਹਾ_Only) {
    OS << ਸੁਨੇਹਾ_4;
  } ਨਹੀਂਤਾਂ {
    OS << EC.ਸੁਨੇਹਾ_2();
    ਜੇ (!ਸੁਨੇਹਾ_4.ਖਾਲੀ_ਥਾਵਾਂ())
      OS << (" " + ਸੁਨੇਹਾ_4);
  }
}

std::error_ਕੋਡ StringError::convert_ਮੰਜ਼ਿਲ_Error_ਕੋਡ() ਅਚਲ {
  ਵਾਪਸ EC;
}

Error llvm::createStringError(std::string &&ਸੁਨੇਹਾ_4, std::error_ਕੋਡ EC) {
  ਵਾਪਸ make_error<StringError>(std::move(ਸੁਨੇਹਾ_4), EC, ਸੱਚ);
}

ਖਾਲੀ llvm::report_fatal_error(Error Err, ਤਾਰਕਿਕ GenCrashDiag) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Err && "report_fatal_error called with success value");
  std::string Err_ਸੁਨੇਹਾ;
  {
    raw_string_ostream Err_ਧਾਰਾ(Err_ਸੁਨੇਹਾ);
    logAllUnhandledErrors(std::move(Err), Err_ਧਾਰਾ);
  }
  report_fatal_error(Twine(Err_ਸੁਨੇਹਾ), GenCrashDiag);
}

ਖਾਲੀ llvm::report_Fatal_ਅੰਦਰੂਨੀ_Error(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/ਸੱਚ);
}

ਖਾਲੀ llvm::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/ਝੂਠ);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  ਵਾਪਸ reinterpret_cast<Error_Info_ਅਧਾਰ *>(Err)->dynamicClassID();
}

ਖਾਲੀ LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

ਖਾਲੀ LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

ਅੱਖਰ *LLVMGet_Error_ਸੁਨੇਹਾ(LLVMErrorRef Err) {
  std::string Tmp = ਮੰਜ਼ਿਲ_String(unwrap(Err));
  ਅੱਖਰ *Err_ਸੁਨੇਹਾ = ਨਵਾਂ ਅੱਖਰ[Tmp.ਮਾਪ_2() + 1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(Err_ਸੁਨੇਹਾ, Tmp.ਅੰਕੜੇ(), Tmp.ਮਾਪ_2());
  Err_ਸੁਨੇਹਾ[Tmp.ਮਾਪ_2()] = '\0';
  ਵਾਪਸ Err_ਸੁਨੇਹਾ;
}

ਖਾਲੀ LLVMDispose_Error_ਸੁਨੇਹਾ(ਅੱਖਰ *Err_ਸੁਨੇਹਾ) { ਮਿਟਾਓ[] Err_ਸੁਨੇਹਾ; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  ਵਾਪਸ reinterpret_cast<ਖਾਲੀ *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(ਅਚਲ ਅੱਖਰ *Err_ਸੁਨੇਹਾ) {
  ਵਾਪਸ wrap(make_error<StringError>(Err_ਸੁਨੇਹਾ, inconvertible_Error_ਕੋਡ()));
}
