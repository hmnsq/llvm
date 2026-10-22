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

استفاده نطاق llvm;
استفاده نطاق llvm::Win64EH;

StringRef Win64EH::get_ثبات_Name_V3(بیعلامت Reg) {
  ساكن ثابت حرف *ثابت نامها[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  إذا (Reg >= std::اندازه_2(نامها))
    أرجع "<invalid>";
  أرجع نامها[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        بیعلامت جابه_جایی) {
  إذا (جابه_جایی >= Pool.اندازه_2())
    أرجع createStringError("WOD pool overflow at offset %u", جابه_جایی);

  uint8_t اولی_Byte = Pool[جابه_جایی];
  DecodedWOD W = {};

  // Determine opcode from variable-width prefix encoding.
  // The dispatch order matters: check shorter prefixes first since they
  // occupy the lowest bits, then fall through to longer prefixes.
  //   3-bit prefix (bits [2:0] >= 4): opcodes 4-7
  //   4-bit prefix (bits [3:0] >= 8): opcodes 8-10
  //   6-bit prefix (bits [5:0] == 0x20): opcode 32 (PUSH2)
  //   8-bit prefix (full byte 0-3): opcodes 0-3
  uint8_t Low3 = اولی_Byte & 0x07;

  // 3-bit opcode: bits [2:0] in {4, 5, 6, 7}
  إذا (Low3 >= 4) {
    بدل (Low3) {
    حالة WOD_افزودن: {
      W.Opcode = WOD_افزودن;
      W.Byte_اندازه = 1;
      W.ثبات = (اولی_Byte >> 3) & 0x1F; // 5-bit register
      أرجع W;
    }
    حالة WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_اندازه = 5;
      إذا (جابه_جایی + 5 > Pool.اندازه_2())
        أرجع createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 جابه_جایی);
      W.ثبات = (اولی_Byte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[جابه_جایی + 1]);
      أرجع W;
    }
    حالة WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_اندازه = 3;
      إذا (جابه_جایی + 3 > Pool.اندازه_2())
        أرجع createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 جابه_جایی);
      W.ثبات = (اولی_Byte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[جابه_جایی + 1]) * 8;
      أرجع W;
    }
    حالة WOD_افزودن_CONSECUTIVE_2: {
      W.Opcode = WOD_افزودن_CONSECUTIVE_2;
      W.Byte_اندازه = 1;
      W.ثبات = (اولی_Byte >> 3) & 0x1F;
      إذا (W.ثبات > 30)
        أرجع createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.ثبات, جابه_جایی);
      أرجع W;
    }
    افتراضي:
      أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               اولی_Byte, جابه_جایی);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = اولی_Byte & 0x0F;
  إذا (Low4 >= 8) {
    بدل (Low4) {
    حالة WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_اندازه = 1;
      W.اندازه = (بیعلامت)(((اولی_Byte >> 4) & 0x0F) + 1) * 8;
      أرجع W;
    }
    حالة WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_اندازه = 5;
      إذا (جابه_جایی + 5 > Pool.اندازه_2())
        أرجع createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 جابه_جایی);
      W.ثبات = (اولی_Byte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[جابه_جایی + 1]);
      أرجع W;
    }
    حالة WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_اندازه = 3;
      إذا (جابه_جایی + 3 > Pool.اندازه_2())
        أرجع createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 جابه_جایی);
      W.ثبات = (اولی_Byte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[جابه_جایی + 1]) * 16;
      أرجع W;
    }
    افتراضي:
      أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               اولی_Byte, جابه_جایی);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = اولی_Byte & 0x3F;
  إذا (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_اندازه = 2;
    إذا (جابه_جایی + 2 > Pool.اندازه_2())
      أرجع createStringError("WOD_PUSH2 truncated at offset %u", جابه_جایی);
    uint8_t دومی_Byte = Pool[جابه_جایی + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.ثبات = ((اولی_Byte >> 6) & 0x03) | ((دومی_Byte & 0x07) << 2);
    W.Register2 = (دومی_Byte >> 3) & 0x1F;
    أرجع W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  بدل (اولی_Byte) {
  حالة WOD_مجموعه_FPREG: {
    W.Opcode = WOD_مجموعه_FPREG;
    W.Byte_اندازه = 2;
    إذا (جابه_جایی + 2 > Pool.اندازه_2())
      أرجع createStringError("WOD_SET_FPREG truncated at offset %u", جابه_جایی);
    uint8_t دومی_Byte = Pool[جابه_جایی + 1];
    W.ثبات = دومی_Byte & 0x0F; // 4-bit register
    W.Displacement = (بیعلامت)((دومی_Byte >> 4) & 0x0F) * 16;
    أرجع W;
  }
  حالة WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_اندازه = 5;
    إذا (جابه_جایی + 5 > Pool.اندازه_2())
      أرجع createStringError("WOD_ALLOC_HUGE truncated at offset %u", جابه_جایی);
    W.اندازه = support::endian::read32le(&Pool[جابه_جایی + 1]);
    أرجع W;
  }
  حالة WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_اندازه = 3;
    إذا (جابه_جایی + 3 > Pool.اندازه_2())
      أرجع createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               جابه_جایی);
    W.اندازه = (uint32_t)support::endian::read16le(&Pool[جابه_جایی + 1]) * 8;
    أرجع W;
  }
  حالة WOD_افزودن_CANONICAL_قاب: {
    W.Opcode = WOD_افزودن_CANONICAL_قاب;
    W.Byte_اندازه = 2;
    إذا (جابه_جایی + 2 > Pool.اندازه_2())
      أرجع createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", جابه_جایی);
    W.Type = Pool[جابه_جایی + 1];
    أرجع W;
  }
  افتراضي:
    أرجع createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             اولی_Byte, جابه_جایی);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> داده_2) {
  إذا (داده_2.اندازه_2() < 4)
    أرجع createStringError("V3 unwind info too short: %zu bytes",
                             داده_2.اندازه_2());

  DecodedUnwindInfoV3 Info;
  Info.نسخه = داده_2[0] & 0x07;
  Info.Flags = (داده_2[0] >> 3) & 0x1F;
  Info.اندازه_از_Prolog = داده_2[1];
  Info.Payload_واژگان = داده_2[2];
  Info.عدد_از_Ops = داده_2[3] & 0x1F;
  Info.عدد_از_Epilogs = (داده_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  بیعلامت جابه_جایی = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  بیعلامت Payload_انتها = 4 + Info.Payload_واژگان * 2;
  إذا (Payload_انتها > داده_2.اندازه_2())
    أرجع createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_واژگان);

  منطقي IsLarge = Info.isLarge();
  إذا (IsLarge) {
    إذا (جابه_جایی >= Payload_انتها)
      أرجع createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_واژگان);
    Info.اندازه_از_Prolog |= static_cast<uint16_t>(داده_2[جابه_جایی]) << 8;
    جابه_جایی += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  لكل (بیعلامت I = 0; I < Info.عدد_از_Ops; ++I) {
    إذا (IsLarge) {
      إذا (جابه_جایی + 2 > Payload_انتها)
        أرجع createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.افزودن_پشت(support::endian::read16le(&داده_2[جابه_جایی]));
      جابه_جایی += 2;
    } وإلا {
      إذا (جابه_جایی >= Payload_انتها)
        أرجع createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.افزودن_پشت(داده_2[جابه_جایی++]);
    }
  }

  // Read epilog descriptors
  int32_t Prev_Resolved_جابه_جایی = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  صحيح پایه_Epilog_Idx = -1;
  لكل (بیعلامت I = 0; I < Info.عدد_از_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    إذا (جابه_جایی >= Payload_انتها)
      أرجع createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_رقم_Ops = داده_2[جابه_جایی++];
    Epi.Flags = Flags_And_رقم_Ops & 0x07;
    Epi.عدد_از_Ops = (Flags_And_رقم_Ops >> 3) & 0x1F;

    إذا (جابه_جایی + 2 > Payload_انتها)
      أرجع createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t Raw_جابه_جایی =
        static_cast<int16_t>(support::endian::read16le(&داده_2[جابه_جایی]));
    جابه_جایی += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    إذا (I == 0)
      Epi.Epilog_جابه_جایی = Raw_جابه_جایی;
    وإلا
      Epi.Epilog_جابه_جایی = Prev_Resolved_جابه_جایی + Raw_جابه_جایی;
    Prev_Resolved_جابه_جایی = Epi.Epilog_جابه_جایی;

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
    إذا (Epi.عدد_از_Ops == 0) {
      إذا (پایه_Epilog_Idx >= 0) {
        ثابت DecodedEpilogV3 &پایه = Info.Epilogs[پایه_Epilog_Idx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.اولی_Op = پایه.اولی_Op;
        Epi.Ip_جابه_جایی_از_آخرین_Instruction = پایه.Ip_جابه_جایی_از_آخرین_Instruction;
        Epi.IpOffsets = پایه.IpOffsets;
      } وإلا {
        Epi.اولی_Op = 0;
        Epi.Ip_جابه_جایی_از_آخرین_Instruction = 0;
      }
      Info.Epilogs.افزودن_پشت(std::move(Epi));
      استمر;
    }

    منطقي EpiLarge = Epi.isLarge();

    إذا (جابه_جایی + 2 > Payload_انتها)
      أرجع createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.اولی_Op = support::endian::read16le(&داده_2[جابه_جایی]);
    جابه_جایی += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    إذا (EpiLarge) {
      إذا (جابه_جایی + 2 > Payload_انتها)
        أرجع createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_جابه_جایی_از_آخرین_Instruction = support::endian::read16le(&داده_2[جابه_جایی]);
      جابه_جایی += 2;
    } وإلا {
      إذا (جابه_جایی >= Payload_انتها)
        أرجع createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_جابه_جایی_از_آخرین_Instruction = داده_2[جابه_جایی++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    لكل (بیعلامت J = 0; J < Epi.عدد_از_Ops; ++J) {
      إذا (EpiLarge) {
        إذا (جابه_جایی + 2 > Payload_انتها)
          أرجع createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.افزودن_پشت(support::endian::read16le(&داده_2[جابه_جایی]));
        جابه_جایی += 2;
      } وإلا {
        إذا (جابه_جایی >= Payload_انتها)
          أرجع createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.افزودن_پشت(داده_2[جابه_جایی++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    پایه_Epilog_Idx = Info.Epilogs.اندازه_2();
    Info.Epilogs.افزودن_پشت(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  إذا (جابه_جایی < Payload_انتها)
    Info.WODPool = داده_2.برش(جابه_جایی, Payload_انتها - جابه_جایی);
  وإلا
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_اندازه = align_مقصد(Payload_انتها, 4);

  أرجع Info;
}
