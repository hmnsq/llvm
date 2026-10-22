//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/JSON.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/NativeFormatting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cctype>
#ಅನ್ತರ್ಭಾವ <cerrno>
#ಅನ್ತರ್ಭಾವ <optional>

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ json {

ಮೌಲ್ಯ &Object::operator[](ಧ್ರುವ Object_ಕೀಲಿ &K) {
  ಪ್ರತ್ಯಾಗಮ try_emplace(K, nullptr).first->getSecond();
}
ಮೌಲ್ಯ &Object::operator[](Object_ಕೀಲಿ &&K) {
  ಪ್ರತ್ಯಾಗಮ try_emplace(std::move(K), nullptr).first->getSecond();
}
ಮೌಲ್ಯ *Object::get(StringRef K) {
  auto I = find(K);
  ಯದಿ (I == end())
    ಪ್ರತ್ಯಾಗಮ nullptr;
  ಪ್ರತ್ಯಾಗಮ &I->second;
}
ಧ್ರುವ ಮೌಲ್ಯ *Object::get(StringRef K) ಧ್ರುವ {
  auto I = find(K);
  ಯದಿ (I == end())
    ಪ್ರತ್ಯಾಗಮ nullptr;
  ಪ್ರತ್ಯಾಗಮ &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ಧ್ರುವ {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->getAsNull();
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}
std::optional<ತಾರ್ಕಿಕ> Object::getBoolean(StringRef K) ಧ್ರುವ {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->getAsBoolean();
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}
std::optional<ದ್ವಿಗುಣ> Object::get_ಸಂಖ್ಯೆ(StringRef K) ಧ್ರುವ {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->get_As_ಸಂಖ್ಯೆ();
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ಧ್ರುವ {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->getAsInteger();
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ಧ್ರುವ {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->getAsString();
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}
ಧ್ರುವ json::Object *Object::getObject(StringRef K) ಧ್ರುವ {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->getAsObject();
  ಪ್ರತ್ಯಾಗಮ nullptr;
}
json::Object *Object::getObject(StringRef K) {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->getAsObject();
  ಪ್ರತ್ಯಾಗಮ nullptr;
}
ಧ್ರುವ json::Array *Object::getArray(StringRef K) ಧ್ರುವ {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->getAsArray();
  ಪ್ರತ್ಯಾಗಮ nullptr;
}
json::Array *Object::getArray(StringRef K) {
  ಯದಿ (auto *V = get(K))
    ಪ್ರತ್ಯಾಗಮ V->getAsArray();
  ಪ್ರತ್ಯಾಗಮ nullptr;
}
ತಾರ್ಕಿಕ operator==(ಧ್ರುವ Object &LHS, ಧ್ರುವ Object &RHS) { ಪ್ರತ್ಯಾಗಮ LHS.M == RHS.M; }

Array::Array(std::initializer_list<ಮೌಲ್ಯ> Elements) {
  V.reserve(Elements.size());
  ಪ್ರತಿ (ಧ್ರುವ ಮೌಲ್ಯ &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

ಮೌಲ್ಯ::ಮೌಲ್ಯ(std::initializer_list<ಮೌಲ್ಯ> Elements)
    : ಮೌಲ್ಯ(json::Array(Elements)) {}

ಶೂನ್ಯ ಮೌಲ್ಯ::copyFrom(ಧ್ರುವ ಮೌಲ್ಯ &M) {
  Type = M.Type;
  ವಿಕಲ್ಪ (Type) {
  ಪಕ್ಷ T_Null:
  ಪಕ್ಷ T_Boolean:
  ಪಕ್ಷ T_Double:
  ಪಕ್ಷ T_Integer:
  ಪಕ್ಷ T_UINT64:
    memcpy(&Union, &M.Union, ಪರಿಮಾಣ(Union));
    ಭಙ್ಗ;
  ಪಕ್ಷ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ಭಙ್ಗ;
  ಪಕ್ಷ T_String:
    create<std::string>(M.as<std::string>());
    ಭಙ್ಗ;
  ಪಕ್ಷ T_Object:
    create<json::Object>(M.as<json::Object>());
    ಭಙ್ಗ;
  ಪಕ್ಷ T_Array:
    create<json::Array>(M.as<json::Array>());
    ಭಙ್ಗ;
  }
}

ಶೂನ್ಯ ಮೌಲ್ಯ::moveFrom(ಧ್ರುವ ಮೌಲ್ಯ &&M) {
  Type = M.Type;
  ವಿಕಲ್ಪ (Type) {
  ಪಕ್ಷ T_Null:
  ಪಕ್ಷ T_Boolean:
  ಪಕ್ಷ T_Double:
  ಪಕ್ಷ T_Integer:
  ಪಕ್ಷ T_UINT64:
    memcpy(&Union, &M.Union, ಪರಿಮಾಣ(Union));
    ಭಙ್ಗ;
  ಪಕ್ಷ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ಭಙ್ಗ;
  ಪಕ್ಷ T_String:
    create<std::string>(std::move(M.as<std::string>()));
    ಭಙ್ಗ;
  ಪಕ್ಷ T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    ಭಙ್ಗ;
  ಪಕ್ಷ T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    ಭಙ್ಗ;
  }
  const_cast<ಮೌಲ್ಯ &>(M).destroy();
  M.Type = T_Null;
}

ಶೂನ್ಯ ಮೌಲ್ಯ::destroy() {
  ವಿಕಲ್ಪ (Type) {
  ಪಕ್ಷ T_Null:
  ಪಕ್ಷ T_Boolean:
  ಪಕ್ಷ T_Double:
  ಪಕ್ಷ T_Integer:
  ಪಕ್ಷ T_UINT64:
    ಭಙ್ಗ;
  ಪಕ್ಷ T_StringRef:
    as<StringRef>().~StringRef();
    ಭಙ್ಗ;
  ಪಕ್ಷ T_String:
    as<std::string>().~basic_string();
    ಭಙ್ಗ;
  ಪಕ್ಷ T_Object:
    as<json::Object>().~Object();
    ಭಙ್ಗ;
  ಪಕ್ಷ T_Array:
    as<json::Array>().~Array();
    ಭಙ್ಗ;
  }
}

ಶೂನ್ಯ ಮೌಲ್ಯ::print(llvm::raw_ostream &OS) ಧ್ರುವ { OS << *ಏತತ್; }

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ ಮೌಲ್ಯ::dump() ಧ್ರುವ {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

ತಾರ್ಕಿಕ operator==(ಧ್ರುವ ಮೌಲ್ಯ &L, ಧ್ರುವ ಮೌಲ್ಯ &R) {
  ಯದಿ (L.kind() != R.kind())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ವಿಕಲ್ಪ (L.kind()) {
  ಪಕ್ಷ ಮೌಲ್ಯ::Null:
    ಪ್ರತ್ಯಾಗಮ *L.getAsNull() == *R.getAsNull();
  ಪಕ್ಷ ಮೌಲ್ಯ::Boolean:
    ಪ್ರತ್ಯಾಗಮ *L.getAsBoolean() == *R.getAsBoolean();
  ಪಕ್ಷ ಮೌಲ್ಯ::ಸಂಖ್ಯೆ:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    ಯದಿ (L.Type == ಮೌಲ್ಯ::T_Integer || R.Type == ಮೌಲ್ಯ::T_Integer)
      ಪ್ರತ್ಯಾಗಮ L.getAsInteger() == R.getAsInteger();
    ಪ್ರತ್ಯಾಗಮ *L.get_As_ಸಂಖ್ಯೆ() == *R.get_As_ಸಂಖ್ಯೆ();
  ಪಕ್ಷ ಮೌಲ್ಯ::String:
    ಪ್ರತ್ಯಾಗಮ *L.getAsString() == *R.getAsString();
  ಪಕ್ಷ ಮೌಲ್ಯ::Array:
    ಪ್ರತ್ಯಾಗಮ *L.getAsArray() == *R.getAsArray();
  ಪಕ್ಷ ಮೌಲ್ಯ::Object:
    ಪ್ರತ್ಯಾಗಮ *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

ಶೂನ್ಯ Path::report(llvm::StringLiteral ಸಂದೇಶ_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned ಎಣಿಕೆ_2 = 0;
  ಧ್ರುವ Path *P;
  ಪ್ರತಿ (P = ಏತತ್; P->Parent != nullptr; P = P->Parent)
    ++ಎಣಿಕೆ_2;
  Path::ಬೇರು_2 *R = P->Seg.ಬೇರು_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = ಸಂದೇಶ_2;
  R->ErrorPath.resize(ಎಣಿಕೆ_2);
  auto It = R->ErrorPath.begin();
  ಪ್ರತಿ (P = ಏತತ್; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::ಬೇರು_2::getError() ಧ್ರುವ {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  ಯದಿ (ErrorPath.empty()) {
    ಯದಿ (!Name.empty())
      OS << " when parsing " << Name;
  } ಅನ್ಯಥಾ {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    ಪ್ರತಿ (ಧ್ರುವ Path::Segment &S : llvm::reverse(ErrorPath)) {
      ಯದಿ (S.isField())
        OS << '.' << S.field();
      ಅನ್ಯಥಾ
        OS << '[' << S.index() << ']';
    }
  }
  ಪ್ರತ್ಯಾಗಮ createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<ಧ್ರುವ Object::ಮೌಲ್ಯ_type *> sortedElements(ಧ್ರುವ Object &O) {
  std::vector<ಧ್ರುವ Object::ಮೌಲ್ಯ_type *> Elements;
  ಪ್ರತಿ (ಧ್ರುವ auto &E : O)
    Elements.ತಳ್ಳು_back(&E);
  llvm::sort(Elements,
             [](ಧ್ರುವ Object::ಮೌಲ್ಯ_type *L, ಧ್ರುವ Object::ಮೌಲ್ಯ_type *R) {
               ಪ್ರತ್ಯಾಗಮ L->first < R->first;
             });
  ಪ್ರತ್ಯಾಗಮ Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ಸ್ಥಿರ ಶೂನ್ಯ abbreviate(ಧ್ರುವ ಮೌಲ್ಯ &V, OStream &JOS) {
  ವಿಕಲ್ಪ (V.kind()) {
  ಪಕ್ಷ ಮೌಲ್ಯ::Array:
    JOS.raw_ಮೌಲ್ಯ(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    ಭಙ್ಗ;
  ಪಕ್ಷ ಮೌಲ್ಯ::Object:
    JOS.raw_ಮೌಲ್ಯ(V.getAsObject()->empty() ? "{}" : "{ ... }");
    ಭಙ್ಗ;
  ಪಕ್ಷ ಮೌಲ್ಯ::String: {
    llvm::StringRef S = *V.getAsString();
    ಯದಿ (S.size() < 40) {
      JOS.ಮೌಲ್ಯ_2(V);
    } ಅನ್ಯಥಾ {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.ಮೌಲ್ಯ_2(Truncated);
    }
    ಭಙ್ಗ;
  }
  ಸ್ವಭಾವ:
    JOS.ಮೌಲ್ಯ_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ಸ್ಥಿರ ಶೂನ್ಯ abbreviateChildren(ಧ್ರುವ ಮೌಲ್ಯ &V, OStream &JOS) {
  ವಿಕಲ್ಪ (V.kind()) {
  ಪಕ್ಷ ಮೌಲ್ಯ::Array:
    JOS.array([&] {
      ಪ್ರತಿ (ಧ್ರುವ auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    ಭಙ್ಗ;
  ಪಕ್ಷ ಮೌಲ್ಯ::Object:
    JOS.object([&] {
      ಪ್ರತಿ (ಧ್ರುವ auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    ಭಙ್ಗ;
  ಸ್ವಭಾವ:
    JOS.ಮೌಲ್ಯ_2(V);
  }
}

ಶೂನ್ಯ Path::ಬೇರು_2::printErrorContext(ಧ್ರುವ ಮೌಲ್ಯ &R, raw_ostream &OS) ಧ್ರುವ {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_ಮೌಲ್ಯ = [&](ಧ್ರುವ ಮೌಲ್ಯ &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    ಯದಿ (Path.empty()) // We reached our target.
      ಪ್ರತ್ಯಾಗಮ HighlightCurrent();
    ಧ್ರುವ Segment &S = Path.back(); // Path is in reverse order.
    ಯದಿ (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      ಧ್ರುವ Object *O = V.getAsObject();
      ಯದಿ (!O || !O->get(FieldName))
        ಪ್ರತ್ಯಾಗಮ HighlightCurrent();
      JOS.object([&] {
        ಪ್ರತಿ (ಧ್ರುವ auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          ಯದಿ (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          ಅನ್ಯಥಾ
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } ಅನ್ಯಥಾ {
      // Current node is an array, path names an element.
      ಧ್ರುವ Array *A = V.getAsArray();
      ಯದಿ (!A || S.index() >= A->size())
        ಪ್ರತ್ಯಾಗಮ HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        ಪ್ರತಿ (ಧ್ರುವ auto &V : *A) {
          ಯದಿ (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          ಅನ್ಯಥಾ
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_ಮೌಲ್ಯ(R, ErrorPath, Print_ಮೌಲ್ಯ);
}

ನಾಮಾಕಾಶ {
// Simple recursive-descent JSON parser.
ವರ್ಗ Parser {
ಸಾರ್ವಜನಿಕ:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  ತಾರ್ಕಿಕ checkUTF8() {
    size_t ErrOffset;
    ಯದಿ (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    P = Start + ErrOffset; // For line/column calculation.
    ಪ್ರತ್ಯಾಗಮ parseError("Invalid UTF-8 sequence");
  }

  ತಾರ್ಕಿಕ parse_ಮೌಲ್ಯ(ಮೌಲ್ಯ &Out);

  ತಾರ್ಕಿಕ assertEnd() {
    eatWhitespace();
    ಯದಿ (P == End)
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ parseError("Text after end of document");
  }

  Error takeError() {
    ಪ್ರತಿಜ್ಞಾ(Err);
    ಪ್ರತ್ಯಾಗಮ std::move(*Err);
  }

ಗುಪ್ತ:
  ಶೂನ್ಯ eatWhitespace() {
    ಯಾವತ್ (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  ತಾರ್ಕಿಕ parse_ಸಂಖ್ಯೆ(ವರ್ಣ First, ಮೌಲ್ಯ &Out);
  ತಾರ್ಕಿಕ parseString(std::string &Out);
  ತಾರ್ಕಿಕ parseUnicode(std::string &Out);
  ತಾರ್ಕಿಕ parseError(ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ_2); // always returns false

  ವರ್ಣ next() { ಪ್ರತ್ಯಾಗಮ P == End ? 0 : *P++; }
  ವರ್ಣ peek() { ಪ್ರತ್ಯಾಗಮ P == End ? 0 : *P; }
  ಸ್ಥಿರ ತಾರ್ಕಿಕ is_ಸಂಖ್ಯೆ(ವರ್ಣ C) {
    ಪ್ರತ್ಯಾಗಮ C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ಧ್ರುವ ವರ್ಣ *Start, *P, *End;
};
} // namespace

ತಾರ್ಕಿಕ Parser::parse_ಮೌಲ್ಯ(ಮೌಲ್ಯ &Out) {
  eatWhitespace();
  ಯದಿ (P == End)
    ಪ್ರತ್ಯಾಗಮ parseError("Unexpected EOF");
  ವಿಕಲ್ಪ (ವರ್ಣ C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  ಪಕ್ಷ 'n':
    Out = nullptr;
    ಪ್ರತ್ಯಾಗಮ (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  ಪಕ್ಷ 't':
    Out = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  ಪಕ್ಷ 'f':
    Out = ಅಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  ಪಕ್ಷ '"': {
    std::string S;
    ಯದಿ (parseString(S)) {
      Out = std::move(S);
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪಕ್ಷ '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    ಯದಿ (peek() == ']') {
      ++P;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    ಪ್ರತಿ (;;) {
      A.emplace_back(nullptr);
      ಯದಿ (!parse_ಮೌಲ್ಯ(A.back()))
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      eatWhitespace();
      ವಿಕಲ್ಪ (next()) {
      ಪಕ್ಷ ',':
        eatWhitespace();
        ಅನುವರ್ತ;
      ಪಕ್ಷ ']':
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ parseError("Expected , or ] after array element");
      }
    }
  }
  ಪಕ್ಷ '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    ಯದಿ (peek() == '}') {
      ++P;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    ಪ್ರತಿ (;;) {
      ಯದಿ (next() != '"')
        ಪ್ರತ್ಯಾಗಮ parseError("Expected object key");
      std::string K;
      ಯದಿ (!parseString(K))
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      eatWhitespace();
      ಯದಿ (next() != ':')
        ಪ್ರತ್ಯಾಗಮ parseError("Expected : after object key");
      eatWhitespace();
      ಯದಿ (!parse_ಮೌಲ್ಯ(O[std::move(K)]))
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      eatWhitespace();
      ವಿಕಲ್ಪ (next()) {
      ಪಕ್ಷ ',':
        eatWhitespace();
        ಅನುವರ್ತ;
      ಪಕ್ಷ '}':
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ parseError("Expected , or } after object property");
      }
    }
  }
  ಸ್ವಭಾವ:
    ಯದಿ (is_ಸಂಖ್ಯೆ(C))
      ಪ್ರತ್ಯಾಗಮ parse_ಸಂಖ್ಯೆ(C, Out);
    ಪ್ರತ್ಯಾಗಮ parseError("Invalid JSON value");
  }
}

ತಾರ್ಕಿಕ Parser::parse_ಸಂಖ್ಯೆ(ವರ್ಣ First, ಮೌಲ್ಯ &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ತಳ್ಳು_back(First);
  ಯಾವತ್ (is_ಸಂಖ್ಯೆ(peek()))
    S.ತಳ್ಳು_back(next());
  ವರ್ಣ *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  ಯದಿ (End == S.end() && errno != ERANGE) {
    Out = I;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  ಯದಿ (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    ಯದಿ (End == S.end() && errno != ERANGE) {
      Out = UI;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  ಪ್ರತ್ಯಾಗಮ End == S.end() || parseError("Invalid JSON value (number?)");
}

ತಾರ್ಕಿಕ Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  ಪ್ರತಿ (ವರ್ಣ C = next(); C != '"'; C = next()) {
    ಯದಿ (LLVM_UNLIKELY(P == End))
      ಪ್ರತ್ಯಾಗಮ parseError("Unterminated string");
    ಯದಿ (LLVM_UNLIKELY((C & 0x1f) == C))
      ಪ್ರತ್ಯಾಗಮ parseError("Control character in string");
    ಯದಿ (LLVM_LIKELY(C != '\\')) {
      Out.ತಳ್ಳು_back(C);
      ಅನುವರ್ತ;
    }
    // Handle escape sequence.
    ವಿಕಲ್ಪ (C = next()) {
    ಪಕ್ಷ '"':
    ಪಕ್ಷ '\\':
    ಪಕ್ಷ '/':
      Out.ತಳ್ಳು_back(C);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'b':
      Out.ತಳ್ಳು_back('\b');
      ಭಙ್ಗ;
    ಪಕ್ಷ 'f':
      Out.ತಳ್ಳು_back('\f');
      ಭಙ್ಗ;
    ಪಕ್ಷ 'n':
      Out.ತಳ್ಳು_back('\n');
      ಭಙ್ಗ;
    ಪಕ್ಷ 'r':
      Out.ತಳ್ಳು_back('\r');
      ಭಙ್ಗ;
    ಪಕ್ಷ 't':
      Out.ತಳ್ಳು_back('\t');
      ಭಙ್ಗ;
    ಪಕ್ಷ 'u':
      ಯದಿ (!parseUnicode(Out))
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ parseError("Invalid escape sequence");
    }
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಸ್ಥಿರ ಶೂನ್ಯ encodeUtf8(uint32_t Rune, std::string &Out) {
  ಯದಿ (Rune < 0x80) {
    Out.ತಳ್ಳು_back(Rune & 0x7F);
  } ಅನ್ಯಥಾ ಯದಿ (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.ತಳ್ಳು_back(FirstByte);
    Out.ತಳ್ಳು_back(SecondByte);
  } ಅನ್ಯಥಾ ಯದಿ (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.ತಳ್ಳು_back(FirstByte);
    Out.ತಳ್ಳು_back(SecondByte);
    Out.ತಳ್ಳು_back(ThirdByte);
  } ಅನ್ಯಥಾ ಯದಿ (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.ತಳ್ಳು_back(FirstByte);
    Out.ತಳ್ಳು_back(SecondByte);
    Out.ತಳ್ಳು_back(ThirdByte);
    Out.ತಳ್ಳು_back(FourthByte);
  } ಅನ್ಯಥಾ {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
ತಾರ್ಕಿಕ Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [ಏತತ್](uint16_t &Out) -> ತಾರ್ಕಿಕ {
    Out = 0;
    ವರ್ಣ Bytes[] = {next(), next(), next(), next()};
    ಪ್ರತಿ (unsigned ವರ್ಣ C : Bytes) {
      ಯದಿ (!std::isxdigit(C))
        ಪ್ರತ್ಯಾಗಮ parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  ಯದಿ (!Parse4Hex(First))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // We loop to allow proper surrogate-pair error handling.
  ಯಾವತ್ (ಸತ್ಯ) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    ಯದಿ (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    ಯದಿ (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    ಯದಿ (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    P += 2;
    uint16_t Second;
    ಯದಿ (!Parse4Hex(Second))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    ಯದಿ (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      ಅನುವರ್ತ;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
}

ತಾರ್ಕಿಕ Parser::parseError(ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ_2) {
  ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = 1;
  ಧ್ರುವ ವರ್ಣ *Start_Of_ಸಾಲು = Start;
  ಪ್ರತಿ (ಧ್ರುವ ವರ್ಣ *X = Start; X < P; ++X) {
    ಯದಿ (*X == 0x0A) {
      ++ಸಾಲು;
      Start_Of_ಸಾಲು = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(ಸಂದೇಶ_2, ಸಾಲು, P - Start_Of_ಸಾಲು, P - Start));
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

Expected<ಮೌಲ್ಯ> parse(StringRef JSON) {
  Parser P(JSON);
  ಮೌಲ್ಯ E = nullptr;
  ಯದಿ (P.checkUTF8())
    ಯದಿ (P.parse_ಮೌಲ್ಯ(E))
      ಯದಿ (P.assertEnd())
        ಪ್ರತ್ಯಾಗಮ std::move(E);
  ಪ್ರತ್ಯಾಗಮ P.takeError();
}

ವರ್ಣ ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
ಶೂನ್ಯ ParseError::log(llvm::raw_ostream &OS) ಧ್ರುವ {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", ಸಾಲು, Column, Offset, ಸಂದೇಶ_2);
}

ತಾರ್ಕಿಕ isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  ಯದಿ (LLVM_LIKELY(isASCII(S)))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಧ್ರುವ UTF8 *Data = reinterpret_cast<ಧ್ರುವ UTF8 *>(S.data()), *Rest = Data;
  ಯದಿ (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (ErrOffset)
    *ErrOffset = Rest - Data;
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  ಧ್ರುವ UTF8 *In8 = reinterpret_cast<ಧ್ರುವ UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  ಧ್ರುವ UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<ವರ್ಣ *>(Out8) - Res.data());
  ಪ್ರತ್ಯಾಗಮ Res;
}

ಸ್ಥಿರ ಶೂನ್ಯ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  ಪ್ರತಿ (unsigned ವರ್ಣ C : S) {
    ಯದಿ (C == 0x22 || C == 0x5C)
      OS << '\\';
    ಯದಿ (C >= 0x20) {
      OS << C;
      ಅನುವರ್ತ;
    }
    OS << '\\';
    ವಿಕಲ್ಪ (C) {
    // A few characters are common enough to make short escapes worthwhile.
    ಪಕ್ಷ '\t':
      OS << 't';
      ಭಙ್ಗ;
    ಪಕ್ಷ '\n':
      OS << 'n';
      ಭಙ್ಗ;
    ಪಕ್ಷ '\r':
      OS << 'r';
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      ಭಙ್ಗ;
    }
  }
  OS << '\"';
}

ಶೂನ್ಯ llvm::json::OStream::ಮೌಲ್ಯ_2(ಧ್ರುವ ಮೌಲ್ಯ &V) {
  ವಿಕಲ್ಪ (V.kind()) {
  ಪಕ್ಷ ಮೌಲ್ಯ::Null:
    ಮೌಲ್ಯ_Begin();
    OS << "null";
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ ಮೌಲ್ಯ::Boolean:
    ಮೌಲ್ಯ_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ ಮೌಲ್ಯ::ಸಂಖ್ಯೆ:
    ಮೌಲ್ಯ_Begin();
    ಯದಿ (V.Type == ಮೌಲ್ಯ::T_Integer)
      OS << *V.getAsInteger();
    ಅನ್ಯಥಾ ಯದಿ (V.Type == ಮೌಲ್ಯ::T_UINT64)
      OS << *V.getAsUINT64();
    ಅನ್ಯಥಾ
      OS << format("%.*g", std::numeric_limits<ದ್ವಿಗುಣ>::max_digits10,
                   *V.get_As_ಸಂಖ್ಯೆ());
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ ಮೌಲ್ಯ::String:
    ಮೌಲ್ಯ_Begin();
    quote(OS, *V.getAsString());
    ಪ್ರತ್ಯಾಗಮ;
  ಪಕ್ಷ ಮೌಲ್ಯ::Array:
    ಪ್ರತ್ಯಾಗಮ array([&] {
      ಪ್ರತಿ (ಧ್ರುವ ಮೌಲ್ಯ &E : *V.getAsArray())
        ಮೌಲ್ಯ_2(E);
    });
  ಪಕ್ಷ ಮೌಲ್ಯ::Object:
    ಪ್ರತ್ಯಾಗಮ object([&] {
      ಪ್ರತಿ (ಧ್ರುವ Object::ಮೌಲ್ಯ_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

ಶೂನ್ಯ llvm::json::OStream::ಮೌಲ್ಯ_Begin() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Ctx != Object && "Only attributes allowed here");
  ಯದಿ (ರಾಶಿ.back().Has_ಮೌಲ್ಯ) {
    ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  ಯದಿ (ರಾಶಿ.back().Ctx == Array)
    newline();
  flushComment();
  ರಾಶಿ.back().Has_ಮೌಲ್ಯ = ಸತ್ಯ;
}

ಶೂನ್ಯ OStream::comment(llvm::StringRef Comment) {
  ಪ್ರತಿಜ್ಞಾ(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

ಶೂನ್ಯ OStream::flushComment() {
  ಯದಿ (PendingComment.empty())
    ಪ್ರತ್ಯಾಗಮ;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  ಯಾವತ್ (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    ಯದಿ (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } ಅನ್ಯಥಾ {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  ಯದಿ (ರಾಶಿ.size() > 1 && ರಾಶಿ.back().Ctx == Singleton) {
    ಯದಿ (IndentSize)
      OS << ' ';
  } ಅನ್ಯಥಾ {
    newline();
  }
}

ಶೂನ್ಯ llvm::json::OStream::newline() {
  ಯದಿ (IndentSize) {
    OS.ಲಿಖ('\n');
    OS.indent(Indent);
  }
}

ಶೂನ್ಯ llvm::json::OStream::arrayBegin() {
  ಮೌಲ್ಯ_Begin();
  ರಾಶಿ.emplace_back();
  ರಾಶಿ.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

ಶೂನ್ಯ llvm::json::OStream::arrayEnd() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Ctx == Array);
  Indent -= IndentSize;
  ಯದಿ (ರಾಶಿ.back().Has_ಮೌಲ್ಯ)
    newline();
  OS << ']';
  ಪ್ರತಿಜ್ಞಾ(PendingComment.empty());
  ರಾಶಿ.ತೆಗೆ_back();
  ಪ್ರತಿಜ್ಞಾ(!ರಾಶಿ.empty());
}

ಶೂನ್ಯ llvm::json::OStream::objectBegin() {
  ಮೌಲ್ಯ_Begin();
  ರಾಶಿ.emplace_back();
  ರಾಶಿ.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

ಶೂನ್ಯ llvm::json::OStream::objectEnd() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Ctx == Object);
  Indent -= IndentSize;
  ಯದಿ (ರಾಶಿ.back().Has_ಮೌಲ್ಯ)
    newline();
  OS << '}';
  ಪ್ರತಿಜ್ಞಾ(PendingComment.empty());
  ರಾಶಿ.ತೆಗೆ_back();
  ಪ್ರತಿಜ್ಞಾ(!ರಾಶಿ.empty());
}

ಶೂನ್ಯ llvm::json::OStream::attributeBegin(llvm::StringRef ಕೀಲಿ) {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Ctx == Object);
  ಯದಿ (ರಾಶಿ.back().Has_ಮೌಲ್ಯ)
    OS << ',';
  newline();
  flushComment();
  ರಾಶಿ.back().Has_ಮೌಲ್ಯ = ಸತ್ಯ;
  ರಾಶಿ.emplace_back();
  ರಾಶಿ.back().Ctx = Singleton;
  ಯದಿ (LLVM_LIKELY(isUTF8(ಕೀಲಿ))) {
    quote(OS, ಕೀಲಿ);
  } ಅನ್ಯಥಾ {
    ಪ್ರತಿಜ್ಞಾ(ಅಸತ್ಯ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(ಕೀಲಿ));
  }
  OS.ಲಿಖ(':');
  ಯದಿ (IndentSize)
    OS.ಲಿಖ(' ');
}

ಶೂನ್ಯ llvm::json::OStream::attributeEnd() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Ctx == Singleton);
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Has_ಮೌಲ್ಯ && "Attribute must have a value");
  ಪ್ರತಿಜ್ಞಾ(PendingComment.empty());
  ರಾಶಿ.ತೆಗೆ_back();
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_ಮೌಲ್ಯ_Begin() {
  ಮೌಲ್ಯ_Begin();
  ರಾಶಿ.emplace_back();
  ರಾಶಿ.back().Ctx = Raw_ಮೌಲ್ಯ;
  ಪ್ರತ್ಯಾಗಮ OS;
}

ಶೂನ್ಯ llvm::json::OStream::raw_ಮೌಲ್ಯ_End() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.back().Ctx == Raw_ಮೌಲ್ಯ);
  ರಾಶಿ.ತೆಗೆ_back();
}

} // namespace json
} // namespace llvm

ಶೂನ್ಯ llvm::format_provider<llvm::json::ಮೌಲ್ಯ>::format(
    ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  ಯದಿ (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).ಮೌಲ್ಯ_2(E);
}

