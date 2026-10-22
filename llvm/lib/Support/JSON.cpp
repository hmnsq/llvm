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

بەها &Object::operator[](ثابت Object_کلیل &K) {
  أرجع try_emplace(K, nullptr).first->getSecond();
}
بەها &Object::operator[](Object_کلیل &&K) {
  أرجع try_emplace(std::move(K), nullptr).first->getSecond();
}
بەها *Object::get(StringRef K) {
  auto I = find(K);
  إذا (I == end())
    أرجع nullptr;
  أرجع &I->second;
}
ثابت بەها *Object::get(StringRef K) ثابت {
  auto I = find(K);
  إذا (I == end())
    أرجع nullptr;
  أرجع &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ثابت {
  إذا (auto *V = get(K))
    أرجع V->getAsNull();
  أرجع std::nullopt;
}
std::optional<منطقي> Object::getBoolean(StringRef K) ثابت {
  إذا (auto *V = get(K))
    أرجع V->getAsBoolean();
  أرجع std::nullopt;
}
std::optional<مزدوج> Object::get_ژمار(StringRef K) ثابت {
  إذا (auto *V = get(K))
    أرجع V->get_As_ژمار();
  أرجع std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ثابت {
  إذا (auto *V = get(K))
    أرجع V->getAsInteger();
  أرجع std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ثابت {
  إذا (auto *V = get(K))
    أرجع V->getAsString();
  أرجع std::nullopt;
}
ثابت json::Object *Object::getObject(StringRef K) ثابت {
  إذا (auto *V = get(K))
    أرجع V->getAsObject();
  أرجع nullptr;
}
json::Object *Object::getObject(StringRef K) {
  إذا (auto *V = get(K))
    أرجع V->getAsObject();
  أرجع nullptr;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  إذا (auto *V = get(K))
    أرجع V->getAsArray();
  أرجع nullptr;
}
json::Array *Object::getArray(StringRef K) {
  إذا (auto *V = get(K))
    أرجع V->getAsArray();
  أرجع nullptr;
}
منطقي operator==(ثابت Object &LHS, ثابت Object &RHS) { أرجع LHS.M == RHS.M; }

Array::Array(std::initializer_list<بەها> Elements) {
  V.reserve(Elements.size());
  لكل (ثابت بەها &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

بەها::بەها(std::initializer_list<بەها> Elements)
    : بەها(json::Array(Elements)) {}

فراغ بەها::copyFrom(ثابت بەها &M) {
  Type = M.Type;
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    memcpy(&Union, &M.Union, حجم(Union));
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

فراغ بەها::moveFrom(ثابت بەها &&M) {
  Type = M.Type;
  بدل (Type) {
  حالة T_Null:
  حالة T_Boolean:
  حالة T_Double:
  حالة T_Integer:
  حالة T_UINT64:
    memcpy(&Union, &M.Union, حجم(Union));
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
  const_cast<بەها &>(M).destroy();
  M.Type = T_Null;
}

فراغ بەها::destroy() {
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

فراغ بەها::print(llvm::raw_ostream &OS) ثابت { OS << *هذا; }

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ بەها::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

منطقي operator==(ثابت بەها &L, ثابت بەها &R) {
  إذا (L.kind() != R.kind())
    أرجع خطأ;
  بدل (L.kind()) {
  حالة بەها::Null:
    أرجع *L.getAsNull() == *R.getAsNull();
  حالة بەها::Boolean:
    أرجع *L.getAsBoolean() == *R.getAsBoolean();
  حالة بەها::ژمار:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    إذا (L.Type == بەها::T_Integer || R.Type == بەها::T_Integer)
      أرجع L.getAsInteger() == R.getAsInteger();
    أرجع *L.get_As_ژمار() == *R.get_As_ژمار();
  حالة بەها::String:
    أرجع *L.getAsString() == *R.getAsString();
  حالة بەها::Array:
    أرجع *L.getAsArray() == *R.getAsArray();
  حالة بەها::Object:
    أرجع *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

فراغ Path::report(llvm::StringLiteral پەیام_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned ژماردن_2 = 0;
  ثابت Path *P;
  لكل (P = هذا; P->Parent != nullptr; P = P->Parent)
    ++ژماردن_2;
  Path::ڕەگ_2 *R = P->Seg.ڕەگ_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = پەیام_2;
  R->ErrorPath.resize(ژماردن_2);
  auto It = R->ErrorPath.begin();
  لكل (P = هذا; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::ڕەگ_2::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  إذا (ErrorPath.empty()) {
    إذا (!Name.empty())
      OS << " when parsing " << Name;
  } وإلا {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    لكل (ثابت Path::Segment &S : llvm::reverse(ErrorPath)) {
      إذا (S.isField())
        OS << '.' << S.field();
      وإلا
        OS << '[' << S.index() << ']';
    }
  }
  أرجع createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ثابت Object::بەها_type *> sortedElements(ثابت Object &O) {
  std::vector<ثابت Object::بەها_type *> Elements;
  لكل (ثابت auto &E : O)
    Elements.پاڵنان_back(&E);
  llvm::sort(Elements,
             [](ثابت Object::بەها_type *L, ثابت Object::بەها_type *R) {
               أرجع L->first < R->first;
             });
  أرجع Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ساكن فراغ abbreviate(ثابت بەها &V, OStream &JOS) {
  بدل (V.kind()) {
  حالة بەها::Array:
    JOS.raw_بەها(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    اكسر;
  حالة بەها::Object:
    JOS.raw_بەها(V.getAsObject()->empty() ? "{}" : "{ ... }");
    اكسر;
  حالة بەها::String: {
    llvm::StringRef S = *V.getAsString();
    إذا (S.size() < 40) {
      JOS.بەها_2(V);
    } وإلا {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.بەها_2(Truncated);
    }
    اكسر;
  }
  افتراضي:
    JOS.بەها_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ساكن فراغ abbreviateChildren(ثابت بەها &V, OStream &JOS) {
  بدل (V.kind()) {
  حالة بەها::Array:
    JOS.array([&] {
      لكل (ثابت auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    اكسر;
  حالة بەها::Object:
    JOS.object([&] {
      لكل (ثابت auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    اكسر;
  افتراضي:
    JOS.بەها_2(V);
  }
}

فراغ Path::ڕەگ_2::printErrorContext(ثابت بەها &R, raw_ostream &OS) ثابت {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_بەها = [&](ثابت بەها &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    إذا (Path.empty()) // We reached our target.
      أرجع HighlightCurrent();
    ثابت Segment &S = Path.back(); // Path is in reverse order.
    إذا (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ثابت Object *O = V.getAsObject();
      إذا (!O || !O->get(FieldName))
        أرجع HighlightCurrent();
      JOS.object([&] {
        لكل (ثابت auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          إذا (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          وإلا
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } وإلا {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      إذا (!A || S.index() >= A->size())
        أرجع HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        لكل (ثابت auto &V : *A) {
          إذا (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          وإلا
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_بەها(R, ErrorPath, Print_بەها);
}

نطاق {
// Simple recursive-descent JSON parser.
صنف Parser {
عام:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  منطقي checkUTF8() {
    size_t ErrOffset;
    إذا (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      أرجع true;
    P = Start + ErrOffset; // For line/column calculation.
    أرجع parseError("Invalid UTF-8 sequence");
  }

  منطقي parse_بەها(بەها &Out);

  منطقي assertEnd() {
    eatWhitespace();
    إذا (P == End)
      أرجع true;
    أرجع parseError("Text after end of document");
  }

  Error takeError() {
    أكد(Err);
    أرجع std::move(*Err);
  }

خاص:
  فراغ eatWhitespace() {
    طالما (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  منطقي parse_ژمار(حرف First, بەها &Out);
  منطقي parseString(std::string &Out);
  منطقي parseUnicode(std::string &Out);
  منطقي parseError(ثابت حرف *پەیام_2); // always returns false

  حرف next() { أرجع P == End ? 0 : *P++; }
  حرف peek() { أرجع P == End ? 0 : *P; }
  ساكن منطقي is_ژمار(حرف C) {
    أرجع C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت حرف *Start, *P, *End;
};
} // namespace

منطقي Parser::parse_بەها(بەها &Out) {
  eatWhitespace();
  إذا (P == End)
    أرجع parseError("Unexpected EOF");
  بدل (حرف C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  حالة 'n':
    Out = nullptr;
    أرجع (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالة 't':
    Out = true;
    أرجع (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالة 'f':
    Out = خطأ;
    أرجع (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالة '"': {
    std::string S;
    إذا (parseString(S)) {
      Out = std::move(S);
      أرجع true;
    }
    أرجع خطأ;
  }
  حالة '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    إذا (peek() == ']') {
      ++P;
      أرجع true;
    }
    لكل (;;) {
      A.emplace_back(nullptr);
      إذا (!parse_بەها(A.back()))
        أرجع خطأ;
      eatWhitespace();
      بدل (next()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة ']':
        أرجع true;
      افتراضي:
        أرجع parseError("Expected , or ] after array element");
      }
    }
  }
  حالة '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    إذا (peek() == '}') {
      ++P;
      أرجع true;
    }
    لكل (;;) {
      إذا (next() != '"')
        أرجع parseError("Expected object key");
      std::string K;
      إذا (!parseString(K))
        أرجع خطأ;
      eatWhitespace();
      إذا (next() != ':')
        أرجع parseError("Expected : after object key");
      eatWhitespace();
      إذا (!parse_بەها(O[std::move(K)]))
        أرجع خطأ;
      eatWhitespace();
      بدل (next()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة '}':
        أرجع true;
      افتراضي:
        أرجع parseError("Expected , or } after object property");
      }
    }
  }
  افتراضي:
    إذا (is_ژمار(C))
      أرجع parse_ژمار(C, Out);
    أرجع parseError("Invalid JSON value");
  }
}

منطقي Parser::parse_ژمار(حرف First, بەها &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.پاڵنان_back(First);
  طالما (is_ژمار(peek()))
    S.پاڵنان_back(next());
  حرف *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  إذا (End == S.end() && errno != ERANGE) {
    Out = I;
    أرجع true;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  إذا (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    إذا (End == S.end() && errno != ERANGE) {
      Out = UI;
      أرجع true;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  أرجع End == S.end() || parseError("Invalid JSON value (number?)");
}

منطقي Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  لكل (حرف C = next(); C != '"'; C = next()) {
    إذا (LLVM_UNLIKELY(P == End))
      أرجع parseError("Unterminated string");
    إذا (LLVM_UNLIKELY((C & 0x1f) == C))
      أرجع parseError("Control character in string");
    إذا (LLVM_LIKELY(C != '\\')) {
      Out.پاڵنان_back(C);
      استمر;
    }
    // Handle escape sequence.
    بدل (C = next()) {
    حالة '"':
    حالة '\\':
    حالة '/':
      Out.پاڵنان_back(C);
      اكسر;
    حالة 'b':
      Out.پاڵنان_back('\b');
      اكسر;
    حالة 'f':
      Out.پاڵنان_back('\f');
      اكسر;
    حالة 'n':
      Out.پاڵنان_back('\n');
      اكسر;
    حالة 'r':
      Out.پاڵنان_back('\r');
      اكسر;
    حالة 't':
      Out.پاڵنان_back('\t');
      اكسر;
    حالة 'u':
      إذا (!parseUnicode(Out))
        أرجع خطأ;
      اكسر;
    افتراضي:
      أرجع parseError("Invalid escape sequence");
    }
  }
  أرجع true;
}

ساكن فراغ encodeUtf8(uint32_t Rune, std::string &Out) {
  إذا (Rune < 0x80) {
    Out.پاڵنان_back(Rune & 0x7F);
  } وإلا إذا (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.پاڵنان_back(FirstByte);
    Out.پاڵنان_back(SecondByte);
  } وإلا إذا (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.پاڵنان_back(FirstByte);
    Out.پاڵنان_back(SecondByte);
    Out.پاڵنان_back(ThirdByte);
  } وإلا إذا (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.پاڵنان_back(FirstByte);
    Out.پاڵنان_back(SecondByte);
    Out.پاڵنان_back(ThirdByte);
    Out.پاڵنان_back(FourthByte);
  } وإلا {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
منطقي Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [هذا](uint16_t &Out) -> منطقي {
    Out = 0;
    حرف Bytes[] = {next(), next(), next(), next()};
    لكل (unsigned حرف C : Bytes) {
      إذا (!std::isxdigit(C))
        أرجع parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    أرجع true;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  إذا (!Parse4Hex(First))
    أرجع خطأ;

  // We loop to allow proper surrogate-pair error handling.
  طالما (true) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    إذا (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      أرجع true;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    إذا (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      أرجع true;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    إذا (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      أرجع true;
    }
    P += 2;
    uint16_t Second;
    إذا (!Parse4Hex(Second))
      أرجع خطأ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    إذا (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      استمر;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    أرجع true;
  }
}

منطقي Parser::parseError(ثابت حرف *پەیام_2) {
  صحيح هێڵ = 1;
  ثابت حرف *Start_Of_هێڵ = Start;
  لكل (ثابت حرف *X = Start; X < P; ++X) {
    إذا (*X == 0x0A) {
      ++هێڵ;
      Start_Of_هێڵ = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(پەیام_2, هێڵ, P - Start_Of_هێڵ, P - Start));
  أرجع خطأ;
}

Expected<بەها> parse(StringRef JSON) {
  Parser P(JSON);
  بەها E = nullptr;
  إذا (P.checkUTF8())
    إذا (P.parse_بەها(E))
      إذا (P.assertEnd())
        أرجع std::move(E);
  أرجع P.takeError();
}

حرف ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
فراغ ParseError::log(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", هێڵ, Column, Offset, پەیام_2);
}

منطقي isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  إذا (LLVM_LIKELY(isASCII(S)))
    أرجع true;

  ثابت UTF8 *Data = reinterpret_cast<ثابت UTF8 *>(S.data()), *Rest = Data;
  إذا (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    أرجع true;

  إذا (ErrOffset)
    *ErrOffset = Rest - Data;
  أرجع خطأ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  ثابت UTF8 *In8 = reinterpret_cast<ثابت UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  ثابت UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<حرف *>(Out8) - Res.data());
  أرجع Res;
}

ساكن فراغ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  لكل (unsigned حرف C : S) {
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
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      اكسر;
    }
  }
  OS << '\"';
}

فراغ llvm::json::OStream::بەها_2(ثابت بەها &V) {
  بدل (V.kind()) {
  حالة بەها::Null:
    بەها_Begin();
    OS << "null";
    أرجع;
  حالة بەها::Boolean:
    بەها_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    أرجع;
  حالة بەها::ژمار:
    بەها_Begin();
    إذا (V.Type == بەها::T_Integer)
      OS << *V.getAsInteger();
    وإلا إذا (V.Type == بەها::T_UINT64)
      OS << *V.getAsUINT64();
    وإلا
      OS << format("%.*g", std::numeric_limits<مزدوج>::max_digits10,
                   *V.get_As_ژمار());
    أرجع;
  حالة بەها::String:
    بەها_Begin();
    quote(OS, *V.getAsString());
    أرجع;
  حالة بەها::Array:
    أرجع array([&] {
      لكل (ثابت بەها &E : *V.getAsArray())
        بەها_2(E);
    });
  حالة بەها::Object:
    أرجع object([&] {
      لكل (ثابت Object::بەها_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

فراغ llvm::json::OStream::بەها_Begin() {
  أكد(کەڵەکە.back().Ctx != Object && "Only attributes allowed here");
  إذا (کەڵەکە.back().Has_بەها) {
    أكد(کەڵەکە.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  إذا (کەڵەکە.back().Ctx == Array)
    newline();
  flushComment();
  کەڵەکە.back().Has_بەها = true;
}

فراغ OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

فراغ OStream::flushComment() {
  إذا (PendingComment.empty())
    أرجع;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  طالما (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    إذا (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } وإلا {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  إذا (کەڵەکە.size() > 1 && کەڵەکە.back().Ctx == Singleton) {
    إذا (IndentSize)
      OS << ' ';
  } وإلا {
    newline();
  }
}

فراغ llvm::json::OStream::newline() {
  إذا (IndentSize) {
    OS.اكتب('\n');
    OS.indent(Indent);
  }
}

فراغ llvm::json::OStream::arrayBegin() {
  بەها_Begin();
  کەڵەکە.emplace_back();
  کەڵەکە.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

فراغ llvm::json::OStream::arrayEnd() {
  أكد(کەڵەکە.back().Ctx == Array);
  Indent -= IndentSize;
  إذا (کەڵەکە.back().Has_بەها)
    newline();
  OS << ']';
  أكد(PendingComment.empty());
  کەڵەکە.دەرهێنان_back();
  أكد(!کەڵەکە.empty());
}

فراغ llvm::json::OStream::objectBegin() {
  بەها_Begin();
  کەڵەکە.emplace_back();
  کەڵەکە.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

فراغ llvm::json::OStream::objectEnd() {
  أكد(کەڵەکە.back().Ctx == Object);
  Indent -= IndentSize;
  إذا (کەڵەکە.back().Has_بەها)
    newline();
  OS << '}';
  أكد(PendingComment.empty());
  کەڵەکە.دەرهێنان_back();
  أكد(!کەڵەکە.empty());
}

فراغ llvm::json::OStream::attributeBegin(llvm::StringRef کلیل) {
  أكد(کەڵەکە.back().Ctx == Object);
  إذا (کەڵەکە.back().Has_بەها)
    OS << ',';
  newline();
  flushComment();
  کەڵەکە.back().Has_بەها = true;
  کەڵەکە.emplace_back();
  کەڵەکە.back().Ctx = Singleton;
  إذا (LLVM_LIKELY(isUTF8(کلیل))) {
    quote(OS, کلیل);
  } وإلا {
    أكد(خطأ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(کلیل));
  }
  OS.اكتب(':');
  إذا (IndentSize)
    OS.اكتب(' ');
}

فراغ llvm::json::OStream::attributeEnd() {
  أكد(کەڵەکە.back().Ctx == Singleton);
  أكد(کەڵەکە.back().Has_بەها && "Attribute must have a value");
  أكد(PendingComment.empty());
  کەڵەکە.دەرهێنان_back();
  أكد(کەڵەکە.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_بەها_Begin() {
  بەها_Begin();
  کەڵەکە.emplace_back();
  کەڵەکە.back().Ctx = Raw_بەها;
  أرجع OS;
}

فراغ llvm::json::OStream::raw_بەها_End() {
  أكد(کەڵەکە.back().Ctx == Raw_بەها);
  کەڵەکە.دەرهێنان_back();
}

} // namespace json
} // namespace llvm

فراغ llvm::format_provider<llvm::json::بەها>::format(
    ثابت llvm::json::بەها &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  إذا (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).بەها_2(E);
}

