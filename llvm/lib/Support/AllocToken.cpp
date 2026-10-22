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
  परत StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  निवड (Mode) {
  स्थिती AllocTokenMode::Increment:
    परत "increment";
  स्थिती AllocTokenMode::Random:
    परत "random";
  स्थिती AllocTokenMode::TypeHash:
    परत "typehash";
  स्थिती AllocTokenMode::TypeHashPointerSplit:
    परत "typehashpointersplit";
  स्थिती AllocTokenMode::TypeFuncHash:
    परत "typefunchash";
  स्थिती AllocTokenMode::TypeFuncHashPointerSplit:
    परत "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

स्थिर uint64_t getStableHash(स्थिरांक AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  परत getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
स्थिर uint64_t getTypeFuncHash(स्थिरांक AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, तार्किक PointerSplit) {
  जर (MaxTokens == 1)
    परत 0;
  स्थिरांक अचिन्हित Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  स्थिरांक अचिन्हित FuncBits = Bits / 2;
  अचिन्हित TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  जर (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  जर (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  परत Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            स्थिरांक AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  प्रतिज्ञा(MaxTokens && "Must provide non-zero max tokens");

  निवड (Mode) {
  स्थिती AllocTokenMode::Increment:
  स्थिती AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    परत std::nullopt;

  स्थिती AllocTokenMode::TypeFuncHash:
  स्थिती AllocTokenMode::TypeFuncHashPointerSplit:
    जर (!Metadata.FunctionName)
      परत std::nullopt;
    परत getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  स्थिती AllocTokenMode::TypeHash:
    परत getStableHash(Metadata, MaxTokens);

  स्थिती AllocTokenMode::TypeHashPointerSplit: {
    जर (MaxTokens == 1)
      परत 0;
    स्थिरांक uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    जर (Metadata.ContainsPointer)
      Hash += HalfTokens;
    परत Hash;
  }
  }

  llvm_unreachable("");
}
