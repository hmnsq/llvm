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

#அந்தர்பாவ "llvm/Support/AllocToken.h"
#அந்தர்பாவ "llvm/ADT/StringSwitch.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/MathExtras.h"
#அந்தர்பாவ "llvm/Support/SipHash.h"

using நாமாகாஶ llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  திரும்பு StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  தேர்வு (Mode) {
  நிலை AllocTokenMode::Increment:
    திரும்பு "increment";
  நிலை AllocTokenMode::Random:
    திரும்பு "random";
  நிலை AllocTokenMode::TypeHash:
    திரும்பு "typehash";
  நிலை AllocTokenMode::TypeHashPointerSplit:
    திரும்பு "typehashpointersplit";
  நிலை AllocTokenMode::TypeFuncHash:
    திரும்பு "typefunchash";
  நிலை AllocTokenMode::TypeFuncHashPointerSplit:
    திரும்பு "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

நிலையான uint64_t getStableHash(மாறிலி AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  திரும்பு getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
நிலையான uint64_t getTypeFuncHash(மாறிலி AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, தருக்க PointerSplit) {
  எனில் (MaxTokens == 1)
    திரும்பு 0;
  மாறிலி unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  மாறிலி unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  எனில் (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  எனில் (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  திரும்பு Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            மாறிலி AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  ப்ரதிஜ்ஞா(MaxTokens && "Must provide non-zero max tokens");

  தேர்வு (Mode) {
  நிலை AllocTokenMode::Increment:
  நிலை AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    திரும்பு std::nullopt;

  நிலை AllocTokenMode::TypeFuncHash:
  நிலை AllocTokenMode::TypeFuncHashPointerSplit:
    எனில் (!Metadata.FunctionName)
      திரும்பு std::nullopt;
    திரும்பு getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  நிலை AllocTokenMode::TypeHash:
    திரும்பு getStableHash(Metadata, MaxTokens);

  நிலை AllocTokenMode::TypeHashPointerSplit: {
    எனில் (MaxTokens == 1)
      திரும்பு 0;
    மாறிலி uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    எனில் (Metadata.ContainsPointer)
      Hash += HalfTokens;
    திரும்பு Hash;
  }
  }

  llvm_unreachable("");
}
