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
  প্রত্যাগম StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  বিকল্প (Mode) {
  পক্ষ AllocTokenMode::Increment:
    প্রত্যাগম "increment";
  পক্ষ AllocTokenMode::Random:
    প্রত্যাগম "random";
  পক্ষ AllocTokenMode::TypeHash:
    প্রত্যাগম "typehash";
  পক্ষ AllocTokenMode::TypeHashPointerSplit:
    প্রত্যাগম "typehashpointersplit";
  পক্ষ AllocTokenMode::TypeFuncHash:
    প্রত্যাগম "typefunchash";
  পক্ষ AllocTokenMode::TypeFuncHashPointerSplit:
    প্রত্যাগম "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

স্থির uint64_t getStableHash(ধ্রুব AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  প্রত্যাগম getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
স্থির uint64_t getTypeFuncHash(ধ্রুব AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, তার্কিক PointerSplit) {
  যদি (MaxTokens == 1)
    প্রত্যাগম 0;
  ধ্রুব unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  ধ্রুব unsigned FuncBits = Bits / 2;
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
  প্রত্যাগম Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            ধ্রুব AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  প্রতিজ্ঞা(MaxTokens && "Must provide non-zero max tokens");

  বিকল্প (Mode) {
  পক্ষ AllocTokenMode::Increment:
  পক্ষ AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    প্রত্যাগম std::nullopt;

  পক্ষ AllocTokenMode::TypeFuncHash:
  পক্ষ AllocTokenMode::TypeFuncHashPointerSplit:
    যদি (!Metadata.FunctionName)
      প্রত্যাগম std::nullopt;
    প্রত্যাগম getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  পক্ষ AllocTokenMode::TypeHash:
    প্রত্যাগম getStableHash(Metadata, MaxTokens);

  পক্ষ AllocTokenMode::TypeHashPointerSplit: {
    যদি (MaxTokens == 1)
      প্রত্যাগম 0;
    ধ্রুব uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    যদি (Metadata.ContainsPointer)
      Hash += HalfTokens;
    প্রত্যাগম Hash;
  }
  }

  llvm_unreachable("");
}
