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

استفاده نطاق llvm;

نطاق {

تعداد صنف Error_Error_کد : صحيح {
  مضرب_Errors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
صنف ErrorErrorCategory : عام std::error_category {
عام:
  ثابت حرف *name() ثابت noexcept override { أرجع "Error"; }

  std::string پیام_2(صحيح condition) ثابت override {
    بدل (static_cast<Error_Error_کد>(condition)) {
    حالة Error_Error_کد::مضرب_Errors:
      أرجع "Multiple errors";
    حالة Error_Error_کد::InconvertibleError:
      أرجع "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    حالة Error_Error_کد::FileError:
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

فراغ Error_Info_پایه::anchor() {}
حرف Error_Info_پایه::ID = 0;
حرف ErrorList::ID = 0;
فراغ ECError::anchor() {}
حرف ECError::ID = 0;
حرف StringError::ID = 0;
حرف FileError::ID = 0;

فراغ llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  إذا (!E)
    أرجع;
  OS << ErrorBanner;
  handleAllErrors(std::move(E), [&](ثابت Error_Info_پایه &EI) {
    EI.لگاریتم(OS);
    OS << "\n";
  });
}

/// Write all error messages (if any) in E to a string. The newline character
/// is used to separate error messages.
std::string llvm::مقصد_String(Error E) {
  Small_وکتور<std::string, 2> Errors;
  handleAllErrors(std::move(E), [&Errors](ثابت Error_Info_پایه &EI) {
    Errors.افزودن_پشت(EI.پیام_2());
  });
  أرجع join(Errors.begin(), Errors.انتها(), "\n");
}

std::string llvm::مقصد_String_Without_Consuming(ثابت Error &E) {
  Small_وکتور<std::string, 2> Errors;
  بازدید_Errors(E, [&Errors](ثابت Error_Info_پایه &EI) {
    Errors.افزودن_پشت(EI.پیام_2());
  });
  أرجع join(Errors.begin(), Errors.انتها(), "\n");
}

std::error_کد ErrorList::convert_مقصد_Error_کد() ثابت {
  أرجع std::error_کد(static_cast<صحيح>(Error_Error_کد::مضرب_Errors),
                         getErrorErrorCat());
}

std::error_کد llvm::inconvertible_Error_کد() {
  أرجع std::error_کد(static_cast<صحيح>(Error_Error_کد::InconvertibleError),
                         getErrorErrorCat());
}

std::error_کد FileError::convert_مقصد_Error_کد() ثابت {
  std::error_کد NestedEC = Err->convert_مقصد_Error_کد();
  إذا (NestedEC == inconvertible_Error_کد())
    أرجع std::error_کد(static_cast<صحيح>(Error_Error_کد::FileError),
                           getErrorErrorCat());
  أرجع NestedEC;
}

Error llvm::error_کد_مقصد_Error(std::error_کد EC) {
  إذا (!EC)
    أرجع Error::success();
  أرجع Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_کد llvm::error_مقصد_Error_کد(Error Err) {
  std::error_کد EC;
  handleAllErrors(std::move(Err), [&](ثابت Error_Info_پایه &EI) {
    EC = EI.convert_مقصد_Error_کد();
  });
  إذا (EC == inconvertible_Error_کد())
    report_fatal_error(Twine(EC.پیام_2()));
  أرجع EC;
}

#إذا LLVM_ENABLE_ABI_BREAKING_CHECKS
فراغ Error::fatalUncheckedError() ثابت {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  إذا (getPtr()) {
    getPtr()->لگاریتم(dbgs());
    dbgs() << "\n";
  }وإلا
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  abort();
}
#پایان_اگر

StringError::StringError(std::error_کد EC, ثابت Twine &S)
    : پیام_4(S.str()), EC(EC) {}

StringError::StringError(ثابت Twine &S, std::error_کد EC)
    : پیام_4(S.str()), EC(EC), Print_پیام_Only(درست) {}

StringError::StringError(std::string &&S, std::error_کد EC, منطقي Print_پیام_Only)
    : پیام_4(std::move(S)), EC(EC), Print_پیام_Only(Print_پیام_Only) {}

فراغ StringError::لگاریتم(raw_ostream &OS) ثابت {
  إذا (Print_پیام_Only) {
    OS << پیام_4;
  } وإلا {
    OS << EC.پیام_2();
    إذا (!پیام_4.خالی())
      OS << (" " + پیام_4);
  }
}

std::error_کد StringError::convert_مقصد_Error_کد() ثابت {
  أرجع EC;
}

Error llvm::createStringError(std::string &&پیام_4, std::error_کد EC) {
  أرجع make_error<StringError>(std::move(پیام_4), EC, درست);
}

فراغ llvm::report_fatal_error(Error Err, منطقي GenCrashDiag) {
  أكد(Err && "report_fatal_error called with success value");
  std::string Err_پیام;
  {
    raw_string_ostream Err_جریان(Err_پیام);
    logAllUnhandledErrors(std::move(Err), Err_جریان);
  }
  report_fatal_error(Twine(Err_پیام), GenCrashDiag);
}

فراغ llvm::report_Fatal_داخلی_Error(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/درست);
}

فراغ llvm::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/خطأ);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  أرجع reinterpret_cast<Error_Info_پایه *>(Err)->dynamicClassID();
}

فراغ LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

فراغ LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

حرف *LLVMGet_Error_پیام(LLVMErrorRef Err) {
  std::string Tmp = مقصد_String(unwrap(Err));
  حرف *Err_پیام = جديد حرف[Tmp.اندازه_2() + 1];
  کپی_حافظه(Err_پیام, Tmp.داده(), Tmp.اندازه_2());
  Err_پیام[Tmp.اندازه_2()] = '\0';
  أرجع Err_پیام;
}

فراغ LLVMDispose_Error_پیام(حرف *Err_پیام) { حذف[] Err_پیام; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  أرجع reinterpret_cast<فراغ *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(ثابت حرف *Err_پیام) {
  أرجع wrap(make_error<StringError>(Err_پیام, inconvertible_Error_کد()));
}
