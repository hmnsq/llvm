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

#అన్తర్భావ "llvm/Support/Win64EH.h"
#అన్తర్భావ "llvm/Support/Endian.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::Win64EH;

StringRef Win64EH::get_రిజిస్టరు_Name_V3(గుర్తులేని Reg) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి పేర్లు[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  ఒకవేళ (Reg >= std::కొలత_2(పేర్లు))
    తిరిగి "<invalid>";
  తిరిగి పేర్లు[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        గుర్తులేని స్థానభ్రంశం) {
  ఒకవేళ (స్థానభ్రంశం >= Pool.కొలత_2())
    తిరిగి createStringError("WOD pool overflow at offset %u", స్థానభ్రంశం);

  uint8_t మొదటి_Byte = Pool[స్థానభ్రంశం];
  DecodedWOD W = {};

  // Determine opcode from variable-width prefix encoding.
  // The dispatch order matters: check shorter prefixes first since they
  // occupy the lowest bits, then fall through to longer prefixes.
  //   3-bit prefix (bits [2:0] >= 4): opcodes 4-7
  //   4-bit prefix (bits [3:0] >= 8): opcodes 8-10
  //   6-bit prefix (bits [5:0] == 0x20): opcode 32 (PUSH2)
  //   8-bit prefix (full byte 0-3): opcodes 0-3
  uint8_t Low3 = మొదటి_Byte & 0x07;

  // 3-bit opcode: bits [2:0] in {4, 5, 6, 7}
  ఒకవేళ (Low3 >= 4) {
    ఎంపిక (Low3) {
    స్థితి WOD_నెట్టు: {
      W.Opcode = WOD_నెట్టు;
      W.Byte_కొలత = 1;
      W.రిజిస్టరు = (మొదటి_Byte >> 3) & 0x1F; // 5-bit register
      తిరిగి W;
    }
    స్థితి WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_కొలత = 5;
      ఒకవేళ (స్థానభ్రంశం + 5 > Pool.కొలత_2())
        తిరిగి createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 స్థానభ్రంశం);
      W.రిజిస్టరు = (మొదటి_Byte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[స్థానభ్రంశం + 1]);
      తిరిగి W;
    }
    స్థితి WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_కొలత = 3;
      ఒకవేళ (స్థానభ్రంశం + 3 > Pool.కొలత_2())
        తిరిగి createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 స్థానభ్రంశం);
      W.రిజిస్టరు = (మొదటి_Byte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[స్థానభ్రంశం + 1]) * 8;
      తిరిగి W;
    }
    స్థితి WOD_నెట్టు_CONSECUTIVE_2: {
      W.Opcode = WOD_నెట్టు_CONSECUTIVE_2;
      W.Byte_కొలత = 1;
      W.రిజిస్టరు = (మొదటి_Byte >> 3) & 0x1F;
      ఒకవేళ (W.రిజిస్టరు > 30)
        తిరిగి createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.రిజిస్టరు, స్థానభ్రంశం);
      తిరిగి W;
    }
    అప్రమేయ:
      తిరిగి createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               మొదటి_Byte, స్థానభ్రంశం);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = మొదటి_Byte & 0x0F;
  ఒకవేళ (Low4 >= 8) {
    ఎంపిక (Low4) {
    స్థితి WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_కొలత = 1;
      W.కొలత = (గుర్తులేని)(((మొదటి_Byte >> 4) & 0x0F) + 1) * 8;
      తిరిగి W;
    }
    స్థితి WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_కొలత = 5;
      ఒకవేళ (స్థానభ్రంశం + 5 > Pool.కొలత_2())
        తిరిగి createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 స్థానభ్రంశం);
      W.రిజిస్టరు = (మొదటి_Byte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[స్థానభ్రంశం + 1]);
      తిరిగి W;
    }
    స్థితి WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_కొలత = 3;
      ఒకవేళ (స్థానభ్రంశం + 3 > Pool.కొలత_2())
        తిరిగి createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 స్థానభ్రంశం);
      W.రిజిస్టరు = (మొదటి_Byte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[స్థానభ్రంశం + 1]) * 16;
      తిరిగి W;
    }
    అప్రమేయ:
      తిరిగి createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               మొదటి_Byte, స్థానభ్రంశం);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = మొదటి_Byte & 0x3F;
  ఒకవేళ (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_కొలత = 2;
    ఒకవేళ (స్థానభ్రంశం + 2 > Pool.కొలత_2())
      తిరిగి createStringError("WOD_PUSH2 truncated at offset %u", స్థానభ్రంశం);
    uint8_t రెండవ_Byte = Pool[స్థానభ్రంశం + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.రిజిస్టరు = ((మొదటి_Byte >> 6) & 0x03) | ((రెండవ_Byte & 0x07) << 2);
    W.Register2 = (రెండవ_Byte >> 3) & 0x1F;
    తిరిగి W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  ఎంపిక (మొదటి_Byte) {
  స్థితి WOD_సమితి_FPREG: {
    W.Opcode = WOD_సమితి_FPREG;
    W.Byte_కొలత = 2;
    ఒకవేళ (స్థానభ్రంశం + 2 > Pool.కొలత_2())
      తిరిగి createStringError("WOD_SET_FPREG truncated at offset %u", స్థానభ్రంశం);
    uint8_t రెండవ_Byte = Pool[స్థానభ్రంశం + 1];
    W.రిజిస్టరు = రెండవ_Byte & 0x0F; // 4-bit register
    W.Displacement = (గుర్తులేని)((రెండవ_Byte >> 4) & 0x0F) * 16;
    తిరిగి W;
  }
  స్థితి WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_కొలత = 5;
    ఒకవేళ (స్థానభ్రంశం + 5 > Pool.కొలత_2())
      తిరిగి createStringError("WOD_ALLOC_HUGE truncated at offset %u", స్థానభ్రంశం);
    W.కొలత = support::endian::read32le(&Pool[స్థానభ్రంశం + 1]);
    తిరిగి W;
  }
  స్థితి WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_కొలత = 3;
    ఒకవేళ (స్థానభ్రంశం + 3 > Pool.కొలత_2())
      తిరిగి createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               స్థానభ్రంశం);
    W.కొలత = (uint32_t)support::endian::read16le(&Pool[స్థానభ్రంశం + 1]) * 8;
    తిరిగి W;
  }
  స్థితి WOD_నెట్టు_CANONICAL_చట్రం: {
    W.Opcode = WOD_నెట్టు_CANONICAL_చట్రం;
    W.Byte_కొలత = 2;
    ఒకవేళ (స్థానభ్రంశం + 2 > Pool.కొలత_2())
      తిరిగి createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", స్థానభ్రంశం);
    W.Type = Pool[స్థానభ్రంశం + 1];
    తిరిగి W;
  }
  అప్రమేయ:
    తిరిగి createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             మొదటి_Byte, స్థానభ్రంశం);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> దత్తాంశం_2) {
  ఒకవేళ (దత్తాంశం_2.కొలత_2() < 4)
    తిరిగి createStringError("V3 unwind info too short: %zu bytes",
                             దత్తాంశం_2.కొలత_2());

  DecodedUnwindInfoV3 Info;
  Info.సంస్కరణ = దత్తాంశం_2[0] & 0x07;
  Info.Flags = (దత్తాంశం_2[0] >> 3) & 0x1F;
  Info.కొలత_యొక్క_Prolog = దత్తాంశం_2[1];
  Info.Payload_పదాలు = దత్తాంశం_2[2];
  Info.సంఖ్య_యొక్క_Ops = దత్తాంశం_2[3] & 0x1F;
  Info.సంఖ్య_యొక్క_Epilogs = (దత్తాంశం_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  గుర్తులేని స్థానభ్రంశం = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  గుర్తులేని Payload_చివర = 4 + Info.Payload_పదాలు * 2;
  ఒకవేళ (Payload_చివర > దత్తాంశం_2.కొలత_2())
    తిరిగి createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_పదాలు);

  తార్కిక IsLarge = Info.isLarge();
  ఒకవేళ (IsLarge) {
    ఒకవేళ (స్థానభ్రంశం >= Payload_చివర)
      తిరిగి createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_పదాలు);
    Info.కొలత_యొక్క_Prolog |= static_cast<uint16_t>(దత్తాంశం_2[స్థానభ్రంశం]) << 8;
    స్థానభ్రంశం += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  కొరకు (గుర్తులేని I = 0; I < Info.సంఖ్య_యొక్క_Ops; ++I) {
    ఒకవేళ (IsLarge) {
      ఒకవేళ (స్థానభ్రంశం + 2 > Payload_చివర)
        తిరిగి createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.నెట్టు_వెనుక(support::endian::read16le(&దత్తాంశం_2[స్థానభ్రంశం]));
      స్థానభ్రంశం += 2;
    } లేకపోతే {
      ఒకవేళ (స్థానభ్రంశం >= Payload_చివర)
        తిరిగి createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.నెట్టు_వెనుక(దత్తాంశం_2[స్థానభ్రంశం++]);
    }
  }

  // Read epilog descriptors
  int32_t Prev_Resolved_స్థానభ్రంశం = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  పూర్ణాంకం పునాది_Epilog_Idx = -1;
  కొరకు (గుర్తులేని I = 0; I < Info.సంఖ్య_యొక్క_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    ఒకవేళ (స్థానభ్రంశం >= Payload_చివర)
      తిరిగి createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_అంకె_Ops = దత్తాంశం_2[స్థానభ్రంశం++];
    Epi.Flags = Flags_And_అంకె_Ops & 0x07;
    Epi.సంఖ్య_యొక్క_Ops = (Flags_And_అంకె_Ops >> 3) & 0x1F;

    ఒకవేళ (స్థానభ్రంశం + 2 > Payload_చివర)
      తిరిగి createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t Raw_స్థానభ్రంశం =
        static_cast<int16_t>(support::endian::read16le(&దత్తాంశం_2[స్థానభ్రంశం]));
    స్థానభ్రంశం += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    ఒకవేళ (I == 0)
      Epi.Epilog_స్థానభ్రంశం = Raw_స్థానభ్రంశం;
    లేకపోతే
      Epi.Epilog_స్థానభ్రంశం = Prev_Resolved_స్థానభ్రంశం + Raw_స్థానభ్రంశం;
    Prev_Resolved_స్థానభ్రంశం = Epi.Epilog_స్థానభ్రంశం;

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
    ఒకవేళ (Epi.సంఖ్య_యొక్క_Ops == 0) {
      ఒకవేళ (పునాది_Epilog_Idx >= 0) {
        స్థిరరాశి DecodedEpilogV3 &పునాది = Info.Epilogs[పునాది_Epilog_Idx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.మొదటి_Op = పునాది.మొదటి_Op;
        Epi.Ip_స్థానభ్రంశం_యొక్క_చివరి_Instruction = పునాది.Ip_స్థానభ్రంశం_యొక్క_చివరి_Instruction;
        Epi.IpOffsets = పునాది.IpOffsets;
      } లేకపోతే {
        Epi.మొదటి_Op = 0;
        Epi.Ip_స్థానభ్రంశం_యొక్క_చివరి_Instruction = 0;
      }
      Info.Epilogs.నెట్టు_వెనుక(std::move(Epi));
      కొనసాగు;
    }

    తార్కిక EpiLarge = Epi.isLarge();

    ఒకవేళ (స్థానభ్రంశం + 2 > Payload_చివర)
      తిరిగి createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.మొదటి_Op = support::endian::read16le(&దత్తాంశం_2[స్థానభ్రంశం]);
    స్థానభ్రంశం += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    ఒకవేళ (EpiLarge) {
      ఒకవేళ (స్థానభ్రంశం + 2 > Payload_చివర)
        తిరిగి createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_స్థానభ్రంశం_యొక్క_చివరి_Instruction = support::endian::read16le(&దత్తాంశం_2[స్థానభ్రంశం]);
      స్థానభ్రంశం += 2;
    } లేకపోతే {
      ఒకవేళ (స్థానభ్రంశం >= Payload_చివర)
        తిరిగి createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_స్థానభ్రంశం_యొక్క_చివరి_Instruction = దత్తాంశం_2[స్థానభ్రంశం++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    కొరకు (గుర్తులేని J = 0; J < Epi.సంఖ్య_యొక్క_Ops; ++J) {
      ఒకవేళ (EpiLarge) {
        ఒకవేళ (స్థానభ్రంశం + 2 > Payload_చివర)
          తిరిగి createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.నెట్టు_వెనుక(support::endian::read16le(&దత్తాంశం_2[స్థానభ్రంశం]));
        స్థానభ్రంశం += 2;
      } లేకపోతే {
        ఒకవేళ (స్థానభ్రంశం >= Payload_చివర)
          తిరిగి createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.నెట్టు_వెనుక(దత్తాంశం_2[స్థానభ్రంశం++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    పునాది_Epilog_Idx = Info.Epilogs.కొలత_2();
    Info.Epilogs.నెట్టు_వెనుక(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  ఒకవేళ (స్థానభ్రంశం < Payload_చివర)
    Info.WODPool = దత్తాంశం_2.ముక్క(స్థానభ్రంశం, Payload_చివర - స్థానభ్రంశం);
  లేకపోతే
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_కొలత = align_గమ్యం(Payload_చివర, 4);

  తిరిగి Info;
}
