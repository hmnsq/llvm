//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamReader.h"

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamError.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LEB128.h"

using ನಾಮಾಕಾಶ llvm;

ದ್ವಿಮಾನ_Stream_Reader::ದ್ವಿಮಾನ_Stream_Reader(ದ್ವಿಮಾನ_Stream_Ref Ref) : Stream(Ref) {}

ದ್ವಿಮಾನ_Stream_Reader::ದ್ವಿಮಾನ_Stream_Reader(ದ್ವಿಮಾನ_Stream &Stream) : Stream(Stream) {}

ದ್ವಿಮಾನ_Stream_Reader::ದ್ವಿಮಾನ_Stream_Reader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

ದ್ವಿಮಾನ_Stream_Reader::ದ್ವಿಮಾನ_Stream_Reader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error ದ್ವಿಮಾನ_Stream_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  ಯದಿ (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    ಪ್ರತ್ಯಾಗಮ EC;
  Offset += Buffer.size();
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  ಯದಿ (auto EC = Stream.readBytes(Offset, Size, Buffer))
    ಪ್ರತ್ಯಾಗಮ EC;
  Offset += Size;
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  ಕುರು {
    ಯದಿ (auto Err = readBytes(NextByte, 1))
      ಪ್ರತ್ಯಾಗಮ Err;
    EncodedBytes.ತಳ್ಳು_back(NextByte[0]);
  } ಯಾವತ್ (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  ಕುರು {
    ಯದಿ (auto Err = readBytes(NextByte, 1))
      ಪ್ರತ್ಯಾಗಮ Err;
    EncodedBytes.ತಳ್ಳು_back(NextByte[0]);
  } ಯಾವತ್ (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  ಯಾವತ್ (ಸತ್ಯ) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    ಯದಿ (auto EC = readLongestContiguousChunk(Buffer))
      ಪ್ರತ್ಯಾಗಮ EC;
    StringRef S(reinterpret_cast<ಧ್ರುವ ವರ್ಣ *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    ಯದಿ (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತಿಜ್ಞಾ(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  ಯದಿ (auto EC = readFixedString(Dest, Length))
    ಪ್ರತ್ಯಾಗಮ EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  ಧ್ರುವ UTF16 *C;
  ಯಾವತ್ (ಸತ್ಯ) {
    ಯದಿ (auto EC = readObject(C))
      ಪ್ರತ್ಯಾಗಮ EC;
    ಯದಿ (*C == 0x0000)
      ಭಙ್ಗ;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  ಯದಿ (auto EC = readArray(Dest, Length))
    ಪ್ರತ್ಯಾಗಮ EC;
  setOffset(NewOffset);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  ಯದಿ (auto EC = readBytes(Bytes, Length))
    ಪ್ರತ್ಯಾಗಮ EC;
  Dest = StringRef(reinterpret_cast<ಧ್ರುವ ವರ್ಣ *>(Bytes.begin()), Bytes.size());
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::readStreamRef(ದ್ವಿಮಾನ_Stream_Ref &Ref) {
  ಪ್ರತ್ಯಾಗಮ readStreamRef(Ref, bytesRemaining());
}

Error ದ್ವಿಮಾನ_Stream_Reader::readStreamRef(ದ್ವಿಮಾನ_Stream_Ref &Ref, uint32_t Length) {
  ಯದಿ (bytesRemaining() < Length)
    ಪ್ರತ್ಯಾಗಮ make_error<ದ್ವಿಮಾನ_Stream_Error>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::readSubstream(ದ್ವಿಮಾನ_Substream_Ref &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  ಪ್ರತ್ಯಾಗಮ readStreamRef(Ref.StreamData, Length);
}

Error ದ್ವಿಮಾನ_Stream_Reader::skip(uint64_t Amount) {
  ಯದಿ (Amount > bytesRemaining())
    ಪ್ರತ್ಯಾಗಮ make_error<ದ್ವಿಮಾನ_Stream_Error>(stream_error_code::stream_too_short);
  Offset += Amount;
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Reader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  ಪ್ರತ್ಯಾಗಮ skip(NewOffset - Offset);
}

uint8_t ದ್ವಿಮಾನ_Stream_Reader::peek() ಧ್ರುವ {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  ಪ್ರತಿಜ್ಞಾ(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  ಪ್ರತ್ಯಾಗಮ Buffer[0];
}

std::pair<ದ್ವಿಮಾನ_Stream_Reader, ದ್ವಿಮಾನ_Stream_Reader>
ದ್ವಿಮಾನ_Stream_Reader::split(uint64_t Off) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(getLength() >= Off);

  ದ್ವಿಮಾನ_Stream_Ref First = Stream.drop_front(Offset);

  ದ್ವಿಮಾನ_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  ದ್ವಿಮಾನ_Stream_Reader W1{First};
  ದ್ವಿಮಾನ_Stream_Reader W2{Second};
  ಪ್ರತ್ಯಾಗಮ {W1, W2};
}
