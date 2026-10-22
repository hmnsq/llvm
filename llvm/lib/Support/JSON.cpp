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

मूल्य &Object::चालक[](स्थिरांक Object_किल्ली &K) {
  परत try_emplace(K, nullptr).first->getSecond();
}
मूल्य &Object::चालक[](Object_किल्ली &&K) {
  परत try_emplace(std::move(K), nullptr).first->getSecond();
}
मूल्य *Object::get(StringRef K) {
  स्वतः I = find(K);
  जर (I == end())
    परत nullptr;
  परत &I->second;
}
स्थिरांक मूल्य *Object::get(StringRef K) स्थिरांक {
  स्वतः I = find(K);
  जर (I == end())
    परत nullptr;
  परत &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) स्थिरांक {
  जर (स्वतः *V = get(K))
    परत V->getAsNull();
  परत std::nullopt;
}
std::optional<तार्किक> Object::getBoolean(StringRef K) स्थिरांक {
  जर (स्वतः *V = get(K))
    परत V->getAsBoolean();
  परत std::nullopt;
}
std::optional<द्विगुण> Object::get_संख्या(StringRef K) स्थिरांक {
  जर (स्वतः *V = get(K))
    परत V->get_As_संख्या();
  परत std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) स्थिरांक {
  जर (स्वतः *V = get(K))
    परत V->getAsInteger();
  परत std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) स्थिरांक {
  जर (स्वतः *V = get(K))
    परत V->getAsString();
  परत std::nullopt;
}
स्थिरांक json::Object *Object::getObject(StringRef K) स्थिरांक {
  जर (स्वतः *V = get(K))
    परत V->getAsObject();
  परत nullptr;
}
json::Object *Object::getObject(StringRef K) {
  जर (स्वतः *V = get(K))
    परत V->getAsObject();
  परत nullptr;
}
स्थिरांक json::Array *Object::getArray(StringRef K) स्थिरांक {
  जर (स्वतः *V = get(K))
    परत V->getAsArray();
  परत nullptr;
}
json::Array *Object::getArray(StringRef K) {
  जर (स्वतः *V = get(K))
    परत V->getAsArray();
  परत nullptr;
}
तार्किक चालक==(स्थिरांक Object &LHS, स्थिरांक Object &RHS) { परत LHS.M == RHS.M; }

