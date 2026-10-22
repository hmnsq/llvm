//=== JSON.cpp - JSON value, parsing and serialization - C++ -----------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/JSON.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertUTF.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/NativeFormatting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cctype>
#ਅਨ੍ਤਰ੍ਭਾਵ <cerrno>
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ json {

ਮੁੱਲ &Object::ਸੰਚਾਲਕ[](ਅਚਲ Object_ਕੁੰਜੀ &K) {
  ਵਾਪਸ try_emplace(K, ਖਾਲੀ_ਸੂਚਕ).ਪਹਿਲਾ_2->get_ਦੂਜਾ();
}
ਮੁੱਲ &Object::ਸੰਚਾਲਕ[](Object_ਕੁੰਜੀ &&K) {
  ਵਾਪਸ try_emplace(std::move(K), ਖਾਲੀ_ਸੂਚਕ).ਪਹਿਲਾ_2->get_ਦੂਜਾ();
}
ਮੁੱਲ *Object::get(StringRef K) {
  ਸਵੈਚਾਲਿਤ I = ਲੱਭੋ(K);
  ਜੇ (I == ਸਿਰਾ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  ਵਾਪਸ &I->ਦੂਜਾ_2;
}
ਅਚਲ ਮੁੱਲ *Object::get(StringRef K) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ I = ਲੱਭੋ(K);
  ਜੇ (I == ਸਿਰਾ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  ਵਾਪਸ &I->ਦੂਜਾ_2;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->getAsNull();
  ਵਾਪਸ std::nullopt;
}
std::optional<ਤਾਰਕਿਕ> Object::getBoolean(StringRef K) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->getAsBoolean();
  ਵਾਪਸ std::nullopt;
}
std::optional<ਦੁੱਗਣਾ> Object::get_ਸੰਖਿਆ(StringRef K) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->get_As_ਸੰਖਿਆ();
  ਵਾਪਸ std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->getAsInteger();
  ਵਾਪਸ std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->getAsString();
  ਵਾਪਸ std::nullopt;
}
ਅਚਲ json::Object *Object::getObject(StringRef K) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->getAsObject();
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}
json::Object *Object::getObject(StringRef K) {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->getAsObject();
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}
ਅਚਲ json::Array *Object::getArray(StringRef K) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->getAsArray();
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}
json::Array *Object::getArray(StringRef K) {
  ਜੇ (ਸਵੈਚਾਲਿਤ *V = get(K))
    ਵਾਪਸ V->getAsArray();
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}
ਤਾਰਕਿਕ ਸੰਚਾਲਕ==(ਅਚਲ Object &LHS, ਅਚਲ Object &RHS) { ਵਾਪਸ LHS.M == RHS.M; }

Array::Array(std::initializer_list<ਮੁੱਲ> Elements) {
  V.reserve(Elements.ਮਾਪ_2());
  ਲਈ (ਅਚਲ ਮੁੱਲ &V : Elements) {
    emplace_ਪਿੱਠ(ਖਾਲੀ_ਸੂਚਕ);
    ਪਿੱਠ().moveFrom(std::move(V));
  }
}

ਮੁੱਲ::ਮੁੱਲ(std::initializer_list<ਮੁੱਲ> Elements)
    : ਮੁੱਲ(json::Array(Elements)) {}

ਖਾਲੀ ਮੁੱਲ::copyFrom(ਅਚਲ ਮੁੱਲ &M) {
  Type = M.Type;
  ਚੋਣ (Type) {
  ਹਾਲਤ T_Null:
  ਹਾਲਤ T_Boolean:
  ਹਾਲਤ T_Double:
  ਹਾਲਤ T_Integer:
  ਹਾਲਤ T_UINT64:
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&Union, &M.Union, ਆਕਾਰ(Union));
    ਰੋਕੋ;
  ਹਾਲਤ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ਰੋਕੋ;
  ਹਾਲਤ T_String:
    create<std::string>(M.as<std::string>());
    ਰੋਕੋ;
  ਹਾਲਤ T_Object:
    create<json::Object>(M.as<json::Object>());
    ਰੋਕੋ;
  ਹਾਲਤ T_Array:
    create<json::Array>(M.as<json::Array>());
    ਰੋਕੋ;
  }
}

ਖਾਲੀ ਮੁੱਲ::moveFrom(ਅਚਲ ਮੁੱਲ &&M) {
  Type = M.Type;
  ਚੋਣ (Type) {
  ਹਾਲਤ T_Null:
  ਹਾਲਤ T_Boolean:
  ਹਾਲਤ T_Double:
  ਹਾਲਤ T_Integer:
  ਹਾਲਤ T_UINT64:
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&Union, &M.Union, ਆਕਾਰ(Union));
    ਰੋਕੋ;
  ਹਾਲਤ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ਰੋਕੋ;
  ਹਾਲਤ T_String:
    create<std::string>(std::move(M.as<std::string>()));
    ਰੋਕੋ;
  ਹਾਲਤ T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    ਰੋਕੋ;
  ਹਾਲਤ T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    ਰੋਕੋ;
  }
  const_cast<ਮੁੱਲ &>(M).destroy();
  M.Type = T_Null;
}

