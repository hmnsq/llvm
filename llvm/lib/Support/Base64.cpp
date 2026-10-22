//===- Base64.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define INVALID_BASE64_BYTE 64
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Base64.h"

ସ୍ଥିର ଅକ୍ଷର decodeBase64Byte(uint8_t Ch) {
  constexpr ଅକ୍ଷର Inv = INVALID_BASE64_BYTE;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର DecodeTable[] = {
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
  ଯଦି (Ch >= ଆକାର(DecodeTable))
    ଫେରନ୍ତୁ Inv;
  ଫେରନ୍ତୁ DecodeTable[Ch];
}

llvm::Error llvm::decodeBase64(llvm::StringRef ନିବେଶ,
                               std::vector<ଅକ୍ଷର> &Output) {
  constexpr ଅକ୍ଷର Base64InvalidByte = INVALID_BASE64_BYTE;
  // Invalid table value with short name to fit in the table init below. The
  // invalid value is 64 since valid base64 values are 0 - 63.
  Output.clear();
  ଅଚଳ uint64_t ନିବେଶ_Length = ନିବେଶ.size();
  ଯଦି (ନିବେଶ_Length == 0)
    ଫେରନ୍ତୁ Error::success();
  // Make sure we have a valid input string length which must be a multiple
  // of 4.
  ଯଦି ((ନିବେଶ_Length % 4) != 0)
    ଫେରନ୍ତୁ createStringError(std::errc::illegal_byte_sequence,
                             "Base64 encoded strings must be a multiple of 4 "
                             "bytes in length");
  ଅଚଳ uint64_t FirstValidEqualIdx = ନିବେଶ_Length - 2;
  ଅକ୍ଷର Hex64Bytes[4];
  ପାଇଁ (uint64_t Idx = 0; Idx < ନିବେଶ_Length; Idx += 4) {
    ପାଇଁ (uint64_t ByteOffset = 0; ByteOffset < 4; ++ByteOffset) {
      ଅଚଳ uint64_t ByteIdx = Idx + ByteOffset;
      ଅଚଳ ଅକ୍ଷର Byte = ନିବେଶ[ByteIdx];
      ଅଚଳ ଅକ୍ଷର DecodedByte = decodeBase64Byte(Byte);
      ତାର୍କିକ Illegal = DecodedByte == Base64InvalidByte;
      ଯଦି (!Illegal && Byte == '=') {
        ଯଦି (ByteIdx < FirstValidEqualIdx) {
          // We have an '=' in the middle of the string which is invalid, only
          // the last two characters can be '=' characters.
          Illegal = ସତ୍ୟ;
        } ଅନ୍ୟଥା ଯଦି (ByteIdx == FirstValidEqualIdx && ନିବେଶ[ByteIdx + 1] != '=') {
          // We have an equal second to last from the end and the last character
          // is not also an equal, so the '=' character is invalid
          Illegal = ସତ୍ୟ;
        }
      }
      ଯଦି (Illegal)
        ଫେରନ୍ତୁ createStringError(
            std::errc::illegal_byte_sequence,
            "Invalid Base64 character %#2.2x at index %" PRIu64, Byte, ByteIdx);
      Hex64Bytes[ByteOffset] = DecodedByte;
    }
    // Now we have 6 bits of 3 bytes in value in each of the Hex64Bytes bytes.
    // Extract the right bytes into the Output buffer.
    Output.ଠେଲ_back((Hex64Bytes[0] << 2) + ((Hex64Bytes[1] >> 4) & 0x03));
    Output.ଠେଲ_back((Hex64Bytes[1] << 4) + ((Hex64Bytes[2] >> 2) & 0x0f));
    Output.ଠେଲ_back((Hex64Bytes[2] << 6) + (Hex64Bytes[3] & 0x3f));
  }
  // If we had valid trailing '=' characters strip the right number of bytes
  // from the end of the output buffer. We already know that the Input length
  // it a multiple of 4 and is not zero, so direct character access is safe.
  ଯଦି (ନିବେଶ.back() == '=') {
    Output.କାଢ଼_back();
    ଯଦି (ନିବେଶ[ନିବେଶ_Length - 2] == '=')
      Output.କାଢ଼_back();
  }
  ଫେରନ୍ତୁ Error::success();
}
