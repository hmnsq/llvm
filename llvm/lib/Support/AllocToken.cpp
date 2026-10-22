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

#अन्तर्भाव "llvm/Support/AllocToken.h"
#अन्तर्भाव "llvm/ADT/StringSwitch.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/SipHash.h"

उपयुज्य नामाकाश llvm;

std::optional<Alloc_Token_बहुलकम्>
llvm::get_Alloc_Token_बहुलकम्_From_String(StringRef Name) {
  प्रत्यागम StringSwitch<std::optional<Alloc_Token_बहुलकम्>>(Name)
      .Case("increment", Alloc_Token_बहुलकम्::Increment)
      .Case("random", Alloc_Token_बहुलकम्::Random)
      .Case("typehash", Alloc_Token_बहुलकम्::TypeHash)
      .Case("typehashpointersplit", Alloc_Token_बहुलकम्::Type_Hash_Pointer_विभज)
      .Case("typefunchash", Alloc_Token_बहुलकम्::TypeFuncHash)
      .Case("typefunchashpointersplit",
            Alloc_Token_बहुलकम्::Type_Func_Hash_Pointer_विभज)
      .Case("default", Default_Alloc_Token_बहुलकम्)
      .Default(std::nullopt);
}

StringRef llvm::get_Alloc_Token_बहुलकम्_As_String(Alloc_Token_बहुलकम् बहुलकम्) {
  विकल्प (बहुलकम्) {
  पक्ष Alloc_Token_बहुलकम्::Increment:
    प्रत्यागम "increment";
  पक्ष Alloc_Token_बहुलकम्::Random:
    प्रत्यागम "random";
  पक्ष Alloc_Token_बहुलकम्::TypeHash:
    प्रत्यागम "typehash";
  पक्ष Alloc_Token_बहुलकम्::Type_Hash_Pointer_विभज:
    प्रत्यागम "typehashpointersplit";
  पक्ष Alloc_Token_बहुलकम्::TypeFuncHash:
    प्रत्यागम "typefunchash";
  पक्ष Alloc_Token_बहुलकम्::Type_Func_Hash_Pointer_विभज:
    प्रत्यागम "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

स्थिर uint64_t getStableHash(ध्रुव AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  प्रत्यागम getStableSipHash(Metadata.TypeName) % MaxTokens;
}

/// Splits the Bits bits into: [pointer flag,] type name hash, function name
/// hash. The function name hash gets Bits / 2 bits. Bits is k if MaxTokens is
/// 2^k-1 (e.g. SIZE_MAX, tokens in [0, MaxTokens]), or Log2(MaxTokens)
/// otherwise. With pointer split, the MSB is the pointer flag.
स्थिर uint64_t getTypeFuncHash(ध्रुव AllocTokenMetadata &Metadata,
                                uint64_t MaxTokens, तार्किक Pointer_विभज) {
  यदि (MaxTokens == 1)
    प्रत्यागम 0;
  ध्रुव अचिह्नित द्वयङ्काः_2 =
      is_आवरणम्_64(MaxTokens) ? llvm::countr_one(MaxTokens) : Log2_64(MaxTokens);
  ध्रुव अचिह्नित Func_द्वयङ्काः = द्वयङ्काः_2 / 2;
  अचिह्नित Type_द्वयङ्काः = द्वयङ्काः_2 - Func_द्वयङ्काः;
  uint64_t Token = 0;
  यदि (Pointer_विभज) {
    --Type_द्वयङ्काः;
    Token = uint64_t(Metadata.अन्तर्भवति_Pointer) << (द्वयङ्काः_2 - 1);
  }
  // An empty type name denotes an unknown type.
  यदि (!Metadata.TypeName.रिक्तानि())
    Token |= (getStableSipHash(Metadata.TypeName) &
              आवरणम्_Trailing_Ones<uint64_t>(Type_द्वयङ्काः))
             << Func_द्वयङ्काः;
  Token |= getStableSipHash(*Metadata.FunctionName) &
           आवरणम्_Trailing_Ones<uint64_t>(Func_द्वयङ्काः);
  प्रत्यागम Token;
}

std::optional<uint64_t> llvm::getAllocToken(Alloc_Token_बहुलकम् बहुलकम्,
                                            ध्रुव AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  प्रतिज्ञा(MaxTokens && "Must provide non-zero max tokens");

  विकल्प (बहुलकम्) {
  पक्ष Alloc_Token_बहुलकम्::Increment:
  पक्ष Alloc_Token_बहुलकम्::Random:
    // Stateful modes cannot be implemented as a pure function.
    प्रत्यागम std::nullopt;

  पक्ष Alloc_Token_बहुलकम्::TypeFuncHash:
  पक्ष Alloc_Token_बहुलकम्::Type_Func_Hash_Pointer_विभज:
    यदि (!Metadata.FunctionName)
      प्रत्यागम std::nullopt;
    प्रत्यागम getTypeFuncHash(Metadata, MaxTokens,
                           बहुलकम् == Alloc_Token_बहुलकम्::Type_Func_Hash_Pointer_विभज);

  पक्ष Alloc_Token_बहुलकम्::TypeHash:
    प्रत्यागम getStableHash(Metadata, MaxTokens);

  पक्ष Alloc_Token_बहुलकम्::Type_Hash_Pointer_विभज: {
    यदि (MaxTokens == 1)
      प्रत्यागम 0;
    ध्रुव uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    यदि (Metadata.अन्तर्भवति_Pointer)
      Hash += HalfTokens;
    प्रत्यागम Hash;
  }
  }

  llvm_unreachable("");
}
