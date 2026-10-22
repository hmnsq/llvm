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

#ضمن "llvm/Support/Win64EH.h"
#ضمن "llvm/Support/Endian.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/MathExtras.h"

ورتوں نطاق llvm;
ورتوں نطاق llvm::Win64EH;

StringRef Win64EH::get_رجسٹر_Name_V3(بنانشانی Reg) {
  ساكن ثابت حرف *ثابت ناں[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  إذا (Reg >= std::ناپ_2(ناں))
    أرجع "<invalid>";
  أرجع ناں[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        بنانشانی وتھ) {
  إذا (وتھ >= Pool.ناپ_2())
    أرجع createStringError("WOD pool overflow at offset %u", وتھ);

  uint8_t پہلا_Byte = Pool[وتھ];
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
  إذا (Low3 >= 4) {
    بدل (Low3) {
    حالة WOD_ڈالو: {
      W.Opcode = WOD_ڈالو;
      W.Byte_ناپ = 1;
      W.رجسٹر = (پہلا_Byte >> 3) & 0x1F; // 5-bit register
      أرجع W;
    }
    حالة WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_ناپ = 5;
      إذا (وتھ + 5 > Pool.ناپ_2())
        أرجع createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 وتھ);
      W.رجسٹر = (پہلا_Byte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[وتھ + 1]);
      أرجع W;
    }
    حالة WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_ناپ = 3;
      إذا (وتھ + 3 > Pool.ناپ_2())
        أرجع createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 وتھ);
      W.رجسٹر = (پہلا_Byte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[وتھ + 1]) * 8;
      أرجع W;
    }
    حالة WOD_ڈالو_CONSECUTIVE_2: {
      W.Opcode = WOD_ڈالو_CONSECUTIVE_2;
      W.Byte_ناپ = 1;
      W.رجسٹر = (پہلا_Byte >> 3) & 0x1F;
      إذا (W.رجسٹر > 30)
        أرجع createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.رجسٹر, وتھ);
      أرجع W;
    }
    افتراضي:
      أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               پہلا_Byte, وتھ);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = پہلا_Byte & 0x0F;
  إذا (Low4 >= 8) {
    بدل (Low4) {
    حالة WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_ناپ = 1;
      W.ناپ = (بنانشانی)(((پہلا_Byte >> 4) & 0x0F) + 1) * 8;
      أرجع W;
    }
    حالة WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_ناپ = 5;
      إذا (وتھ + 5 > Pool.ناپ_2())
        أرجع createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 وتھ);
      W.رجسٹر = (پہلا_Byte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[وتھ + 1]);
      أرجع W;
    }
    حالة WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_ناپ = 3;
      إذا (وتھ + 3 > Pool.ناپ_2())
        أرجع createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 وتھ);
      W.رجسٹر = (پہلا_Byte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[وتھ + 1]) * 16;
      أرجع W;
    }
    افتراضي:
      أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               پہلا_Byte, وتھ);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = پہلا_Byte & 0x3F;
  إذا (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_ناپ = 2;
    إذا (وتھ + 2 > Pool.ناپ_2())
      أرجع createStringError("WOD_PUSH2 truncated at offset %u", وتھ);
    uint8_t دوجا_Byte = Pool[وتھ + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.رجسٹر = ((پہلا_Byte >> 6) & 0x03) | ((دوجا_Byte & 0x07) << 2);
    W.Register2 = (دوجا_Byte >> 3) & 0x1F;
    أرجع W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  بدل (پہلا_Byte) {
  حالة WOD_سیٹ_FPREG: {
    W.Opcode = WOD_سیٹ_FPREG;
    W.Byte_ناپ = 2;
    إذا (وتھ + 2 > Pool.ناپ_2())
      أرجع createStringError("WOD_SET_FPREG truncated at offset %u", وتھ);
    uint8_t دوجا_Byte = Pool[وتھ + 1];
    W.رجسٹر = دوجا_Byte & 0x0F; // 4-bit register
    W.Displacement = (بنانشانی)((دوجا_Byte >> 4) & 0x0F) * 16;
    أرجع W;
  }
  حالة WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_ناپ = 5;
    إذا (وتھ + 5 > Pool.ناپ_2())
      أرجع createStringError("WOD_ALLOC_HUGE truncated at offset %u", وتھ);
    W.ناپ = support::endian::read32le(&Pool[وتھ + 1]);
    أرجع W;
  }
  حالة WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_ناپ = 3;
    إذا (وتھ + 3 > Pool.ناپ_2())
      أرجع createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               وتھ);
    W.ناپ = (uint32_t)support::endian::read16le(&Pool[وتھ + 1]) * 8;
    أرجع W;
  }
  حالة WOD_ڈالو_CANONICAL_چوکھٹا: {
    W.Opcode = WOD_ڈالو_CANONICAL_چوکھٹا;
    W.Byte_ناپ = 2;
    إذا (وتھ + 2 > Pool.ناپ_2())
      أرجع createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", وتھ);
    W.Type = Pool[وتھ + 1];
    أرجع W;
  }
  افتراضي:
    أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             پہلا_Byte, وتھ);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> انکڑے_2) {
  إذا (انکڑے_2.ناپ_2() < 4)
    أرجع createStringError("V3 unwind info too short: %zu bytes",
                             انکڑے_2.ناپ_2());

  DecodedUnwindInfoV3 Info;
  Info.نسخہ = انکڑے_2[0] & 0x07;
  Info.Flags = (انکڑے_2[0] >> 3) & 0x1F;
  Info.ناپ_دا_Prolog = انکڑے_2[1];
  Info.Payload_الفاظ = انکڑے_2[2];
  Info.نمبر_دا_Ops = انکڑے_2[3] & 0x1F;
  Info.نمبر_دا_Epilogs = (انکڑے_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  بنانشانی وتھ = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  بنانشانی Payload_سرا = 4 + Info.Payload_الفاظ * 2;
  إذا (Payload_سرا > انکڑے_2.ناپ_2())
    أرجع createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_الفاظ);

  منطقي IsLarge = Info.isLarge();
  إذا (IsLarge) {
    إذا (وتھ >= Payload_سرا)
      أرجع createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_الفاظ);
    Info.ناپ_دا_Prolog |= static_cast<uint16_t>(انکڑے_2[وتھ]) << 8;
    وتھ += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  لكل (بنانشانی I = 0; I < Info.نمبر_دا_Ops; ++I) {
    إذا (IsLarge) {
      إذا (وتھ + 2 > Payload_سرا)
        أرجع createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ڈالو_پٹھ(support::endian::read16le(&انکڑے_2[وتھ]));
      وتھ += 2;
    } وإلا {
      إذا (وتھ >= Payload_سرا)
        أرجع createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ڈالو_پٹھ(انکڑے_2[وتھ++]);
    }
  }

  // Read epilog descriptors
  int32_t Prev_Resolved_وتھ = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  صحيح ادھار_Epilog_Idx = -1;
  لكل (بنانشانی I = 0; I < Info.نمبر_دا_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    إذا (وتھ >= Payload_سرا)
      أرجع createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_عدد_Ops = انکڑے_2[وتھ++];
    Epi.Flags = Flags_And_عدد_Ops & 0x07;
    Epi.نمبر_دا_Ops = (Flags_And_عدد_Ops >> 3) & 0x1F;

    إذا (وتھ + 2 > Payload_سرا)
      أرجع createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t Raw_وتھ =
        static_cast<int16_t>(support::endian::read16le(&انکڑے_2[وتھ]));
    وتھ += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    إذا (I == 0)
      Epi.Epilog_وتھ = Raw_وتھ;
    وإلا
      Epi.Epilog_وتھ = Prev_Resolved_وتھ + Raw_وتھ;
    Prev_Resolved_وتھ = Epi.Epilog_وتھ;

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
    إذا (Epi.نمبر_دا_Ops == 0) {
      إذا (ادھار_Epilog_Idx >= 0) {
        ثابت DecodedEpilogV3 &ادھار = Info.Epilogs[ادھار_Epilog_Idx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.پہلا_Op = ادھار.پہلا_Op;
        Epi.Ip_وتھ_دا_آخری_Instruction = ادھار.Ip_وتھ_دا_آخری_Instruction;
        Epi.IpOffsets = ادھار.IpOffsets;
      } وإلا {
        Epi.پہلا_Op = 0;
        Epi.Ip_وتھ_دا_آخری_Instruction = 0;
      }
      Info.Epilogs.ڈالو_پٹھ(std::move(Epi));
      استمر;
    }

    منطقي EpiLarge = Epi.isLarge();

    إذا (وتھ + 2 > Payload_سرا)
      أرجع createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.پہلا_Op = support::endian::read16le(&انکڑے_2[وتھ]);
    وتھ += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    إذا (EpiLarge) {
      إذا (وتھ + 2 > Payload_سرا)
        أرجع createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_وتھ_دا_آخری_Instruction = support::endian::read16le(&انکڑے_2[وتھ]);
      وتھ += 2;
    } وإلا {
      إذا (وتھ >= Payload_سرا)
        أرجع createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_وتھ_دا_آخری_Instruction = انکڑے_2[وتھ++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    لكل (بنانشانی J = 0; J < Epi.نمبر_دا_Ops; ++J) {
      إذا (EpiLarge) {
        إذا (وتھ + 2 > Payload_سرا)
          أرجع createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ڈالو_پٹھ(support::endian::read16le(&انکڑے_2[وتھ]));
        وتھ += 2;
      } وإلا {
        إذا (وتھ >= Payload_سرا)
          أرجع createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ڈالو_پٹھ(انکڑے_2[وتھ++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    ادھار_Epilog_Idx = Info.Epilogs.ناپ_2();
    Info.Epilogs.ڈالو_پٹھ(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  إذا (وتھ < Payload_سرا)
    Info.WODPool = انکڑے_2.ٹکڑا(وتھ, Payload_سرا - وتھ);
  وإلا
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_ناپ = align_منزل(Payload_سرا, 4);

  أرجع Info;
}
