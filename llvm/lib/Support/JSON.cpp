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

ಮೌಲ್ಯ &Object::ಕ್ರಿಯಾಕಾರಕ[](ಸ್ಥಿರಾಂಕ Object_ಕೀಲಿ &K) {
  ಹಿಂತಿರುಗಿ try_emplace(K, ಶೂನ್ಯಸೂಚಕ).ಮೊದಲ_2->get_ಎರಡನೇ();
}
ಮೌಲ್ಯ &Object::ಕ್ರಿಯಾಕಾರಕ[](Object_ಕೀಲಿ &&K) {
  ಹಿಂತಿರುಗಿ try_emplace(std::move(K), ಶೂನ್ಯಸೂಚಕ).ಮೊದಲ_2->get_ಎರಡನೇ();
}
ಮೌಲ್ಯ *Object::get(StringRef K) {
  ಸ್ವಯಂಚಾಲಿತ I = ಕಂಡುಹಿಡಿ(K);
  ಆದರೆ (I == ಕೊನೆ())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  ಹಿಂತಿರುಗಿ &I->ಎರಡನೇ_2;
}
ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ *Object::get(StringRef K) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ I = ಕಂಡುಹಿಡಿ(K);
  ಆದರೆ (I == ಕೊನೆ())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  ಹಿಂತಿರುಗಿ &I->ಎರಡನೇ_2;
}
std::optional<std::nullptr_t> Object::getNull(StringRef K) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->getAsNull();
  ಹಿಂತಿರುಗಿ std::nullopt;
}
std::optional<ತಾರ್ಕಿಕ> Object::getBoolean(StringRef K) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->getAsBoolean();
  ಹಿಂತಿರುಗಿ std::nullopt;
}
std::optional<ದ್ವಿಗುಣ> Object::get_ಸಂಖ್ಯೆ(StringRef K) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->get_As_ಸಂಖ್ಯೆ();
  ಹಿಂತಿರುಗಿ std::nullopt;
}
std::optional<int64_t> Object::getInteger(StringRef K) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->getAsInteger();
  ಹಿಂತಿರುಗಿ std::nullopt;
}
std::optional<llvm::StringRef> Object::getString(StringRef K) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->getAsString();
  ಹಿಂತಿರುಗಿ std::nullopt;
}
ಸ್ಥಿರಾಂಕ json::Object *Object::getObject(StringRef K) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->getAsObject();
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}
json::Object *Object::getObject(StringRef K) {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->getAsObject();
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}
ಸ್ಥಿರಾಂಕ json::Array *Object::getArray(StringRef K) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->getAsArray();
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}
json::Array *Object::getArray(StringRef K) {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *V = get(K))
    ಹಿಂತಿರುಗಿ V->getAsArray();
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}
ತಾರ್ಕಿಕ ಕ್ರಿಯಾಕಾರಕ==(ಸ್ಥಿರಾಂಕ Object &LHS, ಸ್ಥಿರಾಂಕ Object &RHS) { ಹಿಂತಿರುಗಿ LHS.M == RHS.M; }

Array::Array(std::initializer_list<ಮೌಲ್ಯ> Elements) {
  V.reserve(Elements.ಗಾತ್ರ_3());
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &V : Elements) {
    emplace_ಹಿಂಭಾಗ(ಶೂನ್ಯಸೂಚಕ);
    ಹಿಂಭಾಗ().moveFrom(std::move(V));
  }
}

ಮೌಲ್ಯ::ಮೌಲ್ಯ(std::initializer_list<ಮೌಲ್ಯ> Elements)
    : ಮೌಲ್ಯ(json::Array(Elements)) {}

ಶೂನ್ಯ ಮೌಲ್ಯ::copyFrom(ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &M) {
  Type = M.Type;
  ಆಯ್ಕೆ (Type) {
  ಸ್ಥಿತಿ T_Null:
  ಸ್ಥಿತಿ T_Boolean:
  ಸ್ಥಿತಿ T_Double:
  ಸ್ಥಿತಿ T_Integer:
  ಸ್ಥಿತಿ T_UINT64:
    ಸ್ಮರಣೆ_ನಕಲು(&Union, &M.Union, ಗಾತ್ರ(Union));
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_String:
    create<std::string>(M.as<std::string>());
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_Object:
    create<json::Object>(M.as<json::Object>());
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_Array:
    create<json::Array>(M.as<json::Array>());
    ನಿಲ್ಲಿಸು;
  }
}

ಶೂನ್ಯ ಮೌಲ್ಯ::moveFrom(ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &&M) {
  Type = M.Type;
  ಆಯ್ಕೆ (Type) {
  ಸ್ಥಿತಿ T_Null:
  ಸ್ಥಿತಿ T_Boolean:
  ಸ್ಥಿತಿ T_Double:
  ಸ್ಥಿತಿ T_Integer:
  ಸ್ಥಿತಿ T_UINT64:
    ಸ್ಮರಣೆ_ನಕಲು(&Union, &M.Union, ಗಾತ್ರ(Union));
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_StringRef:
    create<StringRef>(M.as<StringRef>());
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_String:
    create<std::string>(std::move(M.as<std::string>()));
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_Object:
    create<json::Object>(std::move(M.as<json::Object>()));
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_Array:
    create<json::Array>(std::move(M.as<json::Array>()));
    ನಿಲ್ಲಿಸು;
  }
  const_cast<ಮೌಲ್ಯ &>(M).destroy();
  M.Type = T_Null;
}

ಶೂನ್ಯ ಮೌಲ್ಯ::destroy() {
  ಆಯ್ಕೆ (Type) {
  ಸ್ಥಿತಿ T_Null:
  ಸ್ಥಿತಿ T_Boolean:
  ಸ್ಥಿತಿ T_Double:
  ಸ್ಥಿತಿ T_Integer:
  ಸ್ಥಿತಿ T_UINT64:
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_StringRef:
    as<StringRef>().~StringRef();
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_String:
    as<std::string>().~basic_string();
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_Object:
    as<json::Object>().~Object();
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ T_Array:
    as<json::Array>().~Array();
    ನಿಲ್ಲಿಸು;
  }
}

