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

مقدار &Object::عملگر[](ثابت Object_کلید &K) {
  أرجع try_emplace(K, اشاره_تهی).اولی_2->get_دومی();
}
مقدار &Object::عملگر[](Object_کلید &&K) {
  أرجع try_emplace(std::move(K), اشاره_تهی).اولی_2->get_دومی();
}
مقدار *Object::get(StringRef K) {
  خودکار I = پیدا_کن(K);
  إذا (I == انتها())
    أرجع اشاره_تهی;
  أرجع &I->دومی_2;
}
ثابت مقدار *Object::get(StringRef K) ثابت {
  خودکار I = پیدا_کن(K);
  إذا (I == انتها())
    أرجع اشاره_تهی;
  أرجع &I->دومی_2;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->getAsNull();
  أرجع std::nullopt;
}
std::optional<منطقي> Object::getBoolean(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->getAsBoolean();
  أرجع std::nullopt;
}
std::optional<مزدوج> Object::get_عدد(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->get_As_عدد();
  أرجع std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->getAsInteger();
  أرجع std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->getAsString();
  أرجع std::nullopt;
}
ثابت json::Object *Object::getObject(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->getAsObject();
  أرجع اشاره_تهی;
}
json::Object *Object::getObject(StringRef K) {
  إذا (خودکار *V = get(K))
    أرجع V->getAsObject();
  أرجع اشاره_تهی;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->getAsArray();
  أرجع اشاره_تهی;
}
json::Array *Object::getArray(StringRef K) {
  إذا (خودکار *V = get(K))
    أرجع V->getAsArray();
  أرجع اشاره_تهی;
}
منطقي عملگر==(ثابت Object &LHS, ثابت Object &RHS) { أرجع LHS.M == RHS.M; }

Array::Array(std::initializer_list<مقدار> Elements) {
  V.reserve(Elements.اندازه_2());
  لكل (ثابت مقدار &V : Elements) {
    emplace_پشت(اشاره_تهی);
    پشت().moveFrom(std::move(V));
  }
}

مقدار::مقدار(std::initializer_list<مقدار> Elements)
    : مقدار(json::Array(Elements)) {}

