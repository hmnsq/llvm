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

#شامل "llvm/Support/Win64EH.h"
#شامل "llvm/Support/Endian.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/MathExtras.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::Win64EH;

StringRef Win64EH::get_رجسٹر_Name_V3(غیرعلامتی Reg) {
  ساکن ثابت حرف *ثابت نام[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  اگر (Reg >= std::سائز_2(نام))
    واپس "<invalid>";
  واپس نام[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        غیرعلامتی انحراف) {
  اگر (انحراف >= Pool.سائز_2())
    واپس createStringError("WOD pool overflow at offset %u", انحراف);

  uint8_t پہلا_Byte = Pool[انحراف];
  DecodedWOD W = {};

  // Determine opcode from variable-width prefix encoding.
  // The dispatch order matters: check shorter prefixes first since they
  // occupy the lowest bits, then fall through to longer prefixes.
  //   3-bit prefix (bits [2:0] >= 4): opcodes 4-7
  //   4-bit prefix (bits [3:0] >= 8): opcodes 8-10
  //   6-bit prefix (bits [5:0] == 0x20): opcode 32 (PUSH2)
  //   8-bit prefix (full byte 0-3): opcodes 0-3
  uint8_t Low3 = پہلا_Byte & 0x07;

  // 3-bit opcode: bits [2:0] in {4, 5, 6, 7}
  اگر (Low3 >= 4) {
    انتخاب (Low3) {
    حالت WOD_ڈالو: {
      W.Opcode = WOD_ڈالو;
      W.Byte_سائز = 1;
      W.رجسٹر = (پہلا_Byte >> 3) & 0x1F; // 5-bit register
      واپس W;
    }
    حالت WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_سائز = 5;
      اگر (انحراف + 5 > Pool.سائز_2())
        واپس createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 انحراف);
      W.رجسٹر = (پہلا_Byte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[انحراف + 1]);
      واپس W;
    }
    حالت WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_سائز = 3;
      اگر (انحراف + 3 > Pool.سائز_2())
        واپس createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 انحراف);
      W.رجسٹر = (پہلا_Byte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[انحراف + 1]) * 8;
      واپس W;
    }
    حالت WOD_ڈالو_CONSECUTIVE_2: {
      W.Opcode = WOD_ڈالو_CONSECUTIVE_2;
      W.Byte_سائز = 1;
      W.رجسٹر = (پہلا_Byte >> 3) & 0x1F;
      اگر (W.رجسٹر > 30)
        واپس createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.رجسٹر, انحراف);
      واپس W;
    }
    طےشدہ:
      واپس createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               پہلا_Byte, انحراف);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = پہلا_Byte & 0x0F;
  اگر (Low4 >= 8) {
    انتخاب (Low4) {
    حالت WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_سائز = 1;
      W.سائز = (غیرعلامتی)(((پہلا_Byte >> 4) & 0x0F) + 1) * 8;
      واپس W;
    }
    حالت WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_سائز = 5;
      اگر (انحراف + 5 > Pool.سائز_2())
        واپس createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 انحراف);
      W.رجسٹر = (پہلا_Byte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[انحراف + 1]);
      واپس W;
    }
    حالت WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_سائز = 3;
      اگر (انحراف + 3 > Pool.سائز_2())
        واپس createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 انحراف);
      W.رجسٹر = (پہلا_Byte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[انحراف + 1]) * 16;
      واپس W;
    }
    طےشدہ:
      واپس createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               پہلا_Byte, انحراف);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = پہلا_Byte & 0x3F;
  اگر (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_سائز = 2;
    اگر (انحراف + 2 > Pool.سائز_2())
      واپس createStringError("WOD_PUSH2 truncated at offset %u", انحراف);
    uint8_t دوسرا_Byte = Pool[انحراف + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.رجسٹر = ((پہلا_Byte >> 6) & 0x03) | ((دوسرا_Byte & 0x07) << 2);
    W.Register2 = (دوسرا_Byte >> 3) & 0x1F;
    واپس W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  انتخاب (پہلا_Byte) {
  حالت WOD_سیٹ_FPREG: {
    W.Opcode = WOD_سیٹ_FPREG;
    W.Byte_سائز = 2;
    اگر (انحراف + 2 > Pool.سائز_2())
      واپس createStringError("WOD_SET_FPREG truncated at offset %u", انحراف);
    uint8_t دوسرا_Byte = Pool[انحراف + 1];
    W.رجسٹر = دوسرا_Byte & 0x0F; // 4-bit register
    W.Displacement = (غیرعلامتی)((دوسرا_Byte >> 4) & 0x0F) * 16;
    واپس W;
  }
  حالت WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_سائز = 5;
    اگر (انحراف + 5 > Pool.سائز_2())
      واپس createStringError("WOD_ALLOC_HUGE truncated at offset %u", انحراف);
    W.سائز = support::endian::read32le(&Pool[انحراف + 1]);
    واپس W;
  }
  حالت WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_سائز = 3;
    اگر (انحراف + 3 > Pool.سائز_2())
      واپس createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               انحراف);
    W.سائز = (uint32_t)support::endian::read16le(&Pool[انحراف + 1]) * 8;
    واپس W;
  }
  حالت WOD_ڈالو_CANONICAL_چوکھٹا: {
    W.Opcode = WOD_ڈالو_CANONICAL_چوکھٹا;
    W.Byte_سائز = 2;
    اگر (انحراف + 2 > Pool.سائز_2())
      واپس createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", انحراف);
    W.Type = Pool[انحراف + 1];
    واپس W;
  }
  طےشدہ:
    واپس createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             پہلا_Byte, انحراف);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> ڈیٹا_2) {
  اگر (ڈیٹا_2.سائز_2() < 4)
    واپس createStringError("V3 unwind info too short: %zu bytes",
                             ڈیٹا_2.سائز_2());

  DecodedUnwindInfoV3 Info;
  Info.نسخہ = ڈیٹا_2[0] & 0x07;
  Info.Flags = (ڈیٹا_2[0] >> 3) & 0x1F;
  Info.سائز_کا_Prolog = ڈیٹا_2[1];
  Info.Payload_الفاظ = ڈیٹا_2[2];
  Info.نمبر_کا_Ops = ڈیٹا_2[3] & 0x1F;
  Info.نمبر_کا_Epilogs = (ڈیٹا_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  غیرعلامتی انحراف = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  غیرعلامتی Payload_انتہا = 4 + Info.Payload_الفاظ * 2;
  اگر (Payload_انتہا > ڈیٹا_2.سائز_2())
    واپس createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_الفاظ);

  منطقی IsLarge = Info.isLarge();
  اگر (IsLarge) {
    اگر (انحراف >= Payload_انتہا)
      واپس createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_الفاظ);
    Info.سائز_کا_Prolog |= static_cast<uint16_t>(ڈیٹا_2[انحراف]) << 8;
    انحراف += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  برائے (غیرعلامتی I = 0; I < Info.نمبر_کا_Ops; ++I) {
    اگر (IsLarge) {
      اگر (انحراف + 2 > Payload_انتہا)
        واپس createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ڈالو_عقب(support::endian::read16le(&ڈیٹا_2[انحراف]));
      انحراف += 2;
    } ورنہ {
      اگر (انحراف >= Payload_انتہا)
        واپس createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ڈالو_عقب(ڈیٹا_2[انحراف++]);
    }
  }

  // Read epilog descriptors
  int32_t پچھلا_Resolved_انحراف = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  صحیح بنیاد_Epilog_Idx = -1;
  برائے (غیرعلامتی I = 0; I < Info.نمبر_کا_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    اگر (انحراف >= Payload_انتہا)
      واپس createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_عدد_Ops = ڈیٹا_2[انحراف++];
    Epi.Flags = Flags_And_عدد_Ops & 0x07;
    Epi.نمبر_کا_Ops = (Flags_And_عدد_Ops >> 3) & 0x1F;

    اگر (انحراف + 2 > Payload_انتہا)
      واپس createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t Raw_انحراف =
        static_cast<int16_t>(support::endian::read16le(&ڈیٹا_2[انحراف]));
    انحراف += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    اگر (I == 0)
      Epi.Epilog_انحراف = Raw_انحراف;
    ورنہ
      Epi.Epilog_انحراف = پچھلا_Resolved_انحراف + Raw_انحراف;
    پچھلا_Resolved_انحراف = Epi.Epilog_انحراف;

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
    اگر (Epi.نمبر_کا_Ops == 0) {
      اگر (بنیاد_Epilog_Idx >= 0) {
        ثابت DecodedEpilogV3 &بنیاد = Info.Epilogs[بنیاد_Epilog_Idx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.پہلا_Op = بنیاد.پہلا_Op;
        Epi.Ip_انحراف_کا_آخری_Instruction = بنیاد.Ip_انحراف_کا_آخری_Instruction;
        Epi.IpOffsets = بنیاد.IpOffsets;
      } ورنہ {
        Epi.پہلا_Op = 0;
        Epi.Ip_انحراف_کا_آخری_Instruction = 0;
      }
      Info.Epilogs.ڈالو_عقب(std::move(Epi));
      جاری;
    }

    منطقی EpiLarge = Epi.isLarge();

    اگر (انحراف + 2 > Payload_انتہا)
      واپس createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.پہلا_Op = support::endian::read16le(&ڈیٹا_2[انحراف]);
    انحراف += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    اگر (EpiLarge) {
      اگر (انحراف + 2 > Payload_انتہا)
        واپس createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_انحراف_کا_آخری_Instruction = support::endian::read16le(&ڈیٹا_2[انحراف]);
      انحراف += 2;
    } ورنہ {
      اگر (انحراف >= Payload_انتہا)
        واپس createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_انحراف_کا_آخری_Instruction = ڈیٹا_2[انحراف++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    برائے (غیرعلامتی J = 0; J < Epi.نمبر_کا_Ops; ++J) {
      اگر (EpiLarge) {
        اگر (انحراف + 2 > Payload_انتہا)
          واپس createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ڈالو_عقب(support::endian::read16le(&ڈیٹا_2[انحراف]));
        انحراف += 2;
      } ورنہ {
        اگر (انحراف >= Payload_انتہا)
          واپس createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ڈالو_عقب(ڈیٹا_2[انحراف++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    بنیاد_Epilog_Idx = Info.Epilogs.سائز_2();
    Info.Epilogs.ڈالو_عقب(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  اگر (انحراف < Payload_انتہا)
    Info.WODPool = ڈیٹا_2.قاش(انحراف, Payload_انتہا - انحراف);
  ورنہ
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_سائز = align_مقصد(Payload_انتہا, 4);

  واپس Info;
}
