//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#include "llvm/Support/JSON.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/NativeFormatting.h"
#include "llvm/Support/raw_ostream.h"
#include <cctype>
#include <cerrno>
#include <optional>

namespace llvm {
namespace json {

Value &Object::operator[](const ObjectKey &K) {
  ត្រឡប់ try_emplace(K, nullptr).first->getSecond();
}
Value &Object::operator[](ObjectKey &&K) {
  ត្រឡប់ try_emplace(std::move(K), nullptr).first->getSecond();
}
Value *Object::get(StringRef K) {
  auto I = find(K);
  ប្រសិនបើ (I == end())
    ត្រឡប់ nullptr;
  ត្រឡប់ &I->second;
}
const Value *Object::get(StringRef K) const {
  auto I = find(K);
  ប្រសិនបើ (I == end())
    ត្រឡប់ nullptr;
  ត្រឡប់ &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) const {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsNull();
  ត្រឡប់ std::nullopt;
}
std::optional<bool> Object::getBoolean(StringRef K) const {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsBoolean();
  ត្រឡប់ std::nullopt;
}
std::optional<dviguu_na> Object::getNumber(StringRef K) const {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsNumber();
  ត្រឡប់ std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) const {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsInteger();
  ត្រឡប់ std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) const {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsString();
  ត្រឡប់ std::nullopt;
}
const json::Object *Object::getObject(StringRef K) const {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsObject();
  ត្រឡប់ nullptr;
}
json::Object *Object::getObject(StringRef K) {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsObject();
  ត្រឡប់ nullptr;
}
const json::Array *Object::getArray(StringRef K) const {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsArray();
  ត្រឡប់ nullptr;
}
json::Array *Object::getArray(StringRef K) {
  ប្រសិនបើ (auto *V = get(K))
    ត្រឡប់ V->getAsArray();
  ត្រឡប់ nullptr;
}
bool operator==(const Object &LHS, const Object &RHS) { ត្រឡប់ LHS.M == RHS.M; }

Array::Array(std::initializer_list<Value> Elements) {
  V.reserve(Elements.size());
  សម្រាប់ (const Value &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

Value::Value(std::initializer_list<Value> Elements)
    : Value(json::Array(Elements)) {}

void Value::copyFrom(const Value &M) {
  Type = M.Type;
  switch (Type) {
  pak_sa T_Null:
  pak_sa T_Boolean:
  pak_sa T_Double:
  pak_sa T_Integer:
  pak_sa T_UINT64:
    memcpy(&Union, &M.Union, parimaa_na(Union));
    bha_nga;
  pak_sa T_StringRef:
    create<StringRef>(M.as<StringRef>());
    bha_nga;
  pak_sa T_String:
    create<std::string>(M.as<std::string>());
    bha_nga;
  pak_sa T_Object:
    create<json::Object>(M.as<json::Object>());
    bha_nga;
  pak_sa T_Array:
    create<json::Array>(M.as<json::Array>());
    bha_nga;
  }
}

void Value::moveFrom(const Value &&M) {
  Type = M.Type;
  switch (Type) {
  pak_sa T_Null:
  pak_sa T_Boolean:
  pak_sa T_Double:
  pak_sa T_Integer:
  pak_sa T_UINT64:
    memcpy(&Union, &M.Union, parimaa_na(Union));
    bha_nga;
  pak_sa T_StringRef:
    create<StringRef>(M.as<StringRef>());
    bha_nga;
  pak_sa T_String:
    create<std::string>(std::move(M.as<std::string>()));
    bha_nga;
  pak_sa T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    bha_nga;
  pak_sa T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    bha_nga;
  }
  const_cast<Value &>(M).destroy();
  M.Type = T_Null;
}

void Value::destroy() {
  switch (Type) {
  pak_sa T_Null:
  pak_sa T_Boolean:
  pak_sa T_Double:
  pak_sa T_Integer:
  pak_sa T_UINT64:
    bha_nga;
  pak_sa T_StringRef:
    as<StringRef>().~StringRef();
    bha_nga;
  pak_sa T_String:
    as<std::string>().~basic_string();
    bha_nga;
  pak_sa T_Object:
    as<json::Object>().~Object();
    bha_nga;
  pak_sa T_Array:
    as<json::Array>().~Array();
    bha_nga;
  }
}

void Value::print(llvm::raw_ostream &OS) const { OS << *this; }

#ប្រសិនបើ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void Value::dump() const {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

bool operator==(const Value &L, const Value &R) {
  ប្រសិនបើ (L.kind() != R.kind())
    ត្រឡប់ false;
  switch (L.kind()) {
  pak_sa Value::Null:
    ត្រឡប់ *L.getAsNull() == *R.getAsNull();
  pak_sa Value::Boolean:
    ត្រឡប់ *L.getAsBoolean() == *R.getAsBoolean();
  pak_sa Value::Number:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    ប្រសិនបើ (L.Type == Value::T_Integer || R.Type == Value::T_Integer)
      ត្រឡប់ L.getAsInteger() == R.getAsInteger();
    ត្រឡប់ *L.getAsNumber() == *R.getAsNumber();
  pak_sa Value::String:
    ត្រឡប់ *L.getAsString() == *R.getAsString();
  pak_sa Value::Array:
    ត្រឡប់ *L.getAsArray() == *R.getAsArray();
  pak_sa Value::Object:
    ត្រឡប់ *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

void Path::report(llvm::StringLiteral Msg) {
  // Walk up to the root context, and count the number of segments.
  unsigned Count = 0;
  const Path *P;
  សម្រាប់ (P = this; P->Parent != nullptr; P = P->Parent)
    ++Count;
  Path::Root *R = P->Seg.root();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = Msg;
  R->ErrorPath.resize(Count);
  auto It = R->ErrorPath.begin();
  សម្រាប់ (P = this; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::Root::getError() const {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  ប្រសិនបើ (ErrorPath.empty()) {
    ប្រសិនបើ (!Name.empty())
      OS << " when parsing " << Name;
  } បើមិនដូច្នេះ {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    សម្រាប់ (const Path::Segment &S : llvm::reverse(ErrorPath)) {
      ប្រសិនបើ (S.isField())
        OS << '.' << S.field();
      បើមិនដូច្នេះ
        OS << '[' << S.index() << ']';
    }
  }
  ត្រឡប់ createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<const Object::value_type *> sortedElements(const Object &O) {
  std::vector<const Object::value_type *> Elements;
  សម្រាប់ (const auto &E : O)
    Elements.push_back(&E);
  llvm::sort(Elements,
             [](const Object::value_type *L, const Object::value_type *R) {
               ត្រឡប់ L->first < R->first;
             });
  ត្រឡប់ Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
static void abbreviate(const Value &V, OStream &JOS) {
  switch (V.kind()) {
  pak_sa Value::Array:
    JOS.rawValue(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    bha_nga;
  pak_sa Value::Object:
    JOS.rawValue(V.getAsObject()->empty() ? "{}" : "{ ... }");
    bha_nga;
  pak_sa Value::String: {
    llvm::StringRef S = *V.getAsString();
    ប្រសិនបើ (S.size() < 40) {
      JOS.value(V);
    } បើមិនដូច្នេះ {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.value(Truncated);
    }
    bha_nga;
  }
  default:
    JOS.value(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
static void abbreviateChildren(const Value &V, OStream &JOS) {
  switch (V.kind()) {
  pak_sa Value::Array:
    JOS.array([&] {
      សម្រាប់ (const auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    bha_nga;
  pak_sa Value::Object:
    JOS.object([&] {
      សម្រាប់ (const auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    bha_nga;
  default:
    JOS.value(V);
  }
}

void Path::Root::printErrorContext(const Value &R, raw_ostream &OS) const {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto PrintValue = [&](const Value &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    ប្រសិនបើ (Path.empty()) // We reached our target.
      ត្រឡប់ HighlightCurrent();
    const Segment &S = Path.back(); // Path is in reverse order.
    ប្រសិនបើ (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      const Object *O = V.getAsObject();
      ប្រសិនបើ (!O || !O->get(FieldName))
        ត្រឡប់ HighlightCurrent();
      JOS.object([&] {
        សម្រាប់ (const auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          ប្រសិនបើ (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          បើមិនដូច្នេះ
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } បើមិនដូច្នេះ {
      // Current node is an array, path names an element.
      const Array *A = V.getAsArray();
      ប្រសិនបើ (!A || S.index() >= A->size())
        ត្រឡប់ HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        សម្រាប់ (const auto &V : *A) {
          ប្រសិនបើ (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          បើមិនដូច្នេះ
            abbreviate(V, JOS);
        }
      });
    }
  };
  PrintValue(R, ErrorPath, PrintValue);
}

namespace {
// Simple recursive-descent JSON parser.
class Parser {
public:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  bool checkUTF8() {
    size_t ErrOffset;
    ប្រសិនបើ (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      ត្រឡប់ true;
    P = Start + ErrOffset; // For line/column calculation.
    ត្រឡប់ parseError("Invalid UTF-8 sequence");
  }

  bool parseValue(Value &Out);

  bool assertEnd() {
    eatWhitespace();
    ប្រសិនបើ (P == End)
      ត្រឡប់ true;
    ត្រឡប់ parseError("Text after end of document");
  }

  Error takeError() {
    assert(Err);
    ត្រឡប់ std::move(*Err);
  }

private:
  void eatWhitespace() {
    ខណៈពេល (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  bool parseNumber(var_na First, Value &Out);
  bool parseString(std::string &Out);
  bool parseUnicode(std::string &Out);
  bool parseError(const var_na *Msg); // always returns false

  var_na next() { ត្រឡប់ P == End ? 0 : *P++; }
  var_na peek() { ត្រឡប់ P == End ? 0 : *P; }
  static bool isNumber(var_na C) {
    ត្រឡប់ C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  const var_na *Start, *P, *End;
};
} // namespace

bool Parser::parseValue(Value &Out) {
  eatWhitespace();
  ប្រសិនបើ (P == End)
    ត្រឡប់ parseError("Unexpected EOF");
  switch (var_na C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  pak_sa 'n':
    Out = nullptr;
    ត្រឡប់ (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  pak_sa 't':
    Out = true;
    ត្រឡប់ (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  pak_sa 'f':
    Out = false;
    ត្រឡប់ (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  pak_sa '"': {
    std::string S;
    ប្រសិនបើ (parseString(S)) {
      Out = std::move(S);
      ត្រឡប់ true;
    }
    ត្រឡប់ false;
  }
  pak_sa '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    ប្រសិនបើ (peek() == ']') {
      ++P;
      ត្រឡប់ true;
    }
    សម្រាប់ (;;) {
      A.emplace_back(nullptr);
      ប្រសិនបើ (!parseValue(A.back()))
        ត្រឡប់ false;
      eatWhitespace();
      switch (next()) {
      pak_sa ',':
        eatWhitespace();
        continue;
      pak_sa ']':
        ត្រឡប់ true;
      default:
        ត្រឡប់ parseError("Expected , or ] after array element");
      }
    }
  }
  pak_sa '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    ប្រសិនបើ (peek() == '}') {
      ++P;
      ត្រឡប់ true;
    }
    សម្រាប់ (;;) {
      ប្រសិនបើ (next() != '"')
        ត្រឡប់ parseError("Expected object key");
      std::string K;
      ប្រសិនបើ (!parseString(K))
        ត្រឡប់ false;
      eatWhitespace();
      ប្រសិនបើ (next() != ':')
        ត្រឡប់ parseError("Expected : after object key");
      eatWhitespace();
      ប្រសិនបើ (!parseValue(O[std::move(K)]))
        ត្រឡប់ false;
      eatWhitespace();
      switch (next()) {
      pak_sa ',':
        eatWhitespace();
        continue;
      pak_sa '}':
        ត្រឡប់ true;
      default:
        ត្រឡប់ parseError("Expected , or } after object property");
      }
    }
  }
  default:
    ប្រសិនបើ (isNumber(C))
      ត្រឡប់ parseNumber(C, Out);
    ត្រឡប់ parseError("Invalid JSON value");
  }
}

bool Parser::parseNumber(var_na First, Value &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.push_back(First);
  ខណៈពេល (isNumber(peek()))
    S.push_back(next());
  var_na *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  ប្រសិនបើ (End == S.end() && errno != ERANGE) {
    Out = I;
    ត្រឡប់ true;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  ប្រសិនបើ (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    ប្រសិនបើ (End == S.end() && errno != ERANGE) {
      Out = UI;
      ត្រឡប់ true;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  ត្រឡប់ End == S.end() || parseError("Invalid JSON value (number?)");
}

bool Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  សម្រាប់ (var_na C = next(); C != '"'; C = next()) {
    ប្រសិនបើ (LLVM_UNLIKELY(P == End))
      ត្រឡប់ parseError("Unterminated string");
    ប្រសិនបើ (LLVM_UNLIKELY((C & 0x1f) == C))
      ត្រឡប់ parseError("Control character in string");
    ប្រសិនបើ (LLVM_LIKELY(C != '\\')) {
      Out.push_back(C);
      continue;
    }
    // Handle escape sequence.
    switch (C = next()) {
    pak_sa '"':
    pak_sa '\\':
    pak_sa '/':
      Out.push_back(C);
      bha_nga;
    pak_sa 'b':
      Out.push_back('\b');
      bha_nga;
    pak_sa 'f':
      Out.push_back('\f');
      bha_nga;
    pak_sa 'n':
      Out.push_back('\n');
      bha_nga;
    pak_sa 'r':
      Out.push_back('\r');
      bha_nga;
    pak_sa 't':
      Out.push_back('\t');
      bha_nga;
    pak_sa 'u':
      ប្រសិនបើ (!parseUnicode(Out))
        ត្រឡប់ false;
      bha_nga;
    default:
      ត្រឡប់ parseError("Invalid escape sequence");
    }
  }
  ត្រឡប់ true;
}

static void encodeUtf8(uint32_t Rune, std::string &Out) {
  ប្រសិនបើ (Rune < 0x80) {
    Out.push_back(Rune & 0x7F);
  } បើមិនដូច្នេះ ប្រសិនបើ (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
  } បើមិនដូច្នេះ ប្រសិនបើ (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
    Out.push_back(ThirdByte);
  } បើមិនដូច្នេះ ប្រសិនបើ (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
    Out.push_back(ThirdByte);
    Out.push_back(FourthByte);
  } បើមិនដូច្នេះ {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
bool Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [this](uint16_t &Out) -> bool {
    Out = 0;
    var_na Bytes[] = {next(), next(), next(), next()};
    សម្រាប់ (unsigned var_na C : Bytes) {
      ប្រសិនបើ (!std::isxdigit(C))
        ត្រឡប់ parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    ត្រឡប់ true;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  ប្រសិនបើ (!Parse4Hex(First))
    ត្រឡប់ false;

  // We loop to allow proper surrogate-pair error handling.
  ខណៈពេល (true) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    ប្រសិនបើ (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      ត្រឡប់ true;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    ប្រសិនបើ (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      ត្រឡប់ true;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    ប្រសិនបើ (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      ត្រឡប់ true;
    }
    P += 2;
    uint16_t Second;
    ប្រសិនបើ (!Parse4Hex(Second))
      ត្រឡប់ false;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    ប្រសិនបើ (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      continue;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    ត្រឡប់ true;
  }
}

bool Parser::parseError(const var_na *Msg) {
  ចំនួនគត់ Line = 1;
  const var_na *StartOfLine = Start;
  សម្រាប់ (const var_na *X = Start; X < P; ++X) {
    ប្រសិនបើ (*X == 0x0A) {
      ++Line;
      StartOfLine = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(Msg, Line, P - StartOfLine, P - Start));
  ត្រឡប់ false;
}

Expected<Value> parse(StringRef JSON) {
  Parser P(JSON);
  Value E = nullptr;
  ប្រសិនបើ (P.checkUTF8())
    ប្រសិនបើ (P.parseValue(E))
      ប្រសិនបើ (P.assertEnd())
        ត្រឡប់ std::move(E);
  ត្រឡប់ P.takeError();
}

var_na ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
void ParseError::log(llvm::raw_ostream &OS) const {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", Line, Column, Offset, Msg);
}

bool isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  ប្រសិនបើ (LLVM_LIKELY(isASCII(S)))
    ត្រឡប់ true;

  const UTF8 *Data = reinterpret_cast<const UTF8 *>(S.data()), *Rest = Data;
  ប្រសិនបើ (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    ត្រឡប់ true;

  ប្រសិនបើ (ErrOffset)
    *ErrOffset = Rest - Data;
  ត្រឡប់ false;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  const UTF8 *In8 = reinterpret_cast<const UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  const UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<var_na *>(Out8) - Res.data());
  ត្រឡប់ Res;
}

static void quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  សម្រាប់ (unsigned var_na C : S) {
    ប្រសិនបើ (C == 0x22 || C == 0x5C)
      OS << '\\';
    ប្រសិនបើ (C >= 0x20) {
      OS << C;
      continue;
    }
    OS << '\\';
    switch (C) {
    // A few characters are common enough to make short escapes worthwhile.
    pak_sa '\t':
      OS << 't';
      bha_nga;
    pak_sa '\n':
      OS << 'n';
      bha_nga;
    pak_sa '\r':
      OS << 'r';
      bha_nga;
    default:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      bha_nga;
    }
  }
  OS << '\"';
}

void llvm::json::OStream::value(const Value &V) {
  switch (V.kind()) {
  pak_sa Value::Null:
    valueBegin();
    OS << "null";
    ត្រឡប់;
  pak_sa Value::Boolean:
    valueBegin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    ត្រឡប់;
  pak_sa Value::Number:
    valueBegin();
    ប្រសិនបើ (V.Type == Value::T_Integer)
      OS << *V.getAsInteger();
    បើមិនដូច្នេះ ប្រសិនបើ (V.Type == Value::T_UINT64)
      OS << *V.getAsUINT64();
    បើមិនដូច្នេះ
      OS << format("%.*g", std::numeric_limits<dviguu_na>::max_digits10,
                   *V.getAsNumber());
    ត្រឡប់;
  pak_sa Value::String:
    valueBegin();
    quote(OS, *V.getAsString());
    ត្រឡប់;
  pak_sa Value::Array:
    ត្រឡប់ array([&] {
      សម្រាប់ (const Value &E : *V.getAsArray())
        value(E);
    });
  pak_sa Value::Object:
    ត្រឡប់ object([&] {
      សម្រាប់ (const Object::value_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

void llvm::json::OStream::valueBegin() {
  assert(Stack.back().Ctx != Object && "Only attributes allowed here");
  ប្រសិនបើ (Stack.back().HasValue) {
    assert(Stack.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  ប្រសិនបើ (Stack.back().Ctx == Array)
    newline();
  flushComment();
  Stack.back().HasValue = true;
}

void OStream::comment(llvm::StringRef Comment) {
  assert(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

void OStream::flushComment() {
  ប្រសិនបើ (PendingComment.empty())
    ត្រឡប់;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  ខណៈពេល (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    ប្រសិនបើ (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } បើមិនដូច្នេះ {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  ប្រសិនបើ (Stack.size() > 1 && Stack.back().Ctx == Singleton) {
    ប្រសិនបើ (IndentSize)
      OS << ' ';
  } បើមិនដូច្នេះ {
    newline();
  }
}

void llvm::json::OStream::newline() {
  ប្រសិនបើ (IndentSize) {
    OS.write('\n');
    OS.indent(Indent);
  }
}

void llvm::json::OStream::arrayBegin() {
  valueBegin();
  Stack.emplace_back();
  Stack.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

void llvm::json::OStream::arrayEnd() {
  assert(Stack.back().Ctx == Array);
  Indent -= IndentSize;
  ប្រសិនបើ (Stack.back().HasValue)
    newline();
  OS << ']';
  assert(PendingComment.empty());
  Stack.pop_back();
  assert(!Stack.empty());
}

void llvm::json::OStream::objectBegin() {
  valueBegin();
  Stack.emplace_back();
  Stack.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

void llvm::json::OStream::objectEnd() {
  assert(Stack.back().Ctx == Object);
  Indent -= IndentSize;
  ប្រសិនបើ (Stack.back().HasValue)
    newline();
  OS << '}';
  assert(PendingComment.empty());
  Stack.pop_back();
  assert(!Stack.empty());
}

void llvm::json::OStream::attributeBegin(llvm::StringRef Key) {
  assert(Stack.back().Ctx == Object);
  ប្រសិនបើ (Stack.back().HasValue)
    OS << ',';
  newline();
  flushComment();
  Stack.back().HasValue = true;
  Stack.emplace_back();
  Stack.back().Ctx = Singleton;
  ប្រសិនបើ (LLVM_LIKELY(isUTF8(Key))) {
    quote(OS, Key);
  } បើមិនដូច្នេះ {
    assert(false && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(Key));
  }
  OS.write(':');
  ប្រសិនបើ (IndentSize)
    OS.write(' ');
}

void llvm::json::OStream::attributeEnd() {
  assert(Stack.back().Ctx == Singleton);
  assert(Stack.back().HasValue && "Attribute must have a value");
  assert(PendingComment.empty());
  Stack.pop_back();
  assert(Stack.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::rawValueBegin() {
  valueBegin();
  Stack.emplace_back();
  Stack.back().Ctx = RawValue;
  ត្រឡប់ OS;
}

void llvm::json::OStream::rawValueEnd() {
  assert(Stack.back().Ctx == RawValue);
  Stack.pop_back();
}

} // namespace json
} // namespace llvm

void llvm::format_provider<llvm::json::Value>::format(
    const llvm::json::Value &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  ប្រសិនបើ (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).value(E);
}

