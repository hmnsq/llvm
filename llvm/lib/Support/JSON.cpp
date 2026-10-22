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

قدر &Object::عامل[](ثابت Object_کلید &K) {
  أرجع try_emplace(K, خالی_سوچک).پہلا_2->get_دوجا();
}
قدر &Object::عامل[](Object_کلید &&K) {
  أرجع try_emplace(std::move(K), خالی_سوچک).پہلا_2->get_دوجا();
}
قدر *Object::get(StringRef K) {
  خودکار I = لبھو(K);
  إذا (I == سرا())
    أرجع خالی_سوچک;
  أرجع &I->دوجا_2;
}
ثابت قدر *Object::get(StringRef K) ثابت {
  خودکار I = لبھو(K);
  إذا (I == سرا())
    أرجع خالی_سوچک;
  أرجع &I->دوجا_2;
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
std::optional<مزدوج> Object::get_نمبر(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->get_As_نمبر();
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
  أرجع خالی_سوچک;
}
json::Object *Object::getObject(StringRef K) {
  إذا (خودکار *V = get(K))
    أرجع V->getAsObject();
  أرجع خالی_سوچک;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  إذا (خودکار *V = get(K))
    أرجع V->getAsArray();
  أرجع خالی_سوچک;
}
json::Array *Object::getArray(StringRef K) {
  إذا (خودکار *V = get(K))
    أرجع V->getAsArray();
  أرجع خالی_سوچک;
}
منطقي عامل==(ثابت Object &LHS, ثابت Object &RHS) { أرجع LHS.M == RHS.M; }

Array::Array(std::initializer_list<قدر> Elements) {
  V.reserve(Elements.ناپ_2());
  لكل (ثابت قدر &V : Elements) {
    emplace_پٹھ(خالی_سوچک);
    پٹھ().moveFrom(std::move(V));
  }
}

قدر::قدر(std::initializer_list<قدر> Elements)
    : قدر(json::Array(Elements)) {}

فراغ قدر::copyFrom(ثابت قدر &M) {
  Type = M.Type;
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    حافظہ_نقل(&Union, &M.Union, حجم(Union));
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

فراغ قدر::moveFrom(ثابت قدر &&M) {
  Type = M.Type;
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    حافظہ_نقل(&Union, &M.Union, حجم(Union));
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
  const_cast<قدر &>(M).destroy();
  M.Type = T_Null;
}

فراغ قدر::destroy() {
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

فراغ قدر::print(llvm::raw_ostream &OS) ثابت { OS << *هذا; }

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ فراغ قدر::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#جے_انت

منطقي عامل==(ثابت قدر &L, ثابت قدر &R) {
  إذا (L.قسم() != R.قسم())
    أرجع خطأ;
  بدل (L.قسم()) {
  حالة قدر::Null:
    أرجع *L.getAsNull() == *R.getAsNull();
  حالة قدر::Boolean:
    أرجع *L.getAsBoolean() == *R.getAsBoolean();
  حالة قدر::نمبر:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    إذا (L.Type == قدر::T_Integer || R.Type == قدر::T_Integer)
      أرجع L.getAsInteger() == R.getAsInteger();
    أرجع *L.get_As_نمبر() == *R.get_As_نمبر();
  حالة قدر::String:
    أرجع *L.getAsString() == *R.getAsString();
  حالة قدر::Array:
    أرجع *L.getAsArray() == *R.getAsArray();
  حالة قدر::Object:
    أرجع *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

فراغ رستہ::report(llvm::StringLiteral پیغام_2) {
  // Walk up to the root context, and count the number of segments.
  بنانشانی گنتی_2 = 0;
  ثابت رستہ *P;
  لكل (P = هذا; P->ماپا_2 != خالی_سوچک; P = P->ماپا_2)
    ++گنتی_2;
  رستہ::جڑ_2 *R = P->Seg.جڑ_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_سنیہا = پیغام_2;
  R->Error_رستہ.resize(گنتی_2);
  خودکار It = R->Error_رستہ.begin();
  لكل (P = هذا; P->ماپا_2 != خالی_سوچک; P = P->ماپا_2)
    *It++ = P->Seg;
}

Error رستہ::جڑ_2::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_سنیہا.خالی_تھاواں() ? "invalid JSON contents" : Error_سنیہا);
  إذا (Error_رستہ.خالی_تھاواں()) {
    إذا (!Name.خالی_تھاواں())
      OS << " when parsing " << Name;
  } وإلا {
    OS << " at " << (Name.خالی_تھاواں() ? "(root)" : Name);
    لكل (ثابت رستہ::Segment &S : llvm::الٹ(Error_رستہ)) {
      إذا (S.is_میدان())
        OS << '.' << S.میدان();
      وإلا
        OS << '[' << S.اشاریہ() << ']';
    }
  }
  أرجع createStringError(llvm::inconvertible_Error_کوڈ(), S);
}

std::سمتیہ<ثابت Object::قدر_type *> sortedElements(ثابت Object &O) {
  std::سمتیہ<ثابت Object::قدر_type *> Elements;
  لكل (ثابت خودکار &E : O)
    Elements.ڈالو_پٹھ(&E);
  llvm::ترتیب_دیو(Elements,
             [](ثابت Object::قدر_type *L, ثابت Object::قدر_type *R) {
               أرجع L->پہلا_2 < R->پہلا_2;
             });
  أرجع Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ساكن فراغ abbreviate(ثابت قدر &V, OStream &JOS) {
  بدل (V.قسم()) {
  حالة قدر::Array:
    JOS.raw_قدر(V.getAsArray()->خالی_تھاواں() ? "[]" : "[ ... ]");
    اكسر;
  حالة قدر::Object:
    JOS.raw_قدر(V.getAsObject()->خالی_تھاواں() ? "{}" : "{ ... }");
    اكسر;
  حالة قدر::String: {
    llvm::StringRef S = *V.getAsString();
    إذا (S.ناپ_2() < 40) {
      JOS.قدر_2(V);
    } وإلا {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.قدر_2(Truncated);
    }
    اكسر;
  }
  افتراضي:
    JOS.قدر_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ساكن فراغ abbreviateChildren(ثابت قدر &V, OStream &JOS) {
  بدل (V.قسم()) {
  حالة قدر::Array:
    JOS.array([&] {
      لكل (ثابت خودکار &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    اكسر;
  حالة قدر::Object:
    JOS.object([&] {
      لكل (ثابت خودکار *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->پہلا_2);
        abbreviate(KV->دوجا_2, JOS);
        JOS.attribute_سرا();
      }
    });
    اكسر;
  افتراضي:
    JOS.قدر_2(V);
  }
}

فراغ رستہ::جڑ_2::printErrorContext(ثابت قدر &R, raw_ostream &OS) ثابت {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  خودکار Print_قدر = [&](ثابت قدر &V, ArrayRef<Segment> رستہ, خودکار &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    خودکار HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_سنیہا.انکڑے(), Error_سنیہا.ناپ_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    إذا (رستہ.خالی_تھاواں()) // We reached our target.
      أرجع HighlightCurrent();
    ثابت Segment &S = رستہ.پٹھ(); // Path is in reverse order.
    إذا (S.is_میدان()) {
      // Current node is an object, path names a field.
      llvm::StringRef میدان_Name = S.میدان();
      ثابت Object *O = V.getAsObject();
      إذا (!O || !O->get(میدان_Name))
        أرجع HighlightCurrent();
      JOS.object([&] {
        لكل (ثابت خودکار *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->پہلا_2);
          إذا (میدان_Name == StringRef(KV->پہلا_2))
            Recurse(KV->دوجا_2, رستہ.drop_پٹھ(), Recurse);
          وإلا
            abbreviate(KV->دوجا_2, JOS);
          JOS.attribute_سرا();
        }
      });
    } وإلا {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      إذا (!A || S.اشاریہ() >= A->ناپ_2())
        أرجع HighlightCurrent();
      JOS.array([&] {
        بنانشانی Current = 0;
        لكل (ثابت خودکار &V : *A) {
          إذا (Current++ == S.اشاریہ())
            Recurse(V, رستہ.drop_پٹھ(), Recurse);
          وإلا
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_قدر(R, Error_رستہ, Print_قدر);
}

نطاق {
// Simple recursive-descent JSON parser.
صنف Parser {
عام:
  Parser(StringRef JSON)
      : شروع(JSON.begin()), P(JSON.begin()), سرا_2(JSON.سرا()) {}

  منطقي checkUTF8() {
    size_t Err_وتھ;
    إذا (isUTF8(StringRef(شروع, سرا_2 - شروع), &Err_وتھ))
      أرجع سچ;
    P = شروع + Err_وتھ; // For line/column calculation.
    أرجع parseError("Invalid UTF-8 sequence");
  }

  منطقي parse_قدر(قدر &باہرلا);

  منطقي assert_سرا() {
    eatWhitespace();
    إذا (P == سرا_2)
      أرجع سچ;
    أرجع parseError("Text after end of document");
  }

  Error takeError() {
    أكد(Err);
    أرجع std::move(*Err);
  }

خاص:
  فراغ eatWhitespace() {
    طالما (P != سرا_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  منطقي parse_نمبر(حرف پہلا, قدر &باہرلا);
  منطقي parseString(std::string &باہرلا);
  منطقي parseUnicode(std::string &باہرلا);
  منطقي parseError(ثابت حرف *پیغام_2); // always returns false

  حرف اگلا() { أرجع P == سرا_2 ? 0 : *P++; }
  حرف peek() { أرجع P == سرا_2 ? 0 : *P; }
  ساكن منطقي is_نمبر(حرف C) {
    أرجع C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت حرف *شروع, *P, *سرا_2;
};
} // namespace

منطقي Parser::parse_قدر(قدر &باہرلا) {
  eatWhitespace();
  إذا (P == سرا_2)
    أرجع parseError("Unexpected EOF");
  بدل (حرف C = اگلا()) {
  // Bare null/true/false are easy - first char identifies them.
  حالة 'n':
    باہرلا = خالی_سوچک;
    أرجع (اگلا() == 'u' && اگلا() == 'l' && اگلا() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالة 't':
    باہرلا = سچ;
    أرجع (اگلا() == 'r' && اگلا() == 'u' && اگلا() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالة 'f':
    باہرلا = خطأ;
    أرجع (اگلا() == 'a' && اگلا() == 'l' && اگلا() == 's' && اگلا() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالة '"': {
    std::string S;
    إذا (parseString(S)) {
      باہرلا = std::move(S);
      أرجع سچ;
    }
    أرجع خطأ;
  }
  حالة '[': {
    باہرلا = Array{};
    Array &A = *باہرلا.getAsArray();
    eatWhitespace();
    إذا (peek() == ']') {
      ++P;
      أرجع سچ;
    }
    لكل (;;) {
      A.emplace_پٹھ(خالی_سوچک);
      إذا (!parse_قدر(A.پٹھ()))
        أرجع خطأ;
      eatWhitespace();
      بدل (اگلا()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة ']':
        أرجع سچ;
      افتراضي:
        أرجع parseError("Expected , or ] after array element");
      }
    }
  }
  حالة '{': {
    باہرلا = Object{};
    Object &O = *باہرلا.getAsObject();
    eatWhitespace();
    إذا (peek() == '}') {
      ++P;
      أرجع سچ;
    }
    لكل (;;) {
      إذا (اگلا() != '"')
        أرجع parseError("Expected object key");
      std::string K;
      إذا (!parseString(K))
        أرجع خطأ;
      eatWhitespace();
      إذا (اگلا() != ':')
        أرجع parseError("Expected : after object key");
      eatWhitespace();
      إذا (!parse_قدر(O[std::move(K)]))
        أرجع خطأ;
      eatWhitespace();
      بدل (اگلا()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة '}':
        أرجع سچ;
      افتراضي:
        أرجع parseError("Expected , or } after object property");
      }
    }
  }
  افتراضي:
    إذا (is_نمبر(C))
      أرجع parse_نمبر(C, باہرلا);
    أرجع parseError("Invalid JSON value");
  }
}

منطقي Parser::parse_نمبر(حرف پہلا, قدر &باہرلا) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ڈالو_پٹھ(پہلا);
  طالما (is_نمبر(peek()))
    S.ڈالو_پٹھ(اگلا());
  حرف *سرا_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &سرا_2, 10);
  إذا (سرا_2 == S.سرا() && errno != ERANGE) {
    باہرلا = I;
    أرجع سچ;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  إذا (پہلا != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &سرا_2, 10);
    إذا (سرا_2 == S.سرا() && errno != ERANGE) {
      باہرلا = UI;
      أرجع سچ;
    }
  }
  // If it's not an integer
  باہرلا = std::strtod(S.c_str(), &سرا_2);
  أرجع سرا_2 == S.سرا() || parseError("Invalid JSON value (number?)");
}

منطقي Parser::parseString(std::string &باہرلا) {
  // leading quote was already consumed.
  لكل (حرف C = اگلا(); C != '"'; C = اگلا()) {
    إذا (LLVM_UNLIKELY(P == سرا_2))
      أرجع parseError("Unterminated string");
    إذا (LLVM_UNLIKELY((C & 0x1f) == C))
      أرجع parseError("Control character in string");
    إذا (LLVM_LIKELY(C != '\\')) {
      باہرلا.ڈالو_پٹھ(C);
      استمر;
    }
    // Handle escape sequence.
    بدل (C = اگلا()) {
    حالة '"':
    حالة '\\':
    حالة '/':
      باہرلا.ڈالو_پٹھ(C);
      اكسر;
    حالة 'b':
      باہرلا.ڈالو_پٹھ('\b');
      اكسر;
    حالة 'f':
      باہرلا.ڈالو_پٹھ('\f');
      اكسر;
    حالة 'n':
      باہرلا.ڈالو_پٹھ('\n');
      اكسر;
    حالة 'r':
      باہرلا.ڈالو_پٹھ('\r');
      اكسر;
    حالة 't':
      باہرلا.ڈالو_پٹھ('\t');
      اكسر;
    حالة 'u':
      إذا (!parseUnicode(باہرلا))
        أرجع خطأ;
      اكسر;
    افتراضي:
      أرجع parseError("Invalid escape sequence");
    }
  }
  أرجع سچ;
}

ساكن فراغ encodeUtf8(uint32_t Rune, std::string &باہرلا) {
  إذا (Rune < 0x80) {
    باہرلا.ڈالو_پٹھ(Rune & 0x7F);
  } وإلا إذا (Rune < 0x800) {
    uint8_t پہلا_Byte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t دوجا_Byte = 0x80 | (Rune & 0x3F);
    باہرلا.ڈالو_پٹھ(پہلا_Byte);
    باہرلا.ڈالو_پٹھ(دوجا_Byte);
  } وإلا إذا (Rune < 0x10000) {
    uint8_t پہلا_Byte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t دوجا_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t تیجا_Byte = 0x80 | (Rune & 0x3F);
    باہرلا.ڈالو_پٹھ(پہلا_Byte);
    باہرلا.ڈالو_پٹھ(دوجا_Byte);
    باہرلا.ڈالو_پٹھ(تیجا_Byte);
  } وإلا إذا (Rune < 0x110000) {
    uint8_t پہلا_Byte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t دوجا_Byte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t تیجا_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    باہرلا.ڈالو_پٹھ(پہلا_Byte);
    باہرلا.ڈالو_پٹھ(دوجا_Byte);
    باہرلا.ڈالو_پٹھ(تیجا_Byte);
    باہرلا.ڈالو_پٹھ(FourthByte);
  } وإلا {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
منطقي Parser::parseUnicode(std::string &باہرلا) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  خودکار Invalid = [&] { باہرلا.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  خودکار Parse4Hex = [هذا](uint16_t &باہرلا) -> منطقي {
    باہرلا = 0;
    حرف بائٹاں_2[] = {اگلا(), اگلا(), اگلا(), اگلا()};
    لكل (بنانشانی حرف C : بائٹاں_2) {
      إذا (!std::isxdigit(C))
        أرجع parseError("Invalid \\u escape sequence");
      باہرلا <<= 4;
      باہرلا |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    أرجع سچ;
  };
  uint16_t پہلا; // UTF-16 code unit from the first \u escape.
  إذا (!Parse4Hex(پہلا))
    أرجع خطأ;

  // We loop to allow proper surrogate-pair error handling.
  طالما (سچ) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    إذا (LLVM_LIKELY(پہلا < 0xD800 || پہلا >= 0xE000)) {
      encodeUtf8(پہلا, باہرلا);
      أرجع سچ;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    إذا (LLVM_UNLIKELY(پہلا >= 0xDC00)) {
      Invalid();
      أرجع سچ;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    إذا (LLVM_UNLIKELY(P + 2 > سرا_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      أرجع سچ;
    }
    P += 2;
    uint16_t دوجا;
    إذا (!Parse4Hex(دوجا))
      أرجع خطأ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    إذا (LLVM_UNLIKELY(دوجا < 0xDC00 || دوجا >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      پہلا = دوجا; // Second escape still needs to be processed.
      استمر;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((پہلا - 0xD800) << 10) | (دوجا - 0xDC00), باہرلا);
    أرجع سچ;
  }
}

منطقي Parser::parseError(ثابت حرف *پیغام_2) {
  صحيح سطر = 1;
  ثابت حرف *شروع_دا_سطر = شروع;
  لكل (ثابت حرف *X = شروع; X < P; ++X) {
    إذا (*X == 0x0A) {
      ++سطر;
      شروع_دا_سطر = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(پیغام_2, سطر, P - شروع_دا_سطر, P - شروع));
  أرجع خطأ;
}

Expected<قدر> parse(StringRef JSON) {
  Parser P(JSON);
  قدر E = خالی_سوچک;
  إذا (P.checkUTF8())
    إذا (P.parse_قدر(E))
      إذا (P.assert_سرا())
        أرجع std::move(E);
  أرجع P.takeError();
}

حرف ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
فراغ ParseError::لوگارتھم(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", سطر, کالم, وتھ, پیغام_2);
}

منطقي isUTF8(llvm::StringRef S, size_t *Err_وتھ) {
  // Fast-path for ASCII, which is valid UTF-8.
  إذا (LLVM_LIKELY(isASCII(S)))
    أرجع سچ;

  ثابت UTF8 *انکڑے_2 = reinterpret_cast<ثابت UTF8 *>(S.انکڑے()), *Rest = انکڑے_2;
  إذا (LLVM_LIKELY(isLegalUTF8String(&Rest, انکڑے_2 + S.ناپ_2())))
    أرجع سچ;

  إذا (Err_وتھ)
    *Err_وتھ = Rest - انکڑے_2;
  أرجع خطأ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::سمتیہ<UTF32> Codepoints(S.ناپ_2()); // 1 codepoint per byte suffices.
  ثابت UTF8 *In8 = reinterpret_cast<ثابت UTF8 *>(S.انکڑے());
  UTF32 *Out32 = Codepoints.انکڑے();
  ConvertUTF8toUTF32(&In8, In8 + S.ناپ_2(), &Out32, Out32 + Codepoints.ناپ_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.انکڑے());
  std::string Res(4 * Codepoints.ناپ_2(), 0); // 4 bytes per codepoint suffice
  ثابت UTF32 *In32 = Codepoints.انکڑے();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.ناپ_2(), &Out8, Out8 + Res.ناپ_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<حرف *>(Out8) - Res.انکڑے());
  أرجع Res;
}

ساكن فراغ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  لكل (بنانشانی حرف C : S) {
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
      llvm::write_hex(OS, C, llvm::HexPrintStyle::ہیٹھلا_2, 4);
      اكسر;
    }
  }
  OS << '\"';
}

فراغ llvm::json::OStream::قدر_2(ثابت قدر &V) {
  بدل (V.قسم()) {
  حالة قدر::Null:
    قدر_Begin();
    OS << "null";
    أرجع;
  حالة قدر::Boolean:
    قدر_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    أرجع;
  حالة قدر::نمبر:
    قدر_Begin();
    إذا (V.Type == قدر::T_Integer)
      OS << *V.getAsInteger();
    وإلا إذا (V.Type == قدر::T_UINT64)
      OS << *V.getAsUINT64();
    وإلا
      OS << format("%.*g", std::numeric_limits<مزدوج>::max_digits10,
                   *V.get_As_نمبر());
    أرجع;
  حالة قدر::String:
    قدر_Begin();
    quote(OS, *V.getAsString());
    أرجع;
  حالة قدر::Array:
    أرجع array([&] {
      لكل (ثابت قدر &E : *V.getAsArray())
        قدر_2(E);
    });
  حالة قدر::Object:
    أرجع object([&] {
      لكل (ثابت Object::قدر_type *E : sortedElements(*V.getAsObject()))
        attribute(E->پہلا_2, E->دوجا_2);
    });
  }
}

فراغ llvm::json::OStream::قدر_Begin() {
  أكد(ڈھیر.پٹھ().Ctx != Object && "Only attributes allowed here");
  إذا (ڈھیر.پٹھ().Has_قدر) {
    أكد(ڈھیر.پٹھ().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  إذا (ڈھیر.پٹھ().Ctx == Array)
    newline();
  flushComment();
  ڈھیر.پٹھ().Has_قدر = سچ;
}

فراغ OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.خالی_تھاواں() && "Only one comment per value!");
  PendingComment = Comment;
}

فراغ OStream::flushComment() {
  إذا (PendingComment.خالی_تھاواں())
    أرجع;
  OS << (Indent_ناپ ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  طالما (!PendingComment.خالی_تھاواں()) {
    خودکار Pos = PendingComment.لبھو("*/");
    إذا (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } وإلا {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_ناپ ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  إذا (ڈھیر.ناپ_2() > 1 && ڈھیر.پٹھ().Ctx == Singleton) {
    إذا (Indent_ناپ)
      OS << ' ';
  } وإلا {
    newline();
  }
}

فراغ llvm::json::OStream::newline() {
  إذا (Indent_ناپ) {
    OS.اكتب('\n');
    OS.indent(Indent);
  }
}

فراغ llvm::json::OStream::arrayBegin() {
  قدر_Begin();
  ڈھیر.emplace_پٹھ();
  ڈھیر.پٹھ().Ctx = Array;
  Indent += Indent_ناپ;
  OS << '[';
}

فراغ llvm::json::OStream::array_سرا() {
  أكد(ڈھیر.پٹھ().Ctx == Array);
  Indent -= Indent_ناپ;
  إذا (ڈھیر.پٹھ().Has_قدر)
    newline();
  OS << ']';
  أكد(PendingComment.خالی_تھاواں());
  ڈھیر.نکالو_پٹھ();
  أكد(!ڈھیر.خالی_تھاواں());
}

فراغ llvm::json::OStream::objectBegin() {
  قدر_Begin();
  ڈھیر.emplace_پٹھ();
  ڈھیر.پٹھ().Ctx = Object;
  Indent += Indent_ناپ;
  OS << '{';
}

فراغ llvm::json::OStream::object_سرا() {
  أكد(ڈھیر.پٹھ().Ctx == Object);
  Indent -= Indent_ناپ;
  إذا (ڈھیر.پٹھ().Has_قدر)
    newline();
  OS << '}';
  أكد(PendingComment.خالی_تھاواں());
  ڈھیر.نکالو_پٹھ();
  أكد(!ڈھیر.خالی_تھاواں());
}

فراغ llvm::json::OStream::attributeBegin(llvm::StringRef کلید) {
  أكد(ڈھیر.پٹھ().Ctx == Object);
  إذا (ڈھیر.پٹھ().Has_قدر)
    OS << ',';
  newline();
  flushComment();
  ڈھیر.پٹھ().Has_قدر = سچ;
  ڈھیر.emplace_پٹھ();
  ڈھیر.پٹھ().Ctx = Singleton;
  إذا (LLVM_LIKELY(isUTF8(کلید))) {
    quote(OS, کلید);
  } وإلا {
    أكد(خطأ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(کلید));
  }
  OS.اكتب(':');
  إذا (Indent_ناپ)
    OS.اكتب(' ');
}

فراغ llvm::json::OStream::attribute_سرا() {
  أكد(ڈھیر.پٹھ().Ctx == Singleton);
  أكد(ڈھیر.پٹھ().Has_قدر && "Attribute must have a value");
  أكد(PendingComment.خالی_تھاواں());
  ڈھیر.نکالو_پٹھ();
  أكد(ڈھیر.پٹھ().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_قدر_Begin() {
  قدر_Begin();
  ڈھیر.emplace_پٹھ();
  ڈھیر.پٹھ().Ctx = Raw_قدر;
  أرجع OS;
}

فراغ llvm::json::OStream::raw_قدر_سرا() {
  أكد(ڈھیر.پٹھ().Ctx == Raw_قدر);
  ڈھیر.نکالو_پٹھ();
}

} // namespace json
} // namespace llvm

فراغ llvm::format_provider<llvm::json::قدر>::format(
    ثابت llvm::json::قدر &E, raw_ostream &OS, StringRef Options) {
  بنانشانی IndentAmount = 0;
  إذا (!Options.خالی_تھاواں() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).قدر_2(E);
}

