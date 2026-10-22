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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::Win64EH;

StringRef Win64EH::get_पञ्जिकाकोशः_Name_V3(अचिह्नित Reg) {
  स्थिर ध्रुव वर्ण *ध्रुव नामानि[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  यदि (Reg >= std::आकार_2(नामानि))
    प्रत्यागम "<invalid>";
  प्रत्यागम नामानि[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        अचिह्नित विस्थापनम्) {
  यदि (विस्थापनम् >= Pool.आकार_2())
    प्रत्यागम createStringError("WOD pool overflow at offset %u", विस्थापनम्);

  uint8_t FirstByte = Pool[विस्थापनम्];
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
  यदि (Low3 >= 4) {
    विकल्प (Low3) {
    पक्ष WOD_प्रक्षिप: {
      W.Opcode = WOD_प्रक्षिप;
      W.Byte_आकार = 1;
      W.पञ्जिकाकोशः = (FirstByte >> 3) & 0x1F; // 5-bit register
      प्रत्यागम W;
    }
    पक्ष WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_आकार = 5;
      यदि (विस्थापनम् + 5 > Pool.आकार_2())
        प्रत्यागम createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 विस्थापनम्);
      W.पञ्जिकाकोशः = (FirstByte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[विस्थापनम् + 1]);
      प्रत्यागम W;
    }
    पक्ष WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_आकार = 3;
      यदि (विस्थापनम् + 3 > Pool.आकार_2())
        प्रत्यागम createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 विस्थापनम्);
      W.पञ्जिकाकोशः = (FirstByte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[विस्थापनम् + 1]) * 8;
      प्रत्यागम W;
    }
    पक्ष WOD_प्रक्षिप_CONSECUTIVE_2: {
      W.Opcode = WOD_प्रक्षिप_CONSECUTIVE_2;
      W.Byte_आकार = 1;
      W.पञ्जिकाकोशः = (FirstByte >> 3) & 0x1F;
      यदि (W.पञ्जिकाकोशः > 30)
        प्रत्यागम createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.पञ्जिकाकोशः, विस्थापनम्);
      प्रत्यागम W;
    }
    स्वभाव:
      प्रत्यागम createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               FirstByte, विस्थापनम्);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = FirstByte & 0x0F;
  यदि (Low4 >= 8) {
    विकल्प (Low4) {
    पक्ष WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_आकार = 1;
      W.आकार = (अचिह्नित)(((FirstByte >> 4) & 0x0F) + 1) * 8;
      प्रत्यागम W;
    }
    पक्ष WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_आकार = 5;
      यदि (विस्थापनम् + 5 > Pool.आकार_2())
        प्रत्यागम createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 विस्थापनम्);
      W.पञ्जिकाकोशः = (FirstByte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[विस्थापनम् + 1]);
      प्रत्यागम W;
    }
    पक्ष WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_आकार = 3;
      यदि (विस्थापनम् + 3 > Pool.आकार_2())
        प्रत्यागम createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 विस्थापनम्);
      W.पञ्जिकाकोशः = (FirstByte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[विस्थापनम् + 1]) * 16;
      प्रत्यागम W;
    }
    स्वभाव:
      प्रत्यागम createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               FirstByte, विस्थापनम्);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = FirstByte & 0x3F;
  यदि (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_आकार = 2;
    यदि (विस्थापनम् + 2 > Pool.आकार_2())
      प्रत्यागम createStringError("WOD_PUSH2 truncated at offset %u", विस्थापनम्);
    uint8_t SecondByte = Pool[विस्थापनम् + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.पञ्जिकाकोशः = ((FirstByte >> 6) & 0x03) | ((SecondByte & 0x07) << 2);
    W.Register2 = (SecondByte >> 3) & 0x1F;
    प्रत्यागम W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  विकल्प (FirstByte) {
  पक्ष WOD_समुच्चयः_FPREG: {
    W.Opcode = WOD_समुच्चयः_FPREG;
    W.Byte_आकार = 2;
    यदि (विस्थापनम् + 2 > Pool.आकार_2())
      प्रत्यागम createStringError("WOD_SET_FPREG truncated at offset %u", विस्थापनम्);
    uint8_t SecondByte = Pool[विस्थापनम् + 1];
    W.पञ्जिकाकोशः = SecondByte & 0x0F; // 4-bit register
    W.Displacement = (अचिह्नित)((SecondByte >> 4) & 0x0F) * 16;
    प्रत्यागम W;
  }
  पक्ष WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_आकार = 5;
    यदि (विस्थापनम् + 5 > Pool.आकार_2())
      प्रत्यागम createStringError("WOD_ALLOC_HUGE truncated at offset %u", विस्थापनम्);
    W.आकार = support::endian::read32le(&Pool[विस्थापनम् + 1]);
    प्रत्यागम W;
  }
  पक्ष WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_आकार = 3;
    यदि (विस्थापनम् + 3 > Pool.आकार_2())
      प्रत्यागम createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               विस्थापनम्);
    W.आकार = (uint32_t)support::endian::read16le(&Pool[विस्थापनम् + 1]) * 8;
    प्रत्यागम W;
  }
  पक्ष WOD_प्रक्षिप_CANONICAL_FRAME: {
    W.Opcode = WOD_प्रक्षिप_CANONICAL_FRAME;
    W.Byte_आकार = 2;
    यदि (विस्थापनम् + 2 > Pool.आकार_2())
      प्रत्यागम createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", विस्थापनम्);
    W.Type = Pool[विस्थापनम् + 1];
    प्रत्यागम W;
  }
  स्वभाव:
    प्रत्यागम createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             FirstByte, विस्थापनम्);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> दत्त_2) {
  यदि (दत्त_2.आकार_2() < 4)
    प्रत्यागम createStringError("V3 unwind info too short: %zu bytes",
                             दत्त_2.आकार_2());

  DecodedUnwindInfoV3 Info;
  Info.संस्करणम् = दत्त_2[0] & 0x07;
  Info.Flags = (दत्त_2[0] >> 3) & 0x1F;
  Info.आकार_सम्बन्धि_Prolog = दत्त_2[1];
  Info.Payload_शब्दाः = दत्त_2[2];
  Info.सङ्ख्या_सम्बन्धि_Ops = दत्त_2[3] & 0x1F;
  Info.सङ्ख्या_सम्बन्धि_Epilogs = (दत्त_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  अचिह्नित विस्थापनम् = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  अचिह्नित Payload_अन्तः = 4 + Info.Payload_शब्दाः * 2;
  यदि (Payload_अन्तः > दत्त_2.आकार_2())
    प्रत्यागम createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_शब्दाः);

  तार्किक IsLarge = Info.isLarge();
  यदि (IsLarge) {
    यदि (विस्थापनम् >= Payload_अन्तः)
      प्रत्यागम createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_शब्दाः);
    Info.आकार_सम्बन्धि_Prolog |= static_cast<uint16_t>(दत्त_2[विस्थापनम्]) << 8;
    विस्थापनम् += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  प्रति (अचिह्नित I = 0; I < Info.सङ्ख्या_सम्बन्धि_Ops; ++I) {
    यदि (IsLarge) {
      यदि (विस्थापनम् + 2 > Payload_अन्तः)
        प्रत्यागम createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.प्रक्षिप_पृष्ठभागः(support::endian::read16le(&दत्त_2[विस्थापनम्]));
      विस्थापनम् += 2;
    } अन्यथा {
      यदि (विस्थापनम् >= Payload_अन्तः)
        प्रत्यागम createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.प्रक्षिप_पृष्ठभागः(दत्त_2[विस्थापनम्++]);
    }
  }

  // Read epilog descriptors
  int32_t पूर्व_Resolved_विस्थापनम् = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  पूर्णाङ्क आधारः_Epilog_Idx = -1;
  प्रति (अचिह्नित I = 0; I < Info.सङ्ख्या_सम्बन्धि_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    यदि (विस्थापनम् >= Payload_अन्तः)
      प्रत्यागम createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_अङ्कः_Ops = दत्त_2[विस्थापनम्++];
    Epi.Flags = Flags_And_अङ्कः_Ops & 0x07;
    Epi.सङ्ख्या_सम्बन्धि_Ops = (Flags_And_अङ्कः_Ops >> 3) & 0x1F;

    यदि (विस्थापनम् + 2 > Payload_अन्तः)
      प्रत्यागम createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t Raw_विस्थापनम् =
        static_cast<int16_t>(support::endian::read16le(&दत्त_2[विस्थापनम्]));
    विस्थापनम् += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    यदि (I == 0)
      Epi.Epilog_विस्थापनम् = Raw_विस्थापनम्;
    अन्यथा
      Epi.Epilog_विस्थापनम् = पूर्व_Resolved_विस्थापनम् + Raw_विस्थापनम्;
    पूर्व_Resolved_विस्थापनम् = Epi.Epilog_विस्थापनम्;

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
    यदि (Epi.सङ्ख्या_सम्बन्धि_Ops == 0) {
      यदि (आधारः_Epilog_Idx >= 0) {
        ध्रुव DecodedEpilogV3 &आधारः = Info.Epilogs[आधारः_Epilog_Idx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.FirstOp = आधारः.FirstOp;
        Epi.Ip_विस्थापनम्_सम्बन्धि_चरमः_Instruction = आधारः.Ip_विस्थापनम्_सम्बन्धि_चरमः_Instruction;
        Epi.IpOffsets = आधारः.IpOffsets;
      } अन्यथा {
        Epi.FirstOp = 0;
        Epi.Ip_विस्थापनम्_सम्बन्धि_चरमः_Instruction = 0;
      }
      Info.Epilogs.प्रक्षिप_पृष्ठभागः(std::move(Epi));
      अनुवर्त;
    }

    तार्किक EpiLarge = Epi.isLarge();

    यदि (विस्थापनम् + 2 > Payload_अन्तः)
      प्रत्यागम createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.FirstOp = support::endian::read16le(&दत्त_2[विस्थापनम्]);
    विस्थापनम् += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    यदि (EpiLarge) {
      यदि (विस्थापनम् + 2 > Payload_अन्तः)
        प्रत्यागम createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_विस्थापनम्_सम्बन्धि_चरमः_Instruction = support::endian::read16le(&दत्त_2[विस्थापनम्]);
      विस्थापनम् += 2;
    } अन्यथा {
      यदि (विस्थापनम् >= Payload_अन्तः)
        प्रत्यागम createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_विस्थापनम्_सम्बन्धि_चरमः_Instruction = दत्त_2[विस्थापनम्++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    प्रति (अचिह्नित J = 0; J < Epi.सङ्ख्या_सम्बन्धि_Ops; ++J) {
      यदि (EpiLarge) {
        यदि (विस्थापनम् + 2 > Payload_अन्तः)
          प्रत्यागम createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.प्रक्षिप_पृष्ठभागः(support::endian::read16le(&दत्त_2[विस्थापनम्]));
        विस्थापनम् += 2;
      } अन्यथा {
        यदि (विस्थापनम् >= Payload_अन्तः)
          प्रत्यागम createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.प्रक्षिप_पृष्ठभागः(दत्त_2[विस्थापनम्++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    आधारः_Epilog_Idx = Info.Epilogs.आकार_2();
    Info.Epilogs.प्रक्षिप_पृष्ठभागः(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  यदि (विस्थापनम् < Payload_अन्तः)
    Info.WODPool = दत्त_2.खण्डिका(विस्थापनम्, Payload_अन्तः - विस्थापनम्);
  अन्यथा
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_आकार = align_गन्तव्यम्(Payload_अन्तः, 4);

  प्रत्यागम Info;
}
