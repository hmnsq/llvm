//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#شامل "llvm/Support/JSON.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/ConvertUTF.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/NativeFormatting.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cctype>
#شامل <cerrno>
#شامل <optional>

نامخلا llvm {
نامخلا json {

قدر &Object::عامل[](ثابت Object_کلید &K) {
  واپس try_emplace(K, خالی_اشاریہ).پہلا_2->get_دوسرا();
}
قدر &Object::عامل[](Object_کلید &&K) {
  واپس try_emplace(std::move(K), خالی_اشاریہ).پہلا_2->get_دوسرا();
}
قدر *Object::get(StringRef K) {
  خودکار I = ڈھونڈو(K);
  اگر (I == انتہا())
    واپس خالی_اشاریہ;
  واپس &I->دوسرا_2;
}
ثابت قدر *Object::get(StringRef K) ثابت {
  خودکار I = ڈھونڈو(K);
  اگر (I == انتہا())
    واپس خالی_اشاریہ;
  واپس &I->دوسرا_2;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ثابت {
  اگر (خودکار *V = get(K))
    واپس V->getAsNull();
  واپس std::nullopt;
}
std::optional<منطقی> Object::getBoolean(StringRef K) ثابت {
  اگر (خودکار *V = get(K))
    واپس V->getAsBoolean();
  واپس std::nullopt;
}
std::optional<دوہرا> Object::get_نمبر(StringRef K) ثابت {
  اگر (خودکار *V = get(K))
    واپس V->get_As_نمبر();
  واپس std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ثابت {
  اگر (خودکار *V = get(K))
    واپس V->getAsInteger();
  واپس std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ثابت {
  اگر (خودکار *V = get(K))
    واپس V->getAsString();
  واپس std::nullopt;
}
ثابت json::Object *Object::getObject(StringRef K) ثابت {
  اگر (خودکار *V = get(K))
    واپس V->getAsObject();
  واپس خالی_اشاریہ;
}
json::Object *Object::getObject(StringRef K) {
  اگر (خودکار *V = get(K))
    واپس V->getAsObject();
  واپس خالی_اشاریہ;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  اگر (خودکار *V = get(K))
    واپس V->getAsArray();
  واپس خالی_اشاریہ;
}
json::Array *Object::getArray(StringRef K) {
  اگر (خودکار *V = get(K))
    واپس V->getAsArray();
  واپس خالی_اشاریہ;
}
منطقی عامل==(ثابت Object &LHS, ثابت Object &RHS) { واپس LHS.M == RHS.M; }

Array::Array(std::initializer_list<قدر> Elements) {
  V.reserve(Elements.سائز_2());
  برائے (ثابت قدر &V : Elements) {
    emplace_عقب(خالی_اشاریہ);
    عقب().moveFrom(std::move(V));
  }
}

قدر::قدر(std::initializer_list<قدر> Elements)
    : قدر(json::Array(Elements)) {}

خالی قدر::copyFrom(ثابت قدر &M) {
  Type = M.Type;
  انتخاب (Type) {
  حالت T_Null:
  حالت T_Boolean:
  حالت T_Double:
  حالت T_Integer:
  حالت T_UINT64:
    حافظہ_نقل(&Union, &M.Union, حجم(Union));
    توڑو;
  حالت T_StringRef:
    create<StringRef>(M.as<StringRef>());
    توڑو;
  حالت T_String:
    create<std::string>(M.as<std::string>());
    توڑو;
  حالت T_Object:
    create<json::Object>(M.as<json::Object>());
    توڑو;
  حالت T_Array:
    create<json::Array>(M.as<json::Array>());
    توڑو;
  }
}

خالی قدر::moveFrom(ثابت قدر &&M) {
  Type = M.Type;
  انتخاب (Type) {
  حالت T_Null:
  حالت T_Boolean:
  حالت T_Double:
  حالت T_Integer:
  حالت T_UINT64:
    حافظہ_نقل(&Union, &M.Union, حجم(Union));
    توڑو;
  حالت T_StringRef:
    create<StringRef>(M.as<StringRef>());
    توڑو;
  حالت T_String:
    create<std::string>(std::move(M.as<std::string>()));
    توڑو;
  حالت T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    توڑو;
  حالت T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    توڑو;
  }
  const_cast<قدر &>(M).destroy();
  M.Type = T_Null;
}

خالی قدر::destroy() {
  انتخاب (Type) {
  حالت T_Null:
  حالت T_Boolean:
  حالت T_Double:
  حالت T_Integer:
  حالت T_UINT64:
    توڑو;
  حالت T_StringRef:
    as<StringRef>().~StringRef();
    توڑو;
  حالت T_String:
    as<std::string>().~basic_string();
    توڑو;
  حالت T_Object:
    as<json::Object>().~Object();
    توڑو;
  حالت T_Array:
    as<json::Array>().~Array();
    توڑو;
  }
}

خالی قدر::print(llvm::raw_ostream &OS) ثابت { OS << *یہ; }

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی قدر::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#اگر_ختم

منطقی عامل==(ثابت قدر &L, ثابت قدر &R) {
  اگر (L.قسم_2() != R.قسم_2())
    واپس جھوٹ;
  انتخاب (L.قسم_2()) {
  حالت قدر::Null:
    واپس *L.getAsNull() == *R.getAsNull();
  حالت قدر::Boolean:
    واپس *L.getAsBoolean() == *R.getAsBoolean();
  حالت قدر::نمبر:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    اگر (L.Type == قدر::T_Integer || R.Type == قدر::T_Integer)
      واپس L.getAsInteger() == R.getAsInteger();
    واپس *L.get_As_نمبر() == *R.get_As_نمبر();
  حالت قدر::String:
    واپس *L.getAsString() == *R.getAsString();
  حالت قدر::Array:
    واپس *L.getAsArray() == *R.getAsArray();
  حالت قدر::Object:
    واپس *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

خالی راستہ::report(llvm::StringLiteral پیغام_4) {
  // Walk up to the root context, and count the number of segments.
  غیرعلامتی گنتی_2 = 0;
  ثابت راستہ *P;
  برائے (P = یہ; P->والد_2 != خالی_اشاریہ; P = P->والد_2)
    ++گنتی_2;
  راستہ::جڑ_2 *R = P->Seg.جڑ_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_پیغام_2 = پیغام_4;
  R->Error_راستہ.resize(گنتی_2);
  خودکار It = R->Error_راستہ.begin();
  برائے (P = یہ; P->والد_2 != خالی_اشاریہ; P = P->والد_2)
    *It++ = P->Seg;
}

Error راستہ::جڑ_2::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_پیغام_2.خالی_خانے() ? "invalid JSON contents" : Error_پیغام_2);
  اگر (Error_راستہ.خالی_خانے()) {
    اگر (!Name.خالی_خانے())
      OS << " when parsing " << Name;
  } ورنہ {
    OS << " at " << (Name.خالی_خانے() ? "(root)" : Name);
    برائے (ثابت راستہ::Segment &S : llvm::الٹ(Error_راستہ)) {
      اگر (S.is_میدان())
        OS << '.' << S.میدان();
      ورنہ
        OS << '[' << S.اشاریہ() << ']';
    }
  }
  واپس createStringError(llvm::inconvertible_Error_رمز(), S);
}

std::سمتیہ<ثابت Object::قدر_type *> sortedElements(ثابت Object &O) {
  std::سمتیہ<ثابت Object::قدر_type *> Elements;
  برائے (ثابت خودکار &E : O)
    Elements.ڈالو_عقب(&E);
  llvm::ترتیب_دو(Elements,
             [](ثابت Object::قدر_type *L, ثابت Object::قدر_type *R) {
               واپس L->پہلا_2 < R->پہلا_2;
             });
  واپس Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ساکن خالی abbreviate(ثابت قدر &V, OStream &JOS) {
  انتخاب (V.قسم_2()) {
  حالت قدر::Array:
    JOS.raw_قدر(V.getAsArray()->خالی_خانے() ? "[]" : "[ ... ]");
    توڑو;
  حالت قدر::Object:
    JOS.raw_قدر(V.getAsObject()->خالی_خانے() ? "{}" : "{ ... }");
    توڑو;
  حالت قدر::String: {
    llvm::StringRef S = *V.getAsString();
    اگر (S.سائز_2() < 40) {
      JOS.قدر_2(V);
    } ورنہ {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.قدر_2(Truncated);
    }
    توڑو;
  }
  طےشدہ:
    JOS.قدر_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ساکن خالی abbreviateChildren(ثابت قدر &V, OStream &JOS) {
  انتخاب (V.قسم_2()) {
  حالت قدر::Array:
    JOS.array([&] {
      برائے (ثابت خودکار &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    توڑو;
  حالت قدر::Object:
    JOS.object([&] {
      برائے (ثابت خودکار *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->پہلا_2);
        abbreviate(KV->دوسرا_2, JOS);
        JOS.attribute_انتہا();
      }
    });
    توڑو;
  طےشدہ:
    JOS.قدر_2(V);
  }
}

خالی راستہ::جڑ_2::printErrorContext(ثابت قدر &R, raw_ostream &OS) ثابت {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  خودکار Print_قدر = [&](ثابت قدر &V, ArrayRef<Segment> راستہ, خودکار &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    خودکار HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_پیغام_2.ڈیٹا(), Error_پیغام_2.سائز_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    اگر (راستہ.خالی_خانے()) // We reached our target.
      واپس HighlightCurrent();
    ثابت Segment &S = راستہ.عقب(); // Path is in reverse order.
    اگر (S.is_میدان()) {
      // Current node is an object, path names a field.
      llvm::StringRef میدان_Name = S.میدان();
      ثابت Object *O = V.getAsObject();
      اگر (!O || !O->get(میدان_Name))
        واپس HighlightCurrent();
      JOS.object([&] {
        برائے (ثابت خودکار *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->پہلا_2);
          اگر (میدان_Name == StringRef(KV->پہلا_2))
            Recurse(KV->دوسرا_2, راستہ.drop_عقب(), Recurse);
          ورنہ
            abbreviate(KV->دوسرا_2, JOS);
          JOS.attribute_انتہا();
        }
      });
    } ورنہ {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      اگر (!A || S.اشاریہ() >= A->سائز_2())
        واپس HighlightCurrent();
      JOS.array([&] {
        غیرعلامتی Current = 0;
        برائے (ثابت خودکار &V : *A) {
          اگر (Current++ == S.اشاریہ())
            Recurse(V, راستہ.drop_عقب(), Recurse);
          ورنہ
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_قدر(R, Error_راستہ, Print_قدر);
}

نامخلا {
// Simple recursive-descent JSON parser.
جماعت Parser {
عوامی:
  Parser(StringRef JSON)
      : آغاز(JSON.begin()), P(JSON.begin()), انتہا_2(JSON.انتہا()) {}

  منطقی checkUTF8() {
    size_t Err_انحراف;
    اگر (isUTF8(StringRef(آغاز, انتہا_2 - آغاز), &Err_انحراف))
      واپس سچ;
    P = آغاز + Err_انحراف; // For line/column calculation.
    واپس parseError("Invalid UTF-8 sequence");
  }

  منطقی parse_قدر(قدر &برآمد);

  منطقی assert_انتہا() {
    eatWhitespace();
    اگر (P == انتہا_2)
      واپس سچ;
    واپس parseError("Text after end of document");
  }

  Error takeError() {
    أكد(Err);
    واپس std::move(*Err);
  }

نجی:
  خالی eatWhitespace() {
    جب (P != انتہا_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  منطقی parse_نمبر(حرف پہلا, قدر &برآمد);
  منطقی parseString(std::string &برآمد);
  منطقی parseUnicode(std::string &برآمد);
  منطقی parseError(ثابت حرف *پیغام_4); // always returns false

  حرف اگلا() { واپس P == انتہا_2 ? 0 : *P++; }
  حرف peek() { واپس P == انتہا_2 ? 0 : *P; }
  ساکن منطقی is_نمبر(حرف C) {
    واپس C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت حرف *آغاز, *P, *انتہا_2;
};
} // namespace

منطقی Parser::parse_قدر(قدر &برآمد) {
  eatWhitespace();
  اگر (P == انتہا_2)
    واپس parseError("Unexpected EOF");
  انتخاب (حرف C = اگلا()) {
  // Bare null/true/false are easy - first char identifies them.
  حالت 'n':
    برآمد = خالی_اشاریہ;
    واپس (اگلا() == 'u' && اگلا() == 'l' && اگلا() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالت 't':
    برآمد = سچ;
    واپس (اگلا() == 'r' && اگلا() == 'u' && اگلا() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالت 'f':
    برآمد = جھوٹ;
    واپس (اگلا() == 'a' && اگلا() == 'l' && اگلا() == 's' && اگلا() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالت '"': {
    std::string S;
    اگر (parseString(S)) {
      برآمد = std::move(S);
      واپس سچ;
    }
    واپس جھوٹ;
  }
  حالت '[': {
    برآمد = Array{};
    Array &A = *برآمد.getAsArray();
    eatWhitespace();
    اگر (peek() == ']') {
      ++P;
      واپس سچ;
    }
    برائے (;;) {
      A.emplace_عقب(خالی_اشاریہ);
      اگر (!parse_قدر(A.عقب()))
        واپس جھوٹ;
      eatWhitespace();
      انتخاب (اگلا()) {
      حالت ',':
        eatWhitespace();
        جاری;
      حالت ']':
        واپس سچ;
      طےشدہ:
        واپس parseError("Expected , or ] after array element");
      }
    }
  }
  حالت '{': {
    برآمد = Object{};
    Object &O = *برآمد.getAsObject();
    eatWhitespace();
    اگر (peek() == '}') {
      ++P;
      واپس سچ;
    }
    برائے (;;) {
      اگر (اگلا() != '"')
        واپس parseError("Expected object key");
      std::string K;
      اگر (!parseString(K))
        واپس جھوٹ;
      eatWhitespace();
      اگر (اگلا() != ':')
        واپس parseError("Expected : after object key");
      eatWhitespace();
      اگر (!parse_قدر(O[std::move(K)]))
        واپس جھوٹ;
      eatWhitespace();
      انتخاب (اگلا()) {
      حالت ',':
        eatWhitespace();
        جاری;
      حالت '}':
        واپس سچ;
      طےشدہ:
        واپس parseError("Expected , or } after object property");
      }
    }
  }
  طےشدہ:
    اگر (is_نمبر(C))
      واپس parse_نمبر(C, برآمد);
    واپس parseError("Invalid JSON value");
  }
}

منطقی Parser::parse_نمبر(حرف پہلا, قدر &برآمد) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ڈالو_عقب(پہلا);
  جب (is_نمبر(peek()))
    S.ڈالو_عقب(اگلا());
  حرف *انتہا_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &انتہا_2, 10);
  اگر (انتہا_2 == S.انتہا() && errno != ERANGE) {
    برآمد = I;
    واپس سچ;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  اگر (پہلا != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &انتہا_2, 10);
    اگر (انتہا_2 == S.انتہا() && errno != ERANGE) {
      برآمد = UI;
      واپس سچ;
    }
  }
  // If it's not an integer
  برآمد = std::strtod(S.c_str(), &انتہا_2);
  واپس انتہا_2 == S.انتہا() || parseError("Invalid JSON value (number?)");
}

منطقی Parser::parseString(std::string &برآمد) {
  // leading quote was already consumed.
  برائے (حرف C = اگلا(); C != '"'; C = اگلا()) {
    اگر (LLVM_UNLIKELY(P == انتہا_2))
      واپس parseError("Unterminated string");
    اگر (LLVM_UNLIKELY((C & 0x1f) == C))
      واپس parseError("Control character in string");
    اگر (LLVM_LIKELY(C != '\\')) {
      برآمد.ڈالو_عقب(C);
      جاری;
    }
    // Handle escape sequence.
    انتخاب (C = اگلا()) {
    حالت '"':
    حالت '\\':
    حالت '/':
      برآمد.ڈالو_عقب(C);
      توڑو;
    حالت 'b':
      برآمد.ڈالو_عقب('\b');
      توڑو;
    حالت 'f':
      برآمد.ڈالو_عقب('\f');
      توڑو;
    حالت 'n':
      برآمد.ڈالو_عقب('\n');
      توڑو;
    حالت 'r':
      برآمد.ڈالو_عقب('\r');
      توڑو;
    حالت 't':
      برآمد.ڈالو_عقب('\t');
      توڑو;
    حالت 'u':
      اگر (!parseUnicode(برآمد))
        واپس جھوٹ;
      توڑو;
    طےشدہ:
      واپس parseError("Invalid escape sequence");
    }
  }
  واپس سچ;
}

ساکن خالی encodeUtf8(uint32_t Rune, std::string &برآمد) {
  اگر (Rune < 0x80) {
    برآمد.ڈالو_عقب(Rune & 0x7F);
  } ورنہ اگر (Rune < 0x800) {
    uint8_t پہلا_Byte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t دوسرا_Byte = 0x80 | (Rune & 0x3F);
    برآمد.ڈالو_عقب(پہلا_Byte);
    برآمد.ڈالو_عقب(دوسرا_Byte);
  } ورنہ اگر (Rune < 0x10000) {
    uint8_t پہلا_Byte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t دوسرا_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t تیسرا_Byte = 0x80 | (Rune & 0x3F);
    برآمد.ڈالو_عقب(پہلا_Byte);
    برآمد.ڈالو_عقب(دوسرا_Byte);
    برآمد.ڈالو_عقب(تیسرا_Byte);
  } ورنہ اگر (Rune < 0x110000) {
    uint8_t پہلا_Byte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t دوسرا_Byte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t تیسرا_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    برآمد.ڈالو_عقب(پہلا_Byte);
    برآمد.ڈالو_عقب(دوسرا_Byte);
    برآمد.ڈالو_عقب(تیسرا_Byte);
    برآمد.ڈالو_عقب(FourthByte);
  } ورنہ {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
منطقی Parser::parseUnicode(std::string &برآمد) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  خودکار Invalid = [&] { برآمد.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  خودکار Parse4Hex = [یہ](uint16_t &برآمد) -> منطقی {
    برآمد = 0;
    حرف بائٹیں_2[] = {اگلا(), اگلا(), اگلا(), اگلا()};
    برائے (غیرعلامتی حرف C : بائٹیں_2) {
      اگر (!std::isxdigit(C))
        واپس parseError("Invalid \\u escape sequence");
      برآمد <<= 4;
      برآمد |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    واپس سچ;
  };
  uint16_t پہلا; // UTF-16 code unit from the first \u escape.
  اگر (!Parse4Hex(پہلا))
    واپس جھوٹ;

  // We loop to allow proper surrogate-pair error handling.
  جب (سچ) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    اگر (LLVM_LIKELY(پہلا < 0xD800 || پہلا >= 0xE000)) {
      encodeUtf8(پہلا, برآمد);
      واپس سچ;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    اگر (LLVM_UNLIKELY(پہلا >= 0xDC00)) {
      Invalid();
      واپس سچ;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    اگر (LLVM_UNLIKELY(P + 2 > انتہا_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      واپس سچ;
    }
    P += 2;
    uint16_t دوسرا;
    اگر (!Parse4Hex(دوسرا))
      واپس جھوٹ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    اگر (LLVM_UNLIKELY(دوسرا < 0xDC00 || دوسرا >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      پہلا = دوسرا; // Second escape still needs to be processed.
      جاری;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((پہلا - 0xD800) << 10) | (دوسرا - 0xDC00), برآمد);
    واپس سچ;
  }
}

منطقی Parser::parseError(ثابت حرف *پیغام_4) {
  صحیح سطر = 1;
  ثابت حرف *آغاز_کا_سطر = آغاز;
  برائے (ثابت حرف *X = آغاز; X < P; ++X) {
    اگر (*X == 0x0A) {
      ++سطر;
      آغاز_کا_سطر = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(پیغام_4, سطر, P - آغاز_کا_سطر, P - آغاز));
  واپس جھوٹ;
}

Expected<قدر> parse(StringRef JSON) {
  Parser P(JSON);
  قدر E = خالی_اشاریہ;
  اگر (P.checkUTF8())
    اگر (P.parse_قدر(E))
      اگر (P.assert_انتہا())
        واپس std::move(E);
  واپس P.takeError();
}

حرف ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
خالی ParseError::لوگارتھم(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", سطر, عمود, انحراف, پیغام_4);
}

منطقی isUTF8(llvm::StringRef S, size_t *Err_انحراف) {
  // Fast-path for ASCII, which is valid UTF-8.
  اگر (LLVM_LIKELY(isASCII(S)))
    واپس سچ;

  ثابت UTF8 *ڈیٹا_2 = reinterpret_cast<ثابت UTF8 *>(S.ڈیٹا()), *Rest = ڈیٹا_2;
  اگر (LLVM_LIKELY(isLegalUTF8String(&Rest, ڈیٹا_2 + S.سائز_2())))
    واپس سچ;

  اگر (Err_انحراف)
    *Err_انحراف = Rest - ڈیٹا_2;
  واپس جھوٹ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::سمتیہ<UTF32> Codepoints(S.سائز_2()); // 1 codepoint per byte suffices.
  ثابت UTF8 *In8 = reinterpret_cast<ثابت UTF8 *>(S.ڈیٹا());
  UTF32 *Out32 = Codepoints.ڈیٹا();
  ConvertUTF8toUTF32(&In8, In8 + S.سائز_2(), &Out32, Out32 + Codepoints.سائز_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.ڈیٹا());
  std::string Res(4 * Codepoints.سائز_2(), 0); // 4 bytes per codepoint suffice
  ثابت UTF32 *In32 = Codepoints.ڈیٹا();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.سائز_2(), &Out8, Out8 + Res.سائز_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<حرف *>(Out8) - Res.ڈیٹا());
  واپس Res;
}

ساکن خالی quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  برائے (غیرعلامتی حرف C : S) {
    اگر (C == 0x22 || C == 0x5C)
      OS << '\\';
    اگر (C >= 0x20) {
      OS << C;
      جاری;
    }
    OS << '\\';
    انتخاب (C) {
    // A few characters are common enough to make short escapes worthwhile.
    حالت '\t':
      OS << 't';
      توڑو;
    حالت '\n':
      OS << 'n';
      توڑو;
    حالت '\r':
      OS << 'r';
      توڑو;
    طےشدہ:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::زیریں_2, 4);
      توڑو;
    }
  }
  OS << '\"';
}

خالی llvm::json::OStream::قدر_2(ثابت قدر &V) {
  انتخاب (V.قسم_2()) {
  حالت قدر::Null:
    قدر_Begin();
    OS << "null";
    واپس;
  حالت قدر::Boolean:
    قدر_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    واپس;
  حالت قدر::نمبر:
    قدر_Begin();
    اگر (V.Type == قدر::T_Integer)
      OS << *V.getAsInteger();
    ورنہ اگر (V.Type == قدر::T_UINT64)
      OS << *V.getAsUINT64();
    ورنہ
      OS << format("%.*g", std::numeric_limits<دوہرا>::max_digits10,
                   *V.get_As_نمبر());
    واپس;
  حالت قدر::String:
    قدر_Begin();
    quote(OS, *V.getAsString());
    واپس;
  حالت قدر::Array:
    واپس array([&] {
      برائے (ثابت قدر &E : *V.getAsArray())
        قدر_2(E);
    });
  حالت قدر::Object:
    واپس object([&] {
      برائے (ثابت Object::قدر_type *E : sortedElements(*V.getAsObject()))
        attribute(E->پہلا_2, E->دوسرا_2);
    });
  }
}

خالی llvm::json::OStream::قدر_Begin() {
  أكد(ڈھیر.عقب().Ctx != Object && "Only attributes allowed here");
  اگر (ڈھیر.عقب().Has_قدر) {
    أكد(ڈھیر.عقب().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  اگر (ڈھیر.عقب().Ctx == Array)
    newline();
  flushComment();
  ڈھیر.عقب().Has_قدر = سچ;
}

خالی OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.خالی_خانے() && "Only one comment per value!");
  PendingComment = Comment;
}

خالی OStream::flushComment() {
  اگر (PendingComment.خالی_خانے())
    واپس;
  OS << (Indent_سائز ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  جب (!PendingComment.خالی_خانے()) {
    خودکار Pos = PendingComment.ڈھونڈو("*/");
    اگر (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } ورنہ {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_سائز ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  اگر (ڈھیر.سائز_2() > 1 && ڈھیر.عقب().Ctx == Singleton) {
    اگر (Indent_سائز)
      OS << ' ';
  } ورنہ {
    newline();
  }
}

خالی llvm::json::OStream::newline() {
  اگر (Indent_سائز) {
    OS.اكتب('\n');
    OS.indent(Indent);
  }
}

خالی llvm::json::OStream::arrayBegin() {
  قدر_Begin();
  ڈھیر.emplace_عقب();
  ڈھیر.عقب().Ctx = Array;
  Indent += Indent_سائز;
  OS << '[';
}

خالی llvm::json::OStream::array_انتہا() {
  أكد(ڈھیر.عقب().Ctx == Array);
  Indent -= Indent_سائز;
  اگر (ڈھیر.عقب().Has_قدر)
    newline();
  OS << ']';
  أكد(PendingComment.خالی_خانے());
  ڈھیر.نکالو_عقب();
  أكد(!ڈھیر.خالی_خانے());
}

خالی llvm::json::OStream::objectBegin() {
  قدر_Begin();
  ڈھیر.emplace_عقب();
  ڈھیر.عقب().Ctx = Object;
  Indent += Indent_سائز;
  OS << '{';
}

خالی llvm::json::OStream::object_انتہا() {
  أكد(ڈھیر.عقب().Ctx == Object);
  Indent -= Indent_سائز;
  اگر (ڈھیر.عقب().Has_قدر)
    newline();
  OS << '}';
  أكد(PendingComment.خالی_خانے());
  ڈھیر.نکالو_عقب();
  أكد(!ڈھیر.خالی_خانے());
}

خالی llvm::json::OStream::attributeBegin(llvm::StringRef کلید) {
  أكد(ڈھیر.عقب().Ctx == Object);
  اگر (ڈھیر.عقب().Has_قدر)
    OS << ',';
  newline();
  flushComment();
  ڈھیر.عقب().Has_قدر = سچ;
  ڈھیر.emplace_عقب();
  ڈھیر.عقب().Ctx = Singleton;
  اگر (LLVM_LIKELY(isUTF8(کلید))) {
    quote(OS, کلید);
  } ورنہ {
    أكد(جھوٹ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(کلید));
  }
  OS.اكتب(':');
  اگر (Indent_سائز)
    OS.اكتب(' ');
}

خالی llvm::json::OStream::attribute_انتہا() {
  أكد(ڈھیر.عقب().Ctx == Singleton);
  أكد(ڈھیر.عقب().Has_قدر && "Attribute must have a value");
  أكد(PendingComment.خالی_خانے());
  ڈھیر.نکالو_عقب();
  أكد(ڈھیر.عقب().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_قدر_Begin() {
  قدر_Begin();
  ڈھیر.emplace_عقب();
  ڈھیر.عقب().Ctx = Raw_قدر;
  واپس OS;
}

خالی llvm::json::OStream::raw_قدر_انتہا() {
  أكد(ڈھیر.عقب().Ctx == Raw_قدر);
  ڈھیر.نکالو_عقب();
}

} // namespace json
} // namespace llvm

خالی llvm::format_provider<llvm::json::قدر>::format(
    ثابت llvm::json::قدر &E, raw_ostream &OS, StringRef Options) {
  غیرعلامتی IndentAmount = 0;
  اگر (!Options.خالی_خانے() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).قدر_2(E);
}

