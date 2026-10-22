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

#include "llvm/Support/AllocToken.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SipHash.h"

using namespace llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  ᬯᬮᬶ StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  switch (Mode) {
  pak_sa AllocTokenMode::Increment:
    ᬯᬮᬶ "increment";
  pak_sa AllocTokenMode::Random:
    ᬯᬮᬶ "random";
  pak_sa AllocTokenMode::TypeHash:
    ᬯᬮᬶ "typehash";
  pak_sa AllocTokenMode::TypeHashPointerSplit:
    ᬯᬮᬶ "typehashpointersplit";
  pak_sa AllocTokenMode::TypeFuncHash:
    ᬯᬮᬶ "typefunchash";
  pak_sa AllocTokenMode::TypeFuncHashPointerSplit:
    ᬯᬮᬶ "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

static uint64_t getStableHash(const AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  ᬯᬮᬶ getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
static uint64_t getTypeFuncHash(const AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, bool PointerSplit) {
  ᬬᬾᬦ᭄ (MaxTokens == 1)
    ᬯᬮᬶ 0;
  const unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  const unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  ᬬᬾᬦ᭄ (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  ᬬᬾᬦ᭄ (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  ᬯᬮᬶ Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            const AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  assert(MaxTokens && "Must provide non-zero max tokens");

  switch (Mode) {
  pak_sa AllocTokenMode::Increment:
  pak_sa AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    ᬯᬮᬶ std::nullopt;

  pak_sa AllocTokenMode::TypeFuncHash:
  pak_sa AllocTokenMode::TypeFuncHashPointerSplit:
    ᬬᬾᬦ᭄ (!Metadata.FunctionName)
      ᬯᬮᬶ std::nullopt;
    ᬯᬮᬶ getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  pak_sa AllocTokenMode::TypeHash:
    ᬯᬮᬶ getStableHash(Metadata, MaxTokens);

  pak_sa AllocTokenMode::TypeHashPointerSplit: {
    ᬬᬾᬦ᭄ (MaxTokens == 1)
      ᬯᬮᬶ 0;
    const uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    ᬬᬾᬦ᭄ (Metadata.ContainsPointer)
      Hash += HalfTokens;
    ᬯᬮᬶ Hash;
  }
  }

  llvm_unreachable("");
}
