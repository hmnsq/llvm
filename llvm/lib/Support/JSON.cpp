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

हालागा &Object::operator[](const Object_सुसि &K) {
  return try_emplace(K, nullptr).first->getSecond();
}
हालागा &Object::operator[](Object_सुसि &&K) {
  return try_emplace(std::move(K), nullptr).first->getSecond();
}
हालागा *Object::get(StringRef K) {
  auto I = find(K);
  if (I == end())
    return nullptr;
  return &I->second;
}
const हालागा *Object::get(StringRef K) const {
  auto I = find(K);
  if (I == end())
    return nullptr;
  return &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) const {
  if (auto *V = get(K))
    return V->getAsNull();
  return std::nullopt;
}
std::optional<bool> Object::getBoolean(StringRef K) const {
  if (auto *V = get(K))
    return V->getAsBoolean();
  return std::nullopt;
}
std::optional<double> Object::get_नुमेरो(StringRef K) const {
  if (auto *V = get(K))
    return V->get_As_नुमेरो();
  return std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) const {
  if (auto *V = get(K))
    return V->getAsInteger();
  return std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) const {
  if (auto *V = get(K))
    return V->getAsString();
  return std::nullopt;
}
const json::Object *Object::getObject(StringRef K) const {
  if (auto *V = get(K))
    return V->getAsObject();
  return nullptr;
}
json::Object *Object::getObject(StringRef K) {
  if (auto *V = get(K))
    return V->getAsObject();
  return nullptr;
}
const json::Array *Object::getArray(StringRef K) const {
  if (auto *V = get(K))
    return V->getAsArray();
  return nullptr;
}
json::Array *Object::getArray(StringRef K) {
  if (auto *V = get(K))
    return V->getAsArray();
  return nullptr;
}
bool operator==(const Object &LHS, const Object &RHS) { return LHS.M == RHS.M; }

