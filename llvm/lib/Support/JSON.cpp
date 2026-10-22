//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/JSON.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/NativeFormatting.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cctype>
#अन्तर्भाव <cerrno>
#अन्तर्भाव <optional>

नामाकाश llvm {
नामाकाश json {

मानम् &Object::operator[](ध्रुव Object_कुञ्चिका &K) {
  प्रत्यागम try_emplace(K, nullptr).first->getSecond();
}
मानम् &Object::operator[](Object_कुञ्चिका &&K) {
  प्रत्यागम try_emplace(std::move(K), nullptr).first->getSecond();
}
मानम् *Object::get(StringRef K) {
  auto I = find(K);
  जइ (I == end())
    प्रत्यागम nullptr;
  प्रत्यागम &I->second;
}
ध्रुव मानम् *Object::get(StringRef K) ध्रुव {
  auto I = find(K);
  जइ (I == end())
    प्रत्यागम nullptr;
  प्रत्यागम &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ध्रुव {
  जइ (auto *V = get(K))
    प्रत्यागम V->getAsNull();
  प्रत्यागम std::nullopt;
}
std::optional<तार्किक> Object::getBoolean(StringRef K) ध्रुव {
  जइ (auto *V = get(K))
    प्रत्यागम V->getAsBoolean();
  प्रत्यागम std::nullopt;
}
std::optional<द्विगुण> Object::get_सङ्ख्या(StringRef K) ध्रुव {
  जइ (auto *V = get(K))
    प्रत्यागम V->get_As_सङ्ख्या();
  प्रत्यागम std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ध्रुव {
  जइ (auto *V = get(K))
    प्रत्यागम V->getAsInteger();
  प्रत्यागम std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ध्रुव {
  जइ (auto *V = get(K))
    प्रत्यागम V->getAsString();
  प्रत्यागम std::nullopt;
}
ध्रुव json::Object *Object::getObject(StringRef K) ध्रुव {
  जइ (auto *V = get(K))
    प्रत्यागम V->getAsObject();
  प्रत्यागम nullptr;
}
json::Object *Object::getObject(StringRef K) {
  जइ (auto *V = get(K))
    प्रत्यागम V->getAsObject();
  प्रत्यागम nullptr;
}
ध्रुव json::Array *Object::getArray(StringRef K) ध्रुव {
  जइ (auto *V = get(K))
    प्रत्यागम V->getAsArray();
  प्रत्यागम nullptr;
}
json::Array *Object::getArray(StringRef K) {
  जइ (auto *V = get(K))
    प्रत्यागम V->getAsArray();
  प्रत्यागम nullptr;
}
तार्किक operator==(ध्रुव Object &LHS, ध्रुव Object &RHS) { प्रत्यागम LHS.M == RHS.M; }

Array::Array(std::initializer_list<मानम्> Elements) {
  V.reserve(Elements.size());
  प्रति (ध्रुव मानम् &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

मानम्::मानम्(std::initializer_list<मानम्> Elements)
    : मानम्(json::Array(Elements)) {}

शून्य मानम्::copyFrom(ध्रुव मानम् &M) {
  Type = M.Type;
  विकल्प (Type) {
  पक्ष T_Null:
  पक्ष T_Boolean:
  पक्ष T_Double:
  पक्ष T_Integer:
  पक्ष T_UINT64:
    memcpy(&Union, &M.Union, परिमाण(Union));
    भङ्ग;
  पक्ष T_StringRef:
    create<StringRef>(M.as<StringRef>());
    भङ्ग;
  पक्ष T_String:
    create<std::string>(M.as<std::string>());
    भङ्ग;
  पक्ष T_Object:
    create<json::Object>(M.as<json::Object>());
    भङ्ग;
  पक्ष T_Array:
    create<json::Array>(M.as<json::Array>());
    भङ्ग;
  }
}

शून्य मानम्::moveFrom(ध्रुव मानम् &&M) {
  Type = M.Type;
  विकल्प (Type) {
  पक्ष T_Null:
  पक्ष T_Boolean:
  पक्ष T_Double:
  पक्ष T_Integer:
  पक्ष T_UINT64:
    memcpy(&Union, &M.Union, परिमाण(Union));
    भङ्ग;
  पक्ष T_StringRef:
    create<StringRef>(M.as<StringRef>());
    भङ्ग;
  पक्ष T_String:
    create<std::string>(std::move(M.as<std::string>()));
    भङ्ग;
  पक्ष T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    भङ्ग;
  पक्ष T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    भङ्ग;
  }
  const_cast<मानम् &>(M).destroy();
  M.Type = T_Null;
}

शून्य मानम्::destroy() {
  विकल्प (Type) {
  पक्ष T_Null:
  पक्ष T_Boolean:
  पक्ष T_Double:
  पक्ष T_Integer:
  पक्ष T_UINT64:
    भङ्ग;
  पक्ष T_StringRef:
    as<StringRef>().~StringRef();
    भङ्ग;
  पक्ष T_String:
    as<std::string>().~basic_string();
    भङ्ग;
  पक्ष T_Object:
    as<json::Object>().~Object();
    भङ्ग;
  पक्ष T_Array:
    as<json::Array>().~Array();
    भङ्ग;
  }
}

शून्य मानम्::print(llvm::raw_ostream &OS) ध्रुव { OS << *एतत्; }

#जइ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य मानम्::dump() ध्रुव {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

तार्किक operator==(ध्रुव मानम् &L, ध्रुव मानम् &R) {
  जइ (L.kind() != R.kind())
    प्रत्यागम मिच्छ;
  विकल्प (L.kind()) {
  पक्ष मानम्::Null:
    प्रत्यागम *L.getAsNull() == *R.getAsNull();
  पक्ष मानम्::Boolean:
    प्रत्यागम *L.getAsBoolean() == *R.getAsBoolean();
  पक्ष मानम्::सङ्ख्या:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    जइ (L.Type == मानम्::T_Integer || R.Type == मानम्::T_Integer)
      प्रत्यागम L.getAsInteger() == R.getAsInteger();
    प्रत्यागम *L.get_As_सङ्ख्या() == *R.get_As_सङ्ख्या();
  पक्ष मानम्::String:
    प्रत्यागम *L.getAsString() == *R.getAsString();
  पक्ष मानम्::Array:
    प्रत्यागम *L.getAsArray() == *R.getAsArray();
  पक्ष मानम्::Object:
    प्रत्यागम *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

शून्य Path::report(llvm::StringLiteral सन्देशः_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned गणनम्_2 = 0;
  ध्रुव Path *P;
  प्रति (P = एतत्; P->Parent != nullptr; P = P->Parent)
    ++गणनम्_2;
  Path::मूलम्_2 *R = P->Seg.मूलम्_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = सन्देशः_2;
  R->ErrorPath.resize(गणनम्_2);
  auto It = R->ErrorPath.begin();
  प्रति (P = एतत्; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::मूलम्_2::getError() ध्रुव {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  जइ (ErrorPath.empty()) {
    जइ (!Name.empty())
      OS << " when parsing " << Name;
  } अण्णहा {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    प्रति (ध्रुव Path::Segment &S : llvm::reverse(ErrorPath)) {
      जइ (S.isField())
        OS << '.' << S.field();
      अण्णहा
        OS << '[' << S.index() << ']';
    }
  }
  प्रत्यागम createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ध्रुव Object::मानम्_type *> sortedElements(ध्रुव Object &O) {
  std::vector<ध्रुव Object::मानम्_type *> Elements;
  प्रति (ध्रुव auto &E : O)
    Elements.प्रक्षिप_back(&E);
  llvm::sort(Elements,
             [](ध्रुव Object::मानम्_type *L, ध्रुव Object::मानम्_type *R) {
               प्रत्यागम L->first < R->first;
             });
  प्रत्यागम Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
स्थिर शून्य abbreviate(ध्रुव मानम् &V, OStream &JOS) {
  विकल्प (V.kind()) {
  पक्ष मानम्::Array:
    JOS.raw_मानम्(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    भङ्ग;
  पक्ष मानम्::Object:
    JOS.raw_मानम्(V.getAsObject()->empty() ? "{}" : "{ ... }");
    भङ्ग;
  पक्ष मानम्::String: {
    llvm::StringRef S = *V.getAsString();
    जइ (S.size() < 40) {
      JOS.मानम्_2(V);
    } अण्णहा {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.मानम्_2(Truncated);
    }
    भङ्ग;
  }
  स्वभाव:
    JOS.मानम्_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
स्थिर शून्य abbreviateChildren(ध्रुव मानम् &V, OStream &JOS) {
  विकल्प (V.kind()) {
  पक्ष मानम्::Array:
    JOS.array([&] {
      प्रति (ध्रुव auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    भङ्ग;
  पक्ष मानम्::Object:
    JOS.object([&] {
      प्रति (ध्रुव auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    भङ्ग;
  स्वभाव:
    JOS.मानम्_2(V);
  }
}

शून्य Path::मूलम्_2::printErrorContext(ध्रुव मानम् &R, raw_ostream &OS) ध्रुव {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_मानम् = [&](ध्रुव मानम् &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    जइ (Path.empty()) // We reached our target.
      प्रत्यागम HighlightCurrent();
    ध्रुव Segment &S = Path.back(); // Path is in reverse order.
    जइ (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ध्रुव Object *O = V.getAsObject();
      जइ (!O || !O->get(FieldName))
        प्रत्यागम HighlightCurrent();
      JOS.object([&] {
        प्रति (ध्रुव auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          जइ (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          अण्णहा
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } अण्णहा {
      // Current node is an array, path names an element.
      ध्रुव Array *A = V.getAsArray();
      जइ (!A || S.index() >= A->size())
        प्रत्यागम HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        प्रति (ध्रुव auto &V : *A) {
          जइ (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          अण्णहा
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_मानम्(R, ErrorPath, Print_मानम्);
}

नामाकाश {
// Simple recursive-descent JSON parser.
वर्ग Parser {
सार्वजनिक:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  तार्किक checkUTF8() {
    size_t ErrOffset;
    जइ (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      प्रत्यागम सच्च;
    P = Start + ErrOffset; // For line/column calculation.
    प्रत्यागम parseError("Invalid UTF-8 sequence");
  }

  तार्किक parse_मानम्(मानम् &Out);

  तार्किक assertEnd() {
    eatWhitespace();
    जइ (P == End)
      प्रत्यागम सच्च;
    प्रत्यागम parseError("Text after end of document");
  }

  Error takeError() {
    प्रतिज्ञा(Err);
    प्रत्यागम std::move(*Err);
  }

गुप्त:
  शून्य eatWhitespace() {
    जाव (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  तार्किक parse_सङ्ख्या(वर्ण First, मानम् &Out);
  तार्किक parseString(std::string &Out);
  तार्किक parseUnicode(std::string &Out);
  तार्किक parseError(ध्रुव वर्ण *सन्देशः_2); // always returns false

  वर्ण next() { प्रत्यागम P == End ? 0 : *P++; }
  वर्ण peek() { प्रत्यागम P == End ? 0 : *P; }
  स्थिर तार्किक is_सङ्ख्या(वर्ण C) {
    प्रत्यागम C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ध्रुव वर्ण *Start, *P, *End;
};
} // namespace

तार्किक Parser::parse_मानम्(मानम् &Out) {
  eatWhitespace();
  जइ (P == End)
    प्रत्यागम parseError("Unexpected EOF");
  विकल्प (वर्ण C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  पक्ष 'n':
    Out = nullptr;
    प्रत्यागम (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  पक्ष 't':
    Out = सच्च;
    प्रत्यागम (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  पक्ष 'f':
    Out = मिच्छ;
    प्रत्यागम (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  पक्ष '"': {
    std::string S;
    जइ (parseString(S)) {
      Out = std::move(S);
      प्रत्यागम सच्च;
    }
    प्रत्यागम मिच्छ;
  }
  पक्ष '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    जइ (peek() == ']') {
      ++P;
      प्रत्यागम सच्च;
    }
    प्रति (;;) {
      A.emplace_back(nullptr);
      जइ (!parse_मानम्(A.back()))
        प्रत्यागम मिच्छ;
      eatWhitespace();
      विकल्प (next()) {
      पक्ष ',':
        eatWhitespace();
        अनुवर्त;
      पक्ष ']':
        प्रत्यागम सच्च;
      स्वभाव:
        प्रत्यागम parseError("Expected , or ] after array element");
      }
    }
  }
  पक्ष '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    जइ (peek() == '}') {
      ++P;
      प्रत्यागम सच्च;
    }
    प्रति (;;) {
      जइ (next() != '"')
        प्रत्यागम parseError("Expected object key");
      std::string K;
      जइ (!parseString(K))
        प्रत्यागम मिच्छ;
      eatWhitespace();
      जइ (next() != ':')
        प्रत्यागम parseError("Expected : after object key");
      eatWhitespace();
      जइ (!parse_मानम्(O[std::move(K)]))
        प्रत्यागम मिच्छ;
      eatWhitespace();
      विकल्प (next()) {
      पक्ष ',':
        eatWhitespace();
        अनुवर्त;
      पक्ष '}':
        प्रत्यागम सच्च;
      स्वभाव:
        प्रत्यागम parseError("Expected , or } after object property");
      }
    }
  }
  स्वभाव:
    जइ (is_सङ्ख्या(C))
      प्रत्यागम parse_सङ्ख्या(C, Out);
    प्रत्यागम parseError("Invalid JSON value");
  }
}

तार्किक Parser::parse_सङ्ख्या(वर्ण First, मानम् &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.प्रक्षिप_back(First);
  जाव (is_सङ्ख्या(peek()))
    S.प्रक्षिप_back(next());
  वर्ण *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  जइ (End == S.end() && errno != ERANGE) {
    Out = I;
    प्रत्यागम सच्च;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  जइ (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    जइ (End == S.end() && errno != ERANGE) {
      Out = UI;
      प्रत्यागम सच्च;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  प्रत्यागम End == S.end() || parseError("Invalid JSON value (number?)");
}

तार्किक Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  प्रति (वर्ण C = next(); C != '"'; C = next()) {
    जइ (LLVM_UNLIKELY(P == End))
      प्रत्यागम parseError("Unterminated string");
    जइ (LLVM_UNLIKELY((C & 0x1f) == C))
      प्रत्यागम parseError("Control character in string");
    जइ (LLVM_LIKELY(C != '\\')) {
      Out.प्रक्षिप_back(C);
      अनुवर्त;
    }
    // Handle escape sequence.
    विकल्प (C = next()) {
    पक्ष '"':
    पक्ष '\\':
    पक्ष '/':
      Out.प्रक्षिप_back(C);
      भङ्ग;
    पक्ष 'b':
      Out.प्रक्षिप_back('\b');
      भङ्ग;
    पक्ष 'f':
      Out.प्रक्षिप_back('\f');
      भङ्ग;
    पक्ष 'n':
      Out.प्रक्षिप_back('\n');
      भङ्ग;
    पक्ष 'r':
      Out.प्रक्षिप_back('\r');
      भङ्ग;
    पक्ष 't':
      Out.प्रक्षिप_back('\t');
      भङ्ग;
    पक्ष 'u':
      जइ (!parseUnicode(Out))
        प्रत्यागम मिच्छ;
      भङ्ग;
    स्वभाव:
      प्रत्यागम parseError("Invalid escape sequence");
    }
  }
  प्रत्यागम सच्च;
}

स्थिर शून्य encodeUtf8(uint32_t Rune, std::string &Out) {
  जइ (Rune < 0x80) {
    Out.प्रक्षिप_back(Rune & 0x7F);
  } अण्णहा जइ (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.प्रक्षिप_back(FirstByte);
    Out.प्रक्षिप_back(SecondByte);
  } अण्णहा जइ (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.प्रक्षिप_back(FirstByte);
    Out.प्रक्षिप_back(SecondByte);
    Out.प्रक्षिप_back(ThirdByte);
  } अण्णहा जइ (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.प्रक्षिप_back(FirstByte);
    Out.प्रक्षिप_back(SecondByte);
    Out.प्रक्षिप_back(ThirdByte);
    Out.प्रक्षिप_back(FourthByte);
  } अण्णहा {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
तार्किक Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [एतत्](uint16_t &Out) -> तार्किक {
    Out = 0;
    वर्ण Bytes[] = {next(), next(), next(), next()};
    प्रति (unsigned वर्ण C : Bytes) {
      जइ (!std::isxdigit(C))
        प्रत्यागम parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    प्रत्यागम सच्च;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  जइ (!Parse4Hex(First))
    प्रत्यागम मिच्छ;

  // We loop to allow proper surrogate-pair error handling.
  जाव (सच्च) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    जइ (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      प्रत्यागम सच्च;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    जइ (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      प्रत्यागम सच्च;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    जइ (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      प्रत्यागम सच्च;
    }
    P += 2;
    uint16_t Second;
    जइ (!Parse4Hex(Second))
      प्रत्यागम मिच्छ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    जइ (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      अनुवर्त;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    प्रत्यागम सच्च;
  }
}

तार्किक Parser::parseError(ध्रुव वर्ण *सन्देशः_2) {
  पूर्णाङ्क पङ्क्तिः = 1;
  ध्रुव वर्ण *Start_Of_पङ्क्तिः = Start;
  प्रति (ध्रुव वर्ण *X = Start; X < P; ++X) {
    जइ (*X == 0x0A) {
      ++पङ्क्तिः;
      Start_Of_पङ्क्तिः = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(सन्देशः_2, पङ्क्तिः, P - Start_Of_पङ्क्तिः, P - Start));
  प्रत्यागम मिच्छ;
}

Expected<मानम्> parse(StringRef JSON) {
  Parser P(JSON);
  मानम् E = nullptr;
  जइ (P.checkUTF8())
    जइ (P.parse_मानम्(E))
      जइ (P.assertEnd())
        प्रत्यागम std::move(E);
  प्रत्यागम P.takeError();
}

वर्ण ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
शून्य ParseError::log(llvm::raw_ostream &OS) ध्रुव {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", पङ्क्तिः, Column, Offset, सन्देशः_2);
}

तार्किक isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  जइ (LLVM_LIKELY(isASCII(S)))
    प्रत्यागम सच्च;

  ध्रुव UTF8 *Data = reinterpret_cast<ध्रुव UTF8 *>(S.data()), *Rest = Data;
  जइ (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    प्रत्यागम सच्च;

  जइ (ErrOffset)
    *ErrOffset = Rest - Data;
  प्रत्यागम मिच्छ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  ध्रुव UTF8 *In8 = reinterpret_cast<ध्रुव UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  ध्रुव UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<वर्ण *>(Out8) - Res.data());
  प्रत्यागम Res;
}

स्थिर शून्य quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  प्रति (unsigned वर्ण C : S) {
    जइ (C == 0x22 || C == 0x5C)
      OS << '\\';
    जइ (C >= 0x20) {
      OS << C;
      अनुवर्त;
    }
    OS << '\\';
    विकल्प (C) {
    // A few characters are common enough to make short escapes worthwhile.
    पक्ष '\t':
      OS << 't';
      भङ्ग;
    पक्ष '\n':
      OS << 'n';
      भङ्ग;
    पक्ष '\r':
      OS << 'r';
      भङ्ग;
    स्वभाव:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      भङ्ग;
    }
  }
  OS << '\"';
}

शून्य llvm::json::OStream::मानम्_2(ध्रुव मानम् &V) {
  विकल्प (V.kind()) {
  पक्ष मानम्::Null:
    मानम्_Begin();
    OS << "null";
    प्रत्यागम;
  पक्ष मानम्::Boolean:
    मानम्_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    प्रत्यागम;
  पक्ष मानम्::सङ्ख्या:
    मानम्_Begin();
    जइ (V.Type == मानम्::T_Integer)
      OS << *V.getAsInteger();
    अण्णहा जइ (V.Type == मानम्::T_UINT64)
      OS << *V.getAsUINT64();
    अण्णहा
      OS << format("%.*g", std::numeric_limits<द्विगुण>::max_digits10,
                   *V.get_As_सङ्ख्या());
    प्रत्यागम;
  पक्ष मानम्::String:
    मानम्_Begin();
    quote(OS, *V.getAsString());
    प्रत्यागम;
  पक्ष मानम्::Array:
    प्रत्यागम array([&] {
      प्रति (ध्रुव मानम् &E : *V.getAsArray())
        मानम्_2(E);
    });
  पक्ष मानम्::Object:
    प्रत्यागम object([&] {
      प्रति (ध्रुव Object::मानम्_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

शून्य llvm::json::OStream::मानम्_Begin() {
  प्रतिज्ञा(राशिः.back().Ctx != Object && "Only attributes allowed here");
  जइ (राशिः.back().Has_मानम्) {
    प्रतिज्ञा(राशिः.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  जइ (राशिः.back().Ctx == Array)
    newline();
  flushComment();
  राशिः.back().Has_मानम् = सच्च;
}

शून्य OStream::comment(llvm::StringRef Comment) {
  प्रतिज्ञा(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

शून्य OStream::flushComment() {
  जइ (PendingComment.empty())
    प्रत्यागम;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  जाव (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    जइ (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } अण्णहा {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  जइ (राशिः.size() > 1 && राशिः.back().Ctx == Singleton) {
    जइ (IndentSize)
      OS << ' ';
  } अण्णहा {
    newline();
  }
}

शून्य llvm::json::OStream::newline() {
  जइ (IndentSize) {
    OS.लिख('\n');
    OS.indent(Indent);
  }
}

शून्य llvm::json::OStream::arrayBegin() {
  मानम्_Begin();
  राशिः.emplace_back();
  राशिः.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

शून्य llvm::json::OStream::arrayEnd() {
  प्रतिज्ञा(राशिः.back().Ctx == Array);
  Indent -= IndentSize;
  जइ (राशिः.back().Has_मानम्)
    newline();
  OS << ']';
  प्रतिज्ञा(PendingComment.empty());
  राशिः.निष्कासय_back();
  प्रतिज्ञा(!राशिः.empty());
}

शून्य llvm::json::OStream::objectBegin() {
  मानम्_Begin();
  राशिः.emplace_back();
  राशिः.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

शून्य llvm::json::OStream::objectEnd() {
  प्रतिज्ञा(राशिः.back().Ctx == Object);
  Indent -= IndentSize;
  जइ (राशिः.back().Has_मानम्)
    newline();
  OS << '}';
  प्रतिज्ञा(PendingComment.empty());
  राशिः.निष्कासय_back();
  प्रतिज्ञा(!राशिः.empty());
}

शून्य llvm::json::OStream::attributeBegin(llvm::StringRef कुञ्चिका) {
  प्रतिज्ञा(राशिः.back().Ctx == Object);
  जइ (राशिः.back().Has_मानम्)
    OS << ',';
  newline();
  flushComment();
  राशिः.back().Has_मानम् = सच्च;
  राशिः.emplace_back();
  राशिः.back().Ctx = Singleton;
  जइ (LLVM_LIKELY(isUTF8(कुञ्चिका))) {
    quote(OS, कुञ्चिका);
  } अण्णहा {
    प्रतिज्ञा(मिच्छ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(कुञ्चिका));
  }
  OS.लिख(':');
  जइ (IndentSize)
    OS.लिख(' ');
}

शून्य llvm::json::OStream::attributeEnd() {
  प्रतिज्ञा(राशिः.back().Ctx == Singleton);
  प्रतिज्ञा(राशिः.back().Has_मानम् && "Attribute must have a value");
  प्रतिज्ञा(PendingComment.empty());
  राशिः.निष्कासय_back();
  प्रतिज्ञा(राशिः.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_मानम्_Begin() {
  मानम्_Begin();
  राशिः.emplace_back();
  राशिः.back().Ctx = Raw_मानम्;
  प्रत्यागम OS;
}

शून्य llvm::json::OStream::raw_मानम्_End() {
  प्रतिज्ञा(राशिः.back().Ctx == Raw_मानम्);
  राशिः.निष्कासय_back();
}

} // namespace json
} // namespace llvm

शून्य llvm::format_provider<llvm::json::मानम्>::format(
    ध्रुव llvm::json::मानम् &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  जइ (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).मानम्_2(E);
}

