//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/JSON.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/ConvertUTF.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/Format.h"
#అన్తర్భావ "llvm/Support/NativeFormatting.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cctype>
#అన్తర్భావ <cerrno>
#అన్తర్భావ <optional>

నామాకాశ llvm {
నామాకాశ json {

విలువ &Object::క్రియాకారకం[](స్థిరరాశి Object_కీలకం &K) {
  తిరిగి try_emplace(K, శూన్యసూచిక).మొదటి_2->get_రెండవ();
}
విలువ &Object::క్రియాకారకం[](Object_కీలకం &&K) {
  తిరిగి try_emplace(std::move(K), శూన్యసూచిక).మొదటి_2->get_రెండవ();
}
విలువ *Object::get(StringRef K) {
  స్వయంచాలక I = కనుగొను(K);
  ఒకవేళ (I == చివర())
    తిరిగి శూన్యసూచిక;
  తిరిగి &I->రెండవ_2;
}
స్థిరరాశి విలువ *Object::get(StringRef K) స్థిరరాశి {
  స్వయంచాలక I = కనుగొను(K);
  ఒకవేళ (I == చివర())
    తిరిగి శూన్యసూచిక;
  తిరిగి &I->రెండవ_2;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->getAsNull();
  తిరిగి std::nullopt;
}
std::optional<తార్కిక> Object::getBoolean(StringRef K) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->getAsBoolean();
  తిరిగి std::nullopt;
}
std::optional<ద్విగుణ> Object::get_సంఖ్య(StringRef K) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->get_As_సంఖ్య();
  తిరిగి std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->getAsInteger();
  తిరిగి std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->getAsString();
  తిరిగి std::nullopt;
}
స్థిరరాశి json::Object *Object::getObject(StringRef K) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->getAsObject();
  తిరిగి శూన్యసూచిక;
}
json::Object *Object::getObject(StringRef K) {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->getAsObject();
  తిరిగి శూన్యసూచిక;
}
స్థిరరాశి json::Array *Object::getArray(StringRef K) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->getAsArray();
  తిరిగి శూన్యసూచిక;
}
json::Array *Object::getArray(StringRef K) {
  ఒకవేళ (స్వయంచాలక *V = get(K))
    తిరిగి V->getAsArray();
  తిరిగి శూన్యసూచిక;
}
తార్కిక క్రియాకారకం==(స్థిరరాశి Object &LHS, స్థిరరాశి Object &RHS) { తిరిగి LHS.M == RHS.M; }

Array::Array(std::initializer_list<విలువ> Elements) {
  V.reserve(Elements.కొలత_2());
  కొరకు (స్థిరరాశి విలువ &V : Elements) {
    emplace_వెనుక(శూన్యసూచిక);
    వెనుక().moveFrom(std::move(V));
  }
}

విలువ::విలువ(std::initializer_list<విలువ> Elements)
    : విలువ(json::Array(Elements)) {}

శూన్య విలువ::copyFrom(స్థిరరాశి విలువ &M) {
  Type = M.Type;
  ఎంపిక (Type) {
  స్థితి T_Null:
  స్థితి T_Boolean:
  స్థితి T_Double:
  స్థితి T_Integer:
  స్థితి T_UINT64:
    జ్ఞాపకం_నకలు(&Union, &M.Union, పరిమాణం(Union));
    ఆపు;
  స్థితి T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ఆపు;
  స్థితి T_String:
    create<std::string>(M.as<std::string>());
    ఆపు;
  స్థితి T_Object:
    create<json::Object>(M.as<json::Object>());
    ఆపు;
  స్థితి T_Array:
    create<json::Array>(M.as<json::Array>());
    ఆపు;
  }
}

శూన్య విలువ::moveFrom(స్థిరరాశి విలువ &&M) {
  Type = M.Type;
  ఎంపిక (Type) {
  స్థితి T_Null:
  స్థితి T_Boolean:
  స్థితి T_Double:
  స్థితి T_Integer:
  స్థితి T_UINT64:
    జ్ఞాపకం_నకలు(&Union, &M.Union, పరిమాణం(Union));
    ఆపు;
  స్థితి T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ఆపు;
  స్థితి T_String:
    create<std::string>(std::move(M.as<std::string>()));
    ఆపు;
  స్థితి T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    ఆపు;
  స్థితి T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    ఆపు;
  }
  const_cast<విలువ &>(M).destroy();
  M.Type = T_Null;
}

