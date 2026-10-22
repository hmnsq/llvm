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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Win64EH.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Endian.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::Win64EH;

StringRef Win64EH::get_ರಿಜಿಸ್ಟರ್_Name_V3(ಚಿಹ್ನೆರಹಿತ Reg) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಹೆಸರುಗಳು[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  ಆದರೆ (Reg >= std::ಗಾತ್ರ_3(ಹೆಸರುಗಳು))
    ಹಿಂತಿರುಗಿ "<invalid>";
  ಹಿಂತಿರುಗಿ ಹೆಸರುಗಳು[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ) {
  ಆದರೆ (ಸ್ಥಾನಾಂತರ >= Pool.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ createStringError("WOD pool overflow at offset %u", ಸ್ಥಾನಾಂತರ);

  uint8_t ಮೊದಲ_Byte = Pool[ಸ್ಥಾನಾಂತರ];
  DecodedWOD W = {};

  // Determine opcode from variable-width prefix encoding.
  // The dispatch order matters: check shorter prefixes first since they
  // occupy the lowest bits, then fall through to longer prefixes.
  //   3-bit prefix (bits [2:0] >= 4): opcodes 4-7
  //   4-bit prefix (bits [3:0] >= 8): opcodes 8-10
  //   6-bit prefix (bits [5:0] == 0x20): opcode 32 (PUSH2)
  //   8-bit prefix (full byte 0-3): opcodes 0-3
  uint8_t Low3 = ಮೊದಲ_Byte & 0x07;

  // 3-bit opcode: bits [2:0] in {4, 5, 6, 7}
  ಆದರೆ (Low3 >= 4) {
    ಆಯ್ಕೆ (Low3) {
    ಸ್ಥಿತಿ WOD_ತಳ್ಳು: {
      W.Opcode = WOD_ತಳ್ಳು;
      W.Byte_ಗಾತ್ರ = 1;
      W.ರಿಜಿಸ್ಟರ್ = (ಮೊದಲ_Byte >> 3) & 0x1F; // 5-bit register
      ಹಿಂತಿರುಗಿ W;
    }
    ಸ್ಥಿತಿ WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_ಗಾತ್ರ = 5;
      ಆದರೆ (ಸ್ಥಾನಾಂತರ + 5 > Pool.ಗಾತ್ರ_3())
        ಹಿಂತಿರುಗಿ createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 ಸ್ಥಾನಾಂತರ);
      W.ರಿಜಿಸ್ಟರ್ = (ಮೊದಲ_Byte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[ಸ್ಥಾನಾಂತರ + 1]);
      ಹಿಂತಿರುಗಿ W;
    }
    ಸ್ಥಿತಿ WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_ಗಾತ್ರ = 3;
      ಆದರೆ (ಸ್ಥಾನಾಂತರ + 3 > Pool.ಗಾತ್ರ_3())
        ಹಿಂತಿರುಗಿ createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 ಸ್ಥಾನಾಂತರ);
      W.ರಿಜಿಸ್ಟರ್ = (ಮೊದಲ_Byte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[ಸ್ಥಾನಾಂತರ + 1]) * 8;
      ಹಿಂತಿರುಗಿ W;
    }
    ಸ್ಥಿತಿ WOD_ತಳ್ಳು_CONSECUTIVE_2: {
      W.Opcode = WOD_ತಳ್ಳು_CONSECUTIVE_2;
      W.Byte_ಗಾತ್ರ = 1;
      W.ರಿಜಿಸ್ಟರ್ = (ಮೊದಲ_Byte >> 3) & 0x1F;
      ಆದರೆ (W.ರಿಜಿಸ್ಟರ್ > 30)
        ಹಿಂತಿರುಗಿ createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.ರಿಜಿಸ್ಟರ್, ಸ್ಥಾನಾಂತರ);
      ಹಿಂತಿರುಗಿ W;
    }
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               ಮೊದಲ_Byte, ಸ್ಥಾನಾಂತರ);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = ಮೊದಲ_Byte & 0x0F;
  ಆದರೆ (Low4 >= 8) {
    ಆಯ್ಕೆ (Low4) {
    ಸ್ಥಿತಿ WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_ಗಾತ್ರ = 1;
      W.ಗಾತ್ರ_2 = (ಚಿಹ್ನೆರಹಿತ)(((ಮೊದಲ_Byte >> 4) & 0x0F) + 1) * 8;
      ಹಿಂತಿರುಗಿ W;
    }
    ಸ್ಥಿತಿ WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_ಗಾತ್ರ = 5;
      ಆದರೆ (ಸ್ಥಾನಾಂತರ + 5 > Pool.ಗಾತ್ರ_3())
        ಹಿಂತಿರುಗಿ createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 ಸ್ಥಾನಾಂತರ);
      W.ರಿಜಿಸ್ಟರ್ = (ಮೊದಲ_Byte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[ಸ್ಥಾನಾಂತರ + 1]);
      ಹಿಂತಿರುಗಿ W;
    }
    ಸ್ಥಿತಿ WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_ಗಾತ್ರ = 3;
      ಆದರೆ (ಸ್ಥಾನಾಂತರ + 3 > Pool.ಗಾತ್ರ_3())
        ಹಿಂತಿರುಗಿ createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 ಸ್ಥಾನಾಂತರ);
      W.ರಿಜಿಸ್ಟರ್ = (ಮೊದಲ_Byte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[ಸ್ಥಾನಾಂತರ + 1]) * 16;
      ಹಿಂತಿರುಗಿ W;
    }
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               ಮೊದಲ_Byte, ಸ್ಥಾನಾಂತರ);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = ಮೊದಲ_Byte & 0x3F;
  ಆದರೆ (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_ಗಾತ್ರ = 2;
    ಆದರೆ (ಸ್ಥಾನಾಂತರ + 2 > Pool.ಗಾತ್ರ_3())
      ಹಿಂತಿರುಗಿ createStringError("WOD_PUSH2 truncated at offset %u", ಸ್ಥಾನಾಂತರ);
    uint8_t ಎರಡನೇ_Byte = Pool[ಸ್ಥಾನಾಂತರ + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.ರಿಜಿಸ್ಟರ್ = ((ಮೊದಲ_Byte >> 6) & 0x03) | ((ಎರಡನೇ_Byte & 0x07) << 2);
    W.Register2 = (ಎರಡನೇ_Byte >> 3) & 0x1F;
    ಹಿಂತಿರುಗಿ W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  ಆಯ್ಕೆ (ಮೊದಲ_Byte) {
  ಸ್ಥಿತಿ WOD_ಗಣ_FPREG: {
    W.Opcode = WOD_ಗಣ_FPREG;
    W.Byte_ಗಾತ್ರ = 2;
    ಆದರೆ (ಸ್ಥಾನಾಂತರ + 2 > Pool.ಗಾತ್ರ_3())
      ಹಿಂತಿರುಗಿ createStringError("WOD_SET_FPREG truncated at offset %u", ಸ್ಥಾನಾಂತರ);
    uint8_t ಎರಡನೇ_Byte = Pool[ಸ್ಥಾನಾಂತರ + 1];
    W.ರಿಜಿಸ್ಟರ್ = ಎರಡನೇ_Byte & 0x0F; // 4-bit register
    W.Displacement = (ಚಿಹ್ನೆರಹಿತ)((ಎರಡನೇ_Byte >> 4) & 0x0F) * 16;
    ಹಿಂತಿರುಗಿ W;
  }
  ಸ್ಥಿತಿ WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_ಗಾತ್ರ = 5;
    ಆದರೆ (ಸ್ಥಾನಾಂತರ + 5 > Pool.ಗಾತ್ರ_3())
      ಹಿಂತಿರುಗಿ createStringError("WOD_ALLOC_HUGE truncated at offset %u", ಸ್ಥಾನಾಂತರ);
    W.ಗಾತ್ರ_2 = support::endian::read32le(&Pool[ಸ್ಥಾನಾಂತರ + 1]);
    ಹಿಂತಿರುಗಿ W;
  }
  ಸ್ಥಿತಿ WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_ಗಾತ್ರ = 3;
    ಆದರೆ (ಸ್ಥಾನಾಂತರ + 3 > Pool.ಗಾತ್ರ_3())
      ಹಿಂತಿರುಗಿ createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               ಸ್ಥಾನಾಂತರ);
    W.ಗಾತ್ರ_2 = (uint32_t)support::endian::read16le(&Pool[ಸ್ಥಾನಾಂತರ + 1]) * 8;
    ಹಿಂತಿರುಗಿ W;
  }
  ಸ್ಥಿತಿ WOD_ತಳ್ಳು_CANONICAL_ಚೌಕಟ್ಟು: {
    W.Opcode = WOD_ತಳ್ಳು_CANONICAL_ಚೌಕಟ್ಟು;
    W.Byte_ಗಾತ್ರ = 2;
    ಆದರೆ (ಸ್ಥಾನಾಂತರ + 2 > Pool.ಗಾತ್ರ_3())
      ಹಿಂತಿರುಗಿ createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", ಸ್ಥಾನಾಂತರ);
    W.Type = Pool[ಸ್ಥಾನಾಂತರ + 1];
    ಹಿಂತಿರುಗಿ W;
  }
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             ಮೊದಲ_Byte, ಸ್ಥಾನಾಂತರ);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> ದತ್ತಾಂಶ_2) {
  ಆದರೆ (ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3() < 4)
    ಹಿಂತಿರುಗಿ createStringError("V3 unwind info too short: %zu bytes",
                             ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3());

  DecodedUnwindInfoV3 Info;
  Info.ಆವೃತ್ತಿ = ದತ್ತಾಂಶ_2[0] & 0x07;
  Info.Flags = (ದತ್ತಾಂಶ_2[0] >> 3) & 0x1F;
  Info.ಗಾತ್ರ_ದ_Prolog = ದತ್ತಾಂಶ_2[1];
  Info.Payload_ಪದಗಳು = ದತ್ತಾಂಶ_2[2];
  Info.ಸಂಖ್ಯೆ_ದ_Ops = ದತ್ತಾಂಶ_2[3] & 0x1F;
  Info.ಸಂಖ್ಯೆ_ದ_Epilogs = (ದತ್ತಾಂಶ_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  ಚಿಹ್ನೆರಹಿತ Payload_ಕೊನೆ = 4 + Info.Payload_ಪದಗಳು * 2;
  ಆದರೆ (Payload_ಕೊನೆ > ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_ಪದಗಳು);

  ತಾರ್ಕಿಕ IsLarge = Info.isLarge();
  ಆದರೆ (IsLarge) {
    ಆದರೆ (ಸ್ಥಾನಾಂತರ >= Payload_ಕೊನೆ)
      ಹಿಂತಿರುಗಿ createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_ಪದಗಳು);
    Info.ಗಾತ್ರ_ದ_Prolog |= static_cast<uint16_t>(ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ]) << 8;
    ಸ್ಥಾನಾಂತರ += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < Info.ಸಂಖ್ಯೆ_ದ_Ops; ++I) {
    ಆದರೆ (IsLarge) {
      ಆದರೆ (ಸ್ಥಾನಾಂತರ + 2 > Payload_ಕೊನೆ)
        ಹಿಂತಿರುಗಿ createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ತಳ್ಳು_ಹಿಂಭಾಗ(support::endian::read16le(&ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ]));
      ಸ್ಥಾನಾಂತರ += 2;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (ಸ್ಥಾನಾಂತರ >= Payload_ಕೊನೆ)
        ಹಿಂತಿರುಗಿ createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ತಳ್ಳು_ಹಿಂಭಾಗ(ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ++]);
    }
  }

  // Read epilog descriptors
  int32_t Prev_Resolved_ಸ್ಥಾನಾಂತರ = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  ಪೂರ್ಣಾಂಕ ತಳ_Epilog_Idx = -1;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < Info.ಸಂಖ್ಯೆ_ದ_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    ಆದರೆ (ಸ್ಥಾನಾಂತರ >= Payload_ಕೊನೆ)
      ಹಿಂತಿರುಗಿ createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_ಅಂಕೆ_Ops = ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ++];
    Epi.Flags = Flags_And_ಅಂಕೆ_Ops & 0x07;
    Epi.ಸಂಖ್ಯೆ_ದ_Ops = (Flags_And_ಅಂಕೆ_Ops >> 3) & 0x1F;

    ಆದರೆ (ಸ್ಥಾನಾಂತರ + 2 > Payload_ಕೊನೆ)
      ಹಿಂತಿರುಗಿ createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t Raw_ಸ್ಥಾನಾಂತರ =
        static_cast<int16_t>(support::endian::read16le(&ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ]));
    ಸ್ಥಾನಾಂತರ += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    ಆದರೆ (I == 0)
      Epi.Epilog_ಸ್ಥಾನಾಂತರ = Raw_ಸ್ಥಾನಾಂತರ;
    ಇಲ್ಲದಿದ್ದರೆ
      Epi.Epilog_ಸ್ಥಾನಾಂತರ = Prev_Resolved_ಸ್ಥಾನಾಂತರ + Raw_ಸ್ಥಾನಾಂತರ;
    Prev_Resolved_ಸ್ಥಾನಾಂತರ = Epi.Epilog_ಸ್ಥಾನಾಂತರ;

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
    ಆದರೆ (Epi.ಸಂಖ್ಯೆ_ದ_Ops == 0) {
      ಆದರೆ (ತಳ_Epilog_Idx >= 0) {
        ಸ್ಥಿರಾಂಕ DecodedEpilogV3 &ತಳ = Info.Epilogs[ತಳ_Epilog_Idx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.ಮೊದಲ_Op = ತಳ.ಮೊದಲ_Op;
        Epi.Ip_ಸ್ಥಾನಾಂತರ_ದ_ಕೊನೆಯ_Instruction = ತಳ.Ip_ಸ್ಥಾನಾಂತರ_ದ_ಕೊನೆಯ_Instruction;
        Epi.IpOffsets = ತಳ.IpOffsets;
      } ಇಲ್ಲದಿದ್ದರೆ {
        Epi.ಮೊದಲ_Op = 0;
        Epi.Ip_ಸ್ಥಾನಾಂತರ_ದ_ಕೊನೆಯ_Instruction = 0;
      }
      Info.Epilogs.ತಳ್ಳು_ಹಿಂಭಾಗ(std::move(Epi));
      ಮುಂದುವರಿ;
    }

    ತಾರ್ಕಿಕ EpiLarge = Epi.isLarge();

    ಆದರೆ (ಸ್ಥಾನಾಂತರ + 2 > Payload_ಕೊನೆ)
      ಹಿಂತಿರುಗಿ createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.ಮೊದಲ_Op = support::endian::read16le(&ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ]);
    ಸ್ಥಾನಾಂತರ += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    ಆದರೆ (EpiLarge) {
      ಆದರೆ (ಸ್ಥಾನಾಂತರ + 2 > Payload_ಕೊನೆ)
        ಹಿಂತಿರುಗಿ createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_ಸ್ಥಾನಾಂತರ_ದ_ಕೊನೆಯ_Instruction = support::endian::read16le(&ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ]);
      ಸ್ಥಾನಾಂತರ += 2;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (ಸ್ಥಾನಾಂತರ >= Payload_ಕೊನೆ)
        ಹಿಂತಿರುಗಿ createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_ಸ್ಥಾನಾಂತರ_ದ_ಕೊನೆಯ_Instruction = ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ J = 0; J < Epi.ಸಂಖ್ಯೆ_ದ_Ops; ++J) {
      ಆದರೆ (EpiLarge) {
        ಆದರೆ (ಸ್ಥಾನಾಂತರ + 2 > Payload_ಕೊನೆ)
          ಹಿಂತಿರುಗಿ createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ತಳ್ಳು_ಹಿಂಭಾಗ(support::endian::read16le(&ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ]));
        ಸ್ಥಾನಾಂತರ += 2;
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (ಸ್ಥಾನಾಂತರ >= Payload_ಕೊನೆ)
          ಹಿಂತಿರುಗಿ createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ತಳ್ಳು_ಹಿಂಭಾಗ(ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    ತಳ_Epilog_Idx = Info.Epilogs.ಗಾತ್ರ_3();
    Info.Epilogs.ತಳ್ಳು_ಹಿಂಭಾಗ(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  ಆದರೆ (ಸ್ಥಾನಾಂತರ < Payload_ಕೊನೆ)
    Info.WODPool = ದತ್ತಾಂಶ_2.ತುಂಡು(ಸ್ಥಾನಾಂತರ, Payload_ಕೊನೆ - ಸ್ಥಾನಾಂತರ);
  ಇಲ್ಲದಿದ್ದರೆ
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_ಗಾತ್ರ = align_ಗಮ್ಯ(Payload_ಕೊನೆ, 4);

  ಹಿಂತಿರುಗಿ Info;
}
