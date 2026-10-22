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

#अन्तर्भाव "llvm/Support/AllocToken.h"
#अन्तर्भाव "llvm/ADT/StringSwitch.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/SipHash.h"

using नामाकाश llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  पटिनिवत्त StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  विकल्प (Mode) {
  पक्ष AllocTokenMode::Increment:
    पटिनिवत्त "increment";
  पक्ष AllocTokenMode::Random:
    पटिनिवत्त "random";
  पक्ष AllocTokenMode::TypeHash:
    पटिनिवत्त "typehash";
  पक्ष AllocTokenMode::TypeHashPointerSplit:
    पटिनिवत्त "typehashpointersplit";
  पक्ष AllocTokenMode::TypeFuncHash:
    पटिनिवत्त "typefunchash";
  पक्ष AllocTokenMode::TypeFuncHashPointerSplit:
    पटिनिवत्त "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

स्थिर uint64_t getStableHash(ध्रुव AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  पटिनिवत्त getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
स्थिर uint64_t getTypeFuncHash(ध्रुव AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, तार्किक PointerSplit) {
  सचे (MaxTokens == 1)
    पटिनिवत्त 0;
  ध्रुव unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  ध्रुव unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  सचे (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  सचे (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  पटिनिवत्त Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            ध्रुव AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  प्रतिज्ञा(MaxTokens && "Must provide non-zero max tokens");

  विकल्प (Mode) {
  पक्ष AllocTokenMode::Increment:
  पक्ष AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    पटिनिवत्त std::nullopt;

  पक्ष AllocTokenMode::TypeFuncHash:
  पक्ष AllocTokenMode::TypeFuncHashPointerSplit:
    सचे (!Metadata.FunctionName)
      पटिनिवत्त std::nullopt;
    पटिनिवत्त getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  पक्ष AllocTokenMode::TypeHash:
    पटिनिवत्त getStableHash(Metadata, MaxTokens);

  पक्ष AllocTokenMode::TypeHashPointerSplit: {
    सचे (MaxTokens == 1)
      पटिनिवत्त 0;
    ध्रुव uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    सचे (Metadata.ContainsPointer)
      Hash += HalfTokens;
    पटिनिवत्त Hash;
  }
  }

  llvm_unreachable("");
}