ಶೂನ್ಯ ಮೌಲ್ಯ::print(llvm::raw_ostream &OS) ಸ್ಥಿರಾಂಕ { OS << *ಇದು; }

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ ಮೌಲ್ಯ::dump() ಸ್ಥಿರಾಂಕ {
  print(llvm::dbgs());
  llvm::dbgs() << '\n';
}
#ಆದರೆ_ಅಂತ್ಯ

ತಾರ್ಕಿಕ ಕ್ರಿಯಾಕಾರಕ==(ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &L, ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &R) {
  ಆದರೆ (L.ಬಗೆ() != R.ಬಗೆ())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆಯ್ಕೆ (L.ಬಗೆ()) {
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Null:
    ಹಿಂತಿರುಗಿ *L.getAsNull() == *R.getAsNull();
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Boolean:
    ಹಿಂತಿರುಗಿ *L.getAsBoolean() == *R.getAsBoolean();
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::ಸಂಖ್ಯೆ:
    // Workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    // The same integer must convert to the same double, per the standard.
    // However we see 64-vs-80-bit precision comparisons with gcc-7 -O3 -m32.
    // So we avoid floating point promotion for exact comparisons.
    ಆದರೆ (L.Type == ಮೌಲ್ಯ::T_Integer || R.Type == ಮೌಲ್ಯ::T_Integer)
      ಹಿಂತಿರುಗಿ L.getAsInteger() == R.getAsInteger();
    ಹಿಂತಿರುಗಿ *L.get_As_ಸಂಖ್ಯೆ() == *R.get_As_ಸಂಖ್ಯೆ();
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::String:
    ಹಿಂತಿರುಗಿ *L.getAsString() == *R.getAsString();
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Array:
    ಹಿಂತಿರುಗಿ *L.getAsArray() == *R.getAsArray();
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Object:
    ಹಿಂತಿರುಗಿ *L.getAsObject() == *R.getAsObject();
  }
  llvm_unreachable("Unknown value kind");
}

ಶೂನ್ಯ ಮಾರ್ಗ::report(llvm::StringLiteral ಸಂದೇಶ_4) {
  // Walk up to the root context, and count the number of segments.
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 = 0;
  ಸ್ಥಿರಾಂಕ ಮಾರ್ಗ *P;
  ಗಾಗಿ (P = ಇದು; P->ಪೋಷಕ_2 != ಶೂನ್ಯಸೂಚಕ; P = P->ಪೋಷಕ_2)
    ++ಎಣಿಕೆ_3;
  ಮಾರ್ಗ::ಬೇರು_2 *R = P->Seg.ಬೇರು_3();
  // Fill in the error message and copy the path (in reverse order).
  R->Error_ಸಂದೇಶ_2 = ಸಂದೇಶ_4;
  R->Error_ಮಾರ್ಗ.resize(ಎಣಿಕೆ_3);
  ಸ್ವಯಂಚಾಲಿತ It = R->Error_ಮಾರ್ಗ.begin();
  ಗಾಗಿ (P = ಇದು; P->ಪೋಷಕ_2 != ಶೂನ್ಯಸೂಚಕ; P = P->ಪೋಷಕ_2)
    *It++ = P->Seg;
}

Error ಮಾರ್ಗ::ಬೇರು_2::getError() ಸ್ಥಿರಾಂಕ {
  std::string S;
  raw_string_ostream OS(S);
  OS << (Error_ಸಂದೇಶ_2.ಖಾಲಿಗಳು() ? "invalid JSON contents" : Error_ಸಂದೇಶ_2);
  ಆದರೆ (Error_ಮಾರ್ಗ.ಖಾಲಿಗಳು()) {
    ಆದರೆ (!Name.ಖಾಲಿಗಳು())
      OS << " when parsing " << Name;
  } ಇಲ್ಲದಿದ್ದರೆ {
    OS << " at " << (Name.ಖಾಲಿಗಳು() ? "(root)" : Name);
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಮಾರ್ಗ::Segment &S : llvm::ಹಿಮ್ಮುಖ(Error_ಮಾರ್ಗ)) {
      ಆದರೆ (S.is_ಕ್ಷೇತ್ರ())
        OS << '.' << S.ಕ್ಷೇತ್ರ();
      ಇಲ್ಲದಿದ್ದರೆ
        OS << '[' << S.ಸೂಚ್ಯಂಕ() << ']';
    }
  }
  ಹಿಂತಿರುಗಿ createStringError(llvm::inconvertible_Error_ಸಂಕೇತ(), S);
}

std::ಸದಿಶ<ಸ್ಥಿರಾಂಕ Object::ಮೌಲ್ಯ_type *> sortedElements(ಸ್ಥಿರಾಂಕ Object &O) {
  std::ಸದಿಶ<ಸ್ಥಿರಾಂಕ Object::ಮೌಲ್ಯ_type *> Elements;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &E : O)
    Elements.ತಳ್ಳು_ಹಿಂಭಾಗ(&E);
  llvm::ವಿಂಗಡಿಸು(Elements,
             [](ಸ್ಥಿರಾಂಕ Object::ಮೌಲ್ಯ_type *L, ಸ್ಥಿರಾಂಕ Object::ಮೌಲ್ಯ_type *R) {
               ಹಿಂತಿರುಗಿ L->ಮೊದಲ_2 < R->ಮೊದಲ_2;
             });
  ಹಿಂತಿರುಗಿ Elements;
}

