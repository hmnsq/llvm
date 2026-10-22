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
  החזר try_emplace(K, nullptr).first->getSecond();
}
Value &Object::operator[](ObjectKey &&K) {
  החזר try_emplace(std::move(K), nullptr).first->getSecond();
}
Value *Object::get(StringRef K) {
  auto I = find(K);
  אם (I == end())
    החזר nullptr;
  החזר &I->second;
}
ثابت Value *Object::get(StringRef K) ثابت {
  auto I = find(K);
  אם (I == end())
    החזר nullptr;
  החזר &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ثابت {
  אם (auto *V = get(K))
    החזר V->getAsNull();
  החזר std::nullopt;
}
std::optional<منطقي> Object::getBoolean(StringRef K) ثابت {
  אם (auto *V = get(K))
    החזר V->getAsBoolean();
  החזר std::nullopt;
}
std::optional<مزدوج> Object::getNumber(StringRef K) ثابت {
  אם (auto *V = get(K))
    החזר V->getAsNumber();
  החזר std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ثابت {
  אם (auto *V = get(K))
    החזר V->getAsInteger();
  החזר std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ثابت {
  אם (auto *V = get(K))
    החזר V->getAsString();
  החזר std::nullopt;
}
ثابت json::Object *Object::getObject(StringRef K) ثابت {
  אם (auto *V = get(K))
    החזר V->getAsObject();
  החזר nullptr;
}
json::Object *Object::getObject(StringRef K) {
  אם (auto *V = get(K))
    החזר V->getAsObject();
  החזר nullptr;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  אם (auto *V = get(K))
    החזר V->getAsArray();
  החזר nullptr;
}
json::Array *Object::getArray(StringRef K) {
  אם (auto *V = get(K))
    החזר V->getAsArray();
  החזר nullptr;
}
منطقي operator==(ثابت Object &LHS, ثابت Object &RHS) { החזר LHS.M == RHS.M; }

Array::Array(std::initializer_list<Value> Elements) {
  V.reserve(Elements.size());
  עבור (ثابت Value &V : Elements) {
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

#אם !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ Value::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

منطقي operator==(ثابت Value &L, ثابت Value &R) {
  אם (L.kind() != R.kind())
    החזר שקר;
  بدل (L.kind()) {
  حالة Value::Null:
    החזר *L.getAsNull() == *R.getAsNull();
  حالة Value::Boolean:
    החזר *L.getAsBoolean() == *R.getAsBoolean();
  حالة Value::Number:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    אם (L.Type == Value::T_Integer || R.Type == Value::T_Integer)
      החזר L.getAsInteger() == R.getAsInteger();
    החזר *L.getAsNumber() == *R.getAsNumber();
  حالة Value::String:
    החזר *L.getAsString() == *R.getAsString();
  حالة Value::Array:
    החזר *L.getAsArray() == *R.getAsArray();
  حالة Value::Object:
    החזר *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

فراغ Path::report(llvm::StringLiteral Msg) {
  // Walk up to the root context, and count the number of segments.
  unsigned Count = 0;
  ثابت Path *P;
  עבור (P = هذا; P->Parent != nullptr; P = P->Parent)
    ++Count;
  Path::Root *R = P->Seg.root();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = Msg;
  R->ErrorPath.resize(Count);
  auto It = R->ErrorPath.begin();
  עבור (P = هذا; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::Root::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  אם (ErrorPath.empty()) {
    אם (!Name.empty())
      OS << " when parsing " << Name;
  } אחרת {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    עבור (ثابت Path::Segment &S : llvm::reverse(ErrorPath)) {
      אם (S.isField())
        OS << '.' << S.field();
      אחרת
        OS << '[' << S.index() << ']';
    }
  }
  החזר createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ثابت Object::value_type *> sortedElements(ثابت Object &O) {
  std::vector<ثابت Object::value_type *> Elements;
  עבור (ثابت auto &E : O)
    Elements.push_back(&E);
  llvm::sort(Elements,
             [](ثابت Object::value_type *L, ثابت Object::value_type *R) {
               החזר L->first < R->first;
             });
  החזר Elements;
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
    אם (S.size() < 40) {
      JOS.value(V);
    } אחרת {
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
      עבור (ثابت auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    اكسر;
  حالة Value::Object:
    JOS.object([&] {
      עבור (ثابت auto *KV : sortedElements(*V.getAsObject())) {
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
    אם (Path.empty()) // We reached our target.
      החזר HighlightCurrent();
    ثابت Segment &S = Path.back(); // Path is in reverse order.
    אם (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ثابت Object *O = V.getAsObject();
      אם (!O || !O->get(FieldName))
        החזר HighlightCurrent();
      JOS.object([&] {
        עבור (ثابت auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          אם (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          אחרת
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } אחרת {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      אם (!A || S.index() >= A->size())
        החזר HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        עבור (ثابت auto &V : *A) {
          אם (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          אחרת
            abbreviate(V, JOS);
        }
      });
    }
  };
  PrintValue(R, ErrorPath, PrintValue);
}

نطاق {
// Simple recursive-descent JSON parser.
מחלקה Parser {
عام:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  منطقي checkUTF8() {
    size_t ErrOffset;
    אם (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      החזר אמת;
    P = Start + ErrOffset; // For line/column calculation.
    החזר parseError("Invalid UTF-8 sequence");
  }

  منطقي parseValue(Value &Out);

  منطقي assertEnd() {
    eatWhitespace();
    אם (P == End)
      החזר אמת;
    החזר parseError("Text after end of document");
  }

  Error takeError() {
    أكد(Err);
    החזר std::move(*Err);
  }

خاص:
  فراغ eatWhitespace() {
    כלעוד (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  منطقي parseNumber(حرف First, Value &Out);
  منطقي parseString(std::string &Out);
  منطقي parseUnicode(std::string &Out);
  منطقي parseError(ثابت حرف *Msg); // always returns false

  حرف next() { החזר P == End ? 0 : *P++; }
  حرف peek() { החזר P == End ? 0 : *P; }
  ساكن منطقي isNumber(حرف C) {
    החזר C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت حرف *Start, *P, *End;
};
} // namespace

منطقي Parser::parseValue(Value &Out) {
  eatWhitespace();
  אם (P == End)
    החזר parseError("Unexpected EOF");
  بدل (حرف C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  حالة 'n':
    Out = nullptr;
    החזר (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالة 't':
    Out = אמת;
    החזר (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالة 'f':
    Out = שקר;
    החזר (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالة '"': {
    std::string S;
    אם (parseString(S)) {
      Out = std::move(S);
      החזר אמת;
    }
    החזר שקר;
  }
  حالة '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    אם (peek() == ']') {
      ++P;
      החזר אמת;
    }
    עבור (;;) {
      A.emplace_back(nullptr);
      אם (!parseValue(A.back()))
        החזר שקר;
      eatWhitespace();
      بدل (next()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة ']':
        החזר אמת;
      افتراضي:
        החזר parseError("Expected , or ] after array element");
      }
    }
  }
  حالة '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    אם (peek() == '}') {
      ++P;
      החזר אמת;
    }
    עבור (;;) {
      אם (next() != '"')
        החזר parseError("Expected object key");
      std::string K;
      אם (!parseString(K))
        החזר שקר;
      eatWhitespace();
      אם (next() != ':')
        החזר parseError("Expected : after object key");
      eatWhitespace();
      אם (!parseValue(O[std::move(K)]))
        החזר שקר;
      eatWhitespace();
      بدل (next()) {
      حالة ',':
        eatWhitespace();
        استمر;
      حالة '}':
        החזר אמת;
      افتراضي:
        החזר parseError("Expected , or } after object property");
      }
    }
  }
  افتراضي:
    אם (isNumber(C))
      החזר parseNumber(C, Out);
    החזר parseError("Invalid JSON value");
  }
}

منطقي Parser::parseNumber(حرف First, Value &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.push_back(First);
  כלעוד (isNumber(peek()))
    S.push_back(next());
  حرف *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  אם (End == S.end() && errno != ERANGE) {
    Out = I;
    החזר אמת;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  אם (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    אם (End == S.end() && errno != ERANGE) {
      Out = UI;
      החזר אמת;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  החזר End == S.end() || parseError("Invalid JSON value (number?)");
}

منطقي Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  עבור (حرف C = next(); C != '"'; C = next()) {
    אם (LLVM_UNLIKELY(P == End))
      החזר parseError("Unterminated string");
    אם (LLVM_UNLIKELY((C & 0x1f) == C))
      החזר parseError("Control character in string");
    אם (LLVM_LIKELY(C != '\\')) {
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
      אם (!parseUnicode(Out))
        החזר שקר;
      اكسر;
    افتراضي:
      החזר parseError("Invalid escape sequence");
    }
  }
  החזר אמת;
}

ساكن فراغ encodeUtf8(uint32_t Rune, std::string &Out) {
  אם (Rune < 0x80) {
    Out.push_back(Rune & 0x7F);
  } אחרת אם (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
  } אחרת אם (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
    Out.push_back(ThirdByte);
  } אחרת אם (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
    Out.push_back(ThirdByte);
    Out.push_back(FourthByte);
  } אחרת {
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
    עבור (unsigned حرف C : Bytes) {
      אם (!std::isxdigit(C))
        החזר parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    החזר אמת;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  אם (!Parse4Hex(First))
    החזר שקר;

  // We loop to allow proper surrogate-pair error handling.
  כלעוד (אמת) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    אם (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      החזר אמת;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    אם (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      החזר אמת;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    אם (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      החזר אמת;
    }
    P += 2;
    uint16_t Second;
    אם (!Parse4Hex(Second))
      החזר שקר;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    אם (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      استمر;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    החזר אמת;
  }
}

منطقي Parser::parseError(ثابت حرف *Msg) {
  صحيح Line = 1;
  ثابت حرف *StartOfLine = Start;
  עבור (ثابت حرف *X = Start; X < P; ++X) {
    אם (*X == 0x0A) {
      ++Line;
      StartOfLine = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(Msg, Line, P - StartOfLine, P - Start));
  החזר שקר;
}

Expected<Value> parse(StringRef JSON) {
  Parser P(JSON);
  Value E = nullptr;
  אם (P.checkUTF8())
    אם (P.parseValue(E))
      אם (P.assertEnd())
        החזר std::move(E);
  החזר P.takeError();
}

حرف ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
فراغ ParseError::log(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", Line, Column, Offset, Msg);
}

منطقي isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  אם (LLVM_LIKELY(isASCII(S)))
    החזר אמת;

  ثابت UTF8 *Data = reinterpret_cast<ثابت UTF8 *>(S.data()), *Rest = Data;
  אם (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    החזר אמת;

  אם (ErrOffset)
    *ErrOffset = Rest - Data;
  החזר שקר;
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
  החזר Res;
}

ساكن فراغ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  עבור (unsigned حرف C : S) {
    אם (C == 0x22 || C == 0x5C)
      OS << '\\';
    אם (C >= 0x20) {
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
    החזר;
  حالة Value::Boolean:
    valueBegin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    החזר;
  حالة Value::Number:
    valueBegin();
    אם (V.Type == Value::T_Integer)
      OS << *V.getAsInteger();
    אחרת אם (V.Type == Value::T_UINT64)
      OS << *V.getAsUINT64();
    אחרת
      OS << format("%.*g", std::numeric_limits<مزدوج>::max_digits10,
                   *V.getAsNumber());
    החזר;
  حالة Value::String:
    valueBegin();
    quote(OS, *V.getAsString());
    החזר;
  حالة Value::Array:
    החזר array([&] {
      עבור (ثابت Value &E : *V.getAsArray())
        value(E);
    });
  حالة Value::Object:
    החזר object([&] {
      עבור (ثابت Object::value_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

فراغ llvm::json::OStream::valueBegin() {
  أكد(Stack.back().Ctx != Object && "Only attributes allowed here");
  אם (Stack.back().HasValue) {
    أكد(Stack.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  אם (Stack.back().Ctx == Array)
    newline();
  flushComment();
  Stack.back().HasValue = אמת;
}

فراغ OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

فراغ OStream::flushComment() {
  אם (PendingComment.empty())
    החזר;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  כלעוד (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    אם (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } אחרת {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  אם (Stack.size() > 1 && Stack.back().Ctx == Singleton) {
    אם (IndentSize)
      OS << ' ';
  } אחרת {
    newline();
  }
}

فراغ llvm::json::OStream::newline() {
  אם (IndentSize) {
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
  אם (Stack.back().HasValue)
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
  אם (Stack.back().HasValue)
    newline();
  OS << '}';
  أكد(PendingComment.empty());
  Stack.pop_back();
  أكد(!Stack.empty());
}

فراغ llvm::json::OStream::attributeBegin(llvm::StringRef Key) {
  أكد(Stack.back().Ctx == Object);
  אם (Stack.back().HasValue)
    OS << ',';
  newline();
  flushComment();
  Stack.back().HasValue = אמת;
  Stack.emplace_back();
  Stack.back().Ctx = Singleton;
  אם (LLVM_LIKELY(isUTF8(Key))) {
    quote(OS, Key);
  } אחרת {
    أكد(שקר && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(Key));
  }
  OS.اكتب(':');
  אם (IndentSize)
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
  החזר OS;
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
  אם (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).value(E);
}

