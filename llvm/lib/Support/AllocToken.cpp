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

#ضمن "llvm/Support/AllocToken.h"
#ضمن "llvm/ADT/StringSwitch.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/SipHash.h"

کارول نطاق llvm;

std::optional<Alloc_Token_ډېرپېښ>
llvm::get_Alloc_Token_ډېرپېښ_From_String(StringRef Name) {
  أرجع StringSwitch<std::optional<Alloc_Token_ډېرپېښ>>(Name)
      .Case("increment", Alloc_Token_ډېرپېښ::Increment)
      .Case("random", Alloc_Token_ډېرپېښ::Random)
      .Case("typehash", Alloc_Token_ډېرپېښ::TypeHash)
      .Case("typehashpointersplit", Alloc_Token_ډېرپېښ::Type_Hash_اشاره_ګر_وېشل)
      .Case("typefunchash", Alloc_Token_ډېرپېښ::TypeFuncHash)
      .Case("typefunchashpointersplit",
            Alloc_Token_ډېرپېښ::Type_Func_Hash_اشاره_ګر_وېشل)
      .Case("default", Default_Alloc_Token_ډېرپېښ)
      .Default(std::nullopt);
}

StringRef llvm::get_Alloc_Token_ډېرپېښ_As_String(Alloc_Token_ډېرپېښ ډېرپېښ) {
  بدل (ډېرپېښ) {
  حالة Alloc_Token_ډېرپېښ::Increment:
    أرجع "increment";
  حالة Alloc_Token_ډېرپېښ::Random:
    أرجع "random";
  حالة Alloc_Token_ډېرپېښ::TypeHash:
    أرجع "typehash";
  حالة Alloc_Token_ډېرپېښ::Type_Hash_اشاره_ګر_وېشل:
    أرجع "typehashpointersplit";
  حالة Alloc_Token_ډېرپېښ::TypeFuncHash:
    أرجع "typefunchash";
  حالة Alloc_Token_ډېرپېښ::Type_Func_Hash_اشاره_ګر_وېشل:
    أرجع "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ساكن uint64_t getStableHash(ثابت AllocTokenMetadata &Metadata,
                              uint64_t Max_نښې) {
  أرجع getStableSipHash(Metadata.TypeName) % Max_نښې;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ساكن uint64_t getTypeFuncHash(ثابت AllocTokenMetadata &Metadata,
                                uint64_t Max_نښې, منطقي اشاره_ګر_وېشل) {
  إذا (Max_نښې == 1)
    أرجع 0;
  ثابت بینښه بټونه_2 =
      is_ماسک_64(Max_نښې) ? llvm::countr_one(Max_نښې) : Log2_64(Max_نښې);
  ثابت بینښه Func_بټونه = بټونه_2 / 2;
  بینښه Type_بټونه = بټونه_2 - Func_بټونه;
  uint64_t Token = 0;
  إذا (اشاره_ګر_وېشل) {
    --Type_بټونه;
    Token = uint64_t(Metadata.لري_اشاره_ګر) << (بټونه_2 - 1);
  }
  // An empty type name denotes an unknown type.
  إذا (!Metadata.TypeName.تشه())
    Token |= (getStableSipHash(Metadata.TypeName) &
              ماسک_Trailing_Ones<uint64_t>(Type_بټونه))
             << Func_بټونه;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           ماسک_Trailing_Ones<uint64_t>(Func_بټونه);
  أرجع Token;
}

std::optional<uint64_t> llvm::getAllocToken(Alloc_Token_ډېرپېښ ډېرپېښ,
                                            ثابت AllocTokenMetadata &Metadata,
                                            uint64_t Max_نښې) {
  أكد(Max_نښې && "Must provide non-zero max tokens");

  بدل (ډېرپېښ) {
  حالة Alloc_Token_ډېرپېښ::Increment:
  حالة Alloc_Token_ډېرپېښ::Random:
    // Stateful modes cannot be implemented as a pure function.
    أرجع std::nullopt;

  حالة Alloc_Token_ډېرپېښ::TypeFuncHash:
  حالة Alloc_Token_ډېرپېښ::Type_Func_Hash_اشاره_ګر_وېشل:
    إذا (!Metadata.FunctionName)
      أرجع std::nullopt;
    أرجع getTypeFuncHash(Metadata, Max_نښې,
                           ډېرپېښ == Alloc_Token_ډېرپېښ::Type_Func_Hash_اشاره_ګر_وېشل);

  حالة Alloc_Token_ډېرپېښ::TypeHash:
    أرجع getStableHash(Metadata, Max_نښې);

  حالة Alloc_Token_ډېرپېښ::Type_Hash_اشاره_ګر_وېشل: {
    إذا (Max_نښې == 1)
      أرجع 0;
    ثابت uint64_t Half_نښې = Max_نښې / 2;
    uint64_t Hash = getStableHash(Metadata, Half_نښې);
    إذا (Metadata.لري_اشاره_ګر)
      Hash += Half_نښې;
    أرجع Hash;
  }
  }

  llvm_unreachable("");
}