// Prints a one-line version of a value that isn't our main focus.
// We interleave writes to OS and JOS, exploiting the lack of extra buffering.
// This is OK as we own the implementation.
ಸ್ಥಿರ ಶೂನ್ಯ abbreviate(ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &V, OStream &JOS) {
  ಆಯ್ಕೆ (V.ಬಗೆ()) {
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Array:
    JOS.raw_ಮೌಲ್ಯ(V.getAsArray()->ಖಾಲಿಗಳು() ? "[]" : "[ ... ]");
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Object:
    JOS.raw_ಮೌಲ್ಯ(V.getAsObject()->ಖಾಲಿಗಳು() ? "{}" : "{ ... }");
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::String: {
    llvm::StringRef S = *V.getAsString();
    ಆದರೆ (S.ಗಾತ್ರ_3() < 40) {
      JOS.ಮೌಲ್ಯ_2(V);
    } ಇಲ್ಲದಿದ್ದರೆ {
      std::string Truncated = fixUTF8(S.take_front(37));
      Truncated.append("...");
      JOS.ಮೌಲ್ಯ_2(Truncated);
    }
    ನಿಲ್ಲಿಸು;
  }
  ಪೂರ್ವನಿಯೋಜಿತ:
    JOS.ಮೌಲ್ಯ_2(V);
  }
}

// Prints a semi-expanded version of a value that is our main focus.
// Array/Object entries are printed, but not recursively as they may be huge.
ಸ್ಥಿರ ಶೂನ್ಯ abbreviateChildren(ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &V, OStream &JOS) {
  ಆಯ್ಕೆ (V.ಬಗೆ()) {
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Array:
    JOS.array([&] {
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &I : *V.getAsArray())
        abbreviate(I, JOS);
    });
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Object:
    JOS.object([&] {
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *KV : sortedElements(*V.getAsObject())) {
        JOS.attributeBegin(KV->ಮೊದಲ_2);
        abbreviate(KV->ಎರಡನೇ_2, JOS);
        JOS.attribute_ಕೊನೆ();
      }
    });
    ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ:
    JOS.ಮೌಲ್ಯ_2(V);
  }
}

ಶೂನ್ಯ ಮಾರ್ಗ::ಬೇರು_2::printErrorContext(ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &R, raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OStream JOS(OS, /*IndentSize=*/2);
  // PrintValue recurses down the path, printing the ancestors of our target.
  // Siblings of nodes along the path are printed with abbreviate(), and the
  // target itself is printed with the somewhat richer abbreviateChildren().
  // 'Recurse' is the lambda itself, to allow recursive calls.
  ಸ್ವಯಂಚಾಲಿತ Print_ಮೌಲ್ಯ = [&](ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &V, ArrayRef<Segment> ಮಾರ್ಗ, ಸ್ವಯಂಚಾಲಿತ &Recurse) {
    // Print the target node itself, with the error as a comment.
    // Also used if we can't follow our path, e.g. it names a field that
    // *should* exist but doesn't.
    ಸ್ವಯಂಚಾಲಿತ HighlightCurrent = [&] {
      std::string Comment = "error: ";
      Comment.append(Error_ಸಂದೇಶ_2.ದತ್ತಾಂಶ(), Error_ಸಂದೇಶ_2.ಗಾತ್ರ_3());
      JOS.comment(Comment);
      abbreviateChildren(V, JOS);
    };
    ಆದರೆ (ಮಾರ್ಗ.ಖಾಲಿಗಳು()) // We reached our target.
      ಹಿಂತಿರುಗಿ HighlightCurrent();
    ಸ್ಥಿರಾಂಕ Segment &S = ಮಾರ್ಗ.ಹಿಂಭಾಗ(); // Path is in reverse order.
    ಆದರೆ (S.is_ಕ್ಷೇತ್ರ()) {
      // Current node is an object, path names a field.
      llvm::StringRef ಕ್ಷೇತ್ರ_Name = S.ಕ್ಷೇತ್ರ();
      ಸ್ಥಿರಾಂಕ Object *O = V.getAsObject();
      ಆದರೆ (!O || !O->get(ಕ್ಷೇತ್ರ_Name))
        ಹಿಂತಿರುಗಿ HighlightCurrent();
      JOS.object([&] {
        ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *KV : sortedElements(*O)) {
          JOS.attributeBegin(KV->ಮೊದಲ_2);
          ಆದರೆ (ಕ್ಷೇತ್ರ_Name == StringRef(KV->ಮೊದಲ_2))
            Recurse(KV->ಎರಡನೇ_2, ಮಾರ್ಗ.drop_ಹಿಂಭಾಗ(), Recurse);
          ಇಲ್ಲದಿದ್ದರೆ
            abbreviate(KV->ಎರಡನೇ_2, JOS);
          JOS.attribute_ಕೊನೆ();
        }
      });
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Current node is an array, path names an element.
      ಸ್ಥಿರಾಂಕ Array *A = V.getAsArray();
      ಆದರೆ (!A || S.ಸೂಚ್ಯಂಕ() >= A->ಗಾತ್ರ_3())
        ಹಿಂತಿರುಗಿ HighlightCurrent();
      JOS.array([&] {
        ಚಿಹ್ನೆರಹಿತ Current = 0;
        ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &V : *A) {
          ಆದರೆ (Current++ == S.ಸೂಚ್ಯಂಕ())
            Recurse(V, ಮಾರ್ಗ.drop_ಹಿಂಭಾಗ(), Recurse);
          ಇಲ್ಲದಿದ್ದರೆ
            abbreviate(V, JOS);
        }
      });
    }
  };
  Print_ಮೌಲ್ಯ(R, Error_ಮಾರ್ಗ, Print_ಮೌಲ್ಯ);
}

