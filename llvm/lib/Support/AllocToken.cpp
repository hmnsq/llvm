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

using فضاینام llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  بازگشت StringSwitch<std::optional<AllocTokenMode>>(Name)
      .Case("increment", AllocTokenMode::Increment)
      .Case("random", AllocTokenMode::Random)
      .Case("typehash", AllocTokenMode::TypeHash)
      .Case("typehashpointersplit", AllocTokenMode::TypeHashPointerSplit)
      .Case("typefunchash", AllocTokenMode::TypeFuncHash)
      .Case("typefunchashpointersplit",
            AllocTokenMode::TypeFuncHashPointerSplit)
      .Case("default", DefaultAllocTokenMode)
      .Default(std::nullopt);
}

StringRef llvm::getAllocTokenModeAsString(AllocTokenMode Mode) {
  انتخاب (Mode) {
  حالت AllocTokenMode::Increment:
    بازگشت "increment";
  حالت AllocTokenMode::Random:
    بازگشت "random";
  حالت AllocTokenMode::TypeHash:
    بازگشت "typehash";
  حالت AllocTokenMode::TypeHashPointerSplit:
    بازگشت "typehashpointersplit";
  حالت AllocTokenMode::TypeFuncHash:
    بازگشت "typefunchash";
  حالت AllocTokenMode::TypeFuncHashPointerSplit:
    بازگشت "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ایستا uint64_t getStableHash(ثابت AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  بازگشت getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ایستا uint64_t getTypeFuncHash(ثابت AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, منطقی PointerSplit) {
  اگر (MaxTokens == 1)
    بازگشت 0;
  ثابت unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  ثابت unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  اگر (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  اگر (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  بازگشت Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            ثابت AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  أكد(MaxTokens && "Must provide non-zero max tokens");

  انتخاب (Mode) {
  حالت AllocTokenMode::Increment:
  حالت AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    بازگشت std::nullopt;

  حالت AllocTokenMode::TypeFuncHash:
  حالت AllocTokenMode::TypeFuncHashPointerSplit:
    اگر (!Metadata.FunctionName)
      بازگشت std::nullopt;
    بازگشت getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  حالت AllocTokenMode::TypeHash:
    بازگشت getStableHash(Metadata, MaxTokens);

  حالت AllocTokenMode::TypeHashPointerSplit: {
    اگر (MaxTokens == 1)
      بازگشت 0;
    ثابت uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    اگر (Metadata.ContainsPointer)
      Hash += HalfTokens;
    بازگشت Hash;
  }
  }

  llvm_unreachable("");
}
