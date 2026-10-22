//===- APFixedPoint.cpp - Fixed point constant handling ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the implementation for the fixed point number interface.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/APFixedPoint.h"
#ضمن "llvm/ADT/APFloat.h"

#ضمن <cmath>

نطاق llvm {

فراغ Fixed_نقطو_Semantics::print(llvm::raw_ostream &OS) ثابت {
  OS << "width=" << get_ويڪر() << ", ";
  إذا (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << get_Msb_وزن() << ", ";
  OS << "lsb=" << get_Lsb_وزن() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t Fixed_نقطو_Semantics::منزل_Opaque_Int() ثابت {
  أرجع llvm::ٻٽو_انگ_cast<uint32_t>(*هذا);
}

Fixed_نقطو_Semantics Fixed_نقطو_Semantics::getFromOpaqueInt(uint32_t I) {
  Fixed_نقطو_Semantics F(0, 0, خطأ, خطأ, خطأ);
  std::يادگيري_نقل(&F, &I, حجم(F));
  أرجع F;
}

APFixed_نقطو APFixed_نقطو::convert(ثابت Fixed_نقطو_Semantics &DstSema,
                                   منطقي *حد_کان_مٿي) ثابت {
  APSInt NewVal = Val;
  صحيح RelativeUpscale = get_Lsb_وزن() - DstSema.get_Lsb_وزن();
  إذا (حد_کان_مٿي)
    *حد_کان_مٿي = خطأ;

  إذا (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_ٻٽو_انگ_ويڪر() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  خودڪار نقاب = APInt::get_بٽ_سيٽ_From(
      NewVal.get_ٻٽو_انگ_ويڪر(),
      std::min(DstSema.get_Integral_بٽ() - DstSema.get_Lsb_وزن(),
               NewVal.get_ٻٽو_انگ_ويڪر()));
  APInt Masked(NewVal & نقاب);

  // Change in the bits above the sign
  إذا (!(Masked == نقاب || Masked == 0)) {
    // Found overflow in the bits above the sign
    إذا (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? نقاب : ~نقاب;
    وإلا إذا (حد_کان_مٿي)
      *حد_کان_مٿي = سچ;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  إذا (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    إذا (DstSema.isSaturated())
      NewVal = 0;
    وإلا إذا (حد_کان_مٿي)
      *حد_کان_مٿي = سچ;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_ويڪر());
  NewVal.سيٽ_Is_Signed(DstSema.isSigned());
  أرجع APFixed_نقطو(NewVal, DstSema);
}

صحيح APFixed_نقطو::compare(ثابت APFixed_نقطو &Other) ثابت {
  APSInt ThisVal = get_قدر();
  APSInt OtherVal = Other.get_قدر();
  منطقي ThisSigned = Val.isSigned();
  منطقي OtherSigned = OtherVal.isSigned();

  صحيح CommonLsb = std::min(get_Lsb_وزن(), Other.get_Lsb_وزن());
  صحيح CommonMsb = std::max(get_Msb_وزن(), Other.get_Msb_وزن());
  بنانشان Common_ويڪر = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_ويڪر);
  OtherVal = OtherVal.extOrTrunc(Common_ويڪر);

  ThisVal = ThisVal.shl(get_Lsb_وزن() - CommonLsb);
  OtherVal = OtherVal.shl(Other.get_Lsb_وزن() - CommonLsb);

  إذا (ThisSigned && OtherSigned) {
    إذا (ThisVal.sgt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.slt(OtherVal))
      أرجع -1;
  } وإلا إذا (!ThisSigned && !OtherSigned) {
    إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  } وإلا إذا (ThisSigned && !OtherSigned) {
    إذا (ThisVal.is_Sign_ٻٽو_انگ_سيٽ())
      أرجع -1;
    وإلا إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  } وإلا {
    // !ThisSigned && OtherSigned
    إذا (OtherVal.is_Sign_ٻٽو_انگ_سيٽ())
      أرجع 1;
    وإلا إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  }

  أرجع 0;
}

APFixed_نقطو APFixed_نقطو::getMax(ثابت Fixed_نقطو_Semantics &Sema) {
  منطقي IsUnsigned = !Sema.isSigned();
  خودڪار Val = APSInt::get_Max_قدر(Sema.get_ويڪر(), IsUnsigned);
  إذا (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  أرجع APFixed_نقطو(Val, Sema);
}

APFixed_نقطو APFixed_نقطو::getMin(ثابت Fixed_نقطو_Semantics &Sema) {
  خودڪار Val = APSInt::get_Min_قدر(Sema.get_ويڪر(), !Sema.isSigned());
  أرجع APFixed_نقطو(Val, Sema);
}

APFixed_نقطو APFixed_نقطو::getEpsilon(ثابت Fixed_نقطو_Semantics &Sema) {
  APSInt Val(Sema.get_ويڪر(), !Sema.isSigned());
  Val.سيٽ_ٻٽو_انگ(/*BitPosition=*/0);
  أرجع APFixed_نقطو(Val, Sema);
}

منطقي Fixed_نقطو_Semantics::سمائي_اندريون_Float_Semantics(
    ثابت fltSemantics &FloatSema) ثابت {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixed_نقطو::getMax(*هذا).get_قدر();
  APFloat F(FloatSema);
  APFloat::op_حالت حالت = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Away);
  إذا ((حالت & APFloat::op_حد_کان_مٿي) || !isSigned())
    أرجع !(حالت & APFloat::op_حد_کان_مٿي);

  APSInt MinInt = APFixed_نقطو::getMin(*هذا).get_قدر();
  حالت = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Away);
  أرجع !(حالت & APFloat::op_حد_کان_مٿي);
}

Fixed_نقطو_Semantics Fixed_نقطو_Semantics::getCommonSemantics(
    ثابت Fixed_نقطو_Semantics &Other) ثابت {
  صحيح CommonLsb = std::min(get_Lsb_وزن(), Other.get_Lsb_وزن());
  صحيح CommonMSb = std::max(get_Msb_وزن() - has_Sign_Or_Padding_ٻٽو_انگ(),
                           Other.get_Msb_وزن() - Other.has_Sign_Or_Padding_ٻٽو_انگ());
  بنانشان Common_ويڪر = CommonMSb - CommonLsb + 1;

  منطقي نتيجو_Is_Signed = isSigned() || Other.isSigned();
  منطقي نتيجو_Is_Saturated = isSaturated() || Other.isSaturated();
  منطقي نتيجو_Has_Unsigned_Padding = خطأ;
  إذا (!نتيجو_Is_Signed) {
    // Both are unsigned.
    نتيجو_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !نتيجو_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  إذا (نتيجو_Is_Signed || نتيجو_Has_Unsigned_Padding)
    Common_ويڪر++;

  أرجع Fixed_نقطو_Semantics(Common_ويڪر, Lsb{CommonLsb}, نتيجو_Is_Signed,
                             نتيجو_Is_Saturated, نتيجو_Has_Unsigned_Padding);
}

APFixed_نقطو APFixed_نقطو::جوڙيو(ثابت APFixed_نقطو &Other,
                               منطقي *حد_کان_مٿي) ثابت {
  خودڪار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطو ConvertedThis = convert(CommonFXSema);
  APFixed_نقطو ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقي Overflowed = خطأ;

  APSInt نتيجو;
  إذا (CommonFXSema.isSaturated()) {
    نتيجو = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } وإلا {
    نتيجو = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  إذا (حد_کان_مٿي)
    *حد_کان_مٿي = Overflowed;

  أرجع APFixed_نقطو(نتيجو, CommonFXSema);
}

APFixed_نقطو APFixed_نقطو::sub(ثابت APFixed_نقطو &Other,
                               منطقي *حد_کان_مٿي) ثابت {
  خودڪار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطو ConvertedThis = convert(CommonFXSema);
  APFixed_نقطو ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقي Overflowed = خطأ;

  APSInt نتيجو;
  إذا (CommonFXSema.isSaturated()) {
    نتيجو = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } وإلا {
    نتيجو = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  إذا (حد_کان_مٿي)
    *حد_کان_مٿي = Overflowed;

  أرجع APFixed_نقطو(نتيجو, CommonFXSema);
}

APFixed_نقطو APFixed_نقطو::mul(ثابت APFixed_نقطو &Other,
                               منطقي *حد_کان_مٿي) ثابت {
  خودڪار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطو ConvertedThis = convert(CommonFXSema);
  APFixed_نقطو ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  بنانشان Wide = CommonFXSema.get_ويڪر() * 2;
  إذا (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } وإلا {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Perform the full multiplication and downscale to get the same scale.
  //
  // Note that the right shifts here perform an implicit downwards rounding.
  // This rounding could discard bits that would technically place the result
  // outside the representable range. We interpret the spec as allowing us to
  // perform the rounding step first, avoiding the overflow case that would
  // arise.
  APSInt نتيجو;
  إذا (CommonFXSema.isSigned())
    نتيجو = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.get_Lsb_وزن());
  وإلا
    نتيجو = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.get_Lsb_وزن());
  أكد(!Overflowed && "Full multiplication cannot overflow!");
  نتيجو.سيٽ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطو::getMax(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_نقطو::getMin(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  إذا (CommonFXSema.isSaturated()) {
    إذا (نتيجو < Min)
      نتيجو = std::move(Min);
    وإلا إذا (نتيجو > Max)
      نتيجو = std::move(Max);
  } وإلا {
    Overflowed = نتيجو < Min || نتيجو > Max;
  }

  إذا (حد_کان_مٿي)
    *حد_کان_مٿي = Overflowed;

  أرجع APFixed_نقطو(نتيجو.sextOrTrunc(CommonFXSema.get_ويڪر()),
                      CommonFXSema);
}

APFixed_نقطو APFixed_نقطو::div(ثابت APFixed_نقطو &Other,
                               منطقي *حد_کان_مٿي) ثابت {
  خودڪار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطو ConvertedThis = convert(CommonFXSema);
  APFixed_نقطو ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //حد_کان_مٿي
  بنانشان Wide =
      CommonFXSema.get_ويڪر() * 2 + std::max(-CommonFXSema.get_Msb_وزن(), 0);
  إذا (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } وإلا {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  إذا (CommonFXSema.get_Lsb_وزن() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.get_Lsb_وزن());
  وإلا إذا (CommonFXSema.get_Lsb_وزن() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.get_Lsb_وزن());
  APSInt نتيجو;
  إذا (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, نتيجو, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    إذا (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      نتيجو = نتيجو - 1;
  } وإلا {
    نتيجو = ThisVal.udiv(OtherVal);
  }
  نتيجو.سيٽ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطو::getMax(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_نقطو::getMin(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  إذا (CommonFXSema.isSaturated()) {
    إذا (نتيجو < Min)
      نتيجو = std::move(Min);
    وإلا إذا (نتيجو > Max)
      نتيجو = std::move(Max);
  } وإلا {
    Overflowed = نتيجو < Min || نتيجو > Max;
  }

  إذا (حد_کان_مٿي)
    *حد_کان_مٿي = Overflowed;

  أرجع APFixed_نقطو(نتيجو.sextOrTrunc(CommonFXSema.get_ويڪر()),
                      CommonFXSema);
}

APFixed_نقطو APFixed_نقطو::shl(بنانشان Amt, منطقي *حد_کان_مٿي) ثابت {
  APSInt ThisVal = Val;
  منطقي Overflowed = خطأ;

  // Widen the LHS.
  بنانشان Wide = Sema.get_ويڪر() * 2;
  إذا (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  وإلا
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_ٻٽو_انگ_ويڪر());
  APSInt نتيجو = ThisVal << Amt;
  نتيجو.سيٽ_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطو::getMax(Sema).get_قدر().extOrTrunc(Wide);
  APSInt Min = APFixed_نقطو::getMin(Sema).get_قدر().extOrTrunc(Wide);
  إذا (Sema.isSaturated()) {
    إذا (نتيجو < Min)
      نتيجو = std::move(Min);
    وإلا إذا (نتيجو > Max)
      نتيجو = std::move(Max);
  } وإلا {
    Overflowed = نتيجو < Min || نتيجو > Max;
  }

  إذا (حد_کان_مٿي)
    *حد_کان_مٿي = Overflowed;

  أرجع APFixed_نقطو(نتيجو.sextOrTrunc(Sema.get_ويڪر()), Sema);
}

فراغ APFixed_نقطو::منزل_String(Small_ويڪٽر_Impl<حرف> &Str) ثابت {
  APSInt Val = get_قدر();
  صحيح Lsb = get_Lsb_وزن();
  صحيح Orig_ويڪر = get_ويڪر();

  إذا (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_ٻٽو_انگ_ويڪر() + Lsb);
    IntPart <<= Lsb;
    IntPart.منزل_String(Str, /*Radix=*/10);
    Str.ڌڪو_پٺ('.');
    Str.ڌڪو_پٺ('0');
    أرجع;
  }

  إذا (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.سيٽ_Is_Unsigned(سچ);
    Str.ڌڪو_پٺ('-');
  }

  صحيح Scale = -get_Lsb_وزن();
  APSInt IntPart = (Orig_ويڪر > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  بنانشان ويڪر = std::max(Orig_ويڪر, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(ويڪر);
  APInt Fract_Part_نقاب = APInt::getAllOnes(Scale).zext(ويڪر);
  APInt RadixInt = APInt(ويڪر, 10);

  IntPart.منزل_String(Str, /*Radix=*/10);
  Str.ڌڪو_پٺ('.');
  افعل {
    (FractPart * RadixInt)
        .lshr(Scale)
        .منزل_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_نقاب;
  } طالما (FractPart != 0);
}

فراغ APFixed_نقطو::print(raw_ostream &OS) ثابت {
  OS << "APFixedPoint(" << منزل_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طريقو فراغ APFixed_نقطو::dump() ثابت { print(llvm::errs()); }
#جيڪڏهن_پورو

APFixed_نقطو APFixed_نقطو::negate(منطقي *حد_کان_مٿي) ثابت {
  إذا (!isSaturated()) {
    إذا (حد_کان_مٿي)
      *حد_کان_مٿي =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_قدر());
    أرجع APFixed_نقطو(-Val, Sema);
  }

  // We never overflow for saturation
  إذا (حد_کان_مٿي)
    *حد_کان_مٿي = خطأ;

  إذا (isSigned())
    أرجع Val.is_Min_Signed_قدر() ? getMax(Sema) : APFixed_نقطو(-Val, Sema);
  وإلا
    أرجع APFixed_نقطو(Sema);
}

APSInt APFixed_نقطو::convert_منزل_Int(بنانشان Dst_ويڪر, منطقي DstSign,
                                  منطقي *حد_کان_مٿي) ثابت {
  APSInt نتيجو = getIntPart();
  بنانشان Src_ويڪر = get_ويڪر();

  APSInt DstMin = APSInt::get_Min_قدر(Dst_ويڪر, !DstSign);
  APSInt DstMax = APSInt::get_Max_قدر(Dst_ويڪر, !DstSign);

  إذا (Src_ويڪر < Dst_ويڪر) {
    نتيجو = نتيجو.extend(Dst_ويڪر);
  } وإلا إذا (Src_ويڪر > Dst_ويڪر) {
    DstMin = DstMin.extend(Src_ويڪر);
    DstMax = DstMax.extend(Src_ويڪر);
  }

  إذا (حد_کان_مٿي) {
    إذا (نتيجو.isSigned() && !DstSign) {
      *حد_کان_مٿي = نتيجو.isNegative() || نتيجو.ugt(DstMax);
    } وإلا إذا (نتيجو.isUnsigned() && DstSign) {
      *حد_کان_مٿي = نتيجو.ugt(DstMax);
    } وإلا {
      *حد_کان_مٿي = نتيجو < DstMin || نتيجو > DstMax;
    }
  }

  نتيجو.سيٽ_Is_Signed(DstSign);
  أرجع نتيجو.extOrTrunc(Dst_ويڪر);
}

ثابت fltSemantics *APFixed_نقطو::promoteFloatSemantics(ثابت fltSemantics *S) {
  إذا (S == &APFloat::BFloat())
    أرجع &APFloat::IEEEdouble();
  وإلا إذا (S == &APFloat::IEEEhalf())
    أرجع &APFloat::IEEEsingle();
  وإلا إذا (S == &APFloat::IEEEsingle())
    أرجع &APFloat::IEEEdouble();
  وإلا إذا (S == &APFloat::IEEEdouble())
    أرجع &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixed_نقطو::convert_منزل_Float(ثابت fltSemantics &FloatSema) ثابت {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_گهڻائي RM = APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Even;
  APFloat::rounding_گهڻائي LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!Sema.سمائي_اندريون_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::op_حالت S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (فراغ)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat Scale_ضرب_ڪندڙ(std::قوت(2, Sema.get_Lsb_وزن()));
  منطقي Ignored;
  Scale_ضرب_ڪندڙ.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_ضرب_ڪندڙ, LosslessRM);

  إذا (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  أرجع Flt;
}

APFixed_نقطو APFixed_نقطو::get_From_Int_قدر(ثابت APSInt &قدر,
                                           ثابت Fixed_نقطو_Semantics &DstFXSema,
                                           منطقي *حد_کان_مٿي) {
  Fixed_نقطو_Semantics IntFXSema = Fixed_نقطو_Semantics::GetIntegerSemantics(
      قدر.get_ٻٽو_انگ_ويڪر(), قدر.isSigned());
  أرجع APFixed_نقطو(قدر, IntFXSema).convert(DstFXSema, حد_کان_مٿي);
}

APFixed_نقطو
APFixed_نقطو::get_From_Float_قدر(ثابت APFloat &قدر,
                                ثابت Fixed_نقطو_Semantics &DstFXSema,
                                منطقي *حد_کان_مٿي) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_گهڻائي RM = APFloat::rmTowardZero;
  APFloat::rounding_گهڻائي LosslessRM = APFloat::rmTowardZero;

  ثابت fltSemantics &FloatSema = قدر.getSemantics();

  إذا (قدر.isNaN()) {
    // Handle NaN immediately.
    إذا (حد_کان_مٿي)
      *حد_کان_مٿي = سچ;
    أرجع APFixed_نقطو(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!DstFXSema.سمائي_اندريون_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = قدر;

  منطقي Ignored;
  إذا (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_ضرب_ڪندڙ(std::قوت(2, -DstFXSema.get_Lsb_وزن()));
  Scale_ضرب_ڪندڙ.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_ضرب_ڪندڙ, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_ويڪر(), !DstFXSema.isSigned());
  Val.convert_منزل_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_ضرب_ڪندڙ = APFloat(std::قوت(2, DstFXSema.get_Lsb_وزن()));
  Scale_ضرب_ڪندڙ.convert(*OpSema, LosslessRM, &Ignored);
  Val.ڦيرو_منزل_Integral(RM);
  Val.multiply(Scale_ضرب_ڪندڙ, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convert_منزل_Float(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convert_منزل_Float(*OpSema);
  منطقي Overflowed = خطأ;
  إذا (DstFXSema.isSaturated()) {
    إذا (Val > FloatMax)
      Res = getMax(DstFXSema).get_قدر();
    وإلا إذا (Val < FloatMin)
      Res = getMin(DstFXSema).get_قدر();
  } وإلا {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  إذا (حد_کان_مٿي)
    *حد_کان_مٿي = Overflowed;

  أرجع APFixed_نقطو(Res, DstFXSema);
}

} // namespace llvm
