//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#समावेश "llvm/Support/JSON.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/ConvertUTF.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/NativeFormatting.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cctype>
#समावेश <cerrno>
#समावेश <optional>

namespace llvm {
namespace json {

मूल्य &Object::चालक[](स्थिर Object_कुंजी &K) {
  वापस try_emplace(K, nullptr).first->getSecond();
}
मूल्य &Object::चालक[](Object_कुंजी &&K) {
  वापस try_emplace(std::move(K), nullptr).first->getSecond();
}
मूल्य *Object::लो(StringRef K) {
  स्वतः I = find(K);
  अगर (I == end())
    वापस nullptr;
  वापस &I->second;
}
स्थिर मूल्य *Object::लो(StringRef K) स्थिर {
  स्वतः I = find(K);
  अगर (I == end())
    वापस nullptr;
  वापस &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) स्थिर {
  अगर (स्वतः *V = लो(K))
    वापस V->getAsNull();
  वापस std::nullopt;
}
std::optional<bool> Object::getBoolean(StringRef K) स्थिर {
  अगर (स्वतः *V = लो(K))
    वापस V->getAsBoolean();
  वापस std::nullopt;
}
std::optional<द्विगुन> Object::get_संख्या(StringRef K) स्थिर {
  अगर (स्वतः *V = लो(K))
    वापस V->get_As_संख्या();
  वापस std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) स्थिर {
  अगर (स्वतः *V = लो(K))
    वापस V->getAsInteger();
  वापस std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) स्थिर {
  अगर (स्वतः *V = लो(K))
    वापस V->getAsString();
  वापस std::nullopt;
}
स्थिर json::Object *Object::getObject(StringRef K) स्थिर {
  अगर (स्वतः *V = लो(K))
    वापस V->getAsObject();
  वापस nullptr;
}
json::Object *Object::getObject(StringRef K) {
  अगर (स्वतः *V = लो(K))
    वापस V->getAsObject();
  वापस nullptr;
}
स्थिर json::Array *Object::getArray(StringRef K) स्थिर {
  अगर (स्वतः *V = लो(K))
    वापस V->getAsArray();
  वापस nullptr;
}
json::Array *Object::getArray(StringRef K) {
  अगर (स्वतः *V = लो(K))
    वापस V->getAsArray();
  वापस nullptr;
}
bool चालक==(स्थिर Object &LHS, स्थिर Object &RHS) { वापस LHS.M == RHS.M; }

Array::Array(std::initializer_list<मूल्य> Elements) {
  V.reserve(Elements.आकार_2());
  क्रम (स्थिर मूल्य &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

मूल्य::मूल्य(std::initializer_list<मूल्य> Elements)
    : मूल्य(json::Array(Elements)) {}

व्योम मूल्य::copyFrom(स्थिर मूल्य &M) {
  Type = M.Type;
  चयन (Type) {
  हाल T_Null:
  हाल T_Boolean:
  हाल T_Double:
  हाल T_Integer:
  हाल T_UINT64:
    स_नकल(&Union, &M.Union, माप(Union));
    अवरोध;
  हाल T_StringRef:
    create<StringRef>(M.as<StringRef>());
    अवरोध;
  हाल T_String:
    create<std::string>(M.as<std::string>());
    अवरोध;
  हाल T_Object:
    create<json::Object>(M.as<json::Object>());
    अवरोध;
  हाल T_Array:
    create<json::Array>(M.as<json::Array>());
    अवरोध;
  }
}

व्योम मूल्य::moveFrom(स्थिर मूल्य &&M) {
  Type = M.Type;
  चयन (Type) {
  हाल T_Null:
  हाल T_Boolean:
  हाल T_Double:
  हाल T_Integer:
  हाल T_UINT64:
    स_नकल(&Union, &M.Union, माप(Union));
    अवरोध;
  हाल T_StringRef:
    create<StringRef>(M.as<StringRef>());
    अवरोध;
  हाल T_String:
    create<std::string>(std::move(M.as<std::string>()));
    अवरोध;
  हाल T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    अवरोध;
  हाल T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    अवरोध;
  }
  const_cast<मूल्य &>(M).destroy();
  M.Type = T_Null;
}

व्योम मूल्य::destroy() {
  चयन (Type) {
  हाल T_Null:
  हाल T_Boolean:
  हाल T_Double:
  हाल T_Integer:
  हाल T_UINT64:
    अवरोध;
  हाल T_StringRef:
    as<StringRef>().~StringRef();
    अवरोध;
  हाल T_String:
    as<std::string>().~basic_string();
    अवरोध;
  हाल T_Object:
    as<json::Object>().~Object();
    अवरोध;
  हाल T_Array:
    as<json::Array>().~Array();
    अवरोध;
  }
}

व्योम मूल्य::print(llvm::raw_ostream &OS) स्थिर { OS << *यह; }

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम मूल्य::dump() स्थिर {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#पूर्ण_अगर

bool चालक==(स्थिर मूल्य &L, स्थिर मूल्य &R) {
  अगर (L.kind() != R.kind())
    वापस false;
  चयन (L.kind()) {
  हाल मूल्य::Null:
    वापस *L.getAsNull() == *R.getAsNull();
  हाल मूल्य::Boolean:
    वापस *L.getAsBoolean() == *R.getAsBoolean();
  हाल मूल्य::संख्या:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    अगर (L.Type == मूल्य::T_Integer || R.Type == मूल्य::T_Integer)
      वापस L.getAsInteger() == R.getAsInteger();
    वापस *L.get_As_संख्या() == *R.get_As_संख्या();
  हाल मूल्य::String:
    वापस *L.getAsString() == *R.getAsString();
  हाल मूल्य::Array:
    वापस *L.getAsArray() == *R.getAsArray();
  हाल मूल्य::Object:
    वापस *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

व्योम Path::report(llvm::StringLiteral संदेश_2) {
  // Walk up to the root context, and count the number of segments.
  अचिन्हित गिनती_2 = 0;
  स्थिर Path *P;
  क्रम (P = यह; P->Parent != nullptr; P = P->Parent)
    ++गिनती_2;
  Path::मूल_2 *R = P->Seg.मूल_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = संदेश_2;
  R->ErrorPath.resize(गिनती_2);
  स्वतः It = R->ErrorPath.begin();
  क्रम (P = यह; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::मूल_2::getError() स्थिर {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  अगर (ErrorPath.empty()) {
    अगर (!Name.empty())
      OS << " when parsing " << Name;
  } अन्यथा {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    क्रम (स्थिर Path::Segment &S : llvm::reverse(ErrorPath)) {
      अगर (S.isField())
        OS << '.' << S.field();
      अन्यथा
        OS << '[' << S.सूचकांक() << ']';
    }
  }
  वापस createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<स्थिर Object::मूल्य_type *> sortedElements(स्थिर Object &O) {
  std::vector<स्थिर Object::मूल्य_type *> Elements;
  क्रम (स्थिर स्वतः &E : O)
    Elements.धकेलो_back(&E);
  llvm::sort(Elements,
             [](स्थिर Object::मूल्य_type *L, स्थिर Object::मूल्य_type *R) {
               वापस L->first < R->first;
             });
  वापस Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
अटल व्योम abbreviate(स्थिर मूल्य &V, OStream &JOS) {
  चयन (V.kind()) {
  हाल मूल्य::Array:
    JOS.raw_मूल्य(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    अवरोध;
  हाल मूल्य::Object:
    JOS.raw_मूल्य(V.getAsObject()->empty() ? "{}" : "{ ... }");
    अवरोध;
  हाल मूल्य::String: {
    llvm::StringRef S = *V.getAsString();
    अगर (S.आकार_2() < 40) {
      JOS.मूल्य_2(V);
    } अन्यथा {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.मूल्य_2(Truncated);
    }
    अवरोध;
  }
  शेष:
    JOS.मूल्य_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
अटल व्योम abbreviateChildren(स्थिर मूल्य &V, OStream &JOS) {
  चयन (V.kind()) {
  हाल मूल्य::Array:
    JOS.array([&] {
      क्रम (स्थिर स्वतः &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    अवरोध;
  हाल मूल्य::Object:
    JOS.object([&] {
      क्रम (स्थिर स्वतः *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    अवरोध;
  शेष:
    JOS.मूल्य_2(V);
  }
}

व्योम Path::मूल_2::printErrorContext(स्थिर मूल्य &R, raw_ostream &OS) स्थिर {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  स्वतः Print_मूल्य = [&](स्थिर मूल्य &V, ArrayRef<Segment> Path, स्वतः &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    स्वतः HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.तथ्य(), ErrorMessage.आकार_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    अगर (Path.empty()) // We reached our target.
      वापस HighlightCurrent();
    स्थिर Segment &S = Path.back(); // Path is in reverse order.
    अगर (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      स्थिर Object *O = V.getAsObject();
      अगर (!O || !O->लो(FieldName))
        वापस HighlightCurrent();
      JOS.object([&] {
        क्रम (स्थिर स्वतः *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          अगर (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          अन्यथा
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } अन्यथा {
      // Current node is an array, path names an element.
      स्थिर Array *A = V.getAsArray();
      अगर (!A || S.सूचकांक() >= A->आकार_2())
        वापस HighlightCurrent();
      JOS.array([&] {
        अचिन्हित Current = 0;
        क्रम (स्थिर स्वतः &V : *A) {
          अगर (Current++ == S.सूचकांक())
            Recurse(V, Path.drop_back(), Recurse);
          अन्यथा
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_मूल्य(R, ErrorPath, Print_मूल्य);
}

namespace {
// Simple recursive-descent JSON parser.
श्रेणी Parser {
खुला:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  bool checkUTF8() {
    माप_प्रकार ErrOffset;
    अगर (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      वापस true;
    P = Start + ErrOffset; // For line/column calculation.
    वापस parseError("Invalid UTF-8 sequence");
  }

  bool parse_मूल्य(मूल्य &Out);

  bool assertEnd() {
    eatWhitespace();
    अगर (P == End)
      वापस true;
    वापस parseError("Text after end of document");
  }

  Error takeError() {
    निश्चित(Err);
    वापस std::move(*Err);
  }

निजी:
  व्योम eatWhitespace() {
    जबतक (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  bool parse_संख्या(अक्षर First, मूल्य &Out);
  bool parseString(std::string &Out);
  bool parseUnicode(std::string &Out);
  bool parseError(स्थिर अक्षर *संदेश_2); // always returns false

  अक्षर अग्र() { वापस P == End ? 0 : *P++; }
  अक्षर peek() { वापस P == End ? 0 : *P; }
  अटल bool is_संख्या(अक्षर C) {
    वापस C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  स्थिर अक्षर *Start, *P, *End;
};
} // namespace

bool Parser::parse_मूल्य(मूल्य &Out) {
  eatWhitespace();
  अगर (P == End)
    वापस parseError("Unexpected EOF");
  चयन (अक्षर C = अग्र()) {
  // Bare null/true/false are easy - first char identifies them.
  हाल 'n':
    Out = nullptr;
    वापस (अग्र() == 'u' && अग्र() == 'l' && अग्र() == 'l') ||
           parseError("Invalid JSON value (null?)");
  हाल 't':
    Out = true;
    वापस (अग्र() == 'r' && अग्र() == 'u' && अग्र() == 'e') ||
           parseError("Invalid JSON value (true?)");
  हाल 'f':
    Out = false;
    वापस (अग्र() == 'a' && अग्र() == 'l' && अग्र() == 's' && अग्र() == 'e') ||
           parseError("Invalid JSON value (false?)");
  हाल '"': {
    std::string S;
    अगर (parseString(S)) {
      Out = std::move(S);
      वापस true;
    }
    वापस false;
  }
  हाल '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    अगर (peek() == ']') {
      ++P;
      वापस true;
    }
    क्रम (;;) {
      A.emplace_back(nullptr);
      अगर (!parse_मूल्य(A.back()))
        वापस false;
      eatWhitespace();
      चयन (अग्र()) {
      हाल ',':
        eatWhitespace();
        जारी;
      हाल ']':
        वापस true;
      शेष:
        वापस parseError("Expected , or ] after array element");
      }
    }
  }
  हाल '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    अगर (peek() == '}') {
      ++P;
      वापस true;
    }
    क्रम (;;) {
      अगर (अग्र() != '"')
        वापस parseError("Expected object key");
      std::string K;
      अगर (!parseString(K))
        वापस false;
      eatWhitespace();
      अगर (अग्र() != ':')
        वापस parseError("Expected : after object key");
      eatWhitespace();
      अगर (!parse_मूल्य(O[std::move(K)]))
        वापस false;
      eatWhitespace();
      चयन (अग्र()) {
      हाल ',':
        eatWhitespace();
        जारी;
      हाल '}':
        वापस true;
      शेष:
        वापस parseError("Expected , or } after object property");
      }
    }
  }
  शेष:
    अगर (is_संख्या(C))
      वापस parse_संख्या(C, Out);
    वापस parseError("Invalid JSON value");
  }
}

bool Parser::parse_संख्या(अक्षर First, मूल्य &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.धकेलो_back(First);
  जबतक (is_संख्या(peek()))
    S.धकेलो_back(अग्र());
  अक्षर *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  त्रुटि_सं = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  अगर (End == S.end() && त्रुटि_सं != दुस्फल) {
    Out = I;
    वापस true;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  अगर (First != '-') {
    त्रुटि_सं = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    अगर (End == S.end() && त्रुटि_सं != दुस्फल) {
      Out = UI;
      वापस true;
    }
  }
  // If it's not an integer
  Out = std::म_से_भग्न(S.c_str(), &End);
  वापस End == S.end() || parseError("Invalid JSON value (number?)");
}

bool Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  क्रम (अक्षर C = अग्र(); C != '"'; C = अग्र()) {
    अगर (LLVM_UNLIKELY(P == End))
      वापस parseError("Unterminated string");
    अगर (LLVM_UNLIKELY((C & 0x1f) == C))
      वापस parseError("Control character in string");
    अगर (LLVM_LIKELY(C != '\\')) {
      Out.धकेलो_back(C);
      जारी;
    }
    // Handle escape sequence.
    चयन (C = अग्र()) {
    हाल '"':
    हाल '\\':
    हाल '/':
      Out.धकेलो_back(C);
      अवरोध;
    हाल 'b':
      Out.धकेलो_back('\b');
      अवरोध;
    हाल 'f':
      Out.धकेलो_back('\f');
      अवरोध;
    हाल 'n':
      Out.धकेलो_back('\n');
      अवरोध;
    हाल 'r':
      Out.धकेलो_back('\r');
      अवरोध;
    हाल 't':
      Out.धकेलो_back('\t');
      अवरोध;
    हाल 'u':
      अगर (!parseUnicode(Out))
        वापस false;
      अवरोध;
    शेष:
      वापस parseError("Invalid escape sequence");
    }
  }
  वापस true;
}

अटल व्योम encodeUtf8(uint32_t Rune, std::string &Out) {
  अगर (Rune < 0x80) {
    Out.धकेलो_back(Rune & 0x7F);
  } अन्यथा अगर (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.धकेलो_back(FirstByte);
    Out.धकेलो_back(SecondByte);
  } अन्यथा अगर (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.धकेलो_back(FirstByte);
    Out.धकेलो_back(SecondByte);
    Out.धकेलो_back(ThirdByte);
  } अन्यथा अगर (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.धकेलो_back(FirstByte);
    Out.धकेलो_back(SecondByte);
    Out.धकेलो_back(ThirdByte);
    Out.धकेलो_back(FourthByte);
  } अन्यथा {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
bool Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  स्वतः Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  स्वतः Parse4Hex = [यह](uint16_t &Out) -> bool {
    Out = 0;
    अक्षर Bytes[] = {अग्र(), अग्र(), अग्र(), अग्र()};
    क्रम (अचिन्हित अक्षर C : Bytes) {
      अगर (!std::है_षष्ठादशक(C))
        वापस parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    वापस true;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  अगर (!Parse4Hex(First))
    वापस false;

  // We loop to allow proper surrogate-pair error handling.
  जबतक (true) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    अगर (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      वापस true;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    अगर (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      वापस true;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    अगर (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      वापस true;
    }
    P += 2;
    uint16_t Second;
    अगर (!Parse4Hex(Second))
      वापस false;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    अगर (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      जारी;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    वापस true;
  }
}

bool Parser::parseError(स्थिर अक्षर *संदेश_2) {
  पूर्णांक पंक्ति_2 = 1;
  स्थिर अक्षर *Start_Of_पंक्ति = Start;
  क्रम (स्थिर अक्षर *X = Start; X < P; ++X) {
    अगर (*X == 0x0A) {
      ++पंक्ति_2;
      Start_Of_पंक्ति = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(संदेश_2, पंक्ति_2, P - Start_Of_पंक्ति, P - Start));
  वापस false;
}

Expected<मूल्य> parse(StringRef JSON) {
  Parser P(JSON);
  मूल्य E = nullptr;
  अगर (P.checkUTF8())
    अगर (P.parse_मूल्य(E))
      अगर (P.assertEnd())
        वापस std::move(E);
  वापस P.takeError();
}

अक्षर ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
व्योम ParseError::log(llvm::raw_ostream &OS) स्थिर {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", पंक्ति_2, Column, Offset, संदेश_2);
}

bool isUTF8(llvm::StringRef S, माप_प्रकार *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  अगर (LLVM_LIKELY(isASCII(S)))
    वापस true;

  स्थिर UTF8 *तथ्य_2 = reinterpret_cast<स्थिर UTF8 *>(S.तथ्य()), *Rest = तथ्य_2;
  अगर (LLVM_LIKELY(isLegalUTF8String(&Rest, तथ्य_2 + S.आकार_2())))
    वापस true;

  अगर (ErrOffset)
    *ErrOffset = Rest - तथ्य_2;
  वापस false;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.आकार_2()); // 1 codepoint per byte suffices.
  स्थिर UTF8 *In8 = reinterpret_cast<स्थिर UTF8 *>(S.तथ्य());
  UTF32 *Out32 = Codepoints.तथ्य();
  ConvertUTF8toUTF32(&In8, In8 + S.आकार_2(), &Out32, Out32 + Codepoints.आकार_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.तथ्य());
  std::string Res(4 * Codepoints.आकार_2(), 0); // 4 bytes per codepoint suffice
  स्थिर UTF32 *In32 = Codepoints.तथ्य();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.आकार_2(), &Out8, Out8 + Res.आकार_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<अक्षर *>(Out8) - Res.तथ्य());
  वापस Res;
}

अटल व्योम quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  क्रम (अचिन्हित अक्षर C : S) {
    अगर (C == 0x22 || C == 0x5C)
      OS << '\\';
    अगर (C >= 0x20) {
      OS << C;
      जारी;
    }
    OS << '\\';
    चयन (C) {
    // A few characters are common enough to make short escapes worthwhile.
    हाल '\t':
      OS << 't';
      अवरोध;
    हाल '\n':
      OS << 'n';
      अवरोध;
    हाल '\r':
      OS << 'r';
      अवरोध;
    शेष:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      अवरोध;
    }
  }
  OS << '\"';
}

व्योम llvm::json::OStream::मूल्य_2(स्थिर मूल्य &V) {
  चयन (V.kind()) {
  हाल मूल्य::Null:
    मूल्य_Begin();
    OS << "null";
    वापस;
  हाल मूल्य::Boolean:
    मूल्य_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    वापस;
  हाल मूल्य::संख्या:
    मूल्य_Begin();
    अगर (V.Type == मूल्य::T_Integer)
      OS << *V.getAsInteger();
    अन्यथा अगर (V.Type == मूल्य::T_UINT64)
      OS << *V.getAsUINT64();
    अन्यथा
      OS << format("%.*g", std::numeric_limits<द्विगुन>::max_digits10,
                   *V.get_As_संख्या());
    वापस;
  हाल मूल्य::String:
    मूल्य_Begin();
    quote(OS, *V.getAsString());
    वापस;
  हाल मूल्य::Array:
    वापस array([&] {
      क्रम (स्थिर मूल्य &E : *V.getAsArray())
        मूल्य_2(E);
    });
  हाल मूल्य::Object:
    वापस object([&] {
      क्रम (स्थिर Object::मूल्य_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

व्योम llvm::json::OStream::मूल्य_Begin() {
  निश्चित(ढेर_2.back().Ctx != Object && "Only attributes allowed here");
  अगर (ढेर_2.back().Has_मूल्य) {
    निश्चित(ढेर_2.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  अगर (ढेर_2.back().Ctx == Array)
    newline();
  flushComment();
  ढेर_2.back().Has_मूल्य = true;
}

व्योम OStream::comment(llvm::StringRef Comment) {
  निश्चित(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

व्योम OStream::flushComment() {
  अगर (PendingComment.empty())
    वापस;
  OS << (Indent_आकार ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  जबतक (!PendingComment.empty()) {
    स्वतः Pos = PendingComment.find("*/");
    अगर (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } अन्यथा {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_आकार ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  अगर (ढेर_2.आकार_2() > 1 && ढेर_2.back().Ctx == Singleton) {
    अगर (Indent_आकार)
      OS << ' ';
  } अन्यथा {
    newline();
  }
}

व्योम llvm::json::OStream::newline() {
  अगर (Indent_आकार) {
    OS.ग_लिखो('\n');
    OS.indent(Indent);
  }
}

व्योम llvm::json::OStream::arrayBegin() {
  मूल्य_Begin();
  ढेर_2.emplace_back();
  ढेर_2.back().Ctx = Array;
  Indent += Indent_आकार;
  OS << '[';
}

व्योम llvm::json::OStream::arrayEnd() {
  निश्चित(ढेर_2.back().Ctx == Array);
  Indent -= Indent_आकार;
  अगर (ढेर_2.back().Has_मूल्य)
    newline();
  OS << ']';
  निश्चित(PendingComment.empty());
  ढेर_2.निकालो_back();
  निश्चित(!ढेर_2.empty());
}

व्योम llvm::json::OStream::objectBegin() {
  मूल्य_Begin();
  ढेर_2.emplace_back();
  ढेर_2.back().Ctx = Object;
  Indent += Indent_आकार;
  OS << '{';
}

व्योम llvm::json::OStream::objectEnd() {
  निश्चित(ढेर_2.back().Ctx == Object);
  Indent -= Indent_आकार;
  अगर (ढेर_2.back().Has_मूल्य)
    newline();
  OS << '}';
  निश्चित(PendingComment.empty());
  ढेर_2.निकालो_back();
  निश्चित(!ढेर_2.empty());
}

व्योम llvm::json::OStream::attributeBegin(llvm::StringRef कुंजी) {
  निश्चित(ढेर_2.back().Ctx == Object);
  अगर (ढेर_2.back().Has_मूल्य)
    OS << ',';
  newline();
  flushComment();
  ढेर_2.back().Has_मूल्य = true;
  ढेर_2.emplace_back();
  ढेर_2.back().Ctx = Singleton;
  अगर (LLVM_LIKELY(isUTF8(कुंजी))) {
    quote(OS, कुंजी);
  } अन्यथा {
    निश्चित(false && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(कुंजी));
  }
  OS.ग_लिखो(':');
  अगर (Indent_आकार)
    OS.ग_लिखो(' ');
}

व्योम llvm::json::OStream::attributeEnd() {
  निश्चित(ढेर_2.back().Ctx == Singleton);
  निश्चित(ढेर_2.back().Has_मूल्य && "Attribute must have a value");
  निश्चित(PendingComment.empty());
  ढेर_2.निकालो_back();
  निश्चित(ढेर_2.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_मूल्य_Begin() {
  मूल्य_Begin();
  ढेर_2.emplace_back();
  ढेर_2.back().Ctx = Raw_मूल्य;
  वापस OS;
}

व्योम llvm::json::OStream::raw_मूल्य_End() {
  निश्चित(ढेर_2.back().Ctx == Raw_मूल्य);
  ढेर_2.निकालो_back();
}

} // namespace json
} // namespace llvm

व्योम llvm::format_provider<llvm::json::मूल्य>::format(
    स्थिर llvm::json::मूल्य &E, raw_ostream &OS, StringRef Options) {
  अचिन्हित IndentAmount = 0;
  अगर (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).मूल्य_2(E);
}

