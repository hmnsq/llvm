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
  फर्काऊ StringSwitch<std::optional<AllocTokenMode>>(Name)
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
    फर्काऊ "increment";
  पक्ष AllocTokenMode::Random:
    फर्काऊ "random";
  पक्ष AllocTokenMode::TypeHash:
    फर्काऊ "typehash";
  पक्ष AllocTokenMode::TypeHashPointerSplit:
    फर्काऊ "typehashpointersplit";
  पक्ष AllocTokenMode::TypeFuncHash:
    फर्काऊ "typefunchash";
  पक्ष AllocTokenMode::TypeFuncHashPointerSplit:
    फर्काऊ "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

स्थिर uint64_t getStableHash(अचर AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  फर्काऊ getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
स्थिर uint64_t getTypeFuncHash(अचर AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, तार्किक PointerSplit) {
  यदि (MaxTokens == 1)
    फर्काऊ 0;
  अचर अचिन्हित Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  अचर अचिन्हित FuncBits = Bits / 2;
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
  फर्काऊ Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            अचर AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  प्रतिज्ञा(MaxTokens && "Must provide non-zero max tokens");

  विकल्प (Mode) {
  पक्ष AllocTokenMode::Increment:
  पक्ष AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    फर्काऊ std::nullopt;

  पक्ष AllocTokenMode::TypeFuncHash:
  पक्ष AllocTokenMode::TypeFuncHashPointerSplit:
    यदि (!Metadata.FunctionName)
      फर्काऊ std::nullopt;
    फर्काऊ getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  पक्ष AllocTokenMode::TypeHash:
    फर्काऊ getStableHash(Metadata, MaxTokens);

  पक्ष AllocTokenMode::TypeHashPointerSplit: {
    यदि (MaxTokens == 1)
      फर्काऊ 0;
    अचर uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    यदि (Metadata.ContainsPointer)
      Hash += HalfTokens;
    फर्काऊ Hash;
  }
  }

  llvm_unreachable("");
}
