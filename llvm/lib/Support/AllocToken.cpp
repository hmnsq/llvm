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

#অন্তর্ভাব "llvm/Support/AllocToken.h"
#অন্তর্ভাব "llvm/ADT/StringSwitch.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/SipHash.h"

using নামাকাশ llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  ফেরত StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  নির্বাচন (Mode) {
  ক্ষেত্র AllocTokenMode::Increment:
    ফেরত "increment";
  ক্ষেত্র AllocTokenMode::Random:
    ফেরত "random";
  ক্ষেত্র AllocTokenMode::TypeHash:
    ফেরত "typehash";
  ক্ষেত্র AllocTokenMode::TypeHashPointerSplit:
    ফেরত "typehashpointersplit";
  ক্ষেত্র AllocTokenMode::TypeFuncHash:
    ফেরত "typefunchash";
  ক্ষেত্র AllocTokenMode::TypeFuncHashPointerSplit:
    ফেরত "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

স্থির uint64_t getStableHash(ধ্রুবক AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  ফেরত getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
স্থির uint64_t getTypeFuncHash(ধ্রুবক AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, যৌক্তিক PointerSplit) {
  যদি (MaxTokens == 1)
    ফেরত 0;
  ধ্রুবক unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  ধ্রুবক unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  যদি (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  যদি (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  ফেরত Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            ধ্রুবক AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  প্রতিজ্ঞা(MaxTokens && "Must provide non-zero max tokens");

  নির্বাচন (Mode) {
  ক্ষেত্র AllocTokenMode::Increment:
  ক্ষেত্র AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    ফেরত std::nullopt;

  ক্ষেত্র AllocTokenMode::TypeFuncHash:
  ক্ষেত্র AllocTokenMode::TypeFuncHashPointerSplit:
    যদি (!Metadata.FunctionName)
      ফেরত std::nullopt;
    ফেরত getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  ক্ষেত্র AllocTokenMode::TypeHash:
    ফেরত getStableHash(Metadata, MaxTokens);

  ক্ষেত্র AllocTokenMode::TypeHashPointerSplit: {
    যদি (MaxTokens == 1)
      ফেরত 0;
    ধ্রুবক uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    যদি (Metadata.ContainsPointer)
      Hash += HalfTokens;
    ফেরত Hash;
  }
  }

  llvm_unreachable("");
}
