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

Value &Object::operator[](ثابت ObjectKey &K) {
  أرجع try_emplace(K, nullptr).first->getSecond();
}
Value &Object::operator[](ObjectKey &&K) {
  أرجع try_emplace(std::move(K), nullptr).first->getSecond();
}
Value *Object::get(StringRef K) {
  auto I = find(K);
  𐡄𐡍 (I == end())
    أرجع nullptr;
  أرجع &I->second;
}
ثابت Value *Object::get(StringRef K) ثابت {
  auto I = find(K);
  𐡄𐡍 (I == end())
    أرجع nullptr;
  أرجع &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ثابت {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsNull();
  أرجع std::nullopt;
}
std::optional<منطقي> Object::getBoolean(StringRef K) ثابت {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsBoolean();
  أرجع std::nullopt;
}
std::optional<مزدوج> Object::getNumber(StringRef K) ثابت {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsNumber();
  أرجع std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ثابت {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsInteger();
  أرجع std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ثابت {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsString();
  أرجع std::nullopt;
}
ثابت json::Object *Object::getObject(StringRef K) ثابت {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsObject();
  أرجع nullptr;
}
json::Object *Object::getObject(StringRef K) {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsObject();
  أرجع nullptr;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsArray();
  أرجع nullptr;
}
json::Array *Object::getArray(StringRef K) {
  𐡄𐡍 (auto *V = get(K))
    أرجع V->getAsArray();
  أرجع nullptr;
}
منطقي operator==(ثابت Object &LHS, ثابت Object &RHS) { أرجع LHS.M == RHS.M; }

Array::Array(std::initializer_list<Value> Elements) {
  V.reserve(Elements.size());
  لكل (ثابت Value &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

Value::Value(std::initializer_list<Value> Elements)
    : Value(json::Array(Elements)) {}

فراغ Value::copyFrom(ثابت Value &M) {
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

فراغ Value::moveFrom(ثابت Value &&M) {
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
  const_cast<Value &>(M).destroy();
  M.Type = T_Null;
}

فراغ Value::destroy() {
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

فراغ Value::print(llvm::raw_ostream &OS) ثابت { OS << *هذا; }

#𐡄𐡍 !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ Value::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

منطقي operator==(ثابت Value &L, ثابت Value &R) {
  𐡄𐡍 (L.kind() != R.kind())
    أرجع خطأ;
  بدل (L.kind()) {
  حالة Value::Null:
    أرجع *L.getAsNull() == *R.getAsNull();
  حالة Value::Boolean:
    أرجع *L.getAsBoolean() == *R.getAsBoolean();
  حالة Value::Number:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    𐡄𐡍 (L.Type == Value::T_Integer || R.Type == Value::T_Integer)
      أرجع L.getAsInteger() == R.getAsInteger();
    أرجع *L.getAsNumber() == *R.getAsNumber();
  حالة Value::String:
    أرجع *L.getAsString() == *R.getAsString();
  حالة Value::Array:
    أرجع *L.getAsArray() == *R.getAsArray();
  حالة Value::Object:
    أرجع *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

فراغ Path::report(llvm::StringLiteral Msg) {
  // Walk up to the root context, and count the number of segments.
  unsigned Count = 0;
  ثابت Path *P;
  لكل (P = هذا; P->Parent != nullptr; P = P->Parent)
    ++Count;
  Path::Root *R = P->Seg.root();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = Msg;
  R->ErrorPath.resize(Count);
  auto It = R->ErrorPath.begin();
  لكل (P = هذا; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::Root::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  𐡄𐡍 (ErrorPath.empty()) {
    𐡄𐡍 (!Name.empty())
      OS << " when parsing " << Name;
  } وإلا {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    لكل (ثابت Path::Segment &S : llvm::reverse(ErrorPath)) {
      𐡄𐡍 (S.isField())
        OS << '.' << S.field();
      وإلا
        OS << '[' << S.index() << ']';
    }
  }
  أرجع createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ثابت Object::value_type *> sortedElements(ثابت Object &O) {
  std::vector<ثابت Object::value_type *> Elements;
  لكل (ثابت auto &E : O)
    Elements.push_back(&E);
  llvm::sort(Elements,
             [](ثابت Object::value_type *L, ثابت Object::value_type *R) {
               أرجع L->first < R->first;
             });
  أرجع Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ساكن فراغ abbreviate(ثابت Value &V, OStream &JOS) {
  بدل (V.kind()) {
  حالة Value::Array:
    JOS.rawValue(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    اكسر;
  حالة Value::Object:
    JOS.rawValue(V.getAsObject()->empty() ? "{}" : "{ ... }");
    اكسر;
  حالة Value::String: {
    llvm::StringRef S = *V.getAsString();
    𐡄𐡍 (S.size() < 40) {
      JOS.value(V);
    } وإلا {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.value(Truncated);
    }
    اكسر;
  }
  افتراضي:
    JOS.value(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ساكن فراغ abbreviateChildren(ثابت Value &V, OStream &JOS) {
  بدل (V.kind()) {
  حالة Value::Array:
    JOS.array([&] {
      لكل (ثابت auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    اكسر;
  حالة Value::Object:
    JOS.object([&] {
      لكل (ثابت auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    اكسر;
  افتراضي:
    JOS.value(V);
  }
}

فراغ Path::Root::printErrorContext(ثابت Value &R, raw_ostream &OS) ثابت {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto PrintValue = [&](ثابت Value &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    𐡄𐡍 (Path.empty()) // We reached our target.
      أرجع HighlightCurrent();
    ثابت Segment &S = Path.back(); // Path is in reverse order.
    𐡄𐡍 (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ثابت Object *O = V.getAsObject();
      𐡄𐡍 (!O || !O->get(FieldName))
        أرجع HighlightCurrent();
      JOS.object([&] {
        لكل (ثابت auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          𐡄𐡍 (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          وإلا
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } وإلا {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      𐡄𐡍 (!A || S.index() >= A->size())
        أرجع HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        لكل (ثابت auto &V : *A) {
          𐡄𐡍 (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          وإلا
            abbreviate(V, JOS);
        }
      });
    }
  };
  PrintValue(R, ErrorPath, PrintValue);
}

نطاق {
// Simple recursive-descent JSON parser.
صنف Parser {
عام:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  منطقي checkUTF8() {
    size_t ErrOffset;
    𐡄𐡍 (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      أرجع 𐡒𐡔𐡈;
    P = Start + ErrOffset; // For line/column calculation.
    أرجع parseError("Invalid UTF-8 sequence");
  }

  منطقي parseValue(Value &Out);

  منطقي assertEnd() {
    eatWhitespace();
    𐡄𐡍 (P == End)
      أرجع 𐡒𐡔𐡈;
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
  منطقي parseNumber(حرف First, Value &Out);
  منطقي parseString(std::string &Out);
  منطقي parseUnicode(std::string &Out);
  منطقي parseError(ثابت حرف *Msg); // always returns false

  حرف next() { أرجع P == End ? 0 : *P++; }
  حرف peek() { أرجع P == End ? 0 : *P; }
  ساكن منطقي isNumber(حرف C) {
    أرجع C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت حرف *Start, *P, *End;
};
} // namespace

منطقي Parser::parseValue(Value &Out) {
  eatWhitespace();
  𐡄𐡍 (P == End)
    أرجع parseError("Unexpected EOF");
  بدل (حرف C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  حالة 'n':
    Out = nullptr;
    أرجع (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالة 't':
    Out = 𐡒𐡔𐡈;
    أرجع (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالة 'f':
    Out = خطأ;
    أرجع (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالة '"': {
    std::string S;
    𐡄𐡍 (parseString(S)) {
      Out = std::move(S);
      أرجع 𐡒𐡔𐡈;
    }
    أرجع خطأ;
  }
  حالة '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    𐡄𐡍 (peek() == ']') {
      ++P;
      أرجع 𐡒𐡔𐡈;
    }
    لكل (;;) {
      A.emplace_back(nullptr);
      𐡄𐡍 (!parseValue(A.back()))
        أرجع خطأ;
      eatWhitespace();
      بدل (next()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة ']':
        أرجع 𐡒𐡔𐡈;
      افتراضي:
        أرجع parseError("Expected , or ] after array element");
      }
    }
  }
  حالة '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    𐡄𐡍 (peek() == '}') {
      ++P;
      أرجع 𐡒𐡔𐡈;
    }
    لكل (;;) {
      𐡄𐡍 (next() != '"')
        أرجع parseError("Expected object key");
      std::string K;
      𐡄𐡍 (!parseString(K))
        أرجع خطأ;
      eatWhitespace();
      𐡄𐡍 (next() != ':')
        أرجع parseError("Expected : after object key");
      eatWhitespace();
      𐡄𐡍 (!parseValue(O[std::move(K)]))
        أرجع خطأ;
      eatWhitespace();
      بدل (next()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة '}':
        أرجع 𐡒𐡔𐡈;
      افتراضي:
        أرجع parseError("Expected , or } after object property");
      }
    }
  }
  افتراضي:
    𐡄𐡍 (isNumber(C))
      أرجع parseNumber(C, Out);
    أرجع parseError("Invalid JSON value");
  }
}

منطقي Parser::parseNumber(حرف First, Value &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.push_back(First);
  طالما (isNumber(peek()))
    S.push_back(next());
  حرف *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  𐡄𐡍 (End == S.end() && errno != ERANGE) {
    Out = I;
    أرجع 𐡒𐡔𐡈;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  𐡄𐡍 (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    𐡄𐡍 (End == S.end() && errno != ERANGE) {
      Out = UI;
      أرجع 𐡒𐡔𐡈;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  أرجع End == S.end() || parseError("Invalid JSON value (number?)");
}

منطقي Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  لكل (حرف C = next(); C != '"'; C = next()) {
    𐡄𐡍 (LLVM_UNLIKELY(P == End))
      أرجع parseError("Unterminated string");
    𐡄𐡍 (LLVM_UNLIKELY((C & 0x1f) == C))
      أرجع parseError("Control character in string");
    𐡄𐡍 (LLVM_LIKELY(C != '\\')) {
      Out.push_back(C);
      استمر;
    }
    // Handle escape sequence.
    بدل (C = next()) {
    حالة '"':
    حالة '\\':
    حالة '/':
      Out.push_back(C);
      اكسر;
    حالة 'b':
      Out.push_back('\b');
      اكسر;
    حالة 'f':
      Out.push_back('\f');
      اكسر;
    حالة 'n':
      Out.push_back('\n');
      اكسر;
    حالة 'r':
      Out.push_back('\r');
      اكسر;
    حالة 't':
      Out.push_back('\t');
      اكسر;
    حالة 'u':
      𐡄𐡍 (!parseUnicode(Out))
        أرجع خطأ;
      اكسر;
    افتراضي:
      أرجع parseError("Invalid escape sequence");
    }
  }
  أرجع 𐡒𐡔𐡈;
}

ساكن فراغ encodeUtf8(uint32_t Rune, std::string &Out) {
  𐡄𐡍 (Rune < 0x80) {
    Out.push_back(Rune & 0x7F);
  } وإلا 𐡄𐡍 (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
  } وإلا 𐡄𐡍 (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
    Out.push_back(ThirdByte);
  } وإلا 𐡄𐡍 (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
    Out.push_back(ThirdByte);
    Out.push_back(FourthByte);
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
      𐡄𐡍 (!std::isxdigit(C))
        أرجع parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    أرجع 𐡒𐡔𐡈;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  𐡄𐡍 (!Parse4Hex(First))
    أرجع خطأ;

  // We loop to allow proper surrogate-pair error handling.
  طالما (𐡒𐡔𐡈) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    𐡄𐡍 (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      أرجع 𐡒𐡔𐡈;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    𐡄𐡍 (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      أرجع 𐡒𐡔𐡈;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    𐡄𐡍 (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      أرجع 𐡒𐡔𐡈;
    }
    P += 2;
    uint16_t Second;
    𐡄𐡍 (!Parse4Hex(Second))
      أرجع خطأ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    𐡄𐡍 (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      استمر;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    أرجع 𐡒𐡔𐡈;
  }
}

منطقي Parser::parseError(ثابت حرف *Msg) {
  صحيح Line = 1;
  ثابت حرف *StartOfLine = Start;
  لكل (ثابت حرف *X = Start; X < P; ++X) {
    𐡄𐡍 (*X == 0x0A) {
      ++Line;
      StartOfLine = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(Msg, Line, P - StartOfLine, P - Start));
  أرجع خطأ;
}

Expected<Value> parse(StringRef JSON) {
  Parser P(JSON);
  Value E = nullptr;
  𐡄𐡍 (P.checkUTF8())
    𐡄𐡍 (P.parseValue(E))
      𐡄𐡍 (P.assertEnd())
        أرجع std::move(E);
  أرجع P.takeError();
}

حرف ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
فراغ ParseError::log(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", Line, Column, Offset, Msg);
}

منطقي isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  𐡄𐡍 (LLVM_LIKELY(isASCII(S)))
    أرجع 𐡒𐡔𐡈;

  ثابت UTF8 *Data = reinterpret_cast<ثابت UTF8 *>(S.data()), *Rest = Data;
  𐡄𐡍 (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    أرجع 𐡒𐡔𐡈;

  𐡄𐡍 (ErrOffset)
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
    𐡄𐡍 (C == 0x22 || C == 0x5C)
      OS << '\\';
    𐡄𐡍 (C >= 0x20) {
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

فراغ llvm::json::OStream::value(ثابت Value &V) {
  بدل (V.kind()) {
  حالة Value::Null:
    valueBegin();
    OS << "null";
    أرجع;
  حالة Value::Boolean:
    valueBegin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    أرجع;
  حالة Value::Number:
    valueBegin();
    𐡄𐡍 (V.Type == Value::T_Integer)
      OS << *V.getAsInteger();
    وإلا 𐡄𐡍 (V.Type == Value::T_UINT64)
      OS << *V.getAsUINT64();
    وإلا
      OS << format("%.*g", std::numeric_limits<مزدوج>::max_digits10,
                   *V.getAsNumber());
    أرجع;
  حالة Value::String:
    valueBegin();
    quote(OS, *V.getAsString());
    أرجع;
  حالة Value::Array:
    أرجع array([&] {
      لكل (ثابت Value &E : *V.getAsArray())
        value(E);
    });
  حالة Value::Object:
    أرجع object([&] {
      لكل (ثابت Object::value_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

فراغ llvm::json::OStream::valueBegin() {
  أكد(Stack.back().Ctx != Object && "Only attributes allowed here");
  𐡄𐡍 (Stack.back().HasValue) {
    أكد(Stack.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  𐡄𐡍 (Stack.back().Ctx == Array)
    newline();
  flushComment();
  Stack.back().HasValue = 𐡒𐡔𐡈;
}

فراغ OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

فراغ OStream::flushComment() {
  𐡄𐡍 (PendingComment.empty())
    أرجع;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  طالما (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    𐡄𐡍 (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } وإلا {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  𐡄𐡍 (Stack.size() > 1 && Stack.back().Ctx == Singleton) {
    𐡄𐡍 (IndentSize)
      OS << ' ';
  } وإلا {
    newline();
  }
}

فراغ llvm::json::OStream::newline() {
  𐡄𐡍 (IndentSize) {
    OS.اكتب('\n');
    OS.indent(Indent);
  }
}

فراغ llvm::json::OStream::arrayBegin() {
  valueBegin();
  Stack.emplace_back();
  Stack.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

فراغ llvm::json::OStream::arrayEnd() {
  أكد(Stack.back().Ctx == Array);
  Indent -= IndentSize;
  𐡄𐡍 (Stack.back().HasValue)
    newline();
  OS << ']';
  أكد(PendingComment.empty());
  Stack.pop_back();
  أكد(!Stack.empty());
}

فراغ llvm::json::OStream::objectBegin() {
  valueBegin();
  Stack.emplace_back();
  Stack.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

فراغ llvm::json::OStream::objectEnd() {
  أكد(Stack.back().Ctx == Object);
  Indent -= IndentSize;
  𐡄𐡍 (Stack.back().HasValue)
    newline();
  OS << '}';
  أكد(PendingComment.empty());
  Stack.pop_back();
  أكد(!Stack.empty());
}

فراغ llvm::json::OStream::attributeBegin(llvm::StringRef Key) {
  أكد(Stack.back().Ctx == Object);
  𐡄𐡍 (Stack.back().HasValue)
    OS << ',';
  newline();
  flushComment();
  Stack.back().HasValue = 𐡒𐡔𐡈;
  Stack.emplace_back();
  Stack.back().Ctx = Singleton;
  𐡄𐡍 (LLVM_LIKELY(isUTF8(Key))) {
    quote(OS, Key);
  } وإلا {
    أكد(خطأ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(Key));
  }
  OS.اكتب(':');
  𐡄𐡍 (IndentSize)
    OS.اكتب(' ');
}

فراغ llvm::json::OStream::attributeEnd() {
  أكد(Stack.back().Ctx == Singleton);
  أكد(Stack.back().HasValue && "Attribute must have a value");
  أكد(PendingComment.empty());
  Stack.pop_back();
  أكد(Stack.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::rawValueBegin() {
  valueBegin();
  Stack.emplace_back();
  Stack.back().Ctx = RawValue;
  أرجع OS;
}

فراغ llvm::json::OStream::rawValueEnd() {
  أكد(Stack.back().Ctx == RawValue);
  Stack.pop_back();
}

} // namespace json
} // namespace llvm

فراغ llvm::format_provider<llvm::json::Value>::format(
    ثابت llvm::json::Value &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  𐡄𐡍 (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).value(E);
}

