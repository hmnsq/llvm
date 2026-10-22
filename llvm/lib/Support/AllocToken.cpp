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
  प्रत्यागम StringSwitch<std::optional<AllocTokenMode>>(Name)
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
    प्रत्यागम "increment";
  पक्ष AllocTokenMode::Random:
    प्रत्यागम "random";
  पक्ष AllocTokenMode::TypeHash:
    प्रत्यागम "typehash";
  पक्ष AllocTokenMode::TypeHashPointerSplit:
    प्रत्यागम "typehashpointersplit";
  पक्ष AllocTokenMode::TypeFuncHash:
    प्रत्यागम "typefunchash";
  पक्ष AllocTokenMode::TypeFuncHashPointerSplit:
    प्रत्यागम "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

स्थिर uint64_t getStableHash(ध्रुव AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  प्रत्यागम getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
स्थिर uint64_t getTypeFuncHash(ध्रुव AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, तार्किक PointerSplit) {
  यदि (MaxTokens == 1)
    प्रत्यागम 0;
  ध्रुव अचिन्हित Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  ध्रुव अचिन्हित FuncBits = Bits / 2;
  अचिन्हित TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  यदि (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  यदि (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  प्रत्यागम Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            ध्रुव AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  प्रतिज्ञा(MaxTokens && "Must provide non-zero max tokens");

  विकल्प (Mode) {
  पक्ष AllocTokenMode::Increment:
  पक्ष AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    प्रत्यागम std::nullopt;

  पक्ष AllocTokenMode::TypeFuncHash:
  पक्ष AllocTokenMode::TypeFuncHashPointerSplit:
    यदि (!Metadata.FunctionName)
      प्रत्यागम std::nullopt;
    प्रत्यागम getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  पक्ष AllocTokenMode::TypeHash:
    प्रत्यागम getStableHash(Metadata, MaxTokens);

  पक्ष AllocTokenMode::TypeHashPointerSplit: {
    यदि (MaxTokens == 1)
      प्रत्यागम 0;
    ध्रुव uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    यदि (Metadata.ContainsPointer)
      Hash += HalfTokens;
    प्रत्यागम Hash;
  }
  }

  llvm_unreachable("");
}
