//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamWriter.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamReader.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LEB128.h"

using ನಾಮಾಕಾಶ llvm;

ದ್ವಿಮಾನ_Stream_Writer::ದ್ವಿಮಾನ_Stream_Writer(Writable_ದ್ವಿಮಾನ_Stream_Ref Ref)
    : Stream(Ref) {}

ದ್ವಿಮಾನ_Stream_Writer::ದ್ವಿಮಾನ_Stream_Writer(Writable_ದ್ವಿಮಾನ_Stream &Stream)
    : Stream(Stream) {}

ದ್ವಿಮಾನ_Stream_Writer::ದ್ವಿಮಾನ_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error ದ್ವಿಮಾನ_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  ಯದಿ (auto EC = Stream.writeBytes(Offset, Buffer))
    ಪ್ರತ್ಯಾಗಮ EC;
  Offset += Buffer.size();
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Writer::writeULEB128(uint64_t ಮೌಲ್ಯ) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(ಮೌಲ್ಯ, &EncodedBytes[0]);
  ಪ್ರತ್ಯಾಗಮ writeBytes({EncodedBytes, Size});
}

Error ದ್ವಿಮಾನ_Stream_Writer::writeSLEB128(int64_t ಮೌಲ್ಯ) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(ಮೌಲ್ಯ, &EncodedBytes[0]);
  ಪ್ರತ್ಯಾಗಮ writeBytes({EncodedBytes, Size});
}

Error ದ್ವಿಮಾನ_Stream_Writer::writeCString(StringRef Str) {
  ಯದಿ (auto EC = writeFixedString(Str))
    ಪ್ರತ್ಯಾಗಮ EC;
  ಯದಿ (auto EC = writeObject('\0'))
    ಪ್ರತ್ಯಾಗಮ EC;

  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ದ್ವಿಮಾನ_Stream_Writer::writeFixedString(StringRef Str) {

  ಪ್ರತ್ಯಾಗಮ writeBytes(arrayRefFromStringRef(Str));
}

Error ದ್ವಿಮಾನ_Stream_Writer::writeStreamRef(ದ್ವಿಮಾನ_Stream_Ref Ref) {
  ಪ್ರತ್ಯಾಗಮ writeStreamRef(Ref, Ref.getLength());
}

Error ದ್ವಿಮಾನ_Stream_Writer::writeStreamRef(ದ್ವಿಮಾನ_Stream_Ref Ref, uint64_t Length) {
  ದ್ವಿಮಾನ_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  ಯಾವತ್ (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    ಯದಿ (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      ಪ್ರತ್ಯಾಗಮ EC;
    ಯದಿ (auto EC = writeBytes(Chunk))
      ಪ್ರತ್ಯಾಗಮ EC;
  }
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

std::pair<ದ್ವಿಮಾನ_Stream_Writer, ದ್ವಿಮಾನ_Stream_Writer>
ದ್ವಿಮಾನ_Stream_Writer::split(uint64_t Off) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(getLength() >= Off);

  Writable_ದ್ವಿಮಾನ_Stream_Ref First = Stream.drop_front(Offset);

  Writable_ದ್ವಿಮಾನ_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  ದ್ವಿಮಾನ_Stream_Writer W1{First};
  ದ್ವಿಮಾನ_Stream_Writer W2{Second};
  ಪ್ರತ್ಯಾಗಮ {W1, W2};
}

Error ದ್ವಿಮಾನ_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ಧ್ರುವ uint64_t ZerosSize = 64;
  ಸ್ಥಿರ constexpr ವರ್ಣ Zeros[ZerosSize] = {};
  ಯಾವತ್ (Offset < NewOffset)
    ಯದಿ (auto E = writeArray(
            ArrayRef<ವರ್ಣ>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      ಪ್ರತ್ಯಾಗಮ E;
  ಪ್ರತ್ಯಾಗಮ Error::success();
}
