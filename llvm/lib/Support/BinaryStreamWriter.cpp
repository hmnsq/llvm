//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/BinaryStreamWriter.h"

#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/BinaryStreamReader.h"
#അന്തര്ഭാവ "llvm/Support/BinaryStreamRef.h"
#അന്തര്ഭാവ "llvm/Support/LEB128.h"

using നാമാകാശ llvm;

ദ്വിമാന_Stream_Writer::ദ്വിമാന_Stream_Writer(Writable_ദ്വിമാന_Stream_Ref Ref)
    : Stream(Ref) {}

ദ്വിമാന_Stream_Writer::ദ്വിമാന_Stream_Writer(Writable_ദ്വിമാന_Stream &Stream)
    : Stream(Stream) {}

ദ്വിമാന_Stream_Writer::ദ്വിമാന_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error ദ്വിമാന_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  എങ്കിൽ (auto EC = Stream.writeBytes(Offset, Buffer))
    തിരികെ EC;
  Offset += Buffer.size();
  തിരികെ Error::success();
}

Error ദ്വിമാന_Stream_Writer::writeULEB128(uint64_t മൂല്യം) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(മൂല്യം, &EncodedBytes[0]);
  തിരികെ writeBytes({EncodedBytes, Size});
}

Error ദ്വിമാന_Stream_Writer::writeSLEB128(int64_t മൂല്യം) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(മൂല്യം, &EncodedBytes[0]);
  തിരികെ writeBytes({EncodedBytes, Size});
}

Error ദ്വിമാന_Stream_Writer::writeCString(StringRef Str) {
  എങ്കിൽ (auto EC = writeFixedString(Str))
    തിരികെ EC;
  എങ്കിൽ (auto EC = writeObject('\0'))
    തിരികെ EC;

  തിരികെ Error::success();
}

Error ദ്വിമാന_Stream_Writer::writeFixedString(StringRef Str) {

  തിരികെ writeBytes(arrayRefFromStringRef(Str));
}

Error ദ്വിമാന_Stream_Writer::writeStreamRef(ദ്വിമാന_Stream_Ref Ref) {
  തിരികെ writeStreamRef(Ref, Ref.getLength());
}

Error ദ്വിമാന_Stream_Writer::writeStreamRef(ദ്വിമാന_Stream_Ref Ref, uint64_t Length) {
  ദ്വിമാന_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  ആകുമ്പോൾ (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    എങ്കിൽ (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      തിരികെ EC;
    എങ്കിൽ (auto EC = writeBytes(Chunk))
      തിരികെ EC;
  }
  തിരികെ Error::success();
}

std::pair<ദ്വിമാന_Stream_Writer, ദ്വിമാന_Stream_Writer>
ദ്വിമാന_Stream_Writer::split(uint64_t Off) സ്ഥിരം {
  പ്രതിജ്ഞാ(getLength() >= Off);

  Writable_ദ്വിമാന_Stream_Ref First = Stream.drop_front(Offset);

  Writable_ദ്വിമാന_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  ദ്വിമാന_Stream_Writer W1{First};
  ദ്വിമാന_Stream_Writer W2{Second};
  തിരികെ {W1, W2};
}

Error ദ്വിമാന_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  സ്ഥിരം uint64_t ZerosSize = 64;
  സ്ഥിതം constexpr അക്ഷരം Zeros[ZerosSize] = {};
  ആകുമ്പോൾ (Offset < NewOffset)
    എങ്കിൽ (auto E = writeArray(
            ArrayRef<അക്ഷരം>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      തിരികെ E;
  തിരികെ Error::success();
}
