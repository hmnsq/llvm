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

#અન્તર્ભાવ "llvm/Support/AllocToken.h"
#અન્તર્ભાવ "llvm/ADT/StringSwitch.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/MathExtras.h"
#અન્તર્ભાવ "llvm/Support/SipHash.h"

using નામાકાશ llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  પાછા StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  પસંદગી (Mode) {
  સ્થિતિ AllocTokenMode::Increment:
    પાછા "increment";
  સ્થિતિ AllocTokenMode::Random:
    પાછા "random";
  સ્થિતિ AllocTokenMode::TypeHash:
    પાછા "typehash";
  સ્થિતિ AllocTokenMode::TypeHashPointerSplit:
    પાછા "typehashpointersplit";
  સ્થિતિ AllocTokenMode::TypeFuncHash:
    પાછા "typefunchash";
  સ્થિતિ AllocTokenMode::TypeFuncHashPointerSplit:
    પાછા "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

સ્થિર uint64_t getStableHash(અચલ AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  પાછા getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
સ્થિર uint64_t getTypeFuncHash(અચલ AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, તાર્કિક PointerSplit) {
  જો (MaxTokens == 1)
    પાછા 0;
  અચલ unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  અચલ unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  જો (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  જો (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  પાછા Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            અચલ AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  પ્રતિજ્ઞા(MaxTokens && "Must provide non-zero max tokens");

  પસંદગી (Mode) {
  સ્થિતિ AllocTokenMode::Increment:
  સ્થિતિ AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    પાછા std::nullopt;

  સ્થિતિ AllocTokenMode::TypeFuncHash:
  સ્થિતિ AllocTokenMode::TypeFuncHashPointerSplit:
    જો (!Metadata.FunctionName)
      પાછા std::nullopt;
    પાછા getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  સ્થિતિ AllocTokenMode::TypeHash:
    પાછા getStableHash(Metadata, MaxTokens);

  સ્થિતિ AllocTokenMode::TypeHashPointerSplit: {
    જો (MaxTokens == 1)
      પાછા 0;
    અચલ uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    જો (Metadata.ContainsPointer)
      Hash += HalfTokens;
    પાછા Hash;
  }
  }

  llvm_unreachable("");
}