ನಾಮಾಕಾಶ {
// Simple recursive-descent JSON parser.
ವರ್ಗ Parser {
ಸಾರ್ವಜನಿಕ:
  Parser(StringRef JSON)
      : ಆರಂಭ(JSON.begin()), P(JSON.begin()), ಕೊನೆ_2(JSON.ಕೊನೆ()) {}

  ತಾರ್ಕಿಕ checkUTF8() {
    size_t Err_ಸ್ಥಾನಾಂತರ;
    ಆದರೆ (isUTF8(StringRef(ಆರಂಭ, ಕೊನೆ_2 - ಆರಂಭ), &Err_ಸ್ಥಾನಾಂತರ))
      ಹಿಂತಿರುಗಿ ನಿಜ;
    P = ಆರಂಭ + Err_ಸ್ಥಾನಾಂತರ; // For line/column calculation.
    ಹಿಂತಿರುಗಿ parseError("Invalid UTF-8 sequence");
  }

  ತಾರ್ಕಿಕ parse_ಮೌಲ್ಯ(ಮೌಲ್ಯ &ಹೊರಹೋಗು);

  ತಾರ್ಕಿಕ assert_ಕೊನೆ() {
    eatWhitespace();
    ಆದರೆ (P == ಕೊನೆ_2)
      ಹಿಂತಿರುಗಿ ನಿಜ;
    ಹಿಂತಿರುಗಿ parseError("Text after end of document");
  }

  Error takeError() {
    ಪ್ರತಿಜ್ಞಾ(Err);
    ಹಿಂತಿರುಗಿ std::move(*Err);
  }

ಖಾಸಗಿ:
  ಶೂನ್ಯ eatWhitespace() {
    ಆಗ (P != ಕೊನೆ_2 && (*P == ' ' || *P == '\r' || *P == '\n' || *P == '\t'))
      ++P;
  }

  // On invalid syntax, parseX() functions return false and set Err.
  ತಾರ್ಕಿಕ parse_ಸಂಖ್ಯೆ(ಅಕ್ಷರ ಮೊದಲ, ಮೌಲ್ಯ &ಹೊರಹೋಗು);
  ತಾರ್ಕಿಕ parseString(std::string &ಹೊರಹೋಗು);
  ತಾರ್ಕಿಕ parseUnicode(std::string &ಹೊರಹೋಗು);
  ತಾರ್ಕಿಕ parseError(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_4); // always returns false

  ಅಕ್ಷರ ಮುಂದಿನ() { ಹಿಂತಿರುಗಿ P == ಕೊನೆ_2 ? 0 : *P++; }
  ಅಕ್ಷರ peek() { ಹಿಂತಿರುಗಿ P == ಕೊನೆ_2 ? 0 : *P; }
  ಸ್ಥಿರ ತಾರ್ಕಿಕ is_ಸಂಖ್ಯೆ(ಅಕ್ಷರ C) {
    ಹಿಂತಿರುಗಿ C == '0' || C == '1' || C == '2' || C == '3' || C == '4' ||
           C == '5' || C == '6' || C == '7' || C == '8' || C == '9' ||
           C == 'e' || C == 'E' || C == '+' || C == '-' || C == '.';
  }

  std::optional<Error> Err;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ, *P, *ಕೊನೆ_2;
};
} // namespace

