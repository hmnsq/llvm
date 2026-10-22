//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/JSON.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/ConvertUTF.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/Format.h"
#અન્તર્ભાવ "llvm/Support/NativeFormatting.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cctype>
#અન્તર્ભાવ <cerrno>
#અન્તર્ભાવ <optional>

નામાકાશ llvm {
નામાકાશ json {

કિંમત &Object::operator[](અચલ Object_ચાવી &K) {
  પાછા try_emplace(K, nullptr).first->getSecond();
}
કિંમત &Object::operator[](Object_ચાવી &&K) {
  પાછા try_emplace(std::move(K), nullptr).first->getSecond();
}
કિંમત *Object::get(StringRef K) {
  auto I = find(K);
  જો (I == end())
    પાછા nullptr;
  પાછા &I->second;
}
અચલ કિંમત *Object::get(StringRef K) અચલ {
  auto I = find(K);
  જો (I == end())
    પાછા nullptr;
  પાછા &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) અચલ {
  જો (auto *V = get(K))
    પાછા V->getAsNull();
  પાછા std::nullopt;
}
std::optional<તાર્કિક> Object::getBoolean(StringRef K) અચલ {
  જો (auto *V = get(K))
    પાછા V->getAsBoolean();
  પાછા std::nullopt;
}
std::optional<દ્વિગુણ> Object::get_સંખ્યા(StringRef K) અચલ {
  જો (auto *V = get(K))
    પાછા V->get_As_સંખ્યા();
  પાછા std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) અચલ {
  જો (auto *V = get(K))
    પાછા V->getAsInteger();
  પાછા std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) અચલ {
  જો (auto *V = get(K))
    પાછા V->getAsString();
  પાછા std::nullopt;
}
અચલ json::Object *Object::getObject(StringRef K) અચલ {
  જો (auto *V = get(K))
    પાછા V->getAsObject();
  પાછા nullptr;
}
json::Object *Object::getObject(StringRef K) {
  જો (auto *V = get(K))
    પાછા V->getAsObject();
  પાછા nullptr;
}
અચલ json::Array *Object::getArray(StringRef K) અચલ {
  જો (auto *V = get(K))
    પાછા V->getAsArray();
  પાછા nullptr;
}
json::Array *Object::getArray(StringRef K) {
  જો (auto *V = get(K))
    પાછા V->getAsArray();
  પાછા nullptr;
}
તાર્કિક operator==(અચલ Object &LHS, અચલ Object &RHS) { પાછા LHS.M == RHS.M; }

Array::Array(std::initializer_list<કિંમત> Elements) {
  V.reserve(Elements.size());
  માટે (અચલ કિંમત &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

કિંમત::કિંમત(std::initializer_list<કિંમત> Elements)
    : કિંમત(json::Array(Elements)) {}

શૂન્ય કિંમત::copyFrom(અચલ કિંમત &M) {
  Type = M.Type;
  પસંદગી (Type) {
  સ્થિતિ T_Null:
  સ્થિતિ T_Boolean:
  સ્થિતિ T_Double:
  સ્થિતિ T_Integer:
  સ્થિતિ T_UINT64:
    memcpy(&Union, &M.Union, કદ(Union));
    તોડો;
  સ્થિતિ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    તોડો;
  સ્થિતિ T_String:
    create<std::string>(M.as<std::string>());
    તોડો;
  સ્થિતિ T_Object:
    create<json::Object>(M.as<json::Object>());
    તોડો;
  સ્થિતિ T_Array:
    create<json::Array>(M.as<json::Array>());
    તોડો;
  }
}

શૂન્ય કિંમત::moveFrom(અચલ કિંમત &&M) {
  Type = M.Type;
  પસંદગી (Type) {
  સ્થિતિ T_Null:
  સ્થિતિ T_Boolean:
  સ્થિતિ T_Double:
  સ્થિતિ T_Integer:
  સ્થિતિ T_UINT64:
    memcpy(&Union, &M.Union, કદ(Union));
    તોડો;
  સ્થિતિ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    તોડો;
  સ્થિતિ T_String:
    create<std::string>(std::move(M.as<std::string>()));
    તોડો;
  સ્થિતિ T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    તોડો;
  સ્થિતિ T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    તોડો;
  }
  const_cast<કિંમત &>(M).destroy();
  M.Type = T_Null;
}

શૂન્ય કિંમત::destroy() {
  પસંદગી (Type) {
  સ્થિતિ T_Null:
  સ્થિતિ T_Boolean:
  સ્થિતિ T_Double:
  સ્થિતિ T_Integer:
  સ્થિતિ T_UINT64:
    તોડો;
  સ્થિતિ T_StringRef:
    as<StringRef>().~StringRef();
    તોડો;
  સ્થિતિ T_String:
    as<std::string>().~basic_string();
    તોડો;
  સ્થિતિ T_Object:
    as<json::Object>().~Object();
    તોડો;
  સ્થિતિ T_Array:
    as<json::Array>().~Array();
    તોડો;
  }
}

શૂન્ય કિંમત::print(llvm::raw_ostream &OS) અચલ { OS << *આ; }

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય કિંમત::dump() અચલ {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

તાર્કિક operator==(અચલ કિંમત &L, અચલ કિંમત &R) {
  જો (L.kind() != R.kind())
    પાછા ખોટું;
  પસંદગી (L.kind()) {
  સ્થિતિ કિંમત::Null:
    પાછા *L.getAsNull() == *R.getAsNull();
  સ્થિતિ કિંમત::Boolean:
    પાછા *L.getAsBoolean() == *R.getAsBoolean();
  સ્થિતિ કિંમત::સંખ્યા:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    જો (L.Type == કિંમત::T_Integer || R.Type == કિંમત::T_Integer)
      પાછા L.getAsInteger() == R.getAsInteger();
    પાછા *L.get_As_સંખ્યા() == *R.get_As_સંખ્યા();
  સ્થિતિ કિંમત::String:
    પાછા *L.getAsString() == *R.getAsString();
  સ્થિતિ કિંમત::Array:
    પાછા *L.getAsArray() == *R.getAsArray();
  સ્થિતિ કિંમત::Object:
    પાછા *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

શૂન્ય Path::report(llvm::StringLiteral સંદેશ_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned ગણતરી_2 = 0;
  અચલ Path *P;
  માટે (P = આ; P->Parent != nullptr; P = P->Parent)
    ++ગણતરી_2;
  Path::મૂળ_2 *R = P->Seg.મૂળ_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = સંદેશ_2;
  R->ErrorPath.resize(ગણતરી_2);
  auto It = R->ErrorPath.begin();
  માટે (P = આ; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::મૂળ_2::getError() અચલ {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  જો (ErrorPath.empty()) {
    જો (!Name.empty())
      OS << " when parsing " << Name;
  } નહીંતર {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    માટે (અચલ Path::Segment &S : llvm::reverse(ErrorPath)) {
      જો (S.isField())
        OS << '.' << S.field();
      નહીંતર
        OS << '[' << S.index() << ']';
    }
  }
  પાછા createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<અચલ Object::કિંમત_type *> sortedElements(અચલ Object &O) {
  std::vector<અચલ Object::કિંમત_type *> Elements;
  માટે (અચલ auto &E : O)
    Elements.ધકેલો_back(&E);
  llvm::sort(Elements,
             [](અચલ Object::કિંમત_type *L, અચલ Object::કિંમત_type *R) {
               પાછા L->first < R->first;
             });
  પાછા Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
સ્થિર શૂન્ય abbreviate(અચલ કિંમત &V, OStream &JOS) {
  પસંદગી (V.kind()) {
  સ્થિતિ કિંમત::Array:
    JOS.raw_કિંમત(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    તોડો;
  સ્થિતિ કિંમત::Object:
    JOS.raw_કિંમત(V.getAsObject()->empty() ? "{}" : "{ ... }");
    તોડો;
  સ્થિતિ કિંમત::String: {
    llvm::StringRef S = *V.getAsString();
    જો (S.size() < 40) {
      JOS.કિંમત_2(V);
    } નહીંતર {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.કિંમત_2(Truncated);
    }
    તોડો;
  }
  મૂળભૂત:
    JOS.કિંમત_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
સ્થિર શૂન્ય abbreviateChildren(અચલ કિંમત &V, OStream &JOS) {
  પસંદગી (V.kind()) {
  સ્થિતિ કિંમત::Array:
    JOS.array([&] {
      માટે (અચલ auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    તોડો;
  સ્થિતિ કિંમત::Object:
    JOS.object([&] {
      માટે (અચલ auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    તોડો;
  મૂળભૂત:
    JOS.કિંમત_2(V);
  }
}

શૂન્ય Path::મૂળ_2::printErrorContext(અચલ કિંમત &R, raw_ostream &OS) અચલ {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_કિંમત = [&](અચલ કિંમત &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    જો (Path.empty()) // We reached our target.
      પાછા HighlightCurrent();
    અચલ Segment &S = Path.back(); // Path is in reverse order.
    જો (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      અચલ Object *O = V.getAsObject();
      જો (!O || !O->get(FieldName))
        પાછા HighlightCurrent();
      JOS.object([&] {
        માટે (અચલ auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          જો (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          નહીંતર
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } નહીંતર {
      // Current node is an array, path names an element.
      અચલ Array *A = V.getAsArray();
      જો (!A || S.index() >= A->size())
        પાછા HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        માટે (અચલ auto &V : *A) {
          જો (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          નહીંતર
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_કિંમત(R, ErrorPath, Print_કિંમત);
}

નામાકાશ {
// Simple recursive-descent JSON parser.
વર્ગ Parser {
જાહેર:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  તાર્કિક checkUTF8() {
    size_t ErrOffset;
    જો (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      પાછા સાચું;
    P = Start + ErrOffset; // For line/column calculation.
    પાછા parseError("Invalid UTF-8 sequence");
  }

  તાર્કિક parse_કિંમત(કિંમત &Out);

  તાર્કિક assertEnd() {
    eatWhitespace();
    જો (P == End)
      પાછા સાચું;
    પાછા parseError("Text after end of document");
  }

  Error takeError() {
    પ્રતિજ્ઞા(Err);
    પાછા std::move(*Err);
  }

ખાનગી:
  શૂન્ય eatWhitespace() {
    જ્યારે (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  તાર્કિક parse_સંખ્યા(અક્ષર First, કિંમત &Out);
  તાર્કિક parseString(std::string &Out);
  તાર્કિક parseUnicode(std::string &Out);
  તાર્કિક parseError(અચલ અક્ષર *સંદેશ_2); // always returns false

  અક્ષર next() { પાછા P == End ? 0 : *P++; }
  અક્ષર peek() { પાછા P == End ? 0 : *P; }
  સ્થિર તાર્કિક is_સંખ્યા(અક્ષર C) {
    પાછા C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  અચલ અક્ષર *Start, *P, *End;
};
} // namespace

તાર્કિક Parser::parse_કિંમત(કિંમત &Out) {
  eatWhitespace();
  જો (P == End)
    પાછા parseError("Unexpected EOF");
  પસંદગી (અક્ષર C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  સ્થિતિ 'n':
    Out = nullptr;
    પાછા (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  સ્થિતિ 't':
    Out = સાચું;
    પાછા (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  સ્થિતિ 'f':
    Out = ખોટું;
    પાછા (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  સ્થિતિ '"': {
    std::string S;
    જો (parseString(S)) {
      Out = std::move(S);
      પાછા સાચું;
    }
    પાછા ખોટું;
  }
  સ્થિતિ '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    જો (peek() == ']') {
      ++P;
      પાછા સાચું;
    }
    માટે (;;) {
      A.emplace_back(nullptr);
      જો (!parse_કિંમત(A.back()))
        પાછા ખોટું;
      eatWhitespace();
      પસંદગી (next()) {
      સ્થિતિ ',':
        eatWhitespace();
        ચાલુ;
      સ્થિતિ ']':
        પાછા સાચું;
      મૂળભૂત:
        પાછા parseError("Expected , or ] after array element");
      }
    }
  }
  સ્થિતિ '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    જો (peek() == '}') {
      ++P;
      પાછા સાચું;
    }
    માટે (;;) {
      જો (next() != '"')
        પાછા parseError("Expected object key");
      std::string K;
      જો (!parseString(K))
        પાછા ખોટું;
      eatWhitespace();
      જો (next() != ':')
        પાછા parseError("Expected : after object key");
      eatWhitespace();
      જો (!parse_કિંમત(O[std::move(K)]))
        પાછા ખોટું;
      eatWhitespace();
      પસંદગી (next()) {
      સ્થિતિ ',':
        eatWhitespace();
        ચાલુ;
      સ્થિતિ '}':
        પાછા સાચું;
      મૂળભૂત:
        પાછા parseError("Expected , or } after object property");
      }
    }
  }
  મૂળભૂત:
    જો (is_સંખ્યા(C))
      પાછા parse_સંખ્યા(C, Out);
    પાછા parseError("Invalid JSON value");
  }
}

તાર્કિક Parser::parse_સંખ્યા(અક્ષર First, કિંમત &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ધકેલો_back(First);
  જ્યારે (is_સંખ્યા(peek()))
    S.ધકેલો_back(next());
  અક્ષર *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  જો (End == S.end() && errno != ERANGE) {
    Out = I;
    પાછા સાચું;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  જો (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    જો (End == S.end() && errno != ERANGE) {
      Out = UI;
      પાછા સાચું;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  પાછા End == S.end() || parseError("Invalid JSON value (number?)");
}

તાર્કિક Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  માટે (અક્ષર C = next(); C != '"'; C = next()) {
    જો (LLVM_UNLIKELY(P == End))
      પાછા parseError("Unterminated string");
    જો (LLVM_UNLIKELY((C & 0x1f) == C))
      પાછા parseError("Control character in string");
    જો (LLVM_LIKELY(C != '\\')) {
      Out.ધકેલો_back(C);
      ચાલુ;
    }
    // Handle escape sequence.
    પસંદગી (C = next()) {
    સ્થિતિ '"':
    સ્થિતિ '\\':
    સ્થિતિ '/':
      Out.ધકેલો_back(C);
      તોડો;
    સ્થિતિ 'b':
      Out.ધકેલો_back('\b');
      તોડો;
    સ્થિતિ 'f':
      Out.ધકેલો_back('\f');
      તોડો;
    સ્થિતિ 'n':
      Out.ધકેલો_back('\n');
      તોડો;
    સ્થિતિ 'r':
      Out.ધકેલો_back('\r');
      તોડો;
    સ્થિતિ 't':
      Out.ધકેલો_back('\t');
      તોડો;
    સ્થિતિ 'u':
      જો (!parseUnicode(Out))
        પાછા ખોટું;
      તોડો;
    મૂળભૂત:
      પાછા parseError("Invalid escape sequence");
    }
  }
  પાછા સાચું;
}

સ્થિર શૂન્ય encodeUtf8(uint32_t Rune, std::string &Out) {
  જો (Rune < 0x80) {
    Out.ધકેલો_back(Rune & 0x7F);
  } નહીંતર જો (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.ધકેલો_back(FirstByte);
    Out.ધકેલો_back(SecondByte);
  } નહીંતર જો (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.ધકેલો_back(FirstByte);
    Out.ધકેલો_back(SecondByte);
    Out.ધકેલો_back(ThirdByte);
  } નહીંતર જો (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.ધકેલો_back(FirstByte);
    Out.ધકેલો_back(SecondByte);
    Out.ધકેલો_back(ThirdByte);
    Out.ધકેલો_back(FourthByte);
  } નહીંતર {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
તાર્કિક Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [આ](uint16_t &Out) -> તાર્કિક {
    Out = 0;
    અક્ષર Bytes[] = {next(), next(), next(), next()};
    માટે (unsigned અક્ષર C : Bytes) {
      જો (!std::isxdigit(C))
        પાછા parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    પાછા સાચું;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  જો (!Parse4Hex(First))
    પાછા ખોટું;

  // We loop to allow proper surrogate-pair error handling.
  જ્યારે (સાચું) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    જો (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      પાછા સાચું;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    જો (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      પાછા સાચું;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    જો (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      પાછા સાચું;
    }
    P += 2;
    uint16_t Second;
    જો (!Parse4Hex(Second))
      પાછા ખોટું;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    જો (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      ચાલુ;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    પાછા સાચું;
  }
}

તાર્કિક Parser::parseError(અચલ અક્ષર *સંદેશ_2) {
  પૂર્ણાંક લીટી = 1;
  અચલ અક્ષર *Start_Of_લીટી = Start;
  માટે (અચલ અક્ષર *X = Start; X < P; ++X) {
    જો (*X == 0x0A) {
      ++લીટી;
      Start_Of_લીટી = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(સંદેશ_2, લીટી, P - Start_Of_લીટી, P - Start));
  પાછા ખોટું;
}

Expected<કિંમત> parse(StringRef JSON) {
  Parser P(JSON);
  કિંમત E = nullptr;
  જો (P.checkUTF8())
    જો (P.parse_કિંમત(E))
      જો (P.assertEnd())
        પાછા std::move(E);
  પાછા P.takeError();
}

અક્ષર ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
શૂન્ય ParseError::log(llvm::raw_ostream &OS) અચલ {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", લીટી, Column, Offset, સંદેશ_2);
}

તાર્કિક isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  જો (LLVM_LIKELY(isASCII(S)))
    પાછા સાચું;

  અચલ UTF8 *Data = reinterpret_cast<અચલ UTF8 *>(S.data()), *Rest = Data;
  જો (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    પાછા સાચું;

  જો (ErrOffset)
    *ErrOffset = Rest - Data;
  પાછા ખોટું;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  અચલ UTF8 *In8 = reinterpret_cast<અચલ UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  અચલ UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<અક્ષર *>(Out8) - Res.data());
  પાછા Res;
}

સ્થિર શૂન્ય quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  માટે (unsigned અક્ષર C : S) {
    જો (C == 0x22 || C == 0x5C)
      OS << '\\';
    જો (C >= 0x20) {
      OS << C;
      ચાલુ;
    }
    OS << '\\';
    પસંદગી (C) {
    // A few characters are common enough to make short escapes worthwhile.
    સ્થિતિ '\t':
      OS << 't';
      તોડો;
    સ્થિતિ '\n':
      OS << 'n';
      તોડો;
    સ્થિતિ '\r':
      OS << 'r';
      તોડો;
    મૂળભૂત:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      તોડો;
    }
  }
  OS << '\"';
}

શૂન્ય llvm::json::OStream::કિંમત_2(અચલ કિંમત &V) {
  પસંદગી (V.kind()) {
  સ્થિતિ કિંમત::Null:
    કિંમત_Begin();
    OS << "null";
    પાછા;
  સ્થિતિ કિંમત::Boolean:
    કિંમત_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    પાછા;
  સ્થિતિ કિંમત::સંખ્યા:
    કિંમત_Begin();
    જો (V.Type == કિંમત::T_Integer)
      OS << *V.getAsInteger();
    નહીંતર જો (V.Type == કિંમત::T_UINT64)
      OS << *V.getAsUINT64();
    નહીંતર
      OS << format("%.*g", std::numeric_limits<દ્વિગુણ>::max_digits10,
                   *V.get_As_સંખ્યા());
    પાછા;
  સ્થિતિ કિંમત::String:
    કિંમત_Begin();
    quote(OS, *V.getAsString());
    પાછા;
  સ્થિતિ કિંમત::Array:
    પાછા array([&] {
      માટે (અચલ કિંમત &E : *V.getAsArray())
        કિંમત_2(E);
    });
  સ્થિતિ કિંમત::Object:
    પાછા object([&] {
      માટે (અચલ Object::કિંમત_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

શૂન્ય llvm::json::OStream::કિંમત_Begin() {
  પ્રતિજ્ઞા(ઢગલો.back().Ctx != Object && "Only attributes allowed here");
  જો (ઢગલો.back().Has_કિંમત) {
    પ્રતિજ્ઞા(ઢગલો.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  જો (ઢગલો.back().Ctx == Array)
    newline();
  flushComment();
  ઢગલો.back().Has_કિંમત = સાચું;
}

શૂન્ય OStream::comment(llvm::StringRef Comment) {
  પ્રતિજ્ઞા(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

શૂન્ય OStream::flushComment() {
  જો (PendingComment.empty())
    પાછા;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  જ્યારે (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    જો (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } નહીંતર {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  જો (ઢગલો.size() > 1 && ઢગલો.back().Ctx == Singleton) {
    જો (IndentSize)
      OS << ' ';
  } નહીંતર {
    newline();
  }
}

શૂન્ય llvm::json::OStream::newline() {
  જો (IndentSize) {
    OS.લિખ('\n');
    OS.indent(Indent);
  }
}

શૂન્ય llvm::json::OStream::arrayBegin() {
  કિંમત_Begin();
  ઢગલો.emplace_back();
  ઢગલો.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

શૂન્ય llvm::json::OStream::arrayEnd() {
  પ્રતિજ્ઞા(ઢગલો.back().Ctx == Array);
  Indent -= IndentSize;
  જો (ઢગલો.back().Has_કિંમત)
    newline();
  OS << ']';
  પ્રતિજ્ઞા(PendingComment.empty());
  ઢગલો.કાઢો_back();
  પ્રતિજ્ઞા(!ઢગલો.empty());
}

શૂન્ય llvm::json::OStream::objectBegin() {
  કિંમત_Begin();
  ઢગલો.emplace_back();
  ઢગલો.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

શૂન્ય llvm::json::OStream::objectEnd() {
  પ્રતિજ્ઞા(ઢગલો.back().Ctx == Object);
  Indent -= IndentSize;
  જો (ઢગલો.back().Has_કિંમત)
    newline();
  OS << '}';
  પ્રતિજ્ઞા(PendingComment.empty());
  ઢગલો.કાઢો_back();
  પ્રતિજ્ઞા(!ઢગલો.empty());
}

શૂન્ય llvm::json::OStream::attributeBegin(llvm::StringRef ચાવી) {
  પ્રતિજ્ઞા(ઢગલો.back().Ctx == Object);
  જો (ઢગલો.back().Has_કિંમત)
    OS << ',';
  newline();
  flushComment();
  ઢગલો.back().Has_કિંમત = સાચું;
  ઢગલો.emplace_back();
  ઢગલો.back().Ctx = Singleton;
  જો (LLVM_LIKELY(isUTF8(ચાવી))) {
    quote(OS, ચાવી);
  } નહીંતર {
    પ્રતિજ્ઞા(ખોટું && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(ચાવી));
  }
  OS.લિખ(':');
  જો (IndentSize)
    OS.લિખ(' ');
}

શૂન્ય llvm::json::OStream::attributeEnd() {
  પ્રતિજ્ઞા(ઢગલો.back().Ctx == Singleton);
  પ્રતિજ્ઞા(ઢગલો.back().Has_કિંમત && "Attribute must have a value");
  પ્રતિજ્ઞા(PendingComment.empty());
  ઢગલો.કાઢો_back();
  પ્રતિજ્ઞા(ઢગલો.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_કિંમત_Begin() {
  કિંમત_Begin();
  ઢગલો.emplace_back();
  ઢગલો.back().Ctx = Raw_કિંમત;
  પાછા OS;
}

શૂન્ય llvm::json::OStream::raw_કિંમત_End() {
  પ્રતિજ્ઞા(ઢગલો.back().Ctx == Raw_કિંમત);
  ઢગલો.કાઢો_back();
}

} // namespace json
} // namespace llvm

શૂન્ય llvm::format_provider<llvm::json::કિંમત>::format(
    અચલ llvm::json::કિંમત &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  જો (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).કિંમત_2(E);
}

