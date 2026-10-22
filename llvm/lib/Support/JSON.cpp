//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/JSON.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ConvertUTF.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/NativeFormatting.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cctype>
#ଅନ୍ତର୍ଭାଵ <cerrno>
#ଅନ୍ତର୍ଭାଵ <optional>

ନାମାକାଶ llvm {
ନାମାକାଶ json {

ମୂଲ୍ୟ &Object::operator[](ଅଚଳ Object_ଚାବି &K) {
  ଫେରନ୍ତୁ try_emplace(K, nullptr).first->getSecond();
}
ମୂଲ୍ୟ &Object::operator[](Object_ଚାବି &&K) {
  ଫେରନ୍ତୁ try_emplace(std::move(K), nullptr).first->getSecond();
}
ମୂଲ୍ୟ *Object::get(StringRef K) {
  auto I = find(K);
  ଯଦି (I == end())
    ଫେରନ୍ତୁ nullptr;
  ଫେରନ୍ତୁ &I->second;
}
ଅଚଳ ମୂଲ୍ୟ *Object::get(StringRef K) ଅଚଳ {
  auto I = find(K);
  ଯଦି (I == end())
    ଫେରନ୍ତୁ nullptr;
  ଫେରନ୍ତୁ &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ଅଚଳ {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->getAsNull();
  ଫେରନ୍ତୁ std::nullopt;
}
std::optional<ତାର୍କିକ> Object::getBoolean(StringRef K) ଅଚଳ {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->getAsBoolean();
  ଫେରନ୍ତୁ std::nullopt;
}
std::optional<ଦ୍ୱିଗୁଣ> Object::get_ସଂଖ୍ୟା(StringRef K) ଅଚଳ {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->get_As_ସଂଖ୍ୟା();
  ଫେରନ୍ତୁ std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ଅଚଳ {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->getAsInteger();
  ଫେରନ୍ତୁ std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ଅଚଳ {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->getAsString();
  ଫେରନ୍ତୁ std::nullopt;
}
ଅଚଳ json::Object *Object::getObject(StringRef K) ଅଚଳ {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->getAsObject();
  ଫେରନ୍ତୁ nullptr;
}
json::Object *Object::getObject(StringRef K) {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->getAsObject();
  ଫେରନ୍ତୁ nullptr;
}
ଅଚଳ json::Array *Object::getArray(StringRef K) ଅଚଳ {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->getAsArray();
  ଫେରନ୍ତୁ nullptr;
}
json::Array *Object::getArray(StringRef K) {
  ଯଦି (auto *V = get(K))
    ଫେରନ୍ତୁ V->getAsArray();
  ଫେରନ୍ତୁ nullptr;
}
ତାର୍କିକ operator==(ଅଚଳ Object &LHS, ଅଚଳ Object &RHS) { ଫେରନ୍ତୁ LHS.M == RHS.M; }

Array::Array(std::initializer_list<ମୂଲ୍ୟ> Elements) {
  V.reserve(Elements.size());
  ପାଇଁ (ଅଚଳ ମୂଲ୍ୟ &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

ମୂଲ୍ୟ::ମୂଲ୍ୟ(std::initializer_list<ମୂଲ୍ୟ> Elements)
    : ମୂଲ୍ୟ(json::Array(Elements)) {}

ଶୂନ୍ୟ ମୂଲ୍ୟ::copyFrom(ଅଚଳ ମୂଲ୍ୟ &M) {
  Type = M.Type;
  ବାଛ (Type) {
  ସ୍ଥିତି T_Null:
  ସ୍ଥିତି T_Boolean:
  ସ୍ଥିତି T_Double:
  ସ୍ଥିତି T_Integer:
  ସ୍ଥିତି T_UINT64:
    memcpy(&Union, &M.Union, ଆକାର(Union));
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_String:
    create<std::string>(M.as<std::string>());
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_Object:
    create<json::Object>(M.as<json::Object>());
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_Array:
    create<json::Array>(M.as<json::Array>());
    ଭାଙ୍ଗ;
  }
}

ଶୂନ୍ୟ ମୂଲ୍ୟ::moveFrom(ଅଚଳ ମୂଲ୍ୟ &&M) {
  Type = M.Type;
  ବାଛ (Type) {
  ସ୍ଥିତି T_Null:
  ସ୍ଥିତି T_Boolean:
  ସ୍ଥିତି T_Double:
  ସ୍ଥିତି T_Integer:
  ସ୍ଥିତି T_UINT64:
    memcpy(&Union, &M.Union, ଆକାର(Union));
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_String:
    create<std::string>(std::move(M.as<std::string>()));
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    ଭାଙ୍ଗ;
  }
  const_cast<ମୂଲ୍ୟ &>(M).destroy();
  M.Type = T_Null;
}

ଶୂନ୍ୟ ମୂଲ୍ୟ::destroy() {
  ବାଛ (Type) {
  ସ୍ଥିତି T_Null:
  ସ୍ଥିତି T_Boolean:
  ସ୍ଥିତି T_Double:
  ସ୍ଥିତି T_Integer:
  ସ୍ଥିତି T_UINT64:
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_StringRef:
    as<StringRef>().~StringRef();
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_String:
    as<std::string>().~basic_string();
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_Object:
    as<json::Object>().~Object();
    ଭାଙ୍ଗ;
  ସ୍ଥିତି T_Array:
    as<json::Array>().~Array();
    ଭାଙ୍ଗ;
  }
}

ଶୂନ୍ୟ ମୂଲ୍ୟ::print(llvm::raw_ostream &OS) ଅଚଳ { OS << *ଏହି; }

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ ମୂଲ୍ୟ::dump() ଅଚଳ {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

ତାର୍କିକ operator==(ଅଚଳ ମୂଲ୍ୟ &L, ଅଚଳ ମୂଲ୍ୟ &R) {
  ଯଦି (L.kind() != R.kind())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ବାଛ (L.kind()) {
  ସ୍ଥିତି ମୂଲ୍ୟ::Null:
    ଫେରନ୍ତୁ *L.getAsNull() == *R.getAsNull();
  ସ୍ଥିତି ମୂଲ୍ୟ::Boolean:
    ଫେରନ୍ତୁ *L.getAsBoolean() == *R.getAsBoolean();
  ସ୍ଥିତି ମୂଲ୍ୟ::ସଂଖ୍ୟା:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    ଯଦି (L.Type == ମୂଲ୍ୟ::T_Integer || R.Type == ମୂଲ୍ୟ::T_Integer)
      ଫେରନ୍ତୁ L.getAsInteger() == R.getAsInteger();
    ଫେରନ୍ତୁ *L.get_As_ସଂଖ୍ୟା() == *R.get_As_ସଂଖ୍ୟା();
  ସ୍ଥିତି ମୂଲ୍ୟ::String:
    ଫେରନ୍ତୁ *L.getAsString() == *R.getAsString();
  ସ୍ଥିତି ମୂଲ୍ୟ::Array:
    ଫେରନ୍ତୁ *L.getAsArray() == *R.getAsArray();
  ସ୍ଥିତି ମୂଲ୍ୟ::Object:
    ଫେରନ୍ତୁ *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

ଶୂନ୍ୟ Path::report(llvm::StringLiteral ସନ୍ଦେଶ_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned ଗଣନା_3 = 0;
  ଅଚଳ Path *P;
  ପାଇଁ (P = ଏହି; P->Parent != nullptr; P = P->Parent)
    ++ଗଣନା_3;
  Path::ମୂଳ_2 *R = P->Seg.ମୂଳ_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = ସନ୍ଦେଶ_2;
  R->ErrorPath.resize(ଗଣନା_3);
  auto It = R->ErrorPath.begin();
  ପାଇଁ (P = ଏହି; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::ମୂଳ_2::getError() ଅଚଳ {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  ଯଦି (ErrorPath.empty()) {
    ଯଦି (!Name.empty())
      OS << " when parsing " << Name;
  } ଅନ୍ୟଥା {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    ପାଇଁ (ଅଚଳ Path::Segment &S : llvm::reverse(ErrorPath)) {
      ଯଦି (S.isField())
        OS << '.' << S.field();
      ଅନ୍ୟଥା
        OS << '[' << S.index() << ']';
    }
  }
  ଫେରନ୍ତୁ createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ଅଚଳ Object::ମୂଲ୍ୟ_type *> sortedElements(ଅଚଳ Object &O) {
  std::vector<ଅଚଳ Object::ମୂଲ୍ୟ_type *> Elements;
  ପାଇଁ (ଅଚଳ auto &E : O)
    Elements.ଠେଲ_back(&E);
  llvm::sort(Elements,
             [](ଅଚଳ Object::ମୂଲ୍ୟ_type *L, ଅଚଳ Object::ମୂଲ୍ୟ_type *R) {
               ଫେରନ୍ତୁ L->first < R->first;
             });
  ଫେରନ୍ତୁ Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ସ୍ଥିର ଶୂନ୍ୟ abbreviate(ଅଚଳ ମୂଲ୍ୟ &V, OStream &JOS) {
  ବାଛ (V.kind()) {
  ସ୍ଥିତି ମୂଲ୍ୟ::Array:
    JOS.raw_ମୂଲ୍ୟ(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    ଭାଙ୍ଗ;
  ସ୍ଥିତି ମୂଲ୍ୟ::Object:
    JOS.raw_ମୂଲ୍ୟ(V.getAsObject()->empty() ? "{}" : "{ ... }");
    ଭାଙ୍ଗ;
  ସ୍ଥିତି ମୂଲ୍ୟ::String: {
    llvm::StringRef S = *V.getAsString();
    ଯଦି (S.size() < 40) {
      JOS.ମୂଲ୍ୟ_2(V);
    } ଅନ୍ୟଥା {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.ମୂଲ୍ୟ_2(Truncated);
    }
    ଭାଙ୍ଗ;
  }
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    JOS.ମୂଲ୍ୟ_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ସ୍ଥିର ଶୂନ୍ୟ abbreviateChildren(ଅଚଳ ମୂଲ୍ୟ &V, OStream &JOS) {
  ବାଛ (V.kind()) {
  ସ୍ଥିତି ମୂଲ୍ୟ::Array:
    JOS.array([&] {
      ପାଇଁ (ଅଚଳ auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    ଭାଙ୍ଗ;
  ସ୍ଥିତି ମୂଲ୍ୟ::Object:
    JOS.object([&] {
      ପାଇଁ (ଅଚଳ auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    ଭାଙ୍ଗ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    JOS.ମୂଲ୍ୟ_2(V);
  }
}

ଶୂନ୍ୟ Path::ମୂଳ_2::printErrorContext(ଅଚଳ ମୂଲ୍ୟ &R, raw_ostream &OS) ଅଚଳ {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_ମୂଲ୍ୟ = [&](ଅଚଳ ମୂଲ୍ୟ &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    ଯଦି (Path.empty()) // We reached our target.
      ଫେରନ୍ତୁ HighlightCurrent();
    ଅଚଳ Segment &S = Path.back(); // Path is in reverse order.
    ଯଦି (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ଅଚଳ Object *O = V.getAsObject();
      ଯଦି (!O || !O->get(FieldName))
        ଫେରନ୍ତୁ HighlightCurrent();
      JOS.object([&] {
        ପାଇଁ (ଅଚଳ auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          ଯଦି (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          ଅନ୍ୟଥା
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } ଅନ୍ୟଥା {
      // Current node is an array, path names an element.
      ଅଚଳ Array *A = V.getAsArray();
      ଯଦି (!A || S.index() >= A->size())
        ଫେରନ୍ତୁ HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        ପାଇଁ (ଅଚଳ auto &V : *A) {
          ଯଦି (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          ଅନ୍ୟଥା
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_ମୂଲ୍ୟ(R, ErrorPath, Print_ମୂଲ୍ୟ);
}

ନାମାକାଶ {
// Simple recursive-descent JSON parser.
ଶ୍ରେଣୀ Parser {
ସର୍ବସାଧାରଣ:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  ତାର୍କିକ checkUTF8() {
    size_t ErrOffset;
    ଯଦି (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      ଫେରନ୍ତୁ ସତ୍ୟ;
    P = Start + ErrOffset; // For line/column calculation.
    ଫେରନ୍ତୁ parseError("Invalid UTF-8 sequence");
  }

  ତାର୍କିକ parse_ମୂଲ୍ୟ(ମୂଲ୍ୟ &Out);

  ତାର୍କିକ assertEnd() {
    eatWhitespace();
    ଯଦି (P == End)
      ଫେରନ୍ତୁ ସତ୍ୟ;
    ଫେରନ୍ତୁ parseError("Text after end of document");
  }

  Error takeError() {
    ପ୍ରତିଜ୍ଞା(Err);
    ଫେରନ୍ତୁ std::move(*Err);
  }

ବ୍ୟକ୍ତିଗତ:
  ଶୂନ୍ୟ eatWhitespace() {
    ଯେତେବେଳେ (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  ତାର୍କିକ parse_ସଂଖ୍ୟା(ଅକ୍ଷର First, ମୂଲ୍ୟ &Out);
  ତାର୍କିକ parseString(std::string &Out);
  ତାର୍କିକ parseUnicode(std::string &Out);
  ତାର୍କିକ parseError(ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ_2); // always returns false

  ଅକ୍ଷର next() { ଫେରନ୍ତୁ P == End ? 0 : *P++; }
  ଅକ୍ଷର peek() { ଫେରନ୍ତୁ P == End ? 0 : *P; }
  ସ୍ଥିର ତାର୍କିକ is_ସଂଖ୍ୟା(ଅକ୍ଷର C) {
    ଫେରନ୍ତୁ C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ଅଚଳ ଅକ୍ଷର *Start, *P, *End;
};
} // namespace

ତାର୍କିକ Parser::parse_ମୂଲ୍ୟ(ମୂଲ୍ୟ &Out) {
  eatWhitespace();
  ଯଦି (P == End)
    ଫେରନ୍ତୁ parseError("Unexpected EOF");
  ବାଛ (ଅକ୍ଷର C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  ସ୍ଥିତି 'n':
    Out = nullptr;
    ଫେରନ୍ତୁ (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  ସ୍ଥିତି 't':
    Out = ସତ୍ୟ;
    ଫେରନ୍ତୁ (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  ସ୍ଥିତି 'f':
    Out = ମିଥ୍ୟା;
    ଫେରନ୍ତୁ (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  ସ୍ଥିତି '"': {
    std::string S;
    ଯଦି (parseString(S)) {
      Out = std::move(S);
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ସ୍ଥିତି '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    ଯଦି (peek() == ']') {
      ++P;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    ପାଇଁ (;;) {
      A.emplace_back(nullptr);
      ଯଦି (!parse_ମୂଲ୍ୟ(A.back()))
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      eatWhitespace();
      ବାଛ (next()) {
      ସ୍ଥିତି ',':
        eatWhitespace();
        ଜାରି;
      ସ୍ଥିତି ']':
        ଫେରନ୍ତୁ ସତ୍ୟ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ parseError("Expected , or ] after array element");
      }
    }
  }
  ସ୍ଥିତି '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    ଯଦି (peek() == '}') {
      ++P;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    ପାଇଁ (;;) {
      ଯଦି (next() != '"')
        ଫେରନ୍ତୁ parseError("Expected object key");
      std::string K;
      ଯଦି (!parseString(K))
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      eatWhitespace();
      ଯଦି (next() != ':')
        ଫେରନ୍ତୁ parseError("Expected : after object key");
      eatWhitespace();
      ଯଦି (!parse_ମୂଲ୍ୟ(O[std::move(K)]))
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      eatWhitespace();
      ବାଛ (next()) {
      ସ୍ଥିତି ',':
        eatWhitespace();
        ଜାରି;
      ସ୍ଥିତି '}':
        ଫେରନ୍ତୁ ସତ୍ୟ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ parseError("Expected , or } after object property");
      }
    }
  }
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଯଦି (is_ସଂଖ୍ୟା(C))
      ଫେରନ୍ତୁ parse_ସଂଖ୍ୟା(C, Out);
    ଫେରନ୍ତୁ parseError("Invalid JSON value");
  }
}

ତାର୍କିକ Parser::parse_ସଂଖ୍ୟା(ଅକ୍ଷର First, ମୂଲ୍ୟ &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ଠେଲ_back(First);
  ଯେତେବେଳେ (is_ସଂଖ୍ୟା(peek()))
    S.ଠେଲ_back(next());
  ଅକ୍ଷର *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  ଯଦି (End == S.end() && errno != ERANGE) {
    Out = I;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  ଯଦି (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    ଯଦି (End == S.end() && errno != ERANGE) {
      Out = UI;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  ଫେରନ୍ତୁ End == S.end() || parseError("Invalid JSON value (number?)");
}

ତାର୍କିକ Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  ପାଇଁ (ଅକ୍ଷର C = next(); C != '"'; C = next()) {
    ଯଦି (LLVM_UNLIKELY(P == End))
      ଫେରନ୍ତୁ parseError("Unterminated string");
    ଯଦି (LLVM_UNLIKELY((C & 0x1f) == C))
      ଫେରନ୍ତୁ parseError("Control character in string");
    ଯଦି (LLVM_LIKELY(C != '\\')) {
      Out.ଠେଲ_back(C);
      ଜାରି;
    }
    // Handle escape sequence.
    ବାଛ (C = next()) {
    ସ୍ଥିତି '"':
    ସ୍ଥିତି '\\':
    ସ୍ଥିତି '/':
      Out.ଠେଲ_back(C);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'b':
      Out.ଠେଲ_back('\b');
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'f':
      Out.ଠେଲ_back('\f');
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'n':
      Out.ଠେଲ_back('\n');
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'r':
      Out.ଠେଲ_back('\r');
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 't':
      Out.ଠେଲ_back('\t');
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'u':
      ଯଦି (!parseUnicode(Out))
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ parseError("Invalid escape sequence");
    }
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ସ୍ଥିର ଶୂନ୍ୟ encodeUtf8(uint32_t Rune, std::string &Out) {
  ଯଦି (Rune < 0x80) {
    Out.ଠେଲ_back(Rune & 0x7F);
  } ଅନ୍ୟଥା ଯଦି (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.ଠେଲ_back(FirstByte);
    Out.ଠେଲ_back(SecondByte);
  } ଅନ୍ୟଥା ଯଦି (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.ଠେଲ_back(FirstByte);
    Out.ଠେଲ_back(SecondByte);
    Out.ଠେଲ_back(ThirdByte);
  } ଅନ୍ୟଥା ଯଦି (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.ଠେଲ_back(FirstByte);
    Out.ଠେଲ_back(SecondByte);
    Out.ଠେଲ_back(ThirdByte);
    Out.ଠେଲ_back(FourthByte);
  } ଅନ୍ୟଥା {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
ତାର୍କିକ Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [ଏହି](uint16_t &Out) -> ତାର୍କିକ {
    Out = 0;
    ଅକ୍ଷର Bytes[] = {next(), next(), next(), next()};
    ପାଇଁ (unsigned ଅକ୍ଷର C : Bytes) {
      ଯଦି (!std::isxdigit(C))
        ଫେରନ୍ତୁ parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    ଫେରନ୍ତୁ ସତ୍ୟ;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  ଯଦି (!Parse4Hex(First))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // We loop to allow proper surrogate-pair error handling.
  ଯେତେବେଳେ (ସତ୍ୟ) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    ଯଦି (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    ଯଦି (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    ଯଦି (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    P += 2;
    uint16_t Second;
    ଯଦି (!Parse4Hex(Second))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    ଯଦି (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      ଜାରି;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
}

ତାର୍କିକ Parser::parseError(ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ_2) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = 1;
  ଅଚଳ ଅକ୍ଷର *Start_Of_ଧାଡ଼ି = Start;
  ପାଇଁ (ଅଚଳ ଅକ୍ଷର *X = Start; X < P; ++X) {
    ଯଦି (*X == 0x0A) {
      ++ଧାଡ଼ି;
      Start_Of_ଧାଡ଼ି = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(ସନ୍ଦେଶ_2, ଧାଡ଼ି, P - Start_Of_ଧାଡ଼ି, P - Start));
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

Expected<ମୂଲ୍ୟ> parse(StringRef JSON) {
  Parser P(JSON);
  ମୂଲ୍ୟ E = nullptr;
  ଯଦି (P.checkUTF8())
    ଯଦି (P.parse_ମୂଲ୍ୟ(E))
      ଯଦି (P.assertEnd())
        ଫେରନ୍ତୁ std::move(E);
  ଫେରନ୍ତୁ P.takeError();
}

ଅକ୍ଷର ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
ଶୂନ୍ୟ ParseError::log(llvm::raw_ostream &OS) ଅଚଳ {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", ଧାଡ଼ି, Column, Offset, ସନ୍ଦେଶ_2);
}

ତାର୍କିକ isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  ଯଦି (LLVM_LIKELY(isASCII(S)))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଅଚଳ UTF8 *Data = reinterpret_cast<ଅଚଳ UTF8 *>(S.data()), *Rest = Data;
  ଯଦି (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (ErrOffset)
    *ErrOffset = Rest - Data;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  ଅଚଳ UTF8 *In8 = reinterpret_cast<ଅଚଳ UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  ଅଚଳ UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<ଅକ୍ଷର *>(Out8) - Res.data());
  ଫେରନ୍ତୁ Res;
}

ସ୍ଥିର ଶୂନ୍ୟ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  ପାଇଁ (unsigned ଅକ୍ଷର C : S) {
    ଯଦି (C == 0x22 || C == 0x5C)
      OS << '\\';
    ଯଦି (C >= 0x20) {
      OS << C;
      ଜାରି;
    }
    OS << '\\';
    ବାଛ (C) {
    // A few characters are common enough to make short escapes worthwhile.
    ସ୍ଥିତି '\t':
      OS << 't';
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '\n':
      OS << 'n';
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '\r':
      OS << 'r';
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      ଭାଙ୍ଗ;
    }
  }
  OS << '\"';
}

ଶୂନ୍ୟ llvm::json::OStream::ମୂଲ୍ୟ_2(ଅଚଳ ମୂଲ୍ୟ &V) {
  ବାଛ (V.kind()) {
  ସ୍ଥିତି ମୂଲ୍ୟ::Null:
    ମୂଲ୍ୟ_Begin();
    OS << "null";
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି ମୂଲ୍ୟ::Boolean:
    ମୂଲ୍ୟ_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି ମୂଲ୍ୟ::ସଂଖ୍ୟା:
    ମୂଲ୍ୟ_Begin();
    ଯଦି (V.Type == ମୂଲ୍ୟ::T_Integer)
      OS << *V.getAsInteger();
    ଅନ୍ୟଥା ଯଦି (V.Type == ମୂଲ୍ୟ::T_UINT64)
      OS << *V.getAsUINT64();
    ଅନ୍ୟଥା
      OS << format("%.*g", std::numeric_limits<ଦ୍ୱିଗୁଣ>::max_digits10,
                   *V.get_As_ସଂଖ୍ୟା());
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି ମୂଲ୍ୟ::String:
    ମୂଲ୍ୟ_Begin();
    quote(OS, *V.getAsString());
    ଫେରନ୍ତୁ;
  ସ୍ଥିତି ମୂଲ୍ୟ::Array:
    ଫେରନ୍ତୁ array([&] {
      ପାଇଁ (ଅଚଳ ମୂଲ୍ୟ &E : *V.getAsArray())
        ମୂଲ୍ୟ_2(E);
    });
  ସ୍ଥିତି ମୂଲ୍ୟ::Object:
    ଫେରନ୍ତୁ object([&] {
      ପାଇଁ (ଅଚଳ Object::ମୂଲ୍ୟ_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

ଶୂନ୍ୟ llvm::json::OStream::ମୂଲ୍ୟ_Begin() {
  ପ୍ରତିଜ୍ଞା(ଗଦା.back().Ctx != Object && "Only attributes allowed here");
  ଯଦି (ଗଦା.back().Has_ମୂଲ୍ୟ) {
    ପ୍ରତିଜ୍ଞା(ଗଦା.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  ଯଦି (ଗଦା.back().Ctx == Array)
    newline();
  flushComment();
  ଗଦା.back().Has_ମୂଲ୍ୟ = ସତ୍ୟ;
}

ଶୂନ୍ୟ OStream::comment(llvm::StringRef Comment) {
  ପ୍ରତିଜ୍ଞା(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

ଶୂନ୍ୟ OStream::flushComment() {
  ଯଦି (PendingComment.empty())
    ଫେରନ୍ତୁ;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  ଯେତେବେଳେ (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    ଯଦି (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } ଅନ୍ୟଥା {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  ଯଦି (ଗଦା.size() > 1 && ଗଦା.back().Ctx == Singleton) {
    ଯଦି (IndentSize)
      OS << ' ';
  } ଅନ୍ୟଥା {
    newline();
  }
}

ଶୂନ୍ୟ llvm::json::OStream::newline() {
  ଯଦି (IndentSize) {
    OS.ଲିଖ('\n');
    OS.indent(Indent);
  }
}

ଶୂନ୍ୟ llvm::json::OStream::arrayBegin() {
  ମୂଲ୍ୟ_Begin();
  ଗଦା.emplace_back();
  ଗଦା.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

ଶୂନ୍ୟ llvm::json::OStream::arrayEnd() {
  ପ୍ରତିଜ୍ଞା(ଗଦା.back().Ctx == Array);
  Indent -= IndentSize;
  ଯଦି (ଗଦା.back().Has_ମୂଲ୍ୟ)
    newline();
  OS << ']';
  ପ୍ରତିଜ୍ଞା(PendingComment.empty());
  ଗଦା.କାଢ଼_back();
  ପ୍ରତିଜ୍ଞା(!ଗଦା.empty());
}

ଶୂନ୍ୟ llvm::json::OStream::objectBegin() {
  ମୂଲ୍ୟ_Begin();
  ଗଦା.emplace_back();
  ଗଦା.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

ଶୂନ୍ୟ llvm::json::OStream::objectEnd() {
  ପ୍ରତିଜ୍ଞା(ଗଦା.back().Ctx == Object);
  Indent -= IndentSize;
  ଯଦି (ଗଦା.back().Has_ମୂଲ୍ୟ)
    newline();
  OS << '}';
  ପ୍ରତିଜ୍ଞା(PendingComment.empty());
  ଗଦା.କାଢ଼_back();
  ପ୍ରତିଜ୍ଞା(!ଗଦା.empty());
}

ଶୂନ୍ୟ llvm::json::OStream::attributeBegin(llvm::StringRef ଚାବି) {
  ପ୍ରତିଜ୍ଞା(ଗଦା.back().Ctx == Object);
  ଯଦି (ଗଦା.back().Has_ମୂଲ୍ୟ)
    OS << ',';
  newline();
  flushComment();
  ଗଦା.back().Has_ମୂଲ୍ୟ = ସତ୍ୟ;
  ଗଦା.emplace_back();
  ଗଦା.back().Ctx = Singleton;
  ଯଦି (LLVM_LIKELY(isUTF8(ଚାବି))) {
    quote(OS, ଚାବି);
  } ଅନ୍ୟଥା {
    ପ୍ରତିଜ୍ଞା(ମିଥ୍ୟା && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(ଚାବି));
  }
  OS.ଲିଖ(':');
  ଯଦି (IndentSize)
    OS.ଲିଖ(' ');
}

ଶୂନ୍ୟ llvm::json::OStream::attributeEnd() {
  ପ୍ରତିଜ୍ଞା(ଗଦା.back().Ctx == Singleton);
  ପ୍ରତିଜ୍ଞା(ଗଦା.back().Has_ମୂଲ୍ୟ && "Attribute must have a value");
  ପ୍ରତିଜ୍ଞା(PendingComment.empty());
  ଗଦା.କାଢ଼_back();
  ପ୍ରତିଜ୍ଞା(ଗଦା.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_ମୂଲ୍ୟ_Begin() {
  ମୂଲ୍ୟ_Begin();
  ଗଦା.emplace_back();
  ଗଦା.back().Ctx = Raw_ମୂଲ୍ୟ;
  ଫେରନ୍ତୁ OS;
}

ଶୂନ୍ୟ llvm::json::OStream::raw_ମୂଲ୍ୟ_End() {
  ପ୍ରତିଜ୍ଞା(ଗଦା.back().Ctx == Raw_ମୂଲ୍ୟ);
  ଗଦା.କାଢ଼_back();
}

} // namespace json
} // namespace llvm

ଶୂନ୍ୟ llvm::format_provider<llvm::json::ମୂଲ୍ୟ>::format(
    ଅଚଳ llvm::json::ମୂଲ୍ୟ &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  ଯଦି (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).ମୂଲ୍ୟ_2(E);
}