ತಾರ್ಕಿಕ Parser::parse_ಮೌಲ್ಯ(ಮೌಲ್ಯ &ಹೊರಹೋಗು) {
  eatWhitespace();
  ಆದರೆ (P == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ parseError("Unexpected EOF");
  ಆಯ್ಕೆ (ಅಕ್ಷರ C = ಮುಂದಿನ()) {
  // Bare null/true/false are easy - first char identifies them.
  ಸ್ಥಿತಿ 'n':
    ಹೊರಹೋಗು = ಶೂನ್ಯಸೂಚಕ;
    ಹಿಂತಿರುಗಿ (ಮುಂದಿನ() == 'u' && ಮುಂದಿನ() == 'l' && ಮುಂದಿನ() == 'l') ||
           parseError("Invalid JSON value (null?)");
  ಸ್ಥಿತಿ 't':
    ಹೊರಹೋಗು = ನಿಜ;
    ಹಿಂತಿರುಗಿ (ಮುಂದಿನ() == 'r' && ಮುಂದಿನ() == 'u' && ಮುಂದಿನ() == 'e') ||
           parseError("Invalid JSON value (true?)");
  ಸ್ಥಿತಿ 'f':
    ಹೊರಹೋಗು = ಸುಳ್ಳು;
    ಹಿಂತಿರುಗಿ (ಮುಂದಿನ() == 'a' && ಮುಂದಿನ() == 'l' && ಮುಂದಿನ() == 's' && ಮುಂದಿನ() == 'e') ||
           parseError("Invalid JSON value (false?)");
  ಸ್ಥಿತಿ '"': {
    std::string S;
    ಆದರೆ (parseString(S)) {
      ಹೊರಹೋಗು = std::move(S);
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಸ್ಥಿತಿ '[': {
    ಹೊರಹೋಗು = Array{};
    Array &A = *ಹೊರಹೋಗು.getAsArray();
    eatWhitespace();
    ಆದರೆ (peek() == ']') {
      ++P;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ಗಾಗಿ (;;) {
      A.emplace_ಹಿಂಭಾಗ(ಶೂನ್ಯಸೂಚಕ);
      ಆದರೆ (!parse_ಮೌಲ್ಯ(A.ಹಿಂಭಾಗ()))
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      eatWhitespace();
      ಆಯ್ಕೆ (ಮುಂದಿನ()) {
      ಸ್ಥಿತಿ ',':
        eatWhitespace();
        ಮುಂದುವರಿ;
      ಸ್ಥಿತಿ ']':
        ಹಿಂತಿರುಗಿ ನಿಜ;
      ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ parseError("Expected , or ] after array element");
      }
    }
  }
  ಸ್ಥಿತಿ '{': {
    ಹೊರಹೋಗು = Object{};
    Object &O = *ಹೊರಹೋಗು.getAsObject();
    eatWhitespace();
    ಆದರೆ (peek() == '}') {
      ++P;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ಗಾಗಿ (;;) {
      ಆದರೆ (ಮುಂದಿನ() != '"')
        ಹಿಂತಿರುಗಿ parseError("Expected object key");
      std::string K;
      ಆದರೆ (!parseString(K))
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      eatWhitespace();
      ಆದರೆ (ಮುಂದಿನ() != ':')
        ಹಿಂತಿರುಗಿ parseError("Expected : after object key");
      eatWhitespace();
      ಆದರೆ (!parse_ಮೌಲ್ಯ(O[std::move(K)]))
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      eatWhitespace();
      ಆಯ್ಕೆ (ಮುಂದಿನ()) {
      ಸ್ಥಿತಿ ',':
        eatWhitespace();
        ಮುಂದುವರಿ;
      ಸ್ಥಿತಿ '}':
        ಹಿಂತಿರುಗಿ ನಿಜ;
      ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ parseError("Expected , or } after object property");
      }
    }
  }
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಆದರೆ (is_ಸಂಖ್ಯೆ(C))
      ಹಿಂತಿರುಗಿ parse_ಸಂಖ್ಯೆ(C, ಹೊರಹೋಗು);
    ಹಿಂತಿರುಗಿ parseError("Invalid JSON value");
  }
}

ತಾರ್ಕಿಕ Parser::parse_ಸಂಖ್ಯೆ(ಅಕ್ಷರ ಮೊದಲ, ಮೌಲ್ಯ &ಹೊರಹೋಗು) {
  // Read the number into a string. (Must be null-terminated for strto*).
  SmallString<24> S;
  S.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೊದಲ);
  ಆಗ (is_ಸಂಖ್ಯೆ(peek()))
    S.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮುಂದಿನ());
  ಅಕ್ಷರ *ಕೊನೆ_2;
  // Try first to parse as integer, and if so preserve full 64 bits.
  // We check for errno for out of bounds errors and for End == S.end()
  // to make sure that the numeric string is not malformed.
  errno = 0;
  int64_t I = std::strtoll(S.c_str(), &ಕೊನೆ_2, 10);
  ಆದರೆ (ಕೊನೆ_2 == S.ಕೊನೆ() && errno != ERANGE) {
    ಹೊರಹೋಗು = I;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  // strtroull has a special handling for negative numbers, but in this
  // case we don't want to do that because negative numbers were already
  // handled in the previous block.
  ಆದರೆ (ಮೊದಲ != '-') {
    errno = 0;
    uint64_t UI = std::strtoull(S.c_str(), &ಕೊನೆ_2, 10);
    ಆದರೆ (ಕೊನೆ_2 == S.ಕೊನೆ() && errno != ERANGE) {
      ಹೊರಹೋಗು = UI;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
  }
  // If it's not an integer
  ಹೊರಹೋಗು = std::strtod(S.c_str(), &ಕೊನೆ_2);
  ಹಿಂತಿರುಗಿ ಕೊನೆ_2 == S.ಕೊನೆ() || parseError("Invalid JSON value (number?)");
}

ತಾರ್ಕಿಕ Parser::parseString(std::string &ಹೊರಹೋಗು) {
  // leading quote was already consumed.
  ಗಾಗಿ (ಅಕ್ಷರ C = ಮುಂದಿನ(); C != '"'; C = ಮುಂದಿನ()) {
    ಆದರೆ (LLVM_UNLIKELY(P == ಕೊನೆ_2))
      ಹಿಂತಿರುಗಿ parseError("Unterminated string");
    ಆದರೆ (LLVM_UNLIKELY((C & 0x1f) == C))
      ಹಿಂತಿರುಗಿ parseError("Control character in string");
    ಆದರೆ (LLVM_LIKELY(C != '\\')) {
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(C);
      ಮುಂದುವರಿ;
    }
    // Handle escape sequence.
    ಆಯ್ಕೆ (C = ಮುಂದಿನ()) {
    ಸ್ಥಿತಿ '"':
    ಸ್ಥಿತಿ '\\':
    ಸ್ಥಿತಿ '/':
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(C);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'b':
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ('\b');
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'f':
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ('\f');
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'n':
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ('\n');
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'r':
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ('\r');
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 't':
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ('\t');
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'u':
      ಆದರೆ (!parseUnicode(ಹೊರಹೋಗು))
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ parseError("Invalid escape sequence");
    }
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಸ್ಥಿರ ಶೂನ್ಯ encodeUtf8(uint32_t Rune, std::string &ಹೊರಹೋಗು) {
  ಆದರೆ (Rune < 0x80) {
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(Rune & 0x7F);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Rune < 0x800) {
    uint8_t ಮೊದಲ_Byte = 0xC0 | ((Rune & 0x7C0) >> 6);
    uint8_t ಎರಡನೇ_Byte = 0x80 | (Rune & 0x3F);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೊದಲ_Byte);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಎರಡನೇ_Byte);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Rune < 0x10000) {
    uint8_t ಮೊದಲ_Byte = 0xE0 | ((Rune & 0xF000) >> 12);
    uint8_t ಎರಡನೇ_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t ಮೂರನೇ_Byte = 0x80 | (Rune & 0x3F);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೊದಲ_Byte);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಎರಡನೇ_Byte);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೂರನೇ_Byte);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Rune < 0x110000) {
    uint8_t ಮೊದಲ_Byte = 0xF0 | ((Rune & 0x1F0000) >> 18);
    uint8_t ಎರಡನೇ_Byte = 0x80 | ((Rune & 0x3F000) >> 12);
    uint8_t ಮೂರನೇ_Byte = 0x80 | ((Rune & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Rune & 0x3F);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೊದಲ_Byte);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಎರಡನೇ_Byte);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೂರನೇ_Byte);
    ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(FourthByte);
  } ಇಲ್ಲದಿದ್ದರೆ {
    llvm_unreachable("Invalid codepoint");
  }
}