ਖਾਲੀ ਮੁੱਲ::destroy() {
  ਚੋਣ (Type) {
  ਹਾਲਤ T_Null:
  ਹਾਲਤ T_Boolean:
  ਹਾਲਤ T_Double:
  ਹਾਲਤ T_Integer:
  ਹਾਲਤ T_UINT64:
    ਰੋਕੋ;
  ਹਾਲਤ T_StringRef:
    as<StringRef>().~StringRef();
    ਰੋਕੋ;
  ਹਾਲਤ T_String:
    as<std::string>().~basic_string();
    ਰੋਕੋ;
  ਹਾਲਤ T_Object:
    as<json::Object>().~Object();
    ਰੋਕੋ;
  ਹਾਲਤ T_Array:
    as<json::Array>().~Array();
    ਰੋਕੋ;
  }
}

ਖਾਲੀ ਮੁੱਲ::print(llvm::raw_ostream &OS) ਅਚਲ { OS << *ਇਹ; }

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ ਮੁੱਲ::dump() ਅਚਲ {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#ਜੇ_ਅੰਤ

ਤਾਰਕਿਕ ਸੰਚਾਲਕ==(ਅਚਲ ਮੁੱਲ &L, ਅਚਲ ਮੁੱਲ &R) {
  ਜੇ (L.ਕਿਸਮ_2() != R.ਕਿਸਮ_2())
    ਵਾਪਸ ਝੂਠ;
  ਚੋਣ (L.ਕਿਸਮ_2()) {
  ਹਾਲਤ ਮੁੱਲ::Null:
    ਵਾਪਸ *L.getAsNull() == *R.getAsNull();
  ਹਾਲਤ ਮੁੱਲ::Boolean:
    ਵਾਪਸ *L.getAsBoolean() == *R.getAsBoolean();
  ਹਾਲਤ ਮੁੱਲ::ਸੰਖਿਆ:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    ਜੇ (L.Type == ਮੁੱਲ::T_Integer || R.Type == ਮੁੱਲ::T_Integer)
      ਵਾਪਸ L.getAsInteger() == R.getAsInteger();
    ਵਾਪਸ *L.get_As_ਸੰਖਿਆ() == *R.get_As_ਸੰਖਿਆ();
  ਹਾਲਤ ਮੁੱਲ::String:
    ਵਾਪਸ *L.getAsString() == *R.getAsString();
  ਹਾਲਤ ਮੁੱਲ::Array:
    ਵਾਪਸ *L.getAsArray() == *R.getAsArray();
  ਹਾਲਤ ਮੁੱਲ::Object:
    ਵਾਪਸ *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

ਖਾਲੀ ਰਸਤਾ::report(llvm::StringLiteral ਸੁਨੇਹਾ_4) {
  // Walk up to the root context, and count the number of segments.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 = 0;
  ਅਚਲ ਰਸਤਾ *P;
  ਲਈ (P = ਇਹ; P->ਮਾਪਾ_2 != ਖਾਲੀ_ਸੂਚਕ; P = P->ਮਾਪਾ_2)
    ++ਗਿਣਤੀ_3;
  ਰਸਤਾ::ਜੜ੍ਹ_2 *R = P->Seg.ਜੜ੍ਹ_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_ਸੁਨੇਹਾ_2 = ਸੁਨੇਹਾ_4;
  R->Error_ਰਸਤਾ.resize(ਗਿਣਤੀ_3);
  ਸਵੈਚਾਲਿਤ It = R->Error_ਰਸਤਾ.begin();
  ਲਈ (P = ਇਹ; P->ਮਾਪਾ_2 != ਖਾਲੀ_ਸੂਚਕ; P = P->ਮਾਪਾ_2)
    *It++ = P->Seg;
}

Error ਰਸਤਾ::ਜੜ੍ਹ_2::getError() ਅਚਲ {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_ਸੁਨੇਹਾ_2.ਖਾਲੀ_ਥਾਵਾਂ() ? "invalid JSON contents" : Error_ਸੁਨੇਹਾ_2);
  ਜੇ (Error_ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਜੇ (!Name.ਖਾਲੀ_ਥਾਵਾਂ())
      OS << " when parsing " << Name;
  } ਨਹੀਂਤਾਂ {
    OS << " at " << (Name.ਖਾਲੀ_ਥਾਵਾਂ() ? "(root)" : Name);
    ਲਈ (ਅਚਲ ਰਸਤਾ::Segment &S : llvm::ਉਲਟ(Error_ਰਸਤਾ)) {
      ਜੇ (S.is_ਖੇਤਰ())
        OS << '.' << S.ਖੇਤਰ();
      ਨਹੀਂਤਾਂ
        OS << '[' << S.ਸੂਚਕਾਂਕ() << ']';
    }
  }
  ਵਾਪਸ createStringError(llvm::inconvertible_Error_ਕੋਡ(), S);
}

std::ਸਦਿਸ਼<ਅਚਲ Object::ਮੁੱਲ_type *> sortedElements(ਅਚਲ Object &O) {
  std::ਸਦਿਸ਼<ਅਚਲ Object::ਮੁੱਲ_type *> Elements;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &E : O)
    Elements.ਧੱਕੋ_ਪਿੱਠ(&E);
  llvm::ਛਾਂਟੋ(Elements,
             [](ਅਚਲ Object::ਮੁੱਲ_type *L, ਅਚਲ Object::ਮੁੱਲ_type *R) {
               ਵਾਪਸ L->ਪਹਿਲਾ_2 < R->ਪਹਿਲਾ_2;
             });
  ਵਾਪਸ Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ਸਥਿਰ ਖਾਲੀ abbreviate(ਅਚਲ ਮੁੱਲ &V, OStream &JOS) {
  ਚੋਣ (V.ਕਿਸਮ_2()) {
  ਹਾਲਤ ਮੁੱਲ::Array:
    JOS.raw_ਮੁੱਲ(V.getAsArray()->ਖਾਲੀ_ਥਾਵਾਂ() ? "[]" : "[ ... ]");
    ਰੋਕੋ;
  ਹਾਲਤ ਮੁੱਲ::Object:
    JOS.raw_ਮੁੱਲ(V.getAsObject()->ਖਾਲੀ_ਥਾਵਾਂ() ? "{}" : "{ ... }");
    ਰੋਕੋ;
  ਹਾਲਤ ਮੁੱਲ::String: {
    llvm::StringRef S = *V.getAsString();
    ਜੇ (S.ਮਾਪ_2() < 40) {
      JOS.ਮੁੱਲ_2(V);
    } ਨਹੀਂਤਾਂ {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.ਮੁੱਲ_2(Truncated);
    }
    ਰੋਕੋ;
  }
  ਮੂਲ:
    JOS.ਮੁੱਲ_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ਸਥਿਰ ਖਾਲੀ abbreviateChildren(ਅਚਲ ਮੁੱਲ &V, OStream &JOS) {
  ਚੋਣ (V.ਕਿਸਮ_2()) {
  ਹਾਲਤ ਮੁੱਲ::Array:
    JOS.array([&] {
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    ਰੋਕੋ;
  ਹਾਲਤ ਮੁੱਲ::Object:
    JOS.object([&] {
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->ਪਹਿਲਾ_2);
        abbreviate(KV->ਦੂਜਾ_2, JOS);
        JOS.attribute_ਸਿਰਾ();
      }
    });
    ਰੋਕੋ;
  ਮੂਲ:
    JOS.ਮੁੱਲ_2(V);
  }
}

ਖਾਲੀ ਰਸਤਾ::ਜੜ੍ਹ_2::printErrorContext(ਅਚਲ ਮੁੱਲ &R, raw_ostream &OS) ਅਚਲ {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  ਸਵੈਚਾਲਿਤ Print_ਮੁੱਲ = [&](ਅਚਲ ਮੁੱਲ &V, ArrayRef<Segment> ਰਸਤਾ, ਸਵੈਚਾਲਿਤ &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    ਸਵੈਚਾਲਿਤ HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_ਸੁਨੇਹਾ_2.ਅੰਕੜੇ(), Error_ਸੁਨੇਹਾ_2.ਮਾਪ_2());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    ਜੇ (ਰਸਤਾ.ਖਾਲੀ_ਥਾਵਾਂ()) // We reached our target.
      ਵਾਪਸ HighlightCurrent();
    ਅਚਲ Segment &S = ਰਸਤਾ.ਪਿੱਠ(); // Path is in reverse order.
    ਜੇ (S.is_ਖੇਤਰ()) {
      // Current node is an object, path names a field.
      llvm::StringRef ਖੇਤਰ_Name = S.ਖੇਤਰ();
      ਅਚਲ Object *O = V.getAsObject();
      ਜੇ (!O || !O->get(ਖੇਤਰ_Name))
        ਵਾਪਸ HighlightCurrent();
      JOS.object([&] {
        ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->ਪਹਿਲਾ_2);
          ਜੇ (ਖੇਤਰ_Name == StringRef(KV->ਪਹਿਲਾ_2))
            Recurse(KV->ਦੂਜਾ_2, ਰਸਤਾ.drop_ਪਿੱਠ(), Recurse);
          ਨਹੀਂਤਾਂ
            abbreviate(KV->ਦੂਜਾ_2, JOS);
          JOS.attribute_ਸਿਰਾ();
        }
      });
    } ਨਹੀਂਤਾਂ {
      // Current node is an array, path names an element.
      ਅਚਲ Array *A = V.getAsArray();
      ਜੇ (!A || S.ਸੂਚਕਾਂਕ() >= A->ਮਾਪ_2())
        ਵਾਪਸ HighlightCurrent();
      JOS.array([&] {
        ਬਿਨਾਨਿਸ਼ਾਨੀ Current = 0;
        ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &V : *A) {
          ਜੇ (Current++ == S.ਸੂਚਕਾਂਕ())
            Recurse(V, ਰਸਤਾ.drop_ਪਿੱਠ(), Recurse);
          ਨਹੀਂਤਾਂ
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_ਮੁੱਲ(R, Error_ਰਸਤਾ, Print_ਮੁੱਲ);
}

ਨਾਮਾਕਾਸ਼ {
// Simple recursive-descent JSON parser.
ਵਰਗ Parser {
ਜਨਤਕ:
  Parser(StringRef JSON)
      : ਸ਼ੁਰੂ(JSON.begin()), P(JSON.begin()), ਸਿਰਾ_2(JSON.ਸਿਰਾ()) {}

  ਤਾਰਕਿਕ checkUTF8() {
    size_t Err_ਵਿੱਥ;
    ਜੇ (isUTF8(StringRef(ਸ਼ੁਰੂ, ਸਿਰਾ_2 - ਸ਼ੁਰੂ), &Err_ਵਿੱਥ))
      ਵਾਪਸ ਸੱਚ;
    P = ਸ਼ੁਰੂ + Err_ਵਿੱਥ; // For line/column calculation.
    ਵਾਪਸ parseError("Invalid UTF-8 sequence");
  }

  ਤਾਰਕਿਕ parse_ਮੁੱਲ(ਮੁੱਲ &ਬਾਹਰਲਾ);

  ਤਾਰਕਿਕ assert_ਸਿਰਾ() {
    eatWhitespace();
    ਜੇ (P == ਸਿਰਾ_2)
      ਵਾਪਸ ਸੱਚ;
    ਵਾਪਸ parseError("Text after end of document");
  }

  Error takeError() {
    ਪ੍ਰਤਿਜ੍ਞਾ(Err);
    ਵਾਪਸ std::move(*Err);
  }

ਨਿਜੀ:
  ਖਾਲੀ eatWhitespace() {
    ਜਦੋਂ (P != ਸਿਰਾ_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  ਤਾਰਕਿਕ parse_ਸੰਖਿਆ(ਅੱਖਰ ਪਹਿਲਾ, ਮੁੱਲ &ਬਾਹਰਲਾ);
  ਤਾਰਕਿਕ parseString(std::string &ਬਾਹਰਲਾ);
  ਤਾਰਕਿਕ parseUnicode(std::string &ਬਾਹਰਲਾ);
  ਤਾਰਕਿਕ parseError(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_4); // always returns false

  ਅੱਖਰ ਅਗਲਾ() { ਵਾਪਸ P == ਸਿਰਾ_2 ? 0 : *P++; }
  ਅੱਖਰ peek() { ਵਾਪਸ P == ਸਿਰਾ_2 ? 0 : *P; }
  ਸਥਿਰ ਤਾਰਕਿਕ is_ਸੰਖਿਆ(ਅੱਖਰ C) {
    ਵਾਪਸ C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ, *P, *ਸਿਰਾ_2;
};
} // namespace

ਤਾਰਕਿਕ Parser::parse_ਮੁੱਲ(ਮੁੱਲ &ਬਾਹਰਲਾ) {
  eatWhitespace();
  ਜੇ (P == ਸਿਰਾ_2)
    ਵਾਪਸ parseError("Unexpected EOF");
  ਚੋਣ (ਅੱਖਰ C = ਅਗਲਾ()) {
  // Bare null/true/false are easy - first char identifies them.
  ਹਾਲਤ 'n':
    ਬਾਹਰਲਾ = ਖਾਲੀ_ਸੂਚਕ;
    ਵਾਪਸ (ਅਗਲਾ() == 'u' && ਅਗਲਾ() == 'l' && ਅਗਲਾ() == 'l') ||
           parseError("Invalid JSON value (null?)");
  ਹਾਲਤ 't':
    ਬਾਹਰਲਾ = ਸੱਚ;
    ਵਾਪਸ (ਅਗਲਾ() == 'r' && ਅਗਲਾ() == 'u' && ਅਗਲਾ() == 'e') ||
           parseError("Invalid JSON value (true?)");
  ਹਾਲਤ 'f':
    ਬਾਹਰਲਾ = ਝੂਠ;
    ਵਾਪਸ (ਅਗਲਾ() == 'a' && ਅਗਲਾ() == 'l' && ਅਗਲਾ() == 's' && ਅਗਲਾ() == 'e') ||
           parseError("Invalid JSON value (false?)");
  ਹਾਲਤ '"': {
    std::string S;
    ਜੇ (parseString(S)) {
      ਬਾਹਰਲਾ = std::move(S);
      ਵਾਪਸ ਸੱਚ;
    }
    ਵਾਪਸ ਝੂਠ;
  }
  ਹਾਲਤ '[': {
    ਬਾਹਰਲਾ = Array{};
    Array &A = *ਬਾਹਰਲਾ.getAsArray();
    eatWhitespace();
    ਜੇ (peek() == ']') {
      ++P;
      ਵਾਪਸ ਸੱਚ;
    }
    ਲਈ (;;) {
      A.emplace_ਪਿੱਠ(ਖਾਲੀ_ਸੂਚਕ);
      ਜੇ (!parse_ਮੁੱਲ(A.ਪਿੱਠ()))
        ਵਾਪਸ ਝੂਠ;
      eatWhitespace();
      ਚੋਣ (ਅਗਲਾ()) {
      ਹਾਲਤ ',':
        eatWhitespace();
        ਜਾਰੀ;
      ਹਾਲਤ ']':
        ਵਾਪਸ ਸੱਚ;
      ਮੂਲ:
        ਵਾਪਸ parseError("Expected , or ] after array element");
      }
    }
  }
  ਹਾਲਤ '{': {
    ਬਾਹਰਲਾ = Object{};
    Object &O = *ਬਾਹਰਲਾ.getAsObject();
    eatWhitespace();
    ਜੇ (peek() == '}') {
      ++P;
      ਵਾਪਸ ਸੱਚ;
    }
    ਲਈ (;;) {
      ਜੇ (ਅਗਲਾ() != '"')
        ਵਾਪਸ parseError("Expected object key");
      std::string K;
      ਜੇ (!parseString(K))
        ਵਾਪਸ ਝੂਠ;
      eatWhitespace();
      ਜੇ (ਅਗਲਾ() != ':')
        ਵਾਪਸ parseError("Expected : after object key");
      eatWhitespace();
      ਜੇ (!parse_ਮੁੱਲ(O[std::move(K)]))
        ਵਾਪਸ ਝੂਠ;
      eatWhitespace();
      ਚੋਣ (ਅਗਲਾ()) {
      ਹਾਲਤ ',':
        eatWhitespace();
        ਜਾਰੀ;
      ਹਾਲਤ '}':
        ਵਾਪਸ ਸੱਚ;
      ਮੂਲ:
        ਵਾਪਸ parseError("Expected , or } after object property");
      }
    }
  }
  ਮੂਲ:
    ਜੇ (is_ਸੰਖਿਆ(C))
      ਵਾਪਸ parse_ਸੰਖਿਆ(C, ਬਾਹਰਲਾ);
    ਵਾਪਸ parseError("Invalid JSON value");
  }
}

ਤਾਰਕਿਕ Parser::parse_ਸੰਖਿਆ(ਅੱਖਰ ਪਹਿਲਾ, ਮੁੱਲ &ਬਾਹਰਲਾ) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ਧੱਕੋ_ਪਿੱਠ(ਪਹਿਲਾ);
  ਜਦੋਂ (is_ਸੰਖਿਆ(peek()))
    S.ਧੱਕੋ_ਪਿੱਠ(ਅਗਲਾ());
  ਅੱਖਰ *ਸਿਰਾ_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &ਸਿਰਾ_2, 10);
  ਜੇ (ਸਿਰਾ_2 == S.ਸਿਰਾ() && errno != ERANGE) {
    ਬਾਹਰਲਾ = I;
    ਵਾਪਸ ਸੱਚ;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  ਜੇ (ਪਹਿਲਾ != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &ਸਿਰਾ_2, 10);
    ਜੇ (ਸਿਰਾ_2 == S.ਸਿਰਾ() && errno != ERANGE) {
      ਬਾਹਰਲਾ = UI;
      ਵਾਪਸ ਸੱਚ;
    }
  }
  // If it's not an integer
  ਬਾਹਰਲਾ = std::strtod(S.c_str(), &ਸਿਰਾ_2);
  ਵਾਪਸ ਸਿਰਾ_2 == S.ਸਿਰਾ() || parseError("Invalid JSON value (number?)");
}

ਤਾਰਕਿਕ Parser::parseString(std::string &ਬਾਹਰਲਾ) {
  // leading quote was already consumed.
  ਲਈ (ਅੱਖਰ C = ਅਗਲਾ(); C != '"'; C = ਅਗਲਾ()) {
    ਜੇ (LLVM_UNLIKELY(P == ਸਿਰਾ_2))
      ਵਾਪਸ parseError("Unterminated string");
    ਜੇ (LLVM_UNLIKELY((C & 0x1f) == C))
      ਵਾਪਸ parseError("Control character in string");
    ਜੇ (LLVM_LIKELY(C != '\\')) {
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(C);
      ਜਾਰੀ;
    }
    // Handle escape sequence.
    ਚੋਣ (C = ਅਗਲਾ()) {
    ਹਾਲਤ '"':
    ਹਾਲਤ '\\':
    ਹਾਲਤ '/':
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(C);
      ਰੋਕੋ;
    ਹਾਲਤ 'b':
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ('\b');
      ਰੋਕੋ;
    ਹਾਲਤ 'f':
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ('\f');
      ਰੋਕੋ;
    ਹਾਲਤ 'n':
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ('\n');
      ਰੋਕੋ;
    ਹਾਲਤ 'r':
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ('\r');
      ਰੋਕੋ;
    ਹਾਲਤ 't':
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ('\t');
      ਰੋਕੋ;
    ਹਾਲਤ 'u':
      ਜੇ (!parseUnicode(ਬਾਹਰਲਾ))
        ਵਾਪਸ ਝੂਠ;
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ parseError("Invalid escape sequence");
    }
  }
  ਵਾਪਸ ਸੱਚ;
}

ਸਥਿਰ ਖਾਲੀ encodeUtf8(uint32_t Rune, std::string &ਬਾਹਰਲਾ) {
  ਜੇ (Rune < 0x80) {
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(Rune & 0x7F);
  } ਨਹੀਂਤਾਂ ਜੇ (Rune < 0x800) {
    uint8_t ਪਹਿਲਾ_Byte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t ਦੂਜਾ_Byte = 0x80 | (Rune & 0x3F);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਪਹਿਲਾ_Byte);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਦੂਜਾ_Byte);
  } ਨਹੀਂਤਾਂ ਜੇ (Rune < 0x10000) {
    uint8_t ਪਹਿਲਾ_Byte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t ਦੂਜਾ_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ਤੀਜਾ_Byte = 0x80 | (Rune & 0x3F);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਪਹਿਲਾ_Byte);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਦੂਜਾ_Byte);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਤੀਜਾ_Byte);
  } ਨਹੀਂਤਾਂ ਜੇ (Rune < 0x110000) {
    uint8_t ਪਹਿਲਾ_Byte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t ਦੂਜਾ_Byte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ਤੀਜਾ_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਪਹਿਲਾ_Byte);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਦੂਜਾ_Byte);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਤੀਜਾ_Byte);
    ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(FourthByte);
  } ਨਹੀਂਤਾਂ {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
ਤਾਰਕਿਕ Parser::parseUnicode(std::string &ਬਾਹਰਲਾ) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  ਸਵੈਚਾਲਿਤ Invalid = [&] { ਬਾਹਰਲਾ.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  ਸਵੈਚਾਲਿਤ Parse4Hex = [ਇਹ](uint16_t &ਬਾਹਰਲਾ) -> ਤਾਰਕਿਕ {
    ਬਾਹਰਲਾ = 0;
    ਅੱਖਰ ਬਾਈਟਾਂ_2[] = {ਅਗਲਾ(), ਅਗਲਾ(), ਅਗਲਾ(), ਅਗਲਾ()};
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ C : ਬਾਈਟਾਂ_2) {
      ਜੇ (!std::isxdigit(C))
        ਵਾਪਸ parseError("Invalid \\u escape sequence");
      ਬਾਹਰਲਾ <<= 4;
      ਬਾਹਰਲਾ |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    ਵਾਪਸ ਸੱਚ;
  };
  uint16_t ਪਹਿਲਾ; // UTF-16 code unit from the first \u escape.
  ਜੇ (!Parse4Hex(ਪਹਿਲਾ))
    ਵਾਪਸ ਝੂਠ;

  // We loop to allow proper surrogate-pair error handling.
  ਜਦੋਂ (ਸੱਚ) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    ਜੇ (LLVM_LIKELY(ਪਹਿਲਾ < 0xD800 || ਪਹਿਲਾ >= 0xE000)) {
      encodeUtf8(ਪਹਿਲਾ, ਬਾਹਰਲਾ);
      ਵਾਪਸ ਸੱਚ;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    ਜੇ (LLVM_UNLIKELY(ਪਹਿਲਾ >= 0xDC00)) {
      Invalid();
      ਵਾਪਸ ਸੱਚ;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    ਜੇ (LLVM_UNLIKELY(P + 2 > ਸਿਰਾ_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      ਵਾਪਸ ਸੱਚ;
    }
    P += 2;
    uint16_t ਦੂਜਾ;
    ਜੇ (!Parse4Hex(ਦੂਜਾ))
      ਵਾਪਸ ਝੂਠ;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    ਜੇ (LLVM_UNLIKELY(ਦੂਜਾ < 0xDC00 || ਦੂਜਾ >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      ਪਹਿਲਾ = ਦੂਜਾ; // Second escape still needs to be processed.
      ਜਾਰੀ;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((ਪਹਿਲਾ - 0xD800) << 10) | (ਦੂਜਾ - 0xDC00), ਬਾਹਰਲਾ);
    ਵਾਪਸ ਸੱਚ;
  }
}

ਤਾਰਕਿਕ Parser::parseError(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_4) {
  ਪੂਰਨ ਸਤਰ = 1;
  ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ_ਦਾ_ਸਤਰ = ਸ਼ੁਰੂ;
  ਲਈ (ਅਚਲ ਅੱਖਰ *X = ਸ਼ੁਰੂ; X < P; ++X) {
    ਜੇ (*X == 0x0A) {
      ++ਸਤਰ;
      ਸ਼ੁਰੂ_ਦਾ_ਸਤਰ = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(ਸੁਨੇਹਾ_4, ਸਤਰ, P - ਸ਼ੁਰੂ_ਦਾ_ਸਤਰ, P - ਸ਼ੁਰੂ));
  ਵਾਪਸ ਝੂਠ;
}

Expected<ਮੁੱਲ> parse(StringRef JSON) {
  Parser P(JSON);
  ਮੁੱਲ E = ਖਾਲੀ_ਸੂਚਕ;
  ਜੇ (P.checkUTF8())
    ਜੇ (P.parse_ਮੁੱਲ(E))
      ਜੇ (P.assert_ਸਿਰਾ())
        ਵਾਪਸ std::move(E);
  ਵਾਪਸ P.takeError();
}

ਅੱਖਰ ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
ਖਾਲੀ ParseError::ਲਘੂਗਣਕ(llvm::raw_ostream &OS) ਅਚਲ {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", ਸਤਰ, ਕਾਲਮ, ਵਿੱਥ, ਸੁਨੇਹਾ_4);
}

ਤਾਰਕਿਕ isUTF8(llvm::StringRef S, size_t *Err_ਵਿੱਥ) {
  // Fast-path for ASCII, which is valid UTF-8.
  ਜੇ (LLVM_LIKELY(isASCII(S)))
    ਵਾਪਸ ਸੱਚ;

  ਅਚਲ UTF8 *ਅੰਕੜੇ_2 = reinterpret_cast<ਅਚਲ UTF8 *>(S.ਅੰਕੜੇ()), *Rest = ਅੰਕੜੇ_2;
  ਜੇ (LLVM_LIKELY(isLegalUTF8String(&Rest, ਅੰਕੜੇ_2 + S.ਮਾਪ_2())))
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (Err_ਵਿੱਥ)
    *Err_ਵਿੱਥ = Rest - ਅੰਕੜੇ_2;
  ਵਾਪਸ ਝੂਠ;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::ਸਦਿਸ਼<UTF32> Codepoints(S.ਮਾਪ_2()); // 1 codepoint per byte suffices.
  ਅਚਲ UTF8 *In8 = reinterpret_cast<ਅਚਲ UTF8 *>(S.ਅੰਕੜੇ());
  UTF32 *Out32 = Codepoints.ਅੰਕੜੇ();
  ConvertUTF8toUTF32(&In8, In8 + S.ਮਾਪ_2(), &Out32, Out32 + Codepoints.ਮਾਪ_2(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.ਅੰਕੜੇ());
  std::string Res(4 * Codepoints.ਮਾਪ_2(), 0); // 4 bytes per codepoint suffice
  ਅਚਲ UTF32 *In32 = Codepoints.ਅੰਕੜੇ();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.ਮਾਪ_2(), &Out8, Out8 + Res.ਮਾਪ_2(),
                     strictConversion);
  Res.resize(reinterpret_cast<ਅੱਖਰ *>(Out8) - Res.ਅੰਕੜੇ());
  ਵਾਪਸ Res;
}

ਸਥਿਰ ਖਾਲੀ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ C : S) {
    ਜੇ (C == 0x22 || C == 0x5C)
      OS << '\\';
    ਜੇ (C >= 0x20) {
      OS << C;
      ਜਾਰੀ;
    }
    OS << '\\';
    ਚੋਣ (C) {
    // A few characters are common enough to make short escapes worthwhile.
    ਹਾਲਤ '\t':
      OS << 't';
      ਰੋਕੋ;
    ਹਾਲਤ '\n':
      OS << 'n';
      ਰੋਕੋ;
    ਹਾਲਤ '\r':
      OS << 'r';
      ਰੋਕੋ;
    ਮੂਲ:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::ਹੇਠਲਾ_2, 4);
      ਰੋਕੋ;
    }
  }
  OS << '\"';
}

ਖਾਲੀ llvm::json::OStream::ਮੁੱਲ_2(ਅਚਲ ਮੁੱਲ &V) {
  ਚੋਣ (V.ਕਿਸਮ_2()) {
  ਹਾਲਤ ਮੁੱਲ::Null:
    ਮੁੱਲ_Begin();
    OS << "null";
    ਵਾਪਸ;
  ਹਾਲਤ ਮੁੱਲ::Boolean:
    ਮੁੱਲ_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    ਵਾਪਸ;
  ਹਾਲਤ ਮੁੱਲ::ਸੰਖਿਆ:
    ਮੁੱਲ_Begin();
    ਜੇ (V.Type == ਮੁੱਲ::T_Integer)
      OS << *V.getAsInteger();
    ਨਹੀਂਤਾਂ ਜੇ (V.Type == ਮੁੱਲ::T_UINT64)
      OS << *V.getAsUINT64();
    ਨਹੀਂਤਾਂ
      OS << format("%.*g", std::numeric_limits<ਦੁੱਗਣਾ>::max_digits10,
                   *V.get_As_ਸੰਖਿਆ());
    ਵਾਪਸ;
  ਹਾਲਤ ਮੁੱਲ::String:
    ਮੁੱਲ_Begin();
    quote(OS, *V.getAsString());
    ਵਾਪਸ;
  ਹਾਲਤ ਮੁੱਲ::Array:
    ਵਾਪਸ array([&] {
      ਲਈ (ਅਚਲ ਮੁੱਲ &E : *V.getAsArray())
        ਮੁੱਲ_2(E);
    });
  ਹਾਲਤ ਮੁੱਲ::Object:
    ਵਾਪਸ object([&] {
      ਲਈ (ਅਚਲ Object::ਮੁੱਲ_type *E : sortedElements(*V.getAsObject()))
        attribute(E->ਪਹਿਲਾ_2, E->ਦੂਜਾ_2);
    });
  }
}

ਖਾਲੀ llvm::json::OStream::ਮੁੱਲ_Begin() {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Ctx != Object && "Only attributes allowed here");
  ਜੇ (ਢੇਰ.ਪਿੱਠ().Has_ਮੁੱਲ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  ਜੇ (ਢੇਰ.ਪਿੱਠ().Ctx == Array)
    newline();
  flushComment();
  ਢੇਰ.ਪਿੱਠ().Has_ਮੁੱਲ = ਸੱਚ;
}

ਖਾਲੀ OStream::comment(llvm::StringRef Comment) {
  ਪ੍ਰਤਿਜ੍ਞਾ(PendingComment.ਖਾਲੀ_ਥਾਵਾਂ() && "Only one comment per value!");
  PendingComment = Comment;
}

ਖਾਲੀ OStream::flushComment() {
  ਜੇ (PendingComment.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;
  OS << (Indent_ਮਾਪ ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  ਜਦੋਂ (!PendingComment.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਸਵੈਚਾਲਿਤ Pos = PendingComment.ਲੱਭੋ("*/");
    ਜੇ (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } ਨਹੀਂਤਾਂ {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_ਮਾਪ ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  ਜੇ (ਢੇਰ.ਮਾਪ_2() > 1 && ਢੇਰ.ਪਿੱਠ().Ctx == Singleton) {
    ਜੇ (Indent_ਮਾਪ)
      OS << ' ';
  } ਨਹੀਂਤਾਂ {
    newline();
  }
}

ਖਾਲੀ llvm::json::OStream::newline() {
  ਜੇ (Indent_ਮਾਪ) {
    OS.ਲਿਖ('\n');
    OS.indent(Indent);
  }
}

ਖਾਲੀ llvm::json::OStream::arrayBegin() {
  ਮੁੱਲ_Begin();
  ਢੇਰ.emplace_ਪਿੱਠ();
  ਢੇਰ.ਪਿੱਠ().Ctx = Array;
  Indent += Indent_ਮਾਪ;
  OS << '[';
}

ਖਾਲੀ llvm::json::OStream::array_ਸਿਰਾ() {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Ctx == Array);
  Indent -= Indent_ਮਾਪ;
  ਜੇ (ਢੇਰ.ਪਿੱਠ().Has_ਮੁੱਲ)
    newline();
  OS << ']';
  ਪ੍ਰਤਿਜ੍ਞਾ(PendingComment.ਖਾਲੀ_ਥਾਵਾਂ());
  ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ());
}

ਖਾਲੀ llvm::json::OStream::objectBegin() {
  ਮੁੱਲ_Begin();
  ਢੇਰ.emplace_ਪਿੱਠ();
  ਢੇਰ.ਪਿੱਠ().Ctx = Object;
  Indent += Indent_ਮਾਪ;
  OS << '{';
}

ਖਾਲੀ llvm::json::OStream::object_ਸਿਰਾ() {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Ctx == Object);
  Indent -= Indent_ਮਾਪ;
  ਜੇ (ਢੇਰ.ਪਿੱਠ().Has_ਮੁੱਲ)
    newline();
  OS << '}';
  ਪ੍ਰਤਿਜ੍ਞਾ(PendingComment.ਖਾਲੀ_ਥਾਵਾਂ());
  ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ());
}

ਖਾਲੀ llvm::json::OStream::attributeBegin(llvm::StringRef ਕੁੰਜੀ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Ctx == Object);
  ਜੇ (ਢੇਰ.ਪਿੱਠ().Has_ਮੁੱਲ)
    OS << ',';
  newline();
  flushComment();
  ਢੇਰ.ਪਿੱਠ().Has_ਮੁੱਲ = ਸੱਚ;
  ਢੇਰ.emplace_ਪਿੱਠ();
  ਢੇਰ.ਪਿੱਠ().Ctx = Singleton;
  ਜੇ (LLVM_LIKELY(isUTF8(ਕੁੰਜੀ))) {
    quote(OS, ਕੁੰਜੀ);
  } ਨਹੀਂਤਾਂ {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਝੂਠ && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(ਕੁੰਜੀ));
  }
  OS.ਲਿਖ(':');
  ਜੇ (Indent_ਮਾਪ)
    OS.ਲਿਖ(' ');
}

ਖਾਲੀ llvm::json::OStream::attribute_ਸਿਰਾ() {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Ctx == Singleton);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Has_ਮੁੱਲ && "Attribute must have a value");
  ਪ੍ਰਤਿਜ੍ਞਾ(PendingComment.ਖਾਲੀ_ਥਾਵਾਂ());
  ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
  ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_ਮੁੱਲ_Begin() {
  ਮੁੱਲ_Begin();
  ਢੇਰ.emplace_ਪਿੱਠ();
  ਢੇਰ.ਪਿੱਠ().Ctx = Raw_ਮੁੱਲ;
  ਵਾਪਸ OS;
}

ਖਾਲੀ llvm::json::OStream::raw_ਮੁੱਲ_ਸਿਰਾ() {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ.ਪਿੱਠ().Ctx == Raw_ਮੁੱਲ);
  ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
}

} // namespace json
} // namespace llvm

ਖਾਲੀ llvm::format_provider<llvm::json::ਮੁੱਲ>::format(
    ਅਚਲ llvm::json::ਮੁੱਲ &E, raw_ostream &OS, StringRef Options) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ IndentAmount = 0;
  ਜੇ (!Options.ਖਾਲੀ_ਥਾਵਾਂ() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).ਮੁੱਲ_2(E);
}

