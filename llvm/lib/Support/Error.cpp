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

उपयुज्य नामाकाश llvm;

नामाकाश {

गणना वर्ग Error_Error_संहिता : पूर्णाङ्क {
  गुणजः_Errors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
वर्ग ErrorErrorCategory : सार्वजनिक std::error_category {
सार्वजनिक:
  ध्रुव वर्ण *name() ध्रुव noexcept override { प्रत्यागम "Error"; }

  std::string सन्देशः_2(पूर्णाङ्क condition) ध्रुव override {
    विकल्प (static_cast<Error_Error_संहिता>(condition)) {
    पक्ष Error_Error_संहिता::गुणजः_Errors:
      प्रत्यागम "Multiple errors";
    पक्ष Error_Error_संहिता::InconvertibleError:
      प्रत्यागम "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    पक्ष Error_Error_संहिता::FileError:
      प्रत्यागम "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

स्थिर ErrorErrorCategory &getErrorErrorCat() {
  स्थिर ErrorErrorCategory ErrorErrorCat;
  प्रत्यागम ErrorErrorCat;
}

शून्य Error_Info_आधारः::anchor() {}
वर्ण Error_Info_आधारः::ID = 0;
वर्ण ErrorList::ID = 0;
शून्य ECError::anchor() {}
वर्ण ECError::ID = 0;
वर्ण StringError::ID = 0;
वर्ण FileError::ID = 0;

शून्य llvm::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  यदि (!E)
    प्रत्यागम;
  OS << ErrorBanner;
  handleAllErrors(std::move(E), [&](ध्रुव Error_Info_आधारः &EI) {
    EI.लघुगणकः(OS);
    OS << "\n";
  });
}

/// Write all error messages (if any) in E to a string. The newline character
/// is used to separate error messages.
std::string llvm::गन्तव्यम्_String(Error E) {
  Small_सदिशम्<std::string, 2> Errors;
  handleAllErrors(std::move(E), [&Errors](ध्रुव Error_Info_आधारः &EI) {
    Errors.प्रक्षिप_पृष्ठभागः(EI.सन्देशः_2());
  });
  प्रत्यागम join(Errors.begin(), Errors.अन्तः(), "\n");
}

std::string llvm::गन्तव्यम्_String_Without_Consuming(ध्रुव Error &E) {
  Small_सदिशम्<std::string, 2> Errors;
  दर्शय_Errors(E, [&Errors](ध्रुव Error_Info_आधारः &EI) {
    Errors.प्रक्षिप_पृष्ठभागः(EI.सन्देशः_2());
  });
  प्रत्यागम join(Errors.begin(), Errors.अन्तः(), "\n");
}

std::error_संहिता ErrorList::convert_गन्तव्यम्_Error_संहिता() ध्रुव {
  प्रत्यागम std::error_संहिता(static_cast<पूर्णाङ्क>(Error_Error_संहिता::गुणजः_Errors),
                         getErrorErrorCat());
}

std::error_संहिता llvm::inconvertible_Error_संहिता() {
  प्रत्यागम std::error_संहिता(static_cast<पूर्णाङ्क>(Error_Error_संहिता::InconvertibleError),
                         getErrorErrorCat());
}

std::error_संहिता FileError::convert_गन्तव्यम्_Error_संहिता() ध्रुव {
  std::error_संहिता NestedEC = Err->convert_गन्तव्यम्_Error_संहिता();
  यदि (NestedEC == inconvertible_Error_संहिता())
    प्रत्यागम std::error_संहिता(static_cast<पूर्णाङ्क>(Error_Error_संहिता::FileError),
                           getErrorErrorCat());
  प्रत्यागम NestedEC;
}

Error llvm::error_संहिता_गन्तव्यम्_Error(std::error_संहिता EC) {
  यदि (!EC)
    प्रत्यागम Error::success();
  प्रत्यागम Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_संहिता llvm::error_गन्तव्यम्_Error_संहिता(Error Err) {
  std::error_संहिता EC;
  handleAllErrors(std::move(Err), [&](ध्रुव Error_Info_आधारः &EI) {
    EC = EI.convert_गन्तव्यम्_Error_संहिता();
  });
  यदि (EC == inconvertible_Error_संहिता())
    report_fatal_error(Twine(EC.सन्देशः_2()));
  प्रत्यागम EC;
}

#यदि LLVM_ENABLE_ABI_BREAKING_CHECKS
शून्य Error::fatalUncheckedError() ध्रुव {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  यदि (getPtr()) {
    getPtr()->लघुगणकः(dbgs());
    dbgs() << "\n";
  }अन्यथा
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  abort();
}
#यदि_अन्तः

StringError::StringError(std::error_संहिता EC, ध्रुव Twine &S)
    : सन्देशः_4(S.str()), EC(EC) {}

StringError::StringError(ध्रुव Twine &S, std::error_संहिता EC)
    : सन्देशः_4(S.str()), EC(EC), Print_सन्देशः_Only(सत्य) {}

StringError::StringError(std::string &&S, std::error_संहिता EC, तार्किक Print_सन्देशः_Only)
    : सन्देशः_4(std::move(S)), EC(EC), Print_सन्देशः_Only(Print_सन्देशः_Only) {}

शून्य StringError::लघुगणकः(raw_ostream &OS) ध्रुव {
  यदि (Print_सन्देशः_Only) {
    OS << सन्देशः_4;
  } अन्यथा {
    OS << EC.सन्देशः_2();
    यदि (!सन्देशः_4.रिक्तानि())
      OS << (" " + सन्देशः_4);
  }
}

std::error_संहिता StringError::convert_गन्तव्यम्_Error_संहिता() ध्रुव {
  प्रत्यागम EC;
}

Error llvm::createStringError(std::string &&सन्देशः_4, std::error_संहिता EC) {
  प्रत्यागम make_error<StringError>(std::move(सन्देशः_4), EC, सत्य);
}

शून्य llvm::report_fatal_error(Error Err, तार्किक GenCrashDiag) {
  प्रतिज्ञा(Err && "report_fatal_error called with success value");
  std::string Err_सन्देशः;
  {
    raw_string_ostream Err_प्रवाहः(Err_सन्देशः);
    logAllUnhandledErrors(std::move(Err), Err_प्रवाहः);
  }
  report_fatal_error(Twine(Err_सन्देशः), GenCrashDiag);
}

शून्य llvm::report_Fatal_आन्तरिकम्_Error(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/सत्य);
}

शून्य llvm::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/असत्य);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  प्रत्यागम reinterpret_cast<Error_Info_आधारः *>(Err)->dynamicClassID();
}

शून्य LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

शून्य LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

वर्ण *LLVMGet_Error_सन्देशः(LLVMErrorRef Err) {
  std::string Tmp = गन्तव्यम्_String(unwrap(Err));
  वर्ण *Err_सन्देशः = नूतन वर्ण[Tmp.आकार_2() + 1];
  स्मृतिं_प्रतिलिख(Err_सन्देशः, Tmp.दत्त(), Tmp.आकार_2());
  Err_सन्देशः[Tmp.आकार_2()] = '\0';
  प्रत्यागम Err_सन्देशः;
}

शून्य LLVMDispose_Error_सन्देशः(वर्ण *Err_सन्देशः) { विलोपय[] Err_सन्देशः; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  प्रत्यागम reinterpret_cast<शून्य *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(ध्रुव वर्ण *Err_सन्देशः) {
  प्रत्यागम wrap(make_error<StringError>(Err_सन्देशः, inconvertible_Error_संहिता()));
}
