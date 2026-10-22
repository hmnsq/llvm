//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/JSON.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Support/ConvertUTF.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/Format.h"
#অন্তর্ভাব "llvm/Support/NativeFormatting.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cctype>
#অন্তর্ভাব <cerrno>
#অন্তর্ভাব <optional>

নামাকাশ llvm {
নামাকাশ json {

মান &Object::operator[](ধ্রুব Object_চাবি &K) {
  প্রত্যাগম try_emplace(K, nullptr).first->getSecond();
}
মান &Object::operator[](Object_চাবি &&K) {
  প্রত্যাগম try_emplace(std::move(K), nullptr).first->getSecond();
}
মান *Object::get(StringRef K) {
  auto I = find(K);
  যদি (I == end())
    প্রত্যাগম nullptr;
  প্রত্যাগম &I->second;
}
ধ্রুব মান *Object::get(StringRef K) ধ্রুব {
  auto I = find(K);
  যদি (I == end())
    প্রত্যাগম nullptr;
  প্রত্যাগম &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ধ্রুব {
  যদি (auto *V = get(K))
    প্রত্যাগম V->getAsNull();
  প্রত্যাগম std::nullopt;
}
std::optional<তার্কিক> Object::getBoolean(StringRef K) ধ্রুব {
  যদি (auto *V = get(K))
    প্রত্যাগম V->getAsBoolean();
  প্রত্যাগম std::nullopt;
}
std::optional<দ্বিগুণ> Object::get_সংখ্যা(StringRef K) ধ্রুব {
  যদি (auto *V = get(K))
    প্রত্যাগম V->get_As_সংখ্যা();
  প্রত্যাগম std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ধ্রুব {
  যদি (auto *V = get(K))
    প্রত্যাগম V->getAsInteger();
  প্রত্যাগম std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ধ্রুব {
  যদি (auto *V = get(K))
    প্রত্যাগম V->getAsString();
  প্রত্যাগম std::nullopt;
}
ধ্রুব json::Object *Object::getObject(StringRef K) ধ্রুব {
  যদি (auto *V = get(K))
    প্রত্যাগম V->getAsObject();
  প্রত্যাগম nullptr;
}
json::Object *Object::getObject(StringRef K) {
  যদি (auto *V = get(K))
    প্রত্যাগম V->getAsObject();
  প্রত্যাগম nullptr;
}
ধ্রুব json::Array *Object::getArray(StringRef K) ধ্রুব {
  যদি (auto *V = get(K))
    প্রত্যাগম V->getAsArray();
  প্রত্যাগম nullptr;
}
json::Array *Object::getArray(StringRef K) {
  যদি (auto *V = get(K))
    প্রত্যাগম V->getAsArray();
  প্রত্যাগম nullptr;
}
তার্কিক operator==(ধ্রুব Object &LHS, ধ্রুব Object &RHS) { প্রত্যাগম LHS.M == RHS.M; }

Array::Array(std::initializer_list<মান> Elements) {
  V.reserve(Elements.size());
  প্রতি (ধ্রুব মান &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

মান::মান(std::initializer_list<মান> Elements)
    : মান(json::Array(Elements)) {}

শূন্য মান::copyFrom(ধ্রুব মান &M) {
  Type = M.Type;
  বিকল্প (Type) {
  পক্ষ T_Null:
  পক্ষ T_Boolean:
  পক্ষ T_Double:
  পক্ষ T_Integer:
  পক্ষ T_UINT64:
    memcpy(&Union, &M.Union, পরিমাণ(Union));
    ভঙ্গ;
  পক্ষ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ভঙ্গ;
  পক্ষ T_String:
    create<std::string>(M.as<std::string>());
    ভঙ্গ;
  পক্ষ T_Object:
    create<json::Object>(M.as<json::Object>());
    ভঙ্গ;
  পক্ষ T_Array:
    create<json::Array>(M.as<json::Array>());
    ভঙ্গ;
  }
}

শূন্য মান::moveFrom(ধ্রুব মান &&M) {
  Type = M.Type;
  বিকল্প (Type) {
  পক্ষ T_Null:
  পক্ষ T_Boolean:
  পক্ষ T_Double:
  পক্ষ T_Integer:
  পক্ষ T_UINT64:
    memcpy(&Union, &M.Union, পরিমাণ(Union));
    ভঙ্গ;
  পক্ষ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ভঙ্গ;
  পক্ষ T_String:
    create<std::string>(std::move(M.as<std::string>()));
    ভঙ্গ;
  পক্ষ T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    ভঙ্গ;
  পক্ষ T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    ভঙ্গ;
  }
  const_cast<মান &>(M).destroy();
  M.Type = T_Null;
}

শূন্য মান::destroy() {
  বিকল্প (Type) {
  পক্ষ T_Null:
  পক্ষ T_Boolean:
  পক্ষ T_Double:
  পক্ষ T_Integer:
  পক্ষ T_UINT64:
    ভঙ্গ;
  পক্ষ T_StringRef:
    as<StringRef>().~StringRef();
    ভঙ্গ;
  পক্ষ T_String:
    as<std::string>().~basic_string();
    ভঙ্গ;
  পক্ষ T_Object:
    as<json::Object>().~Object();
    ভঙ্গ;
  পক্ষ T_Array:
    as<json::Array>().~Array();
    ভঙ্গ;
  }
}

শূন্য মান::print(llvm::raw_ostream &OS) ধ্রুব { OS << *এতত্; }

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য মান::dump() ধ্রুব {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

তার্কিক operator==(ধ্রুব মান &L, ধ্রুব মান &R) {
  যদি (L.kind() != R.kind())
    প্রত্যাগম অসত্য;
  বিকল্প (L.kind()) {
  পক্ষ মান::Null:
    প্রত্যাগম *L.getAsNull() == *R.getAsNull();
  পক্ষ মান::Boolean:
    প্রত্যাগম *L.getAsBoolean() == *R.getAsBoolean();
  পক্ষ মান::সংখ্যা:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    যদি (L.Type == মান::T_Integer || R.Type == মান::T_Integer)
      প্রত্যাগম L.getAsInteger() == R.getAsInteger();
    প্রত্যাগম *L.get_As_সংখ্যা() == *R.get_As_সংখ্যা();
  পক্ষ মান::String:
    প্রত্যাগম *L.getAsString() == *R.getAsString();
  পক্ষ মান::Array:
    প্রত্যাগম *L.getAsArray() == *R.getAsArray();
  পক্ষ মান::Object:
    প্রত্যাগম *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

শূন্য Path::report(llvm::StringLiteral বার্তা_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned গুনতি_2 = 0;
  ধ্রুব Path *P;
  প্রতি (P = এতত্; P->Parent != nullptr; P = P->Parent)
    ++গুনতি_2;
  Path::মূল_2 *R = P->Seg.মূল_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = বার্তা_2;
  R->ErrorPath.resize(গুনতি_2);
  auto It = R->ErrorPath.begin();
  প্রতি (P = এতত্; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::মূল_2::getError() ধ্রুব {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  যদি (ErrorPath.empty()) {
    যদি (!Name.empty())
      OS << " when parsing " << Name;
  } অন্যথা {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    প্রতি (ধ্রুব Path::Segment &S : llvm::reverse(ErrorPath)) {
      যদি (S.isField())
        OS << '.' << S.field();
      অন্যথা
        OS << '[' << S.index() << ']';
    }
  }
  প্রত্যাগম createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ধ্রুব Object::মান_type *> sortedElements(ধ্রুব Object &O) {
  std::vector<ধ্রুব Object::মান_type *> Elements;
  প্রতি (ধ্রুব auto &E : O)
    Elements.ঠেলো_back(&E);
  llvm::sort(Elements,
             [](ধ্রুব Object::মান_type *L, ধ্রুব Object::মান_type *R) {
               প্রত্যাগম L->first < R->first;
             });
  প্রত্যাগম Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
স্থির শূন্য abbreviate(ধ্রুব মান &V, OStream &JOS) {
  বিকল্প (V.kind()) {
  পক্ষ মান::Array:
    JOS.raw_মান(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    ভঙ্গ;
  পক্ষ মান::Object:
    JOS.raw_মান(V.getAsObject()->empty() ? "{}" : "{ ... }");
    ভঙ্গ;
  পক্ষ মান::String: {
    llvm::StringRef S = *V.getAsString();
    যদি (S.size() < 40) {
      JOS.মান_2(V);
    } অন্যথা {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.মান_2(Truncated);
    }
    ভঙ্গ;
  }
  স্বভাব:
    JOS.মান_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
স্থির শূন্য abbreviateChildren(ধ্রুব মান &V, OStream &JOS) {
  বিকল্প (V.kind()) {
  পক্ষ মান::Array:
    JOS.array([&] {
      প্রতি (ধ্রুব auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    ভঙ্গ;
  পক্ষ মান::Object:
    JOS.object([&] {
      প্রতি (ধ্রুব auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    ভঙ্গ;
  স্বভাব:
    JOS.মান_2(V);
  }
}

শূন্য Path::মূল_2::printErrorContext(ধ্রুব মান &R, raw_ostream &OS) ধ্রুব {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_মান = [&](ধ্রুব মান &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    যদি (Path.empty()) // We reached our target.
      প্রত্যাগম HighlightCurrent();
    ধ্রুব Segment &S = Path.back(); // Path is in reverse order.
    যদি (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ধ্রুব Object *O = V.getAsObject();
      যদি (!O || !O->get(FieldName))
        প্রত্যাগম HighlightCurrent();
      JOS.object([&] {
        প্রতি (ধ্রুব auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          যদি (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          অন্যথা
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } অন্যথা {
      // Current node is an array, path names an element.
      ধ্রুব Array *A = V.getAsArray();
      যদি (!A || S.index() >= A->size())
        প্রত্যাগম HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        প্রতি (ধ্রুব auto &V : *A) {
          যদি (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          অন্যথা
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_মান(R, ErrorPath, Print_মান);
}

নামাকাশ {
// Simple recursive-descent JSON parser.
বর্গ Parser {
সার্বজনিক:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  তার্কিক checkUTF8() {
    size_t ErrOffset;
    যদি (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      প্রত্যাগম সত্য;
    P = Start + ErrOffset; // For line/column calculation.
    প্রত্যাগম parseError("Invalid UTF-8 sequence");
  }

  তার্কিক parse_মান(মান &Out);

  তার্কিক assertEnd() {
    eatWhitespace();
    যদি (P == End)
      প্রত্যাগম সত্য;
    প্রত্যাগম parseError("Text after end of document");
  }

  Error takeError() {
    প্রতিজ্ঞা(Err);
    প্রত্যাগম std::move(*Err);
  }

গুপ্ত:
  শূন্য eatWhitespace() {
    যাবত্ (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  তার্কিক parse_সংখ্যা(বর্ণ First, মান &Out);
  তার্কিক parseString(std::string &Out);
  তার্কিক parseUnicode(std::string &Out);
  তার্কিক parseError(ধ্রুব বর্ণ *বার্তা_2); // always returns false

  বর্ণ next() { প্রত্যাগম P == End ? 0 : *P++; }
  বর্ণ peek() { প্রত্যাগম P == End ? 0 : *P; }
  স্থির তার্কিক is_সংখ্যা(বর্ণ C) {
    প্রত্যাগম C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ধ্রুব বর্ণ *Start, *P, *End;
};
} // namespace

তার্কিক Parser::parse_মান(মান &Out) {
  eatWhitespace();
  যদি (P == End)
    প্রত্যাগম parseError("Unexpected EOF");
  বিকল্প (বর্ণ C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  পক্ষ 'n':
    Out = nullptr;
    প্রত্যাগম (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  পক্ষ 't':
    Out = সত্য;
    প্রত্যাগম (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  পক্ষ 'f':
    Out = অসত্য;
    প্রত্যাগম (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  পক্ষ '"': {
    std::string S;
    যদি (parseString(S)) {
      Out = std::move(S);
      প্রত্যাগম সত্য;
    }
    প্রত্যাগম অসত্য;
  }
  পক্ষ '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    যদি (peek() == ']') {
      ++P;
      প্রত্যাগম সত্য;
    }
    প্রতি (;;) {
      A.emplace_back(nullptr);
      যদি (!parse_মান(A.back()))
        প্রত্যাগম অসত্য;
      eatWhitespace();
      বিকল্প (next()) {
      পক্ষ ',':
        eatWhitespace();
        অনুবর্ত;
      পক্ষ ']':
        প্রত্যাগম সত্য;
      স্বভাব:
        প্রত্যাগম parseError("Expected , or ] after array element");
      }
    }
  }
  পক্ষ '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    যদি (peek() == '}') {
      ++P;
      প্রত্যাগম সত্য;
    }
    প্রতি (;;) {
      যদি (next() != '"')
        প্রত্যাগম parseError("Expected object key");
      std::string K;
      যদি (!parseString(K))
        প্রত্যাগম অসত্য;
      eatWhitespace();
      যদি (next() != ':')
        প্রত্যাগম parseError("Expected : after object key");
      eatWhitespace();
      যদি (!parse_মান(O[std::move(K)]))
        প্রত্যাগম অসত্য;
      eatWhitespace();
      বিকল্প (next()) {
      পক্ষ ',':
        eatWhitespace();
        অনুবর্ত;
      পক্ষ '}':
        প্রত্যাগম সত্য;
      স্বভাব:
        প্রত্যাগম parseError("Expected , or } after object property");
      }
    }
  }
  স্বভাব:
    যদি (is_সংখ্যা(C))
      প্রত্যাগম parse_সংখ্যা(C, Out);
    প্রত্যাগম parseError("Invalid JSON value");
  }
}

তার্কিক Parser::parse_সংখ্যা(বর্ণ First, মান &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ঠেলো_back(First);
  যাবত্ (is_সংখ্যা(peek()))
    S.ঠেলো_back(next());
  বর্ণ *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  যদি (End == S.end() && errno != ERANGE) {
    Out = I;
    প্রত্যাগম সত্য;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  যদি (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    যদি (End == S.end() && errno != ERANGE) {
      Out = UI;
      প্রত্যাগম সত্য;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  প্রত্যাগম End == S.end() || parseError("Invalid JSON value (number?)");
}

তার্কিক Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  প্রতি (বর্ণ C = next(); C != '"'; C = next()) {
    যদি (LLVM_UNLIKELY(P == End))
      প্রত্যাগম parseError("Unterminated string");
    যদি (LLVM_UNLIKELY((C & 0x1f) == C))
      প্রত্যাগম parseError("Control character in string");
    যদি (LLVM_LIKELY(C != '\\')) {
      Out.ঠেলো_back(C);
      অনুবর্ত;
    }
    // Handle escape sequence.
    বিকল্প (C = next()) {
    পক্ষ '"':
    পক্ষ '\\':
    পক্ষ '/':
      Out.ঠেলো_back(C);
      ভঙ্গ;
    পক্ষ 'b':
      Out.ঠেলো_back('\b');
      ভঙ্গ;
    পক্ষ 'f':
      Out.ঠেলো_back('\f');
      ভঙ্গ;
    পক্ষ 'n':
      Out.ঠেলো_back('\n');
      ভঙ্গ;
    পক্ষ 'r':
      Out.ঠেলো_back('\r');
      ভঙ্গ;
    পক্ষ 't':
      Out.ঠেলো_back('\t');
      ভঙ্গ;
    পক্ষ 'u':
      যদি (!parseUnicode(Out))
        প্রত্যাগম অসত্য;
      ভঙ্গ;
    স্বভাব:
      প্রত্যাগম parseError("Invalid escape sequence");
    }
  }
  প্রত্যাগম সত্য;
}

স্থির শূন্য encodeUtf8(uint32_t Rune, std::string &Out) {
  যদি (Rune < 0x80) {
    Out.ঠেলো_back(Rune & 0x7F);
  } অন্যথা যদি (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.ঠেলো_back(FirstByte);
    Out.ঠেলো_back(SecondByte);
  } অন্যথা যদি (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.ঠেলো_back(FirstByte);
    Out.ঠেলো_back(SecondByte);
    Out.ঠেলো_back(ThirdByte);
  } অন্যথা যদি (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.ঠেলো_back(FirstByte);
    Out.ঠেলো_back(SecondByte);
    Out.ঠেলো_back(ThirdByte);
    Out.ঠেলো_back(FourthByte);
  } অন্যথা {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
তার্কিক Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [এতত্](uint16_t &Out) -> তার্কিক {
    Out = 0;
    বর্ণ Bytes[] = {next(), next(), next(), next()};
    প্রতি (unsigned বর্ণ C : Bytes) {
      যদি (!std::isxdigit(C))
        প্রত্যাগম parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    প্রত্যাগম সত্য;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  যদি (!Parse4Hex(First))
    প্রত্যাগম অসত্য;

  // We loop to allow proper surrogate-pair error handling.
  যাবত্ (সত্য) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    যদি (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      প্রত্যাগম সত্য;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    যদি (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      প্রত্যাগম সত্য;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    যদি (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      প্রত্যাগম সত্য;
    }
    P += 2;
    uint16_t Second;
    যদি (!Parse4Hex(Second))
      প্রত্যাগম অসত্য;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    যদি (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      অনুবর্ত;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    প্রত্যাগম সত্য;
  }
}

তার্কিক Parser::parseError(ধ্রুব বর্ণ *বার্তা_2) {
  পূর্ণাঙ্ক সারি = 1;
  ধ্রুব বর্ণ *Start_Of_সারি = Start;
  প্রতি (ধ্রুব বর্ণ *X = Start; X < P; ++X) {
    যদি (*X == 0x0A) {
      ++সারি;
      Start_Of_সারি = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(বার্তা_2, সারি, P - Start_Of_সারি, P - Start));
  প্রত্যাগম অসত্য;
}

Expected<মান> parse(StringRef JSON) {
  Parser P(JSON);
  মান E = nullptr;
  যদি (P.checkUTF8())
    যদি (P.parse_মান(E))
      যদি (P.assertEnd())
        প্রত্যাগম std::move(E);
  প্রত্যাগম P.takeError();
}

বর্ণ ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
শূন্য ParseError::log(llvm::raw_ostream &OS) ধ্রুব {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", সারি, Column, Offset, বার্তা_2);
}

তার্কিক isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  যদি (LLVM_LIKELY(isASCII(S)))
    প্রত্যাগম সত্য;

  ধ্রুব UTF8 *Data = reinterpret_cast<ধ্রুব UTF8 *>(S.data()), *Rest = Data;
  যদি (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    প্রত্যাগম সত্য;

  যদি (ErrOffset)
    *ErrOffset = Rest - Data;
  প্রত্যাগম অসত্য;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  ধ্রুব UTF8 *In8 = reinterpret_cast<ধ্রুব UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  ধ্রুব UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<বর্ণ *>(Out8) - Res.data());
  প্রত্যাগম Res;
}

স্থির শূন্য quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  প্রতি (unsigned বর্ণ C : S) {
    যদি (C == 0x22 || C == 0x5C)
      OS << '\\';
    যদি (C >= 0x20) {
      OS << C;
      অনুবর্ত;
    }
    OS << '\\';
    বিকল্প (C) {
    // A few characters are common enough to make short escapes worthwhile.
    পক্ষ '\t':
      OS << 't';
      ভঙ্গ;
    পক্ষ '\n':
      OS << 'n';
      ভঙ্গ;
    পক্ষ '\r':
      OS << 'r';
      ভঙ্গ;
    স্বভাব:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      ভঙ্গ;
    }
  }
  OS << '\"';
}

শূন্য llvm::json::OStream::মান_2(ধ্রুব মান &V) {
  বিকল্প (V.kind()) {
  পক্ষ মান::Null:
    মান_Begin();
    OS << "null";
    প্রত্যাগম;
  পক্ষ মান::Boolean:
    মান_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    প্রত্যাগম;
  পক্ষ মান::সংখ্যা:
    মান_Begin();
    যদি (V.Type == মান::T_Integer)
      OS << *V.getAsInteger();
    অন্যথা যদি (V.Type == মান::T_UINT64)
      OS << *V.getAsUINT64();
    অন্যথা
      OS << format("%.*g", std::numeric_limits<দ্বিগুণ>::max_digits10,
                   *V.get_As_সংখ্যা());
    প্রত্যাগম;
  পক্ষ মান::String:
    মান_Begin();
    quote(OS, *V.getAsString());
    প্রত্যাগম;
  পক্ষ মান::Array:
    প্রত্যাগম array([&] {
      প্রতি (ধ্রুব মান &E : *V.getAsArray())
        মান_2(E);
    });
  পক্ষ মান::Object:
    প্রত্যাগম object([&] {
      প্রতি (ধ্রুব Object::মান_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

শূন্য llvm::json::OStream::মান_Begin() {
  প্রতিজ্ঞা(স্তূপ.back().Ctx != Object && "Only attributes allowed here");
  যদি (স্তূপ.back().Has_মান) {
    প্রতিজ্ঞা(স্তূপ.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  যদি (স্তূপ.back().Ctx == Array)
    newline();
  flushComment();
  স্তূপ.back().Has_মান = সত্য;
}

শূন্য OStream::comment(llvm::StringRef Comment) {
  প্রতিজ্ঞা(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

শূন্য OStream::flushComment() {
  যদি (PendingComment.empty())
    প্রত্যাগম;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  যাবত্ (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    যদি (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } অন্যথা {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  যদি (স্তূপ.size() > 1 && স্তূপ.back().Ctx == Singleton) {
    যদি (IndentSize)
      OS << ' ';
  } অন্যথা {
    newline();
  }
}

শূন্য llvm::json::OStream::newline() {
  যদি (IndentSize) {
    OS.লিখ('\n');
    OS.indent(Indent);
  }
}

শূন্য llvm::json::OStream::arrayBegin() {
  মান_Begin();
  স্তূপ.emplace_back();
  স্তূপ.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

শূন্য llvm::json::OStream::arrayEnd() {
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Array);
  Indent -= IndentSize;
  যদি (স্তূপ.back().Has_মান)
    newline();
  OS << ']';
  প্রতিজ্ঞা(PendingComment.empty());
  স্তূপ.বের_করো_back();
  প্রতিজ্ঞা(!স্তূপ.empty());
}

শূন্য llvm::json::OStream::objectBegin() {
  মান_Begin();
  স্তূপ.emplace_back();
  স্তূপ.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

শূন্য llvm::json::OStream::objectEnd() {
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Object);
  Indent -= IndentSize;
  যদি (স্তূপ.back().Has_মান)
    newline();
  OS << '}';
  প্রতিজ্ঞা(PendingComment.empty());
  স্তূপ.বের_করো_back();
  প্রতিজ্ঞা(!স্তূপ.empty());
}

শূন্য llvm::json::OStream::attributeBegin(llvm::StringRef চাবি) {
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Object);
  যদি (স্তূপ.back().Has_মান)
    OS << ',';
  newline();
  flushComment();
  স্তূপ.back().Has_মান = সত্য;
  স্তূপ.emplace_back();
  স্তূপ.back().Ctx = Singleton;
  যদি (LLVM_LIKELY(isUTF8(চাবি))) {
    quote(OS, চাবি);
  } অন্যথা {
    প্রতিজ্ঞা(অসত্য && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(চাবি));
  }
  OS.লিখ(':');
  যদি (IndentSize)
    OS.লিখ(' ');
}

শূন্য llvm::json::OStream::attributeEnd() {
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Singleton);
  প্রতিজ্ঞা(স্তূপ.back().Has_মান && "Attribute must have a value");
  প্রতিজ্ঞা(PendingComment.empty());
  স্তূপ.বের_করো_back();
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_মান_Begin() {
  মান_Begin();
  স্তূপ.emplace_back();
  স্তূপ.back().Ctx = Raw_মান;
  প্রত্যাগম OS;
}

শূন্য llvm::json::OStream::raw_মান_End() {
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Raw_মান);
  স্তূপ.বের_করো_back();
}

} // namespace json
} // namespace llvm

শূন্য llvm::format_provider<llvm::json::মান>::format(
    ধ্রুব llvm::json::মান &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  যদি (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).মান_2(E);
}

