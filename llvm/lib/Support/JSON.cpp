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

মান &Object::operator[](ধ্রুবক Object_চাবি &K) {
  ফেরত try_emplace(K, nullptr).first->getSecond();
}
মান &Object::operator[](Object_চাবি &&K) {
  ফেরত try_emplace(std::move(K), nullptr).first->getSecond();
}
মান *Object::get(StringRef K) {
  auto I = find(K);
  যদি (I == end())
    ফেরত nullptr;
  ফেরত &I->second;
}
ধ্রুবক মান *Object::get(StringRef K) ধ্রুবক {
  auto I = find(K);
  যদি (I == end())
    ফেরত nullptr;
  ফেরত &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ধ্রুবক {
  যদি (auto *V = get(K))
    ফেরত V->getAsNull();
  ফেরত std::nullopt;
}
std::optional<যৌক্তিক> Object::getBoolean(StringRef K) ধ্রুবক {
  যদি (auto *V = get(K))
    ফেরত V->getAsBoolean();
  ফেরত std::nullopt;
}
std::optional<দ্বিগুণ> Object::get_সংখ্যা(StringRef K) ধ্রুবক {
  যদি (auto *V = get(K))
    ফেরত V->get_As_সংখ্যা();
  ফেরত std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ধ্রুবক {
  যদি (auto *V = get(K))
    ফেরত V->getAsInteger();
  ফেরত std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ধ্রুবক {
  যদি (auto *V = get(K))
    ফেরত V->getAsString();
  ফেরত std::nullopt;
}
ধ্রুবক json::Object *Object::getObject(StringRef K) ধ্রুবক {
  যদি (auto *V = get(K))
    ফেরত V->getAsObject();
  ফেরত nullptr;
}
json::Object *Object::getObject(StringRef K) {
  যদি (auto *V = get(K))
    ফেরত V->getAsObject();
  ফেরত nullptr;
}
ধ্রুবক json::Array *Object::getArray(StringRef K) ধ্রুবক {
  যদি (auto *V = get(K))
    ফেরত V->getAsArray();
  ফেরত nullptr;
}
json::Array *Object::getArray(StringRef K) {
  যদি (auto *V = get(K))
    ফেরত V->getAsArray();
  ফেরত nullptr;
}
যৌক্তিক operator==(ধ্রুবক Object &LHS, ধ্রুবক Object &RHS) { ফেরত LHS.M == RHS.M; }

Array::Array(std::initializer_list<মান> Elements) {
  V.reserve(Elements.আকার_3());
  জন্য (ধ্রুবক মান &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

মান::মান(std::initializer_list<মান> Elements)
    : মান(json::Array(Elements)) {}

শূন্য মান::copyFrom(ধ্রুবক মান &M) {
  Type = M.Type;
  নির্বাচন (Type) {
  ক্ষেত্র T_Null:
  ক্ষেত্র T_Boolean:
  ক্ষেত্র T_Double:
  ক্ষেত্র T_Integer:
  ক্ষেত্র T_UINT64:
    memcpy(&Union, &M.Union, আকার(Union));
    বিরতি;
  ক্ষেত্র T_StringRef:
    create<StringRef>(M.as<StringRef>());
    বিরতি;
  ক্ষেত্র T_String:
    create<std::string>(M.as<std::string>());
    বিরতি;
  ক্ষেত্র T_Object:
    create<json::Object>(M.as<json::Object>());
    বিরতি;
  ক্ষেত্র T_Array:
    create<json::Array>(M.as<json::Array>());
    বিরতি;
  }
}

শূন্য মান::moveFrom(ধ্রুবক মান &&M) {
  Type = M.Type;
  নির্বাচন (Type) {
  ক্ষেত্র T_Null:
  ক্ষেত্র T_Boolean:
  ক্ষেত্র T_Double:
  ক্ষেত্র T_Integer:
  ক্ষেত্র T_UINT64:
    memcpy(&Union, &M.Union, আকার(Union));
    বিরতি;
  ক্ষেত্র T_StringRef:
    create<StringRef>(M.as<StringRef>());
    বিরতি;
  ক্ষেত্র T_String:
    create<std::string>(std::move(M.as<std::string>()));
    বিরতি;
  ক্ষেত্র T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    বিরতি;
  ক্ষেত্র T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    বিরতি;
  }
  const_cast<মান &>(M).destroy();
  M.Type = T_Null;
}

শূন্য মান::destroy() {
  নির্বাচন (Type) {
  ক্ষেত্র T_Null:
  ক্ষেত্র T_Boolean:
  ক্ষেত্র T_Double:
  ক্ষেত্র T_Integer:
  ক্ষেত্র T_UINT64:
    বিরতি;
  ক্ষেত্র T_StringRef:
    as<StringRef>().~StringRef();
    বিরতি;
  ক্ষেত্র T_String:
    as<std::string>().~basic_string();
    বিরতি;
  ক্ষেত্র T_Object:
    as<json::Object>().~Object();
    বিরতি;
  ক্ষেত্র T_Array:
    as<json::Array>().~Array();
    বিরতি;
  }
}

শূন্য মান::print(llvm::raw_ostream &OS) ধ্রুবক { OS << *এই; }

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য মান::dump() ধ্রুবক {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

যৌক্তিক operator==(ধ্রুবক মান &L, ধ্রুবক মান &R) {
  যদি (L.kind() != R.kind())
    ফেরত মিথ্যা;
  নির্বাচন (L.kind()) {
  ক্ষেত্র মান::Null:
    ফেরত *L.getAsNull() == *R.getAsNull();
  ক্ষেত্র মান::Boolean:
    ফেরত *L.getAsBoolean() == *R.getAsBoolean();
  ক্ষেত্র মান::সংখ্যা:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    যদি (L.Type == মান::T_Integer || R.Type == মান::T_Integer)
      ফেরত L.getAsInteger() == R.getAsInteger();
    ফেরত *L.get_As_সংখ্যা() == *R.get_As_সংখ্যা();
  ক্ষেত্র মান::String:
    ফেরত *L.getAsString() == *R.getAsString();
  ক্ষেত্র মান::Array:
    ফেরত *L.getAsArray() == *R.getAsArray();
  ক্ষেত্র মান::Object:
    ফেরত *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

শূন্য Path::report(llvm::StringLiteral বার্তা_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned গুনতি_2 = 0;
  ধ্রুবক Path *P;
  জন্য (P = এই; P->Parent != nullptr; P = P->Parent)
    ++গুনতি_2;
  Path::মূল_2 *R = P->Seg.মূল_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = বার্তা_2;
  R->ErrorPath.resize(গুনতি_2);
  auto It = R->ErrorPath.begin();
  জন্য (P = এই; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::মূল_2::getError() ধ্রুবক {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  যদি (ErrorPath.empty()) {
    যদি (!Name.empty())
      OS << " when parsing " << Name;
  } অন্যথা {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    জন্য (ধ্রুবক Path::Segment &S : llvm::reverse(ErrorPath)) {
      যদি (S.isField())
        OS << '.' << S.field();
      অন্যথা
        OS << '[' << S.সূচক() << ']';
    }
  }
  ফেরত createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ধ্রুবক Object::মান_type *> sortedElements(ধ্রুবক Object &O) {
  std::vector<ধ্রুবক Object::মান_type *> Elements;
  জন্য (ধ্রুবক auto &E : O)
    Elements.ঠেলো_back(&E);
  llvm::sort(Elements,
             [](ধ্রুবক Object::মান_type *L, ধ্রুবক Object::মান_type *R) {
               ফেরত L->first < R->first;
             });
  ফেরত Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
স্থির শূন্য abbreviate(ধ্রুবক মান &V, OStream &JOS) {
  নির্বাচন (V.kind()) {
  ক্ষেত্র মান::Array:
    JOS.raw_মান(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    বিরতি;
  ক্ষেত্র মান::Object:
    JOS.raw_মান(V.getAsObject()->empty() ? "{}" : "{ ... }");
    বিরতি;
  ক্ষেত্র মান::String: {
    llvm::StringRef S = *V.getAsString();
    যদি (S.আকার_3() < 40) {
      JOS.মান_2(V);
    } অন্যথা {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.মান_2(Truncated);
    }
    বিরতি;
  }
  পূর্বনির্ধারিত:
    JOS.মান_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
স্থির শূন্য abbreviateChildren(ধ্রুবক মান &V, OStream &JOS) {
  নির্বাচন (V.kind()) {
  ক্ষেত্র মান::Array:
    JOS.array([&] {
      জন্য (ধ্রুবক auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    বিরতি;
  ক্ষেত্র মান::Object:
    JOS.object([&] {
      জন্য (ধ্রুবক auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    বিরতি;
  পূর্বনির্ধারিত:
    JOS.মান_2(V);
  }
}

শূন্য Path::মূল_2::printErrorContext(ধ্রুবক মান &R, raw_ostream &OS) ধ্রুবক {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_মান = [&](ধ্রুবক মান &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.তথ্য(), ErrorMessage.আকার_3());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    যদি (Path.empty()) // We reached our target.
      ফেরত HighlightCurrent();
    ধ্রুবক Segment &S = Path.back(); // Path is in reverse order.
    যদি (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ধ্রুবক Object *O = V.getAsObject();
      যদি (!O || !O->get(FieldName))
        ফেরত HighlightCurrent();
      JOS.object([&] {
        জন্য (ধ্রুবক auto *KV : sortedElements(*O)) {
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
      ধ্রুবক Array *A = V.getAsArray();
      যদি (!A || S.সূচক() >= A->আকার_3())
        ফেরত HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        জন্য (ধ্রুবক auto &V : *A) {
          যদি (Current++ == S.সূচক())
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
শ্রেণী Parser {
সর্বজনীন:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  যৌক্তিক checkUTF8() {
    size_t ErrOffset;
    যদি (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      ফেরত সত্য;
    P = Start + ErrOffset; // For line/column calculation.
    ফেরত parseError("Invalid UTF-8 sequence");
  }

  যৌক্তিক parse_মান(মান &Out);

  যৌক্তিক assertEnd() {
    eatWhitespace();
    যদি (P == End)
      ফেরত সত্য;
    ফেরত parseError("Text after end of document");
  }

  Error takeError() {
    প্রতিজ্ঞা(Err);
    ফেরত std::move(*Err);
  }

ব্যক্তিগত:
  শূন্য eatWhitespace() {
    যখন (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  যৌক্তিক parse_সংখ্যা(অক্ষর First, মান &Out);
  যৌক্তিক parseString(std::string &Out);
  যৌক্তিক parseUnicode(std::string &Out);
  যৌক্তিক parseError(ধ্রুবক অক্ষর *বার্তা_2); // always returns false

  অক্ষর পরবর্তী() { ফেরত P == End ? 0 : *P++; }
  অক্ষর peek() { ফেরত P == End ? 0 : *P; }
  স্থির যৌক্তিক is_সংখ্যা(অক্ষর C) {
    ফেরত C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ধ্রুবক অক্ষর *Start, *P, *End;
};
} // namespace

যৌক্তিক Parser::parse_মান(মান &Out) {
  eatWhitespace();
  যদি (P == End)
    ফেরত parseError("Unexpected EOF");
  নির্বাচন (অক্ষর C = পরবর্তী()) {
  // Bare null/true/false are easy - first char identifies them.
  ক্ষেত্র 'n':
    Out = nullptr;
    ফেরত (পরবর্তী() == 'u' && পরবর্তী() == 'l' && পরবর্তী() == 'l') ||
           parseError("Invalid JSON value (null?)");
  ক্ষেত্র 't':
    Out = সত্য;
    ফেরত (পরবর্তী() == 'r' && পরবর্তী() == 'u' && পরবর্তী() == 'e') ||
           parseError("Invalid JSON value (true?)");
  ক্ষেত্র 'f':
    Out = মিথ্যা;
    ফেরত (পরবর্তী() == 'a' && পরবর্তী() == 'l' && পরবর্তী() == 's' && পরবর্তী() == 'e') ||
           parseError("Invalid JSON value (false?)");
  ক্ষেত্র '"': {
    std::string S;
    যদি (parseString(S)) {
      Out = std::move(S);
      ফেরত সত্য;
    }
    ফেরত মিথ্যা;
  }
  ক্ষেত্র '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    যদি (peek() == ']') {
      ++P;
      ফেরত সত্য;
    }
    জন্য (;;) {
      A.emplace_back(nullptr);
      যদি (!parse_মান(A.back()))
        ফেরত মিথ্যা;
      eatWhitespace();
      নির্বাচন (পরবর্তী()) {
      ক্ষেত্র ',':
        eatWhitespace();
        চালু;
      ক্ষেত্র ']':
        ফেরত সত্য;
      পূর্বনির্ধারিত:
        ফেরত parseError("Expected , or ] after array element");
      }
    }
  }
  ক্ষেত্র '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    যদি (peek() == '}') {
      ++P;
      ফেরত সত্য;
    }
    জন্য (;;) {
      যদি (পরবর্তী() != '"')
        ফেরত parseError("Expected object key");
      std::string K;
      যদি (!parseString(K))
        ফেরত মিথ্যা;
      eatWhitespace();
      যদি (পরবর্তী() != ':')
        ফেরত parseError("Expected : after object key");
      eatWhitespace();
      যদি (!parse_মান(O[std::move(K)]))
        ফেরত মিথ্যা;
      eatWhitespace();
      নির্বাচন (পরবর্তী()) {
      ক্ষেত্র ',':
        eatWhitespace();
        চালু;
      ক্ষেত্র '}':
        ফেরত সত্য;
      পূর্বনির্ধারিত:
        ফেরত parseError("Expected , or } after object property");
      }
    }
  }
  পূর্বনির্ধারিত:
    যদি (is_সংখ্যা(C))
      ফেরত parse_সংখ্যা(C, Out);
    ফেরত parseError("Invalid JSON value");
  }
}

যৌক্তিক Parser::parse_সংখ্যা(অক্ষর First, মান &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ঠেলো_back(First);
  যখন (is_সংখ্যা(peek()))
    S.ঠেলো_back(পরবর্তী());
  অক্ষর *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  যদি (End == S.end() && errno != ERANGE) {
    Out = I;
    ফেরত সত্য;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  যদি (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    যদি (End == S.end() && errno != ERANGE) {
      Out = UI;
      ফেরত সত্য;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  ফেরত End == S.end() || parseError("Invalid JSON value (number?)");
}

যৌক্তিক Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  জন্য (অক্ষর C = পরবর্তী(); C != '"'; C = পরবর্তী()) {
    যদি (LLVM_UNLIKELY(P == End))
      ফেরত parseError("Unterminated string");
    যদি (LLVM_UNLIKELY((C & 0x1f) == C))
      ফেরত parseError("Control character in string");
    যদি (LLVM_LIKELY(C != '\\')) {
      Out.ঠেলো_back(C);
      চালু;
    }
    // Handle escape sequence.
    নির্বাচন (C = পরবর্তী()) {
    ক্ষেত্র '"':
    ক্ষেত্র '\\':
    ক্ষেত্র '/':
      Out.ঠেলো_back(C);
      বিরতি;
    ক্ষেত্র 'b':
      Out.ঠেলো_back('\b');
      বিরতি;
    ক্ষেত্র 'f':
      Out.ঠেলো_back('\f');
      বিরতি;
    ক্ষেত্র 'n':
      Out.ঠেলো_back('\n');
      বিরতি;
    ক্ষেত্র 'r':
      Out.ঠেলো_back('\r');
      বিরতি;
    ক্ষেত্র 't':
      Out.ঠেলো_back('\t');
      বিরতি;
    ক্ষেত্র 'u':
      যদি (!parseUnicode(Out))
        ফেরত মিথ্যা;
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত parseError("Invalid escape sequence");
    }
  }
  ফেরত সত্য;
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
যৌক্তিক Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [এই](uint16_t &Out) -> যৌক্তিক {
    Out = 0;
    অক্ষর Bytes[] = {পরবর্তী(), পরবর্তী(), পরবর্তী(), পরবর্তী()};
    জন্য (unsigned অক্ষর C : Bytes) {
      যদি (!std::isxdigit(C))
        ফেরত parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    ফেরত সত্য;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  যদি (!Parse4Hex(First))
    ফেরত মিথ্যা;

  // We loop to allow proper surrogate-pair error handling.
  যখন (সত্য) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    যদি (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      ফেরত সত্য;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    যদি (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      ফেরত সত্য;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    যদি (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      ফেরত সত্য;
    }
    P += 2;
    uint16_t Second;
    যদি (!Parse4Hex(Second))
      ফেরত মিথ্যা;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    যদি (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      চালু;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    ফেরত সত্য;
  }
}

যৌক্তিক Parser::parseError(ধ্রুবক অক্ষর *বার্তা_2) {
  পূর্ণসংখ্যা সারি = 1;
  ধ্রুবক অক্ষর *Start_Of_সারি = Start;
  জন্য (ধ্রুবক অক্ষর *X = Start; X < P; ++X) {
    যদি (*X == 0x0A) {
      ++সারি;
      Start_Of_সারি = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(বার্তা_2, সারি, P - Start_Of_সারি, P - Start));
  ফেরত মিথ্যা;
}

Expected<মান> parse(StringRef JSON) {
  Parser P(JSON);
  মান E = nullptr;
  যদি (P.checkUTF8())
    যদি (P.parse_মান(E))
      যদি (P.assertEnd())
        ফেরত std::move(E);
  ফেরত P.takeError();
}

অক্ষর ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
শূন্য ParseError::log(llvm::raw_ostream &OS) ধ্রুবক {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", সারি, Column, Offset, বার্তা_2);
}

যৌক্তিক isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  যদি (LLVM_LIKELY(isASCII(S)))
    ফেরত সত্য;

  ধ্রুবক UTF8 *তথ্য_2 = reinterpret_cast<ধ্রুবক UTF8 *>(S.তথ্য()), *Rest = তথ্য_2;
  যদি (LLVM_LIKELY(isLegalUTF8String(&Rest, তথ্য_2 + S.আকার_3())))
    ফেরত সত্য;

  যদি (ErrOffset)
    *ErrOffset = Rest - তথ্য_2;
  ফেরত মিথ্যা;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.আকার_3()); // 1 codepoint per byte suffices.
  ধ্রুবক UTF8 *In8 = reinterpret_cast<ধ্রুবক UTF8 *>(S.তথ্য());
  UTF32 *Out32 = Codepoints.তথ্য();
  ConvertUTF8toUTF32(&In8, In8 + S.আকার_3(), &Out32, Out32 + Codepoints.আকার_3(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.তথ্য());
  std::string Res(4 * Codepoints.আকার_3(), 0); // 4 bytes per codepoint suffice
  ধ্রুবক UTF32 *In32 = Codepoints.তথ্য();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.আকার_3(), &Out8, Out8 + Res.আকার_3(),
                     strictConversion);
  Res.resize(reinterpret_cast<অক্ষর *>(Out8) - Res.তথ্য());
  ফেরত Res;
}

স্থির শূন্য quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  জন্য (unsigned অক্ষর C : S) {
    যদি (C == 0x22 || C == 0x5C)
      OS << '\\';
    যদি (C >= 0x20) {
      OS << C;
      চালু;
    }
    OS << '\\';
    নির্বাচন (C) {
    // A few characters are common enough to make short escapes worthwhile.
    ক্ষেত্র '\t':
      OS << 't';
      বিরতি;
    ক্ষেত্র '\n':
      OS << 'n';
      বিরতি;
    ক্ষেত্র '\r':
      OS << 'r';
      বিরতি;
    পূর্বনির্ধারিত:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      বিরতি;
    }
  }
  OS << '\"';
}

শূন্য llvm::json::OStream::মান_2(ধ্রুবক মান &V) {
  নির্বাচন (V.kind()) {
  ক্ষেত্র মান::Null:
    মান_Begin();
    OS << "null";
    ফেরত;
  ক্ষেত্র মান::Boolean:
    মান_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    ফেরত;
  ক্ষেত্র মান::সংখ্যা:
    মান_Begin();
    যদি (V.Type == মান::T_Integer)
      OS << *V.getAsInteger();
    অন্যথা যদি (V.Type == মান::T_UINT64)
      OS << *V.getAsUINT64();
    অন্যথা
      OS << format("%.*g", std::numeric_limits<দ্বিগুণ>::max_digits10,
                   *V.get_As_সংখ্যা());
    ফেরত;
  ক্ষেত্র মান::String:
    মান_Begin();
    quote(OS, *V.getAsString());
    ফেরত;
  ক্ষেত্র মান::Array:
    ফেরত array([&] {
      জন্য (ধ্রুবক মান &E : *V.getAsArray())
        মান_2(E);
    });
  ক্ষেত্র মান::Object:
    ফেরত object([&] {
      জন্য (ধ্রুবক Object::মান_type *E : sortedElements(*V.getAsObject()))
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
    ফেরত;
  OS << (Indent_আকার ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  যখন (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    যদি (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } অন্যথা {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_আকার ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  যদি (স্তূপ.আকার_3() > 1 && স্তূপ.back().Ctx == Singleton) {
    যদি (Indent_আকার)
      OS << ' ';
  } অন্যথা {
    newline();
  }
}

শূন্য llvm::json::OStream::newline() {
  যদি (Indent_আকার) {
    OS.লিখ('\n');
    OS.indent(Indent);
  }
}

শূন্য llvm::json::OStream::arrayBegin() {
  মান_Begin();
  স্তূপ.emplace_back();
  স্তূপ.back().Ctx = Array;
  Indent += Indent_আকার;
  OS << '[';
}

শূন্য llvm::json::OStream::arrayEnd() {
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Array);
  Indent -= Indent_আকার;
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
  Indent += Indent_আকার;
  OS << '{';
}

শূন্য llvm::json::OStream::objectEnd() {
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Object);
  Indent -= Indent_আকার;
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
    প্রতিজ্ঞা(মিথ্যা && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(চাবি));
  }
  OS.লিখ(':');
  যদি (Indent_আকার)
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
  ফেরত OS;
}

শূন্য llvm::json::OStream::raw_মান_End() {
  প্রতিজ্ঞা(স্তূপ.back().Ctx == Raw_মান);
  স্তূপ.বের_করো_back();
}

} // namespace json
} // namespace llvm

শূন্য llvm::format_provider<llvm::json::মান>::format(
    ধ্রুবক llvm::json::মান &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  যদি (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).মান_2(E);
}

