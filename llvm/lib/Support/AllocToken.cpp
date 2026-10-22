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

استفاده نطاق llvm;

std::optional<Alloc_Token_نما>
llvm::get_Alloc_Token_نما_From_String(StringRef Name) {
  أرجع StringSwitch<std::optional<Alloc_Token_نما>>(Name)
      .Case("increment", Alloc_Token_نما::Increment)
      .Case("random", Alloc_Token_نما::Random)
      .Case("typehash", Alloc_Token_نما::TypeHash)
      .Case("typehashpointersplit", Alloc_Token_نما::Type_Hash_اشاره_گر_تقسیم)
      .Case("typefunchash", Alloc_Token_نما::TypeFuncHash)
      .Case("typefunchashpointersplit",
            Alloc_Token_نما::Type_Func_Hash_اشاره_گر_تقسیم)
      .Case("default", Default_Alloc_Token_نما)
      .Default(std::nullopt);
}

StringRef llvm::get_Alloc_Token_نما_As_String(Alloc_Token_نما نما) {
  بدل (نما) {
  حالة Alloc_Token_نما::Increment:
    أرجع "increment";
  حالة Alloc_Token_نما::Random:
    أرجع "random";
  حالة Alloc_Token_نما::TypeHash:
    أرجع "typehash";
  حالة Alloc_Token_نما::Type_Hash_اشاره_گر_تقسیم:
    أرجع "typehashpointersplit";
  حالة Alloc_Token_نما::TypeFuncHash:
    أرجع "typefunchash";
  حالة Alloc_Token_نما::Type_Func_Hash_اشاره_گر_تقسیم:
    أرجع "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ساكن uint64_t getStableHash(ثابت AllocTokenMetadata &Metadata,
                              uint64_t Max_نشانهها) {
  أرجع getStableSipHash(Metadata.TypeName) % Max_نشانهها;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ساكن uint64_t getTypeFuncHash(ثابت AllocTokenMetadata &Metadata,
                                uint64_t Max_نشانهها, منطقي اشاره_گر_تقسیم) {
  إذا (Max_نشانهها == 1)
    أرجع 0;
  ثابت بیعلامت بیتها_2 =
      is_ماسک_64(Max_نشانهها) ? llvm::countr_one(Max_نشانهها) : Log2_64(Max_نشانهها);
  ثابت بیعلامت Func_بیتها = بیتها_2 / 2;
  بیعلامت Type_بیتها = بیتها_2 - Func_بیتها;
  uint64_t Token = 0;
  إذا (اشاره_گر_تقسیم) {
    --Type_بیتها;
    Token = uint64_t(Metadata.شامل_است_اشاره_گر) << (بیتها_2 - 1);
  }
  // An empty type name denotes an unknown type.
  إذا (!Metadata.TypeName.خالی())
    Token |= (getStableSipHash(Metadata.TypeName) &
              ماسک_Trailing_Ones<uint64_t>(Type_بیتها))
             << Func_بیتها;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           ماسک_Trailing_Ones<uint64_t>(Func_بیتها);
  أرجع Token;
}

std::optional<uint64_t> llvm::getAllocToken(Alloc_Token_نما نما,
                                            ثابت AllocTokenMetadata &Metadata,
                                            uint64_t Max_نشانهها) {
  أكد(Max_نشانهها && "Must provide non-zero max tokens");

  بدل (نما) {
  حالة Alloc_Token_نما::Increment:
  حالة Alloc_Token_نما::Random:
    // Stateful modes cannot be implemented as a pure function.
    أرجع std::nullopt;

  حالة Alloc_Token_نما::TypeFuncHash:
  حالة Alloc_Token_نما::Type_Func_Hash_اشاره_گر_تقسیم:
    إذا (!Metadata.FunctionName)
      أرجع std::nullopt;
    أرجع getTypeFuncHash(Metadata, Max_نشانهها,
                           نما == Alloc_Token_نما::Type_Func_Hash_اشاره_گر_تقسیم);

  حالة Alloc_Token_نما::TypeHash:
    أرجع getStableHash(Metadata, Max_نشانهها);

  حالة Alloc_Token_نما::Type_Hash_اشاره_گر_تقسیم: {
    إذا (Max_نشانهها == 1)
      أرجع 0;
    ثابت uint64_t Half_نشانهها = Max_نشانهها / 2;
    uint64_t Hash = getStableHash(Metadata, Half_نشانهها);
    إذا (Metadata.شامل_است_اشاره_گر)
      Hash += Half_نشانهها;
    أرجع Hash;
  }
  }

  llvm_unreachable("");
}