శూన్య విలువ::destroy() {
  ఎంపిక (Type) {
  స్థితి T_Null:
  స్థితి T_Boolean:
  స్థితి T_Double:
  స్థితి T_Integer:
  స్థితి T_UINT64:
    ఆపు;
  స్థితి T_StringRef:
    as<StringRef>().~StringRef();
    ఆపు;
  స్థితి T_String:
    as<std::string>().~basic_string();
    ఆపు;
  స్థితి T_Object:
    as<json::Object>().~Object();
    ఆపు;
  స్థితి T_Array:
    as<json::Array>().~Array();
    ఆపు;
  }
}

శూన్య విలువ::print(llvm::raw_ostream &OS) స్థిరరాశి { OS << *ఇది; }

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య విలువ::dump() స్థిరరాశి {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#అయితే_ముగింపు

తార్కిక క్రియాకారకం==(స్థిరరాశి విలువ &L, స్థిరరాశి విలువ &R) {
  ఒకవేళ (L.రకం_2() != R.రకం_2())
    తిరిగి అబద్ధం;
  ఎంపిక (L.రకం_2()) {
  స్థితి విలువ::Null:
    తిరిగి *L.getAsNull() == *R.getAsNull();
  స్థితి విలువ::Boolean:
    తిరిగి *L.getAsBoolean() == *R.getAsBoolean();
  స్థితి విలువ::సంఖ్య:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    ఒకవేళ (L.Type == విలువ::T_Integer || R.Type == విలువ::T_Integer)
      తిరిగి L.getAsInteger() == R.getAsInteger();
    తిరిగి *L.get_As_సంఖ్య() == *R.get_As_సంఖ్య();
  స్థితి విలువ::String:
    తిరిగి *L.getAsString() == *R.getAsString();
  స్థితి విలువ::Array:
    తిరిగి *L.getAsArray() == *R.getAsArray();
  స్థితి విలువ::Object:
    తిరిగి *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

శూన్య మార్గం::report(llvm::StringLiteral సందేశం_4) {
  // Walk up to the root context, and count the number of segments.
  గుర్తులేని లెక్క_2 = 0;
  స్థిరరాశి మార్గం *P;
  కొరకు (P = ఇది; P->మాతృ_2 != శూన్యసూచిక; P = P->మాతృ_2)
    ++లెక్క_2;
  మార్గం::మూలం_2 *R = P->Seg.మూలం_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_సందేశం_2 = సందేశం_4;
  R->Error_మార్గం.resize(లెక్క_2);
  స్వయంచాలక It = R->Error_మార్గం.begin();
  కొరకు (P = ఇది; P->మాతృ_2 != శూన్యసూచిక; P = P->మాతృ_2)
    *It++ = P->Seg;
}

Error మార్గం::మూలం_2::getError() స్థిరరాశి {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_సందేశం_2.ఖాళీలు() ? "invalid JSON contents" : Error_సందేశం_2);
  ఒకవేళ (Error_మార్గం.ఖాళీలు()) {
    ఒకవేళ (!Name.ఖాళీలు())
      OS << " when parsing " << Name;
  } లేకపోతే {
    OS << " at " << (Name.ఖాళీలు() ? "(root)" : Name);
    కొరకు (స్థిరరాశి మార్గం::Segment &S : llvm::తిరగేయి(Error_మార్గం)) {
      ఒకవేళ (S.is_క్షేత్రం())
        OS << '.' << S.క్షేత్రం();
      లేకపోతే
        OS << '[' << S.సూచీ() << ']';
    }
  }
  తిరిగి createStringError(llvm::inconvertible_Error_సంకేతం(), S);
}

std::సదిశ<స్థిరరాశి Object::విలువ_type *> sortedElements(స్థిరరాశి Object &O) {
  std::సదిశ<స్థిరరాశి Object::విలువ_type *> Elements;
  కొరకు (స్థిరరాశి స్వయంచాలక &E : O)
    Elements.నెట్టు_వెనుక(&E);
  llvm::క్రమీకరించు(Elements,
             [](స్థిరరాశి Object::విలువ_type *L, స్థిరరాశి Object::విలువ_type *R) {
               తిరిగి L->మొదటి_2 < R->మొదటి_2;
             });
  తిరిగి Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
స్థిర శూన్య abbreviate(స్థిరరాశి విలువ &V, OStream &JOS) {
  ఎంపిక (V.రకం_2()) {
  స్థితి విలువ::Array:
    JOS.raw_విలువ(V.getAsArray()->ఖాళీలు() ? "[]" : "[ ... ]");
    ఆపు;
  స్థితి విలువ::Object:
    JOS.raw_విలువ(V.getAsObject()->ఖాళీలు() ? "{}" : "{ ... }");
    ఆపు;
  స్థితి విలువ::String: {
    llvm::StringRef S = *V.getAsString();
    ఒకవేళ (S.కొలత_2() < 40) {
      JOS.విలువ_2(V);
    } లేకపోతే {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.విలువ_2(Truncated);
    }
    ఆపు;
  }
  అప్రమేయ:
    JOS.విలువ_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
స్థిర శూన్య abbreviateChildren(స్థిరరాశి విలువ &V, OStream &JOS) {
  ఎంపిక (V.రకం_2()) {
  స్థితి విలువ::Array:
    JOS.array([&] {
      కొరకు (స్థిరరాశి స్వయంచాలక &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    ఆపు;
  స్థితి విలువ::Object:
    JOS.object([&] {
      కొరకు (స్థిరరాశి స్వయంచాలక *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->మొదటి_2);
        abbreviate(KV->రెండవ_2, JOS);
        JOS.attribute_చివర();
      }
    });
    ఆపు;
  అప్రమేయ:
    JOS.విలువ_2(V);
  }
}

శూన్య మార్గం::మూలం_2::printErrorContext(స్థిరరాశి విలువ &R, raw_ostream &OS) స్థిరరాశి {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  స్వయంచాలక Print_విలువ = [&](స్థిరరాశి విలువ &V, ArrayRef<Segment> మార్గం, స్వయంచాలక &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    స్వయంచాలక HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_సందేశం_2.దత్తాంశం(), Error_సందేశం_2.కొలత_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    ఒకవేళ (మార్గం.ఖాళీలు()) // We reached our target.
      తిరిగి HighlightCurrent();
    స్థిరరాశి Segment &S = మార్గం.వెనుక(); // Path is in reverse order.
    ఒకవేళ (S.is_క్షేత్రం()) {
      // Current node is an object, path names a field.
      llvm::StringRef క్షేత్రం_Name = S.క్షేత్రం();
      స్థిరరాశి Object *O = V.getAsObject();
      ఒకవేళ (!O || !O->get(క్షేత్రం_Name))
        తిరిగి HighlightCurrent();
      JOS.object([&] {
        కొరకు (స్థిరరాశి స్వయంచాలక *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->మొదటి_2);
          ఒకవేళ (క్షేత్రం_Name == StringRef(KV->మొదటి_2))
            Recurse(KV->రెండవ_2, మార్గం.drop_వెనుక(), Recurse);
          లేకపోతే
            abbreviate(KV->రెండవ_2, JOS);
          JOS.attribute_చివర();
        }
      });
    } లేకపోతే {
      // Current node is an array, path names an element.
      స్థిరరాశి Array *A = V.getAsArray();
      ఒకవేళ (!A || S.సూచీ() >= A->కొలత_2())
        తిరిగి HighlightCurrent();
      JOS.array([&] {
        గుర్తులేని Current = 0;
        కొరకు (స్థిరరాశి స్వయంచాలక &V : *A) {
          ఒకవేళ (Current++ == S.సూచీ())
            Recurse(V, మార్గం.drop_వెనుక(), Recurse);
          లేకపోతే
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_విలువ(R, Error_మార్గం, Print_విలువ);
}

నామాకాశ {
// Simple recursive-descent JSON parser.
తరగతి Parser {
బహిరంగ:
  Parser(StringRef JSON)
      : ప్రారంభం(JSON.begin()), P(JSON.begin()), చివర_2(JSON.చివర()) {}

  తార్కిక checkUTF8() {
    size_t Err_స్థానభ్రంశం;
    ఒకవేళ (isUTF8(StringRef(ప్రారంభం, చివర_2 - ప్రారంభం), &Err_స్థానభ్రంశం))
      తిరిగి నిజం;
    P = ప్రారంభం + Err_స్థానభ్రంశం; // For line/column calculation.
    తిరిగి parseError("Invalid UTF-8 sequence");
  }

  తార్కిక parse_విలువ(విలువ &బహిర్గతం);

  తార్కిక assert_చివర() {
    eatWhitespace();
    ఒకవేళ (P == చివర_2)
      తిరిగి నిజం;
    తిరిగి parseError("Text after end of document");
  }

  Error takeError() {
    ప్రతిజ్ఞా(Err);
    తిరిగి std::move(*Err);
  }

అంతరంగ:
  శూన్య eatWhitespace() {
    అయితే (P != చివర_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  తార్కిక parse_సంఖ్య(అక్షరం మొదటి, విలువ &బహిర్గతం);
  తార్కిక parseString(std::string &బహిర్గతం);
  తార్కిక parseUnicode(std::string &బహిర్గతం);
  తార్కిక parseError(స్థిరరాశి అక్షరం *సందేశం_4); // always returns false

  అక్షరం తదుపరి() { తిరిగి P == చివర_2 ? 0 : *P++; }
  అక్షరం peek() { తిరిగి P == చివర_2 ? 0 : *P; }
  స్థిర తార్కిక is_సంఖ్య(అక్షరం C) {
    తిరిగి C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  స్థిరరాశి అక్షరం *ప్రారంభం, *P, *చివర_2;
};
} // namespace

తార్కిక Parser::parse_విలువ(విలువ &బహిర్గతం) {
  eatWhitespace();
  ఒకవేళ (P == చివర_2)
    తిరిగి parseError("Unexpected EOF");
  ఎంపిక (అక్షరం C = తదుపరి()) {
  // Bare null/true/false are easy - first char identifies them.
  స్థితి 'n':
    బహిర్గతం = శూన్యసూచిక;
    తిరిగి (తదుపరి() == 'u' && తదుపరి() == 'l' && తదుపరి() == 'l') ||
           parseError("Invalid JSON value (null?)");
  స్థితి 't':
    బహిర్గతం = నిజం;
    తిరిగి (తదుపరి() == 'r' && తదుపరి() == 'u' && తదుపరి() == 'e') ||
           parseError("Invalid JSON value (true?)");
  స్థితి 'f':
    బహిర్గతం = అబద్ధం;
    తిరిగి (తదుపరి() == 'a' && తదుపరి() == 'l' && తదుపరి() == 's' && తదుపరి() == 'e') ||
           parseError("Invalid JSON value (false?)");
  స్థితి '"': {
    std::string S;
    ఒకవేళ (parseString(S)) {
      బహిర్గతం = std::move(S);
      తిరిగి నిజం;
    }
    తిరిగి అబద్ధం;
  }
  స్థితి '[': {
    బహిర్గతం = Array{};
    Array &A = *బహిర్గతం.getAsArray();
    eatWhitespace();
    ఒకవేళ (peek() == ']') {
      ++P;
      తిరిగి నిజం;
    }
    కొరకు (;;) {
      A.emplace_వెనుక(శూన్యసూచిక);
      ఒకవేళ (!parse_విలువ(A.వెనుక()))
        తిరిగి అబద్ధం;
      eatWhitespace();
      ఎంపిక (తదుపరి()) {
      స్థితి ',':
        eatWhitespace();
        కొనసాగు;
      స్థితి ']':
        తిరిగి నిజం;
      అప్రమేయ:
        తిరిగి parseError("Expected , or ] after array element");
      }
    }
  }
  స్థితి '{': {
    బహిర్గతం = Object{};
    Object &O = *బహిర్గతం.getAsObject();
    eatWhitespace();
    ఒకవేళ (peek() == '}') {
      ++P;
      తిరిగి నిజం;
    }
    కొరకు (;;) {
      ఒకవేళ (తదుపరి() != '"')
        తిరిగి parseError("Expected object key");
      std::string K;
      ఒకవేళ (!parseString(K))
        తిరిగి అబద్ధం;
      eatWhitespace();
      ఒకవేళ (తదుపరి() != ':')
        తిరిగి parseError("Expected : after object key");
      eatWhitespace();
      ఒకవేళ (!parse_విలువ(O[std::move(K)]))
        తిరిగి అబద్ధం;
      eatWhitespace();
      ఎంపిక (తదుపరి()) {
      స్థితి ',':
        eatWhitespace();
        కొనసాగు;
      స్థితి '}':
        తిరిగి నిజం;
      అప్రమేయ:
        తిరిగి parseError("Expected , or } after object property");
      }
    }
  }
  అప్రమేయ:
    ఒకవేళ (is_సంఖ్య(C))
      తిరిగి parse_సంఖ్య(C, బహిర్గతం);
    తిరిగి parseError("Invalid JSON value");
  }
}

తార్కిక Parser::parse_సంఖ్య(అక్షరం మొదటి, విలువ &బహిర్గతం) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.నెట్టు_వెనుక(మొదటి);
  అయితే (is_సంఖ్య(peek()))
    S.నెట్టు_వెనుక(తదుపరి());
  అక్షరం *చివర_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &చివర_2, 10);
  ఒకవేళ (చివర_2 == S.చివర() && errno != ERANGE) {
    బహిర్గతం = I;
    తిరిగి నిజం;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  ఒకవేళ (మొదటి != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &చివర_2, 10);
    ఒకవేళ (చివర_2 == S.చివర() && errno != ERANGE) {
      బహిర్గతం = UI;
      తిరిగి నిజం;
    }
  }
  // If it's not an integer
  బహిర్గతం = std::strtod(S.c_str(), &చివర_2);
  తిరిగి చివర_2 == S.చివర() || parseError("Invalid JSON value (number?)");
}

తార్కిక Parser::parseString(std::string &బహిర్గతం) {
  // leading quote was already consumed.
  కొరకు (అక్షరం C = తదుపరి(); C != '"'; C = తదుపరి()) {
    ఒకవేళ (LLVM_UNLIKELY(P == చివర_2))
      తిరిగి parseError("Unterminated string");
    ఒకవేళ (LLVM_UNLIKELY((C & 0x1f) == C))
      తిరిగి parseError("Control character in string");
    ఒకవేళ (LLVM_LIKELY(C != '\\')) {
      బహిర్గతం.నెట్టు_వెనుక(C);
      కొనసాగు;
    }
    // Handle escape sequence.
    ఎంపిక (C = తదుపరి()) {
    స్థితి '"':
    స్థితి '\\':
    స్థితి '/':
      బహిర్గతం.నెట్టు_వెనుక(C);
      ఆపు;
    స్థితి 'b':
      బహిర్గతం.నెట్టు_వెనుక('\b');
      ఆపు;
    స్థితి 'f':
      బహిర్గతం.నెట్టు_వెనుక('\f');
      ఆపు;
    స్థితి 'n':
      బహిర్గతం.నెట్టు_వెనుక('\n');
      ఆపు;
    స్థితి 'r':
      బహిర్గతం.నెట్టు_వెనుక('\r');
      ఆపు;
    స్థితి 't':
      బహిర్గతం.నెట్టు_వెనుక('\t');
      ఆపు;
    స్థితి 'u':
      ఒకవేళ (!parseUnicode(బహిర్గతం))
        తిరిగి అబద్ధం;
      ఆపు;
    అప్రమేయ:
      తిరిగి parseError("Invalid escape sequence");
    }
  }
  తిరిగి నిజం;
}

స్థిర శూన్య encodeUtf8(uint32_t Rune, std::string &బహిర్గతం) {
  ఒకవేళ (Rune < 0x80) {
    బహిర్గతం.నెట్టు_వెనుక(Rune & 0x7F);
  } లేకపోతే ఒకవేళ (Rune < 0x800) {
    uint8_t మొదటి_Byte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t రెండవ_Byte = 0x80 | (Rune & 0x3F);
    బహిర్గతం.నెట్టు_వెనుక(మొదటి_Byte);
    బహిర్గతం.నెట్టు_వెనుక(రెండవ_Byte);
  } లేకపోతే ఒకవేళ (Rune < 0x10000) {
    uint8_t మొదటి_Byte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t రెండవ_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t మూడవ_Byte = 0x80 | (Rune & 0x3F);
    బహిర్గతం.నెట్టు_వెనుక(మొదటి_Byte);
    బహిర్గతం.నెట్టు_వెనుక(రెండవ_Byte);
    బహిర్గతం.నెట్టు_వెనుక(మూడవ_Byte);
  } లేకపోతే ఒకవేళ (Rune < 0x110000) {
    uint8_t మొదటి_Byte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t రెండవ_Byte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t మూడవ_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    బహిర్గతం.నెట్టు_వెనుక(మొదటి_Byte);
    బహిర్గతం.నెట్టు_వెనుక(రెండవ_Byte);
    బహిర్గతం.నెట్టు_వెనుక(మూడవ_Byte);
    బహిర్గతం.నెట్టు_వెనుక(FourthByte);
  } లేకపోతే {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
తార్కిక Parser::parseUnicode(std::string &బహిర్గతం) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  స్వయంచాలక Invalid = [&] { బహిర్గతం.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  స్వయంచాలక Parse4Hex = [ఇది](uint16_t &బహిర్గతం) -> తార్కిక {
    బహిర్గతం = 0;
    అక్షరం బైట్లు_2[] = {తదుపరి(), తదుపరి(), తదుపరి(), తదుపరి()};
    కొరకు (గుర్తులేని అక్షరం C : బైట్లు_2) {
      ఒకవేళ (!std::isxdigit(C))
        తిరిగి parseError("Invalid \\u escape sequence");
      బహిర్గతం <<= 4;
      బహిర్గతం |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    తిరిగి నిజం;
  };
  uint16_t మొదటి; // UTF-16 code unit from the first \u escape.
  ఒకవేళ (!Parse4Hex(మొదటి))
    తిరిగి అబద్ధం;

  // We loop to allow proper surrogate-pair error handling.
  అయితే (నిజం) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    ఒకవేళ (LLVM_LIKELY(మొదటి < 0xD800 || మొదటి >= 0xE000)) {
      encodeUtf8(మొదటి, బహిర్గతం);
      తిరిగి నిజం;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    ఒకవేళ (LLVM_UNLIKELY(మొదటి >= 0xDC00)) {
      Invalid();
      తిరిగి నిజం;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    ఒకవేళ (LLVM_UNLIKELY(P + 2 > చివర_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      తిరిగి నిజం;
    }
    P += 2;
    uint16_t రెండవ;
    ఒకవేళ (!Parse4Hex(రెండవ))
      తిరిగి అబద్ధం;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    ఒకవేళ (LLVM_UNLIKELY(రెండవ < 0xDC00 || రెండవ >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      మొదటి = రెండవ; // Second escape still needs to be processed.
      కొనసాగు;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((మొదటి - 0xD800) << 10) | (రెండవ - 0xDC00), బహిర్గతం);
    తిరిగి నిజం;
  }
}

తార్కిక Parser::parseError(స్థిరరాశి అక్షరం *సందేశం_4) {
  పూర్ణాంకం పంక్తి = 1;
  స్థిరరాశి అక్షరం *ప్రారంభం_యొక్క_పంక్తి = ప్రారంభం;
  కొరకు (స్థిరరాశి అక్షరం *X = ప్రారంభం; X < P; ++X) {
    ఒకవేళ (*X == 0x0A) {
      ++పంక్తి;
      ప్రారంభం_యొక్క_పంక్తి = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(సందేశం_4, పంక్తి, P - ప్రారంభం_యొక్క_పంక్తి, P - ప్రారంభం));
  తిరిగి అబద్ధం;
}

Expected<విలువ> parse(StringRef JSON) {
  Parser P(JSON);
  విలువ E = శూన్యసూచిక;
  ఒకవేళ (P.checkUTF8())
    ఒకవేళ (P.parse_విలువ(E))
      ఒకవేళ (P.assert_చివర())
        తిరిగి std::move(E);
  తిరిగి P.takeError();
}

అక్షరం ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
శూన్య ParseError::సంవర్గమానం(llvm::raw_ostream &OS) స్థిరరాశి {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", పంక్తి, స్తంభం, స్థానభ్రంశం, సందేశం_4);
}

తార్కిక isUTF8(llvm::StringRef S, size_t *Err_స్థానభ్రంశం) {
  // Fast-path for ASCII, which is valid UTF-8.
  ఒకవేళ (LLVM_LIKELY(isASCII(S)))
    తిరిగి నిజం;

  స్థిరరాశి UTF8 *దత్తాంశం_2 = reinterpret_cast<స్థిరరాశి UTF8 *>(S.దత్తాంశం()), *Rest = దత్తాంశం_2;
  ఒకవేళ (LLVM_LIKELY(isLegalUTF8String(&Rest, దత్తాంశం_2 + S.కొలత_2())))
    తిరిగి నిజం;

  ఒకవేళ (Err_స్థానభ్రంశం)
    *Err_స్థానభ్రంశం = Rest - దత్తాంశం_2;
  తిరిగి అబద్ధం;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::సదిశ<UTF32> Codepoints(S.కొలత_2()); // 1 codepoint per byte suffices.
  స్థిరరాశి UTF8 *In8 = reinterpret_cast<స్థిరరాశి UTF8 *>(S.దత్తాంశం());
  UTF32 *Out32 = Codepoints.దత్తాంశం();
  ConvertUTF8toUTF32(&In8, In8 + S.కొలత_2(), &Out32, Out32 + Codepoints.కొలత_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.దత్తాంశం());
  std::string Res(4 * Codepoints.కొలత_2(), 0); // 4 bytes per codepoint suffice
  స్థిరరాశి UTF32 *In32 = Codepoints.దత్తాంశం();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.కొలత_2(), &Out8, Out8 + Res.కొలత_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<అక్షరం *>(Out8) - Res.దత్తాంశం());
  తిరిగి Res;
}

స్థిర శూన్య quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  కొరకు (గుర్తులేని అక్షరం C : S) {
    ఒకవేళ (C == 0x22 || C == 0x5C)
      OS << '\\';
    ఒకవేళ (C >= 0x20) {
      OS << C;
      కొనసాగు;
    }
    OS << '\\';
    ఎంపిక (C) {
    // A few characters are common enough to make short escapes worthwhile.
    స్థితి '\t':
      OS << 't';
      ఆపు;
    స్థితి '\n':
      OS << 'n';
      ఆపు;
    స్థితి '\r':
      OS << 'r';
      ఆపు;
    అప్రమేయ:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::క్రింది_2, 4);
      ఆపు;
    }
  }
  OS << '\"';
}

శూన్య llvm::json::OStream::విలువ_2(స్థిరరాశి విలువ &V) {
  ఎంపిక (V.రకం_2()) {
  స్థితి విలువ::Null:
    విలువ_Begin();
    OS << "null";
    తిరిగి;
  స్థితి విలువ::Boolean:
    విలువ_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    తిరిగి;
  స్థితి విలువ::సంఖ్య:
    విలువ_Begin();
    ఒకవేళ (V.Type == విలువ::T_Integer)
      OS << *V.getAsInteger();
    లేకపోతే ఒకవేళ (V.Type == విలువ::T_UINT64)
      OS << *V.getAsUINT64();
    లేకపోతే
      OS << format("%.*g", std::numeric_limits<ద్విగుణ>::max_digits10,
                   *V.get_As_సంఖ్య());
    తిరిగి;
  స్థితి విలువ::String:
    విలువ_Begin();
    quote(OS, *V.getAsString());
    తిరిగి;
  స్థితి విలువ::Array:
    తిరిగి array([&] {
      కొరకు (స్థిరరాశి విలువ &E : *V.getAsArray())
        విలువ_2(E);
    });
  స్థితి విలువ::Object:
    తిరిగి object([&] {
      కొరకు (స్థిరరాశి Object::విలువ_type *E : sortedElements(*V.getAsObject()))
        attribute(E->మొదటి_2, E->రెండవ_2);
    });
  }
}

శూన్య llvm::json::OStream::విలువ_Begin() {
  ప్రతిజ్ఞా(దొంతర.వెనుక().Ctx != Object && "Only attributes allowed here");
  ఒకవేళ (దొంతర.వెనుక().Has_విలువ) {
    ప్రతిజ్ఞా(దొంతర.వెనుక().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  ఒకవేళ (దొంతర.వెనుక().Ctx == Array)
    newline();
  flushComment();
  దొంతర.వెనుక().Has_విలువ = నిజం;
}

శూన్య OStream::comment(llvm::StringRef Comment) {
  ప్రతిజ్ఞా(PendingComment.ఖాళీలు() && "Only one comment per value!");
  PendingComment = Comment;
}

శూన్య OStream::flushComment() {
  ఒకవేళ (PendingComment.ఖాళీలు())
    తిరిగి;
  OS << (Indent_కొలత ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  అయితే (!PendingComment.ఖాళీలు()) {
    స్వయంచాలక Pos = PendingComment.కనుగొను("*/");
    ఒకవేళ (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } లేకపోతే {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_కొలత ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  ఒకవేళ (దొంతర.కొలత_2() > 1 && దొంతర.వెనుక().Ctx == Singleton) {
    ఒకవేళ (Indent_కొలత)
      OS << ' ';
  } లేకపోతే {
    newline();
  }
}

శూన్య llvm::json::OStream::newline() {
  ఒకవేళ (Indent_కొలత) {
    OS.లిఖ('\n');
    OS.indent(Indent);
  }
}

శూన్య llvm::json::OStream::arrayBegin() {
  విలువ_Begin();
  దొంతర.emplace_వెనుక();
  దొంతర.వెనుక().Ctx = Array;
  Indent += Indent_కొలత;
  OS << '[';
}

శూన్య llvm::json::OStream::array_చివర() {
  ప్రతిజ్ఞా(దొంతర.వెనుక().Ctx == Array);
  Indent -= Indent_కొలత;
  ఒకవేళ (దొంతర.వెనుక().Has_విలువ)
    newline();
  OS << ']';
  ప్రతిజ్ఞా(PendingComment.ఖాళీలు());
  దొంతర.తీయి_వెనుక();
  ప్రతిజ్ఞా(!దొంతర.ఖాళీలు());
}

శూన్య llvm::json::OStream::objectBegin() {
  విలువ_Begin();
  దొంతర.emplace_వెనుక();
  దొంతర.వెనుక().Ctx = Object;
  Indent += Indent_కొలత;
  OS << '{';
}

శూన్య llvm::json::OStream::object_చివర() {
  ప్రతిజ్ఞా(దొంతర.వెనుక().Ctx == Object);
  Indent -= Indent_కొలత;
  ఒకవేళ (దొంతర.వెనుక().Has_విలువ)
    newline();
  OS << '}';
  ప్రతిజ్ఞా(PendingComment.ఖాళీలు());
  దొంతర.తీయి_వెనుక();
  ప్రతిజ్ఞా(!దొంతర.ఖాళీలు());
}

శూన్య llvm::json::OStream::attributeBegin(llvm::StringRef కీలకం) {
  ప్రతిజ్ఞా(దొంతర.వెనుక().Ctx == Object);
  ఒకవేళ (దొంతర.వెనుక().Has_విలువ)
    OS << ',';
  newline();
  flushComment();
  దొంతర.వెనుక().Has_విలువ = నిజం;
  దొంతర.emplace_వెనుక();
  దొంతర.వెనుక().Ctx = Singleton;
  ఒకవేళ (LLVM_LIKELY(isUTF8(కీలకం))) {
    quote(OS, కీలకం);
  } లేకపోతే {
    ప్రతిజ్ఞా(అబద్ధం && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(కీలకం));
  }
  OS.లిఖ(':');
  ఒకవేళ (Indent_కొలత)
    OS.లిఖ(' ');
}

శూన్య llvm::json::OStream::attribute_చివర() {
  ప్రతిజ్ఞా(దొంతర.వెనుక().Ctx == Singleton);
  ప్రతిజ్ఞా(దొంతర.వెనుక().Has_విలువ && "Attribute must have a value");
  ప్రతిజ్ఞా(PendingComment.ఖాళీలు());
  దొంతర.తీయి_వెనుక();
  ప్రతిజ్ఞా(దొంతర.వెనుక().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_విలువ_Begin() {
  విలువ_Begin();
  దొంతర.emplace_వెనుక();
  దొంతర.వెనుక().Ctx = Raw_విలువ;
  తిరిగి OS;
}

శూన్య llvm::json::OStream::raw_విలువ_చివర() {
  ప్రతిజ్ఞా(దొంతర.వెనుక().Ctx == Raw_విలువ);
  దొంతర.తీయి_వెనుక();
}

} // namespace json
} // namespace llvm

శూన్య llvm::format_provider<llvm::json::విలువ>::format(
    స్థిరరాశి llvm::json::విలువ &E, raw_ostream &OS, StringRef Options) {
  గుర్తులేని IndentAmount = 0;
  ఒకవేళ (!Options.ఖాళీలు() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).విలువ_2(E);
}

