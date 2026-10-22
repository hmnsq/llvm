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

#అన్తర్భావ "llvm/Support/AllocToken.h"
#అన్తర్భావ "llvm/ADT/StringSwitch.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/SipHash.h"

ఉపయోగించి నామాకాశ llvm;

std::optional<Alloc_Token_బహుళకం>
llvm::get_Alloc_Token_బహుళకం_From_String(StringRef Name) {
  తిరిగి StringSwitch<std::optional<Alloc_Token_బహుళకం>>(Name)
      .Case("increment", Alloc_Token_బహుళకం::Increment)
      .Case("random", Alloc_Token_బహుళకం::Random)
      .Case("typehash", Alloc_Token_బహుళకం::TypeHash)
      .Case("typehashpointersplit", Alloc_Token_బహుళకం::Type_Hash_సూచిక_విభజించు)
      .Case("typefunchash", Alloc_Token_బహుళకం::TypeFuncHash)
      .Case("typefunchashpointersplit",
            Alloc_Token_బహుళకం::Type_Func_Hash_సూచిక_విభజించు)
      .Case("default", Default_Alloc_Token_బహుళకం)
      .Default(std::nullopt);
}

StringRef llvm::get_Alloc_Token_బహుళకం_As_String(Alloc_Token_బహుళకం బహుళకం) {
  ఎంపిక (బహుళకం) {
  స్థితి Alloc_Token_బహుళకం::Increment:
    తిరిగి "increment";
  స్థితి Alloc_Token_బహుళకం::Random:
    తిరిగి "random";
  స్థితి Alloc_Token_బహుళకం::TypeHash:
    తిరిగి "typehash";
  స్థితి Alloc_Token_బహుళకం::Type_Hash_సూచిక_విభజించు:
    తిరిగి "typehashpointersplit";
  స్థితి Alloc_Token_బహుళకం::TypeFuncHash:
    తిరిగి "typefunchash";
  స్థితి Alloc_Token_బహుళకం::Type_Func_Hash_సూచిక_విభజించు:
    తిరిగి "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

స్థిర uint64_t getStableHash(స్థిరరాశి AllocTokenMetadata &Metadata,
                              uint64_t Max_టోకెన్లు) {
  తిరిగి getStableSipHash(Metadata.TypeName) % Max_టోకెన్లు;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
స్థిర uint64_t getTypeFuncHash(స్థిరరాశి AllocTokenMetadata &Metadata,
                                uint64_t Max_టోకెన్లు, తార్కిక సూచిక_విభజించు) {
  ఒకవేళ (Max_టోకెన్లు == 1)
    తిరిగి 0;
  స్థిరరాశి గుర్తులేని బిట్లు_2 =
      is_ముసుగు_64(Max_టోకెన్లు) ? llvm::countr_one(Max_టోకెన్లు) : Log2_64(Max_టోకెన్లు);
  స్థిరరాశి గుర్తులేని Func_బిట్లు = బిట్లు_2 / 2;
  గుర్తులేని Type_బిట్లు = బిట్లు_2 - Func_బిట్లు;
  uint64_t Token = 0;
  ఒకవేళ (సూచిక_విభజించు) {
    --Type_బిట్లు;
    Token = uint64_t(Metadata.కలిగియుంది_సూచిక) << (బిట్లు_2 - 1);
  }
  // An empty type name denotes an unknown type.
  ఒకవేళ (!Metadata.TypeName.ఖాళీలు())
    Token |= (getStableSipHash(Metadata.TypeName) &
              ముసుగు_Trailing_Ones<uint64_t>(Type_బిట్లు))
             << Func_బిట్లు;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           ముసుగు_Trailing_Ones<uint64_t>(Func_బిట్లు);
  తిరిగి Token;
}

std::optional<uint64_t> llvm::getAllocToken(Alloc_Token_బహుళకం బహుళకం,
                                            స్థిరరాశి AllocTokenMetadata &Metadata,
                                            uint64_t Max_టోకెన్లు) {
  ప్రతిజ్ఞా(Max_టోకెన్లు && "Must provide non-zero max tokens");

  ఎంపిక (బహుళకం) {
  స్థితి Alloc_Token_బహుళకం::Increment:
  స్థితి Alloc_Token_బహుళకం::Random:
    // Stateful modes cannot be implemented as a pure function.
    తిరిగి std::nullopt;

  స్థితి Alloc_Token_బహుళకం::TypeFuncHash:
  స్థితి Alloc_Token_బహుళకం::Type_Func_Hash_సూచిక_విభజించు:
    ఒకవేళ (!Metadata.FunctionName)
      తిరిగి std::nullopt;
    తిరిగి getTypeFuncHash(Metadata, Max_టోకెన్లు,
                           బహుళకం == Alloc_Token_బహుళకం::Type_Func_Hash_సూచిక_విభజించు);

  స్థితి Alloc_Token_బహుళకం::TypeHash:
    తిరిగి getStableHash(Metadata, Max_టోకెన్లు);

  స్థితి Alloc_Token_బహుళకం::Type_Hash_సూచిక_విభజించు: {
    ఒకవేళ (Max_టోకెన్లు == 1)
      తిరిగి 0;
    స్థిరరాశి uint64_t Half_టోకెన్లు = Max_టోకెన్లు / 2;
    uint64_t Hash = getStableHash(Metadata, Half_టోకెన్లు);
    ఒకవేళ (Metadata.కలిగియుంది_సూచిక)
      Hash += Half_టోకెన్లు;
    తిరిగి Hash;
  }
  }

  llvm_unreachable("");
}
