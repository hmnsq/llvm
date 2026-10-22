//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/BinaryStreamReader.h"

#અન્તર્ભાવ "llvm/Support/BinaryStreamError.h"
#અન્તર્ભાવ "llvm/Support/BinaryStreamRef.h"
#અન્તર્ભાવ "llvm/Support/LEB128.h"

using નામાકાશ llvm;

દ્વિઅંકી_Stream_Reader::દ્વિઅંકી_Stream_Reader(દ્વિઅંકી_Stream_Ref Ref) : Stream(Ref) {}

દ્વિઅંકી_Stream_Reader::દ્વિઅંકી_Stream_Reader(દ્વિઅંકી_Stream &Stream) : Stream(Stream) {}

દ્વિઅંકી_Stream_Reader::દ્વિઅંકી_Stream_Reader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

દ્વિઅંકી_Stream_Reader::દ્વિઅંકી_Stream_Reader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error દ્વિઅંકી_Stream_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  જો (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    પાછા EC;
  Offset += Buffer.size();
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  જો (auto EC = Stream.readBytes(Offset, Size, Buffer))
    પાછા EC;
  Offset += Size;
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  કરો {
    જો (auto Err = readBytes(NextByte, 1))
      પાછા Err;
    EncodedBytes.ધકેલો_back(NextByte[0]);
  } જ્યારે (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  કરો {
    જો (auto Err = readBytes(NextByte, 1))
      પાછા Err;
    EncodedBytes.ધકેલો_back(NextByte[0]);
  } જ્યારે (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  જ્યારે (સાચું) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    જો (auto EC = readLongestContiguousChunk(Buffer))
      પાછા EC;
    StringRef S(reinterpret_cast<અચલ અક્ષર *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    જો (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      તોડો;
    }
  }
  પ્રતિજ્ઞા(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  જો (auto EC = readFixedString(Dest, Length))
    પાછા EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  અચલ UTF16 *C;
  જ્યારે (સાચું) {
    જો (auto EC = readObject(C))
      પાછા EC;
    જો (*C == 0x0000)
      તોડો;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  જો (auto EC = readArray(Dest, Length))
    પાછા EC;
  setOffset(NewOffset);
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  જો (auto EC = readBytes(Bytes, Length))
    પાછા EC;
  Dest = StringRef(reinterpret_cast<અચલ અક્ષર *>(Bytes.begin()), Bytes.size());
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::readStreamRef(દ્વિઅંકી_Stream_Ref &Ref) {
  પાછા readStreamRef(Ref, bytesRemaining());
}

Error દ્વિઅંકી_Stream_Reader::readStreamRef(દ્વિઅંકી_Stream_Ref &Ref, uint32_t Length) {
  જો (bytesRemaining() < Length)
    પાછા make_error<દ્વિઅંકી_Stream_Error>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::readSubstream(દ્વિઅંકી_Substream_Ref &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  પાછા readStreamRef(Ref.StreamData, Length);
}

Error દ્વિઅંકી_Stream_Reader::skip(uint64_t Amount) {
  જો (Amount > bytesRemaining())
    પાછા make_error<દ્વિઅંકી_Stream_Error>(stream_error_code::stream_too_short);
  Offset += Amount;
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Reader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  પાછા skip(NewOffset - Offset);
}

uint8_t દ્વિઅંકી_Stream_Reader::peek() અચલ {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  પ્રતિજ્ઞા(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  પાછા Buffer[0];
}

std::pair<દ્વિઅંકી_Stream_Reader, દ્વિઅંકી_Stream_Reader>
દ્વિઅંકી_Stream_Reader::split(uint64_t Off) અચલ {
  પ્રતિજ્ઞા(getLength() >= Off);

  દ્વિઅંકી_Stream_Ref First = Stream.drop_front(Offset);

  દ્વિઅંકી_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  દ્વિઅંકી_Stream_Reader W1{First};
  દ્વિઅંકી_Stream_Reader W2{Second};
  પાછા {W1, W2};
}
