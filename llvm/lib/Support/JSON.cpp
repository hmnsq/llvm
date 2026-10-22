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

قدر &Object::عامل[](ثابت Object_ڪنجي &K) {
  أرجع try_emplace(K, خالي_اشارو).پهريون_2->get_ٻيو();
}
قدر &Object::عامل[](Object_ڪنجي &&K) {
  أرجع try_emplace(std::move(K), خالي_اشارو).پهريون_2->get_ٻيو();
}
قدر *Object::get(StringRef K) {
  خودڪار I = ڳوليو(K);
  إذا (I == پڇاڙي())
    أرجع خالي_اشارو;
  أرجع &I->ٻيو_2;
}
ثابت قدر *Object::get(StringRef K) ثابت {
  خودڪار I = ڳوليو(K);
  إذا (I == پڇاڙي())
    أرجع خالي_اشارو;
  أرجع &I->ٻيو_2;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ثابت {
  إذا (خودڪار *V = get(K))
    أرجع V->getAsNull();
  أرجع std::nullopt;
}
std::optional<منطقي> Object::getBoolean(StringRef K) ثابت {
  إذا (خودڪار *V = get(K))
    أرجع V->getAsBoolean();
  أرجع std::nullopt;
}
std::optional<مزدوج> Object::get_عدد(StringRef K) ثابت {
  إذا (خودڪار *V = get(K))
    أرجع V->get_As_عدد();
  أرجع std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ثابت {
  إذا (خودڪار *V = get(K))
    أرجع V->getAsInteger();
  أرجع std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ثابت {
  إذا (خودڪار *V = get(K))
    أرجع V->getAsString();
  أرجع std::nullopt;
}
ثابت json::Object *Object::getObject(StringRef K) ثابت {
  إذا (خودڪار *V = get(K))
    أرجع V->getAsObject();
  أرجع خالي_اشارو;
}
json::Object *Object::getObject(StringRef K) {
  إذا (خودڪار *V = get(K))
    أرجع V->getAsObject();
  أرجع خالي_اشارو;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  إذا (خودڪار *V = get(K))
    أرجع V->getAsArray();
  أرجع خالي_اشارو;
}
json::Array *Object::getArray(StringRef K) {
  إذا (خودڪار *V = get(K))
    أرجع V->getAsArray();
  أرجع خالي_اشارو;
}
منطقي عامل==(ثابت Object &LHS, ثابت Object &RHS) { أرجع LHS.M == RHS.M; }

Array::Array(std::initializer_list<قدر> Elements) {
  V.reserve(Elements.سائيز_2());
  لكل (ثابت قدر &V : Elements) {
    emplace_پٺ(خالي_اشارو);
    پٺ().moveFrom(std::move(V));
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
    يادگيري_نقل(&Union, &M.Union, حجم(Union));
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
    يادگيري_نقل(&Union, &M.Union, حجم(Union));
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
LLVM_DUMP_طريقو فراغ قدر::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#جيڪڏهن_پورو

منطقي عامل==(ثابت قدر &L, ثابت قدر &R) {
  إذا (L.قسم() != R.قسم())
    أرجع خطأ;
  بدل (L.قسم()) {
  حالة قدر::Null:
    أرجع *L.getAsNull() == *R.getAsNull();
  حالة قدر::Boolean:
    أرجع *L.getAsBoolean() == *R.getAsBoolean();
  حالة قدر::عدد:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    إذا (L.Type == قدر::T_Integer || R.Type == قدر::T_Integer)
      أرجع L.getAsInteger() == R.getAsInteger();
    أرجع *L.get_As_عدد() == *R.get_As_عدد();
  حالة قدر::String:
    أرجع *L.getAsString() == *R.getAsString();
  حالة قدر::Array:
    أرجع *L.getAsArray() == *R.getAsArray();
  حالة قدر::Object:
    أرجع *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

فراغ رستو::report(llvm::StringLiteral نياپو_2) {
  // Walk up to the root context, and count the number of segments.
  بنانشان ڳڻپ_2 = 0;
  ثابت رستو *P;
  لكل (P = هذا; P->والد_2 != خالي_اشارو; P = P->والد_2)
    ++ڳڻپ_2;
  رستو::پاڙ_2 *R = P->Seg.پاڙ_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_پيغام = نياپو_2;
  R->Error_رستو.resize(ڳڻپ_2);
  خودڪار It = R->Error_رستو.begin();
  لكل (P = هذا; P->والد_2 != خالي_اشارو; P = P->والد_2)
    *It++ = P->Seg;
}

Error رستو::پاڙ_2::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_پيغام.خالي_جايون() ? "invalid JSON contents" : Error_پيغام);
  إذا (Error_رستو.خالي_جايون()) {
    إذا (!Name.خالي_جايون())
      OS << " when parsing " << Name;
  } وإلا {
    OS << " at " << (Name.خالي_جايون() ? "(root)" : Name);
    لكل (ثابت رستو::Segment &S : llvm::ابتو(Error_رستو)) {
      إذا (S.is_ميدان())
        OS << '.' << S.ميدان();
      وإلا
        OS << '[' << S.اشاريو() << ']';
    }
  }
  أرجع createStringError(llvm::inconvertible_Error_ڪوڊ(), S);
}

std::ويڪٽر<ثابت Object::قدر_type *> sortedElements(ثابت Object &O) {
  std::ويڪٽر<ثابت Object::قدر_type *> Elements;
  لكل (ثابت خودڪار &E : O)
    Elements.ڌڪو_پٺ(&E);
  llvm::ترتيب_ڏيو(Elements,
             [](ثابت Object::قدر_type *L, ثابت Object::قدر_type *R) {
               أرجع L->پهريون_2 < R->پهريون_2;
             });
  أرجع Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ساكن فراغ abbreviate(ثابت قدر &V, OStream &JOS) {
  بدل (V.قسم()) {
  حالة قدر::Array:
    JOS.raw_قدر(V.getAsArray()->خالي_جايون() ? "[]" : "[ ... ]");
    اكسر;
  حالة قدر::Object:
    JOS.raw_قدر(V.getAsObject()->خالي_جايون() ? "{}" : "{ ... }");
    اكسر;
  حالة قدر::String: {
    llvm::StringRef S = *V.getAsString();
    إذا (S.سائيز_2() < 40) {
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
      لكل (ثابت خودڪار &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    اكسر;
  حالة قدر::Object:
    JOS.object([&] {
      لكل (ثابت خودڪار *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->پهريون_2);
        abbreviate(KV->ٻيو_2, JOS);
        JOS.attribute_پڇاڙي();
      }
    });
    اكسر;
  افتراضي:
    JOS.قدر_2(V);
  }
}

فراغ رستو::پاڙ_2::printErrorContext(ثابت قدر &R, raw_ostream &OS) ثابت {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  خودڪار Print_قدر = [&](ثابت قدر &V, ArrayRef<Segment> رستو, خودڪار &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    خودڪار HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_پيغام.معلومات(), Error_پيغام.سائيز_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    إذا (رستو.خالي_جايون()) // We reached our target.
      أرجع HighlightCurrent();
    ثابت Segment &S = رستو.پٺ(); // Path is in reverse order.
    إذا (S.is_ميدان()) {
      // Current node is an object, path names a field.
      llvm::StringRef ميدان_Name = S.ميدان();
      ثابت Object *O = V.getAsObject();
      إذا (!O || !O->get(ميدان_Name))
        أرجع HighlightCurrent();
      JOS.object([&] {
        لكل (ثابت خودڪار *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->پهريون_2);
          إذا (ميدان_Name == StringRef(KV->پهريون_2))
            Recurse(KV->ٻيو_2, رستو.drop_پٺ(), Recurse);
          وإلا
            abbreviate(KV->ٻيو_2, JOS);
          JOS.attribute_پڇاڙي();
        }
      });
    } وإلا {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      إذا (!A || S.اشاريو() >= A->سائيز_2())
        أرجع HighlightCurrent();
      JOS.array([&] {
        بنانشان Current = 0;
        لكل (ثابت خودڪار &V : *A) {
          إذا (Current++ == S.اشاريو())
            Recurse(V, رستو.drop_پٺ(), Recurse);
          وإلا
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_قدر(R, Error_رستو, Print_قدر);
}

نطاق {
// Simple recursive-descent JSON parser.
صنف Parser {
عام:
  Parser(StringRef JSON)
      : شروعات(JSON.begin()), P(JSON.begin()), پڇاڙي_2(JSON.پڇاڙي()) {}

  منطقي checkUTF8() {
    size_t Err_وٿي;
    إذا (isUTF8(StringRef(شروعات, پڇاڙي_2 - شروعات), &Err_وٿي))
      أرجع سچ;
    P = شروعات + Err_وٿي; // For line/column calculation.
    أرجع parseError("Invalid UTF-8 sequence");
  }

  منطقي parse_قدر(قدر &ٻاهريون);

  منطقي assert_پڇاڙي() {
    eatWhitespace();
    إذا (P == پڇاڙي_2)
      أرجع سچ;
    أرجع parseError("Text after end of document");
  }

  Error takeError() {
    أكد(Err);
    أرجع std::move(*Err);
  }

خاص:
  فراغ eatWhitespace() {
    طالما (P != پڇاڙي_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  منطقي parse_عدد(حرف پهريون, قدر &ٻاهريون);
  منطقي parseString(std::string &ٻاهريون);
  منطقي parseUnicode(std::string &ٻاهريون);
  منطقي parseError(ثابت حرف *نياپو_2); // always returns false

  حرف ايندڙ() { أرجع P == پڇاڙي_2 ? 0 : *P++; }
  حرف peek() { أرجع P == پڇاڙي_2 ? 0 : *P; }
  ساكن منطقي is_عدد(حرف C) {
    أرجع C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت حرف *شروعات, *P, *پڇاڙي_2;
};
} // namespace

منطقي Parser::parse_قدر(قدر &ٻاهريون) {
  eatWhitespace();
  إذا (P == پڇاڙي_2)
    أرجع parseError("Unexpected EOF");
  بدل (حرف C = ايندڙ()) {
  // Bare null/true/false are easy - first char identifies them.
  حالة 'n':
    ٻاهريون = خالي_اشارو;
    أرجع (ايندڙ() == 'u' && ايندڙ() == 'l' && ايندڙ() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالة 't':
    ٻاهريون = سچ;
    أرجع (ايندڙ() == 'r' && ايندڙ() == 'u' && ايندڙ() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالة 'f':
    ٻاهريون = خطأ;
    أرجع (ايندڙ() == 'a' && ايندڙ() == 'l' && ايندڙ() == 's' && ايندڙ() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالة '"': {
    std::string S;
    إذا (parseString(S)) {
      ٻاهريون = std::move(S);
      أرجع سچ;
    }
    أرجع خطأ;
  }
  حالة '[': {
    ٻاهريون = Array{};
    Array &A = *ٻاهريون.getAsArray();
    eatWhitespace();
    إذا (peek() == ']') {
      ++P;
      أرجع سچ;
    }
    لكل (;;) {
      A.emplace_پٺ(خالي_اشارو);
      إذا (!parse_قدر(A.پٺ()))
        أرجع خطأ;
      eatWhitespace();
      بدل (ايندڙ()) {
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
    ٻاهريون = Object{};
    Object &O = *ٻاهريون.getAsObject();
    eatWhitespace();
    إذا (peek() == '}') {
      ++P;
      أرجع سچ;
    }
    لكل (;;) {
      إذا (ايندڙ() != '"')
        أرجع parseError("Expected object key");
      std::string K;
      إذا (!parseString(K))
        أرجع خطأ;
      eatWhitespace();
      إذا (ايندڙ() != ':')
        أرجع parseError("Expected : after object key");
      eatWhitespace();
      إذا (!parse_قدر(O[std::move(K)]))
        أرجع خطأ;
      eatWhitespace();
      بدل (ايندڙ()) {
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
    إذا (is_عدد(C))
      أرجع parse_عدد(C, ٻاهريون);
    أرجع parseError("Invalid JSON value");
  }
}

منطقي Parser::parse_عدد(حرف پهريون, قدر &ٻاهريون) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ڌڪو_پٺ(پهريون);
  طالما (is_عدد(peek()))
    S.ڌڪو_پٺ(ايندڙ());
  حرف *پڇاڙي_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &پڇاڙي_2, 10);
  إذا (پڇاڙي_2 == S.پڇاڙي() && errno != ERANGE) {
    ٻاهريون = I;
    أرجع سچ;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  إذا (پهريون != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &پڇاڙي_2, 10);
    إذا (پڇاڙي_2 == S.پڇاڙي() && errno != ERANGE) {
      ٻاهريون = UI;
      أرجع سچ;
    }
  }
  // If it's not an integer
  ٻاهريون = std::strtod(S.c_str(), &پڇاڙي_2);
  أرجع پڇاڙي_2 == S.پڇاڙي() || parseError("Invalid JSON value (number?)");
}

منطقي Parser::parseString(std::string &ٻاهريون) {
  // leading quote was already consumed.
  لكل (حرف C = ايندڙ(); C != '"'; C = ايندڙ()) {
    إذا (LLVM_UNLIKELY(P == پڇاڙي_2))
      أرجع parseError("Unterminated string");
    إذا (LLVM_UNLIKELY((C & 0x1f) == C))
      أرجع parseError("Control character in string");
    إذا (LLVM_LIKELY(C != '\\')) {
      ٻاهريون.ڌڪو_پٺ(C);
      استمر;
    }
    // Handle escape sequence.
    بدل (C = ايندڙ()) {
    حالة '"':
    حالة '\\':
    حالة '/':
      ٻاهريون.ڌڪو_پٺ(C);
      اكسر;
    حالة 'b':
      ٻاهريون.ڌڪو_پٺ('\b');
      اكسر;
    حالة 'f':
      ٻاهريون.ڌڪو_پٺ('\f');
      اكسر;
    حالة 'n':
      ٻاهريون.ڌڪو_پٺ('\n');
      اكسر;
    حالة 'r':
      ٻاهريون.ڌڪو_پٺ('\r');
      اكسر;
    حالة 't':
      ٻاهريون.ڌڪو_پٺ('\t');
      اكسر;
    حالة 'u':
      إذا (!parseUnicode(ٻاهريون))
        أرجع خطأ;
      اكسر;
    افتراضي:
      أرجع parseError("Invalid escape sequence");
    }
  }
  أرجع سچ;
}

ساكن فراغ encodeUtf8(uint32_t Rune, std::string &ٻاهريون) {
  إذا (Rune < 0x80) {
    ٻاهريون.ڌڪو_پٺ(Rune & 0x7F);
  } وإلا إذا (Rune < 0x800) {
    uint8_t پهريون_Byte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t ٻيو_Byte = 0x80 | (Rune & 0x3F);
    ٻاهريون.ڌڪو_پٺ(پهريون_Byte);
    ٻاهريون.ڌڪو_پٺ(ٻيو_Byte);
  } وإلا إذا (Rune < 0x10000) {
    uint8_t پهريون_Byte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t ٻيو_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ٽيون_Byte = 0x80 | (Rune & 0x3F);
    ٻاهريون.ڌڪو_پٺ(پهريون_Byte);
    ٻاهريون.ڌڪو_پٺ(ٻيو_Byte);
    ٻاهريون.ڌڪو_پٺ(ٽيون_Byte);
  } وإلا إذا (Rune < 0x110000) {
    uint8_t پهريون_Byte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t ٻيو_Byte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ٽيون_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    ٻاهريون.ڌڪو_پٺ(پهريون_Byte);
    ٻاهريون.ڌڪو_پٺ(ٻيو_Byte);
    ٻاهريون.ڌڪو_پٺ(ٽيون_Byte);
    ٻاهريون.ڌڪو_پٺ(FourthByte);
  } وإلا {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
منطقي Parser::parseUnicode(std::string &ٻاهريون) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  خودڪار Invalid = [&] { ٻاهريون.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  خودڪار Parse4Hex = [هذا](uint16_t &ٻاهريون) -> منطقي {
    ٻاهريون = 0;
    حرف بائيٽون_2[] = {ايندڙ(), ايندڙ(), ايندڙ(), ايندڙ()};
    لكل (بنانشان حرف C : بائيٽون_2) {
      إذا (!std::isxdigit(C))
        أرجع parseError("Invalid \\u escape sequence");
      ٻاهريون <<= 4;
      ٻاهريون |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    أرجع سچ;
  };
  uint16_t پهريون; // UTF-16 code unit from the first \u escape.
  إذا (!Parse4Hex(پهريون))
    أرجع خطأ;

  // We loop to allow proper surrogate-pair error handling.
  طالما (سچ) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    إذا (LLVM_LIKELY(پهريون < 0xD800 || پهريون >= 0xE000)) {
      encodeUtf8(پهريون, ٻاهريون);
      أرجع سچ;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    إذا (LLVM_UNLIKELY(پهريون >= 0xDC00)) {
      Invalid();
      أرجع سچ;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    إذا (LLVM_UNLIKELY(P + 2 > پڇاڙي_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      أرجع سچ;
    }
    P += 2;
    uint16_t ٻيو;
    إذا (!Parse4Hex(ٻيو))
      أرجع خطأ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    إذا (LLVM_UNLIKELY(ٻيو < 0xDC00 || ٻيو >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      پهريون = ٻيو; // Second escape still needs to be processed.
      استمر;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((پهريون - 0xD800) << 10) | (ٻيو - 0xDC00), ٻاهريون);
    أرجع سچ;
  }
}

منطقي Parser::parseError(ثابت حرف *نياپو_2) {
  صحيح سٽ = 1;
  ثابت حرف *شروعات_جو_سٽ = شروعات;
  لكل (ثابت حرف *X = شروعات; X < P; ++X) {
    إذا (*X == 0x0A) {
      ++سٽ;
      شروعات_جو_سٽ = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(نياپو_2, سٽ, P - شروعات_جو_سٽ, P - شروعات));
  أرجع خطأ;
}

Expected<قدر> parse(StringRef JSON) {
  Parser P(JSON);
  قدر E = خالي_اشارو;
  إذا (P.checkUTF8())
    إذا (P.parse_قدر(E))
      إذا (P.assert_پڇاڙي())
        أرجع std::move(E);
  أرجع P.takeError();
}

حرف ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
فراغ ParseError::لاگرٿم(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", سٽ, ڪالم, وٿي, نياپو_2);
}

منطقي isUTF8(llvm::StringRef S, size_t *Err_وٿي) {
  // Fast-path for ASCII, which is valid UTF-8.
  إذا (LLVM_LIKELY(isASCII(S)))
    أرجع سچ;

  ثابت UTF8 *معلومات_2 = reinterpret_cast<ثابت UTF8 *>(S.معلومات()), *Rest = معلومات_2;
  إذا (LLVM_LIKELY(isLegalUTF8String(&Rest, معلومات_2 + S.سائيز_2())))
    أرجع سچ;

  إذا (Err_وٿي)
    *Err_وٿي = Rest - معلومات_2;
  أرجع خطأ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::ويڪٽر<UTF32> Codepoints(S.سائيز_2()); // 1 codepoint per byte suffices.
  ثابت UTF8 *In8 = reinterpret_cast<ثابت UTF8 *>(S.معلومات());
  UTF32 *Out32 = Codepoints.معلومات();
  ConvertUTF8toUTF32(&In8, In8 + S.سائيز_2(), &Out32, Out32 + Codepoints.سائيز_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.معلومات());
  std::string Res(4 * Codepoints.سائيز_2(), 0); // 4 bytes per codepoint suffice
  ثابت UTF32 *In32 = Codepoints.معلومات();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.سائيز_2(), &Out8, Out8 + Res.سائيز_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<حرف *>(Out8) - Res.معلومات());
  أرجع Res;
}

ساكن فراغ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  لكل (بنانشان حرف C : S) {
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
      llvm::write_hex(OS, C, llvm::HexPrintStyle::هيٺاهون_2, 4);
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
  حالة قدر::عدد:
    قدر_Begin();
    إذا (V.Type == قدر::T_Integer)
      OS << *V.getAsInteger();
    وإلا إذا (V.Type == قدر::T_UINT64)
      OS << *V.getAsUINT64();
    وإلا
      OS << format("%.*g", std::numeric_limits<مزدوج>::max_digits10,
                   *V.get_As_عدد());
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
        attribute(E->پهريون_2, E->ٻيو_2);
    });
  }
}

فراغ llvm::json::OStream::قدر_Begin() {
  أكد(ڍير.پٺ().Ctx != Object && "Only attributes allowed here");
  إذا (ڍير.پٺ().Has_قدر) {
    أكد(ڍير.پٺ().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  إذا (ڍير.پٺ().Ctx == Array)
    newline();
  flushComment();
  ڍير.پٺ().Has_قدر = سچ;
}

فراغ OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.خالي_جايون() && "Only one comment per value!");
  PendingComment = Comment;
}

فراغ OStream::flushComment() {
  إذا (PendingComment.خالي_جايون())
    أرجع;
  OS << (Indent_سائيز ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  طالما (!PendingComment.خالي_جايون()) {
    خودڪار Pos = PendingComment.ڳوليو("*/");
    إذا (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } وإلا {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_سائيز ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  إذا (ڍير.سائيز_2() > 1 && ڍير.پٺ().Ctx == Singleton) {
    إذا (Indent_سائيز)
      OS << ' ';
  } وإلا {
    newline();
  }
}

فراغ llvm::json::OStream::newline() {
  إذا (Indent_سائيز) {
    OS.اكتب('\n');
    OS.indent(Indent);
  }
}

فراغ llvm::json::OStream::arrayBegin() {
  قدر_Begin();
  ڍير.emplace_پٺ();
  ڍير.پٺ().Ctx = Array;
  Indent += Indent_سائيز;
  OS << '[';
}

فراغ llvm::json::OStream::array_پڇاڙي() {
  أكد(ڍير.پٺ().Ctx == Array);
  Indent -= Indent_سائيز;
  إذا (ڍير.پٺ().Has_قدر)
    newline();
  OS << ']';
  أكد(PendingComment.خالي_جايون());
  ڍير.ڪڍو_پٺ();
  أكد(!ڍير.خالي_جايون());
}

فراغ llvm::json::OStream::objectBegin() {
  قدر_Begin();
  ڍير.emplace_پٺ();
  ڍير.پٺ().Ctx = Object;
  Indent += Indent_سائيز;
  OS << '{';
}

فراغ llvm::json::OStream::object_پڇاڙي() {
  أكد(ڍير.پٺ().Ctx == Object);
  Indent -= Indent_سائيز;
  إذا (ڍير.پٺ().Has_قدر)
    newline();
  OS << '}';
  أكد(PendingComment.خالي_جايون());
  ڍير.ڪڍو_پٺ();
  أكد(!ڍير.خالي_جايون());
}

فراغ llvm::json::OStream::attributeBegin(llvm::StringRef ڪنجي) {
  أكد(ڍير.پٺ().Ctx == Object);
  إذا (ڍير.پٺ().Has_قدر)
    OS << ',';
  newline();
  flushComment();
  ڍير.پٺ().Has_قدر = سچ;
  ڍير.emplace_پٺ();
  ڍير.پٺ().Ctx = Singleton;
  إذا (LLVM_LIKELY(isUTF8(ڪنجي))) {
    quote(OS, ڪنجي);
  } وإلا {
    أكد(خطأ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(ڪنجي));
  }
  OS.اكتب(':');
  إذا (Indent_سائيز)
    OS.اكتب(' ');
}

فراغ llvm::json::OStream::attribute_پڇاڙي() {
  أكد(ڍير.پٺ().Ctx == Singleton);
  أكد(ڍير.پٺ().Has_قدر && "Attribute must have a value");
  أكد(PendingComment.خالي_جايون());
  ڍير.ڪڍو_پٺ();
  أكد(ڍير.پٺ().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_قدر_Begin() {
  قدر_Begin();
  ڍير.emplace_پٺ();
  ڍير.پٺ().Ctx = Raw_قدر;
  أرجع OS;
}

فراغ llvm::json::OStream::raw_قدر_پڇاڙي() {
  أكد(ڍير.پٺ().Ctx == Raw_قدر);
  ڍير.ڪڍو_پٺ();
}

} // namespace json
} // namespace llvm

فراغ llvm::format_provider<llvm::json::قدر>::format(
    ثابت llvm::json::قدر &E, raw_ostream &OS, StringRef Options) {
  بنانشان IndentAmount = 0;
  إذا (!Options.خالي_جايون() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).قدر_2(E);
}

