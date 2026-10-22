//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryStreamReader.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryStreamError.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryStreamRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LEB128.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader(ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref Ref) : ਧਾਰਾ(Ref) {}

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader(ਦੁਆਧਾਰੀ_ਧਾਰਾ &ਧਾਰਾ) : ਧਾਰਾ(ਧਾਰਾ) {}

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader(ArrayRef<uint8_t> ਅੰਕੜੇ_2,
                                       endianness Endian)
    : ਧਾਰਾ(ਅੰਕੜੇ_2, Endian) {}

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader(StringRef ਅੰਕੜੇ_2, endianness Endian)
    : ਧਾਰਾ(ਅੰਕੜੇ_2, Endian) {}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &ਬਫਰ) {
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = ਧਾਰਾ.readLongestContiguousChunk(ਵਿੱਥ, ਬਫਰ))
    ਵਾਪਸ EC;
  ਵਿੱਥ += ਬਫਰ.ਮਾਪ_2();
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::read_ਬਾਈਟਾਂ(ArrayRef<uint8_t> &ਬਫਰ, uint32_t ਮਾਪ) {
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = ਧਾਰਾ.read_ਬਾਈਟਾਂ(ਵਿੱਥ, ਮਾਪ, ਬਫਰ))
    ਵਾਪਸ EC;
  ਵਿੱਥ += ਮਾਪ;
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::readULEB128(uint64_t &Dest) {
  Small_ਸਦਿਸ਼<uint8_t, 10> Encoded_ਬਾਈਟਾਂ;
  ArrayRef<uint8_t> ਅਗਲਾ_Byte;

  // Copy the encoded ULEB into the buffer.
  ਕਰੋ {
    ਜੇ (ਸਵੈਚਾਲਿਤ Err = read_ਬਾਈਟਾਂ(ਅਗਲਾ_Byte, 1))
      ਵਾਪਸ Err;
    Encoded_ਬਾਈਟਾਂ.ਧੱਕੋ_ਪਿੱਠ(ਅਗਲਾ_Byte[0]);
  } ਜਦੋਂ (ਅਗਲਾ_Byte[0] & 0x80);

  Dest = decodeULEB128(Encoded_ਬਾਈਟਾਂ.begin(), ਖਾਲੀ_ਸੂਚਕ, Encoded_ਬਾਈਟਾਂ.ਸਿਰਾ());
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::readSLEB128(int64_t &Dest) {
  Small_ਸਦਿਸ਼<uint8_t, 10> Encoded_ਬਾਈਟਾਂ;
  ArrayRef<uint8_t> ਅਗਲਾ_Byte;

  // Copy the encoded ULEB into the buffer.
  ਕਰੋ {
    ਜੇ (ਸਵੈਚਾਲਿਤ Err = read_ਬਾਈਟਾਂ(ਅਗਲਾ_Byte, 1))
      ਵਾਪਸ Err;
    Encoded_ਬਾਈਟਾਂ.ਧੱਕੋ_ਪਿੱਠ(ਅਗਲਾ_Byte[0]);
  } ਜਦੋਂ (ਅਗਲਾ_Byte[0] & 0x80);

  Dest = decodeSLEB128(Encoded_ਬਾਈਟਾਂ.begin(), ਖਾਲੀ_ਸੂਚਕ, Encoded_ਬਾਈਟਾਂ.ਸਿਰਾ());
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::readCString(StringRef &Dest) {
  uint64_t Original_ਵਿੱਥ = get_ਵਿੱਥ();
  uint64_t ਮਿਲਿਆ_ਵਿੱਥ = 0;
  ਜਦੋਂ (ਸੱਚ) {
    uint64_t This_ਵਿੱਥ = get_ਵਿੱਥ();
    ArrayRef<uint8_t> ਬਫਰ;
    ਜੇ (ਸਵੈਚਾਲਿਤ EC = readLongestContiguousChunk(ਬਫਰ))
      ਵਾਪਸ EC;
    StringRef S(reinterpret_cast<ਅਚਲ ਅੱਖਰ *>(ਬਫਰ.begin()), ਬਫਰ.ਮਾਪ_2());
    size_t Pos = S.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ('\0');
    ਜੇ (LLVM_LIKELY(Pos != StringRef::npos)) {
      ਮਿਲਿਆ_ਵਿੱਥ = Pos + This_ਵਿੱਥ;
      ਰੋਕੋ;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(ਮਿਲਿਆ_ਵਿੱਥ >= Original_ਵਿੱਥ);

  ਸੈੱਟ_ਵਿੱਥ(Original_ਵਿੱਥ);
  size_t ਲੰਮਾਈ = ਮਿਲਿਆ_ਵਿੱਥ - Original_ਵਿੱਥ;

  ਜੇ (ਸਵੈਚਾਲਿਤ EC = readFixedString(Dest, ਲੰਮਾਈ))
    ਵਾਪਸ EC;

  // Now set the offset back to after the null terminator.
  ਸੈੱਟ_ਵਿੱਥ(ਮਿਲਿਆ_ਵਿੱਥ + 1);
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t ਲੰਮਾਈ = 0;
  uint64_t Original_ਵਿੱਥ = get_ਵਿੱਥ();
  ਅਚਲ UTF16 *C;
  ਜਦੋਂ (ਸੱਚ) {
    ਜੇ (ਸਵੈਚਾਲਿਤ EC = readObject(C))
      ਵਾਪਸ EC;
    ਜੇ (*C == 0x0000)
      ਰੋਕੋ;
    ++ਲੰਮਾਈ;
  }
  uint64_t New_ਵਿੱਥ = get_ਵਿੱਥ();
  ਸੈੱਟ_ਵਿੱਥ(Original_ਵਿੱਥ);

  ਜੇ (ਸਵੈਚਾਲਿਤ EC = readArray(Dest, ਲੰਮਾਈ))
    ਵਾਪਸ EC;
  ਸੈੱਟ_ਵਿੱਥ(New_ਵਿੱਥ);
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::readFixedString(StringRef &Dest, uint32_t ਲੰਮਾਈ) {
  ArrayRef<uint8_t> ਬਾਈਟਾਂ_2;
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = read_ਬਾਈਟਾਂ(ਬਾਈਟਾਂ_2, ਲੰਮਾਈ))
    ਵਾਪਸ EC;
  Dest = StringRef(reinterpret_cast<ਅਚਲ ਅੱਖਰ *>(ਬਾਈਟਾਂ_2.begin()), ਬਾਈਟਾਂ_2.ਮਾਪ_2());
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::read_ਧਾਰਾ_Ref(ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref &Ref) {
  ਵਾਪਸ read_ਧਾਰਾ_Ref(Ref, ਬਾਈਟਾਂ_ਬਚਿਆ());
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::read_ਧਾਰਾ_Ref(ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref &Ref, uint32_t ਲੰਮਾਈ) {
  ਜੇ (ਬਾਈਟਾਂ_ਬਚਿਆ() < ਲੰਮਾਈ)
    ਵਾਪਸ make_error<ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error>(ਧਾਰਾ_error_ਕੋਡ::ਧਾਰਾ_too_short);
  Ref = ਧਾਰਾ.ਟੁਕੜਾ(ਵਿੱਥ, ਲੰਮਾਈ);
  ਵਿੱਥ += ਲੰਮਾਈ;
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::readSubstream(ਦੁਆਧਾਰੀ_Substream_Ref &Ref,
                                        uint32_t ਲੰਮਾਈ) {
  Ref.ਵਿੱਥ = get_ਵਿੱਥ();
  ਵਾਪਸ read_ਧਾਰਾ_Ref(Ref.ਧਾਰਾ_ਅੰਕੜੇ, ਲੰਮਾਈ);
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::skip(uint64_t Amount) {
  ਜੇ (Amount > ਬਾਈਟਾਂ_ਬਚਿਆ())
    ਵਾਪਸ make_error<ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error>(ਧਾਰਾ_error_ਕੋਡ::ਧਾਰਾ_too_short);
  ਵਿੱਥ += Amount;
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::pad_ਮੰਜ਼ਿਲ_Alignment(uint32_t Align) {
  uint32_t New_ਵਿੱਥ = align_ਮੰਜ਼ਿਲ(ਵਿੱਥ, Align);
  ਵਾਪਸ skip(New_ਵਿੱਥ - ਵਿੱਥ);
}

uint8_t ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::peek() ਅਚਲ {
  ArrayRef<uint8_t> ਬਫਰ;
  ਸਵੈਚਾਲਿਤ EC = ਧਾਰਾ.read_ਬਾਈਟਾਂ(ਵਿੱਥ, 1, ਬਫਰ);
  ਪ੍ਰਤਿਜ੍ਞਾ(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  ਵਾਪਸ ਬਫਰ[0];
}

std::pair<ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader, ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader>
ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader::ਵੰਡੋ(uint64_t Off) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(get_ਲੰਮਾਈ() >= Off);

  ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref ਪਹਿਲਾ = ਧਾਰਾ.drop_front(ਵਿੱਥ);

  ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref ਦੂਜਾ = ਪਹਿਲਾ.drop_front(Off);
  ਪਹਿਲਾ = ਪਹਿਲਾ.keep_front(Off);
  ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader W1{ਪਹਿਲਾ};
  ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader W2{ਦੂਜਾ};
  ਵਾਪਸ {W1, W2};
}
