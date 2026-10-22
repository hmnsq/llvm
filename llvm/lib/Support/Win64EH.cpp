//===-- Win64EH.cpp - Win64 EH V3 Support -----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements decoding helpers for V3 unwind information on Win64.
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/Win64EH.h"
#अन्तर्भाव "llvm/Support/Endian.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"

using नामाकाश llvm;
using नामाकाश llvm::Win64EH;

StringRef Win64EH::getRegisterNameV3(unsigned Reg) {
  स्थिर ध्रुव वर्ण *ध्रुव Names[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  जइ (Reg >= std::size(Names))
    प्रत्यागम "<invalid>";
  प्रत्यागम Names[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        unsigned Offset) {
  जइ (Offset >= Pool.size())
    प्रत्यागम createStringError("WOD pool overflow at offset %u", Offset);

  uint8_t FirstByte = Pool[Offset];
  DecodedWOD W = {};

  // Determine opcode from variable-width prefix encoding.
  // The dispatch order matters: check shorter prefixes first since they
  // occupy the lowest bits, then fall through to longer prefixes.
  //   3-bit prefix (bits [2:0] >= 4): opcodes 4-7
  //   4-bit prefix (bits [3:0] >= 8): opcodes 8-10
  //   6-bit prefix (bits [5:0] == 0x20): opcode 32 (PUSH2)
  //   8-bit prefix (full byte 0-3): opcodes 0-3
  uint8_t Low3 = FirstByte & 0x07;

  // 3-bit opcode: bits [2:0] in {4, 5, 6, 7}
  जइ (Low3 >= 4) {
    विकल्प (Low3) {
    पक्ष WOD_प्रक्षिप: {
      W.Opcode = WOD_प्रक्षिप;
      W.ByteSize = 1;
      W.Register = (FirstByte >> 3) & 0x1F; // 5-bit register
      प्रत्यागम W;
    }
    पक्ष WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.ByteSize = 5;
      जइ (Offset + 5 > Pool.size())
        प्रत्यागम createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 Offset);
      W.Register = (FirstByte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[Offset + 1]);
      प्रत्यागम W;
    }
    पक्ष WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.ByteSize = 3;
      जइ (Offset + 3 > Pool.size())
        प्रत्यागम createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 Offset);
      W.Register = (FirstByte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[Offset + 1]) * 8;
      प्रत्यागम W;
    }
    पक्ष WOD_प्रक्षिप_CONSECUTIVE_2: {
      W.Opcode = WOD_प्रक्षिप_CONSECUTIVE_2;
      W.ByteSize = 1;
      W.Register = (FirstByte >> 3) & 0x1F;
      जइ (W.Register > 30)
        प्रत्यागम createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.Register, Offset);
      प्रत्यागम W;
    }
    स्वभाव:
      प्रत्यागम createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               FirstByte, Offset);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = FirstByte & 0x0F;
  जइ (Low4 >= 8) {
    विकल्प (Low4) {
    पक्ष WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.ByteSize = 1;
      W.Size = (unsigned)(((FirstByte >> 4) & 0x0F) + 1) * 8;
      प्रत्यागम W;
    }
    पक्ष WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.ByteSize = 5;
      जइ (Offset + 5 > Pool.size())
        प्रत्यागम createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 Offset);
      W.Register = (FirstByte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[Offset + 1]);
      प्रत्यागम W;
    }
    पक्ष WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.ByteSize = 3;
      जइ (Offset + 3 > Pool.size())
        प्रत्यागम createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 Offset);
      W.Register = (FirstByte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[Offset + 1]) * 16;
      प्रत्यागम W;
    }
    स्वभाव:
      प्रत्यागम createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               FirstByte, Offset);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = FirstByte & 0x3F;
  जइ (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.ByteSize = 2;
    जइ (Offset + 2 > Pool.size())
      प्रत्यागम createStringError("WOD_PUSH2 truncated at offset %u", Offset);
    uint8_t SecondByte = Pool[Offset + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.Register = ((FirstByte >> 6) & 0x03) | ((SecondByte & 0x07) << 2);
    W.Register2 = (SecondByte >> 3) & 0x1F;
    प्रत्यागम W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  विकल्प (FirstByte) {
  पक्ष WOD_SET_FPREG: {
    W.Opcode = WOD_SET_FPREG;
    W.ByteSize = 2;
    जइ (Offset + 2 > Pool.size())
      प्रत्यागम createStringError("WOD_SET_FPREG truncated at offset %u", Offset);
    uint8_t SecondByte = Pool[Offset + 1];
    W.Register = SecondByte & 0x0F; // 4-bit register
    W.Displacement = (unsigned)((SecondByte >> 4) & 0x0F) * 16;
    प्रत्यागम W;
  }
  पक्ष WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.ByteSize = 5;
    जइ (Offset + 5 > Pool.size())
      प्रत्यागम createStringError("WOD_ALLOC_HUGE truncated at offset %u", Offset);
    W.Size = support::endian::read32le(&Pool[Offset + 1]);
    प्रत्यागम W;
  }
  पक्ष WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.ByteSize = 3;
    जइ (Offset + 3 > Pool.size())
      प्रत्यागम createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               Offset);
    W.Size = (uint32_t)support::endian::read16le(&Pool[Offset + 1]) * 8;
    प्रत्यागम W;
  }
  पक्ष WOD_प्रक्षिप_CANONICAL_FRAME: {
    W.Opcode = WOD_प्रक्षिप_CANONICAL_FRAME;
    W.ByteSize = 2;
    जइ (Offset + 2 > Pool.size())
      प्रत्यागम createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", Offset);
    W.Type = Pool[Offset + 1];
    प्रत्यागम W;
  }
  स्वभाव:
    प्रत्यागम createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             FirstByte, Offset);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> Data) {
  जइ (Data.size() < 4)
    प्रत्यागम createStringError("V3 unwind info too short: %zu bytes",
                             Data.size());

  DecodedUnwindInfoV3 Info;
  Info.Version = Data[0] & 0x07;
  Info.Flags = (Data[0] >> 3) & 0x1F;
  Info.SizeOfProlog = Data[1];
  Info.Payload_शब्दाः = Data[2];
  Info.सङ्ख्या_Of_Ops = Data[3] & 0x1F;
  Info.सङ्ख्या_Of_Epilogs = (Data[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  unsigned Offset = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  unsigned PayloadEnd = 4 + Info.Payload_शब्दाः * 2;
  जइ (PayloadEnd > Data.size())
    प्रत्यागम createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_शब्दाः);

  तार्किक IsLarge = Info.isLarge();
  जइ (IsLarge) {
    जइ (Offset >= PayloadEnd)
      प्रत्यागम createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_शब्दाः);
    Info.SizeOfProlog |= static_cast<uint16_t>(Data[Offset]) << 8;
    Offset += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  प्रति (unsigned I = 0; I < Info.सङ्ख्या_Of_Ops; ++I) {
    जइ (IsLarge) {
      जइ (Offset + 2 > PayloadEnd)
        प्रत्यागम createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.प्रक्षिप_back(support::endian::read16le(&Data[Offset]));
      Offset += 2;
    } अण्णहा {
      जइ (Offset >= PayloadEnd)
        प्रत्यागम createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.प्रक्षिप_back(Data[Offset++]);
    }
  }

  // Read epilog descriptors
  int32_t PrevResolvedOffset = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  पूर्णाङ्क BaseEpilogIdx = -1;
  प्रति (unsigned I = 0; I < Info.सङ्ख्या_Of_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    जइ (Offset >= PayloadEnd)
      प्रत्यागम createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_अङ्कः_Ops = Data[Offset++];
    Epi.Flags = Flags_And_अङ्कः_Ops & 0x07;
    Epi.सङ्ख्या_Of_Ops = (Flags_And_अङ्कः_Ops >> 3) & 0x1F;

    जइ (Offset + 2 > PayloadEnd)
      प्रत्यागम createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t RawOffset =
        static_cast<int16_t>(support::endian::read16le(&Data[Offset]));
    Offset += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    जइ (I == 0)
      Epi.EpilogOffset = RawOffset;
    अण्णहा
      Epi.EpilogOffset = PrevResolvedOffset + RawOffset;
    PrevResolvedOffset = Epi.EpilogOffset;

    // Inherited descriptors (NumberOfOps == 0) are only 3 bytes:
    // FlagsAndNumOps(1) + EpilogOffset(2). They have no FirstOp,
    // IpOffsetOfLastInstruction, or IP offset fields appended; instead, the
    // effective NumberOfOps, FirstOp, IpOffsetOfLastInstruction, and IP offset
    // array are inherited from the first preceding descriptor with
    // NumberOfOps != 0 (the "base"). Per the V3 spec, Flags bits 0 and 1 are
    // NOT inherited: the producer is required to replicate them into this
    // record, so we keep the value read from this descriptor's own flags byte.
    //
    // If there is no preceding base descriptor to inherit from — the record is
    // malformed. We leave the extended fields zero-initialized so callers can
    // still see the (broken) header and EpilogOffset; downstream consumers
    // (e.g. the dumpers) surface a warning when they encounter NumberOfOps == 0
    // at index 0.
    जइ (Epi.सङ्ख्या_Of_Ops == 0) {
      जइ (BaseEpilogIdx >= 0) {
        ध्रुव DecodedEpilogV3 &Base = Info.Epilogs[BaseEpilogIdx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.FirstOp = Base.FirstOp;
        Epi.IpOffsetOfLastInstruction = Base.IpOffsetOfLastInstruction;
        Epi.IpOffsets = Base.IpOffsets;
      } अण्णहा {
        Epi.FirstOp = 0;
        Epi.IpOffsetOfLastInstruction = 0;
      }
      Info.Epilogs.प्रक्षिप_back(std::move(Epi));
      अनुवर्त;
    }

    तार्किक EpiLarge = Epi.isLarge();

    जइ (Offset + 2 > PayloadEnd)
      प्रत्यागम createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.FirstOp = support::endian::read16le(&Data[Offset]);
    Offset += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    जइ (EpiLarge) {
      जइ (Offset + 2 > PayloadEnd)
        प्रत्यागम createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.IpOffsetOfLastInstruction = support::endian::read16le(&Data[Offset]);
      Offset += 2;
    } अण्णहा {
      जइ (Offset >= PayloadEnd)
        प्रत्यागम createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.IpOffsetOfLastInstruction = Data[Offset++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    प्रति (unsigned J = 0; J < Epi.सङ्ख्या_Of_Ops; ++J) {
      जइ (EpiLarge) {
        जइ (Offset + 2 > PayloadEnd)
          प्रत्यागम createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.प्रक्षिप_back(support::endian::read16le(&Data[Offset]));
        Offset += 2;
      } अण्णहा {
        जइ (Offset >= PayloadEnd)
          प्रत्यागम createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.प्रक्षिप_back(Data[Offset++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    BaseEpilogIdx = Info.Epilogs.size();
    Info.Epilogs.प्रक्षिप_back(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  जइ (Offset < PayloadEnd)
    Info.WODPool = Data.slice(Offset, PayloadEnd - Offset);
  अण्णहा
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.PayloadSize = alignTo(PayloadEnd, 4);

  प्रत्यागम Info;
}
