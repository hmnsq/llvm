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

#അന്തര്ഭാവ "llvm/Support/AllocToken.h"
#അന്തര്ഭാവ "llvm/ADT/StringSwitch.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/MathExtras.h"
#അന്തര്ഭാവ "llvm/Support/SipHash.h"

using നാമാകാശ llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  തിരികെ StringSwitch<std::optional<AllocTokenMode>>(Name)
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
  തിരഞ്ഞെടുപ്പ് (Mode) {
  അവസ്ഥ AllocTokenMode::Increment:
    തിരികെ "increment";
  അവസ്ഥ AllocTokenMode::Random:
    തിരികെ "random";
  അവസ്ഥ AllocTokenMode::TypeHash:
    തിരികെ "typehash";
  അവസ്ഥ AllocTokenMode::TypeHashPointerSplit:
    തിരികെ "typehashpointersplit";
  അവസ്ഥ AllocTokenMode::TypeFuncHash:
    തിരികെ "typefunchash";
  അവസ്ഥ AllocTokenMode::TypeFuncHashPointerSplit:
    തിരികെ "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

സ്ഥിതം uint64_t getStableHash(സ്ഥിരം AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  തിരികെ getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
സ്ഥിതം uint64_t getTypeFuncHash(സ്ഥിരം AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, തർക്കം PointerSplit) {
  എങ്കിൽ (MaxTokens == 1)
    തിരികെ 0;
  സ്ഥിരം unsigned Bits =
      isMask_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  സ്ഥിരം unsigned FuncBits = Bits / 2;
  unsigned TypeBits = Bits - FuncBits;
  uint64_t Token = 0;
  എങ്കിൽ (PointerSplit) {
    --TypeBits;
    Token = uint64_t(Metadata.ContainsPointer) << (Bits - 1);
  }
  // An empty type name denotes an unknown type.
  എങ്കിൽ (!Metadata.TypeName.empty())
    Token |= (getStableSipHash(Metadata.TypeName) &
              maskTrailingOnes<uint64_t>(TypeBits))
             << FuncBits;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           maskTrailingOnes<uint64_t>(FuncBits);
  തിരികെ Token;
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            സ്ഥിരം AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  പ്രതിജ്ഞാ(MaxTokens && "Must provide non-zero max tokens");

  തിരഞ്ഞെടുപ്പ് (Mode) {
  അവസ്ഥ AllocTokenMode::Increment:
  അവസ്ഥ AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    തിരികെ std::nullopt;

  അവസ്ഥ AllocTokenMode::TypeFuncHash:
  അവസ്ഥ AllocTokenMode::TypeFuncHashPointerSplit:
    എങ്കിൽ (!Metadata.FunctionName)
      തിരികെ std::nullopt;
    തിരികെ getTypeFuncHash(Metadata, MaxTokens,
                           Mode == AllocTokenMode::TypeFuncHashPointerSplit);

  അവസ്ഥ AllocTokenMode::TypeHash:
    തിരികെ getStableHash(Metadata, MaxTokens);

  അവസ്ഥ AllocTokenMode::TypeHashPointerSplit: {
    എങ്കിൽ (MaxTokens == 1)
      തിരികെ 0;
    സ്ഥിരം uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    എങ്കിൽ (Metadata.ContainsPointer)
      Hash += HalfTokens;
    തിരികെ Hash;
  }
  }

  llvm_unreachable("");
}
