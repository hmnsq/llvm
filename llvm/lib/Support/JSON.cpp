//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/JSON.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/ConvertUTF.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/Format.h"
#അന്തര്ഭാവ "llvm/Support/NativeFormatting.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cctype>
#അന്തര്ഭാവ <cerrno>
#അന്തര്ഭാവ <optional>

നാമാകാശ llvm {
നാമാകാശ json {

മൂല്യം &Object::operator[](സ്ഥിരം Object_താക്കോൽ &K) {
  തിരികെ try_emplace(K, nullptr).first->getSecond();
}
മൂല്യം &Object::operator[](Object_താക്കോൽ &&K) {
  തിരികെ try_emplace(std::move(K), nullptr).first->getSecond();
}
മൂല്യം *Object::get(StringRef K) {
  auto I = find(K);
  എങ്കിൽ (I == end())
    തിരികെ nullptr;
  തിരികെ &I->second;
}
സ്ഥിരം മൂല്യം *Object::get(StringRef K) സ്ഥിരം {
  auto I = find(K);
  എങ്കിൽ (I == end())
    തിരികെ nullptr;
  തിരികെ &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) സ്ഥിരം {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->getAsNull();
  തിരികെ std::nullopt;
}
std::optional<തർക്കം> Object::getBoolean(StringRef K) സ്ഥിരം {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->getAsBoolean();
  തിരികെ std::nullopt;
}
std::optional<ഇരട്ട> Object::get_സംഖ്യ(StringRef K) സ്ഥിരം {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->get_As_സംഖ്യ();
  തിരികെ std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) സ്ഥിരം {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->getAsInteger();
  തിരികെ std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) സ്ഥിരം {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->getAsString();
  തിരികെ std::nullopt;
}
സ്ഥിരം json::Object *Object::getObject(StringRef K) സ്ഥിരം {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->getAsObject();
  തിരികെ nullptr;
}
json::Object *Object::getObject(StringRef K) {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->getAsObject();
  തിരികെ nullptr;
}
സ്ഥിരം json::Array *Object::getArray(StringRef K) സ്ഥിരം {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->getAsArray();
  തിരികെ nullptr;
}
json::Array *Object::getArray(StringRef K) {
  എങ്കിൽ (auto *V = get(K))
    തിരികെ V->getAsArray();
  തിരികെ nullptr;
}
തർക്കം operator==(സ്ഥിരം Object &LHS, സ്ഥിരം Object &RHS) { തിരികെ LHS.M == RHS.M; }

Array::Array(std::initializer_list<മൂല്യം> Elements) {
  V.reserve(Elements.size());
  വേണ്ടി (സ്ഥിരം മൂല്യം &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

മൂല്യം::മൂല്യം(std::initializer_list<മൂല്യം> Elements)
    : മൂല്യം(json::Array(Elements)) {}

ശൂന്യം മൂല്യം::copyFrom(സ്ഥിരം മൂല്യം &M) {
  Type = M.Type;
  തിരഞ്ഞെടുപ്പ് (Type) {
  അവസ്ഥ T_Null:
  അവസ്ഥ T_Boolean:
  അവസ്ഥ T_Double:
  അവസ്ഥ T_Integer:
  അവസ്ഥ T_UINT64:
    memcpy(&Union, &M.Union, വലുപ്പം(Union));
    നിർത്തുക;
  അവസ്ഥ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    നിർത്തുക;
  അവസ്ഥ T_String:
    create<std::string>(M.as<std::string>());
    നിർത്തുക;
  അവസ്ഥ T_Object:
    create<json::Object>(M.as<json::Object>());
    നിർത്തുക;
  അവസ്ഥ T_Array:
    create<json::Array>(M.as<json::Array>());
    നിർത്തുക;
  }
}

ശൂന്യം മൂല്യം::moveFrom(സ്ഥിരം മൂല്യം &&M) {
  Type = M.Type;
  തിരഞ്ഞെടുപ്പ് (Type) {
  അവസ്ഥ T_Null:
  അവസ്ഥ T_Boolean:
  അവസ്ഥ T_Double:
  അവസ്ഥ T_Integer:
  അവസ്ഥ T_UINT64:
    memcpy(&Union, &M.Union, വലുപ്പം(Union));
    നിർത്തുക;
  അവസ്ഥ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    നിർത്തുക;
  അവസ്ഥ T_String:
    create<std::string>(std::move(M.as<std::string>()));
    നിർത്തുക;
  അവസ്ഥ T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    നിർത്തുക;
  അവസ്ഥ T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    നിർത്തുക;
  }
  const_cast<മൂല്യം &>(M).destroy();
  M.Type = T_Null;
}

ശൂന്യം മൂല്യം::destroy() {
  തിരഞ്ഞെടുപ്പ് (Type) {
  അവസ്ഥ T_Null:
  അവസ്ഥ T_Boolean:
  അവസ്ഥ T_Double:
  അവസ്ഥ T_Integer:
  അവസ്ഥ T_UINT64:
    നിർത്തുക;
  അവസ്ഥ T_StringRef:
    as<StringRef>().~StringRef();
    നിർത്തുക;
  അവസ്ഥ T_String:
    as<std::string>().~basic_string();
    നിർത്തുക;
  അവസ്ഥ T_Object:
    as<json::Object>().~Object();
    നിർത്തുക;
  അവസ്ഥ T_Array:
    as<json::Array>().~Array();
    നിർത്തുക;
  }
}

ശൂന്യം മൂല്യം::print(llvm::raw_ostream &OS) സ്ഥിരം { OS << *ഇത്; }

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം മൂല്യം::dump() സ്ഥിരം {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

തർക്കം operator==(സ്ഥിരം മൂല്യം &L, സ്ഥിരം മൂല്യം &R) {
  എങ്കിൽ (L.kind() != R.kind())
    തിരികെ അസത്യം;
  തിരഞ്ഞെടുപ്പ് (L.kind()) {
  അവസ്ഥ മൂല്യം::Null:
    തിരികെ *L.getAsNull() == *R.getAsNull();
  അവസ്ഥ മൂല്യം::Boolean:
    തിരികെ *L.getAsBoolean() == *R.getAsBoolean();
  അവസ്ഥ മൂല്യം::സംഖ്യ:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    എങ്കിൽ (L.Type == മൂല്യം::T_Integer || R.Type == മൂല്യം::T_Integer)
      തിരികെ L.getAsInteger() == R.getAsInteger();
    തിരികെ *L.get_As_സംഖ്യ() == *R.get_As_സംഖ്യ();
  അവസ്ഥ മൂല്യം::String:
    തിരികെ *L.getAsString() == *R.getAsString();
  അവസ്ഥ മൂല്യം::Array:
    തിരികെ *L.getAsArray() == *R.getAsArray();
  അവസ്ഥ മൂല്യം::Object:
    തിരികെ *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

ശൂന്യം Path::report(llvm::StringLiteral സന്ദേശം_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned എണ്ണം_2 = 0;
  സ്ഥിരം Path *P;
  വേണ്ടി (P = ഇത്; P->Parent != nullptr; P = P->Parent)
    ++എണ്ണം_2;
  Path::വേര്_2 *R = P->Seg.വേര്_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = സന്ദേശം_2;
  R->ErrorPath.resize(എണ്ണം_2);
  auto It = R->ErrorPath.begin();
  വേണ്ടി (P = ഇത്; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::വേര്_2::getError() സ്ഥിരം {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  എങ്കിൽ (ErrorPath.empty()) {
    എങ്കിൽ (!Name.empty())
      OS << " when parsing " << Name;
  } അല്ലെങ്കിൽ {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    വേണ്ടി (സ്ഥിരം Path::Segment &S : llvm::reverse(ErrorPath)) {
      എങ്കിൽ (S.isField())
        OS << '.' << S.field();
      അല്ലെങ്കിൽ
        OS << '[' << S.index() << ']';
    }
  }
  തിരികെ createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<സ്ഥിരം Object::മൂല്യം_type *> sortedElements(സ്ഥിരം Object &O) {
  std::vector<സ്ഥിരം Object::മൂല്യം_type *> Elements;
  വേണ്ടി (സ്ഥിരം auto &E : O)
    Elements.തള്ളുക_back(&E);
  llvm::sort(Elements,
             [](സ്ഥിരം Object::മൂല്യം_type *L, സ്ഥിരം Object::മൂല്യം_type *R) {
               തിരികെ L->first < R->first;
             });
  തിരികെ Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
സ്ഥിതം ശൂന്യം abbreviate(സ്ഥിരം മൂല്യം &V, OStream &JOS) {
  തിരഞ്ഞെടുപ്പ് (V.kind()) {
  അവസ്ഥ മൂല്യം::Array:
    JOS.raw_മൂല്യം(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    നിർത്തുക;
  അവസ്ഥ മൂല്യം::Object:
    JOS.raw_മൂല്യം(V.getAsObject()->empty() ? "{}" : "{ ... }");
    നിർത്തുക;
  അവസ്ഥ മൂല്യം::String: {
    llvm::StringRef S = *V.getAsString();
    എങ്കിൽ (S.size() < 40) {
      JOS.മൂല്യം_2(V);
    } അല്ലെങ്കിൽ {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.മൂല്യം_2(Truncated);
    }
    നിർത്തുക;
  }
  സ്ഥിരസ്ഥിതി:
    JOS.മൂല്യം_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
സ്ഥിതം ശൂന്യം abbreviateChildren(സ്ഥിരം മൂല്യം &V, OStream &JOS) {
  തിരഞ്ഞെടുപ്പ് (V.kind()) {
  അവസ്ഥ മൂല്യം::Array:
    JOS.array([&] {
      വേണ്ടി (സ്ഥിരം auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    നിർത്തുക;
  അവസ്ഥ മൂല്യം::Object:
    JOS.object([&] {
      വേണ്ടി (സ്ഥിരം auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    നിർത്തുക;
  സ്ഥിരസ്ഥിതി:
    JOS.മൂല്യം_2(V);
  }
}

ശൂന്യം Path::വേര്_2::printErrorContext(സ്ഥിരം മൂല്യം &R, raw_ostream &OS) സ്ഥിരം {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_മൂല്യം = [&](സ്ഥിരം മൂല്യം &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    എങ്കിൽ (Path.empty()) // We reached our target.
      തിരികെ HighlightCurrent();
    സ്ഥിരം Segment &S = Path.back(); // Path is in reverse order.
    എങ്കിൽ (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      സ്ഥിരം Object *O = V.getAsObject();
      എങ്കിൽ (!O || !O->get(FieldName))
        തിരികെ HighlightCurrent();
      JOS.object([&] {
        വേണ്ടി (സ്ഥിരം auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          എങ്കിൽ (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          അല്ലെങ്കിൽ
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } അല്ലെങ്കിൽ {
      // Current node is an array, path names an element.
      സ്ഥിരം Array *A = V.getAsArray();
      എങ്കിൽ (!A || S.index() >= A->size())
        തിരികെ HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        വേണ്ടി (സ്ഥിരം auto &V : *A) {
          എങ്കിൽ (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          അല്ലെങ്കിൽ
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_മൂല്യം(R, ErrorPath, Print_മൂല്യം);
}

നാമാകാശ {
// Simple recursive-descent JSON parser.
വർഗ്ഗം Parser {
പൊതു:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  തർക്കം checkUTF8() {
    size_t ErrOffset;
    എങ്കിൽ (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      തിരികെ സത്യം;
    P = Start + ErrOffset; // For line/column calculation.
    തിരികെ parseError("Invalid UTF-8 sequence");
  }

  തർക്കം parse_മൂല്യം(മൂല്യം &Out);

  തർക്കം assertEnd() {
    eatWhitespace();
    എങ്കിൽ (P == End)
      തിരികെ സത്യം;
    തിരികെ parseError("Text after end of document");
  }

  Error takeError() {
    പ്രതിജ്ഞാ(Err);
    തിരികെ std::move(*Err);
  }

സ്വകാര്യ:
  ശൂന്യം eatWhitespace() {
    ആകുമ്പോൾ (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  തർക്കം parse_സംഖ്യ(അക്ഷരം First, മൂല്യം &Out);
  തർക്കം parseString(std::string &Out);
  തർക്കം parseUnicode(std::string &Out);
  തർക്കം parseError(സ്ഥിരം അക്ഷരം *സന്ദേശം_2); // always returns false

  അക്ഷരം next() { തിരികെ P == End ? 0 : *P++; }
  അക്ഷരം peek() { തിരികെ P == End ? 0 : *P; }
  സ്ഥിതം തർക്കം is_സംഖ്യ(അക്ഷരം C) {
    തിരികെ C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  സ്ഥിരം അക്ഷരം *Start, *P, *End;
};
} // namespace

തർക്കം Parser::parse_മൂല്യം(മൂല്യം &Out) {
  eatWhitespace();
  എങ്കിൽ (P == End)
    തിരികെ parseError("Unexpected EOF");
  തിരഞ്ഞെടുപ്പ് (അക്ഷരം C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  അവസ്ഥ 'n':
    Out = nullptr;
    തിരികെ (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  അവസ്ഥ 't':
    Out = സത്യം;
    തിരികെ (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  അവസ്ഥ 'f':
    Out = അസത്യം;
    തിരികെ (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  അവസ്ഥ '"': {
    std::string S;
    എങ്കിൽ (parseString(S)) {
      Out = std::move(S);
      തിരികെ സത്യം;
    }
    തിരികെ അസത്യം;
  }
  അവസ്ഥ '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    എങ്കിൽ (peek() == ']') {
      ++P;
      തിരികെ സത്യം;
    }
    വേണ്ടി (;;) {
      A.emplace_back(nullptr);
      എങ്കിൽ (!parse_മൂല്യം(A.back()))
        തിരികെ അസത്യം;
      eatWhitespace();
      തിരഞ്ഞെടുപ്പ് (next()) {
      അവസ്ഥ ',':
        eatWhitespace();
        തുടരുക;
      അവസ്ഥ ']':
        തിരികെ സത്യം;
      സ്ഥിരസ്ഥിതി:
        തിരികെ parseError("Expected , or ] after array element");
      }
    }
  }
  അവസ്ഥ '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    എങ്കിൽ (peek() == '}') {
      ++P;
      തിരികെ സത്യം;
    }
    വേണ്ടി (;;) {
      എങ്കിൽ (next() != '"')
        തിരികെ parseError("Expected object key");
      std::string K;
      എങ്കിൽ (!parseString(K))
        തിരികെ അസത്യം;
      eatWhitespace();
      എങ്കിൽ (next() != ':')
        തിരികെ parseError("Expected : after object key");
      eatWhitespace();
      എങ്കിൽ (!parse_മൂല്യം(O[std::move(K)]))
        തിരികെ അസത്യം;
      eatWhitespace();
      തിരഞ്ഞെടുപ്പ് (next()) {
      അവസ്ഥ ',':
        eatWhitespace();
        തുടരുക;
      അവസ്ഥ '}':
        തിരികെ സത്യം;
      സ്ഥിരസ്ഥിതി:
        തിരികെ parseError("Expected , or } after object property");
      }
    }
  }
  സ്ഥിരസ്ഥിതി:
    എങ്കിൽ (is_സംഖ്യ(C))
      തിരികെ parse_സംഖ്യ(C, Out);
    തിരികെ parseError("Invalid JSON value");
  }
}

തർക്കം Parser::parse_സംഖ്യ(അക്ഷരം First, മൂല്യം &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.തള്ളുക_back(First);
  ആകുമ്പോൾ (is_സംഖ്യ(peek()))
    S.തള്ളുക_back(next());
  അക്ഷരം *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  എങ്കിൽ (End == S.end() && errno != ERANGE) {
    Out = I;
    തിരികെ സത്യം;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  എങ്കിൽ (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    എങ്കിൽ (End == S.end() && errno != ERANGE) {
      Out = UI;
      തിരികെ സത്യം;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  തിരികെ End == S.end() || parseError("Invalid JSON value (number?)");
}

തർക്കം Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  വേണ്ടി (അക്ഷരം C = next(); C != '"'; C = next()) {
    എങ്കിൽ (LLVM_UNLIKELY(P == End))
      തിരികെ parseError("Unterminated string");
    എങ്കിൽ (LLVM_UNLIKELY((C & 0x1f) == C))
      തിരികെ parseError("Control character in string");
    എങ്കിൽ (LLVM_LIKELY(C != '\\')) {
      Out.തള്ളുക_back(C);
      തുടരുക;
    }
    // Handle escape sequence.
    തിരഞ്ഞെടുപ്പ് (C = next()) {
    അവസ്ഥ '"':
    അവസ്ഥ '\\':
    അവസ്ഥ '/':
      Out.തള്ളുക_back(C);
      നിർത്തുക;
    അവസ്ഥ 'b':
      Out.തള്ളുക_back('\b');
      നിർത്തുക;
    അവസ്ഥ 'f':
      Out.തള്ളുക_back('\f');
      നിർത്തുക;
    അവസ്ഥ 'n':
      Out.തള്ളുക_back('\n');
      നിർത്തുക;
    അവസ്ഥ 'r':
      Out.തള്ളുക_back('\r');
      നിർത്തുക;
    അവസ്ഥ 't':
      Out.തള്ളുക_back('\t');
      നിർത്തുക;
    അവസ്ഥ 'u':
      എങ്കിൽ (!parseUnicode(Out))
        തിരികെ അസത്യം;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ parseError("Invalid escape sequence");
    }
  }
  തിരികെ സത്യം;
}

സ്ഥിതം ശൂന്യം encodeUtf8(uint32_t Rune, std::string &Out) {
  എങ്കിൽ (Rune < 0x80) {
    Out.തള്ളുക_back(Rune & 0x7F);
  } അല്ലെങ്കിൽ എങ്കിൽ (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.തള്ളുക_back(FirstByte);
    Out.തള്ളുക_back(SecondByte);
  } അല്ലെങ്കിൽ എങ്കിൽ (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.തള്ളുക_back(FirstByte);
    Out.തള്ളുക_back(SecondByte);
    Out.തള്ളുക_back(ThirdByte);
  } അല്ലെങ്കിൽ എങ്കിൽ (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.തള്ളുക_back(FirstByte);
    Out.തള്ളുക_back(SecondByte);
    Out.തള്ളുക_back(ThirdByte);
    Out.തള്ളുക_back(FourthByte);
  } അല്ലെങ്കിൽ {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
തർക്കം Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [ഇത്](uint16_t &Out) -> തർക്കം {
    Out = 0;
    അക്ഷരം Bytes[] = {next(), next(), next(), next()};
    വേണ്ടി (unsigned അക്ഷരം C : Bytes) {
      എങ്കിൽ (!std::isxdigit(C))
        തിരികെ parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    തിരികെ സത്യം;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  എങ്കിൽ (!Parse4Hex(First))
    തിരികെ അസത്യം;

  // We loop to allow proper surrogate-pair error handling.
  ആകുമ്പോൾ (സത്യം) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    എങ്കിൽ (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      തിരികെ സത്യം;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    എങ്കിൽ (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      തിരികെ സത്യം;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    എങ്കിൽ (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      തിരികെ സത്യം;
    }
    P += 2;
    uint16_t Second;
    എങ്കിൽ (!Parse4Hex(Second))
      തിരികെ അസത്യം;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    എങ്കിൽ (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      തുടരുക;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    തിരികെ സത്യം;
  }
}

തർക്കം Parser::parseError(സ്ഥിരം അക്ഷരം *സന്ദേശം_2) {
  പൂർണ്ണസംഖ്യ വരി = 1;
  സ്ഥിരം അക്ഷരം *Start_Of_വരി = Start;
  വേണ്ടി (സ്ഥിരം അക്ഷരം *X = Start; X < P; ++X) {
    എങ്കിൽ (*X == 0x0A) {
      ++വരി;
      Start_Of_വരി = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(സന്ദേശം_2, വരി, P - Start_Of_വരി, P - Start));
  തിരികെ അസത്യം;
}

Expected<മൂല്യം> parse(StringRef JSON) {
  Parser P(JSON);
  മൂല്യം E = nullptr;
  എങ്കിൽ (P.checkUTF8())
    എങ്കിൽ (P.parse_മൂല്യം(E))
      എങ്കിൽ (P.assertEnd())
        തിരികെ std::move(E);
  തിരികെ P.takeError();
}

അക്ഷരം ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
ശൂന്യം ParseError::log(llvm::raw_ostream &OS) സ്ഥിരം {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", വരി, Column, Offset, സന്ദേശം_2);
}

തർക്കം isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  എങ്കിൽ (LLVM_LIKELY(isASCII(S)))
    തിരികെ സത്യം;

  സ്ഥിരം UTF8 *Data = reinterpret_cast<സ്ഥിരം UTF8 *>(S.data()), *Rest = Data;
  എങ്കിൽ (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    തിരികെ സത്യം;

  എങ്കിൽ (ErrOffset)
    *ErrOffset = Rest - Data;
  തിരികെ അസത്യം;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  സ്ഥിരം UTF8 *In8 = reinterpret_cast<സ്ഥിരം UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  സ്ഥിരം UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<അക്ഷരം *>(Out8) - Res.data());
  തിരികെ Res;
}

സ്ഥിതം ശൂന്യം quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  വേണ്ടി (unsigned അക്ഷരം C : S) {
    എങ്കിൽ (C == 0x22 || C == 0x5C)
      OS << '\\';
    എങ്കിൽ (C >= 0x20) {
      OS << C;
      തുടരുക;
    }
    OS << '\\';
    തിരഞ്ഞെടുപ്പ് (C) {
    // A few characters are common enough to make short escapes worthwhile.
    അവസ്ഥ '\t':
      OS << 't';
      നിർത്തുക;
    അവസ്ഥ '\n':
      OS << 'n';
      നിർത്തുക;
    അവസ്ഥ '\r':
      OS << 'r';
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      നിർത്തുക;
    }
  }
  OS << '\"';
}

ശൂന്യം llvm::json::OStream::മൂല്യം_2(സ്ഥിരം മൂല്യം &V) {
  തിരഞ്ഞെടുപ്പ് (V.kind()) {
  അവസ്ഥ മൂല്യം::Null:
    മൂല്യം_Begin();
    OS << "null";
    തിരികെ;
  അവസ്ഥ മൂല്യം::Boolean:
    മൂല്യം_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    തിരികെ;
  അവസ്ഥ മൂല്യം::സംഖ്യ:
    മൂല്യം_Begin();
    എങ്കിൽ (V.Type == മൂല്യം::T_Integer)
      OS << *V.getAsInteger();
    അല്ലെങ്കിൽ എങ്കിൽ (V.Type == മൂല്യം::T_UINT64)
      OS << *V.getAsUINT64();
    അല്ലെങ്കിൽ
      OS << format("%.*g", std::numeric_limits<ഇരട്ട>::max_digits10,
                   *V.get_As_സംഖ്യ());
    തിരികെ;
  അവസ്ഥ മൂല്യം::String:
    മൂല്യം_Begin();
    quote(OS, *V.getAsString());
    തിരികെ;
  അവസ്ഥ മൂല്യം::Array:
    തിരികെ array([&] {
      വേണ്ടി (സ്ഥിരം മൂല്യം &E : *V.getAsArray())
        മൂല്യം_2(E);
    });
  അവസ്ഥ മൂല്യം::Object:
    തിരികെ object([&] {
      വേണ്ടി (സ്ഥിരം Object::മൂല്യം_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

ശൂന്യം llvm::json::OStream::മൂല്യം_Begin() {
  പ്രതിജ്ഞാ(അട്ടി.back().Ctx != Object && "Only attributes allowed here");
  എങ്കിൽ (അട്ടി.back().Has_മൂല്യം) {
    പ്രതിജ്ഞാ(അട്ടി.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  എങ്കിൽ (അട്ടി.back().Ctx == Array)
    newline();
  flushComment();
  അട്ടി.back().Has_മൂല്യം = സത്യം;
}

ശൂന്യം OStream::comment(llvm::StringRef Comment) {
  പ്രതിജ്ഞാ(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

ശൂന്യം OStream::flushComment() {
  എങ്കിൽ (PendingComment.empty())
    തിരികെ;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  ആകുമ്പോൾ (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    എങ്കിൽ (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } അല്ലെങ്കിൽ {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  എങ്കിൽ (അട്ടി.size() > 1 && അട്ടി.back().Ctx == Singleton) {
    എങ്കിൽ (IndentSize)
      OS << ' ';
  } അല്ലെങ്കിൽ {
    newline();
  }
}

ശൂന്യം llvm::json::OStream::newline() {
  എങ്കിൽ (IndentSize) {
    OS.ലിഖ('\n');
    OS.indent(Indent);
  }
}

ശൂന്യം llvm::json::OStream::arrayBegin() {
  മൂല്യം_Begin();
  അട്ടി.emplace_back();
  അട്ടി.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

ശൂന്യം llvm::json::OStream::arrayEnd() {
  പ്രതിജ്ഞാ(അട്ടി.back().Ctx == Array);
  Indent -= IndentSize;
  എങ്കിൽ (അട്ടി.back().Has_മൂല്യം)
    newline();
  OS << ']';
  പ്രതിജ്ഞാ(PendingComment.empty());
  അട്ടി.എടുക്കുക_back();
  പ്രതിജ്ഞാ(!അട്ടി.empty());
}

ശൂന്യം llvm::json::OStream::objectBegin() {
  മൂല്യം_Begin();
  അട്ടി.emplace_back();
  അട്ടി.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

ശൂന്യം llvm::json::OStream::objectEnd() {
  പ്രതിജ്ഞാ(അട്ടി.back().Ctx == Object);
  Indent -= IndentSize;
  എങ്കിൽ (അട്ടി.back().Has_മൂല്യം)
    newline();
  OS << '}';
  പ്രതിജ്ഞാ(PendingComment.empty());
  അട്ടി.എടുക്കുക_back();
  പ്രതിജ്ഞാ(!അട്ടി.empty());
}

ശൂന്യം llvm::json::OStream::attributeBegin(llvm::StringRef താക്കോൽ) {
  പ്രതിജ്ഞാ(അട്ടി.back().Ctx == Object);
  എങ്കിൽ (അട്ടി.back().Has_മൂല്യം)
    OS << ',';
  newline();
  flushComment();
  അട്ടി.back().Has_മൂല്യം = സത്യം;
  അട്ടി.emplace_back();
  അട്ടി.back().Ctx = Singleton;
  എങ്കിൽ (LLVM_LIKELY(isUTF8(താക്കോൽ))) {
    quote(OS, താക്കോൽ);
  } അല്ലെങ്കിൽ {
    പ്രതിജ്ഞാ(അസത്യം && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(താക്കോൽ));
  }
  OS.ലിഖ(':');
  എങ്കിൽ (IndentSize)
    OS.ലിഖ(' ');
}

ശൂന്യം llvm::json::OStream::attributeEnd() {
  പ്രതിജ്ഞാ(അട്ടി.back().Ctx == Singleton);
  പ്രതിജ്ഞാ(അട്ടി.back().Has_മൂല്യം && "Attribute must have a value");
  പ്രതിജ്ഞാ(PendingComment.empty());
  അട്ടി.എടുക്കുക_back();
  പ്രതിജ്ഞാ(അട്ടി.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_മൂല്യം_Begin() {
  മൂല്യം_Begin();
  അട്ടി.emplace_back();
  അട്ടി.back().Ctx = Raw_മൂല്യം;
  തിരികെ OS;
}

ശൂന്യം llvm::json::OStream::raw_മൂല്യം_End() {
  പ്രതിജ്ഞാ(അട്ടി.back().Ctx == Raw_മൂല്യം);
  അട്ടി.എടുക്കുക_back();
}

} // namespace json
} // namespace llvm

ശൂന്യം llvm::format_provider<llvm::json::മൂല്യം>::format(
    സ്ഥിരം llvm::json::മൂല്യം &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  എങ്കിൽ (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).മൂല്യം_2(E);
}