فراغ مقدار::copyFrom(ثابت مقدار &M) {
  Type = M.Type;
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    کپی_حافظه(&Union, &M.Union, حجم(Union));
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

فراغ مقدار::moveFrom(ثابت مقدار &&M) {
  Type = M.Type;
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    کپی_حافظه(&Union, &M.Union, حجم(Union));
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
  const_cast<مقدار &>(M).destroy();
  M.Type = T_Null;
}

فراغ مقدار::destroy() {
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

فراغ مقدار::print(llvm::raw_ostream &OS) ثابت { OS << *هذا; }

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_روش فراغ مقدار::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#پایان_اگر

منطقي عملگر==(ثابت مقدار &L, ثابت مقدار &R) {
  إذا (L.نوع_2() != R.نوع_2())
    أرجع خطأ;
  بدل (L.نوع_2()) {
  حالة مقدار::Null:
    أرجع *L.getAsNull() == *R.getAsNull();
  حالة مقدار::Boolean:
    أرجع *L.getAsBoolean() == *R.getAsBoolean();
  حالة مقدار::عدد:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    إذا (L.Type == مقدار::T_Integer || R.Type == مقدار::T_Integer)
      أرجع L.getAsInteger() == R.getAsInteger();
    أرجع *L.get_As_عدد() == *R.get_As_عدد();
  حالة مقدار::String:
    أرجع *L.getAsString() == *R.getAsString();
  حالة مقدار::Array:
    أرجع *L.getAsArray() == *R.getAsArray();
  حالة مقدار::Object:
    أرجع *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

فراغ مسیر::report(llvm::StringLiteral پیام_4) {
  // Walk up to the root context, and count the number of segments.
  بیعلامت شمار_2 = 0;
  ثابت مسیر *P;
  لكل (P = هذا; P->والد_2 != اشاره_تهی; P = P->والد_2)
    ++شمار_2;
  مسیر::ریشه_2 *R = P->Seg.ریشه_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_پیام_2 = پیام_4;
  R->Error_مسیر.resize(شمار_2);
  خودکار It = R->Error_مسیر.begin();
  لكل (P = هذا; P->والد_2 != اشاره_تهی; P = P->والد_2)
    *It++ = P->Seg;
}

Error مسیر::ریشه_2::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_پیام_2.خالی() ? "invalid JSON contents" : Error_پیام_2);
  إذا (Error_مسیر.خالی()) {
    إذا (!Name.خالی())
      OS << " when parsing " << Name;
  } وإلا {
    OS << " at " << (Name.خالی() ? "(root)" : Name);
    لكل (ثابت مسیر::Segment &S : llvm::معکوس(Error_مسیر)) {
      إذا (S.is_فیلد())
        OS << '.' << S.فیلد();
      وإلا
        OS << '[' << S.شاخص() << ']';
    }
  }
  أرجع createStringError(llvm::inconvertible_Error_کد(), S);
}

std::وکتور<ثابت Object::مقدار_type *> sortedElements(ثابت Object &O) {
  std::وکتور<ثابت Object::مقدار_type *> Elements;
  لكل (ثابت خودکار &E : O)
    Elements.افزودن_پشت(&E);
  llvm::مرتب_سازی(Elements,
             [](ثابت Object::مقدار_type *L, ثابت Object::مقدار_type *R) {
               أرجع L->اولی_2 < R->اولی_2;
             });
  أرجع Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ساكن فراغ abbreviate(ثابت مقدار &V, OStream &JOS) {
  بدل (V.نوع_2()) {
  حالة مقدار::Array:
    JOS.raw_مقدار(V.getAsArray()->خالی() ? "[]" : "[ ... ]");
    اكسر;
  حالة مقدار::Object:
    JOS.raw_مقدار(V.getAsObject()->خالی() ? "{}" : "{ ... }");
    اكسر;
  حالة مقدار::String: {
    llvm::StringRef S = *V.getAsString();
    إذا (S.اندازه_2() < 40) {
      JOS.مقدار_2(V);
    } وإلا {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.مقدار_2(Truncated);
    }
    اكسر;
  }
  افتراضي:
    JOS.مقدار_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ساكن فراغ abbreviateChildren(ثابت مقدار &V, OStream &JOS) {
  بدل (V.نوع_2()) {
  حالة مقدار::Array:
    JOS.array([&] {
      لكل (ثابت خودکار &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    اكسر;
  حالة مقدار::Object:
    JOS.object([&] {
      لكل (ثابت خودکار *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->اولی_2);
        abbreviate(KV->دومی_2, JOS);
        JOS.attribute_انتها();
      }
    });
    اكسر;
  افتراضي:
    JOS.مقدار_2(V);
  }
}

فراغ مسیر::ریشه_2::printErrorContext(ثابت مقدار &R, raw_ostream &OS) ثابت {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  خودکار Print_مقدار = [&](ثابت مقدار &V, ArrayRef<Segment> مسیر, خودکار &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    خودکار HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_پیام_2.داده(), Error_پیام_2.اندازه_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    إذا (مسیر.خالی()) // We reached our target.
      أرجع HighlightCurrent();
    ثابت Segment &S = مسیر.پشت(); // Path is in reverse order.
    إذا (S.is_فیلد()) {
      // Current node is an object, path names a field.
      llvm::StringRef فیلد_Name = S.فیلد();
      ثابت Object *O = V.getAsObject();
      إذا (!O || !O->get(فیلد_Name))
        أرجع HighlightCurrent();
      JOS.object([&] {
        لكل (ثابت خودکار *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->اولی_2);
          إذا (فیلد_Name == StringRef(KV->اولی_2))
            Recurse(KV->دومی_2, مسیر.drop_پشت(), Recurse);
          وإلا
            abbreviate(KV->دومی_2, JOS);
          JOS.attribute_انتها();
        }
      });
    } وإلا {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      إذا (!A || S.شاخص() >= A->اندازه_2())
        أرجع HighlightCurrent();
      JOS.array([&] {
        بیعلامت Current = 0;
        لكل (ثابت خودکار &V : *A) {
          إذا (Current++ == S.شاخص())
            Recurse(V, مسیر.drop_پشت(), Recurse);
          وإلا
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_مقدار(R, Error_مسیر, Print_مقدار);
}

نطاق {
// Simple recursive-descent JSON parser.
صنف Parser {
عام:
  Parser(StringRef JSON)
      : شروع(JSON.begin()), P(JSON.begin()), انتها_2(JSON.انتها()) {}

  منطقي checkUTF8() {
    size_t Err_جابه_جایی;
    إذا (isUTF8(StringRef(شروع, انتها_2 - شروع), &Err_جابه_جایی))
      أرجع درست;
    P = شروع + Err_جابه_جایی; // For line/column calculation.
    أرجع parseError("Invalid UTF-8 sequence");
  }

  منطقي parse_مقدار(مقدار &بیرون);

  منطقي assert_انتها() {
    eatWhitespace();
    إذا (P == انتها_2)
      أرجع درست;
    أرجع parseError("Text after end of document");
  }

  Error takeError() {
    أكد(Err);
    أرجع std::move(*Err);
  }

خاص:
  فراغ eatWhitespace() {
    طالما (P != انتها_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  منطقي parse_عدد(حرف اولی, مقدار &بیرون);
  منطقي parseString(std::string &بیرون);
  منطقي parseUnicode(std::string &بیرون);
  منطقي parseError(ثابت حرف *پیام_4); // always returns false

  حرف بعدی() { أرجع P == انتها_2 ? 0 : *P++; }
  حرف peek() { أرجع P == انتها_2 ? 0 : *P; }
  ساكن منطقي is_عدد(حرف C) {
    أرجع C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت حرف *شروع, *P, *انتها_2;
};
} // namespace

منطقي Parser::parse_مقدار(مقدار &بیرون) {
  eatWhitespace();
  إذا (P == انتها_2)
    أرجع parseError("Unexpected EOF");
  بدل (حرف C = بعدی()) {
  // Bare null/true/false are easy - first char identifies them.
  حالة 'n':
    بیرون = اشاره_تهی;
    أرجع (بعدی() == 'u' && بعدی() == 'l' && بعدی() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالة 't':
    بیرون = درست;
    أرجع (بعدی() == 'r' && بعدی() == 'u' && بعدی() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالة 'f':
    بیرون = خطأ;
    أرجع (بعدی() == 'a' && بعدی() == 'l' && بعدی() == 's' && بعدی() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالة '"': {
    std::string S;
    إذا (parseString(S)) {
      بیرون = std::move(S);
      أرجع درست;
    }
    أرجع خطأ;
  }
  حالة '[': {
    بیرون = Array{};
    Array &A = *بیرون.getAsArray();
    eatWhitespace();
    إذا (peek() == ']') {
      ++P;
      أرجع درست;
    }
    لكل (;;) {
      A.emplace_پشت(اشاره_تهی);
      إذا (!parse_مقدار(A.پشت()))
        أرجع خطأ;
      eatWhitespace();
      بدل (بعدی()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة ']':
        أرجع درست;
      افتراضي:
        أرجع parseError("Expected , or ] after array element");
      }
    }
  }
  حالة '{': {
    بیرون = Object{};
    Object &O = *بیرون.getAsObject();
    eatWhitespace();
    إذا (peek() == '}') {
      ++P;
      أرجع درست;
    }
    لكل (;;) {
      إذا (بعدی() != '"')
        أرجع parseError("Expected object key");
      std::string K;
      إذا (!parseString(K))
        أرجع خطأ;
      eatWhitespace();
      إذا (بعدی() != ':')
        أرجع parseError("Expected : after object key");
      eatWhitespace();
      إذا (!parse_مقدار(O[std::move(K)]))
        أرجع خطأ;
      eatWhitespace();
      بدل (بعدی()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة '}':
        أرجع درست;
      افتراضي:
        أرجع parseError("Expected , or } after object property");
      }
    }
  }
  افتراضي:
    إذا (is_عدد(C))
      أرجع parse_عدد(C, بیرون);
    أرجع parseError("Invalid JSON value");
  }
}

منطقي Parser::parse_عدد(حرف اولی, مقدار &بیرون) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.افزودن_پشت(اولی);
  طالما (is_عدد(peek()))
    S.افزودن_پشت(بعدی());
  حرف *انتها_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &انتها_2, 10);
  إذا (انتها_2 == S.انتها() && errno != ERANGE) {
    بیرون = I;
    أرجع درست;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  إذا (اولی != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &انتها_2, 10);
    إذا (انتها_2 == S.انتها() && errno != ERANGE) {
      بیرون = UI;
      أرجع درست;
    }
  }
  // If it's not an integer
  بیرون = std::strtod(S.c_str(), &انتها_2);
  أرجع انتها_2 == S.انتها() || parseError("Invalid JSON value (number?)");
}

منطقي Parser::parseString(std::string &بیرون) {
  // leading quote was already consumed.
  لكل (حرف C = بعدی(); C != '"'; C = بعدی()) {
    إذا (LLVM_UNLIKELY(P == انتها_2))
      أرجع parseError("Unterminated string");
    إذا (LLVM_UNLIKELY((C & 0x1f) == C))
      أرجع parseError("Control character in string");
    إذا (LLVM_LIKELY(C != '\\')) {
      بیرون.افزودن_پشت(C);
      استمر;
    }
    // Handle escape sequence.
    بدل (C = بعدی()) {
    حالة '"':
    حالة '\\':
    حالة '/':
      بیرون.افزودن_پشت(C);
      اكسر;
    حالة 'b':
      بیرون.افزودن_پشت('\b');
      اكسر;
    حالة 'f':
      بیرون.افزودن_پشت('\f');
      اكسر;
    حالة 'n':
      بیرون.افزودن_پشت('\n');
      اكسر;
    حالة 'r':
      بیرون.افزودن_پشت('\r');
      اكسر;
    حالة 't':
      بیرون.افزودن_پشت('\t');
      اكسر;
    حالة 'u':
      إذا (!parseUnicode(بیرون))
        أرجع خطأ;
      اكسر;
    افتراضي:
      أرجع parseError("Invalid escape sequence");
    }
  }
  أرجع درست;
}

ساكن فراغ encodeUtf8(uint32_t Rune, std::string &بیرون) {
  إذا (Rune < 0x80) {
    بیرون.افزودن_پشت(Rune & 0x7F);
  } وإلا إذا (Rune < 0x800) {
    uint8_t اولی_Byte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t دومی_Byte = 0x80 | (Rune & 0x3F);
    بیرون.افزودن_پشت(اولی_Byte);
    بیرون.افزودن_پشت(دومی_Byte);
  } وإلا إذا (Rune < 0x10000) {
    uint8_t اولی_Byte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t دومی_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t سوم_Byte = 0x80 | (Rune & 0x3F);
    بیرون.افزودن_پشت(اولی_Byte);
    بیرون.افزودن_پشت(دومی_Byte);
    بیرون.افزودن_پشت(سوم_Byte);
  } وإلا إذا (Rune < 0x110000) {
    uint8_t اولی_Byte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t دومی_Byte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t سوم_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    بیرون.افزودن_پشت(اولی_Byte);
    بیرون.افزودن_پشت(دومی_Byte);
    بیرون.افزودن_پشت(سوم_Byte);
    بیرون.افزودن_پشت(FourthByte);
  } وإلا {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
منطقي Parser::parseUnicode(std::string &بیرون) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  خودکار Invalid = [&] { بیرون.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  خودکار Parse4Hex = [هذا](uint16_t &بیرون) -> منطقي {
    بیرون = 0;
    حرف بایتها_2[] = {بعدی(), بعدی(), بعدی(), بعدی()};
    لكل (بیعلامت حرف C : بایتها_2) {
      إذا (!std::isxdigit(C))
        أرجع parseError("Invalid \\u escape sequence");
      بیرون <<= 4;
      بیرون |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    أرجع درست;
  };
  uint16_t اولی; // UTF-16 code unit from the first \u escape.
  إذا (!Parse4Hex(اولی))
    أرجع خطأ;

  // We loop to allow proper surrogate-pair error handling.
  طالما (درست) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    إذا (LLVM_LIKELY(اولی < 0xD800 || اولی >= 0xE000)) {
      encodeUtf8(اولی, بیرون);
      أرجع درست;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    إذا (LLVM_UNLIKELY(اولی >= 0xDC00)) {
      Invalid();
      أرجع درست;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    إذا (LLVM_UNLIKELY(P + 2 > انتها_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      أرجع درست;
    }
    P += 2;
    uint16_t دومی;
    إذا (!Parse4Hex(دومی))
      أرجع خطأ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    إذا (LLVM_UNLIKELY(دومی < 0xDC00 || دومی >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      اولی = دومی; // Second escape still needs to be processed.
      استمر;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((اولی - 0xD800) << 10) | (دومی - 0xDC00), بیرون);
    أرجع درست;
  }
}

منطقي Parser::parseError(ثابت حرف *پیام_4) {
  صحيح سطر = 1;
  ثابت حرف *شروع_از_سطر = شروع;
  لكل (ثابت حرف *X = شروع; X < P; ++X) {
    إذا (*X == 0x0A) {
      ++سطر;
      شروع_از_سطر = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(پیام_4, سطر, P - شروع_از_سطر, P - شروع));
  أرجع خطأ;
}

Expected<مقدار> parse(StringRef JSON) {
  Parser P(JSON);
  مقدار E = اشاره_تهی;
  إذا (P.checkUTF8())
    إذا (P.parse_مقدار(E))
      إذا (P.assert_انتها())
        أرجع std::move(E);
  أرجع P.takeError();
}

حرف ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
فراغ ParseError::لگاریتم(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", سطر, ستونی, جابه_جایی, پیام_4);
}

منطقي isUTF8(llvm::StringRef S, size_t *Err_جابه_جایی) {
  // Fast-path for ASCII, which is valid UTF-8.
  إذا (LLVM_LIKELY(isASCII(S)))
    أرجع درست;

  ثابت UTF8 *داده_2 = reinterpret_cast<ثابت UTF8 *>(S.داده()), *Rest = داده_2;
  إذا (LLVM_LIKELY(isLegalUTF8String(&Rest, داده_2 + S.اندازه_2())))
    أرجع درست;

  إذا (Err_جابه_جایی)
    *Err_جابه_جایی = Rest - داده_2;
  أرجع خطأ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::وکتور<UTF32> Codepoints(S.اندازه_2()); // 1 codepoint per byte suffices.
  ثابت UTF8 *In8 = reinterpret_cast<ثابت UTF8 *>(S.داده());
  UTF32 *Out32 = Codepoints.داده();
  ConvertUTF8toUTF32(&In8, In8 + S.اندازه_2(), &Out32, Out32 + Codepoints.اندازه_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.داده());
  std::string Res(4 * Codepoints.اندازه_2(), 0); // 4 bytes per codepoint suffice
  ثابت UTF32 *In32 = Codepoints.داده();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.اندازه_2(), &Out8, Out8 + Res.اندازه_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<حرف *>(Out8) - Res.داده());
  أرجع Res;
}

ساكن فراغ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  لكل (بیعلامت حرف C : S) {
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
      llvm::write_hex(OS, C, llvm::HexPrintStyle::پایینی_2, 4);
      اكسر;
    }
  }
  OS << '\"';
}

فراغ llvm::json::OStream::مقدار_2(ثابت مقدار &V) {
  بدل (V.نوع_2()) {
  حالة مقدار::Null:
    مقدار_Begin();
    OS << "null";
    أرجع;
  حالة مقدار::Boolean:
    مقدار_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    أرجع;
  حالة مقدار::عدد:
    مقدار_Begin();
    إذا (V.Type == مقدار::T_Integer)
      OS << *V.getAsInteger();
    وإلا إذا (V.Type == مقدار::T_UINT64)
      OS << *V.getAsUINT64();
    وإلا
      OS << format("%.*g", std::numeric_limits<مزدوج>::max_digits10,
                   *V.get_As_عدد());
    أرجع;
  حالة مقدار::String:
    مقدار_Begin();
    quote(OS, *V.getAsString());
    أرجع;
  حالة مقدار::Array:
    أرجع array([&] {
      لكل (ثابت مقدار &E : *V.getAsArray())
        مقدار_2(E);
    });
  حالة مقدار::Object:
    أرجع object([&] {
      لكل (ثابت Object::مقدار_type *E : sortedElements(*V.getAsObject()))
        attribute(E->اولی_2, E->دومی_2);
    });
  }
}

فراغ llvm::json::OStream::مقدار_Begin() {
  أكد(پشته.پشت().Ctx != Object && "Only attributes allowed here");
  إذا (پشته.پشت().Has_مقدار) {
    أكد(پشته.پشت().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  إذا (پشته.پشت().Ctx == Array)
    newline();
  flushComment();
  پشته.پشت().Has_مقدار = درست;
}

فراغ OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.خالی() && "Only one comment per value!");
  PendingComment = Comment;
}

فراغ OStream::flushComment() {
  إذا (PendingComment.خالی())
    أرجع;
  OS << (Indent_اندازه ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  طالما (!PendingComment.خالی()) {
    خودکار Pos = PendingComment.پیدا_کن("*/");
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
  إذا (پشته.اندازه_2() > 1 && پشته.پشت().Ctx == Singleton) {
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
  مقدار_Begin();
  پشته.emplace_پشت();
  پشته.پشت().Ctx = Array;
  Indent += Indent_اندازه;
  OS << '[';
}

فراغ llvm::json::OStream::array_انتها() {
  أكد(پشته.پشت().Ctx == Array);
  Indent -= Indent_اندازه;
  إذا (پشته.پشت().Has_مقدار)
    newline();
  OS << ']';
  أكد(PendingComment.خالی());
  پشته.برداشت_پشت();
  أكد(!پشته.خالی());
}

فراغ llvm::json::OStream::objectBegin() {
  مقدار_Begin();
  پشته.emplace_پشت();
  پشته.پشت().Ctx = Object;
  Indent += Indent_اندازه;
  OS << '{';
}

فراغ llvm::json::OStream::object_انتها() {
  أكد(پشته.پشت().Ctx == Object);
  Indent -= Indent_اندازه;
  إذا (پشته.پشت().Has_مقدار)
    newline();
  OS << '}';
  أكد(PendingComment.خالی());
  پشته.برداشت_پشت();
  أكد(!پشته.خالی());
}

فراغ llvm::json::OStream::attributeBegin(llvm::StringRef کلید) {
  أكد(پشته.پشت().Ctx == Object);
  إذا (پشته.پشت().Has_مقدار)
    OS << ',';
  newline();
  flushComment();
  پشته.پشت().Has_مقدار = درست;
  پشته.emplace_پشت();
  پشته.پشت().Ctx = Singleton;
  إذا (LLVM_LIKELY(isUTF8(کلید))) {
    quote(OS, کلید);
  } وإلا {
    أكد(خطأ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(کلید));
  }
  OS.اكتب(':');
  إذا (Indent_اندازه)
    OS.اكتب(' ');
}

فراغ llvm::json::OStream::attribute_انتها() {
  أكد(پشته.پشت().Ctx == Singleton);
  أكد(پشته.پشت().Has_مقدار && "Attribute must have a value");
  أكد(PendingComment.خالی());
  پشته.برداشت_پشت();
  أكد(پشته.پشت().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_مقدار_Begin() {
  مقدار_Begin();
  پشته.emplace_پشت();
  پشته.پشت().Ctx = Raw_مقدار;
  أرجع OS;
}

فراغ llvm::json::OStream::raw_مقدار_انتها() {
  أكد(پشته.پشت().Ctx == Raw_مقدار);
  پشته.برداشت_پشت();
}

} // namespace json
} // namespace llvm

فراغ llvm::format_provider<llvm::json::مقدار>::format(
    ثابت llvm::json::مقدار &E, raw_ostream &OS, StringRef Options) {
  بیعلامت IndentAmount = 0;
  إذا (!Options.خالی() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).مقدار_2(E);
}

