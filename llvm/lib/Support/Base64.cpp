//===- Base64.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define INVALID_BASE64_BYTE 64
#અન્તર્ભાવ "llvm/Support/Base64.h"

સ્થિર અક્ષર decodeBase64Byte(uint8_t Ch) {
  constexpr અક્ષર Inv = INVALID_BASE64_BYTE;
  સ્થિર અચલ અક્ષર DecodeTable[] = {
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
  જો (Ch >= કદ(DecodeTable))
    પાછા Inv;
  પાછા DecodeTable[Ch];
}

llvm::Error llvm::decodeBase64(llvm::StringRef આગત,
                               std::vector<અક્ષર> &Output) {
  constexpr અક્ષર Base64InvalidByte = INVALID_BASE64_BYTE;
  // Invalid table value with short name to fit in the table init below. The
  // invalid value is 64 since valid base64 values are 0 - 63.
  Output.clear();
  અચલ uint64_t આગત_Length = આગત.size();
  જો (આગત_Length == 0)
    પાછા Error::success();
  // Make sure we have a valid input string length which must be a multiple
  // of 4.
  જો ((આગત_Length % 4) != 0)
    પાછા createStringError(std::errc::illegal_byte_sequence,
                             "Base64 encoded strings must be a multiple of 4 "
                             "bytes in length");
  અચલ uint64_t FirstValidEqualIdx = આગત_Length - 2;
  અક્ષર Hex64Bytes[4];
  માટે (uint64_t Idx = 0; Idx < આગત_Length; Idx += 4) {
    માટે (uint64_t ByteOffset = 0; ByteOffset < 4; ++ByteOffset) {
      અચલ uint64_t ByteIdx = Idx + ByteOffset;
      અચલ અક્ષર Byte = આગત[ByteIdx];
      અચલ અક્ષર DecodedByte = decodeBase64Byte(Byte);
      તાર્કિક Illegal = DecodedByte == Base64InvalidByte;
      જો (!Illegal && Byte == '=') {
        જો (ByteIdx < FirstValidEqualIdx) {
          // We have an '=' in the middle of the string which is invalid, only
          // the last two characters can be '=' characters.
          Illegal = સાચું;
        } નહીંતર જો (ByteIdx == FirstValidEqualIdx && આગત[ByteIdx + 1] != '=') {
          // We have an equal second to last from the end and the last character
          // is not also an equal, so the '=' character is invalid
          Illegal = સાચું;
        }
      }
      જો (Illegal)
        પાછા createStringError(
            std::errc::illegal_byte_sequence,
            "Invalid Base64 character %#2.2x at index %" PRIu64, Byte, ByteIdx);
      Hex64Bytes[ByteOffset] = DecodedByte;
    }
    // Now we have 6 bits of 3 bytes in value in each of the Hex64Bytes bytes.
    // Extract the right bytes into the Output buffer.
    Output.ધકેલો_back((Hex64Bytes[0] << 2) + ((Hex64Bytes[1] >> 4) & 0x03));
    Output.ધકેલો_back((Hex64Bytes[1] << 4) + ((Hex64Bytes[2] >> 2) & 0x0f));
    Output.ધકેલો_back((Hex64Bytes[2] << 6) + (Hex64Bytes[3] & 0x3f));
  }
  // If we had valid trailing '=' characters strip the right number of bytes
  // from the end of the output buffer. We already know that the Input length
  // it a multiple of 4 and is not zero, so direct character access is safe.
  જો (આગત.back() == '=') {
    Output.કાઢો_back();
    જો (આગત[આગત_Length - 2] == '=')
      Output.કાઢો_back();
  }
  પાછા Error::success();
}
