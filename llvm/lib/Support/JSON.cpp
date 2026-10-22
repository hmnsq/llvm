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

मान &Object::चालक[](अचर Object_साँचो &K) {
  फर्काऊ try_emplace(K, nullptr).first->getSecond();
}
मान &Object::चालक[](Object_साँचो &&K) {
  फर्काऊ try_emplace(std::move(K), nullptr).first->getSecond();
}
मान *Object::get(StringRef K) {
  स्वतः I = find(K);
  यदि (I == end())
    फर्काऊ nullptr;
  फर्काऊ &I->second;
}
अचर मान *Object::get(StringRef K) अचर {
  स्वतः I = find(K);
  यदि (I == end())
    फर्काऊ nullptr;
  फर्काऊ &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) अचर {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->getAsNull();
  फर्काऊ std::nullopt;
}
std::optional<तार्किक> Object::getBoolean(StringRef K) अचर {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->getAsBoolean();
  फर्काऊ std::nullopt;
}
std::optional<द्विगुण> Object::get_सङ्ख्या(StringRef K) अचर {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->get_As_सङ्ख्या();
  फर्काऊ std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) अचर {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->getAsInteger();
  फर्काऊ std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) अचर {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->getAsString();
  फर्काऊ std::nullopt;
}
अचर json::Object *Object::getObject(StringRef K) अचर {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->getAsObject();
  फर्काऊ nullptr;
}
json::Object *Object::getObject(StringRef K) {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->getAsObject();
  फर्काऊ nullptr;
}
अचर json::Array *Object::getArray(StringRef K) अचर {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->getAsArray();
  फर्काऊ nullptr;
}
json::Array *Object::getArray(StringRef K) {
  यदि (स्वतः *V = get(K))
    फर्काऊ V->getAsArray();
  फर्काऊ nullptr;
}
तार्किक चालक==(अचर Object &LHS, अचर Object &RHS) { फर्काऊ LHS.M == RHS.M; }

