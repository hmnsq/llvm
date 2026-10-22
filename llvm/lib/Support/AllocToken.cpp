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

#ضمن "llvm/Support/AllocToken.h"
#ضمن "llvm/ADT/StringSwitch.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/SipHash.h"

using نطاق llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  החזר StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  بدل (Mode) {
  حالة AllocTokenMode::Increment:
    החזר "increment";
  حالة AllocTokenMode::Random:
    החזר "random";
  حالة AllocTokenMode::TypeHash:
    החזר "typehash";
  حالة AllocTokenMode::TypeHashPointerSplit:
    החזר "typehashpointersplit";
  حالة AllocTokenMode::TypeFuncHash:
    החזר "typefunchash";
  حالة AllocTokenMode::TypeFuncHashPointerSplit:
    החזר "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ساكن uint64_t getStableHash(ثابت AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  החזר getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ساكن uint64_t getTypeFuncHash(ثابت AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, منطقي PointerSplit) {
  אם (MaxTokens == 1)
    החזר 0;
  ثابت unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  ثابت unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  אם (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  אם (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  החזר Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            ثابت AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  أكد(MaxTokens && "Must provide non-zero max tokens");

  بدل (Mode) {
  حالة AllocTokenMode::Increment:
  حالة AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    החזר std::nullopt;

  حالة AllocTokenMode::TypeFuncHash:
  حالة AllocTokenMode::TypeFuncHashPointerSplit:
    אם (!Metadata.FunctionName)
      החזר std::nullopt;
    החזר getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  حالة AllocTokenMode::TypeHash:
    החזר getStableHash(Metadata, MaxTokens);

  حالة AllocTokenMode::TypeHashPointerSplit: {
    אם (MaxTokens == 1)
      החזר 0;
    ثابت uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    אם (Metadata.ContainsPointer)
      Hash += HalfTokens;
    החזר Hash;
  }
  }

  llvm_unreachable("");
}