// Parse a UTF-16 \uNNNN escape sequence. "\u" has already been consumed.
// May parse several sequential escapes to ensure proper surrogate handling.
// We do not use ConvertUTF.h, it can't accept and replace unpaired surrogates.
// These are invalid Unicode but valid JSON (RFC 8259, section 8.2).
ತಾರ್ಕಿಕ Parser::parseUnicode(std::string &ಹೊರಹೋಗು) {
  // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
  ಸ್ವಯಂಚಾಲಿತ Invalid = [&] { ಹೊರಹೋಗು.append(/* UTF-8 */ {'\xef', '\xbf', '\xbd'}); };
  // Decodes 4 hex digits from the stream into Out, returns false on error.
  ಸ್ವಯಂಚಾಲಿತ Parse4Hex = [ಇದು](uint16_t &ಹೊರಹೋಗು) -> ತಾರ್ಕಿಕ {
    ಹೊರಹೋಗು = 0;
    ಅಕ್ಷರ ಬೈಟುಗಳು_2[] = {ಮುಂದಿನ(), ಮುಂದಿನ(), ಮುಂದಿನ(), ಮುಂದಿನ()};
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ C : ಬೈಟುಗಳು_2) {
      ಆದರೆ (!std::isxdigit(C))
        ಹಿಂತಿರುಗಿ parseError("Invalid \\u escape sequence");
      ಹೊರಹೋಗು <<= 4;
      ಹೊರಹೋಗು |= (C > '9') ? (C & ~0x20) - 'A' + 10 : (C - '0');
    }
    ಹಿಂತಿರುಗಿ ನಿಜ;
  };
  uint16_t ಮೊದಲ; // UTF-16 code unit from the first \u escape.
  ಆದರೆ (!Parse4Hex(ಮೊದಲ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // We loop to allow proper surrogate-pair error handling.
  ಆಗ (ನಿಜ) {
    // Case 1: the UTF-16 code unit is already a codepoint in the BMP.
    ಆದರೆ (LLVM_LIKELY(ಮೊದಲ < 0xD800 || ಮೊದಲ >= 0xE000)) {
      encodeUtf8(ಮೊದಲ, ಹೊರಹೋಗು);
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }

    // Case 2: it's an (unpaired) trailing surrogate.
    ಆದರೆ (LLVM_UNLIKELY(ಮೊದಲ >= 0xDC00)) {
      Invalid();
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }

    // Case 3: it's a leading surrogate. We expect a trailing one next.
    // Case 3a: there's no trailing \u escape. Don't advance in the stream.
    ಆದರೆ (LLVM_UNLIKELY(P + 2 > ಕೊನೆ_2 || *P != '\\' || *(P + 1) != 'u')) {
      Invalid(); // Leading surrogate was unpaired.
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    P += 2;
    uint16_t ಎರಡನೇ;
    ಆದರೆ (!Parse4Hex(ಎರಡನೇ))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    // Case 3b: there was another \u escape, but it wasn't a trailing surrogate.
    ಆದರೆ (LLVM_UNLIKELY(ಎರಡನೇ < 0xDC00 || ಎರಡನೇ >= 0xE000)) {
      Invalid();      // Leading surrogate was unpaired.
      ಮೊದಲ = ಎರಡನೇ; // Second escape still needs to be processed.
      ಮುಂದುವರಿ;
    }
    // Case 3c: a valid surrogate pair encoding an astral codepoint.
    encodeUtf8(0x10000 | ((ಮೊದಲ - 0xD800) << 10) | (ಎರಡನೇ - 0xDC00), ಹೊರಹೋಗು);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
}

ತಾರ್ಕಿಕ Parser::parseError(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_4) {
  ಪೂರ್ಣಾಂಕ ಸಾಲು = 1;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ_ದ_ಸಾಲು = ಆರಂಭ;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *X = ಆರಂಭ; X < P; ++X) {
    ಆದರೆ (*X == 0x0A) {
      ++ಸಾಲು;
      ಆರಂಭ_ದ_ಸಾಲು = X + 1;
    }
  }
  Err.emplace(
      std::make_unique<ParseError>(ಸಂದೇಶ_4, ಸಾಲು, P - ಆರಂಭ_ದ_ಸಾಲು, P - ಆರಂಭ));
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

Expected<ಮೌಲ್ಯ> parse(StringRef JSON) {
  Parser P(JSON);
  ಮೌಲ್ಯ E = ಶೂನ್ಯಸೂಚಕ;
  ಆದರೆ (P.checkUTF8())
    ಆದರೆ (P.parse_ಮೌಲ್ಯ(E))
      ಆದರೆ (P.assert_ಕೊನೆ())
        ಹಿಂತಿರುಗಿ std::move(E);
  ಹಿಂತಿರುಗಿ P.takeError();
}

ಅಕ್ಷರ ParseError::ID = 0;

// Defined out-of-line to place vtable in this compilation unit.
ಶೂನ್ಯ ParseError::ಲಘುಗಣಕ(llvm::raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << llvm::formatv("[{0}:{1}, byte={2}]: {3}", ಸಾಲು, ಸ್ತಂಭ, ಸ್ಥಾನಾಂತರ, ಸಂದೇಶ_4);
}

ತಾರ್ಕಿಕ isUTF8(llvm::StringRef S, size_t *Err_ಸ್ಥಾನಾಂತರ) {
  // Fast-path for ASCII, which is valid UTF-8.
  ಆದರೆ (LLVM_LIKELY(isASCII(S)))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಸ್ಥಿರಾಂಕ UTF8 *ದತ್ತಾಂಶ_2 = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(S.ದತ್ತಾಂಶ()), *Rest = ದತ್ತಾಂಶ_2;
  ಆದರೆ (LLVM_LIKELY(isLegalUTF8String(&Rest, ದತ್ತಾಂಶ_2 + S.ಗಾತ್ರ_3())))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (Err_ಸ್ಥಾನಾಂತರ)
    *Err_ಸ್ಥಾನಾಂತರ = Rest - ದತ್ತಾಂಶ_2;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

std::string fixUTF8(llvm::StringRef S) {
  // This isn't particularly efficient, but is only for error-recovery.
  std::ಸದಿಶ<UTF32> Codepoints(S.ಗಾತ್ರ_3()); // 1 codepoint per byte suffices.
  ಸ್ಥಿರಾಂಕ UTF8 *In8 = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(S.ದತ್ತಾಂಶ());
  UTF32 *Out32 = Codepoints.ದತ್ತಾಂಶ();
  ConvertUTF8toUTF32(&In8, In8 + S.ಗಾತ್ರ_3(), &Out32, Out32 + Codepoints.ಗಾತ್ರ_3(),
                     lenientConversion);
  Codepoints.resize(Out32 - Codepoints.ದತ್ತಾಂಶ());
  std::string Res(4 * Codepoints.ಗಾತ್ರ_3(), 0); // 4 bytes per codepoint suffice
  ಸ್ಥಿರಾಂಕ UTF32 *In32 = Codepoints.ದತ್ತಾಂಶ();
  UTF8 *Out8 = reinterpret_cast<UTF8 *>(&Res[0]);
  ConvertUTF32toUTF8(&In32, In32 + Codepoints.ಗಾತ್ರ_3(), &Out8, Out8 + Res.ಗಾತ್ರ_3(),
                     strictConversion);
  Res.resize(reinterpret_cast<ಅಕ್ಷರ *>(Out8) - Res.ದತ್ತಾಂಶ());
  ಹಿಂತಿರುಗಿ Res;
}

ಸ್ಥಿರ ಶೂನ್ಯ quote(llvm::raw_ostream &OS, llvm::StringRef S) {
  OS << '\"';
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ C : S) {
    ಆದರೆ (C == 0x22 || C == 0x5C)
      OS << '\\';
    ಆದರೆ (C >= 0x20) {
      OS << C;
      ಮುಂದುವರಿ;
    }
    OS << '\\';
    ಆಯ್ಕೆ (C) {
    // A few characters are common enough to make short escapes worthwhile.
    ಸ್ಥಿತಿ '\t':
      OS << 't';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\n':
      OS << 'n';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\r':
      OS << 'r';
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      OS << 'u';
      llvm::write_hex(OS, C, llvm::HexPrintStyle::ಅಧೋ_2, 4);
      ನಿಲ್ಲಿಸು;
    }
  }
  OS << '\"';
}

ಶೂನ್ಯ llvm::json::OStream::ಮೌಲ್ಯ_2(ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &V) {
  ಆಯ್ಕೆ (V.ಬಗೆ()) {
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Null:
    ಮೌಲ್ಯ_Begin();
    OS << "null";
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Boolean:
    ಮೌಲ್ಯ_Begin();
    OS << (*V.getAsBoolean() ? "true" : "false");
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::ಸಂಖ್ಯೆ:
    ಮೌಲ್ಯ_Begin();
    ಆದರೆ (V.Type == ಮೌಲ್ಯ::T_Integer)
      OS << *V.getAsInteger();
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (V.Type == ಮೌಲ್ಯ::T_UINT64)
      OS << *V.getAsUINT64();
    ಇಲ್ಲದಿದ್ದರೆ
      OS << format("%.*g", std::numeric_limits<ದ್ವಿಗುಣ>::max_digits10,
                   *V.get_As_ಸಂಖ್ಯೆ());
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::String:
    ಮೌಲ್ಯ_Begin();
    quote(OS, *V.getAsString());
    ಹಿಂತಿರುಗಿ;
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Array:
    ಹಿಂತಿರುಗಿ array([&] {
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಮೌಲ್ಯ &E : *V.getAsArray())
        ಮೌಲ್ಯ_2(E);
    });
  ಸ್ಥಿತಿ ಮೌಲ್ಯ::Object:
    ಹಿಂತಿರುಗಿ object([&] {
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ Object::ಮೌಲ್ಯ_type *E : sortedElements(*V.getAsObject()))
        attribute(E->ಮೊದಲ_2, E->ಎರಡನೇ_2);
    });
  }
}

ಶೂನ್ಯ llvm::json::OStream::ಮೌಲ್ಯ_Begin() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Ctx != Object && "Only attributes allowed here");
  ಆದರೆ (ರಾಶಿ.ಹಿಂಭಾಗ().Has_ಮೌಲ್ಯ) {
    ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Ctx != Singleton && "Only one value allowed here");
    OS << ',';
  }
  ಆದರೆ (ರಾಶಿ.ಹಿಂಭಾಗ().Ctx == Array)
    newline();
  flushComment();
  ರಾಶಿ.ಹಿಂಭಾಗ().Has_ಮೌಲ್ಯ = ನಿಜ;
}

