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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Win64EH.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Endian.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::Win64EH;

StringRef Win64EH::get_ਰਜਿਸਟਰ_Name_V3(ਬਿਨਾਨਿਸ਼ਾਨੀ Reg) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਨਾਂ[] = {
      "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
      "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
      "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
      "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31",
  };
  ਜੇ (Reg >= std::ਮਾਪ_2(ਨਾਂ))
    ਵਾਪਸ "<invalid>";
  ਵਾਪਸ ਨਾਂ[Reg];
}

Expected<DecodedWOD> Win64EH::decodeWOD(ArrayRef<uint8_t> Pool,
                                        ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ) {
  ਜੇ (ਵਿੱਥ >= Pool.ਮਾਪ_2())
    ਵਾਪਸ createStringError("WOD pool overflow at offset %u", ਵਿੱਥ);

  uint8_t ਪਹਿਲਾ_Byte = Pool[ਵਿੱਥ];
  DecodedWOD W = {};

  // Determine opcode from variable-width prefix encoding.
  // The dispatch order matters: check shorter prefixes first since they
  // occupy the lowest bits, then fall through to longer prefixes.
  //   3-bit prefix (bits [2:0] >= 4): opcodes 4-7
  //   4-bit prefix (bits [3:0] >= 8): opcodes 8-10
  //   6-bit prefix (bits [5:0] == 0x20): opcode 32 (PUSH2)
  //   8-bit prefix (full byte 0-3): opcodes 0-3
  uint8_t Low3 = ਪਹਿਲਾ_Byte & 0x07;

  // 3-bit opcode: bits [2:0] in {4, 5, 6, 7}
  ਜੇ (Low3 >= 4) {
    ਚੋਣ (Low3) {
    ਹਾਲਤ WOD_ਧੱਕੋ: {
      W.Opcode = WOD_ਧੱਕੋ;
      W.Byte_ਮਾਪ = 1;
      W.ਰਜਿਸਟਰ_2 = (ਪਹਿਲਾ_Byte >> 3) & 0x1F; // 5-bit register
      ਵਾਪਸ W;
    }
    ਹਾਲਤ WOD_SAVE_NONVOL_FAR: {
      W.Opcode = WOD_SAVE_NONVOL_FAR;
      W.Byte_ਮਾਪ = 5;
      ਜੇ (ਵਿੱਥ + 5 > Pool.ਮਾਪ_2())
        ਵਾਪਸ createStringError("WOD_SAVE_NONVOL_FAR truncated at offset %u",
                                 ਵਿੱਥ);
      W.ਰਜਿਸਟਰ_2 = (ਪਹਿਲਾ_Byte >> 3) & 0x1F;
      W.Displacement = support::endian::read32le(&Pool[ਵਿੱਥ + 1]);
      ਵਾਪਸ W;
    }
    ਹਾਲਤ WOD_SAVE_NONVOL: {
      W.Opcode = WOD_SAVE_NONVOL;
      W.Byte_ਮਾਪ = 3;
      ਜੇ (ਵਿੱਥ + 3 > Pool.ਮਾਪ_2())
        ਵਾਪਸ createStringError("WOD_SAVE_NONVOL truncated at offset %u",
                                 ਵਿੱਥ);
      W.ਰਜਿਸਟਰ_2 = (ਪਹਿਲਾ_Byte >> 3) & 0x1F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[ਵਿੱਥ + 1]) * 8;
      ਵਾਪਸ W;
    }
    ਹਾਲਤ WOD_ਧੱਕੋ_CONSECUTIVE_2: {
      W.Opcode = WOD_ਧੱਕੋ_CONSECUTIVE_2;
      W.Byte_ਮਾਪ = 1;
      W.ਰਜਿਸਟਰ_2 = (ਪਹਿਲਾ_Byte >> 3) & 0x1F;
      ਜੇ (W.ਰਜਿਸਟਰ_2 > 30)
        ਵਾਪਸ createStringError(
            "WOD_PUSH_CONSECUTIVE_2 Register=%u out of range [0,30] at pool "
            "offset %u",
            W.ਰਜਿਸਟਰ_2, ਵਿੱਥ);
      ਵਾਪਸ W;
    }
    ਮੂਲ:
      ਵਾਪਸ createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               ਪਹਿਲਾ_Byte, ਵਿੱਥ);
    }
  }

  // 4-bit opcode: bits [3:0] in {8, 9, 10, ...}
  uint8_t Low4 = ਪਹਿਲਾ_Byte & 0x0F;
  ਜੇ (Low4 >= 8) {
    ਚੋਣ (Low4) {
    ਹਾਲਤ WOD_ALLOC_SMALL: {
      W.Opcode = WOD_ALLOC_SMALL;
      W.Byte_ਮਾਪ = 1;
      W.ਮਾਪ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(((ਪਹਿਲਾ_Byte >> 4) & 0x0F) + 1) * 8;
      ਵਾਪਸ W;
    }
    ਹਾਲਤ WOD_SAVE_XMM128_FAR: {
      W.Opcode = WOD_SAVE_XMM128_FAR;
      W.Byte_ਮਾਪ = 5;
      ਜੇ (ਵਿੱਥ + 5 > Pool.ਮਾਪ_2())
        ਵਾਪਸ createStringError("WOD_SAVE_XMM128_FAR truncated at offset %u",
                                 ਵਿੱਥ);
      W.ਰਜਿਸਟਰ_2 = (ਪਹਿਲਾ_Byte >> 4) & 0x0F;
      W.Displacement = support::endian::read32le(&Pool[ਵਿੱਥ + 1]);
      ਵਾਪਸ W;
    }
    ਹਾਲਤ WOD_SAVE_XMM128: {
      W.Opcode = WOD_SAVE_XMM128;
      W.Byte_ਮਾਪ = 3;
      ਜੇ (ਵਿੱਥ + 3 > Pool.ਮਾਪ_2())
        ਵਾਪਸ createStringError("WOD_SAVE_XMM128 truncated at offset %u",
                                 ਵਿੱਥ);
      W.ਰਜਿਸਟਰ_2 = (ਪਹਿਲਾ_Byte >> 4) & 0x0F;
      W.Displacement =
          (uint32_t)support::endian::read16le(&Pool[ਵਿੱਥ + 1]) * 16;
      ਵਾਪਸ W;
    }
    ਮੂਲ:
      ਵਾਪਸ createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                               ਪਹਿਲਾ_Byte, ਵਿੱਥ);
    }
  }

  // 6-bit opcode: bits [5:0] == 0x20 (WOD_PUSH2)
  uint8_t Low6 = ਪਹਿਲਾ_Byte & 0x3F;
  ਜੇ (Low6 == WOD_PUSH2) {
    W.Opcode = WOD_PUSH2;
    W.Byte_ਮਾਪ = 2;
    ਜੇ (ਵਿੱਥ + 2 > Pool.ਮਾਪ_2())
      ਵਾਪਸ createStringError("WOD_PUSH2 truncated at offset %u", ਵਿੱਥ);
    uint8_t ਦੂਜਾ_Byte = Pool[ਵਿੱਥ + 1];
    // First reg from bits [7:6] of first byte (2 bits) and bits [2:0] of second
    // (3 bits)
    W.ਰਜਿਸਟਰ_2 = ((ਪਹਿਲਾ_Byte >> 6) & 0x03) | ((ਦੂਜਾ_Byte & 0x07) << 2);
    W.Register2 = (ਦੂਜਾ_Byte >> 3) & 0x1F;
    ਵਾਪਸ W;
  }

  // 8-bit opcode: full byte is opcode (values 0-3)
  ਚੋਣ (ਪਹਿਲਾ_Byte) {
  ਹਾਲਤ WOD_ਸੈੱਟ_FPREG: {
    W.Opcode = WOD_ਸੈੱਟ_FPREG;
    W.Byte_ਮਾਪ = 2;
    ਜੇ (ਵਿੱਥ + 2 > Pool.ਮਾਪ_2())
      ਵਾਪਸ createStringError("WOD_SET_FPREG truncated at offset %u", ਵਿੱਥ);
    uint8_t ਦੂਜਾ_Byte = Pool[ਵਿੱਥ + 1];
    W.ਰਜਿਸਟਰ_2 = ਦੂਜਾ_Byte & 0x0F; // 4-bit register
    W.Displacement = (ਬਿਨਾਨਿਸ਼ਾਨੀ)((ਦੂਜਾ_Byte >> 4) & 0x0F) * 16;
    ਵਾਪਸ W;
  }
  ਹਾਲਤ WOD_ALLOC_HUGE: {
    W.Opcode = WOD_ALLOC_HUGE;
    W.Byte_ਮਾਪ = 5;
    ਜੇ (ਵਿੱਥ + 5 > Pool.ਮਾਪ_2())
      ਵਾਪਸ createStringError("WOD_ALLOC_HUGE truncated at offset %u", ਵਿੱਥ);
    W.ਮਾਪ = support::endian::read32le(&Pool[ਵਿੱਥ + 1]);
    ਵਾਪਸ W;
  }
  ਹਾਲਤ WOD_ALLOC_LARGE: {
    W.Opcode = WOD_ALLOC_LARGE;
    W.Byte_ਮਾਪ = 3;
    ਜੇ (ਵਿੱਥ + 3 > Pool.ਮਾਪ_2())
      ਵਾਪਸ createStringError("WOD_ALLOC_LARGE truncated at offset %u",
                               ਵਿੱਥ);
    W.ਮਾਪ = (uint32_t)support::endian::read16le(&Pool[ਵਿੱਥ + 1]) * 8;
    ਵਾਪਸ W;
  }
  ਹਾਲਤ WOD_ਧੱਕੋ_CANONICAL_ਚੌਖਟਾ: {
    W.Opcode = WOD_ਧੱਕੋ_CANONICAL_ਚੌਖਟਾ;
    W.Byte_ਮਾਪ = 2;
    ਜੇ (ਵਿੱਥ + 2 > Pool.ਮਾਪ_2())
      ਵਾਪਸ createStringError(
          "WOD_PUSH_CANONICAL_FRAME truncated at offset %u", ਵਿੱਥ);
    W.Type = Pool[ਵਿੱਥ + 1];
    ਵਾਪਸ W;
  }
  ਮੂਲ:
    ਵਾਪਸ createStringError("unknown WOD opcode 0x%02X at pool offset %u",
                             ਪਹਿਲਾ_Byte, ਵਿੱਥ);
  }
}

