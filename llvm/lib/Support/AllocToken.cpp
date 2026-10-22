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

استعمال نطاق llvm;

std::optional<Alloc_Token_گهڻائي>
llvm::get_Alloc_Token_گهڻائي_From_String(StringRef Name) {
  أرجع StringSwitch<std::optional<Alloc_Token_گهڻائي>>(Name)
      .Case("increment", Alloc_Token_گهڻائي::Increment)
      .Case("random", Alloc_Token_گهڻائي::Random)
      .Case("typehash", Alloc_Token_گهڻائي::TypeHash)
      .Case("typehashpointersplit", Alloc_Token_گهڻائي::Type_Hash_اشارو_ورهايو)
      .Case("typefunchash", Alloc_Token_گهڻائي::TypeFuncHash)
      .Case("typefunchashpointersplit",
            Alloc_Token_گهڻائي::Type_Func_Hash_اشارو_ورهايو)
      .Case("default", Default_Alloc_Token_گهڻائي)
      .Default(std::nullopt);
}

StringRef llvm::get_Alloc_Token_گهڻائي_As_String(Alloc_Token_گهڻائي گهڻائي) {
  بدل (گهڻائي) {
  حالة Alloc_Token_گهڻائي::Increment:
    أرجع "increment";
  حالة Alloc_Token_گهڻائي::Random:
    أرجع "random";
  حالة Alloc_Token_گهڻائي::TypeHash:
    أرجع "typehash";
  حالة Alloc_Token_گهڻائي::Type_Hash_اشارو_ورهايو:
    أرجع "typehashpointersplit";
  حالة Alloc_Token_گهڻائي::TypeFuncHash:
    أرجع "typefunchash";
  حالة Alloc_Token_گهڻائي::Type_Func_Hash_اشارو_ورهايو:
    أرجع "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ساكن uint64_t getStableHash(ثابت AllocTokenMetadata &Metadata,
                              uint64_t Max_ٽوڪن) {
  أرجع getStableSipHash(Metadata.TypeName) % Max_ٽوڪن;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ساكن uint64_t getTypeFuncHash(ثابت AllocTokenMetadata &Metadata,
                                uint64_t Max_ٽوڪن, منطقي اشارو_ورهايو) {
  إذا (Max_ٽوڪن == 1)
    أرجع 0;
  ثابت بنانشان بٽ_2 =
      is_نقاب_64(Max_ٽوڪن) ? llvm::countr_one(Max_ٽوڪن) : Log2_64(Max_ٽوڪن);
  ثابت بنانشان Func_بٽ = بٽ_2 / 2;
  بنانشان Type_بٽ = بٽ_2 - Func_بٽ;
  uint64_t Token = 0;
  إذا (اشارو_ورهايو) {
    --Type_بٽ;
    Token = uint64_t(Metadata.شامل_آهي_اشارو) << (بٽ_2 - 1);
  }
  // An empty type name denotes an unknown type.
  إذا (!Metadata.TypeName.خالي_جايون())
    Token |= (getStableSipHash(Metadata.TypeName) &
              نقاب_Trailing_Ones<uint64_t>(Type_بٽ))
             << Func_بٽ;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           نقاب_Trailing_Ones<uint64_t>(Func_بٽ);
  أرجع Token;
}

std::optional<uint64_t> llvm::getAllocToken(Alloc_Token_گهڻائي گهڻائي,
                                            ثابت AllocTokenMetadata &Metadata,
                                            uint64_t Max_ٽوڪن) {
  أكد(Max_ٽوڪن && "Must provide non-zero max tokens");

  بدل (گهڻائي) {
  حالة Alloc_Token_گهڻائي::Increment:
  حالة Alloc_Token_گهڻائي::Random:
    // Stateful modes cannot be implemented as a pure function.
    أرجع std::nullopt;

  حالة Alloc_Token_گهڻائي::TypeFuncHash:
  حالة Alloc_Token_گهڻائي::Type_Func_Hash_اشارو_ورهايو:
    إذا (!Metadata.FunctionName)
      أرجع std::nullopt;
    أرجع getTypeFuncHash(Metadata, Max_ٽوڪن,
                           گهڻائي == Alloc_Token_گهڻائي::Type_Func_Hash_اشارو_ورهايو);

  حالة Alloc_Token_گهڻائي::TypeHash:
    أرجع getStableHash(Metadata, Max_ٽوڪن);

  حالة Alloc_Token_گهڻائي::Type_Hash_اشارو_ورهايو: {
    إذا (Max_ٽوڪن == 1)
      أرجع 0;
    ثابت uint64_t Half_ٽوڪن = Max_ٽوڪن / 2;
    uint64_t Hash = getStableHash(Metadata, Half_ٽوڪن);
    إذا (Metadata.شامل_آهي_اشارو)
      Hash += Half_ٽوڪن;
    أرجع Hash;
  }
  }

  llvm_unreachable("");
}
