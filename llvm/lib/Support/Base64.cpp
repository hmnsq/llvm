//===- Base64.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#تعریف_کن INVALID_BASE64_BYTE 64
#ضمن "llvm/Support/Base64.h"

ساكن حرف decodeBase64Byte(uint8_t Ch) {
  constexpr حرف Inv = INVALID_BASE64_BYTE;
  ساكن ثابت حرف Decode_جدول[] = {
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
  إذا (Ch >= حجم(Decode_جدول))
    أرجع Inv;
  أرجع Decode_جدول[Ch];
}

llvm::Error llvm::decodeBase64(llvm::StringRef درون_داد,
                               std::وکتور<حرف> &Output) {
  constexpr حرف Base64InvalidByte = INVALID_BASE64_BYTE;
  // Invalid table value with short name to fit in the table init below. The
  // invalid value is 64 since valid base64 values are 0 - 63.
  Output.clear();
  ثابت uint64_t درون_داد_درازی = درون_داد.اندازه_2();
  إذا (درون_داد_درازی == 0)
    أرجع Error::success();
  // Make sure we have a valid input string length which must be a multiple
  // of 4.
  إذا ((درون_داد_درازی % 4) != 0)
    أرجع createStringError(std::errc::illegal_byte_sequence,
                             "Base64 encoded strings must be a multiple of 4 "
                             "bytes in length");
  ثابت uint64_t اولی_Valid_Equal_Idx = درون_داد_درازی - 2;
  حرف Hex64_بایتها[4];
  لكل (uint64_t Idx = 0; Idx < درون_داد_درازی; Idx += 4) {
    لكل (uint64_t Byte_جابه_جایی = 0; Byte_جابه_جایی < 4; ++Byte_جابه_جایی) {
      ثابت uint64_t ByteIdx = Idx + Byte_جابه_جایی;
      ثابت حرف Byte = درون_داد[ByteIdx];
      ثابت حرف DecodedByte = decodeBase64Byte(Byte);
      منطقي Illegal = DecodedByte == Base64InvalidByte;
      إذا (!Illegal && Byte == '=') {
        إذا (ByteIdx < اولی_Valid_Equal_Idx) {
          // We have an '=' in the middle of the string which is invalid, only
          // the last two characters can be '=' characters.
          Illegal = درست;
        } وإلا إذا (ByteIdx == اولی_Valid_Equal_Idx && درون_داد[ByteIdx + 1] != '=') {
          // We have an equal second to last from the end and the last character
          // is not also an equal, so the '=' character is invalid
          Illegal = درست;
        }
      }
      إذا (Illegal)
        أرجع createStringError(
            std::errc::illegal_byte_sequence,
            "Invalid Base64 character %#2.2x at index %" PRIu64, Byte, ByteIdx);
      Hex64_بایتها[Byte_جابه_جایی] = DecodedByte;
    }
    // Now we have 6 bits of 3 bytes in value in each of the Hex64Bytes bytes.
    // Extract the right bytes into the Output buffer.
    Output.افزودن_پشت((Hex64_بایتها[0] << 2) + ((Hex64_بایتها[1] >> 4) & 0x03));
    Output.افزودن_پشت((Hex64_بایتها[1] << 4) + ((Hex64_بایتها[2] >> 2) & 0x0f));
    Output.افزودن_پشت((Hex64_بایتها[2] << 6) + (Hex64_بایتها[3] & 0x3f));
  }
  // If we had valid trailing '=' characters strip the right number of bytes
  // from the end of the output buffer. We already know that the Input length
  // it a multiple of 4 and is not zero, so direct character access is safe.
  إذا (درون_داد.پشت() == '=') {
    Output.برداشت_پشت();
    إذا (درون_داد[درون_داد_درازی - 2] == '=')
      Output.برداشت_پشت();
  }
  أرجع Error::success();
}