Array::Array(std::initializer_list<मूल्य> Elements) {
  V.reserve(Elements.size());
  साठी (स्थिरांक मूल्य &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

मूल्य::मूल्य(std::initializer_list<मूल्य> Elements)
    : मूल्य(json::Array(Elements)) {}

शून्य मूल्य::copyFrom(स्थिरांक मूल्य &M) {
  Type = M.Type;
  निवड (Type) {
  स्थिती T_Null:
  स्थिती T_Boolean:
  स्थिती T_Double:
  स्थिती T_Integer:
  स्थिती T_UINT64:
    स_नकल(&Union, &M.Union, आकार(Union));
    थांबा;
  स्थिती T_StringRef:
    create<StringRef>(M.as<StringRef>());
    थांबा;
  स्थिती T_String:
    create<std::string>(M.as<std::string>());
    थांबा;
  स्थिती T_Object:
    create<json::Object>(M.as<json::Object>());
    थांबा;
  स्थिती T_Array:
    create<json::Array>(M.as<json::Array>());
    थांबा;
  }
}

शून्य मूल्य::moveFrom(स्थिरांक मूल्य &&M) {
  Type = M.Type;
  निवड (Type) {
  स्थिती T_Null:
  स्थिती T_Boolean:
  स्थिती T_Double:
  स्थिती T_Integer:
  स्थिती T_UINT64:
    स_नकल(&Union, &M.Union, आकार(Union));
    थांबा;
  स्थिती T_StringRef:
    create<StringRef>(M.as<StringRef>());
    थांबा;
  स्थिती T_String:
    create<std::string>(std::move(M.as<std::string>()));
    थांबा;
  स्थिती T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    थांबा;
  स्थिती T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    थांबा;
  }
  const_cast<मूल्य &>(M).destroy();
  M.Type = T_Null;
}

शून्य मूल्य::destroy() {
  निवड (Type) {
  स्थिती T_Null:
  स्थिती T_Boolean:
  स्थिती T_Double:
  स्थिती T_Integer:
  स्थिती T_UINT64:
    थांबा;
  स्थिती T_StringRef:
    as<StringRef>().~StringRef();
    थांबा;
  स्थिती T_String:
    as<std::string>().~basic_string();
    थांबा;
  स्थिती T_Object:
    as<json::Object>().~Object();
    थांबा;
  स्थिती T_Array:
    as<json::Array>().~Array();
    थांबा;
  }
}

शून्य मूल्य::print(llvm::raw_ostream &OS) स्थिरांक { OS << *हे; }

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य मूल्य::dump() स्थिरांक {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

तार्किक चालक==(स्थिरांक मूल्य &L, स्थिरांक मूल्य &R) {
  जर (L.kind() != R.kind())
    परत असत्य;
  निवड (L.kind()) {
  स्थिती मूल्य::Null:
    परत *L.getAsNull() == *R.getAsNull();
  स्थिती मूल्य::Boolean:
    परत *L.getAsBoolean() == *R.getAsBoolean();
  स्थिती मूल्य::संख्या:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    जर (L.Type == मूल्य::T_Integer || R.Type == मूल्य::T_Integer)
      परत L.getAsInteger() == R.getAsInteger();
    परत *L.get_As_संख्या() == *R.get_As_संख्या();
  स्थिती मूल्य::String:
    परत *L.getAsString() == *R.getAsString();
  स्थिती मूल्य::Array:
    परत *L.getAsArray() == *R.getAsArray();
  स्थिती मूल्य::Object:
    परत *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

शून्य Path::report(llvm::StringLiteral संदेश_2) {
  // Walk up to the root context, and count the number of segments.
  अचिन्हित मोजणी_2 = 0;
  स्थिरांक Path *P;
  साठी (P = हे; P->Parent != nullptr; P = P->Parent)
    ++मोजणी_2;
  Path::मूळ_2 *R = P->Seg.मूळ_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = संदेश_2;
  R->ErrorPath.resize(मोजणी_2);
  स्वतः It = R->ErrorPath.begin();
  साठी (P = हे; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::मूळ_2::getError() स्थिरांक {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  जर (ErrorPath.empty()) {
    जर (!Name.empty())
      OS << " when parsing " << Name;
  } अन्यथा {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    साठी (स्थिरांक Path::Segment &S : llvm::reverse(ErrorPath)) {
      जर (S.isField())
        OS << '.' << S.field();
      अन्यथा
        OS << '[' << S.index() << ']';
    }
  }
  परत createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<स्थिरांक Object::मूल्य_type *> sortedElements(स्थिरांक Object &O) {
  std::vector<स्थिरांक Object::मूल्य_type *> Elements;
  साठी (स्थिरांक स्वतः &E : O)
    Elements.ढकला_back(&E);
  llvm::sort(Elements,
             [](स्थिरांक Object::मूल्य_type *L, स्थिरांक Object::मूल्य_type *R) {
               परत L->first < R->first;
             });
  परत Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
स्थिर शून्य abbreviate(स्थिरांक मूल्य &V, OStream &JOS) {
  निवड (V.kind()) {
  स्थिती मूल्य::Array:
    JOS.raw_मूल्य(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    थांबा;
  स्थिती मूल्य::Object:
    JOS.raw_मूल्य(V.getAsObject()->empty() ? "{}" : "{ ... }");
    थांबा;
  स्थिती मूल्य::String: {
    llvm::StringRef S = *V.getAsString();
    जर (S.size() < 40) {
      JOS.मूल्य_2(V);
    } अन्यथा {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.मूल्य_2(Truncated);
    }
    थांबा;
  }
  पूर्वनिर्धारित:
    JOS.मूल्य_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
स्थिर शून्य abbreviateChildren(स्थिरांक मूल्य &V, OStream &JOS) {
  निवड (V.kind()) {
  स्थिती मूल्य::Array:
    JOS.array([&] {
      साठी (स्थिरांक स्वतः &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    थांबा;
  स्थिती मूल्य::Object:
    JOS.object([&] {
      साठी (स्थिरांक स्वतः *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    थांबा;
  पूर्वनिर्धारित:
    JOS.मूल्य_2(V);
  }
}

शून्य Path::मूळ_2::printErrorContext(स्थिरांक मूल्य &R, raw_ostream &OS) स्थिरांक {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  स्वतः Print_मूल्य = [&](स्थिरांक मूल्य &V, ArrayRef<Segment> Path, स्वतः &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    स्वतः HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    जर (Path.empty()) // We reached our target.
      परत HighlightCurrent();
    स्थिरांक Segment &S = Path.back(); // Path is in reverse order.
    जर (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      स्थिरांक Object *O = V.getAsObject();
      जर (!O || !O->get(FieldName))
        परत HighlightCurrent();
      JOS.object([&] {
        साठी (स्थिरांक स्वतः *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          जर (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          अन्यथा
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } अन्यथा {
      // Current node is an array, path names an element.
      स्थिरांक Array *A = V.getAsArray();
      जर (!A || S.index() >= A->size())
        परत HighlightCurrent();
      JOS.array([&] {
        अचिन्हित Current = 0;
        साठी (स्थिरांक स्वतः &V : *A) {
          जर (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          अन्यथा
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_मूल्य(R, ErrorPath, Print_मूल्य);
}

नामाकाश {
// Simple recursive-descent JSON parser.
वर्ग Parser {
सार्वजनिक:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  तार्किक checkUTF8() {
    माप_प्रकार ErrOffset;
    जर (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      परत सत्य;
    P = Start + ErrOffset; // For line/column calculation.
    परत parseError("Invalid UTF-8 sequence");
  }

  तार्किक parse_मूल्य(मूल्य &Out);

  तार्किक assertEnd() {
    eatWhitespace();
    जर (P == End)
      परत सत्य;
    परत parseError("Text after end of document");
  }

  Error takeError() {
    प्रतिज्ञा(Err);
    परत std::move(*Err);
  }

खाजगी:
  शून्य eatWhitespace() {
    पर्यंत (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  तार्किक parse_संख्या(अक्षर First, मूल्य &Out);
  तार्किक parseString(std::string &Out);
  तार्किक parseUnicode(std::string &Out);
  तार्किक parseError(स्थिरांक अक्षर *संदेश_2); // always returns false

  अक्षर next() { परत P == End ? 0 : *P++; }
  अक्षर peek() { परत P == End ? 0 : *P; }
  स्थिर तार्किक is_संख्या(अक्षर C) {
    परत C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  स्थिरांक अक्षर *Start, *P, *End;
};
} // namespace

तार्किक Parser::parse_मूल्य(मूल्य &Out) {
  eatWhitespace();
  जर (P == End)
    परत parseError("Unexpected EOF");
  निवड (अक्षर C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  स्थिती 'n':
    Out = nullptr;
    परत (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  स्थिती 't':
    Out = सत्य;
    परत (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  स्थिती 'f':
    Out = असत्य;
    परत (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  स्थिती '"': {
    std::string S;
    जर (parseString(S)) {
      Out = std::move(S);
      परत सत्य;
    }
    परत असत्य;
  }
  स्थिती '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    जर (peek() == ']') {
      ++P;
      परत सत्य;
    }
    साठी (;;) {
      A.emplace_back(nullptr);
      जर (!parse_मूल्य(A.back()))
        परत असत्य;
      eatWhitespace();
      निवड (next()) {
      स्थिती ',':
        eatWhitespace();
        सुरूठेवा;
      स्थिती ']':
        परत सत्य;
      पूर्वनिर्धारित:
        परत parseError("Expected , or ] after array element");
      }
    }
  }
  स्थिती '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    जर (peek() == '}') {
      ++P;
      परत सत्य;
    }
    साठी (;;) {
      जर (next() != '"')
        परत parseError("Expected object key");
      std::string K;
      जर (!parseString(K))
        परत असत्य;
      eatWhitespace();
      जर (next() != ':')
        परत parseError("Expected : after object key");
      eatWhitespace();
      जर (!parse_मूल्य(O[std::move(K)]))
        परत असत्य;
      eatWhitespace();
      निवड (next()) {
      स्थिती ',':
        eatWhitespace();
        सुरूठेवा;
      स्थिती '}':
        परत सत्य;
      पूर्वनिर्धारित:
        परत parseError("Expected , or } after object property");
      }
    }
  }
  पूर्वनिर्धारित:
    जर (is_संख्या(C))
      परत parse_संख्या(C, Out);
    परत parseError("Invalid JSON value");
  }
}

तार्किक Parser::parse_संख्या(अक्षर First, मूल्य &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ढकला_back(First);
  पर्यंत (is_संख्या(peek()))
    S.ढकला_back(next());
  अक्षर *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  त्रुटि_सं = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  जर (End == S.end() && त्रुटि_सं != दुस्फल) {
    Out = I;
    परत सत्य;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  जर (First != '-') {
    त्रुटि_सं = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    जर (End == S.end() && त्रुटि_सं != दुस्फल) {
      Out = UI;
      परत सत्य;
    }
  }
  // If it's not an integer
  Out = std::म_से_भग्न(S.c_str(), &End);
  परत End == S.end() || parseError("Invalid JSON value (number?)");
}

तार्किक Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  साठी (अक्षर C = next(); C != '"'; C = next()) {
    जर (LLVM_UNLIKELY(P == End))
      परत parseError("Unterminated string");
    जर (LLVM_UNLIKELY((C & 0x1f) == C))
      परत parseError("Control character in string");
    जर (LLVM_LIKELY(C != '\\')) {
      Out.ढकला_back(C);
      सुरूठेवा;
    }
    // Handle escape sequence.
    निवड (C = next()) {
    स्थिती '"':
    स्थिती '\\':
    स्थिती '/':
      Out.ढकला_back(C);
      थांबा;
    स्थिती 'b':
      Out.ढकला_back('\b');
      थांबा;
    स्थिती 'f':
      Out.ढकला_back('\f');
      थांबा;
    स्थिती 'n':
      Out.ढकला_back('\n');
      थांबा;
    स्थिती 'r':
      Out.ढकला_back('\r');
      थांबा;
    स्थिती 't':
      Out.ढकला_back('\t');
      थांबा;
    स्थिती 'u':
      जर (!parseUnicode(Out))
        परत असत्य;
      थांबा;
    पूर्वनिर्धारित:
      परत parseError("Invalid escape sequence");
    }
  }
  परत सत्य;
}

स्थिर शून्य encodeUtf8(uint32_t Rune, std::string &Out) {
  जर (Rune < 0x80) {
    Out.ढकला_back(Rune & 0x7F);
  } अन्यथा जर (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.ढकला_back(FirstByte);
    Out.ढकला_back(SecondByte);
  } अन्यथा जर (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.ढकला_back(FirstByte);
    Out.ढकला_back(SecondByte);
    Out.ढकला_back(ThirdByte);
  } अन्यथा जर (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.ढकला_back(FirstByte);
    Out.ढकला_back(SecondByte);
    Out.ढकला_back(ThirdByte);
    Out.ढकला_back(FourthByte);
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
  स्वतः Parse4Hex = [हे](uint16_t &Out) -> तार्किक {
    Out = 0;
    अक्षर Bytes[] = {next(), next(), next(), next()};
    साठी (अचिन्हित अक्षर C : Bytes) {
      जर (!std::है_षष्ठादशक(C))
        परत parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    परत सत्य;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  जर (!Parse4Hex(First))
    परत असत्य;

  // We loop to allow proper surrogate-pair error handling.
  पर्यंत (सत्य) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    जर (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      परत सत्य;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    जर (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      परत सत्य;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    जर (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      परत सत्य;
    }
    P += 2;
    uint16_t Second;
    जर (!Parse4Hex(Second))
      परत असत्य;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    जर (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      सुरूठेवा;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    परत सत्य;
  }
}

तार्किक Parser::parseError(स्थिरांक अक्षर *संदेश_2) {
  पूर्णांक ओळ = 1;
  स्थिरांक अक्षर *Start_Of_ओळ = Start;
  साठी (स्थिरांक अक्षर *X = Start; X < P; ++X) {
    जर (*X == 0x0A) {
      ++ओळ;
      Start_Of_ओळ = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(संदेश_2, ओळ, P - Start_Of_ओळ, P - Start));
  परत असत्य;
}

Expected<मूल्य> parse(StringRef JSON) {
  Parser P(JSON);
  मूल्य E = nullptr;
  जर (P.checkUTF8())
    जर (P.parse_मूल्य(E))
      जर (P.assertEnd())
        परत std::move(E);
  परत P.takeError();
}

अक्षर ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
शून्य ParseError::log(llvm::raw_ostream &OS) स्थिरांक {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", ओळ, Column, Offset, संदेश_2);
}

तार्किक isUTF8(llvm::StringRef S, माप_प्रकार *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  जर (LLVM_LIKELY(isASCII(S)))
    परत सत्य;

  स्थिरांक UTF8 *Data = reinterpret_cast<स्थिरांक UTF8 *>(S.data()), *Rest = Data;
  जर (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    परत सत्य;

  जर (ErrOffset)
    *ErrOffset = Rest - Data;
  परत असत्य;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  स्थिरांक UTF8 *In8 = reinterpret_cast<स्थिरांक UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  स्थिरांक UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<अक्षर *>(Out8) - Res.data());
  परत Res;
}

स्थिर शून्य quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  साठी (अचिन्हित अक्षर C : S) {
    जर (C == 0x22 || C == 0x5C)
      OS << '\\';
    जर (C >= 0x20) {
      OS << C;
      सुरूठेवा;
    }
    OS << '\\';
    निवड (C) {
    // A few characters are common enough to make short escapes worthwhile.
    स्थिती '\t':
      OS << 't';
      थांबा;
    स्थिती '\n':
      OS << 'n';
      थांबा;
    स्थिती '\r':
      OS << 'r';
      थांबा;
    पूर्वनिर्धारित:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      थांबा;
    }
  }
  OS << '\"';
}

शून्य llvm::json::OStream::मूल्य_2(स्थिरांक मूल्य &V) {
  निवड (V.kind()) {
  स्थिती मूल्य::Null:
    मूल्य_Begin();
    OS << "null";
    परत;
  स्थिती मूल्य::Boolean:
    मूल्य_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    परत;
  स्थिती मूल्य::संख्या:
    मूल्य_Begin();
    जर (V.Type == मूल्य::T_Integer)
      OS << *V.getAsInteger();
    अन्यथा जर (V.Type == मूल्य::T_UINT64)
      OS << *V.getAsUINT64();
    अन्यथा
      OS << format("%.*g", std::numeric_limits<द्विगुण>::max_digits10,
                   *V.get_As_संख्या());
    परत;
  स्थिती मूल्य::String:
    मूल्य_Begin();
    quote(OS, *V.getAsString());
    परत;
  स्थिती मूल्य::Array:
    परत array([&] {
      साठी (स्थिरांक मूल्य &E : *V.getAsArray())
        मूल्य_2(E);
    });
  स्थिती मूल्य::Object:
    परत object([&] {
      साठी (स्थिरांक Object::मूल्य_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

शून्य llvm::json::OStream::मूल्य_Begin() {
  प्रतिज्ञा(चळत.back().Ctx != Object && "Only attributes allowed here");
  जर (चळत.back().Has_मूल्य) {
    प्रतिज्ञा(चळत.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  जर (चळत.back().Ctx == Array)
    newline();
  flushComment();
  चळत.back().Has_मूल्य = सत्य;
}

शून्य OStream::comment(llvm::StringRef Comment) {
  प्रतिज्ञा(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

शून्य OStream::flushComment() {
  जर (PendingComment.empty())
    परत;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  पर्यंत (!PendingComment.empty()) {
    स्वतः Pos = PendingComment.find("*/");
    जर (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } अन्यथा {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  जर (चळत.size() > 1 && चळत.back().Ctx == Singleton) {
    जर (IndentSize)
      OS << ' ';
  } अन्यथा {
    newline();
  }
}

शून्य llvm::json::OStream::newline() {
  जर (IndentSize) {
    OS.लिख('\n');
    OS.indent(Indent);
  }
}

शून्य llvm::json::OStream::arrayBegin() {
  मूल्य_Begin();
  चळत.emplace_back();
  चळत.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

शून्य llvm::json::OStream::arrayEnd() {
  प्रतिज्ञा(चळत.back().Ctx == Array);
  Indent -= IndentSize;
  जर (चळत.back().Has_मूल्य)
    newline();
  OS << ']';
  प्रतिज्ञा(PendingComment.empty());
  चळत.काढा_back();
  प्रतिज्ञा(!चळत.empty());
}

शून्य llvm::json::OStream::objectBegin() {
  मूल्य_Begin();
  चळत.emplace_back();
  चळत.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

शून्य llvm::json::OStream::objectEnd() {
  प्रतिज्ञा(चळत.back().Ctx == Object);
  Indent -= IndentSize;
  जर (चळत.back().Has_मूल्य)
    newline();
  OS << '}';
  प्रतिज्ञा(PendingComment.empty());
  चळत.काढा_back();
  प्रतिज्ञा(!चळत.empty());
}

शून्य llvm::json::OStream::attributeBegin(llvm::StringRef किल्ली) {
  प्रतिज्ञा(चळत.back().Ctx == Object);
  जर (चळत.back().Has_मूल्य)
    OS << ',';
  newline();
  flushComment();
  चळत.back().Has_मूल्य = सत्य;
  चळत.emplace_back();
  चळत.back().Ctx = Singleton;
  जर (LLVM_LIKELY(isUTF8(किल्ली))) {
    quote(OS, किल्ली);
  } अन्यथा {
    प्रतिज्ञा(असत्य && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(किल्ली));
  }
  OS.लिख(':');
  जर (IndentSize)
    OS.लिख(' ');
}

शून्य llvm::json::OStream::attributeEnd() {
  प्रतिज्ञा(चळत.back().Ctx == Singleton);
  प्रतिज्ञा(चळत.back().Has_मूल्य && "Attribute must have a value");
  प्रतिज्ञा(PendingComment.empty());
  चळत.काढा_back();
  प्रतिज्ञा(चळत.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_मूल्य_Begin() {
  मूल्य_Begin();
  चळत.emplace_back();
  चळत.back().Ctx = Raw_मूल्य;
  परत OS;
}

शून्य llvm::json::OStream::raw_मूल्य_End() {
  प्रतिज्ञा(चळत.back().Ctx == Raw_मूल्य);
  चळत.काढा_back();
}

} // namespace json
} // namespace llvm

शून्य llvm::format_provider<llvm::json::मूल्य>::format(
    स्थिरांक llvm::json::मूल्य &E, raw_ostream &OS, StringRef Options) {
  अचिन्हित IndentAmount = 0;
  जर (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).मूल्य_2(E);
}

