//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/JSON.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/ConvertUTF.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/Format.h"
#அந்தர்பாவ "llvm/Support/NativeFormatting.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cctype>
#அந்தர்பாவ <cerrno>
#அந்தர்பாவ <optional>

நாமாகாஶ llvm {
நாமாகாஶ json {

மதிப்பு &Object::operator[](மாறிலி Object_சாவி &K) {
  திரும்பு try_emplace(K, nullptr).first->getSecond();
}
மதிப்பு &Object::operator[](Object_சாவி &&K) {
  திரும்பு try_emplace(std::move(K), nullptr).first->getSecond();
}
மதிப்பு *Object::get(StringRef K) {
  auto I = find(K);
  எனில் (I == end())
    திரும்பு nullptr;
  திரும்பு &I->second;
}
மாறிலி மதிப்பு *Object::get(StringRef K) மாறிலி {
  auto I = find(K);
  எனில் (I == end())
    திரும்பு nullptr;
  திரும்பு &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) மாறிலி {
  எனில் (auto *V = get(K))
    திரும்பு V->getAsNull();
  திரும்பு std::nullopt;
}
std::optional<தருக்க> Object::getBoolean(StringRef K) மாறிலி {
  எனில் (auto *V = get(K))
    திரும்பு V->getAsBoolean();
  திரும்பு std::nullopt;
}
std::optional<இரட்டை> Object::get_எண்(StringRef K) மாறிலி {
  எனில் (auto *V = get(K))
    திரும்பு V->get_As_எண்();
  திரும்பு std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) மாறிலி {
  எனில் (auto *V = get(K))
    திரும்பு V->getAsInteger();
  திரும்பு std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) மாறிலி {
  எனில் (auto *V = get(K))
    திரும்பு V->getAsString();
  திரும்பு std::nullopt;
}
மாறிலி json::Object *Object::getObject(StringRef K) மாறிலி {
  எனில் (auto *V = get(K))
    திரும்பு V->getAsObject();
  திரும்பு nullptr;
}
json::Object *Object::getObject(StringRef K) {
  எனில் (auto *V = get(K))
    திரும்பு V->getAsObject();
  திரும்பு nullptr;
}
மாறிலி json::Array *Object::getArray(StringRef K) மாறிலி {
  எனில் (auto *V = get(K))
    திரும்பு V->getAsArray();
  திரும்பு nullptr;
}
json::Array *Object::getArray(StringRef K) {
  எனில் (auto *V = get(K))
    திரும்பு V->getAsArray();
  திரும்பு nullptr;
}
தருக்க operator==(மாறிலி Object &LHS, மாறிலி Object &RHS) { திரும்பு LHS.M == RHS.M; }

Array::Array(std::initializer_list<மதிப்பு> Elements) {
  V.reserve(Elements.size());
  க்கு (மாறிலி மதிப்பு &V : Elements) {
    emplace_back(nullptr);
    back().moveFrom(std::move(V));
  }
}

மதிப்பு::மதிப்பு(std::initializer_list<மதிப்பு> Elements)
    : மதிப்பு(json::Array(Elements)) {}

வெறுமை மதிப்பு::copyFrom(மாறிலி மதிப்பு &M) {
  Type = M.Type;
  தேர்வு (Type) {
  நிலை T_Null:
  நிலை T_Boolean:
  நிலை T_Double:
  நிலை T_Integer:
  நிலை T_UINT64:
    memcpy(&Union, &M.Union, அளவு(Union));
    நிறுத்து;
  நிலை T_StringRef:
    create<StringRef>(M.as<StringRef>());
    நிறுத்து;
  நிலை T_String:
    create<std::string>(M.as<std::string>());
    நிறுத்து;
  நிலை T_Object:
    create<json::Object>(M.as<json::Object>());
    நிறுத்து;
  நிலை T_Array:
    create<json::Array>(M.as<json::Array>());
    நிறுத்து;
  }
}

வெறுமை மதிப்பு::moveFrom(மாறிலி மதிப்பு &&M) {
  Type = M.Type;
  தேர்வு (Type) {
  நிலை T_Null:
  நிலை T_Boolean:
  நிலை T_Double:
  நிலை T_Integer:
  நிலை T_UINT64:
    memcpy(&Union, &M.Union, அளவு(Union));
    நிறுத்து;
  நிலை T_StringRef:
    create<StringRef>(M.as<StringRef>());
    நிறுத்து;
  நிலை T_String:
    create<std::string>(std::move(M.as<std::string>()));
    நிறுத்து;
  நிலை T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    நிறுத்து;
  நிலை T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    நிறுத்து;
  }
  const_cast<மதிப்பு &>(M).destroy();
  M.Type = T_Null;
}

வெறுமை மதிப்பு::destroy() {
  தேர்வு (Type) {
  நிலை T_Null:
  நிலை T_Boolean:
  நிலை T_Double:
  நிலை T_Integer:
  நிலை T_UINT64:
    நிறுத்து;
  நிலை T_StringRef:
    as<StringRef>().~StringRef();
    நிறுத்து;
  நிலை T_String:
    as<std::string>().~basic_string();
    நிறுத்து;
  நிலை T_Object:
    as<json::Object>().~Object();
    நிறுத்து;
  நிலை T_Array:
    as<json::Array>().~Array();
    நிறுத்து;
  }
}

வெறுமை மதிப்பு::print(llvm::raw_ostream &OS) மாறிலி { OS << *இது; }

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை மதிப்பு::dump() மாறிலி {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#endif

தருக்க operator==(மாறிலி மதிப்பு &L, மாறிலி மதிப்பு &R) {
  எனில் (L.kind() != R.kind())
    திரும்பு பொய்;
  தேர்வு (L.kind()) {
  நிலை மதிப்பு::Null:
    திரும்பு *L.getAsNull() == *R.getAsNull();
  நிலை மதிப்பு::Boolean:
    திரும்பு *L.getAsBoolean() == *R.getAsBoolean();
  நிலை மதிப்பு::எண்:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    எனில் (L.Type == மதிப்பு::T_Integer || R.Type == மதிப்பு::T_Integer)
      திரும்பு L.getAsInteger() == R.getAsInteger();
    திரும்பு *L.get_As_எண்() == *R.get_As_எண்();
  நிலை மதிப்பு::String:
    திரும்பு *L.getAsString() == *R.getAsString();
  நிலை மதிப்பு::Array:
    திரும்பு *L.getAsArray() == *R.getAsArray();
  நிலை மதிப்பு::Object:
    திரும்பு *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

வெறுமை Path::report(llvm::StringLiteral செய்தி_2) {
  // Walk up to the root context, and count the number of segments.
  unsigned மொத்தம்_2 = 0;
  மாறிலி Path *P;
  க்கு (P = இது; P->Parent != nullptr; P = P->Parent)
    ++மொத்தம்_2;
  Path::வேர்_2 *R = P->Seg.வேர்_3();
  // Fill in the error message and copy the path (in reverse order).
  R->ErrorMessage = செய்தி_2;
  R->ErrorPath.resize(மொத்தம்_2);
  auto It = R->ErrorPath.begin();
  க்கு (P = இது; P->Parent != nullptr; P = P->Parent)
    *It++ = P->Seg;
}

Error Path::வேர்_2::getError() மாறிலி {
  std::string S;
  raw_string_ostream OS(S);
  OS << (ErrorMessage.empty() ? "invalid JSON contents" : ErrorMessage);
  எனில் (ErrorPath.empty()) {
    எனில் (!Name.empty())
      OS << " when parsing " << Name;
  } இல்லையெனில் {
    OS << " at " << (Name.empty() ? "(root)" : Name);
    க்கு (மாறிலி Path::Segment &S : llvm::reverse(ErrorPath)) {
      எனில் (S.isField())
        OS << '.' << S.field();
      இல்லையெனில்
        OS << '[' << S.index() << ']';
    }
  }
  திரும்பு createStringError(llvm::inconvertibleErrorCode(), S);
}

std::vector<மாறிலி Object::மதிப்பு_type *> sortedElements(மாறிலி Object &O) {
  std::vector<மாறிலி Object::மதிப்பு_type *> Elements;
  க்கு (மாறிலி auto &E : O)
    Elements.தள்ளு_back(&E);
  llvm::sort(Elements,
             [](மாறிலி Object::மதிப்பு_type *L, மாறிலி Object::மதிப்பு_type *R) {
               திரும்பு L->first < R->first;
             });
  திரும்பு Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
நிலையான வெறுமை abbreviate(மாறிலி மதிப்பு &V, OStream &JOS) {
  தேர்வு (V.kind()) {
  நிலை மதிப்பு::Array:
    JOS.raw_மதிப்பு(V.getAsArray()->empty() ? "[]" : "[ ... ]");
    நிறுத்து;
  நிலை மதிப்பு::Object:
    JOS.raw_மதிப்பு(V.getAsObject()->empty() ? "{}" : "{ ... }");
    நிறுத்து;
  நிலை மதிப்பு::String: {
    llvm::StringRef S = *V.getAsString();
    எனில் (S.size() < 40) {
      JOS.மதிப்பு_2(V);
    } இல்லையெனில் {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.மதிப்பு_2(Truncated);
    }
    நிறுத்து;
  }
  இயல்பு:
    JOS.மதிப்பு_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
நிலையான வெறுமை abbreviateChildren(மாறிலி மதிப்பு &V, OStream &JOS) {
  தேர்வு (V.kind()) {
  நிலை மதிப்பு::Array:
    JOS.array([&] {
      க்கு (மாறிலி auto &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    நிறுத்து;
  நிலை மதிப்பு::Object:
    JOS.object([&] {
      க்கு (மாறிலி auto *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attributeEnd();
      }
    });
    நிறுத்து;
  இயல்பு:
    JOS.மதிப்பு_2(V);
  }
}

வெறுமை Path::வேர்_2::printErrorContext(மாறிலி மதிப்பு &R, raw_ostream &OS) மாறிலி {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  auto Print_மதிப்பு = [&](மாறிலி மதிப்பு &V, ArrayRef<Segment> Path, auto &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    auto HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(ErrorMessage.data(), ErrorMessage.size());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    எனில் (Path.empty()) // We reached our target.
      திரும்பு HighlightCurrent();
    மாறிலி Segment &S = Path.back(); // Path is in reverse order.
    எனில் (S.isField()) {
      // Current node is an object, path names a field.
      llvm::StringRef FieldName = S.field();
      மாறிலி Object *O = V.getAsObject();
      எனில் (!O || !O->get(FieldName))
        திரும்பு HighlightCurrent();
      JOS.object([&] {
        க்கு (மாறிலி auto *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          எனில் (FieldName == StringRef(KV->first))
            Recurse(KV->second, Path.drop_back(), Recurse);
          இல்லையெனில்
            abbreviate(KV->second, JOS);
          JOS.attributeEnd();
        }
      });
    } இல்லையெனில் {
      // Current node is an array, path names an element.
      மாறிலி Array *A = V.getAsArray();
      எனில் (!A || S.index() >= A->size())
        திரும்பு HighlightCurrent();
      JOS.array([&] {
        unsigned Current = 0;
        க்கு (மாறிலி auto &V : *A) {
          எனில் (Current++ == S.index())
            Recurse(V, Path.drop_back(), Recurse);
          இல்லையெனில்
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_மதிப்பு(R, ErrorPath, Print_மதிப்பு);
}

நாமாகாஶ {
// Simple recursive-descent JSON parser.
வகுப்பு Parser {
பொது:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), End(JSON.end()) {}

  தருக்க checkUTF8() {
    size_t ErrOffset;
    எனில் (isUTF8(StringRef(Start, End - Start), &ErrOffset))
      திரும்பு உண்மை;
    P = Start + ErrOffset; // For line/column calculation.
    திரும்பு parseError("Invalid UTF-8 sequence");
  }

  தருக்க parse_மதிப்பு(மதிப்பு &Out);

  தருக்க assertEnd() {
    eatWhitespace();
    எனில் (P == End)
      திரும்பு உண்மை;
    திரும்பு parseError("Text after end of document");
  }

  Error takeError() {
    ப்ரதிஜ்ஞா(Err);
    திரும்பு std::move(*Err);
  }

தனிப்பட்ட:
  வெறுமை eatWhitespace() {
    போது (P != End && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  தருக்க parse_எண்(எழுத்து First, மதிப்பு &Out);
  தருக்க parseString(std::string &Out);
  தருக்க parseUnicode(std::string &Out);
  தருக்க parseError(மாறிலி எழுத்து *செய்தி_2); // always returns false

  எழுத்து next() { திரும்பு P == End ? 0 : *P++; }
  எழுத்து peek() { திரும்பு P == End ? 0 : *P; }
  நிலையான தருக்க is_எண்(எழுத்து C) {
    திரும்பு C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  மாறிலி எழுத்து *Start, *P, *End;
};
} // namespace

தருக்க Parser::parse_மதிப்பு(மதிப்பு &Out) {
  eatWhitespace();
  எனில் (P == End)
    திரும்பு parseError("Unexpected EOF");
  தேர்வு (எழுத்து C = next()) {
  // Bare null/true/false are easy - first char identifies them.
  நிலை 'n':
    Out = nullptr;
    திரும்பு (next() == 'u' && next() == 'l' && next() == 'l') ||
           parseError("Invalid JSON value (null?)");
  நிலை 't':
    Out = உண்மை;
    திரும்பு (next() == 'r' && next() == 'u' && next() == 'e') ||
           parseError("Invalid JSON value (true?)");
  நிலை 'f':
    Out = பொய்;
    திரும்பு (next() == 'a' && next() == 'l' && next() == 's' && next() == 'e') ||
           parseError("Invalid JSON value (false?)");
  நிலை '"': {
    std::string S;
    எனில் (parseString(S)) {
      Out = std::move(S);
      திரும்பு உண்மை;
    }
    திரும்பு பொய்;
  }
  நிலை '[': {
    Out = Array{};
    Array &A = *Out.getAsArray();
    eatWhitespace();
    எனில் (peek() == ']') {
      ++P;
      திரும்பு உண்மை;
    }
    க்கு (;;) {
      A.emplace_back(nullptr);
      எனில் (!parse_மதிப்பு(A.back()))
        திரும்பு பொய்;
      eatWhitespace();
      தேர்வு (next()) {
      நிலை ',':
        eatWhitespace();
        தொடர்;
      நிலை ']':
        திரும்பு உண்மை;
      இயல்பு:
        திரும்பு parseError("Expected , or ] after array element");
      }
    }
  }
  நிலை '{': {
    Out = Object{};
    Object &O = *Out.getAsObject();
    eatWhitespace();
    எனில் (peek() == '}') {
      ++P;
      திரும்பு உண்மை;
    }
    க்கு (;;) {
      எனில் (next() != '"')
        திரும்பு parseError("Expected object key");
      std::string K;
      எனில் (!parseString(K))
        திரும்பு பொய்;
      eatWhitespace();
      எனில் (next() != ':')
        திரும்பு parseError("Expected : after object key");
      eatWhitespace();
      எனில் (!parse_மதிப்பு(O[std::move(K)]))
        திரும்பு பொய்;
      eatWhitespace();
      தேர்வு (next()) {
      நிலை ',':
        eatWhitespace();
        தொடர்;
      நிலை '}':
        திரும்பு உண்மை;
      இயல்பு:
        திரும்பு parseError("Expected , or } after object property");
      }
    }
  }
  இயல்பு:
    எனில் (is_எண்(C))
      திரும்பு parse_எண்(C, Out);
    திரும்பு parseError("Invalid JSON value");
  }
}

தருக்க Parser::parse_எண்(எழுத்து First, மதிப்பு &Out) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.தள்ளு_back(First);
  போது (is_எண்(peek()))
    S.தள்ளு_back(next());
  எழுத்து *End;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &End, 10);
  எனில் (End == S.end() && errno != ERANGE) {
    Out = I;
    திரும்பு உண்மை;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  எனில் (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &End, 10);
    எனில் (End == S.end() && errno != ERANGE) {
      Out = UI;
      திரும்பு உண்மை;
    }
  }
  // If it's not an integer
  Out = std::strtod(S.c_str(), &End);
  திரும்பு End == S.end() || parseError("Invalid JSON value (number?)");
}

தருக்க Parser::parseString(std::string &Out) {
  // leading quote was already consumed.
  க்கு (எழுத்து C = next(); C != '"'; C = next()) {
    எனில் (LLVM_UNLIKELY(P == End))
      திரும்பு parseError("Unterminated string");
    எனில் (LLVM_UNLIKELY((C & 0x1f) == C))
      திரும்பு parseError("Control character in string");
    எனில் (LLVM_LIKELY(C != '\\')) {
      Out.தள்ளு_back(C);
      தொடர்;
    }
    // Handle escape sequence.
    தேர்வு (C = next()) {
    நிலை '"':
    நிலை '\\':
    நிலை '/':
      Out.தள்ளு_back(C);
      நிறுத்து;
    நிலை 'b':
      Out.தள்ளு_back('\b');
      நிறுத்து;
    நிலை 'f':
      Out.தள்ளு_back('\f');
      நிறுத்து;
    நிலை 'n':
      Out.தள்ளு_back('\n');
      நிறுத்து;
    நிலை 'r':
      Out.தள்ளு_back('\r');
      நிறுத்து;
    நிலை 't':
      Out.தள்ளு_back('\t');
      நிறுத்து;
    நிலை 'u':
      எனில் (!parseUnicode(Out))
        திரும்பு பொய்;
      நிறுத்து;
    இயல்பு:
      திரும்பு parseError("Invalid escape sequence");
    }
  }
  திரும்பு உண்மை;
}

நிலையான வெறுமை encodeUtf8(uint32_t Rune, std::string &Out) {
  எனில் (Rune < 0x80) {
    Out.தள்ளு_back(Rune & 0x7F);
  } இல்லையெனில் எனில் (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    Out.தள்ளு_back(FirstByte);
    Out.தள்ளு_back(SecondByte);
  } இல்லையெனில் எனில் (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    Out.தள்ளு_back(FirstByte);
    Out.தள்ளு_back(SecondByte);
    Out.தள்ளு_back(ThirdByte);
  } இல்லையெனில் எனில் (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    Out.தள்ளு_back(FirstByte);
    Out.தள்ளு_back(SecondByte);
    Out.தள்ளு_back(ThirdByte);
    Out.தள்ளு_back(FourthByte);
  } இல்லையெனில் {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
தருக்க Parser::parseUnicode(std::string &Out) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  auto Invalid = [&] { Out.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  auto Parse4Hex = [இது](uint16_t &Out) -> தருக்க {
    Out = 0;
    எழுத்து Bytes[] = {next(), next(), next(), next()};
    க்கு (unsigned எழுத்து C : Bytes) {
      எனில் (!std::isxdigit(C))
        திரும்பு parseError("Invalid \\u escape sequence");
      Out <<= 4;
      Out |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    திரும்பு உண்மை;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  எனில் (!Parse4Hex(First))
    திரும்பு பொய்;

  // We loop to allow proper surrogate-pair error handling.
  போது (உண்மை) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    எனில் (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, Out);
      திரும்பு உண்மை;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    எனில் (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      திரும்பு உண்மை;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    எனில் (LLVM_UNLIKELY(P + 2 > End || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      திரும்பு உண்மை;
    }
    P += 2;
    uint16_t Second;
    எனில் (!Parse4Hex(Second))
      திரும்பு பொய்;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    எனில் (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      தொடர்;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), Out);
    திரும்பு உண்மை;
  }
}

தருக்க Parser::parseError(மாறிலி எழுத்து *செய்தி_2) {
  முழு வரி = 1;
  மாறிலி எழுத்து *Start_Of_வரி = Start;
  க்கு (மாறிலி எழுத்து *X = Start; X < P; ++X) {
    எனில் (*X == 0x0A) {
      ++வரி;
      Start_Of_வரி = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(செய்தி_2, வரி, P - Start_Of_வரி, P - Start));
  திரும்பு பொய்;
}

Expected<மதிப்பு> parse(StringRef JSON) {
  Parser P(JSON);
  மதிப்பு E = nullptr;
  எனில் (P.checkUTF8())
    எனில் (P.parse_மதிப்பு(E))
      எனில் (P.assertEnd())
        திரும்பு std::move(E);
  திரும்பு P.takeError();
}

எழுத்து ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
வெறுமை ParseError::log(llvm::raw_ostream &OS) மாறிலி {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", வரி, Column, Offset, செய்தி_2);
}

தருக்க isUTF8(llvm::StringRef S, size_t *ErrOffset) {
  // Fast-path for ASCII, which is valid UTF-8.
  எனில் (LLVM_LIKELY(isASCII(S)))
    திரும்பு உண்மை;

  மாறிலி UTF8 *Data = reinterpret_cast<மாறிலி UTF8 *>(S.data()), *Rest = Data;
  எனில் (LLVM_LIKELY(isLegalUTF8String(&Rest, Data + S.size())))
    திரும்பு உண்மை;

  எனில் (ErrOffset)
    *ErrOffset = Rest - Data;
  திரும்பு பொய்;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::vector<UTF32> Codepoints(S.size()); // 1 codepoint per byte suffices.
  மாறிலி UTF8 *In8 = reinterpret_cast<மாறிலி UTF8 *>(S.data());
  UTF32 *Out32 = Codepoints.data();
  ConvertUTF8toUTF32(&In8, In8 + S.size(), &Out32, Out32 + Codepoints.size(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.data());
  std::string Res(4 * Codepoints.size(), 0); // 4 bytes per codepoint suffice
  மாறிலி UTF32 *In32 = Codepoints.data();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.size(), &Out8, Out8 + Res.size(),
                     strictConversion);
  Res.resize(reinterpret_cast<எழுத்து *>(Out8) - Res.data());
  திரும்பு Res;
}

நிலையான வெறுமை quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  க்கு (unsigned எழுத்து C : S) {
    எனில் (C == 0x22 || C == 0x5C)
      OS << '\\';
    எனில் (C >= 0x20) {
      OS << C;
      தொடர்;
    }
    OS << '\\';
    தேர்வு (C) {
    // A few characters are common enough to make short escapes worthwhile.
    நிலை '\t':
      OS << 't';
      நிறுத்து;
    நிலை '\n':
      OS << 'n';
      நிறுத்து;
    நிலை '\r':
      OS << 'r';
      நிறுத்து;
    இயல்பு:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::Lower, 4);
      நிறுத்து;
    }
  }
  OS << '\"';
}

வெறுமை llvm::json::OStream::மதிப்பு_2(மாறிலி மதிப்பு &V) {
  தேர்வு (V.kind()) {
  நிலை மதிப்பு::Null:
    மதிப்பு_Begin();
    OS << "null";
    திரும்பு;
  நிலை மதிப்பு::Boolean:
    மதிப்பு_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    திரும்பு;
  நிலை மதிப்பு::எண்:
    மதிப்பு_Begin();
    எனில் (V.Type == மதிப்பு::T_Integer)
      OS << *V.getAsInteger();
    இல்லையெனில் எனில் (V.Type == மதிப்பு::T_UINT64)
      OS << *V.getAsUINT64();
    இல்லையெனில்
      OS << format("%.*g", std::numeric_limits<இரட்டை>::max_digits10,
                   *V.get_As_எண்());
    திரும்பு;
  நிலை மதிப்பு::String:
    மதிப்பு_Begin();
    quote(OS, *V.getAsString());
    திரும்பு;
  நிலை மதிப்பு::Array:
    திரும்பு array([&] {
      க்கு (மாறிலி மதிப்பு &E : *V.getAsArray())
        மதிப்பு_2(E);
    });
  நிலை மதிப்பு::Object:
    திரும்பு object([&] {
      க்கு (மாறிலி Object::மதிப்பு_type *E : sortedElements(*V.getAsObject()))
        attribute(E->first, E->second);
    });
  }
}

வெறுமை llvm::json::OStream::மதிப்பு_Begin() {
  ப்ரதிஜ்ஞா(அடுக்கு.back().Ctx != Object && "Only attributes allowed here");
  எனில் (அடுக்கு.back().Has_மதிப்பு) {
    ப்ரதிஜ்ஞா(அடுக்கு.back().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  எனில் (அடுக்கு.back().Ctx == Array)
    newline();
  flushComment();
  அடுக்கு.back().Has_மதிப்பு = உண்மை;
}

வெறுமை OStream::comment(llvm::StringRef Comment) {
  ப்ரதிஜ்ஞா(PendingComment.empty() && "Only one comment per value!");
  PendingComment = Comment;
}

வெறுமை OStream::flushComment() {
  எனில் (PendingComment.empty())
    திரும்பு;
  OS << (IndentSize ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  போது (!PendingComment.empty()) {
    auto Pos = PendingComment.find("*/");
    எனில் (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } இல்லையெனில் {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (IndentSize ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  எனில் (அடுக்கு.size() > 1 && அடுக்கு.back().Ctx == Singleton) {
    எனில் (IndentSize)
      OS << ' ';
  } இல்லையெனில் {
    newline();
  }
}

வெறுமை llvm::json::OStream::newline() {
  எனில் (IndentSize) {
    OS.லிக('\n');
    OS.indent(Indent);
  }
}

வெறுமை llvm::json::OStream::arrayBegin() {
  மதிப்பு_Begin();
  அடுக்கு.emplace_back();
  அடுக்கு.back().Ctx = Array;
  Indent += IndentSize;
  OS << '[';
}

வெறுமை llvm::json::OStream::arrayEnd() {
  ப்ரதிஜ்ஞா(அடுக்கு.back().Ctx == Array);
  Indent -= IndentSize;
  எனில் (அடுக்கு.back().Has_மதிப்பு)
    newline();
  OS << ']';
  ப்ரதிஜ்ஞா(PendingComment.empty());
  அடுக்கு.எடு_back();
  ப்ரதிஜ்ஞா(!அடுக்கு.empty());
}

வெறுமை llvm::json::OStream::objectBegin() {
  மதிப்பு_Begin();
  அடுக்கு.emplace_back();
  அடுக்கு.back().Ctx = Object;
  Indent += IndentSize;
  OS << '{';
}

வெறுமை llvm::json::OStream::objectEnd() {
  ப்ரதிஜ்ஞா(அடுக்கு.back().Ctx == Object);
  Indent -= IndentSize;
  எனில் (அடுக்கு.back().Has_மதிப்பு)
    newline();
  OS << '}';
  ப்ரதிஜ்ஞா(PendingComment.empty());
  அடுக்கு.எடு_back();
  ப்ரதிஜ்ஞா(!அடுக்கு.empty());
}

வெறுமை llvm::json::OStream::attributeBegin(llvm::StringRef சாவி) {
  ப்ரதிஜ்ஞா(அடுக்கு.back().Ctx == Object);
  எனில் (அடுக்கு.back().Has_மதிப்பு)
    OS << ',';
  newline();
  flushComment();
  அடுக்கு.back().Has_மதிப்பு = உண்மை;
  அடுக்கு.emplace_back();
  அடுக்கு.back().Ctx = Singleton;
  எனில் (LLVM_LIKELY(isUTF8(சாவி))) {
    quote(OS, சாவி);
  } இல்லையெனில் {
    ப்ரதிஜ்ஞா(பொய் && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(சாவி));
  }
  OS.லிக(':');
  எனில் (IndentSize)
    OS.லிக(' ');
}

வெறுமை llvm::json::OStream::attributeEnd() {
  ப்ரதிஜ்ஞா(அடுக்கு.back().Ctx == Singleton);
  ப்ரதிஜ்ஞா(அடுக்கு.back().Has_மதிப்பு && "Attribute must have a value");
  ப்ரதிஜ்ஞா(PendingComment.empty());
  அடுக்கு.எடு_back();
  ப்ரதிஜ்ஞா(அடுக்கு.back().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_மதிப்பு_Begin() {
  மதிப்பு_Begin();
  அடுக்கு.emplace_back();
  அடுக்கு.back().Ctx = Raw_மதிப்பு;
  திரும்பு OS;
}

வெறுமை llvm::json::OStream::raw_மதிப்பு_End() {
  ப்ரதிஜ்ஞா(அடுக்கு.back().Ctx == Raw_மதிப்பு);
  அடுக்கு.எடு_back();
}

} // namespace json
} // namespace llvm

வெறுமை llvm::format_provider<llvm::json::மதிப்பு>::format(
    மாறிலி llvm::json::மதிப்பு &E, raw_ostream &OS, StringRef Options) {
  unsigned IndentAmount = 0;
  எனில் (!Options.empty() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).மதிப்பு_2(E);
}

