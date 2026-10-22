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

मानम् &Object::संक्रियकः[](ध्रुव Object_कुञ्चिका &K) {
  प्रत्यागम try_emplace(K, शून्यसूचकः).first->getSecond();
}
मानम् &Object::संक्रियकः[](Object_कुञ्चिका &&K) {
  प्रत्यागम try_emplace(std::move(K), शून्यसूचकः).first->getSecond();
}
मानम् *Object::get(StringRef K) {
  स्वचालित I = find(K);
  यदि (I == अन्तः())
    प्रत्यागम शून्यसूचकः;
  प्रत्यागम &I->second;
}
ध्रुव मानम् *Object::get(StringRef K) ध्रुव {
  स्वचालित I = find(K);
  यदि (I == अन्तः())
    प्रत्यागम शून्यसूचकः;
  प्रत्यागम &I->second;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ध्रुव {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->getAsNull();
  प्रत्यागम std::nullopt;
}
std::optional<तार्किक> Object::getBoolean(StringRef K) ध्रुव {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->getAsBoolean();
  प्रत्यागम std::nullopt;
}
std::optional<द्विगुण> Object::get_सङ्ख्या(StringRef K) ध्रुव {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->get_As_सङ्ख्या();
  प्रत्यागम std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ध्रुव {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->getAsInteger();
  प्रत्यागम std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ध्रुव {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->getAsString();
  प्रत्यागम std::nullopt;
}
ध्रुव json::Object *Object::getObject(StringRef K) ध्रुव {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->getAsObject();
  प्रत्यागम शून्यसूचकः;
}
json::Object *Object::getObject(StringRef K) {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->getAsObject();
  प्रत्यागम शून्यसूचकः;
}
ध्रुव json::Array *Object::getArray(StringRef K) ध्रुव {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->getAsArray();
  प्रत्यागम शून्यसूचकः;
}
json::Array *Object::getArray(StringRef K) {
  यदि (स्वचालित *V = get(K))
    प्रत्यागम V->getAsArray();
  प्रत्यागम शून्यसूचकः;
}
तार्किक संक्रियकः==(ध्रुव Object &LHS, ध्रुव Object &RHS) { प्रत्यागम LHS.M == RHS.M; }

Array::Array(std::initializer_list<मानम्> Elements) {
  V.reserve(Elements.आकार_2());
  प्रति (ध्रुव मानम् &V : Elements) {
    emplace_पृष्ठभागः(शून्यसूचकः);
    पृष्ठभागः().moveFrom(std::move(V));
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
    स्मृतिं_प्रतिलिख(&Union, &M.Union, परिमाण(Union));
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
    स्मृतिं_प्रतिलिख(&Union, &M.Union, परिमाण(Union));
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

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_विधानम् शून्य मानम्::dump() ध्रुव {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#यदि_अन्तः

तार्किक संक्रियकः==(ध्रुव मानम् &L, ध्रुव मानम् &R) {
  यदि (L.विधा() != R.विधा())
    प्रत्यागम असत्य;
  विकल्प (L.विधा()) {
  पक्ष मानम्::Null:
    प्रत्यागम *L.getAsNull() == *R.getAsNull();
  पक्ष मानम्::Boolean:
    प्रत्यागम *L.getAsBoolean() == *R.getAsBoolean();
  पक्ष मानम्::सङ्ख्या:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    यदि (L.Type == मानम्::T_Integer || R.Type == मानम्::T_Integer)
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

शून्य मार्गः::report(llvm::StringLiteral सन्देशः_4) {
  // Walk up to the root context, and count the number of segments.
  अचिह्नित गणनम्_2 = 0;
  ध्रुव मार्गः *P;
  प्रति (P = एतत्; P->Parent != शून्यसूचकः; P = P->Parent)
    ++गणनम्_2;
  मार्गः::मूलम्_2 *R = P->Seg.मूलम्_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_सन्देशः_2 = सन्देशः_4;
  R->Error_मार्गः.resize(गणनम्_2);
  स्वचालित It = R->Error_मार्गः.begin();
  प्रति (P = एतत्; P->Parent != शून्यसूचकः; P = P->Parent)
    *It++ = P->Seg;
}

Error मार्गः::मूलम्_2::getError() ध्रुव {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_सन्देशः_2.रिक्तानि() ? "invalid JSON contents" : Error_सन्देशः_2);
  यदि (Error_मार्गः.रिक्तानि()) {
    यदि (!Name.रिक्तानि())
      OS << " when parsing " << Name;
  } अन्यथा {
    OS << " at " << (Name.रिक्तानि() ? "(root)" : Name);
    प्रति (ध्रुव मार्गः::Segment &S : llvm::reverse(Error_मार्गः)) {
      यदि (S.is_क्षेत्रम्())
        OS << '.' << S.क्षेत्रम्();
      अन्यथा
        OS << '[' << S.सूचकांक() << ']';
    }
  }
  प्रत्यागम createStringError(llvm::inconvertible_Error_संहिता(), S);
}

std::सदिशम्<ध्रुव Object::मानम्_type *> sortedElements(ध्रुव Object &O) {
  std::सदिशम्<ध्रुव Object::मानम्_type *> Elements;
  प्रति (ध्रुव स्वचालित &E : O)
    Elements.प्रक्षिप_पृष्ठभागः(&E);
  llvm::क्रमय(Elements,
             [](ध्रुव Object::मानम्_type *L, ध्रुव Object::मानम्_type *R) {
               प्रत्यागम L->first < R->first;
             });
  प्रत्यागम Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
स्थिर शून्य abbreviate(ध्रुव मानम् &V, OStream &JOS) {
  विकल्प (V.विधा()) {
  पक्ष मानम्::Array:
    JOS.raw_मानम्(V.getAsArray()->रिक्तानि() ? "[]" : "[ ... ]");
    भङ्ग;
  पक्ष मानम्::Object:
    JOS.raw_मानम्(V.getAsObject()->रिक्तानि() ? "{}" : "{ ... }");
    भङ्ग;
  पक्ष मानम्::String: {
    llvm::StringRef S = *V.getAsString();
    यदि (S.आकार_2() < 40) {
      JOS.मानम्_2(V);
    } अन्यथा {
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
  विकल्प (V.विधा()) {
  पक्ष मानम्::Array:
    JOS.array([&] {
      प्रति (ध्रुव स्वचालित &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    भङ्ग;
  पक्ष मानम्::Object:
    JOS.object([&] {
      प्रति (ध्रुव स्वचालित *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->first);
        abbreviate(KV->second, JOS);
        JOS.attribute_अन्तः();
      }
    });
    भङ्ग;
  स्वभाव:
    JOS.मानम्_2(V);
  }
}

शून्य मार्गः::मूलम्_2::printErrorContext(ध्रुव मानम् &R, raw_ostream &OS) ध्रुव {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  स्वचालित Print_मानम् = [&](ध्रुव मानम् &V, ArrayRef<Segment> मार्गः, स्वचालित &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    स्वचालित HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_सन्देशः_2.दत्त(), Error_सन्देशः_2.आकार_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    यदि (मार्गः.रिक्तानि()) // We reached our target.
      प्रत्यागम HighlightCurrent();
    ध्रुव Segment &S = मार्गः.पृष्ठभागः(); // Path is in reverse order.
    यदि (S.is_क्षेत्रम्()) {
      // Current node is an object, path names a field.
      llvm::StringRef क्षेत्रम्_Name = S.क्षेत्रम्();
      ध्रुव Object *O = V.getAsObject();
      यदि (!O || !O->get(क्षेत्रम्_Name))
        प्रत्यागम HighlightCurrent();
      JOS.object([&] {
        प्रति (ध्रुव स्वचालित *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->first);
          यदि (क्षेत्रम्_Name == StringRef(KV->first))
            Recurse(KV->second, मार्गः.drop_पृष्ठभागः(), Recurse);
          अन्यथा
            abbreviate(KV->second, JOS);
          JOS.attribute_अन्तः();
        }
      });
    } अन्यथा {
      // Current node is an array, path names an element.
      ध्रुव Array *A = V.getAsArray();
      यदि (!A || S.सूचकांक() >= A->आकार_2())
        प्रत्यागम HighlightCurrent();
      JOS.array([&] {
        अचिह्नित Current = 0;
        प्रति (ध्रुव स्वचालित &V : *A) {
          यदि (Current++ == S.सूचकांक())
            Recurse(V, मार्गः.drop_पृष्ठभागः(), Recurse);
          अन्यथा
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_मानम्(R, Error_मार्गः, Print_मानम्);
}

नामाकाश {
// Simple recursive-descent JSON parser.
वर्ग Parser {
सार्वजनिक:
  Parser(StringRef JSON)
      : Start(JSON.begin()), P(JSON.begin()), अन्तः_2(JSON.अन्तः()) {}

  तार्किक checkUTF8() {
    size_t Err_विस्थापनम्;
    यदि (isUTF8(StringRef(Start, अन्तः_2 - Start), &Err_विस्थापनम्))
      प्रत्यागम सत्य;
    P = Start + Err_विस्थापनम्; // For line/column calculation.
    प्रत्यागम parseError("Invalid UTF-8 sequence");
  }

  तार्किक parse_मानम्(मानम् &बहिर्गमनम्);

  तार्किक assert_अन्तः() {
    eatWhitespace();
    यदि (P == अन्तः_2)
      प्रत्यागम सत्य;
    प्रत्यागम parseError("Text after end of document");
  }

  Error takeError() {
    प्रतिज्ञा(Err);
    प्रत्यागम std::move(*Err);
  }

गुप्त:
  शून्य eatWhitespace() {
    यावत् (P != अन्तः_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  तार्किक parse_सङ्ख्या(वर्ण First, मानम् &बहिर्गमनम्);
  तार्किक parseString(std::string &बहिर्गमनम्);
  तार्किक parseUnicode(std::string &बहिर्गमनम्);
  तार्किक parseError(ध्रुव वर्ण *सन्देशः_4); // always returns false

  वर्ण अग्र() { प्रत्यागम P == अन्तः_2 ? 0 : *P++; }
  वर्ण peek() { प्रत्यागम P == अन्तः_2 ? 0 : *P; }
  स्थिर तार्किक is_सङ्ख्या(वर्ण C) {
    प्रत्यागम C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ध्रुव वर्ण *Start, *P, *अन्तः_2;
};
} // namespace

तार्किक Parser::parse_मानम्(मानम् &बहिर्गमनम्) {
  eatWhitespace();
  यदि (P == अन्तः_2)
    प्रत्यागम parseError("Unexpected EOF");
  विकल्प (वर्ण C = अग्र()) {
  // Bare null/true/false are easy - first char identifies them.
  पक्ष 'n':
    बहिर्गमनम् = शून्यसूचकः;
    प्रत्यागम (अग्र() == 'u' && अग्र() == 'l' && अग्र() == 'l') ||
           parseError("Invalid JSON value (null?)");
  पक्ष 't':
    बहिर्गमनम् = सत्य;
    प्रत्यागम (अग्र() == 'r' && अग्र() == 'u' && अग्र() == 'e') ||
           parseError("Invalid JSON value (true?)");
  पक्ष 'f':
    बहिर्गमनम् = असत्य;
    प्रत्यागम (अग्र() == 'a' && अग्र() == 'l' && अग्र() == 's' && अग्र() == 'e') ||
           parseError("Invalid JSON value (false?)");
  पक्ष '"': {
    std::string S;
    यदि (parseString(S)) {
      बहिर्गमनम् = std::move(S);
      प्रत्यागम सत्य;
    }
    प्रत्यागम असत्य;
  }
  पक्ष '[': {
    बहिर्गमनम् = Array{};
    Array &A = *बहिर्गमनम्.getAsArray();
    eatWhitespace();
    यदि (peek() == ']') {
      ++P;
      प्रत्यागम सत्य;
    }
    प्रति (;;) {
      A.emplace_पृष्ठभागः(शून्यसूचकः);
      यदि (!parse_मानम्(A.पृष्ठभागः()))
        प्रत्यागम असत्य;
      eatWhitespace();
      विकल्प (अग्र()) {
      पक्ष ',':
        eatWhitespace();
        अनुवर्त;
      पक्ष ']':
        प्रत्यागम सत्य;
      स्वभाव:
        प्रत्यागम parseError("Expected , or ] after array element");
      }
    }
  }
  पक्ष '{': {
    बहिर्गमनम् = Object{};
    Object &O = *बहिर्गमनम्.getAsObject();
    eatWhitespace();
    यदि (peek() == '}') {
      ++P;
      प्रत्यागम सत्य;
    }
    प्रति (;;) {
      यदि (अग्र() != '"')
        प्रत्यागम parseError("Expected object key");
      std::string K;
      यदि (!parseString(K))
        प्रत्यागम असत्य;
      eatWhitespace();
      यदि (अग्र() != ':')
        प्रत्यागम parseError("Expected : after object key");
      eatWhitespace();
      यदि (!parse_मानम्(O[std::move(K)]))
        प्रत्यागम असत्य;
      eatWhitespace();
      विकल्प (अग्र()) {
      पक्ष ',':
        eatWhitespace();
        अनुवर्त;
      पक्ष '}':
        प्रत्यागम सत्य;
      स्वभाव:
        प्रत्यागम parseError("Expected , or } after object property");
      }
    }
  }
  स्वभाव:
    यदि (is_सङ्ख्या(C))
      प्रत्यागम parse_सङ्ख्या(C, बहिर्गमनम्);
    प्रत्यागम parseError("Invalid JSON value");
  }
}

तार्किक Parser::parse_सङ्ख्या(वर्ण First, मानम् &बहिर्गमनम्) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.प्रक्षिप_पृष्ठभागः(First);
  यावत् (is_सङ्ख्या(peek()))
    S.प्रक्षिप_पृष्ठभागः(अग्र());
  वर्ण *अन्तः_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &अन्तः_2, 10);
  यदि (अन्तः_2 == S.अन्तः() && errno != ERANGE) {
    बहिर्गमनम् = I;
    प्रत्यागम सत्य;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  यदि (First != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &अन्तः_2, 10);
    यदि (अन्तः_2 == S.अन्तः() && errno != ERANGE) {
      बहिर्गमनम् = UI;
      प्रत्यागम सत्य;
    }
  }
  // If it's not an integer
  बहिर्गमनम् = std::strtod(S.c_str(), &अन्तः_2);
  प्रत्यागम अन्तः_2 == S.अन्तः() || parseError("Invalid JSON value (number?)");
}

तार्किक Parser::parseString(std::string &बहिर्गमनम्) {
  // leading quote was already consumed.
  प्रति (वर्ण C = अग्र(); C != '"'; C = अग्र()) {
    यदि (LLVM_UNLIKELY(P == अन्तः_2))
      प्रत्यागम parseError("Unterminated string");
    यदि (LLVM_UNLIKELY((C & 0x1f) == C))
      प्रत्यागम parseError("Control character in string");
    यदि (LLVM_LIKELY(C != '\\')) {
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(C);
      अनुवर्त;
    }
    // Handle escape sequence.
    विकल्प (C = अग्र()) {
    पक्ष '"':
    पक्ष '\\':
    पक्ष '/':
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(C);
      भङ्ग;
    पक्ष 'b':
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः('\b');
      भङ्ग;
    पक्ष 'f':
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः('\f');
      भङ्ग;
    पक्ष 'n':
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः('\n');
      भङ्ग;
    पक्ष 'r':
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः('\r');
      भङ्ग;
    पक्ष 't':
      बहिर्गमनम्.प्रक्षिप_पृष्ठभागः('\t');
      भङ्ग;
    पक्ष 'u':
      यदि (!parseUnicode(बहिर्गमनम्))
        प्रत्यागम असत्य;
      भङ्ग;
    स्वभाव:
      प्रत्यागम parseError("Invalid escape sequence");
    }
  }
  प्रत्यागम सत्य;
}

स्थिर शून्य encodeUtf8(uint32_t Rune, std::string &बहिर्गमनम्) {
  यदि (Rune < 0x80) {
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(Rune & 0x7F);
  } अन्यथा यदि (Rune < 0x800) {
    uint8_t FirstByte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Rune & 0x3F);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(FirstByte);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(SecondByte);
  } अन्यथा यदि (Rune < 0x10000) {
    uint8_t FirstByte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Rune & 0x3F);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(FirstByte);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(SecondByte);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(ThirdByte);
  } अन्यथा यदि (Rune < 0x110000) {
    uint8_t FirstByte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(FirstByte);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(SecondByte);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(ThirdByte);
    बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(FourthByte);
  } अन्यथा {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
तार्किक Parser::parseUnicode(std::string &बहिर्गमनम्) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  स्वचालित Invalid = [&] { बहिर्गमनम्.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  स्वचालित Parse4Hex = [एतत्](uint16_t &बहिर्गमनम्) -> तार्किक {
    बहिर्गमनम् = 0;
    वर्ण अष्टकानि_2[] = {अग्र(), अग्र(), अग्र(), अग्र()};
    प्रति (अचिह्नित वर्ण C : अष्टकानि_2) {
      यदि (!std::isxdigit(C))
        प्रत्यागम parseError("Invalid \\u escape sequence");
      बहिर्गमनम् <<= 4;
      बहिर्गमनम् |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    प्रत्यागम सत्य;
  };
  uint16_t First; // UTF-16 code unit from the first \u escape.
  यदि (!Parse4Hex(First))
    प्रत्यागम असत्य;

  // We loop to allow proper surrogate-pair error handling.
  यावत् (सत्य) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    यदि (LLVM_LIKELY(First < 0xD800 || First >= 0xE000)) {
      encodeUtf8(First, बहिर्गमनम्);
      प्रत्यागम सत्य;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    यदि (LLVM_UNLIKELY(First >= 0xDC00)) {
      Invalid();
      प्रत्यागम सत्य;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    यदि (LLVM_UNLIKELY(P + 2 > अन्तः_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      प्रत्यागम सत्य;
    }
    P += 2;
    uint16_t Second;
    यदि (!Parse4Hex(Second))
      प्रत्यागम असत्य;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    यदि (LLVM_UNLIKELY(Second < 0xDC00 || Second >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      First = Second; // Second escape still needs to be processed.
      अनुवर्त;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((First - 0xD800) << 10) | (Second - 0xDC00), बहिर्गमनम्);
    प्रत्यागम सत्य;
  }
}

तार्किक Parser::parseError(ध्रुव वर्ण *सन्देशः_4) {
  पूर्णाङ्क पङ्क्तिः = 1;
  ध्रुव वर्ण *Start_सम्बन्धि_पङ्क्तिः = Start;
  प्रति (ध्रुव वर्ण *X = Start; X < P; ++X) {
    यदि (*X == 0x0A) {
      ++पङ्क्तिः;
      Start_सम्बन्धि_पङ्क्तिः = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(सन्देशः_4, पङ्क्तिः, P - Start_सम्बन्धि_पङ्क्तिः, P - Start));
  प्रत्यागम असत्य;
}

Expected<मानम्> parse(StringRef JSON) {
  Parser P(JSON);
  मानम् E = शून्यसूचकः;
  यदि (P.checkUTF8())
    यदि (P.parse_मानम्(E))
      यदि (P.assert_अन्तः())
        प्रत्यागम std::move(E);
  प्रत्यागम P.takeError();
}

वर्ण ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
शून्य ParseError::लघुगणकः(llvm::raw_ostream &OS) ध्रुव {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", पङ्क्तिः, Column, विस्थापनम्, सन्देशः_4);
}

तार्किक isUTF8(llvm::StringRef S, size_t *Err_विस्थापनम्) {
  // Fast-path for ASCII, which is valid UTF-8.
  यदि (LLVM_LIKELY(isASCII(S)))
    प्रत्यागम सत्य;

  ध्रुव UTF8 *दत्त_2 = reinterpret_cast<ध्रुव UTF8 *>(S.दत्त()), *Rest = दत्त_2;
  यदि (LLVM_LIKELY(isLegalUTF8String(&Rest, दत्त_2 + S.आकार_2())))
    प्रत्यागम सत्य;

  यदि (Err_विस्थापनम्)
    *Err_विस्थापनम् = Rest - दत्त_2;
  प्रत्यागम असत्य;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::सदिशम्<UTF32> Codepoints(S.आकार_2()); // 1 codepoint per byte suffices.
  ध्रुव UTF8 *In8 = reinterpret_cast<ध्रुव UTF8 *>(S.दत्त());
  UTF32 *Out32 = Codepoints.दत्त();
  ConvertUTF8toUTF32(&In8, In8 + S.आकार_2(), &Out32, Out32 + Codepoints.आकार_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.दत्त());
  std::string Res(4 * Codepoints.आकार_2(), 0); // 4 bytes per codepoint suffice
  ध्रुव UTF32 *In32 = Codepoints.दत्त();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.आकार_2(), &Out8, Out8 + Res.आकार_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<वर्ण *>(Out8) - Res.दत्त());
  प्रत्यागम Res;
}

स्थिर शून्य quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  प्रति (अचिह्नित वर्ण C : S) {
    यदि (C == 0x22 || C == 0x5C)
      OS << '\\';
    यदि (C >= 0x20) {
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
      llvm::write_hex(OS, C, llvm::HexPrintStyle::अधस्तनम्_2, 4);
      भङ्ग;
    }
  }
  OS << '\"';
}

शून्य llvm::json::OStream::मानम्_2(ध्रुव मानम् &V) {
  विकल्प (V.विधा()) {
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
    यदि (V.Type == मानम्::T_Integer)
      OS << *V.getAsInteger();
    अन्यथा यदि (V.Type == मानम्::T_UINT64)
      OS << *V.getAsUINT64();
    अन्यथा
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
  प्रतिज्ञा(राशिः.पृष्ठभागः().Ctx != Object && "Only attributes allowed here");
  यदि (राशिः.पृष्ठभागः().Has_मानम्) {
    प्रतिज्ञा(राशिः.पृष्ठभागः().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  यदि (राशिः.पृष्ठभागः().Ctx == Array)
    newline();
  flushComment();
  राशिः.पृष्ठभागः().Has_मानम् = सत्य;
}

शून्य OStream::comment(llvm::StringRef Comment) {
  प्रतिज्ञा(PendingComment.रिक्तानि() && "Only one comment per value!");
  PendingComment = Comment;
}

शून्य OStream::flushComment() {
  यदि (PendingComment.रिक्तानि())
    प्रत्यागम;
  OS << (Indent_आकार ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  यावत् (!PendingComment.रिक्तानि()) {
    स्वचालित Pos = PendingComment.find("*/");
    यदि (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } अन्यथा {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_आकार ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  यदि (राशिः.आकार_2() > 1 && राशिः.पृष्ठभागः().Ctx == Singleton) {
    यदि (Indent_आकार)
      OS << ' ';
  } अन्यथा {
    newline();
  }
}

शून्य llvm::json::OStream::newline() {
  यदि (Indent_आकार) {
    OS.लिख('\n');
    OS.indent(Indent);
  }
}

शून्य llvm::json::OStream::arrayBegin() {
  मानम्_Begin();
  राशिः.emplace_पृष्ठभागः();
  राशिः.पृष्ठभागः().Ctx = Array;
  Indent += Indent_आकार;
  OS << '[';
}

शून्य llvm::json::OStream::array_अन्तः() {
  प्रतिज्ञा(राशिः.पृष्ठभागः().Ctx == Array);
  Indent -= Indent_आकार;
  यदि (राशिः.पृष्ठभागः().Has_मानम्)
    newline();
  OS << ']';
  प्रतिज्ञा(PendingComment.रिक्तानि());
  राशिः.निष्कासय_पृष्ठभागः();
  प्रतिज्ञा(!राशिः.रिक्तानि());
}

शून्य llvm::json::OStream::objectBegin() {
  मानम्_Begin();
  राशिः.emplace_पृष्ठभागः();
  राशिः.पृष्ठभागः().Ctx = Object;
  Indent += Indent_आकार;
  OS << '{';
}

शून्य llvm::json::OStream::object_अन्तः() {
  प्रतिज्ञा(राशिः.पृष्ठभागः().Ctx == Object);
  Indent -= Indent_आकार;
  यदि (राशिः.पृष्ठभागः().Has_मानम्)
    newline();
  OS << '}';
  प्रतिज्ञा(PendingComment.रिक्तानि());
  राशिः.निष्कासय_पृष्ठभागः();
  प्रतिज्ञा(!राशिः.रिक्तानि());
}

शून्य llvm::json::OStream::attributeBegin(llvm::StringRef कुञ्चिका) {
  प्रतिज्ञा(राशिः.पृष्ठभागः().Ctx == Object);
  यदि (राशिः.पृष्ठभागः().Has_मानम्)
    OS << ',';
  newline();
  flushComment();
  राशिः.पृष्ठभागः().Has_मानम् = सत्य;
  राशिः.emplace_पृष्ठभागः();
  राशिः.पृष्ठभागः().Ctx = Singleton;
  यदि (LLVM_LIKELY(isUTF8(कुञ्चिका))) {
    quote(OS, कुञ्चिका);
  } अन्यथा {
    प्रतिज्ञा(असत्य && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(कुञ्चिका));
  }
  OS.लिख(':');
  यदि (Indent_आकार)
    OS.लिख(' ');
}

शून्य llvm::json::OStream::attribute_अन्तः() {
  प्रतिज्ञा(राशिः.पृष्ठभागः().Ctx == Singleton);
  प्रतिज्ञा(राशिः.पृष्ठभागः().Has_मानम् && "Attribute must have a value");
  प्रतिज्ञा(PendingComment.रिक्तानि());
  राशिः.निष्कासय_पृष्ठभागः();
  प्रतिज्ञा(राशिः.पृष्ठभागः().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_मानम्_Begin() {
  मानम्_Begin();
  राशिः.emplace_पृष्ठभागः();
  राशिः.पृष्ठभागः().Ctx = Raw_मानम्;
  प्रत्यागम OS;
}

शून्य llvm::json::OStream::raw_मानम्_अन्तः() {
  प्रतिज्ञा(राशिः.पृष्ठभागः().Ctx == Raw_मानम्);
  राशिः.निष्कासय_पृष्ठभागः();
}

} // namespace json
} // namespace llvm

शून्य llvm::format_provider<llvm::json::मानम्>::format(
    ध्रुव llvm::json::मानम् &E, raw_ostream &OS, StringRef Options) {
  अचिह्नित IndentAmount = 0;
  यदि (!Options.रिक्तानि() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).मानम्_2(E);
}

