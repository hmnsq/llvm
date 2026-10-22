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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AllocToken.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringSwitch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SipHash.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

std::optional<Alloc_Token_ਬਹੁਲਕ>
llvm::get_Alloc_Token_ਬਹੁਲਕ_From_String(StringRef Name) {
  ਵਾਪਸ StringSwitch<std::optional<Alloc_Token_ਬਹੁਲਕ>>(Name)
      .Case("increment", Alloc_Token_ਬਹੁਲਕ::Increment)
      .Case("random", Alloc_Token_ਬਹੁਲਕ::Random)
      .Case("typehash", Alloc_Token_ਬਹੁਲਕ::TypeHash)
      .Case("typehashpointersplit", Alloc_Token_ਬਹੁਲਕ::Type_Hash_ਸੂਚਕ_ਵੰਡੋ)
      .Case("typefunchash", Alloc_Token_ਬਹੁਲਕ::TypeFuncHash)
      .Case("typefunchashpointersplit",
            Alloc_Token_ਬਹੁਲਕ::Type_Func_Hash_ਸੂਚਕ_ਵੰਡੋ)
      .Case("default", Default_Alloc_Token_ਬਹੁਲਕ)
      .Default(std::nullopt);
}

StringRef llvm::get_Alloc_Token_ਬਹੁਲਕ_As_String(Alloc_Token_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਚੋਣ (ਬਹੁਲਕ) {
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::Increment:
    ਵਾਪਸ "increment";
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::Random:
    ਵਾਪਸ "random";
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::TypeHash:
    ਵਾਪਸ "typehash";
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::Type_Hash_ਸੂਚਕ_ਵੰਡੋ:
    ਵਾਪਸ "typehashpointersplit";
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::TypeFuncHash:
    ਵਾਪਸ "typefunchash";
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::Type_Func_Hash_ਸੂਚਕ_ਵੰਡੋ:
    ਵਾਪਸ "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

ਸਥਿਰ uint64_t getStableHash(ਅਚਲ AllocTokenMetadata &Metadata,
                              uint64_t Max_ਟੋਕਨ) {
  ਵਾਪਸ getStableSipHash(Metadata.TypeName) % Max_ਟੋਕਨ;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
ਸਥਿਰ uint64_t getTypeFuncHash(ਅਚਲ AllocTokenMetadata &Metadata,
                                uint64_t Max_ਟੋਕਨ, ਤਾਰਕਿਕ ਸੂਚਕ_ਵੰਡੋ) {
  ਜੇ (Max_ਟੋਕਨ == 1)
    ਵਾਪਸ 0;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ_2 =
      is_ਨਕਾਬ_64(Max_ਟੋਕਨ) ? llvm::countr_one(Max_ਟੋਕਨ) : Log2_64(Max_ਟੋਕਨ);
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Func_ਬਿੱਟ = ਬਿੱਟ_2 / 2;
  ਬਿਨਾਨਿਸ਼ਾਨੀ Type_ਬਿੱਟ = ਬਿੱਟ_2 - Func_ਬਿੱਟ;
  uint64_t Token = 0;
  ਜੇ (ਸੂਚਕ_ਵੰਡੋ) {
    --Type_ਬਿੱਟ;
    Token = uint64_t(Metadata.ਸ਼ਾਮਲ_ਹੈ_ਸੂਚਕ) << (ਬਿੱਟ_2 - 1);
  }
  // An empty type name denotes an unknown type.
  ਜੇ (!Metadata.TypeName.ਖਾਲੀ_ਥਾਵਾਂ())
    Token |= (getStableSipHash(Metadata.TypeName) &
              ਨਕਾਬ_Trailing_Ones<uint64_t>(Type_ਬਿੱਟ))
             << Func_ਬਿੱਟ;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           ਨਕਾਬ_Trailing_Ones<uint64_t>(Func_ਬਿੱਟ);
  ਵਾਪਸ Token;
}

std::optional<uint64_t> llvm::getAllocToken(Alloc_Token_ਬਹੁਲਕ ਬਹੁਲਕ,
                                            ਅਚਲ AllocTokenMetadata &Metadata,
                                            uint64_t Max_ਟੋਕਨ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Max_ਟੋਕਨ && "Must provide non-zero max tokens");

  ਚੋਣ (ਬਹੁਲਕ) {
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::Increment:
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::Random:
    // Stateful modes cannot be implemented as a pure function.
    ਵਾਪਸ std::nullopt;

  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::TypeFuncHash:
  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::Type_Func_Hash_ਸੂਚਕ_ਵੰਡੋ:
    ਜੇ (!Metadata.FunctionName)
      ਵਾਪਸ std::nullopt;
    ਵਾਪਸ getTypeFuncHash(Metadata, Max_ਟੋਕਨ,
                           ਬਹੁਲਕ == Alloc_Token_ਬਹੁਲਕ::Type_Func_Hash_ਸੂਚਕ_ਵੰਡੋ);

  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::TypeHash:
    ਵਾਪਸ getStableHash(Metadata, Max_ਟੋਕਨ);

  ਹਾਲਤ Alloc_Token_ਬਹੁਲਕ::Type_Hash_ਸੂਚਕ_ਵੰਡੋ: {
    ਜੇ (Max_ਟੋਕਨ == 1)
      ਵਾਪਸ 0;
    ਅਚਲ uint64_t Half_ਟੋਕਨ = Max_ਟੋਕਨ / 2;
    uint64_t Hash = getStableHash(Metadata, Half_ਟੋਕਨ);
    ਜੇ (Metadata.ਸ਼ਾਮਲ_ਹੈ_ਸੂਚਕ)
      Hash += Half_ਟੋਕਨ;
    ਵਾਪਸ Hash;
  }
  }

  llvm_unreachable("");
}
