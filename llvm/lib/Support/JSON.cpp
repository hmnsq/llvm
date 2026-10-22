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

فضاینام llvm {
فضاینام json {

مقدار &Object::operator[](ثابت Object_کلید &K) {
  بازگشت try_emplace(K, nullptr).first->getSecond();
}
مقدار &Object::operator[](Object_کلید &&K) {
  بازگشت try_emplace(std::move(K), nullptr).first->getSecond();
}
مقدار *Object::get(StringRef K) {
  auto I = find(K);
  اگر (I == end())
    بازگشت nullptr;
  بازگشت &I->second;
}
ثابت مقدار *Object::get(StringRef K) ثابت {
  auto I = find(K);
  اگر (I == end())
    بازگشت nullptr;
  بازگشت &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ثابت {
  اگر (auto *V = get(K))
    بازگشت V->getAsNull();
  بازگشت std::nullopt;
}
std::optional<منطقی> Object::getBoolean(StringRef K) ثابت {
  اگر (auto *V = get(K))
    بازگشت V->getAsBoolean();
  بازگشت std::nullopt;
}
std::optional<دوگانه> Object::get_عدد(StringRef K) ثابت {
  اگر (auto *V = get(K))
    بازگشت V->get_As_عدد();
  بازگشت std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ثابت {
  اگر (auto *V = get(K))
    بازگشت V->getAsInteger();
  بازگشت std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ثابت {
  اگر (auto *V = get(K))
    بازگشت V->getAsString();
  بازگشت std::nullopt;
}
ثابت json::Object *Object::getObject(StringRef K) ثابت {
  اگر (auto *V = get(K))
    بازگشت V->getAsObject();
  بازگشت nullptr;
}
json::Object *Object::getObject(StringRef K) {
  اگر (auto *V = get(K))
    بازگشت V->getAsObject();
  بازگشت nullptr;
}
ثابت json::Array *Object::getArray(StringRef K) ثابت {
  اگر (auto *V = get(K))
    بازگشت V->getAsArray();
  بازگشت nullptr;
}
json::Array *Object::getArray(StringRef K) {
  اگر (auto *V = get(K))
    بازگشت V->getAsArray();
  بازگشت nullptr;
}
منطقی operator==(ثابت Object &LHS, ثابت Object &RHS) { بازگشت LHS.M == RHS.M; }

Array::Array(std::initializer_list<مقدار> Elements) {
  V.reserve(Elements.size());
  برای (ثابت مقدار &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

مقدار::مقدار(std::initializer_list<مقدار> Elements)
    : مقدار(json::Array(Elements)) {}

تهی مقدار::copyFrom(ثابت مقدار &M) {
  Type = M.Type;
  انتخاب (Type) {
  حالت T_Null:
  حالت T_Boolean:
  حالت T_Double:
  حالت T_Integer:
  حالت T_UINT64:
    memcpy(&Union, &M.Union, اندازه(Union));
    بشکن;
  حالت T_StringRef:
    create<StringRef>(M.as<StringRef>());
    بشکن;
  حالت T_String:
    create<std::string>(M.as<std::string>());
    بشکن;
  حالت T_Object:
    create<json::Object>(M.as<json::Object>());
    بشکن;
  حالت T_Array:
    create<json::Array>(M.as<json::Array>());
    بشکن;
  }
}

تهی مقدار::moveFrom(ثابت مقدار &&M) {
  Type = M.Type;
  انتخاب (Type) {
  حالت T_Null:
  حالت T_Boolean:
  حالت T_Double:
  حالت T_Integer:
  حالت T_UINT64:
    memcpy(&Union, &M.Union, اندازه(Union));
    بشکن;
  حالت T_StringRef:
    create<StringRef>(M.as<StringRef>());
    بشکن;
  حالت T_String:
    create<std::string>(std::move(M.as<std::string>()));
    بشکن;
  حالت T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    بشکن;
  حالت T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    بشکن;
  }
  const_cast<مقدار &>(M).destroy();
  M.Type = T_Null;
}

تهی مقدار::destroy() {
  انتخاب (Type) {
  حالت T_Null:
  حالت T_Boolean:
  حالت T_Double:
  حالت T_Integer:
  حالت T_UINT64:
    بشکن;
  حالت T_StringRef:
    as<StringRef>().~StringRef();
    بشکن;
  حالت T_String:
    as<std::string>().~basic_string();
    بشکن;
  حالت T_Object:
    as<json::Object>().~Object();
    بشکن;
  حالت T_Array:
    as<json::Array>().~Array();
    بشکن;
  }
}

تهی مقدار::print(llvm::raw_ostream &OS) ثابت { OS << *این; }

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی مقدار::dump() ثابت {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

منطقی operator==(ثابت مقدار &L, ثابت مقدار &R) {
  اگر (L.kind() != R.kind())
    بازگشت نادرست;
  انتخاب (L.kind()) {
  حالت مقدار::Null:
    بازگشت *L.getAsNull() == *R.getAsNull();
  حالت مقدار::Boolean:
    بازگشت *L.getAsBoolean() == *R.getAsBoolean();
  حالت مقدار::عدد:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    اگر (L.Type == مقدار::T_Integer || R.Type == مقدار::T_Integer)
      بازگشت L.getAsInteger() == R.getAsInteger();
    بازگشت *L.get_As_عدد() == *R.get_As_عدد();
  حالت مقدار::String:
    بازگشت *L.getAsString() == *R.getAsString();
  حالت مقدار::Array:
    بازگشت *L.getAsArray() == *R.getAsArray();
  حالت مقدار::Object:
    بازگشت *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

تهی Path::report(llvm::StringLiteral پیام_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned شمار_2 = 0;
  ثابت Path *P;
  برای (P = این; P->Parent != nullptr; P = P->Parent)
    ++شمار_2;
  Path::ریشه_2 *R = P->Seg.ریشه_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = پیام_2;
  R->ErrorPath.resize(شمار_2);
  auto It = R->ErrorPath.begin();
  برای (P = این; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::ریشه_2::getError() ثابت {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  اگر (ErrorPath.empty()) {
    اگر (!Name.empty())
      OS << " when parsing " << Name;
  } وگرنه {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    برای (ثابت Path::Segment &S : llvm::reverse(ErrorPath)) {
      اگر (S.isField())
        OS << '.' << S.field();
      وگرنه
        OS << '[' << S.index() << ']';
    }
  }
  بازگشت createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ثابت Object::مقدار_type *> sortedElements(ثابت Object &O) {
  std::vector<ثابت Object::مقدار_type *> Elements;
  برای (ثابت auto &E : O)
    Elements.افزودن_back(&E);
  llvm::sort(Elements,
             [](ثابت Object::مقدار_type *L, ثابت Object::مقدار_type *R) {
               بازگشت L->first < R->first;
             });
  بازگشت Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ایستا تهی abbreviate(ثابت مقدار &V, OStream &JOS) {
  انتخاب (V.kind()) {
  حالت مقدار::Array:
    JOS.raw_مقدار(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    بشکن;
  حالت مقدار::Object:
    JOS.raw_مقدار(V.getAsObject()->empty() ? "{}" : "{ ... }");
    بشکن;
  حالت مقدار::String: {
    llvm::StringRef S = *V.getAsString();
    اگر (S.size() < 40) {
      JOS.مقدار_2(V);
    } وگرنه {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.مقدار_2(Truncated);
    }
    بشکن;
  }
  پیشفرض:
    JOS.مقدار_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ایستا تهی abbreviateChildren(ثابت مقدار &V, OStream &JOS) {
  انتخاب (V.kind()) {
  حالت مقدار::Array:
    JOS.array([&] {
      برای (ثابت auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    بشکن;
  حالت مقدار::Object:
    JOS.object([&] {
      برای (ثابت auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    بشکن;
  پیشفرض:
    JOS.مقدار_2(V);
  }
}

تهی Path::ریشه_2::printErrorContext(ثابت مقدار &R, raw_ostream &OS) ثابت {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_مقدار = [&](ثابت مقدار &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    اگر (Path.empty()) // We reached our target.
      بازگشت HighlightCurrent();
    ثابت Segment &S = Path.back(); // Path is in reverse order.
    اگر (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ثابت Object *O = V.getAsObject();
      اگر (!O || !O->get(FieldName))
        بازگشت HighlightCurrent();
      JOS.object([&] {
        برای (ثابت auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          اگر (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          وگرنه
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } وگرنه {
      // Current node is an array, path names an element.
      ثابت Array *A = V.getAsArray();
      اگر (!A || S.index() >= A->size())
        بازگشت HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        برای (ثابت auto &V : *A) {
          اگر (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          وگرنه
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_مقدار(R, ErrorPath, Print_مقدار);
}

فضاینام {
// Simple recursive-descent JSON parser.
رده Parser {
عمومی:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  منطقی checkUTF8() {
    size_t ErrOffset;
    اگر (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      بازگشت درست;
    P = Start + ErrOffset; // For line/column calculation.
    بازگشت parseError("Invalid UTF-8 sequence");
  }

  منطقی parse_مقدار(مقدار &Out);

  منطقی assertEnd() {
    eatWhitespace();
    اگر (P == End)
      بازگشت درست;
    بازگشت parseError("Text after end of document");
  }

  Error takeError() {
    أكد(Err);
    بازگشت std::move(*Err);
  }

خصوصی:
  تهی eatWhitespace() {
    وقتی (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  منطقی parse_عدد(نویسه First, مقدار &Out);
  منطقی parseString(std::string &Out);
  منطقی parseUnicode(std::string &Out);
  منطقی parseError(ثابت نویسه *پیام_2); // always returns false

  نویسه next() { بازگشت P == End ? 0 : *P++; }
  نویسه peek() { بازگشت P == End ? 0 : *P; }
  ایستا منطقی is_عدد(نویسه C) {
    بازگشت C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ثابت نویسه *Start, *P, *End;
};
} // namespace

منطقی Parser::parse_مقدار(مقدار &Out) {
  eatWhitespace();
  اگر (P == End)
    بازگشت parseError("Unexpected EOF");
  انتخاب (نویسه C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  حالت 'n':
    Out = nullptr;
    بازگشت (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  حالت 't':
    Out = درست;
    بازگشت (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  حالت 'f':
    Out = نادرست;
    بازگشت (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  حالت '"': {
    std::string S;
    اگر (parseString(S)) {
      Out = std::move(S);
      بازگشت درست;
    }
    بازگشت نادرست;
  }
  حالت '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    اگر (peek() == ']') {
      ++P;
      بازگشت درست;
    }
    برای (;;) {
      A.emplace_back(nullptr);
      اگر (!parse_مقدار(A.back()))
        بازگشت نادرست;
      eatWhitespace();
      انتخاب (next()) {
      حالت ',':
        eatWhitespace();
        ادامه;
      حالت ']':
        بازگشت درست;
      پیشفرض:
        بازگشت parseError("Expected , or ] after array element");
      }
    }
  }
  حالت '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    اگر (peek() == '}') {
      ++P;
      بازگشت درست;
    }
    برای (;;) {
      اگر (next() != '"')
        بازگشت parseError("Expected object key");
      std::string K;
      اگر (!parseString(K))
        بازگشت نادرست;
      eatWhitespace();
      اگر (next() != ':')
        بازگشت parseError("Expected : after object key");
      eatWhitespace();
      اگر (!parse_مقدار(O[std::move(K)]))
        بازگشت نادرست;
      eatWhitespace();
      انتخاب (next()) {
      حالت ',':
        eatWhitespace();
        ادامه;
      حالت '}':
        بازگشت درست;
      پیشفرض:
        بازگشت parseError("Expected , or } after object property");
      }
    }
  }
  پیشفرض:
    اگر (is_عدد(C))
      بازگشت parse_عدد(C, Out);
    بازگشت parseError("Invalid JSON value");
  }
}

منطقی Parser::parse_عدد(نویسه First, مقدار &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.افزودن_back(First);
  وقتی (is_عدد(peek()))
    S.افزودن_back(next());
  نویسه *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  اگر (End == S.end() && errno != ERANGE) {
    Out = I;
    بازگشت درست;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  اگر (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    اگر (End == S.end() && errno != ERANGE) {
      Out = UI;
      بازگشت درست;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  بازگشت End == S.end() || parseError("Invalid JSON value (number?)");
}

منطقی Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  برای (نویسه C = next(); C != '"'; C = next()) {
    اگر (LLVM_UNLIKELY(P == End))
      بازگشت parseError("Unterminated string");
    اگر (LLVM_UNLIKELY((C & 0x1f) == C))
      بازگشت parseError("Control character in string");
    اگر (LLVM_LIKELY(C != '\\')) {
      Out.افزودن_back(C);
      ادامه;
    }
    // Handle escape sequence.
    انتخاب (C = next()) {
    حالت '"':
    حالت '\\':
    حالت '/':
      Out.افزودن_back(C);
      بشکن;
    حالت 'b':
      Out.افزودن_back('\b');
      بشکن;
    حالت 'f':
      Out.افزودن_back('\f');
      بشکن;
    حالت 'n':
      Out.افزودن_back('\n');
      بشکن;
    حالت 'r':
      Out.افزودن_back('\r');
      بشکن;
    حالت 't':
      Out.افزودن_back('\t');
      بشکن;
    حالت 'u':
      اگر (!parseUnicode(Out))
        بازگشت نادرست;
      بشکن;
    پیشفرض:
      بازگشت parseError("Invalid escape sequence");
    }
  }
  بازگشت درست;
}

ایستا تهی encodeUtf8(uint32_t Rune, std::string &Out) {
  اگر (Rune < 0x80) {
    Out.افزودن_back(Rune & 0x7F);
  } وگرنه اگر (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.افزودن_back(FirstByte);
    Out.افزودن_back(SecondByte);
  } وگرنه اگر (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.افزودن_back(FirstByte);
    Out.افزودن_back(SecondByte);
    Out.افزودن_back(ThirdByte);
  } وگرنه اگر (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.افزودن_back(FirstByte);
    Out.افزودن_back(SecondByte);
    Out.افزودن_back(ThirdByte);
    Out.افزودن_back(FourthByte);
  } وگرنه {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
منطقی Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [این](uint16_t &Out) -> منطقی {
    Out = 0;
    نویسه Bytes[] = {next(), next(), next(), next()};
    برای (unsigned نویسه C : Bytes) {
      اگر (!std::isxdigit(C))
        بازگشت parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    بازگشت درست;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  اگر (!Parse4Hex(First))
    بازگشت نادرست;

  // We loop to allow proper surrogate-pair error handling.
  وقتی (درست) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    اگر (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      بازگشت درست;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    اگر (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      بازگشت درست;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    اگر (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      بازگشت درست;
    }
    P += 2;
    uint16_t Second;
    اگر (!Parse4Hex(Second))
      بازگشت نادرست;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    اگر (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      ادامه;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    بازگشت درست;
  }
}

منطقی Parser::parseError(ثابت نویسه *پیام_2) {
  صحیح سطر = 1;
  ثابت نویسه *Start_Of_سطر = Start;
  برای (ثابت نویسه *X = Start; X < P; ++X) {
    اگر (*X == 0x0A) {
      ++سطر;
      Start_Of_سطر = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(پیام_2, سطر, P - Start_Of_سطر, P - Start));
  بازگشت نادرست;
}

Expected<مقدار> parse(StringRef JSON) {
  Parser P(JSON);
  مقدار E = nullptr;
  اگر (P.checkUTF8())
    اگر (P.parse_مقدار(E))
      اگر (P.assertEnd())
        بازگشت std::move(E);
  بازگشت P.takeError();
}

نویسه ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
تهی ParseError::log(llvm::raw_ostream &OS) ثابت {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", سطر, Column, Offset, پیام_2);
}

منطقی isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  اگر (LLVM_LIKELY(isASCII(S)))
    بازگشت درست;

  ثابت UTF8 *Data = reinterpret_cast<ثابت UTF8 *>(S.data()), *Rest = Data;
  اگر (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    بازگشت درست;

  اگر (ErrOffset)
    *ErrOffset = Rest - Data;
  بازگشت نادرست;
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
  Res.resize(reinterpret_cast<نویسه *>(Out8) - Res.data());
  بازگشت Res;
}

ایستا تهی quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  برای (unsigned نویسه C : S) {
    اگر (C == 0x22 || C == 0x5C)
      OS << '\\';
    اگر (C >= 0x20) {
      OS << C;
      ادامه;
    }
    OS << '\\';
    انتخاب (C) {
    // A few characters are common enough to make short escapes worthwhile.
    حالت '\t':
      OS << 't';
      بشکن;
    حالت '\n':
      OS << 'n';
      بشکن;
    حالت '\r':
      OS << 'r';
      بشکن;
    پیشفرض:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      بشکن;
    }
  }
  OS << '\"';
}

تهی llvm::json::OStream::مقدار_2(ثابت مقدار &V) {
  انتخاب (V.kind()) {
  حالت مقدار::Null:
    مقدار_Begin();
    OS << "null";
    بازگشت;
  حالت مقدار::Boolean:
    مقدار_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    بازگشت;
  حالت مقدار::عدد:
    مقدار_Begin();
    اگر (V.Type == مقدار::T_Integer)
      OS << *V.getAsInteger();
    وگرنه اگر (V.Type == مقدار::T_UINT64)
      OS << *V.getAsUINT64();
    وگرنه
      OS << format("%.*g", std::numeric_limits<دوگانه>::max_digits10,
                   *V.get_As_عدد());
    بازگشت;
  حالت مقدار::String:
    مقدار_Begin();
    quote(OS, *V.getAsString());
    بازگشت;
  حالت مقدار::Array:
    بازگشت array([&] {
      برای (ثابت مقدار &E : *V.getAsArray())
        مقدار_2(E);
    });
  حالت مقدار::Object:
    بازگشت object([&] {
      برای (ثابت Object::مقدار_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

تهی llvm::json::OStream::مقدار_Begin() {
  أكد(پشته.back().Ctx != Object && "Only attributes allowed here");
  اگر (پشته.back().Has_مقدار) {
    أكد(پشته.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  اگر (پشته.back().Ctx == Array)
    newline();
  flushComment();
  پشته.back().Has_مقدار = درست;
}

تهی OStream::comment(llvm::StringRef Comment) {
  أكد(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

تهی OStream::flushComment() {
  اگر (PendingComment.empty())
    بازگشت;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  وقتی (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    اگر (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } وگرنه {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  اگر (پشته.size() > 1 && پشته.back().Ctx == Singleton) {
    اگر (IndentSize)
      OS << ' ';
  } وگرنه {
    newline();
  }
}

تهی llvm::json::OStream::newline() {
  اگر (IndentSize) {
    OS.اكتب('\n');
    OS.indent(Indent);
  }
}

تهی llvm::json::OStream::arrayBegin() {
  مقدار_Begin();
  پشته.emplace_back();
  پشته.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

تهی llvm::json::OStream::arrayEnd() {
  أكد(پشته.back().Ctx == Array);
  Indent -= IndentSize;
  اگر (پشته.back().Has_مقدار)
    newline();
  OS << ']';
  أكد(PendingComment.empty());
  پشته.برداشت_back();
  أكد(!پشته.empty());
}

تهی llvm::json::OStream::objectBegin() {
  مقدار_Begin();
  پشته.emplace_back();
  پشته.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

تهی llvm::json::OStream::objectEnd() {
  أكد(پشته.back().Ctx == Object);
  Indent -= IndentSize;
  اگر (پشته.back().Has_مقدار)
    newline();
  OS << '}';
  أكد(PendingComment.empty());
  پشته.برداشت_back();
  أكد(!پشته.empty());
}

تهی llvm::json::OStream::attributeBegin(llvm::StringRef کلید) {
  أكد(پشته.back().Ctx == Object);
  اگر (پشته.back().Has_مقدار)
    OS << ',';
  newline();
  flushComment();
  پشته.back().Has_مقدار = درست;
  پشته.emplace_back();
  پشته.back().Ctx = Singleton;
  اگر (LLVM_LIKELY(isUTF8(کلید))) {
    quote(OS, کلید);
  } وگرنه {
    أكد(نادرست && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(کلید));
  }
  OS.اكتب(':');
  اگر (IndentSize)
    OS.اكتب(' ');
}

تهی llvm::json::OStream::attributeEnd() {
  أكد(پشته.back().Ctx == Singleton);
  أكد(پشته.back().Has_مقدار && "Attribute must have a value");
  أكد(PendingComment.empty());
  پشته.برداشت_back();
  أكد(پشته.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_مقدار_Begin() {
  مقدار_Begin();
  پشته.emplace_back();
  پشته.back().Ctx = Raw_مقدار;
  بازگشت OS;
}

تهی llvm::json::OStream::raw_مقدار_End() {
  أكد(پشته.back().Ctx == Raw_مقدار);
  پشته.برداشت_back();
}

} // namespace json
} // namespace llvm

تهی llvm::format_provider<llvm::json::مقدار>::format(
    ثابت llvm::json::مقدار &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  اگر (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).مقدار_2(E);
}

