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

کارول نطاق llvm;
کارول نطاق llvm::Win64EH;

StringRef Win64EH::get_ثبتوونکی_Name_V3(بینښه Reg) {
  ساكن ثابت حرف *ثابت نومونه[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  إذا (Reg >= std::اندازه_2(نومونه))
    أرجع "<invalid>";
  أرجع نومونه[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        بینښه دننه_واټن) {
  إذا (دننه_واټن >= Pool.اندازه_2())
    أرجع createStringError("WOD pool overflow at offset %u", دننه_واټن);

  uint8_t لومړی_Byte = Pool[دننه_واټن];
  DecodedWOD W = {};

  // Determine opcode from variable-width prefix encoding.
  // The dispatch order matters: check shorter prefixes first since they
  // occupy the lowest bits, then fall through to longer prefixes.
  //   3-bit prefix (bits [2:0] >= 4): opcodes 4-7
  //   4-bit prefix (bits [3:0] >= 8): opcodes 8-10
  //   6-bit prefix (bits [5:0] == 0x20): opcode 32 (PUSH2)
  //   8-bit prefix (full byte 0-3): opcodes 0-3
  uint8_t Low3 = لومړی_Byte & 0x07;

  // 3-bit opcode: bits [2:0] in {4, 5, 6, 7}
  إذا (Low3 >= 4) {
    بدل (Low3) {
    حالة WOD_ټېل: {
      W.Opcode = WOD_ټېل;
      W.Byte_اندازه = 1;
      W.ثبتوونکی = (لومړی_Byte >> 3) & 0x1F; // 5-bit register
      أرجع W;
    }
    حالة WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_اندازه = 5;
      إذا (دننه_واټن + 5 > Pool.اندازه_2())
        أرجع createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 دننه_واټن);
      W.ثبتوونکی = (لومړی_Byte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[دننه_واټن + 1]);
      أرجع W;
    }
    حالة WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_اندازه = 3;
      إذا (دننه_واټن + 3 > Pool.اندازه_2())
        أرجع createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 دننه_واټن);
      W.ثبتوونکی = (لومړی_Byte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[دننه_واټن + 1]) * 8;
      أرجع W;
    }
    حالة WOD_ټېل_CONSECUTIVE_2: {
      W.Opcode = WOD_ټېل_CONSECUTIVE_2;
      W.Byte_اندازه = 1;
      W.ثبتوونکی = (لومړی_Byte >> 3) & 0x1F;
      إذا (W.ثبتوونکی > 30)
        أرجع createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.ثبتوونکی, دننه_واټن);
      أرجع W;
    }
    افتراضي:
      أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               لومړی_Byte, دننه_واټن);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = لومړی_Byte & 0x0F;
  إذا (Low4 >= 8) {
    بدل (Low4) {
    حالة WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_اندازه = 1;
      W.اندازه = (بینښه)(((لومړی_Byte >> 4) & 0x0F) + 1) * 8;
      أرجع W;
    }
    حالة WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_اندازه = 5;
      إذا (دننه_واټن + 5 > Pool.اندازه_2())
        أرجع createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 دننه_واټن);
      W.ثبتوونکی = (لومړی_Byte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[دننه_واټن + 1]);
      أرجع W;
    }
    حالة WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_اندازه = 3;
      إذا (دننه_واټن + 3 > Pool.اندازه_2())
        أرجع createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 دننه_واټن);
      W.ثبتوونکی = (لومړی_Byte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[دننه_واټن + 1]) * 16;
      أرجع W;
    }
    افتراضي:
      أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               لومړی_Byte, دننه_واټن);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = لومړی_Byte & 0x3F;
  إذا (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_اندازه = 2;
    إذا (دننه_واټن + 2 > Pool.اندازه_2())
      أرجع createStringError("WOD_PUSH2 truncated at offset %u", دننه_واټن);
    uint8_t دوهم_Byte = Pool[دننه_واټن + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.ثبتوونکی = ((لومړی_Byte >> 6) & 0x03) | ((دوهم_Byte & 0x07) << 2);
    W.Register2 = (دوهم_Byte >> 3) & 0x1F;
    أرجع W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  بدل (لومړی_Byte) {
  حالة WOD_سېټ_FPREG: {
    W.Opcode = WOD_سېټ_FPREG;
    W.Byte_اندازه = 2;
    إذا (دننه_واټن + 2 > Pool.اندازه_2())
      أرجع createStringError("WOD_SET_FPREG truncated at offset %u", دننه_واټن);
    uint8_t دوهم_Byte = Pool[دننه_واټن + 1];
    W.ثبتوونکی = دوهم_Byte & 0x0F; // 4-bit register
    W.Displacement = (بینښه)((دوهم_Byte >> 4) & 0x0F) * 16;
    أرجع W;
  }
  حالة WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_اندازه = 5;
    إذا (دننه_واټن + 5 > Pool.اندازه_2())
      أرجع createStringError("WOD_ALLOC_HUGE truncated at offset %u", دننه_واټن);
    W.اندازه = support::endian::read32le(&Pool[دننه_واټن + 1]);
    أرجع W;
  }
  حالة WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_اندازه = 3;
    إذا (دننه_واټن + 3 > Pool.اندازه_2())
      أرجع createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               دننه_واټن);
    W.اندازه = (uint32_t)support::endian::read16le(&Pool[دننه_واټن + 1]) * 8;
    أرجع W;
  }
  حالة WOD_ټېل_CANONICAL_چوکاټ: {
    W.Opcode = WOD_ټېل_CANONICAL_چوکاټ;
    W.Byte_اندازه = 2;
    إذا (دننه_واټن + 2 > Pool.اندازه_2())
      أرجع createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", دننه_واټن);
    W.Type = Pool[دننه_واټن + 1];
    أرجع W;
  }
  افتراضي:
    أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             لومړی_Byte, دننه_واټن);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> معلومات_2) {
  إذا (معلومات_2.اندازه_2() < 4)
    أرجع createStringError("V3 unwind info too short: %zu bytes",
                             معلومات_2.اندازه_2());

  DecodedUnwindInfoV3 Info;
  Info.نسخه = معلومات_2[0] & 0x07;
  Info.Flags = (معلومات_2[0] >> 3) & 0x1F;
  Info.اندازه_د_Prolog = معلومات_2[1];
  Info.Payload_ټکي = معلومات_2[2];
  Info.شمېره_د_Ops = معلومات_2[3] & 0x1F;
  Info.شمېره_د_Epilogs = (معلومات_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  بینښه دننه_واټن = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  بینښه Payload_پای_ټکی = 4 + Info.Payload_ټکي * 2;
  إذا (Payload_پای_ټکی > معلومات_2.اندازه_2())
    أرجع createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_ټکي);

  منطقي IsLarge = Info.isLarge();
  إذا (IsLarge) {
    إذا (دننه_واټن >= Payload_پای_ټکی)
      أرجع createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_ټکي);
    Info.اندازه_د_Prolog |= static_cast<uint16_t>(معلومات_2[دننه_واټن]) << 8;
    دننه_واټن += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  لكل (بینښه I = 0; I < Info.شمېره_د_Ops; ++I) {
    إذا (IsLarge) {
      إذا (دننه_واټن + 2 > Payload_پای_ټکی)
        أرجع createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ټېل_شا(support::endian::read16le(&معلومات_2[دننه_واټن]));
      دننه_واټن += 2;
    } وإلا {
      إذا (دننه_واټن >= Payload_پای_ټکی)
        أرجع createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ټېل_شا(معلومات_2[دننه_واټن++]);
    }
  }

  // Read epilog descriptors
  int32_t Prev_Resolved_دننه_واټن = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  صحيح بنسټ_Epilog_Idx = -1;
  لكل (بینښه I = 0; I < Info.شمېره_د_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    إذا (دننه_واټن >= Payload_پای_ټکی)
      أرجع createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_عدد_Ops = معلومات_2[دننه_واټن++];
    Epi.Flags = Flags_And_عدد_Ops & 0x07;
    Epi.شمېره_د_Ops = (Flags_And_عدد_Ops >> 3) & 0x1F;

    إذا (دننه_واټن + 2 > Payload_پای_ټکی)
      أرجع createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t Raw_دننه_واټن =
        static_cast<int16_t>(support::endian::read16le(&معلومات_2[دننه_واټن]));
    دننه_واټن += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    إذا (I == 0)
      Epi.Epilog_دننه_واټن = Raw_دننه_واټن;
    وإلا
      Epi.Epilog_دننه_واټن = Prev_Resolved_دننه_واټن + Raw_دننه_واټن;
    Prev_Resolved_دننه_واټن = Epi.Epilog_دننه_واټن;

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
    إذا (Epi.شمېره_د_Ops == 0) {
      إذا (بنسټ_Epilog_Idx >= 0) {
        ثابت DecodedEpilogV3 &بنسټ = Info.Epilogs[بنسټ_Epilog_Idx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.لومړی_Op = بنسټ.لومړی_Op;
        Epi.Ip_دننه_واټن_د_وروستی_Instruction = بنسټ.Ip_دننه_واټن_د_وروستی_Instruction;
        Epi.IpOffsets = بنسټ.IpOffsets;
      } وإلا {
        Epi.لومړی_Op = 0;
        Epi.Ip_دننه_واټن_د_وروستی_Instruction = 0;
      }
      Info.Epilogs.ټېل_شا(std::move(Epi));
      استمر;
    }

    منطقي EpiLarge = Epi.isLarge();

    إذا (دننه_واټن + 2 > Payload_پای_ټکی)
      أرجع createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.لومړی_Op = support::endian::read16le(&معلومات_2[دننه_واټن]);
    دننه_واټن += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    إذا (EpiLarge) {
      إذا (دننه_واټن + 2 > Payload_پای_ټکی)
        أرجع createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_دننه_واټن_د_وروستی_Instruction = support::endian::read16le(&معلومات_2[دننه_واټن]);
      دننه_واټن += 2;
    } وإلا {
      إذا (دننه_واټن >= Payload_پای_ټکی)
        أرجع createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_دننه_واټن_د_وروستی_Instruction = معلومات_2[دننه_واټن++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    لكل (بینښه J = 0; J < Epi.شمېره_د_Ops; ++J) {
      إذا (EpiLarge) {
        إذا (دننه_واټن + 2 > Payload_پای_ټکی)
          أرجع createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ټېل_شا(support::endian::read16le(&معلومات_2[دننه_واټن]));
        دننه_واټن += 2;
      } وإلا {
        إذا (دننه_واټن >= Payload_پای_ټکی)
          أرجع createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ټېل_شا(معلومات_2[دننه_واټن++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    بنسټ_Epilog_Idx = Info.Epilogs.اندازه_2();
    Info.Epilogs.ټېل_شا(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  إذا (دننه_واټن < Payload_پای_ټکی)
    Info.WODPool = معلومات_2.ټوټه(دننه_واټن, Payload_پای_ټکی - دننه_واټن);
  وإلا
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_اندازه = align_منزل(Payload_پای_ټکی, 4);

  أرجع Info;
}
