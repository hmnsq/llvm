//===- Base64.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INVALID_BASE64_BYTE 64
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Base64.h"

ਸਥਿਰ ਅੱਖਰ decodeBase64Byte(uint8_t Ch) {
  constexpr ਅੱਖਰ Inv = INVALID_BASE64_BYTE;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ Decode_ਸਾਰਣੀ[] = {
      Inv, Inv, Inv, Inv, Inv, Inv, Inv, Inv, // ........
      Inv, Inv, Inv, Inv, Inv, Inv, Inv, Inv, // ........
      Inv, Inv, Inv, Inv, Inv, Inv, Inv, Inv, // ........
      Inv, Inv, Inv, Inv, Inv, Inv, Inv, Inv, // ........
      Inv, Inv, Inv, Inv, Inv, Inv, Inv, Inv, // ........
      Inv, Inv, Inv, 62,  Inv, Inv, Inv, 63,  // ...+.../
      52,  53,  54,  55,  56,  57,  58,  59,  // 01234567
      60,  61,  Inv, Inv, Inv, 0,   Inv, Inv, // 89...=..
      Inv, 0,   1,   2,   3,   4,   5,   6,   // .ABCDEFG
      7,   8,   9,   10,  11,  12,  13,  14,  // HIJKLMNO
      15,  16,  17,  18,  19,  20,  21,  22,  // PQRSTUVW
      23,  24,  25,  Inv, Inv, Inv, Inv, Inv, // XYZ.....
      Inv, 26,  27,  28,  29,  30,  31,  32,  // .abcdefg
      33,  34,  35,  36,  37,  38,  39,  40,  // hijklmno
      41,  42,  43,  44,  45,  46,  47,  48,  // pqrstuvw
      49,  50,  51                            // xyz.....
  };
  ਜੇ (Ch >= ਆਕਾਰ(Decode_ਸਾਰਣੀ))
    ਵਾਪਸ Inv;
  ਵਾਪਸ Decode_ਸਾਰਣੀ[Ch];
}

llvm::Error llvm::decodeBase64(llvm::StringRef ਨਿਵੇਸ਼,
                               std::ਸਦਿਸ਼<ਅੱਖਰ> &Output) {
  constexpr ਅੱਖਰ Base64InvalidByte = INVALID_BASE64_BYTE;
  // Invalid table value with short name to fit in the table init below. The
  // invalid value is 64 since valid base64 values are 0 - 63.
  Output.clear();
  ਅਚਲ uint64_t ਨਿਵੇਸ਼_ਲੰਮਾਈ = ਨਿਵੇਸ਼.ਮਾਪ_2();
  ਜੇ (ਨਿਵੇਸ਼_ਲੰਮਾਈ == 0)
    ਵਾਪਸ Error::success();
  // Make sure we have a valid input string length which must be a multiple
  // of 4.
  ਜੇ ((ਨਿਵੇਸ਼_ਲੰਮਾਈ % 4) != 0)
    ਵਾਪਸ createStringError(std::errc::illegal_byte_sequence,
                             "Base64 encoded strings must be a multiple of 4 "
                             "bytes in length");
  ਅਚਲ uint64_t ਪਹਿਲਾ_Valid_Equal_Idx = ਨਿਵੇਸ਼_ਲੰਮਾਈ - 2;
  ਅੱਖਰ Hex64_ਬਾਈਟਾਂ[4];
  ਲਈ (uint64_t Idx = 0; Idx < ਨਿਵੇਸ਼_ਲੰਮਾਈ; Idx += 4) {
    ਲਈ (uint64_t Byte_ਵਿੱਥ = 0; Byte_ਵਿੱਥ < 4; ++Byte_ਵਿੱਥ) {
      ਅਚਲ uint64_t ByteIdx = Idx + Byte_ਵਿੱਥ;
      ਅਚਲ ਅੱਖਰ Byte = ਨਿਵੇਸ਼[ByteIdx];
      ਅਚਲ ਅੱਖਰ DecodedByte = decodeBase64Byte(Byte);
      ਤਾਰਕਿਕ Illegal = DecodedByte == Base64InvalidByte;
      ਜੇ (!Illegal && Byte == '=') {
        ਜੇ (ByteIdx < ਪਹਿਲਾ_Valid_Equal_Idx) {
          // We have an '=' in the middle of the string which is invalid, only
          // the last two characters can be '=' characters.
          Illegal = ਸੱਚ;
        } ਨਹੀਂਤਾਂ ਜੇ (ByteIdx == ਪਹਿਲਾ_Valid_Equal_Idx && ਨਿਵੇਸ਼[ByteIdx + 1] != '=') {
          // We have an equal second to last from the end and the last character
          // is not also an equal, so the '=' character is invalid
          Illegal = ਸੱਚ;
        }
      }
      ਜੇ (Illegal)
        ਵਾਪਸ createStringError(
            std::errc::illegal_byte_sequence,
            "Invalid Base64 character %#2.2x at index %" PRIu64, Byte, ByteIdx);
      Hex64_ਬਾਈਟਾਂ[Byte_ਵਿੱਥ] = DecodedByte;
    }
    // Now we have 6 bits of 3 bytes in value in each of the Hex64Bytes bytes.
    // Extract the right bytes into the Output buffer.
    Output.ਧੱਕੋ_ਪਿੱਠ((Hex64_ਬਾਈਟਾਂ[0] << 2) + ((Hex64_ਬਾਈਟਾਂ[1] >> 4) & 0x03));
    Output.ਧੱਕੋ_ਪਿੱਠ((Hex64_ਬਾਈਟਾਂ[1] << 4) + ((Hex64_ਬਾਈਟਾਂ[2] >> 2) & 0x0f));
    Output.ਧੱਕੋ_ਪਿੱਠ((Hex64_ਬਾਈਟਾਂ[2] << 6) + (Hex64_ਬਾਈਟਾਂ[3] & 0x3f));
  }
  // If we had valid trailing '=' characters strip the right number of bytes
  // from the end of the output buffer. We already know that the Input length
  // it a multiple of 4 and is not zero, so direct character access is safe.
  ਜੇ (ਨਿਵੇਸ਼.ਪਿੱਠ() == '=') {
    Output.ਕੱਢੋ_ਪਿੱਠ();
    ਜੇ (ਨਿਵੇਸ਼[ਨਿਵੇਸ਼_ਲੰਮਾਈ - 2] == '=')
      Output.ਕੱਢੋ_ਪਿੱਠ();
  }
  ਵਾਪਸ Error::success();
}
