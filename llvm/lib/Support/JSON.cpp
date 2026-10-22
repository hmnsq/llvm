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
  вернуть try_emplace(K, nullptr).first->getSecond();
}
Value &Object::operator[](ObjectKey &&K) {
  вернуть try_emplace(std::move(K), nullptr).first->getSecond();
}
Value *Object::get(StringRef K) {
  auto I = find(K);
  если (I == end())
    вернуть nullptr;
  вернуть &I->second;
}
const Value *Object::get(StringRef K) const {
  auto I = find(K);
  если (I == end())
    вернуть nullptr;
  вернуть &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) const {
  если (auto *V = get(K))
    вернуть V->getAsNull();
  вернуть std::nullopt;
}
std::optional<bool> Object::getBoolean(StringRef K) const {
  если (auto *V = get(K))
    вернуть V->getAsBoolean();
  вернуть std::nullopt;
}
std::optional<double> Object::getNumber(StringRef K) const {
  если (auto *V = get(K))
    вернуть V->getAsNumber();
  вернуть std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) const {
  если (auto *V = get(K))
    вернуть V->getAsInteger();
  вернуть std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) const {
  если (auto *V = get(K))
    вернуть V->getAsString();
  вернуть std::nullopt;
}
const json::Object *Object::getObject(StringRef K) const {
  если (auto *V = get(K))
    вернуть V->getAsObject();
  вернуть nullptr;
}
json::Object *Object::getObject(StringRef K) {
  если (auto *V = get(K))
    вернуть V->getAsObject();
  вернуть nullptr;
}
const json::Array *Object::getArray(StringRef K) const {
  если (auto *V = get(K))
    вернуть V->getAsArray();
  вернуть nullptr;
}
json::Array *Object::getArray(StringRef K) {
  если (auto *V = get(K))
    вернуть V->getAsArray();
  вернуть nullptr;
}
bool operator==(const Object &LHS, const Object &RHS) { вернуть LHS.M == RHS.M; }