ಶೂನ್ಯ OStream::comment(llvm::StringRef Comment) {
  ಪ್ರತಿಜ್ಞಾ(PendingComment.ಖಾಲಿಗಳು() && "Only one comment per value!");
  PendingComment = Comment;
}

ಶೂನ್ಯ OStream::flushComment() {
  ಆದರೆ (PendingComment.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ;
  OS << (Indent_ಗಾತ್ರ ? "/* " : "/*");
  // Be sure not to accidentally emit "*/". Transform to "* /".
  ಆಗ (!PendingComment.ಖಾಲಿಗಳು()) {
    ಸ್ವಯಂಚಾಲಿತ Pos = PendingComment.ಕಂಡುಹಿಡಿ("*/");
    ಆದರೆ (Pos == StringRef::npos) {
      OS << PendingComment;
      PendingComment = "";
    } ಇಲ್ಲದಿದ್ದರೆ {
      OS << PendingComment.take_front(Pos) << "* /";
      PendingComment = PendingComment.drop_front(Pos + 2);
    }
  }
  OS << (Indent_ಗಾತ್ರ ? " */" : "*/");
  // Comments are on their own line unless attached to an attribute value.
  ಆದರೆ (ರಾಶಿ.ಗಾತ್ರ_3() > 1 && ರಾಶಿ.ಹಿಂಭಾಗ().Ctx == Singleton) {
    ಆದರೆ (Indent_ಗಾತ್ರ)
      OS << ' ';
  } ಇಲ್ಲದಿದ್ದರೆ {
    newline();
  }
}

ಶೂನ್ಯ llvm::json::OStream::newline() {
  ಆದರೆ (Indent_ಗಾತ್ರ) {
    OS.ಲಿಖ('\n');
    OS.indent(Indent);
  }
}

ಶೂನ್ಯ llvm::json::OStream::arrayBegin() {
  ಮೌಲ್ಯ_Begin();
  ರಾಶಿ.emplace_ಹಿಂಭಾಗ();
  ರಾಶಿ.ಹಿಂಭಾಗ().Ctx = Array;
  Indent += Indent_ಗಾತ್ರ;
  OS << '[';
}

ಶೂನ್ಯ llvm::json::OStream::array_ಕೊನೆ() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Ctx == Array);
  Indent -= Indent_ಗಾತ್ರ;
  ಆದರೆ (ರಾಶಿ.ಹಿಂಭಾಗ().Has_ಮೌಲ್ಯ)
    newline();
  OS << ']';
  ಪ್ರತಿಜ್ಞಾ(PendingComment.ಖಾಲಿಗಳು());
  ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
  ಪ್ರತಿಜ್ಞಾ(!ರಾಶಿ.ಖಾಲಿಗಳು());
}

