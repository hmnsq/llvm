//===- Base64.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#నిర్వచించు INVALID_BASE64_BYTE 64
#అన్తర్భావ "llvm/Support/Base64.h"

స్థిర అక్షరం decodeBase64Byte(uint8_t Ch) {
  constexpr అక్షరం Inv = INVALID_BASE64_BYTE;
  స్థిర స్థిరరాశి అక్షరం Decode_పట్టిక[] = {
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
  ఒకవేళ (Ch >= పరిమాణం(Decode_పట్టిక))
    తిరిగి Inv;
  తిరిగి Decode_పట్టిక[Ch];
}

llvm::Error llvm::decodeBase64(llvm::StringRef ఆదానం,
                               std::సదిశ<అక్షరం> &Output) {
  constexpr అక్షరం Base64InvalidByte = INVALID_BASE64_BYTE;
  // Invalid table value with short name to fit in the table init below. The
  // invalid value is 64 since valid base64 values are 0 - 63.
  Output.clear();
  స్థిరరాశి uint64_t ఆదానం_నిడివి = ఆదానం.కొలత_2();
  ఒకవేళ (ఆదానం_నిడివి == 0)
    తిరిగి Error::success();
  // Make sure we have a valid input string length which must be a multiple
  // of 4.
  ఒకవేళ ((ఆదానం_నిడివి % 4) != 0)
    తిరిగి createStringError(std::errc::illegal_byte_sequence,
                             "Base64 encoded strings must be a multiple of 4 "
                             "bytes in length");
  స్థిరరాశి uint64_t మొదటి_Valid_Equal_Idx = ఆదానం_నిడివి - 2;
  అక్షరం Hex64_బైట్లు[4];
  కొరకు (uint64_t Idx = 0; Idx < ఆదానం_నిడివి; Idx += 4) {
    కొరకు (uint64_t Byte_స్థానభ్రంశం = 0; Byte_స్థానభ్రంశం < 4; ++Byte_స్థానభ్రంశం) {
      స్థిరరాశి uint64_t ByteIdx = Idx + Byte_స్థానభ్రంశం;
      స్థిరరాశి అక్షరం Byte = ఆదానం[ByteIdx];
      స్థిరరాశి అక్షరం DecodedByte = decodeBase64Byte(Byte);
      తార్కిక Illegal = DecodedByte == Base64InvalidByte;
      ఒకవేళ (!Illegal && Byte == '=') {
        ఒకవేళ (ByteIdx < మొదటి_Valid_Equal_Idx) {
          // We have an '=' in the middle of the string which is invalid, only
          // the last two characters can be '=' characters.
          Illegal = నిజం;
        } లేకపోతే ఒకవేళ (ByteIdx == మొదటి_Valid_Equal_Idx && ఆదానం[ByteIdx + 1] != '=') {
          // We have an equal second to last from the end and the last character
          // is not also an equal, so the '=' character is invalid
          Illegal = నిజం;
        }
      }
      ఒకవేళ (Illegal)
        తిరిగి createStringError(
            std::errc::illegal_byte_sequence,
            "Invalid Base64 character %#2.2x at index %" PRIu64, Byte, ByteIdx);
      Hex64_బైట్లు[Byte_స్థానభ్రంశం] = DecodedByte;
    }
    // Now we have 6 bits of 3 bytes in value in each of the Hex64Bytes bytes.
    // Extract the right bytes into the Output buffer.
    Output.నెట్టు_వెనుక((Hex64_బైట్లు[0] << 2) + ((Hex64_బైట్లు[1] >> 4) & 0x03));
    Output.నెట్టు_వెనుక((Hex64_బైట్లు[1] << 4) + ((Hex64_బైట్లు[2] >> 2) & 0x0f));
    Output.నెట్టు_వెనుక((Hex64_బైట్లు[2] << 6) + (Hex64_బైట్లు[3] & 0x3f));
  }
  // If we had valid trailing '=' characters strip the right number of bytes
  // from the end of the output buffer. We already know that the Input length
  // it a multiple of 4 and is not zero, so direct character access is safe.
  ఒకవేళ (ఆదానం.వెనుక() == '=') {
    Output.తీయి_వెనుక();
    ఒకవేళ (ఆదానం[ఆదానం_నిడివి - 2] == '=')
      Output.తీయి_వెనుక();
  }
  తిరిగి Error::success();
}
