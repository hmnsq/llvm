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

#समावेश "llvm/Support/AllocToken.h"
#समावेश "llvm/ADT/StringSwitch.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/SipHash.h"

using namespace llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  वापस StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  चयन (Mode) {
  हाल AllocTokenMode::Increment:
    वापस "increment";
  हाल AllocTokenMode::Random:
    वापस "random";
  हाल AllocTokenMode::TypeHash:
    वापस "typehash";
  हाल AllocTokenMode::TypeHashPointerSplit:
    वापस "typehashpointersplit";
  हाल AllocTokenMode::TypeFuncHash:
    वापस "typefunchash";
  हाल AllocTokenMode::TypeFuncHashPointerSplit:
    वापस "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

अटल uint64_t getStableHash(स्थिर AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  वापस getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
अटल uint64_t getTypeFuncHash(स्थिर AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, bool PointerSplit) {
  अगर (MaxTokens == 1)
    वापस 0;
  स्थिर अचिन्हित Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  स्थिर अचिन्हित FuncBits = Bits / 2;
  अचिन्हित TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  अगर (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  अगर (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  वापस Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            स्थिर AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  निश्चित(MaxTokens && "Must provide non-zero max tokens");

  चयन (Mode) {
  हाल AllocTokenMode::Increment:
  हाल AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    वापस std::nullopt;

  हाल AllocTokenMode::TypeFuncHash:
  हाल AllocTokenMode::TypeFuncHashPointerSplit:
    अगर (!Metadata.FunctionName)
      वापस std::nullopt;
    वापस getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  हाल AllocTokenMode::TypeHash:
    वापस getStableHash(Metadata, MaxTokens);

  हाल AllocTokenMode::TypeHashPointerSplit: {
    अगर (MaxTokens == 1)
      वापस 0;
    स्थिर uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    अगर (Metadata.ContainsPointer)
      Hash += HalfTokens;
    वापस Hash;
  }
  }

  llvm_unreachable("");
}
