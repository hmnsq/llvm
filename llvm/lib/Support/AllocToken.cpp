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

ورتوں نطاق llvm;

std::optional<Alloc_Token_منوال>
llvm::get_Alloc_Token_منوال_From_String(StringRef Name) {
  أرجع StringSwitch<std::optional<Alloc_Token_منوال>>(Name)
      .Case("increment", Alloc_Token_منوال::Increment)
      .Case("random", Alloc_Token_منوال::Random)
      .Case("typehash", Alloc_Token_منوال::TypeHash)
      .Case("typehashpointersplit", Alloc_Token_منوال::Type_Hash_اشارہ_گر_ونڈو)
      .Case("typefunchash", Alloc_Token_منوال::TypeFuncHash)
      .Case("typefunchashpointersplit",
            Alloc_Token_منوال::Type_Func_Hash_اشارہ_گر_ونڈو)
      .Case("default", Default_Alloc_Token_منوال)
      .Default(std::nullopt);
}

StringRef llvm::get_Alloc_Token_منوال_As_String(Alloc_Token_منوال منوال) {
  بدل (منوال) {
  حالة Alloc_Token_منوال::Increment:
    أرجع "increment";
  حالة Alloc_Token_منوال::Random:
    أرجع "random";
  حالة Alloc_Token_منوال::TypeHash:
    أرجع "typehash";
  حالة Alloc_Token_منوال::Type_Hash_اشارہ_گر_ونڈو:
    أرجع "typehashpointersplit";
  حالة Alloc_Token_منوال::TypeFuncHash:
    أرجع "typefunchash";
  حالة Alloc_Token_منوال::Type_Func_Hash_اشارہ_گر_ونڈو:
    أرجع "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ساكن uint64_t getStableHash(ثابت AllocTokenMetadata &Metadata,
                              uint64_t Max_ٹوکن) {
  أرجع getStableSipHash(Metadata.TypeName) % Max_ٹوکن;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ساكن uint64_t getTypeFuncHash(ثابت AllocTokenMetadata &Metadata,
                                uint64_t Max_ٹوکن, منطقي اشارہ_گر_ونڈو) {
  إذا (Max_ٹوکن == 1)
    أرجع 0;
  ثابت بنانشانی بٹ_2 =
      is_نقاب_64(Max_ٹوکن) ? llvm::countr_one(Max_ٹوکن) : Log2_64(Max_ٹوکن);
  ثابت بنانشانی Func_بٹ = بٹ_2 / 2;
  بنانشانی Type_بٹ = بٹ_2 - Func_بٹ;
  uint64_t Token = 0;
  إذا (اشارہ_گر_ونڈو) {
    --Type_بٹ;
    Token = uint64_t(Metadata.شامل_اے_اشارہ_گر) << (بٹ_2 - 1);
  }
  // An empty type name denotes an unknown type.
  إذا (!Metadata.TypeName.خالی_تھاواں())
    Token |= (getStableSipHash(Metadata.TypeName) &
              نقاب_Trailing_Ones<uint64_t>(Type_بٹ))
             << Func_بٹ;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           نقاب_Trailing_Ones<uint64_t>(Func_بٹ);
  أرجع Token;
}

std::optional<uint64_t> llvm::getAllocToken(Alloc_Token_منوال منوال,
                                            ثابت AllocTokenMetadata &Metadata,
                                            uint64_t Max_ٹوکن) {
  أكد(Max_ٹوکن && "Must provide non-zero max tokens");

  بدل (منوال) {
  حالة Alloc_Token_منوال::Increment:
  حالة Alloc_Token_منوال::Random:
    // Stateful modes cannot be implemented as a pure function.
    أرجع std::nullopt;

  حالة Alloc_Token_منوال::TypeFuncHash:
  حالة Alloc_Token_منوال::Type_Func_Hash_اشارہ_گر_ونڈو:
    إذا (!Metadata.FunctionName)
      أرجع std::nullopt;
    أرجع getTypeFuncHash(Metadata, Max_ٹوکن,
                           منوال == Alloc_Token_منوال::Type_Func_Hash_اشارہ_گر_ونڈو);

  حالة Alloc_Token_منوال::TypeHash:
    أرجع getStableHash(Metadata, Max_ٹوکن);

  حالة Alloc_Token_منوال::Type_Hash_اشارہ_گر_ونڈو: {
    إذا (Max_ٹوکن == 1)
      أرجع 0;
    ثابت uint64_t Half_ٹوکن = Max_ٹوکن / 2;
    uint64_t Hash = getStableHash(Metadata, Half_ٹوکن);
    إذا (Metadata.شامل_اے_اشارہ_گر)
      Hash += Half_ٹوکن;
    أرجع Hash;
  }
  }

  llvm_unreachable("");
}
