//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ضمن "llvm/Support/JSON.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/NativeFormatting.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cctype>
#ضمن <cerrno>
#ضمن <optional>

نطاق llvm {
نطاق json {

ارزښت &Object::چلوونکی[](ثابت Object_کلی &K) {
  أرجع try_emplace(K, تش_اشاره).لومړی_2->get_دوهم();
}
ارزښت &Object::چلوونکی[](Object_کلی &&K) {
  أرجع try_emplace(std::move(K), تش_اشاره).لومړی_2->get_دوهم();
}
ارزښت *Object::get(StringRef K) {
  خپلکاری I = پیدا_کړه(K);
  إذا (I == پای_ټکی())
    أرجع تش_اشاره;
  أرجع &I->دوهم_2;
}
ثابت ارزښت *Object::get(StringRef K) ثابت {
  خپلکاری I = پیدا_کړه(K);
  إذا (I == پای_ټکی())
    أرجع تش_اشاره;
  أرجع &I->دوهم_2;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ثابت {
  إذا (خپلکاری *V = get(K))
    أرجع V->getAsNull();
  أرجع std::nullopt;
}
std::optional<منطقي> Object::getBoolean(StringRef K) ثابت {
  إذا (خپلکاری *V = get(K))
    أرجع V->getAsBoolean();
  أرجع std::nullopt;
}
std::optional<مزدوج> Object::get_شمېره(StringRef K) ثابت {
  إذا (خپلکاری *V = get(K))
    أرجع V->get_As_شمېره();
  أرجع std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ثابت {
  إذا (خپلکاری *V = get(K))
    أرجع V->getAsInteger();
  أرجع std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ثابت {
  إذا (خپلکاری *V = get(K))
    أرجع V->getAsString();
  أرجع std::nullopt;
}
ثابت json::Object *Object::getObject(StringRef K) ثابت {
  إذا (خپلکاری *V = get(K))
    أرجع V->getAsObject();
  أرجع تش_اشاره;
}
json::Object *Object::getObject(StringRef K) {
  إذا (خپلکاری *V = get(K))
    أرجع V->getAsObject();
  أرجع تش_اشاره;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  إذا (خپلکاری *V = get(K))
    أرجع V->getAsArray();
  أرجع تش_اشاره;
}
json::Array *Object::getArray(StringRef K) {
  إذا (خپلکاری *V = get(K))
    أرجع V->getAsArray();
  أرجع تش_اشاره;
}
منطقي چلوونکی==(ثابت Object &LHS, ثابت Object &RHS) { أرجع LHS.M == RHS.M; }

Array::Array(std::initializer_list<ارزښت> Elements) {
  V.reserve(Elements.اندازه_2());
  لكل (ثابت ارزښت &V : Elements) {
    emplace_شا(تش_اشاره);
    شا().moveFrom(std::move(V));
  }
}

ارزښت::ارزښت(std::initializer_list<ارزښت> Elements)
    : ارزښت(json::Array(Elements)) {}

فراغ ارزښت::copyFrom(ثابت ارزښت &M) {
  Type = M.Type;
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    حافظه_نقل(&Union, &M.Union, حجم(Union));
    اكسر;
  حالة T_StringRef:
    create<StringRef>(M.as<StringRef>());
    اكسر;
  حالة T_String:
    create<std::string>(M.as<std::string>());
    اكسر;
  حالة T_Object:
    create<json::Object>(M.as<json::Object>());
    اكسر;
  حالة T_Array:
    create<json::Array>(M.as<json::Array>());
    اكسر;
  }
}

فراغ ارزښت::moveFrom(ثابت ارزښت &&M) {
  Type = M.Type;
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    حافظه_نقل(&Union, &M.Union, حجم(Union));
    اكسر;
  حالة T_StringRef:
    create<StringRef>(M.as<StringRef>());
    اكسر;
  حالة T_String:
    create<std::string>(std::move(M.as<std::string>()));
    اكسر;
  حالة T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    اكسر;
  حالة T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    اكسر;
  }
  const_cast<ارزښت &>(M).destroy();
  M.Type = T_Null;
}

فراغ ارزښت::destroy() {
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    اكسر;
  حالة T_StringRef:
    as<StringRef>().~StringRef();
    اكسر;
  حالة T_String:
    as<std::string>().~basic_string();
    اكسر;
  حالة T_Object:
    as<json::Object>().~Object();
    اكسر;
  حالة T_Array:
    as<json::Array>().~Array();
    اكسر;
  }
}

فراغ ارزښت::print(llvm::raw_ostream &OS) ثابت { OS << *هذا; }

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_لاره فراغ ارزښت::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#د_که_پای

منطقي چلوونکی==(ثابت ارزښت &L, ثابت ارزښت &R) {
  إذا (L.ډول() != R.ډول())
    أرجع خطأ;
  بدل (L.ډول()) {
  حالة ارزښت::Null:
    أرجع *L.getAsNull() == *R.getAsNull();
  حالة ارزښت::Boolean:
    أرجع *L.getAsBoolean() == *R.getAsBoolean();
  حالة ارزښت::شمېره:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    إذا (L.Type == ارزښت::T_Integer || R.Type == ارزښت::T_Integer)
      أرجع L.getAsInteger() == R.getAsInteger();
    أرجع *L.get_As_شمېره() == *R.get_As_شمېره();
  حالة ارزښت::String:
    أرجع *L.getAsString() == *R.getAsString();
  حالة ارزښت::Array:
    أرجع *L.getAsArray() == *R.getAsArray();
  حالة ارزښت::Object:
    أرجع *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

فراغ لار::report(llvm::StringLiteral پیغام_4) {
  // Walk up to the root context, and count the number of segments.
  بینښه شمېر_2 = 0;
  ثابت لار *P;
  لكل (P = هذا; P->پلار_2 != تش_اشاره; P = P->پلار_2)
    ++شمېر_2;
  لار::ریښه_2 *R = P->Seg.ریښه_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_پیغام_2 = پیغام_4;
  R->Error_لار.resize(شمېر_2);
  خپلکاری It = R->Error_لار.begin();
  لكل (P = هذا; P->پلار_2 != تش_اشاره; P = P->پلار_2)
    *It++ = P->Seg;
}

Error لار::ریښه_2::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_پیغام_2.تشه() ? "invalid JSON contents" : Error_پیغام_2);
  إذا (Error_لار.تشه()) {
    إذا (!Name.تشه())
      OS << " when parsing " << Name;
  } وإلا {
    OS << " at " << (Name.تشه() ? "(root)" : Name);
    لكل (ثابت لار::Segment &S : llvm::سرچپه(Error_لار)) {
      إذا (S.is_ډګر())
        OS << '.' << S.ډګر();
      وإلا
        OS << '[' << S.شاخص() << ']';
    }
  }
  أرجع createStringError(llvm::inconvertible_Error_کوډ(), S);
}

std::ویکټور<ثابت Object::ارزښت_type *> sortedElements(ثابت Object &O) {
  std::ویکټور<ثابت Object::ارزښت_type *> Elements;
  لكل (ثابت خپلکاری &E : O)
    Elements.ټېل_شا(&E);
  llvm::ترتیبول(Elements,
             [](ثابت Object::ارزښت_type *L, ثابت Object::ارزښت_type *R) {
               أرجع L->لومړی_2 < R->لومړی_2;
             });
  أرجع Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ساكن فراغ abbreviate(ثابت ارزښت &V, OStream &JOS) {
  بدل (V.ډول()) {
  حالة ارزښت::Array:
    JOS.raw_ارزښت(V.getAsArray()->تشه() ? "[]" : "[ ... ]");
    اكسر;
  حالة ارزښت::Object:
    JOS.raw_ارزښت(V.getAsObject()->تشه() ? "{}" : "{ ... }");
    اكسر;
  حالة ارزښت::String: {
    llvm::StringRef S = *V.getAsString();
    إذا (S.اندازه_2() < 40) {
      JOS.ارزښت_2(V);
    } وإلا {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.ارزښت_2(Truncated);
    }
    اكسر;
  }
  افتراضي:
    JOS.ارزښت_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ساكن فراغ abbreviateChildren(ثابت ارزښت &V, OStream &JOS) {
  بدل (V.ډول()) {
  حالة ارزښت::Array:
    JOS.array([&] {
      لكل (ثابت خپلکاری &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    اكسر;
  حالة ارزښت::Object:
    JOS.object([&] {
      لكل (ثابت خپلکاری *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->لومړی_2);
        abbreviate(KV->دوهم_2, JOS);
        JOS.attribute_پای_ټکی();
      }
    });
    اكسر;
  افتراضي:
    JOS.ارزښت_2(V);
  }
}

فراغ لار::ریښه_2::printErrorContext(ثابت ارزښت &R, raw_ostream &OS) ثابت {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  خپلکاری Print_ارزښت = [&](ثابت ارزښت &V, ArrayRef<Segment> لار, خپلکاری &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    خپلکاری HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_پیغام_2.معلومات(), Error_پیغام_2.اندازه_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    إذا (لار.تشه()) // We reached our target.
      أرجع HighlightCurrent();
    ثابت Segment &S = لار.شا(); // Path is in reverse order.
    إذا (S.is_ډګر()) {
      // Current node is an object, path names a field.
      llvm::StringRef ډګر_Name = S.ډګر();
      ثابت Object *O = V.getAsObject();
      إذا (!O || !O->get(ډګر_Name))
        أرجع HighlightCurrent();
      JOS.object([&] {
        لكل (ثابت خپلکاری *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->لومړی_2);
          إذا (ډګر_Name == StringRef(KV->لومړی_2))
            Recurse(KV->دوهم_2, لار.drop_شا(), Recurse);
          وإلا
            abbreviate(KV->دوهم_2, JOS);
          JOS.attribute_پای_ټکی();
        }
      });
    } وإلا {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      إذا (!A || S.شاخص() >= A->اندازه_2())
        أرجع HighlightCurrent();
      JOS.array([&] {
        بینښه Current = 0;
        لكل (ثابت خپلکاری &V : *A) {
          إذا (Current++ == S.شاخص())
            Recurse(V, لار.drop_شا(), Recurse);
          وإلا
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_ارزښت(R, Error_لار, Print_ارزښت);
}

نطاق {
// Simple recursive-descent JSON parser.
صنف Parser {
عام:
  Parser(StringRef JSON)
      : پیل(JSON.begin()), P(JSON.begin()), پای_ټکی_2(JSON.پای_ټکی()) {}

  منطقي checkUTF8() {
    size_t Err_دننه_واټن;
    إذا (isUTF8(StringRef(پیل, پای_ټکی_2 - پیل), &Err_دننه_واټن))
      أرجع ریښتیا;
    P = پیل + Err_دننه_واټن; // For line/column calculation.
    أرجع parseError("Invalid UTF-8 sequence");
  }

  منطقي parse_ارزښت(ارزښت &بهر);

  منطقي assert_پای_ټکی() {
    eatWhitespace();
    إذا (P == پای_ټکی_2)
      أرجع ریښتیا;
    أرجع parseError("Text after end of document");
  }

  Error takeError() {
    أكد(Err);
    أرجع std::move(*Err);
  }

خاص:
  فراغ eatWhitespace() {
    طالما (P != پای_ټکی_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  منطقي parse_شمېره(حرف لومړی, ارزښت &بهر);
  منطقي parseString(std::string &بهر);
  منطقي parseUnicode(std::string &بهر);
  منطقي parseError(ثابت حرف *پیغام_4); // always returns false

  حرف بل() { أرجع P == پای_ټکی_2 ? 0 : *P++; }
  حرف peek() { أرجع P == پای_ټکی_2 ? 0 : *P; }
  ساكن منطقي is_شمېره(حرف C) {
    أرجع C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت حرف *پیل, *P, *پای_ټکی_2;
};
} // namespace

منطقي Parser::parse_ارزښت(ارزښت &بهر) {
  eatWhitespace();
  إذا (P == پای_ټکی_2)
    أرجع parseError("Unexpected EOF");
  بدل (حرف C = بل()) {
  // Bare null/true/false are easy - first char identifies them.
  حالة 'n':
    بهر = تش_اشاره;
    أرجع (بل() == 'u' && بل() == 'l' && بل() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالة 't':
    بهر = ریښتیا;
    أرجع (بل() == 'r' && بل() == 'u' && بل() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالة 'f':
    بهر = خطأ;
    أرجع (بل() == 'a' && بل() == 'l' && بل() == 's' && بل() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالة '"': {
    std::string S;
    إذا (parseString(S)) {
      بهر = std::move(S);
      أرجع ریښتیا;
    }
    أرجع خطأ;
  }
  حالة '[': {
    بهر = Array{};
    Array &A = *بهر.getAsArray();
    eatWhitespace();
    إذا (peek() == ']') {
      ++P;
      أرجع ریښتیا;
    }
    لكل (;;) {
      A.emplace_شا(تش_اشاره);
      إذا (!parse_ارزښت(A.شا()))
        أرجع خطأ;
      eatWhitespace();
      بدل (بل()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة ']':
        أرجع ریښتیا;
      افتراضي:
        أرجع parseError("Expected , or ] after array element");
      }
    }
  }
  حالة '{': {
    بهر = Object{};
    Object &O = *بهر.getAsObject();
    eatWhitespace();
    إذا (peek() == '}') {
      ++P;
      أرجع ریښتیا;
    }
    لكل (;;) {
      إذا (بل() != '"')
        أرجع parseError("Expected object key");
      std::string K;
      إذا (!parseString(K))
        أرجع خطأ;
      eatWhitespace();
      إذا (بل() != ':')
        أرجع parseError("Expected : after object key");
      eatWhitespace();
      إذا (!parse_ارزښت(O[std::move(K)]))
        أرجع خطأ;
      eatWhitespace();
      بدل (بل()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة '}':
        أرجع ریښتیا;
      افتراضي:
        أرجع parseError("Expected , or } after object property");
      }
    }
  }
  افتراضي:
    إذا (is_شمېره(C))
      أرجع parse_شمېره(C, بهر);
    أرجع parseError("Invalid JSON value");
  }
}

منطقي Parser::parse_شمېره(حرف لومړی, ارزښت &بهر) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ټېل_شا(لومړی);
  طالما (is_شمېره(peek()))
    S.ټېل_شا(بل());
  حرف *پای_ټکی_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &پای_ټکی_2, 10);
  إذا (پای_ټکی_2 == S.پای_ټکی() && errno != ERANGE) {
    بهر = I;
    أرجع ریښتیا;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  إذا (لومړی != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &پای_ټکی_2, 10);
    إذا (پای_ټکی_2 == S.پای_ټکی() && errno != ERANGE) {
      بهر = UI;
      أرجع ریښتیا;
    }
  }
  // If it's not an integer
  بهر = std::strtod(S.c_str(), &پای_ټکی_2);
  أرجع پای_ټکی_2 == S.پای_ټکی() || parseError("Invalid JSON value (number?)");
}

منطقي Parser::parseString(std::string &بهر) {
  // leading quote was already consumed.
  لكل (حرف C = بل(); C != '"'; C = بل()) {
    إذا (LLVM_UNLIKELY(P == پای_ټکی_2))
      أرجع parseError("Unterminated string");
    إذا (LLVM_UNLIKELY((C & 0x1f) == C))
      أرجع parseError("Control character in string");
    إذا (LLVM_LIKELY(C != '\\')) {
      بهر.ټېل_شا(C);
      استمر;
    }
    // Handle escape sequence.
    بدل (C = بل()) {
    حالة '"':
    حالة '\\':
    حالة '/':
      بهر.ټېل_شا(C);
      اكسر;
    حالة 'b':
      بهر.ټېل_شا('\b');
      اكسر;
    حالة 'f':
      بهر.ټېل_شا('\f');
      اكسر;
    حالة 'n':
      بهر.ټېل_شا('\n');
      اكسر;
    حالة 'r':
      بهر.ټېل_شا('\r');
      اكسر;
    حالة 't':
      بهر.ټېل_شا('\t');
      اكسر;
    حالة 'u':
      إذا (!parseUnicode(بهر))
        أرجع خطأ;
      اكسر;
    افتراضي:
      أرجع parseError("Invalid escape sequence");
    }
  }
  أرجع ریښتیا;
}

ساكن فراغ encodeUtf8(uint32_t Rune, std::string &بهر) {
  إذا (Rune < 0x80) {
    بهر.ټېل_شا(Rune & 0x7F);
  } وإلا إذا (Rune < 0x800) {
    uint8_t لومړی_Byte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t دوهم_Byte = 0x80 | (Rune & 0x3F);
    بهر.ټېل_شا(لومړی_Byte);
    بهر.ټېل_شا(دوهم_Byte);
  } وإلا إذا (Rune < 0x10000) {
    uint8_t لومړی_Byte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t دوهم_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t دریم_Byte = 0x80 | (Rune & 0x3F);
    بهر.ټېل_شا(لومړی_Byte);
    بهر.ټېل_شا(دوهم_Byte);
    بهر.ټېل_شا(دریم_Byte);
  } وإلا إذا (Rune < 0x110000) {
    uint8_t لومړی_Byte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t دوهم_Byte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t دریم_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    بهر.ټېل_شا(لومړی_Byte);
    بهر.ټېل_شا(دوهم_Byte);
    بهر.ټېل_شا(دریم_Byte);
    بهر.ټېل_شا(FourthByte);
  } وإلا {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
منطقي Parser::parseUnicode(std::string &بهر) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  خپلکاری Invalid = [&] { بهر.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  خپلکاری Parse4Hex = [هذا](uint16_t &بهر) -> منطقي {
    بهر = 0;
    حرف بایټونه_2[] = {بل(), بل(), بل(), بل()};
    لكل (بینښه حرف C : بایټونه_2) {
      إذا (!std::isxdigit(C))
        أرجع parseError("Invalid \\u escape sequence");
      بهر <<= 4;
      بهر |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    أرجع ریښتیا;
  };
  uint16_t لومړی; // UTF-16 code unit from the first \u escape.
  إذا (!Parse4Hex(لومړی))
    أرجع خطأ;

  // We loop to allow proper surrogate-pair error handling.
  طالما (ریښتیا) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    إذا (LLVM_LIKELY(لومړی < 0xD800 || لومړی >= 0xE000)) {
      encodeUtf8(لومړی, بهر);
      أرجع ریښتیا;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    إذا (LLVM_UNLIKELY(لومړی >= 0xDC00)) {
      Invalid();
      أرجع ریښتیا;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    إذا (LLVM_UNLIKELY(P + 2 > پای_ټکی_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      أرجع ریښتیا;
    }
    P += 2;
    uint16_t دوهم;
    إذا (!Parse4Hex(دوهم))
      أرجع خطأ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    إذا (LLVM_UNLIKELY(دوهم < 0xDC00 || دوهم >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      لومړی = دوهم; // Second escape still needs to be processed.
      استمر;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((لومړی - 0xD800) << 10) | (دوهم - 0xDC00), بهر);
    أرجع ریښتیا;
  }
}

منطقي Parser::parseError(ثابت حرف *پیغام_4) {
  صحيح کرښه = 1;
  ثابت حرف *پیل_د_کرښه = پیل;
  لكل (ثابت حرف *X = پیل; X < P; ++X) {
    إذا (*X == 0x0A) {
      ++کرښه;
      پیل_د_کرښه = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(پیغام_4, کرښه, P - پیل_د_کرښه, P - پیل));
  أرجع خطأ;
}

Expected<ارزښت> parse(StringRef JSON) {
  Parser P(JSON);
  ارزښت E = تش_اشاره;
  إذا (P.checkUTF8())
    إذا (P.parse_ارزښت(E))
      إذا (P.assert_پای_ټکی())
        أرجع std::move(E);
  أرجع P.takeError();
}

حرف ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
فراغ ParseError::لوګاریتم(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", کرښه, ستنه, دننه_واټن, پیغام_4);
}

منطقي isUTF8(llvm::StringRef S, size_t *Err_دننه_واټن) {
  // Fast-path for ASCII, which is valid UTF-8.
  إذا (LLVM_LIKELY(isASCII(S)))
    أرجع ریښتیا;

  ثابت UTF8 *معلومات_2 = reinterpret_cast<ثابت UTF8 *>(S.معلومات()), *Rest = معلومات_2;
  إذا (LLVM_LIKELY(isLegalUTF8String(&Rest, معلومات_2 + S.اندازه_2())))
    أرجع ریښتیا;

  إذا (Err_دننه_واټن)
    *Err_دننه_واټن = Rest - معلومات_2;
  أرجع خطأ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::ویکټور<UTF32> Codepoints(S.اندازه_2()); // 1 codepoint per byte suffices.
  ثابت UTF8 *In8 = reinterpret_cast<ثابت UTF8 *>(S.معلومات());
  UTF32 *Out32 = Codepoints.معلومات();
  ConvertUTF8toUTF32(&In8, In8 + S.اندازه_2(), &Out32, Out32 + Codepoints.اندازه_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.معلومات());
  std::string Res(4 * Codepoints.اندازه_2(), 0); // 4 bytes per codepoint suffice
  ثابت UTF32 *In32 = Codepoints.معلومات();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.اندازه_2(), &Out8, Out8 + Res.اندازه_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<حرف *>(Out8) - Res.معلومات());
  أرجع Res;
}

ساكن فراغ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  لكل (بینښه حرف C : S) {
    إذا (C == 0x22 || C == 0x5C)
      OS << '\\';
    إذا (C >= 0x20) {
      OS << C;
      استمر;
    }
    OS << '\\';
    بدل (C) {
    // A few characters are common enough to make short escapes worthwhile.
    حالة '\t':
      OS << 't';
      اكسر;
    حالة '\n':
      OS << 'n';
      اكسر;
    حالة '\r':
      OS << 'r';
      اكسر;
    افتراضي:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::ښکتنی_2, 4);
      اكسر;
    }
  }
  OS << '\"';
}

فراغ llvm::json::OStream::ارزښت_2(ثابت ارزښت &V) {
  بدل (V.ډول()) {
  حالة ارزښت::Null:
    ارزښت_Begin();
    OS << "null";
    أرجع;
  حالة ارزښت::Boolean:
    ارزښت_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    أرجع;
  حالة ارزښت::شمېره:
    ارزښت_Begin();
    إذا (V.Type == ارزښت::T_Integer)
      OS << *V.getAsInteger();
    وإلا إذا (V.Type == ارزښت::T_UINT64)
      OS << *V.getAsUINT64();
    وإلا
      OS << format("%.*g", std::numeric_limits<مزدوج>::max_digits10,
                   *V.get_As_شمېره());
    أرجع;
  حالة ارزښت::String:
    ارزښت_Begin();
    quote(OS, *V.getAsString());
    أرجع;
  حالة ارزښت::Array:
    أرجع array([&] {
      لكل (ثابت ارزښت &E : *V.getAsArray())
        ارزښت_2(E);
    });
  حالة ارزښت::Object:
    أرجع object([&] {
      لكل (ثابت Object::ارزښت_type *E : sortedElements(*V.getAsObject()))
        attribute(E->لومړی_2, E->دوهم_2);
    });
  }
}

فراغ llvm::json::OStream::ارزښت_Begin() {
  أكد(ډېری.شا().Ctx != Object && "Only attributes allowed here");
  إذا (ډېری.شا().Has_ارزښت) {
    أكد(ډېری.شا().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  إذا (ډېری.شا().Ctx == Array)
    newline();
  flushComment();
  ډېری.شا().Has_ارزښت = ریښتیا;
}

فراغ OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.تشه() && "Only one comment per value!");
  PendingComment = Comment;
}

فراغ OStream::flushComment() {
  إذا (PendingComment.تشه())
    أرجع;
  OS << (Indent_اندازه ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  طالما (!PendingComment.تشه()) {
    خپلکاری Pos = PendingComment.پیدا_کړه("*/");
    إذا (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } وإلا {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_اندازه ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  إذا (ډېری.اندازه_2() > 1 && ډېری.شا().Ctx == Singleton) {
    إذا (Indent_اندازه)
      OS << ' ';
  } وإلا {
    newline();
  }
}

فراغ llvm::json::OStream::newline() {
  إذا (Indent_اندازه) {
    OS.اكتب('\n');
    OS.indent(Indent);
  }
}

فراغ llvm::json::OStream::arrayBegin() {
  ارزښت_Begin();
  ډېری.emplace_شا();
  ډېری.شا().Ctx = Array;
  Indent += Indent_اندازه;
  OS << '[';
}

فراغ llvm::json::OStream::array_پای_ټکی() {
  أكد(ډېری.شا().Ctx == Array);
  Indent -= Indent_اندازه;
  إذا (ډېری.شا().Has_ارزښت)
    newline();
  OS << ']';
  أكد(PendingComment.تشه());
  ډېری.راوباسه_شا();
  أكد(!ډېری.تشه());
}

فراغ llvm::json::OStream::objectBegin() {
  ارزښت_Begin();
  ډېری.emplace_شا();
  ډېری.شا().Ctx = Object;
  Indent += Indent_اندازه;
  OS << '{';
}

فراغ llvm::json::OStream::object_پای_ټکی() {
  أكد(ډېری.شا().Ctx == Object);
  Indent -= Indent_اندازه;
  إذا (ډېری.شا().Has_ارزښت)
    newline();
  OS << '}';
  أكد(PendingComment.تشه());
  ډېری.راوباسه_شا();
  أكد(!ډېری.تشه());
}

فراغ llvm::json::OStream::attributeBegin(llvm::StringRef کلی) {
  أكد(ډېری.شا().Ctx == Object);
  إذا (ډېری.شا().Has_ارزښت)
    OS << ',';
  newline();
  flushComment();
  ډېری.شا().Has_ارزښت = ریښتیا;
  ډېری.emplace_شا();
  ډېری.شا().Ctx = Singleton;
  إذا (LLVM_LIKELY(isUTF8(کلی))) {
    quote(OS, کلی);
  } وإلا {
    أكد(خطأ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(کلی));
  }
  OS.اكتب(':');
  إذا (Indent_اندازه)
    OS.اكتب(' ');
}

فراغ llvm::json::OStream::attribute_پای_ټکی() {
  أكد(ډېری.شا().Ctx == Singleton);
  أكد(ډېری.شا().Has_ارزښت && "Attribute must have a value");
  أكد(PendingComment.تشه());
  ډېری.راوباسه_شا();
  أكد(ډېری.شا().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_ارزښت_Begin() {
  ارزښت_Begin();
  ډېری.emplace_شا();
  ډېری.شا().Ctx = Raw_ارزښت;
  أرجع OS;
}

فراغ llvm::json::OStream::raw_ارزښت_پای_ټکی() {
  أكد(ډېری.شا().Ctx == Raw_ارزښت);
  ډېری.راوباسه_شا();
}

} // namespace json
} // namespace llvm

فراغ llvm::format_provider<llvm::json::ارزښت>::format(
    ثابت llvm::json::ارزښت &E, raw_ostream &OS, StringRef Options) {
  بینښه IndentAmount = 0;
  إذا (!Options.تشه() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).ارزښت_2(E);
}

