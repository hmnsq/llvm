//===- AllocToken.cpp - Allocation Token Calculation ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Definition of AllocToken modes and shared calculation of stateless token IDs.
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/AllocToken.h"
#شامل "llvm/ADT/StringSwitch.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/SipHash.h"

استعمال نامخلا llvm;

std::optional<Alloc_Token_منوال>
llvm::get_Alloc_Token_منوال_From_String(StringRef Name) {
  واپس StringSwitch<std::optional<Alloc_Token_منوال>>(Name)
      .Case("increment", Alloc_Token_منوال::Increment)
      .Case("random", Alloc_Token_منوال::Random)
      .Case("typehash", Alloc_Token_منوال::TypeHash)
      .Case("typehashpointersplit", Alloc_Token_منوال::Type_Hash_اشارہ_گر_بانٹو)
      .Case("typefunchash", Alloc_Token_منوال::TypeFuncHash)
      .Case("typefunchashpointersplit",
            Alloc_Token_منوال::Type_Func_Hash_اشارہ_گر_بانٹو)
      .Case("default", Default_Alloc_Token_منوال)
      .Default(std::nullopt);
}

StringRef llvm::get_Alloc_Token_منوال_As_String(Alloc_Token_منوال منوال) {
  انتخاب (منوال) {
  حالت Alloc_Token_منوال::Increment:
    واپس "increment";
  حالت Alloc_Token_منوال::Random:
    واپس "random";
  حالت Alloc_Token_منوال::TypeHash:
    واپس "typehash";
  حالت Alloc_Token_منوال::Type_Hash_اشارہ_گر_بانٹو:
    واپس "typehashpointersplit";
  حالت Alloc_Token_منوال::TypeFuncHash:
    واپس "typefunchash";
  حالت Alloc_Token_منوال::Type_Func_Hash_اشارہ_گر_بانٹو:
    واپس "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ساکن uint64_t getStableHash(ثابت AllocTokenMetadata &Metadata,
                              uint64_t Max_علامات) {
  واپس getStableSipHash(Metadata.TypeName) % Max_علامات;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ساکن uint64_t getTypeFuncHash(ثابت AllocTokenMetadata &Metadata,
                                uint64_t Max_علامات, منطقی اشارہ_گر_بانٹو) {
  اگر (Max_علامات == 1)
    واپس 0;
  ثابت غیرعلامتی بٹ_2 =
      is_نقاب_64(Max_علامات) ? llvm::countr_one(Max_علامات) : Log2_64(Max_علامات);
  ثابت غیرعلامتی Func_بٹ = بٹ_2 / 2;
  غیرعلامتی Type_بٹ = بٹ_2 - Func_بٹ;
  uint64_t Token = 0;
  اگر (اشارہ_گر_بانٹو) {
    --Type_بٹ;
    Token = uint64_t(Metadata.شامل_ہے_اشارہ_گر) << (بٹ_2 - 1);
  }
  // An empty type name denotes an unknown type.
  اگر (!Metadata.TypeName.خالی_خانے())
    Token |= (getStableSipHash(Metadata.TypeName) &
              نقاب_Trailing_Ones<uint64_t>(Type_بٹ))
             << Func_بٹ;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           نقاب_Trailing_Ones<uint64_t>(Func_بٹ);
  واپس Token;
}

std::optional<uint64_t> llvm::getAllocToken(Alloc_Token_منوال منوال,
                                            ثابت AllocTokenMetadata &Metadata,
                                            uint64_t Max_علامات) {
  أكد(Max_علامات && "Must provide non-zero max tokens");

  انتخاب (منوال) {
  حالت Alloc_Token_منوال::Increment:
  حالت Alloc_Token_منوال::Random:
    // Stateful modes cannot be implemented as a pure function.
    واپس std::nullopt;

  حالت Alloc_Token_منوال::TypeFuncHash:
  حالت Alloc_Token_منوال::Type_Func_Hash_اشارہ_گر_بانٹو:
    اگر (!Metadata.FunctionName)
      واپس std::nullopt;
    واپس getTypeFuncHash(Metadata, Max_علامات,
                           منوال == Alloc_Token_منوال::Type_Func_Hash_اشارہ_گر_بانٹو);

  حالت Alloc_Token_منوال::TypeHash:
    واپس getStableHash(Metadata, Max_علامات);

  حالت Alloc_Token_منوال::Type_Hash_اشارہ_گر_بانٹو: {
    اگر (Max_علامات == 1)
      واپس 0;
    ثابت uint64_t Half_علامات = Max_علامات / 2;
    uint64_t Hash = getStableHash(Metadata, Half_علامات);
    اگر (Metadata.شامل_ہے_اشارہ_گر)
      Hash += Half_علامات;
    واپس Hash;
  }
  }

  llvm_unreachable("");
}