ಶೂನ್ಯ llvm::json::OStream::objectBegin() {
  ಮೌಲ್ಯ_Begin();
  ರಾಶಿ.emplace_ಹಿಂಭಾಗ();
  ರಾಶಿ.ಹಿಂಭಾಗ().Ctx = Object;
  Indent += Indent_ಗಾತ್ರ;
  OS << '{';
}

ಶೂನ್ಯ llvm::json::OStream::object_ಕೊನೆ() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Ctx == Object);
  Indent -= Indent_ಗಾತ್ರ;
  ಆದರೆ (ರಾಶಿ.ಹಿಂಭಾಗ().Has_ಮೌಲ್ಯ)
    newline();
  OS << '}';
  ಪ್ರತಿಜ್ಞಾ(PendingComment.ಖಾಲಿಗಳು());
  ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
  ಪ್ರತಿಜ್ಞಾ(!ರಾಶಿ.ಖಾಲಿಗಳು());
}

ಶೂನ್ಯ llvm::json::OStream::attributeBegin(llvm::StringRef ಕೀಲಿ) {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Ctx == Object);
  ಆದರೆ (ರಾಶಿ.ಹಿಂಭಾಗ().Has_ಮೌಲ್ಯ)
    OS << ',';
  newline();
  flushComment();
  ರಾಶಿ.ಹಿಂಭಾಗ().Has_ಮೌಲ್ಯ = ನಿಜ;
  ರಾಶಿ.emplace_ಹಿಂಭಾಗ();
  ರಾಶಿ.ಹಿಂಭಾಗ().Ctx = Singleton;
  ಆದರೆ (LLVM_LIKELY(isUTF8(ಕೀಲಿ))) {
    quote(OS, ಕೀಲಿ);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಪ್ರತಿಜ್ಞಾ(ಸುಳ್ಳು && "Invalid UTF-8 in attribute key");
    quote(OS, fixUTF8(ಕೀಲಿ));
  }
  OS.ಲಿಖ(':');
  ಆದರೆ (Indent_ಗಾತ್ರ)
    OS.ಲಿಖ(' ');
}

ಶೂನ್ಯ llvm::json::OStream::attribute_ಕೊನೆ() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Ctx == Singleton);
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Has_ಮೌಲ್ಯ && "Attribute must have a value");
  ಪ್ರತಿಜ್ಞಾ(PendingComment.ಖಾಲಿಗಳು());
  ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Ctx == Object);
}

raw_ostream &llvm::json::OStream::raw_ಮೌಲ್ಯ_Begin() {
  ಮೌಲ್ಯ_Begin();
  ರಾಶಿ.emplace_ಹಿಂಭಾಗ();
  ರಾಶಿ.ಹಿಂಭಾಗ().Ctx = Raw_ಮೌಲ್ಯ;
  ಹಿಂತಿರುಗಿ OS;
}

ಶೂನ್ಯ llvm::json::OStream::raw_ಮೌಲ್ಯ_ಕೊನೆ() {
  ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.ಹಿಂಭಾಗ().Ctx == Raw_ಮೌಲ್ಯ);
  ರಾಶಿ.ತೆಗೆ_ಹಿಂಭಾಗ();
}

} // namespace json
} // namespace llvm

ಶೂನ್ಯ llvm::format_provider<llvm::json::ಮೌಲ್ಯ>::format(
    ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &E, raw_ostream &OS, StringRef Options) {
  ಚಿಹ್ನೆರಹಿತ IndentAmount = 0;
  ಆದರೆ (!Options.ಖಾಲಿಗಳು() && Options.getAsInteger(/*Radix=*/10, IndentAmount))
    llvm_unreachable("json::Value format options should be an integer");
  json::OStream(OS, IndentAmount).ಮೌಲ್ಯ_2(E);
}