Array::Array(std::initializer_list<मान> Elements) {
  V.reserve(Elements.size());
  लागि (अचर मान &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

मान::मान(std::initializer_list<मान> Elements)
    : मान(json::Array(Elements)) {}

शून्य मान::copyFrom(अचर मान &M) {
  Type = M.Type;
  विकल्प (Type) {
  पक्ष T_Null:
  पक्ष T_Boolean:
  पक्ष T_Double:
  पक्ष T_Integer:
  पक्ष T_UINT64:
    स_नकल(&Union, &M.Union, परिमाण(Union));
    रोक;
  पक्ष T_StringRef:
    create<StringRef>(M.as<StringRef>());
    रोक;
  पक्ष T_String:
    create<std::string>(M.as<std::string>());
    रोक;
  पक्ष T_Object:
    create<json::Object>(M.as<json::Object>());
    रोक;
  पक्ष T_Array:
    create<json::Array>(M.as<json::Array>());
    रोक;
  }
}

शून्य मान::moveFrom(अचर मान &&M) {
  Type = M.Type;
  विकल्प (Type) {
  पक्ष T_Null:
  पक्ष T_Boolean:
  पक्ष T_Double:
  पक्ष T_Integer:
  पक्ष T_UINT64:
    स_नकल(&Union, &M.Union, परिमाण(Union));
    रोक;
  पक्ष T_StringRef:
    create<StringRef>(M.as<StringRef>());
    रोक;
  पक्ष T_String:
    create<std::string>(std::move(M.as<std::string>()));
    रोक;
  पक्ष T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    रोक;
  पक्ष T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    रोक;
  }
  const_cast<मान &>(M).destroy();
  M.Type = T_Null;
}

शून्य मान::destroy() {
  विकल्प (Type) {
  पक्ष T_Null:
  पक्ष T_Boolean:
  पक्ष T_Double:
  पक्ष T_Integer:
  पक्ष T_UINT64:
    रोक;
  पक्ष T_StringRef:
    as<StringRef>().~StringRef();
    रोक;
  पक्ष T_String:
    as<std::string>().~basic_string();
    रोक;
  पक्ष T_Object:
    as<json::Object>().~Object();
    रोक;
  पक्ष T_Array:
    as<json::Array>().~Array();
    रोक;
  }
}

शून्य मान::print(llvm::raw_ostream &OS) अचर { OS << *एतत्; }

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य मान::dump() अचर {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

तार्किक चालक==(अचर मान &L, अचर मान &R) {
  यदि (L.kind() != R.kind())
    फर्काऊ असत्य;
  विकल्प (L.kind()) {
  पक्ष मान::Null:
    फर्काऊ *L.getAsNull() == *R.getAsNull();
  पक्ष मान::Boolean:
    फर्काऊ *L.getAsBoolean() == *R.getAsBoolean();
  पक्ष मान::सङ्ख्या:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    यदि (L.Type == मान::T_Integer || R.Type == मान::T_Integer)
      फर्काऊ L.getAsInteger() == R.getAsInteger();
    फर्काऊ *L.get_As_सङ्ख्या() == *R.get_As_सङ्ख्या();
  पक्ष मान::String:
    फर्काऊ *L.getAsString() == *R.getAsString();
  पक्ष मान::Array:
    फर्काऊ *L.getAsArray() == *R.getAsArray();
  पक्ष मान::Object:
    फर्काऊ *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

शून्य Path::report(llvm::StringLiteral सन्देश_2) {
  // Walk up to the root context, and count the number of segments.
  अचिन्हित गन्ती_2 = 0;
  अचर Path *P;
  लागि (P = एतत्; P->Parent != nullptr; P = P->Parent)
    ++गन्ती_2;
  Path::जरा_2 *R = P->Seg.जरा_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = सन्देश_2;
  R->ErrorPath.resize(गन्ती_2);
  स्वतः It = R->ErrorPath.begin();
  लागि (P = एतत्; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::जरा_2::getError() अचर {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  यदि (ErrorPath.empty()) {
    यदि (!Name.empty())
      OS << " when parsing " << Name;
  } अन्यथा {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    लागि (अचर Path::Segment &S : llvm::reverse(ErrorPath)) {
      यदि (S.isField())
        OS << '.' << S.field();
      अन्यथा
        OS << '[' << S.index() << ']';
    }
  }
  फर्काऊ createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<अचर Object::मान_type *> sortedElements(अचर Object &O) {
  std::vector<अचर Object::मान_type *> Elements;
  लागि (अचर स्वतः &E : O)
    Elements.थप_back(&E);
  llvm::sort(Elements,
             [](अचर Object::मान_type *L, अचर Object::मान_type *R) {
               फर्काऊ L->first < R->first;
             });
  फर्काऊ Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
स्थिर शून्य abbreviate(अचर मान &V, OStream &JOS) {
  विकल्प (V.kind()) {
  पक्ष मान::Array:
    JOS.raw_मान(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    रोक;
  पक्ष मान::Object:
    JOS.raw_मान(V.getAsObject()->empty() ? "{}" : "{ ... }");
    रोक;
  पक्ष मान::String: {
    llvm::StringRef S = *V.getAsString();
    यदि (S.size() < 40) {
      JOS.मान_2(V);
    } अन्यथा {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.मान_2(Truncated);
    }
    रोक;
  }
  स्वभाव:
    JOS.मान_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
स्थिर शून्य abbreviateChildren(अचर मान &V, OStream &JOS) {
  विकल्प (V.kind()) {
  पक्ष मान::Array:
    JOS.array([&] {
      लागि (अचर स्वतः &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    रोक;
  पक्ष मान::Object:
    JOS.object([&] {
      लागि (अचर स्वतः *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    रोक;
  स्वभाव:
    JOS.मान_2(V);
  }
}

शून्य Path::जरा_2::printErrorContext(अचर मान &R, raw_ostream &OS) अचर {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  स्वतः Print_मान = [&](अचर मान &V, ArrayRef<Segment> Path, स्वतः &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    स्वतः HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    यदि (Path.empty()) // We reached our target.
      फर्काऊ HighlightCurrent();
    अचर Segment &S = Path.back(); // Path is in reverse order.
    यदि (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      अचर Object *O = V.getAsObject();
      यदि (!O || !O->get(FieldName))
        फर्काऊ HighlightCurrent();
      JOS.object([&] {
        लागि (अचर स्वतः *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          यदि (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          अन्यथा
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } अन्यथा {
      // Current node is an array, path names an element.
      अचर Array *A = V.getAsArray();
      यदि (!A || S.index() >= A->size())
        फर्काऊ HighlightCurrent();
      JOS.array([&] {
        अचिन्हित Current = 0;
        लागि (अचर स्वतः &V : *A) {
          यदि (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          अन्यथा
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_मान(R, ErrorPath, Print_मान);
}

नामाकाश {
// Simple recursive-descent JSON parser.
वर्ग Parser {
सार्वजनिक:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  तार्किक checkUTF8() {
    माप_प्रकार ErrOffset;
    यदि (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      फर्काऊ सत्य;
    P = Start + ErrOffset; // For line/column calculation.
    फर्काऊ parseError("Invalid UTF-8 sequence");
  }

  तार्किक parse_मान(मान &Out);

  तार्किक assertEnd() {
    eatWhitespace();
    यदि (P == End)
      फर्काऊ सत्य;
    फर्काऊ parseError("Text after end of document");
  }

  Error takeError() {
    प्रतिज्ञा(Err);
    फर्काऊ std::move(*Err);
  }

गुप्त:
  शून्य eatWhitespace() {
    जबसम्म (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  तार्किक parse_सङ्ख्या(वर्ण First, मान &Out);
  तार्किक parseString(std::string &Out);
  तार्किक parseUnicode(std::string &Out);
  तार्किक parseError(अचर वर्ण *सन्देश_2); // always returns false

  वर्ण next() { फर्काऊ P == End ? 0 : *P++; }
  वर्ण peek() { फर्काऊ P == End ? 0 : *P; }
  स्थिर तार्किक is_सङ्ख्या(वर्ण C) {
    फर्काऊ C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  अचर वर्ण *Start, *P, *End;
};
} // namespace

तार्किक Parser::parse_मान(मान &Out) {
  eatWhitespace();
  यदि (P == End)
    फर्काऊ parseError("Unexpected EOF");
  विकल्प (वर्ण C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  पक्ष 'n':
    Out = nullptr;
    फर्काऊ (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  पक्ष 't':
    Out = सत्य;
    फर्काऊ (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  पक्ष 'f':
    Out = असत्य;
    फर्काऊ (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  पक्ष '"': {
    std::string S;
    यदि (parseString(S)) {
      Out = std::move(S);
      फर्काऊ सत्य;
    }
    फर्काऊ असत्य;
  }
  पक्ष '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    यदि (peek() == ']') {
      ++P;
      फर्काऊ सत्य;
    }
    लागि (;;) {
      A.emplace_back(nullptr);
      यदि (!parse_मान(A.back()))
        फर्काऊ असत्य;
      eatWhitespace();
      विकल्प (next()) {
      पक्ष ',':
        eatWhitespace();
        जारी;
      पक्ष ']':
        फर्काऊ सत्य;
      स्वभाव:
        फर्काऊ parseError("Expected , or ] after array element");
      }
    }
  }
  पक्ष '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    यदि (peek() == '}') {
      ++P;
      फर्काऊ सत्य;
    }
    लागि (;;) {
      यदि (next() != '"')
        फर्काऊ parseError("Expected object key");
      std::string K;
      यदि (!parseString(K))
        फर्काऊ असत्य;
      eatWhitespace();
      यदि (next() != ':')
        फर्काऊ parseError("Expected : after object key");
      eatWhitespace();
      यदि (!parse_मान(O[std::move(K)]))
        फर्काऊ असत्य;
      eatWhitespace();
      विकल्प (next()) {
      पक्ष ',':
        eatWhitespace();
        जारी;
      पक्ष '}':
        फर्काऊ सत्य;
      स्वभाव:
        फर्काऊ parseError("Expected , or } after object property");
      }
    }
  }
  स्वभाव:
    यदि (is_सङ्ख्या(C))
      फर्काऊ parse_सङ्ख्या(C, Out);
    फर्काऊ parseError("Invalid JSON value");
  }
}

तार्किक Parser::parse_सङ्ख्या(वर्ण First, मान &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.थप_back(First);
  जबसम्म (is_सङ्ख्या(peek()))
    S.थप_back(next());
  वर्ण *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  त्रुटि_सं = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  यदि (End == S.end() && त्रुटि_सं != दुस्फल) {
    Out = I;
    फर्काऊ सत्य;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  यदि (First != '-') {
    त्रुटि_सं = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    यदि (End == S.end() && त्रुटि_सं != दुस्फल) {
      Out = UI;
      फर्काऊ सत्य;
    }
  }
  // If it's not an integer
  Out = std::म_से_भग्न(S.c_str(), &End);
  फर्काऊ End == S.end() || parseError("Invalid JSON value (number?)");
}

तार्किक Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  लागि (वर्ण C = next(); C != '"'; C = next()) {
    यदि (LLVM_UNLIKELY(P == End))
      फर्काऊ parseError("Unterminated string");
    यदि (LLVM_UNLIKELY((C & 0x1f) == C))
      फर्काऊ parseError("Control character in string");
    यदि (LLVM_LIKELY(C != '\\')) {
      Out.थप_back(C);
      जारी;
    }
    // Handle escape sequence.
    विकल्प (C = next()) {
    पक्ष '"':
    पक्ष '\\':
    पक्ष '/':
      Out.थप_back(C);
      रोक;
    पक्ष 'b':
      Out.थप_back('\b');
      रोक;
    पक्ष 'f':
      Out.थप_back('\f');
      रोक;
    पक्ष 'n':
      Out.थप_back('\n');
      रोक;
    पक्ष 'r':
      Out.थप_back('\r');
      रोक;
    पक्ष 't':
      Out.थप_back('\t');
      रोक;
    पक्ष 'u':
      यदि (!parseUnicode(Out))
        फर्काऊ असत्य;
      रोक;
    स्वभाव:
      फर्काऊ parseError("Invalid escape sequence");
    }
  }
  फर्काऊ सत्य;
}

स्थिर शून्य encodeUtf8(uint32_t Rune, std::string &Out) {
  यदि (Rune < 0x80) {
    Out.थप_back(Rune & 0x7F);
  } अन्यथा यदि (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.थप_back(FirstByte);
    Out.थप_back(SecondByte);
  } अन्यथा यदि (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.थप_back(FirstByte);
    Out.थप_back(SecondByte);
    Out.थप_back(ThirdByte);
  } अन्यथा यदि (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.थप_back(FirstByte);
    Out.थप_back(SecondByte);
    Out.थप_back(ThirdByte);
    Out.थप_back(FourthByte);
  } अन्यथा {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
तार्किक Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  स्वतः Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  स्वतः Parse4Hex = [एतत्](uint16_t &Out) -> तार्किक {
    Out = 0;
    वर्ण Bytes[] = {next(), next(), next(), next()};
    लागि (अचिन्हित वर्ण C : Bytes) {
      यदि (!std::है_षष्ठादशक(C))
        फर्काऊ parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    फर्काऊ सत्य;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  यदि (!Parse4Hex(First))
    फर्काऊ असत्य;

  // We loop to allow proper surrogate-pair error handling.
  जबसम्म (सत्य) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    यदि (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      फर्काऊ सत्य;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    यदि (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      फर्काऊ सत्य;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    यदि (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      फर्काऊ सत्य;
    }
    P += 2;
    uint16_t Second;
    यदि (!Parse4Hex(Second))
      फर्काऊ असत्य;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    यदि (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      जारी;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    फर्काऊ सत्य;
  }
}

तार्किक Parser::parseError(अचर वर्ण *सन्देश_2) {
  पूर्णाङ्क पङ्क्ति = 1;
  अचर वर्ण *Start_Of_पङ्क्ति = Start;
  लागि (अचर वर्ण *X = Start; X < P; ++X) {
    यदि (*X == 0x0A) {
      ++पङ्क्ति;
      Start_Of_पङ्क्ति = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(सन्देश_2, पङ्क्ति, P - Start_Of_पङ्क्ति, P - Start));
  फर्काऊ असत्य;
}

Expected<मान> parse(StringRef JSON) {
  Parser P(JSON);
  मान E = nullptr;
  यदि (P.checkUTF8())
    यदि (P.parse_मान(E))
      यदि (P.assertEnd())
        फर्काऊ std::move(E);
  फर्काऊ P.takeError();
}

वर्ण ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
शून्य ParseError::log(llvm::raw_ostream &OS) अचर {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", पङ्क्ति, Column, Offset, सन्देश_2);
}

तार्किक isUTF8(llvm::StringRef S, माप_प्रकार *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  यदि (LLVM_LIKELY(isASCII(S)))
    फर्काऊ सत्य;

  अचर UTF8 *Data = reinterpret_cast<अचर UTF8 *>(S.data()), *Rest = Data;
  यदि (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    फर्काऊ सत्य;

  यदि (ErrOffset)
    *ErrOffset = Rest - Data;
  फर्काऊ असत्य;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  अचर UTF8 *In8 = reinterpret_cast<अचर UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  अचर UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<वर्ण *>(Out8) - Res.data());
  फर्काऊ Res;
}

स्थिर शून्य quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  लागि (अचिन्हित वर्ण C : S) {
    यदि (C == 0x22 || C == 0x5C)
      OS << '\\';
    यदि (C >= 0x20) {
      OS << C;
      जारी;
    }
    OS << '\\';
    विकल्प (C) {
    // A few characters are common enough to make short escapes worthwhile.
    पक्ष '\t':
      OS << 't';
      रोक;
    पक्ष '\n':
      OS << 'n';
      रोक;
    पक्ष '\r':
      OS << 'r';
      रोक;
    स्वभाव:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      रोक;
    }
  }
  OS << '\"';
}

शून्य llvm::json::OStream::मान_2(अचर मान &V) {
  विकल्प (V.kind()) {
  पक्ष मान::Null:
    मान_Begin();
    OS << "null";
    फर्काऊ;
  पक्ष मान::Boolean:
    मान_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    फर्काऊ;
  पक्ष मान::सङ्ख्या:
    मान_Begin();
    यदि (V.Type == मान::T_Integer)
      OS << *V.getAsInteger();
    अन्यथा यदि (V.Type == मान::T_UINT64)
      OS << *V.getAsUINT64();
    अन्यथा
      OS << format("%.*g", std::numeric_limits<द्विगुण>::max_digits10,
                   *V.get_As_सङ्ख्या());
    फर्काऊ;
  पक्ष मान::String:
    मान_Begin();
    quote(OS, *V.getAsString());
    फर्काऊ;
  पक्ष मान::Array:
    फर्काऊ array([&] {
      लागि (अचर मान &E : *V.getAsArray())
        मान_2(E);
    });
  पक्ष मान::Object:
    फर्काऊ object([&] {
      लागि (अचर Object::मान_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

शून्य llvm::json::OStream::मान_Begin() {
  प्रतिज्ञा(थाक.back().Ctx != Object && "Only attributes allowed here");
  यदि (थाक.back().Has_मान) {
    प्रतिज्ञा(थाक.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  यदि (थाक.back().Ctx == Array)
    newline();
  flushComment();
  थाक.back().Has_मान = सत्य;
}

शून्य OStream::comment(llvm::StringRef Comment) {
  प्रतिज्ञा(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

शून्य OStream::flushComment() {
  यदि (PendingComment.empty())
    फर्काऊ;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  जबसम्म (!PendingComment.empty()) {
    स्वतः Pos = PendingComment.find("*/");
    यदि (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } अन्यथा {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  यदि (थाक.size() > 1 && थाक.back().Ctx == Singleton) {
    यदि (IndentSize)
      OS << ' ';
  } अन्यथा {
    newline();
  }
}

शून्य llvm::json::OStream::newline() {
  यदि (IndentSize) {
    OS.लिख('\n');
    OS.indent(Indent);
  }
}

शून्य llvm::json::OStream::arrayBegin() {
  मान_Begin();
  थाक.emplace_back();
  थाक.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

शून्य llvm::json::OStream::arrayEnd() {
  प्रतिज्ञा(थाक.back().Ctx == Array);
  Indent -= IndentSize;
  यदि (थाक.back().Has_मान)
    newline();
  OS << ']';
  प्रतिज्ञा(PendingComment.empty());
  थाक.झिक_back();
  प्रतिज्ञा(!थाक.empty());
}

शून्य llvm::json::OStream::objectBegin() {
  मान_Begin();
  थाक.emplace_back();
  थाक.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

शून्य llvm::json::OStream::objectEnd() {
  प्रतिज्ञा(थाक.back().Ctx == Object);
  Indent -= IndentSize;
  यदि (थाक.back().Has_मान)
    newline();
  OS << '}';
  प्रतिज्ञा(PendingComment.empty());
  थाक.झिक_back();
  प्रतिज्ञा(!थाक.empty());
}

शून्य llvm::json::OStream::attributeBegin(llvm::StringRef साँचो) {
  प्रतिज्ञा(थाक.back().Ctx == Object);
  यदि (थाक.back().Has_मान)
    OS << ',';
  newline();
  flushComment();
  थाक.back().Has_मान = सत्य;
  थाक.emplace_back();
  थाक.back().Ctx = Singleton;
  यदि (LLVM_LIKELY(isUTF8(साँचो))) {
    quote(OS, साँचो);
  } अन्यथा {
    प्रतिज्ञा(असत्य && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(साँचो));
  }
  OS.लिख(':');
  यदि (IndentSize)
    OS.लिख(' ');
}

शून्य llvm::json::OStream::attributeEnd() {
  प्रतिज्ञा(थाक.back().Ctx == Singleton);
  प्रतिज्ञा(थाक.back().Has_मान && "Attribute must have a value");
  प्रतिज्ञा(PendingComment.empty());
  थाक.झिक_back();
  प्रतिज्ञा(थाक.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_मान_Begin() {
  मान_Begin();
  थाक.emplace_back();
  थाक.back().Ctx = Raw_मान;
  फर्काऊ OS;
}

शून्य llvm::json::OStream::raw_मान_End() {
  प्रतिज्ञा(थाक.back().Ctx == Raw_मान);
  थाक.झिक_back();
}

} // namespace json
} // namespace llvm

शून्य llvm::format_provider<llvm::json::मान>::format(
    अचर llvm::json::मान &E, raw_ostream &OS, StringRef Options) {
  अचिन्हित IndentAmount = 0;
  यदि (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).मान_2(E);
}