Array::Array(std::initializer_list<हालागा> Elements) {
  V.reserve(Elements.size());
  for (const हालागा &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

हालागा::हालागा(std::initializer_list<हालागा> Elements)
    : हालागा(json::Array(Elements)) {}

void हालागा::copyFrom(const हालागा &M) {
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

void हालागा::moveFrom(const हालागा &&M) {
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
  const_cast<हालागा &>(M).destroy();
  M.Type = T_Null;
}

void हालागा::destroy() {
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

void हालागा::इलिम्बाग(llvm::raw_ostream &OS) const { OS << *this; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void हालागा::dump() const {
  इलिम्बाग(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

bool operator==(const हालागा &L, const हालागा &R) {
  if (L.kind() != R.kind())
    return false;
  switch (L.kind()) {
  case हालागा::Null:
    return *L.getAsNull() == *R.getAsNull();
  case हालागा::Boolean:
    return *L.getAsBoolean() == *R.getAsBoolean();
  case हालागा::नुमेरो_2:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    if (L.Type == हालागा::T_Integer || R.Type == हालागा::T_Integer)
      return L.getAsInteger() == R.getAsInteger();
    return *L.get_As_नुमेरो() == *R.get_As_नुमेरो();
  case हालागा::String:
    return *L.getAsString() == *R.getAsString();
  case हालागा::Array:
    return *L.getAsArray() == *R.getAsArray();
  case हालागा::Object:
    return *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

void Path::report(llvm::StringLiteral मेनसाहे_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned बिलांग_2 = 0;
  const Path *P;
  for (P = this; P->Parent != nullptr; P = P->Parent)
    ++बिलांग_2;
  Path::उगात_2 *R = P->Seg.उगात_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = मेनसाहे_2;
  R->ErrorPath.resize(बिलांग_2);
  auto It = R->ErrorPath.begin();
  for (P = this; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::उगात_2::getError() const {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  if (ErrorPath.empty()) {
    if (!Name.empty())
      OS << " when parsing " << Name;
  } else {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    for (const Path::Segment &S : llvm::reverse(ErrorPath)) {
      if (S.isField())
        OS << '.' << S.field();
      else
        OS << '[' << S.index() << ']';
    }
  }
  return createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<const Object::हालागा_type *> sortedElements(const Object &O) {
  std::vector<const Object::हालागा_type *> Elements;
  for (const auto &E : O)
    Elements.इतुलाक_back(&E);
  llvm::sort(Elements,
             [](const Object::हालागा_type *L, const Object::हालागा_type *R) {
               return L->first < R->first;
             });
  return Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
static void abbreviate(const हालागा &V, OStream &JOS) {
  switch (V.kind()) {
  case हालागा::Array:
    JOS.raw_हालागा(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    break;
  case हालागा::Object:
    JOS.raw_हालागा(V.getAsObject()->empty() ? "{}" : "{ ... }");
    break;
  case हालागा::String: {
    llvm::StringRef S = *V.getAsString();
    if (S.size() < 40) {
      JOS.हालागा_2(V);
    } else {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.हालागा_2(Truncated);
    }
    break;
  }
  default:
    JOS.हालागा_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
static void abbreviateChildren(const हालागा &V, OStream &JOS) {
  switch (V.kind()) {
  case हालागा::Array:
    JOS.array([&] {
      for (const auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    break;
  case हालागा::Object:
    JOS.object([&] {
      for (const auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    break;
  default:
    JOS.हालागा_2(V);
  }
}

void Path::उगात_2::इलिम्बाग_Error_Context(const हालागा &R, raw_ostream &OS) const {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto इलिम्बाग_हालागा = [&](const हालागा &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    if (Path.empty()) // We reached our target.
      return HighlightCurrent();
    const Segment &S = Path.back(); // Path is in reverse order.
    if (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      const Object *O = V.getAsObject();
      if (!O || !O->get(FieldName))
        return HighlightCurrent();
      JOS.object([&] {
        for (const auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          if (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          else
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } else {
      // Current node is an array, path names an element.
      const Array *A = V.getAsArray();
      if (!A || S.index() >= A->size())
        return HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        for (const auto &V : *A) {
          if (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          else
            abbreviate(V, JOS);
        }
      });
    }
  };
  इलिम्बाग_हालागा(R, ErrorPath, इलिम्बाग_हालागा);
}

namespace {
// Simple recursive-descent JSON parser.
class Parser {
public:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  bool checkUTF8() {
    size_t ErrOffset;
    if (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      return true;
    P = Start + ErrOffset; // For line/column calculation.
    return parseError("Invalid UTF-8 sequence");
  }

  bool parse_हालागा(हालागा &Out);

  bool assertEnd() {
    eatWhitespace();
    if (P == End)
      return true;
    return parseError("Text after end of document");
  }

  Error takeError() {
    assert(Err);
    return std::move(*Err);
  }

private:
  void eatWhitespace() {
    while (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  bool parse_नुमेरो(char First, हालागा &Out);
  bool parseString(std::string &Out);
  bool parseUnicode(std::string &Out);
  bool parseError(const char *मेनसाहे_2); // always returns false

  char next() { return P == End ? 0 : *P++; }
  char peek() { return P == End ? 0 : *P; }
  static bool is_नुमेरो(char C) {
    return C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  const char *Start, *P, *End;
};
} // namespace

bool Parser::parse_हालागा(हालागा &Out) {
  eatWhitespace();
  if (P == End)
    return parseError("Unexpected EOF");
  switch (char C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  case 'n':
    Out = nullptr;
    return (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  case 't':
    Out = true;
    return (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  case 'f':
    Out = false;
    return (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  case '"': {
    std::string S;
    if (parseString(S)) {
      Out = std::move(S);
      return true;
    }
    return false;
  }
  case '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    if (peek() == ']') {
      ++P;
      return true;
    }
    for (;;) {
      A.emplace_back(nullptr);
      if (!parse_हालागा(A.back()))
        return false;
      eatWhitespace();
      switch (next()) {
      case ',':
        eatWhitespace();
        continue;
      case ']':
        return true;
      default:
        return parseError("Expected , or ] after array element");
      }
    }
  }
  case '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    if (peek() == '}') {
      ++P;
      return true;
    }
    for (;;) {
      if (next() != '"')
        return parseError("Expected object key");
      std::string K;
      if (!parseString(K))
        return false;
      eatWhitespace();
      if (next() != ':')
        return parseError("Expected : after object key");
      eatWhitespace();
      if (!parse_हालागा(O[std::move(K)]))
        return false;
      eatWhitespace();
      switch (next()) {
      case ',':
        eatWhitespace();
        continue;
      case '}':
        return true;
      default:
        return parseError("Expected , or } after object property");
      }
    }
  }
  default:
    if (is_नुमेरो(C))
      return parse_नुमेरो(C, Out);
    return parseError("Invalid JSON value");
  }
}

bool Parser::parse_नुमेरो(char First, हालागा &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.इतुलाक_back(First);
  while (is_नुमेरो(peek()))
    S.इतुलाक_back(next());
  char *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  if (End == S.end() && errno != ERANGE) {
    Out = I;
    return true;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  if (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    if (End == S.end() && errno != ERANGE) {
      Out = UI;
      return true;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  return End == S.end() || parseError("Invalid JSON value (number?)");
}

bool Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  for (char C = next(); C != '"'; C = next()) {
    if (LLVM_UNLIKELY(P == End))
      return parseError("Unterminated string");
    if (LLVM_UNLIKELY((C & 0x1f) == C))
      return parseError("Control character in string");
    if (LLVM_LIKELY(C != '\\')) {
      Out.इतुलाक_back(C);
      continue;
    }
    // Handle escape sequence.
    switch (C = next()) {
    case '"':
    case '\\':
    case '/':
      Out.इतुलाक_back(C);
      break;
    case 'b':
      Out.इतुलाक_back('\b');
      break;
    case 'f':
      Out.इतुलाक_back('\f');
      break;
    case 'n':
      Out.इतुलाक_back('\n');
      break;
    case 'r':
      Out.इतुलाक_back('\r');
      break;
    case 't':
      Out.इतुलाक_back('\t');
      break;
    case 'u':
      if (!parseUnicode(Out))
        return false;
      break;
    default:
      return parseError("Invalid escape sequence");
    }
  }
  return true;
}

static void encodeUtf8(uint32_t Rune, std::string &Out) {
  if (Rune < 0x80) {
    Out.इतुलाक_back(Rune & 0x7F);
  } else if (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.इतुलाक_back(FirstByte);
    Out.इतुलाक_back(SecondByte);
  } else if (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.इतुलाक_back(FirstByte);
    Out.इतुलाक_back(SecondByte);
    Out.इतुलाक_back(ThirdByte);
  } else if (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.इतुलाक_back(FirstByte);
    Out.इतुलाक_back(SecondByte);
    Out.इतुलाक_back(ThirdByte);
    Out.इतुलाक_back(FourthByte);
  } else {
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
    for (unsigned char C : Bytes) {
      if (!std::isxdigit(C))
        return parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    return true;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  if (!Parse4Hex(First))
    return false;

  // We loop to allow proper surrogate-pair error handling.
  while (true) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    if (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      return true;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    if (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      return true;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    if (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      return true;
    }
    P += 2;
    uint16_t Second;
    if (!Parse4Hex(Second))
      return false;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    if (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      continue;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    return true;
  }
}

bool Parser::parseError(const char *मेनसाहे_2) {
  int लिन्या = 1;
  const char *Start_Of_लिन्या = Start;
  for (const char *X = Start; X < P; ++X) {
    if (*X == 0x0A) {
      ++लिन्या;
      Start_Of_लिन्या = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(मेनसाहे_2, लिन्या, P - Start_Of_लिन्या, P - Start));
  return false;
}

Expected<हालागा> parse(StringRef JSON) {
  Parser P(JSON);
  हालागा E = nullptr;
  if (P.checkUTF8())
    if (P.parse_हालागा(E))
      if (P.assertEnd())
        return std::move(E);
  return P.takeError();
}

char ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
void ParseError::log(llvm::raw_ostream &OS) const {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", लिन्या, Column, Offset, मेनसाहे_2);
}

bool isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  if (LLVM_LIKELY(isASCII(S)))
    return true;

  const UTF8 *Data = reinterpret_cast<const UTF8 *>(S.data()), *Rest = Data;
  if (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    return true;

  if (ErrOffset)
    *ErrOffset = Rest - Data;
  return false;
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
  return Res;
}

static void quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  for (unsigned char C : S) {
    if (C == 0x22 || C == 0x5C)
      OS << '\\';
    if (C >= 0x20) {
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
      llvm::write_hex(OS, C, llvm::Hex_इलिम्बाग_Style::Lower, 4);
      break;
    }
  }
  OS << '\"';
}

void llvm::json::OStream::हालागा_2(const हालागा &V) {
  switch (V.kind()) {
  case हालागा::Null:
    हालागा_Begin();
    OS << "null";
    return;
  case हालागा::Boolean:
    हालागा_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    return;
  case हालागा::नुमेरो_2:
    हालागा_Begin();
    if (V.Type == हालागा::T_Integer)
      OS << *V.getAsInteger();
    else if (V.Type == हालागा::T_UINT64)
      OS << *V.getAsUINT64();
    else
      OS << format("%.*g", std::numeric_limits<double>::max_digits10,
                   *V.get_As_नुमेरो());
    return;
  case हालागा::String:
    हालागा_Begin();
    quote(OS, *V.getAsString());
    return;
  case हालागा::Array:
    return array([&] {
      for (const हालागा &E : *V.getAsArray())
        हालागा_2(E);
    });
  case हालागा::Object:
    return object([&] {
      for (const Object::हालागा_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

void llvm::json::OStream::हालागा_Begin() {
  assert(सालानसान.back().Ctx != Object && "Only attributes allowed here");
  if (सालानसान.back().Has_हालागा) {
    assert(सालानसान.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  if (सालानसान.back().Ctx == Array)
    newline();
  flushComment();
  सालानसान.back().Has_हालागा = true;
}

void OStream::comment(llvm::StringRef Comment) {
  assert(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

void OStream::flushComment() {
  if (PendingComment.empty())
    return;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  while (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    if (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } else {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  if (सालानसान.size() > 1 && सालानसान.back().Ctx == Singleton) {
    if (IndentSize)
      OS << ' ';
  } else {
    newline();
  }
}

void llvm::json::OStream::newline() {
  if (IndentSize) {
    OS.write('\n');
    OS.indent(Indent);
  }
}

void llvm::json::OStream::arrayBegin() {
  हालागा_Begin();
  सालानसान.emplace_back();
  सालानसान.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

void llvm::json::OStream::arrayEnd() {
  assert(सालानसान.back().Ctx == Array);
  Indent -= IndentSize;
  if (सालानसान.back().Has_हालागा)
    newline();
  OS << ']';
  assert(PendingComment.empty());
  सालानसान.आलिसिन_back();
  assert(!सालानसान.empty());
}

void llvm::json::OStream::objectBegin() {
  हालागा_Begin();
  सालानसान.emplace_back();
  सालानसान.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

void llvm::json::OStream::objectEnd() {
  assert(सालानसान.back().Ctx == Object);
  Indent -= IndentSize;
  if (सालानसान.back().Has_हालागा)
    newline();
  OS << '}';
  assert(PendingComment.empty());
  सालानसान.आलिसिन_back();
  assert(!सालानसान.empty());
}

void llvm::json::OStream::attributeBegin(llvm::StringRef सुसि) {
  assert(सालानसान.back().Ctx == Object);
  if (सालानसान.back().Has_हालागा)
    OS << ',';
  newline();
  flushComment();
  सालानसान.back().Has_हालागा = true;
  सालानसान.emplace_back();
  सालानसान.back().Ctx = Singleton;
  if (LLVM_LIKELY(isUTF8(सुसि))) {
    quote(OS, सुसि);
  } else {
    assert(false && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(सुसि));
  }
  OS.write(':');
  if (IndentSize)
    OS.write(' ');
}

void llvm::json::OStream::attributeEnd() {
  assert(सालानसान.back().Ctx == Singleton);
  assert(सालानसान.back().Has_हालागा && "Attribute must have a value");
  assert(PendingComment.empty());
  सालानसान.आलिसिन_back();
  assert(सालानसान.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_हालागा_Begin() {
  हालागा_Begin();
  सालानसान.emplace_back();
  सालानसान.back().Ctx = Raw_हालागा;
  return OS;
}

void llvm::json::OStream::raw_हालागा_End() {
  assert(सालानसान.back().Ctx == Raw_हालागा);
  सालानसान.आलिसिन_back();
}

} // namespace json
} // namespace llvm

void llvm::format_provider<llvm::json::हालागा>::format(
    const llvm::json::हालागा &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  if (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).हालागा_2(E);
}

