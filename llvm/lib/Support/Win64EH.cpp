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

#समावेश "llvm/Support/Win64EH.h"
#समावेश "llvm/Support/Endian.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/MathExtras.h"

using namespace llvm;
using namespace llvm::Win64EH;

StringRef Win64EH::getRegisterNameV3(अचिन्हित Reg) {
  अटल स्थिर अक्षर *स्थिर Names[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  अगर (Reg >= std::आकार_2(Names))
    वापस "<invalid>";
  वापस Names[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        अचिन्हित Offset) {
  अगर (Offset >= Pool.आकार_2())
    वापस createStringError("WOD pool overflow at offset %u", Offset);

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
  अगर (Low3 >= 4) {
    चयन (Low3) {
    हाल WOD_धकेलो: {
      W.Opcode = WOD_धकेलो;
      W.Byte_आकार = 1;
      W.Register = (FirstByte >> 3) & 0x1F; // 5-bit register
      वापस W;
    }
    हाल WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_आकार = 5;
      अगर (Offset + 5 > Pool.आकार_2())
        वापस createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 Offset);
      W.Register = (FirstByte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[Offset + 1]);
      वापस W;
    }
    हाल WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_आकार = 3;
      अगर (Offset + 3 > Pool.आकार_2())
        वापस createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 Offset);
      W.Register = (FirstByte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[Offset + 1]) * 8;
      वापस W;
    }
    हाल WOD_धकेलो_CONSECUTIVE_2: {
      W.Opcode = WOD_धकेलो_CONSECUTIVE_2;
      W.Byte_आकार = 1;
      W.Register = (FirstByte >> 3) & 0x1F;
      अगर (W.Register > 30)
        वापस createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.Register, Offset);
      वापस W;
    }
    शेष:
      वापस createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               FirstByte, Offset);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = FirstByte & 0x0F;
  अगर (Low4 >= 8) {
    चयन (Low4) {
    हाल WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_आकार = 1;
      W.आकार = (अचिन्हित)(((FirstByte >> 4) & 0x0F) + 1) * 8;
      वापस W;
    }
    हाल WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_आकार = 5;
      अगर (Offset + 5 > Pool.आकार_2())
        वापस createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 Offset);
      W.Register = (FirstByte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[Offset + 1]);
      वापस W;
    }
    हाल WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_आकार = 3;
      अगर (Offset + 3 > Pool.आकार_2())
        वापस createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 Offset);
      W.Register = (FirstByte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[Offset + 1]) * 16;
      वापस W;
    }
    शेष:
      वापस createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               FirstByte, Offset);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = FirstByte & 0x3F;
  अगर (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_आकार = 2;
    अगर (Offset + 2 > Pool.आकार_2())
      वापस createStringError("WOD_PUSH2 truncated at offset %u", Offset);
    uint8_t SecondByte = Pool[Offset + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.Register = ((FirstByte >> 6) & 0x03) | ((SecondByte & 0x07) << 2);
    W.Register2 = (SecondByte >> 3) & 0x1F;
    वापस W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  चयन (FirstByte) {
  हाल WOD_SET_FPREG: {
    W.Opcode = WOD_SET_FPREG;
    W.Byte_आकार = 2;
    अगर (Offset + 2 > Pool.आकार_2())
      वापस createStringError("WOD_SET_FPREG truncated at offset %u", Offset);
    uint8_t SecondByte = Pool[Offset + 1];
    W.Register = SecondByte & 0x0F; // 4-bit register
    W.Displacement = (अचिन्हित)((SecondByte >> 4) & 0x0F) * 16;
    वापस W;
  }
  हाल WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_आकार = 5;
    अगर (Offset + 5 > Pool.आकार_2())
      वापस createStringError("WOD_ALLOC_HUGE truncated at offset %u", Offset);
    W.आकार = support::endian::read32le(&Pool[Offset + 1]);
    वापस W;
  }
  हाल WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_आकार = 3;
    अगर (Offset + 3 > Pool.आकार_2())
      वापस createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               Offset);
    W.आकार = (uint32_t)support::endian::read16le(&Pool[Offset + 1]) * 8;
    वापस W;
  }
  हाल WOD_धकेलो_CANONICAL_FRAME: {
    W.Opcode = WOD_धकेलो_CANONICAL_FRAME;
    W.Byte_आकार = 2;
    अगर (Offset + 2 > Pool.आकार_2())
      वापस createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", Offset);
    W.Type = Pool[Offset + 1];
    वापस W;
  }
  शेष:
    वापस createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             FirstByte, Offset);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> तथ्य_2) {
  अगर (तथ्य_2.आकार_2() < 4)
    वापस createStringError("V3 unwind info too short: %zu bytes",
                             तथ्य_2.आकार_2());

  DecodedUnwindInfoV3 Info;
  Info.Version = तथ्य_2[0] & 0x07;
  Info.Flags = (तथ्य_2[0] >> 3) & 0x1F;
  Info.आकार_Of_Prolog = तथ्य_2[1];
  Info.Payload_शब्दगण = तथ्य_2[2];
  Info.संख्या_Of_Ops = तथ्य_2[3] & 0x1F;
  Info.संख्या_Of_Epilogs = (तथ्य_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  अचिन्हित Offset = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  अचिन्हित PayloadEnd = 4 + Info.Payload_शब्दगण * 2;
  अगर (PayloadEnd > तथ्य_2.आकार_2())
    वापस createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_शब्दगण);

  bool IsLarge = Info.isLarge();
  अगर (IsLarge) {
    अगर (Offset >= PayloadEnd)
      वापस createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_शब्दगण);
    Info.आकार_Of_Prolog |= static_cast<uint16_t>(तथ्य_2[Offset]) << 8;
    Offset += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  क्रम (अचिन्हित I = 0; I < Info.संख्या_Of_Ops; ++I) {
    अगर (IsLarge) {
      अगर (Offset + 2 > PayloadEnd)
        वापस createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.धकेलो_back(support::endian::read16le(&तथ्य_2[Offset]));
      Offset += 2;
    } अन्यथा {
      अगर (Offset >= PayloadEnd)
        वापस createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.धकेलो_back(तथ्य_2[Offset++]);
    }
  }

  // Read epilog descriptors
  int32_t पूर्व_Resolved_Offset = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  पूर्णांक BaseEpilogIdx = -1;
  क्रम (अचिन्हित I = 0; I < Info.संख्या_Of_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    अगर (Offset >= PayloadEnd)
      वापस createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_अंक_Ops = तथ्य_2[Offset++];
    Epi.Flags = Flags_And_अंक_Ops & 0x07;
    Epi.संख्या_Of_Ops = (Flags_And_अंक_Ops >> 3) & 0x1F;

    अगर (Offset + 2 > PayloadEnd)
      वापस createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t RawOffset =
        static_cast<int16_t>(support::endian::read16le(&तथ्य_2[Offset]));
    Offset += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    अगर (I == 0)
      Epi.EpilogOffset = RawOffset;
    अन्यथा
      Epi.EpilogOffset = पूर्व_Resolved_Offset + RawOffset;
    पूर्व_Resolved_Offset = Epi.EpilogOffset;

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
    अगर (Epi.संख्या_Of_Ops == 0) {
      अगर (BaseEpilogIdx >= 0) {
        स्थिर DecodedEpilogV3 &Base = Info.Epilogs[BaseEpilogIdx];
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
      } अन्यथा {
        Epi.FirstOp = 0;
        Epi.IpOffsetOfLastInstruction = 0;
      }
      Info.Epilogs.धकेलो_back(std::move(Epi));
      जारी;
    }

    bool EpiLarge = Epi.isLarge();

    अगर (Offset + 2 > PayloadEnd)
      वापस createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.FirstOp = support::endian::read16le(&तथ्य_2[Offset]);
    Offset += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    अगर (EpiLarge) {
      अगर (Offset + 2 > PayloadEnd)
        वापस createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.IpOffsetOfLastInstruction = support::endian::read16le(&तथ्य_2[Offset]);
      Offset += 2;
    } अन्यथा {
      अगर (Offset >= PayloadEnd)
        वापस createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.IpOffsetOfLastInstruction = तथ्य_2[Offset++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    क्रम (अचिन्हित J = 0; J < Epi.संख्या_Of_Ops; ++J) {
      अगर (EpiLarge) {
        अगर (Offset + 2 > PayloadEnd)
          वापस createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.धकेलो_back(support::endian::read16le(&तथ्य_2[Offset]));
        Offset += 2;
      } अन्यथा {
        अगर (Offset >= PayloadEnd)
          वापस createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.धकेलो_back(तथ्य_2[Offset++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    BaseEpilogIdx = Info.Epilogs.आकार_2();
    Info.Epilogs.धकेलो_back(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  अगर (Offset < PayloadEnd)
    Info.WODPool = तथ्य_2.slice(Offset, PayloadEnd - Offset);
  अन्यथा
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_आकार = alignTo(PayloadEnd, 4);

  वापस Info;
}