Expected<DecodedUnwindInfoV3>
Win64EH::decodeUnwindInfoV3(ArrayRef<uint8_t> ਅੰਕੜੇ_2) {
  ਜੇ (ਅੰਕੜੇ_2.ਮਾਪ_2() < 4)
    ਵਾਪਸ createStringError("V3 unwind info too short: %zu bytes",
                             ਅੰਕੜੇ_2.ਮਾਪ_2());

  DecodedUnwindInfoV3 Info;
  Info.ਸੰਸਕਰਣ = ਅੰਕੜੇ_2[0] & 0x07;
  Info.Flags = (ਅੰਕੜੇ_2[0] >> 3) & 0x1F;
  Info.ਮਾਪ_ਦਾ_Prolog = ਅੰਕੜੇ_2[1];
  Info.Payload_ਸ਼ਬਦ = ਅੰਕੜੇ_2[2];
  Info.ਸੰਖਿਆ_ਦਾ_Ops = ਅੰਕੜੇ_2[3] & 0x1F;
  Info.ਸੰਖਿਆ_ਦਾ_Epilogs = (ਅੰਕੜੇ_2[3] >> 5) & 0x07;

  // The fixed header is always 4 bytes. When UNW_FlagLarge is set, the first
  // byte of the payload is the UNWIND_INFO_LARGE_V3 extension byte (which
  // extends SizeOfProlog to 16 bits and widens prolog IP offset entries to
  // 16 bits). That byte IS counted in PayloadWords.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ = 4; // Start of payload

  // Compute the end of the payload area declared by PayloadWords. All
  // subsequent reads of payload structures (the optional UNWIND_INFO_LARGE_V3
  // byte, prolog IP offsets, epilog descriptors) must stay within this region;
  // reading past it would either overflow the buffer or cross into the
  // trailing handler/chain data, both of which indicate a malformed record.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Payload_ਸਿਰਾ = 4 + Info.Payload_ਸ਼ਬਦ * 2;
  ਜੇ (Payload_ਸਿਰਾ > ਅੰਕੜੇ_2.ਮਾਪ_2())
    ਵਾਪਸ createStringError(
        "V3 unwind info PayloadWords (%u) extends past end of buffer",
        Info.Payload_ਸ਼ਬਦ);

  ਤਾਰਕਿਕ IsLarge = Info.isLarge();
  ਜੇ (IsLarge) {
    ਜੇ (ਵਿੱਥ >= Payload_ਸਿਰਾ)
      ਵਾਪਸ createStringError(
          "V3 unwind info with UNW_FlagLarge too short: PayloadWords (%u) "
          "leaves no room for UNWIND_INFO_LARGE_V3",
          Info.Payload_ਸ਼ਬਦ);
    Info.ਮਾਪ_ਦਾ_Prolog |= static_cast<uint16_t>(ਅੰਕੜੇ_2[ਵਿੱਥ]) << 8;
    ਵਿੱਥ += 1;
  }

  // Read prolog IP offsets (8-bit each, or 16-bit when LARGE)
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < Info.ਸੰਖਿਆ_ਦਾ_Ops; ++I) {
    ਜੇ (IsLarge) {
      ਜੇ (ਵਿੱਥ + 2 > Payload_ਸਿਰਾ)
        ਵਾਪਸ createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ਧੱਕੋ_ਪਿੱਠ(support::endian::read16le(&ਅੰਕੜੇ_2[ਵਿੱਥ]));
      ਵਿੱਥ += 2;
    } ਨਹੀਂਤਾਂ {
      ਜੇ (ਵਿੱਥ >= Payload_ਸਿਰਾ)
        ਵਾਪਸ createStringError(
            "V3 payload truncated reading prolog IP offset %u", I);
      Info.PrologIpOffsets.ਧੱਕੋ_ਪਿੱਠ(ਅੰਕੜੇ_2[ਵਿੱਥ++]);
    }
  }

  // Read epilog descriptors
  int32_t Prev_Resolved_ਵਿੱਥ = 0;
  // Index of the most recent full descriptor (NumberOfOps != 0). Per the V3
  // spec, a descriptor with NumberOfOps == 0 inherits its effective fields
  // from the first *preceding* descriptor with NumberOfOps != 0, which is not
  // necessarily the immediately preceding descriptor.
  ਪੂਰਨ ਅਧਾਰ_Epilog_Idx = -1;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < Info.ਸੰਖਿਆ_ਦਾ_Epilogs; ++I) {
    DecodedEpilogV3 Epi;
    ਜੇ (ਵਿੱਥ >= Payload_ਸਿਰਾ)
      ਵਾਪਸ createStringError(
          "V3 payload truncated reading epilog %u FlagsAndNumOps", I);
    uint8_t Flags_And_ਅੰਕ_Ops = ਅੰਕੜੇ_2[ਵਿੱਥ++];
    Epi.Flags = Flags_And_ਅੰਕ_Ops & 0x07;
    Epi.ਸੰਖਿਆ_ਦਾ_Ops = (Flags_And_ਅੰਕ_Ops >> 3) & 0x1F;

    ਜੇ (ਵਿੱਥ + 2 > Payload_ਸਿਰਾ)
      ਵਾਪਸ createStringError(
          "V3 payload truncated reading epilog %u EpilogOffset", I);
    int16_t Raw_ਵਿੱਥ =
        static_cast<int16_t>(support::endian::read16le(&ਅੰਕੜੇ_2[ਵਿੱਥ]));
    ਵਿੱਥ += 2;

    // The first epilog's EpilogOffset is absolute (from fragment start or
    // tail). Subsequent epilogs store a delta from the previous epilog's
    // resolved position. Accumulate to resolve all to absolute.
    ਜੇ (I == 0)
      Epi.Epilog_ਵਿੱਥ = Raw_ਵਿੱਥ;
    ਨਹੀਂਤਾਂ
      Epi.Epilog_ਵਿੱਥ = Prev_Resolved_ਵਿੱਥ + Raw_ਵਿੱਥ;
    Prev_Resolved_ਵਿੱਥ = Epi.Epilog_ਵਿੱਥ;

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
    ਜੇ (Epi.ਸੰਖਿਆ_ਦਾ_Ops == 0) {
      ਜੇ (ਅਧਾਰ_Epilog_Idx >= 0) {
        ਅਚਲ DecodedEpilogV3 &ਅਧਾਰ = Info.Epilogs[ਅਧਾਰ_Epilog_Idx];
        // Flags bits 0 and 1 are producer-replicated, not inherited: a
        // compliant producer must have written the base's values into this
        // descriptor's own flags byte, so they should already match the base.
        // We intentionally keep this record's own bits (rather than copying
        // the base's) so a non-compliant producer is not silently masked. This
        // decoder runs on potentially-malformed object files, so a mismatch is
        // not asserted here; downstream consumers (the dumpers) surface a
        // warning when the replicated bits disagree with the base.
        Epi.ਪਹਿਲਾ_Op = ਅਧਾਰ.ਪਹਿਲਾ_Op;
        Epi.Ip_ਵਿੱਥ_ਦਾ_ਆਖਰੀ_Instruction = ਅਧਾਰ.Ip_ਵਿੱਥ_ਦਾ_ਆਖਰੀ_Instruction;
        Epi.IpOffsets = ਅਧਾਰ.IpOffsets;
      } ਨਹੀਂਤਾਂ {
        Epi.ਪਹਿਲਾ_Op = 0;
        Epi.Ip_ਵਿੱਥ_ਦਾ_ਆਖਰੀ_Instruction = 0;
      }
      Info.Epilogs.ਧੱਕੋ_ਪਿੱਠ(std::move(Epi));
      ਜਾਰੀ;
    }

    ਤਾਰਕਿਕ EpiLarge = Epi.isLarge();

    ਜੇ (ਵਿੱਥ + 2 > Payload_ਸਿਰਾ)
      ਵਾਪਸ createStringError("V3 payload truncated reading epilog %u FirstOp",
                               I);
    Epi.ਪਹਿਲਾ_Op = support::endian::read16le(&ਅੰਕੜੇ_2[ਵਿੱਥ]);
    ਵਿੱਥ += 2;

    // IpOffsetOfLastInstruction: 8-bit normally, 16-bit when EPILOG_INFO_LARGE
    ਜੇ (EpiLarge) {
      ਜੇ (ਵਿੱਥ + 2 > Payload_ਸਿਰਾ)
        ਵਾਪਸ createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_ਵਿੱਥ_ਦਾ_ਆਖਰੀ_Instruction = support::endian::read16le(&ਅੰਕੜੇ_2[ਵਿੱਥ]);
      ਵਿੱਥ += 2;
    } ਨਹੀਂਤਾਂ {
      ਜੇ (ਵਿੱਥ >= Payload_ਸਿਰਾ)
        ਵਾਪਸ createStringError(
            "V3 payload truncated reading epilog %u IpOffsetOfLastInstruction",
            I);
      Epi.Ip_ਵਿੱਥ_ਦਾ_ਆਖਰੀ_Instruction = ਅੰਕੜੇ_2[ਵਿੱਥ++];
    }

    // Read epilog IP offsets (8-bit each, or 16-bit when EPILOG_INFO_LARGE)
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ J = 0; J < Epi.ਸੰਖਿਆ_ਦਾ_Ops; ++J) {
      ਜੇ (EpiLarge) {
        ਜੇ (ਵਿੱਥ + 2 > Payload_ਸਿਰਾ)
          ਵਾਪਸ createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ਧੱਕੋ_ਪਿੱਠ(support::endian::read16le(&ਅੰਕੜੇ_2[ਵਿੱਥ]));
        ਵਿੱਥ += 2;
      } ਨਹੀਂਤਾਂ {
        ਜੇ (ਵਿੱਥ >= Payload_ਸਿਰਾ)
          ਵਾਪਸ createStringError(
              "V3 payload truncated reading epilog %u IP offset %u", I, J);
        Epi.IpOffsets.ਧੱਕੋ_ਪਿੱਠ(ਅੰਕੜੇ_2[ਵਿੱਥ++]);
      }
    }

    // This is a full descriptor (NumberOfOps != 0); it becomes the base that
    // subsequent inherited descriptors reference.
    ਅਧਾਰ_Epilog_Idx = Info.Epilogs.ਮਾਪ_2();
    Info.Epilogs.ਧੱਕੋ_ਪਿੱਠ(std::move(Epi));
  }

  // Identify WOD pool: everything from current offset until the end of
  // the payload area declared by PayloadWords.
  ਜੇ (ਵਿੱਥ < Payload_ਸਿਰਾ)
    Info.WODPool = ਅੰਕੜੇ_2.ਟੁਕੜਾ(ਵਿੱਥ, Payload_ਸਿਰਾ - ਵਿੱਥ);
  ਨਹੀਂਤਾਂ
    Info.WODPool = ArrayRef<uint8_t>();

  // When PayloadWords is odd, the encoder emits 2 trailing zero bytes inside
  // the payload region as padding before the handler/chain. Report the
  // aligned offset so consumers locate the next field correctly.
  Info.Payload_ਮਾਪ = align_ਮੰਜ਼ਿਲ(Payload_ਸਿਰਾ, 4);

  ਵਾਪਸ Info;
}
