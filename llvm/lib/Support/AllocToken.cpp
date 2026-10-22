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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/AllocToken.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringSwitch.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SipHash.h"

using ନାମାକାଶ llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  ଫେରନ୍ତୁ StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  ବାଛ (Mode) {
  ସ୍ଥିତି AllocTokenMode::Increment:
    ଫେରନ୍ତୁ "increment";
  ସ୍ଥିତି AllocTokenMode::Random:
    ଫେରନ୍ତୁ "random";
  ସ୍ଥିତି AllocTokenMode::TypeHash:
    ଫେରନ୍ତୁ "typehash";
  ସ୍ଥିତି AllocTokenMode::TypeHashPointerSplit:
    ଫେରନ୍ତୁ "typehashpointersplit";
  ସ୍ଥିତି AllocTokenMode::TypeFuncHash:
    ଫେରନ୍ତୁ "typefunchash";
  ସ୍ଥିତି AllocTokenMode::TypeFuncHashPointerSplit:
    ଫେରନ୍ତୁ "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ସ୍ଥିର uint64_t getStableHash(ଅଚଳ AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  ଫେରନ୍ତୁ getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ସ୍ଥିର uint64_t getTypeFuncHash(ଅଚଳ AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, ତାର୍କିକ PointerSplit) {
  ଯଦି (MaxTokens == 1)
    ଫେରନ୍ତୁ 0;
  ଅଚଳ unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  ଅଚଳ unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  ଯଦି (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  ଯଦି (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  ଫେରନ୍ତୁ Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            ଅଚଳ AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  ପ୍ରତିଜ୍ଞା(MaxTokens && "Must provide non-zero max tokens");

  ବାଛ (Mode) {
  ସ୍ଥିତି AllocTokenMode::Increment:
  ସ୍ଥିତି AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    ଫେରନ୍ତୁ std::nullopt;

  ସ୍ଥିତି AllocTokenMode::TypeFuncHash:
  ସ୍ଥିତି AllocTokenMode::TypeFuncHashPointerSplit:
    ଯଦି (!Metadata.FunctionName)
      ଫେରନ୍ତୁ std::nullopt;
    ଫେରନ୍ତୁ getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  ସ୍ଥିତି AllocTokenMode::TypeHash:
    ଫେରନ୍ତୁ getStableHash(Metadata, MaxTokens);

  ସ୍ଥିତି AllocTokenMode::TypeHashPointerSplit: {
    ଯଦି (MaxTokens == 1)
      ଫେରନ୍ତୁ 0;
    ଅଚଳ uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    ଯଦି (Metadata.ContainsPointer)
      Hash += HalfTokens;
    ଫେରନ୍ତୁ Hash;
  }
  }

  llvm_unreachable("");
}
