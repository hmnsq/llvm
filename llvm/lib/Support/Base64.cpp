//===- Base64.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ವ್ಯಾಖ್ಯಾನಿಸು INVALID_BASE64_BYTE 64
#ಅನ್ತರ್ಭಾವ "llvm/Support/Base64.h"

ಸ್ಥಿರ ಅಕ್ಷರ decodeBase64Byte(uint8_t Ch) {
  constexpr ಅಕ್ಷರ Inv = INVALID_BASE64_BYTE;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ Decode_ಕೋಷ್ಟಕ[] = {
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
  ಆದರೆ (Ch >= ಗಾತ್ರ(Decode_ಕೋಷ್ಟಕ))
    ಹಿಂತಿರುಗಿ Inv;
  ಹಿಂತಿರುಗಿ Decode_ಕೋಷ್ಟಕ[Ch];
}

llvm::Error llvm::decodeBase64(llvm::StringRef ಒಳಹರಿವು,
                               std::ಸದಿಶ<ಅಕ್ಷರ> &Output) {
  constexpr ಅಕ್ಷರ Base64InvalidByte = INVALID_BASE64_BYTE;
  // Invalid table value with short name to fit in the table init below. The
  // invalid value is 64 since valid base64 values are 0 - 63.
  Output.clear();
  ಸ್ಥಿರಾಂಕ uint64_t ಒಳಹರಿವು_ಉದ್ದಳತೆ = ಒಳಹರಿವು.ಗಾತ್ರ_3();
  ಆದರೆ (ಒಳಹರಿವು_ಉದ್ದಳತೆ == 0)
    ಹಿಂತಿರುಗಿ Error::success();
  // Make sure we have a valid input string length which must be a multiple
  // of 4.
  ಆದರೆ ((ಒಳಹರಿವು_ಉದ್ದಳತೆ % 4) != 0)
    ಹಿಂತಿರುಗಿ createStringError(std::errc::illegal_byte_sequence,
                             "Base64 encoded strings must be a multiple of 4 "
                             "bytes in length");
  ಸ್ಥಿರಾಂಕ uint64_t ಮೊದಲ_Valid_Equal_Idx = ಒಳಹರಿವು_ಉದ್ದಳತೆ - 2;
  ಅಕ್ಷರ Hex64_ಬೈಟುಗಳು[4];
  ಗಾಗಿ (uint64_t Idx = 0; Idx < ಒಳಹರಿವು_ಉದ್ದಳತೆ; Idx += 4) {
    ಗಾಗಿ (uint64_t Byte_ಸ್ಥಾನಾಂತರ = 0; Byte_ಸ್ಥಾನಾಂತರ < 4; ++Byte_ಸ್ಥಾನಾಂತರ) {
      ಸ್ಥಿರಾಂಕ uint64_t ByteIdx = Idx + Byte_ಸ್ಥಾನಾಂತರ;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ Byte = ಒಳಹರಿವು[ByteIdx];
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ DecodedByte = decodeBase64Byte(Byte);
      ತಾರ್ಕಿಕ Illegal = DecodedByte == Base64InvalidByte;
      ಆದರೆ (!Illegal && Byte == '=') {
        ಆದರೆ (ByteIdx < ಮೊದಲ_Valid_Equal_Idx) {
          // We have an '=' in the middle of the string which is invalid, only
          // the last two characters can be '=' characters.
          Illegal = ನಿಜ;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ByteIdx == ಮೊದಲ_Valid_Equal_Idx && ಒಳಹರಿವು[ByteIdx + 1] != '=') {
          // We have an equal second to last from the end and the last character
          // is not also an equal, so the '=' character is invalid
          Illegal = ನಿಜ;
        }
      }
      ಆದರೆ (Illegal)
        ಹಿಂತಿರುಗಿ createStringError(
            std::errc::illegal_byte_sequence,
            "Invalid Base64 character %#2.2x at index %" PRIu64, Byte, ByteIdx);
      Hex64_ಬೈಟುಗಳು[Byte_ಸ್ಥಾನಾಂತರ] = DecodedByte;
    }
    // Now we have 6 bits of 3 bytes in value in each of the Hex64Bytes bytes.
    // Extract the right bytes into the Output buffer.
    Output.ತಳ್ಳು_ಹಿಂಭಾಗ((Hex64_ಬೈಟುಗಳು[0] << 2) + ((Hex64_ಬೈಟುಗಳು[1] >> 4) & 0x03));
    Output.ತಳ್ಳು_ಹಿಂಭಾಗ((Hex64_ಬೈಟುಗಳು[1] << 4) + ((Hex64_ಬೈಟುಗಳು[2] >> 2) & 0x0f));
    Output.ತಳ್ಳು_ಹಿಂಭಾಗ((Hex64_ಬೈಟುಗಳು[2] << 6) + (Hex64_ಬೈಟುಗಳು[3] & 0x3f));
  }
  // If we had valid trailing '=' characters strip the right number of bytes
  // from the end of the output buffer. We already know that the Input length
  // it a multiple of 4 and is not zero, so direct character access is safe.
  ಆದರೆ (ಒಳಹರಿವು.ಹಿಂಭಾಗ() == '=') {
    Output.ತೆಗೆ_ಹಿಂಭಾಗ();
    ಆದರೆ (ಒಳಹರಿವು[ಒಳಹರಿವು_ಉದ್ದಳತೆ - 2] == '=')
      Output.ತೆಗೆ_ಹಿಂಭಾಗ();
  }
  ಹಿಂತಿರುಗಿ Error::success();
}