Array::Array(std::initializer_list<Value> Elements) {
  V.reserve(Elements.size());
  для (const Value &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

Value::Value(std::initializer_list<Value> Elements)
    : Value(json::Array(Elements)) {}

void Value::copyFrom(const Value &M) {
  Type = M.Type;
  switch (Type) {
  case T_Null:
  case T_Boolean:
  case T_Double:
  case T_Integer:
  case T_UINT64:
    memcpy(&Union, &M.Union, sizeof(Union));
    break;
  case T_StringRef:
    create<StringRef>(M.as<StringRef>());
    break;
  case T_String:
    create<std::string>(M.as<std::string>());
    break;
  case T_Object:
    create<json::Object>(M.as<json::Object>());
    break;
  case T_Array:
    create<json::Array>(M.as<json::Array>());
    break;
  }
}

void Value::moveFrom(const Value &&M) {
  Type = M.Type;
  switch (Type) {
  case T_Null:
  case T_Boolean:
  case T_Double:
  case T_Integer:
  case T_UINT64:
    memcpy(&Union, &M.Union, sizeof(Union));
    break;
  case T_StringRef:
    create<StringRef>(M.as<StringRef>());
    break;
  case T_String:
    create<std::string>(std::move(M.as<std::string>()));
    break;
  case T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    break;
  case T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    break;
  }
  const_cast<Value &>(M).destroy();
  M.Type = T_Null;
}

void Value::destroy() {
  switch (Type) {
  case T_Null:
  case T_Boolean:
  case T_Double:
  case T_Integer:
  case T_UINT64:
    break;
  case T_StringRef:
    as<StringRef>().~StringRef();
    break;
  case T_String:
    as<std::string>().~basic_string();
    break;
  case T_Object:
    as<json::Object>().~Object();
    break;
  case T_Array:
    as<json::Array>().~Array();
    break;
  }
}

void Value::print(llvm::raw_ostream &OS) const { OS << *this; }

#если !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void Value::dump() const {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

bool operator==(const Value &L, const Value &R) {
  если (L.kind() != R.kind())
    вернуть false;
  switch (L.kind()) {
  case Value::Null:
    вернуть *L.getAsNull() == *R.getAsNull();
  case Value::Boolean:
    вернуть *L.getAsBoolean() == *R.getAsBoolean();
  case Value::Number:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    если (L.Type == Value::T_Integer || R.Type == Value::T_Integer)
      вернуть L.getAsInteger() == R.getAsInteger();
    вернуть *L.getAsNumber() == *R.getAsNumber();
  case Value::String:
    вернуть *L.getAsString() == *R.getAsString();
  case Value::Array:
    вернуть *L.getAsArray() == *R.getAsArray();
  case Value::Object:
    вернуть *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

void Path::report(llvm::StringLiteral Msg) {
  // Walk up to the root context, and count the number of segments.
  unsigned Count = 0;
  const Path *P;
  для (P = this; P->Parent != nullptr; P = P->Parent)
    ++Count;
  Path::Root *R = P->Seg.root();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = Msg;
  R->ErrorPath.resize(Count);
  auto It = R->ErrorPath.begin();
  для (P = this; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::Root::getError() const {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  если (ErrorPath.empty()) {
    если (!Name.empty())
      OS << " when parsing " << Name;
  } иначе {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    для (const Path::Segment &S : llvm::reverse(ErrorPath)) {
      если (S.isField())
        OS << '.' << S.field();
      иначе
        OS << '[' << S.index() << ']';
    }
  }
  вернуть createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<const Object::value_type *> sortedElements(const Object &O) {
  std::vector<const Object::value_type *> Elements;
  для (const auto &E : O)
    Elements.push_back(&E);
  llvm::sort(Elements,
             [](const Object::value_type *L, const Object::value_type *R) {
               вернуть L->first < R->first;
             });
  вернуть Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
static void abbreviate(const Value &V, OStream &JOS) {
  switch (V.kind()) {
  case Value::Array:
    JOS.rawValue(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    break;
  case Value::Object:
    JOS.rawValue(V.getAsObject()->empty() ? "{}" : "{ ... }");
    break;
  case Value::String: {
    llvm::StringRef S = *V.getAsString();
    если (S.size() < 40) {
      JOS.value(V);
    } иначе {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.value(Truncated);
    }
    break;
  }
  default:
    JOS.value(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
static void abbreviateChildren(const Value &V, OStream &JOS) {
  switch (V.kind()) {
  case Value::Array:
    JOS.array([&] {
      для (const auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    break;
  case Value::Object:
    JOS.object([&] {
      для (const auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    break;
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
    если (Path.empty()) // We reached our target.
      вернуть HighlightCurrent();
    const Segment &S = Path.back(); // Path is in reverse order.
    если (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      const Object *O = V.getAsObject();
      если (!O || !O->get(FieldName))
        вернуть HighlightCurrent();
      JOS.object([&] {
        для (const auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          если (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          иначе
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } иначе {
      // Current node is an array, path names an element.
      const Array *A = V.getAsArray();
      если (!A || S.index() >= A->size())
        вернуть HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        для (const auto &V : *A) {
          если (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          иначе
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
    если (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      вернуть true;
    P = Start + ErrOffset; // For line/column calculation.
    вернуть parseError("Invalid UTF-8 sequence");
  }

  bool parseValue(Value &Out);

  bool assertEnd() {
    eatWhitespace();
    если (P == End)
      вернуть true;
    вернуть parseError("Text after end of document");
  }

  Error takeError() {
    assert(Err);
    вернуть std::move(*Err);
  }

private:
  void eatWhitespace() {
    пока (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  bool parseNumber(char First, Value &Out);
  bool parseString(std::string &Out);
  bool parseUnicode(std::string &Out);
  bool parseError(const char *Msg); // always returns false

  char next() { вернуть P == End ? 0 : *P++; }
  char peek() { вернуть P == End ? 0 : *P; }
  static bool isNumber(char C) {
    вернуть C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  const char *Start, *P, *End;
};
} // namespace

bool Parser::parseValue(Value &Out) {
  eatWhitespace();
  если (P == End)
    вернуть parseError("Unexpected EOF");
  switch (char C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  case 'n':
    Out = nullptr;
    вернуть (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  case 't':
    Out = true;
    вернуть (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  case 'f':
    Out = false;
    вернуть (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  case '"': {
    std::string S;
    если (parseString(S)) {
      Out = std::move(S);
      вернуть true;
    }
    вернуть false;
  }
  case '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    если (peek() == ']') {
      ++P;
      вернуть true;
    }
    для (;;) {
      A.emplace_back(nullptr);
      если (!parseValue(A.back()))
        вернуть false;
      eatWhitespace();
      switch (next()) {
      case ',':
        eatWhitespace();
        continue;
      case ']':
        вернуть true;
      default:
        вернуть parseError("Expected , or ] after array element");
      }
    }
  }
  case '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    если (peek() == '}') {
      ++P;
      вернуть true;
    }
    для (;;) {
      если (next() != '"')
        вернуть parseError("Expected object key");
      std::string K;
      если (!parseString(K))
        вернуть false;
      eatWhitespace();
      если (next() != ':')
        вернуть parseError("Expected : after object key");
      eatWhitespace();
      если (!parseValue(O[std::move(K)]))
        вернуть false;
      eatWhitespace();
      switch (next()) {
      case ',':
        eatWhitespace();
        continue;
      case '}':
        вернуть true;
      default:
        вернуть parseError("Expected , or } after object property");
      }
    }
  }
  default:
    если (isNumber(C))
      вернуть parseNumber(C, Out);
    вернуть parseError("Invalid JSON value");
  }
}

bool Parser::parseNumber(char First, Value &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.push_back(First);
  пока (isNumber(peek()))
    S.push_back(next());
  char *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  если (End == S.end() && errno != ERANGE) {
    Out = I;
    вернуть true;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  если (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    если (End == S.end() && errno != ERANGE) {
      Out = UI;
      вернуть true;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  вернуть End == S.end() || parseError("Invalid JSON value (number?)");
}

bool Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  для (char C = next(); C != '"'; C = next()) {
    если (LLVM_UNLIKELY(P == End))
      вернуть parseError("Unterminated string");
    если (LLVM_UNLIKELY((C & 0x1f) == C))
      вернуть parseError("Control character in string");
    если (LLVM_LIKELY(C != '\\')) {
      Out.push_back(C);
      continue;
    }
    // Handle escape sequence.
    switch (C = next()) {
    case '"':
    case '\\':
    case '/':
      Out.push_back(C);
      break;
    case 'b':
      Out.push_back('\b');
      break;
    case 'f':
      Out.push_back('\f');
      break;
    case 'n':
      Out.push_back('\n');
      break;
    case 'r':
      Out.push_back('\r');
      break;
    case 't':
      Out.push_back('\t');
      break;
    case 'u':
      если (!parseUnicode(Out))
        вернуть false;
      break;
    default:
      вернуть parseError("Invalid escape sequence");
    }
  }
  вернуть true;
}

static void encodeUtf8(uint32_t Rune, std::string &Out) {
  если (Rune < 0x80) {
    Out.push_back(Rune & 0x7F);
  } иначе если (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
  } иначе если (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
    Out.push_back(ThirdByte);
  } иначе если (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.push_back(FirstByte);
    Out.push_back(SecondByte);
    Out.push_back(ThirdByte);
    Out.push_back(FourthByte);
  } иначе {
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
    char Bytes[] = {next(), next(), next(), next()};
    для (unsigned char C : Bytes) {
      если (!std::isxdigit(C))
        вернуть parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    вернуть true;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  если (!Parse4Hex(First))
    вернуть false;

  // We loop to allow proper surrogate-pair error handling.
  пока (true) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    если (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      вернуть true;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    если (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      вернуть true;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    если (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      вернуть true;
    }
    P += 2;
    uint16_t Second;
    если (!Parse4Hex(Second))
      вернуть false;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    если (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      continue;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    вернуть true;
  }
}

bool Parser::parseError(const char *Msg) {
  целое Line = 1;
  const char *StartOfLine = Start;
  для (const char *X = Start; X < P; ++X) {
    если (*X == 0x0A) {
      ++Line;
      StartOfLine = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(Msg, Line, P - StartOfLine, P - Start));
  вернуть false;
}

Expected<Value> parse(StringRef JSON) {
  Parser P(JSON);
  Value E = nullptr;
  если (P.checkUTF8())
    если (P.parseValue(E))
      если (P.assertEnd())
        вернуть std::move(E);
  вернуть P.takeError();
}

char ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
void ParseError::log(llvm::raw_ostream &OS) const {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", Line, Column, Offset, Msg);
}

bool isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  если (LLVM_LIKELY(isASCII(S)))
    вернуть true;

  const UTF8 *Data = reinterpret_cast<const UTF8 *>(S.data()), *Rest = Data;
  если (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    вернуть true;

  если (ErrOffset)
    *ErrOffset = Rest - Data;
  вернуть false;
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
  Res.resize(reinterpret_cast<char *>(Out8) - Res.data());
  вернуть Res;
}

static void quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  для (unsigned char C : S) {
    если (C == 0x22 || C == 0x5C)
      OS << '\\';
    если (C >= 0x20) {
      OS << C;
      continue;
    }
    OS << '\\';
    switch (C) {
    // A few characters are common enough to make short escapes worthwhile.
    case '\t':
      OS << 't';
      break;
    case '\n':
      OS << 'n';
      break;
    case '\r':
      OS << 'r';
      break;
    default:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      break;
    }
  }
  OS << '\"';
}

void llvm::json::OStream::value(const Value &V) {
  switch (V.kind()) {
  case Value::Null:
    valueBegin();
    OS << "null";
    вернуть;
  case Value::Boolean:
    valueBegin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    вернуть;
  case Value::Number:
    valueBegin();
    если (V.Type == Value::T_Integer)
      OS << *V.getAsInteger();
    иначе если (V.Type == Value::T_UINT64)
      OS << *V.getAsUINT64();
    иначе
      OS << format("%.*g", std::numeric_limits<double>::max_digits10,
                   *V.getAsNumber());
    вернуть;
  case Value::String:
    valueBegin();
    quote(OS, *V.getAsString());
    вернуть;
  case Value::Array:
    вернуть array([&] {
      для (const Value &E : *V.getAsArray())
        value(E);
    });
  case Value::Object:
    вернуть object([&] {
      для (const Object::value_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

void llvm::json::OStream::valueBegin() {
  assert(Stack.back().Ctx != Object && "Only attributes allowed here");
  если (Stack.back().HasValue) {
    assert(Stack.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  если (Stack.back().Ctx == Array)
    newline();
  flushComment();
  Stack.back().HasValue = true;
}

void OStream::comment(llvm::StringRef Comment) {
  assert(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

void OStream::flushComment() {
  если (PendingComment.empty())
    вернуть;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  пока (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    если (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } иначе {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  если (Stack.size() > 1 && Stack.back().Ctx == Singleton) {
    если (IndentSize)
      OS << ' ';
  } иначе {
    newline();
  }
}

void llvm::json::OStream::newline() {
  если (IndentSize) {
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
  если (Stack.back().HasValue)
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
  если (Stack.back().HasValue)
    newline();
  OS << '}';
  assert(PendingComment.empty());
  Stack.pop_back();
  assert(!Stack.empty());
}

void llvm::json::OStream::attributeBegin(llvm::StringRef Key) {
  assert(Stack.back().Ctx == Object);
  если (Stack.back().HasValue)
    OS << ',';
  newline();
  flushComment();
  Stack.back().HasValue = true;
  Stack.emplace_back();
  Stack.back().Ctx = Singleton;
  если (LLVM_LIKELY(isUTF8(Key))) {
    quote(OS, Key);
  } иначе {
    assert(false && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(Key));
  }
  OS.write(':');
  если (IndentSize)
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
  вернуть OS;
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
  если (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).value(E);
}

