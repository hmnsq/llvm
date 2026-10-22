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

فراغ Fixed_نقطہ_Semantics::print(llvm::raw_ostream &OS) ثابت {
  OS << "width=" << get_چوڑائی() << ", ";
  إذا (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << get_Msb_بھار() << ", ";
  OS << "lsb=" << get_Lsb_بھار() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t Fixed_نقطہ_Semantics::منزل_Opaque_Int() ثابت {
  أرجع llvm::ثنائی_ہندسہ_cast<uint32_t>(*هذا);
}

Fixed_نقطہ_Semantics Fixed_نقطہ_Semantics::getFromOpaqueInt(uint32_t I) {
  Fixed_نقطہ_Semantics F(0, 0, خطأ, خطأ, خطأ);
  std::حافظہ_نقل(&F, &I, حجم(F));
  أرجع F;
}

APFixed_نقطہ APFixed_نقطہ::convert(ثابت Fixed_نقطہ_Semantics &DstSema,
                                   منطقي *حد_توں_ودھ) ثابت {
  APSInt NewVal = Val;
  صحيح RelativeUpscale = get_Lsb_بھار() - DstSema.get_Lsb_بھار();
  إذا (حد_توں_ودھ)
    *حد_توں_ودھ = خطأ;

  إذا (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_ثنائی_ہندسہ_چوڑائی() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  خودکار نقاب = APInt::get_بٹ_سیٹ_From(
      NewVal.get_ثنائی_ہندسہ_چوڑائی(),
      std::min(DstSema.get_Integral_بٹ() - DstSema.get_Lsb_بھار(),
               NewVal.get_ثنائی_ہندسہ_چوڑائی()));
  APInt Masked(NewVal & نقاب);

  // Change in the bits above the sign
  إذا (!(Masked == نقاب || Masked == 0)) {
    // Found overflow in the bits above the sign
    إذا (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? نقاب : ~نقاب;
    وإلا إذا (حد_توں_ودھ)
      *حد_توں_ودھ = سچ;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  إذا (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    إذا (DstSema.isSaturated())
      NewVal = 0;
    وإلا إذا (حد_توں_ودھ)
      *حد_توں_ودھ = سچ;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_چوڑائی());
  NewVal.سیٹ_Is_Signed(DstSema.isSigned());
  أرجع APFixed_نقطہ(NewVal, DstSema);
}

صحيح APFixed_نقطہ::compare(ثابت APFixed_نقطہ &Other) ثابت {
  APSInt ThisVal = get_قدر();
  APSInt OtherVal = Other.get_قدر();
  منطقي ThisSigned = Val.isSigned();
  منطقي OtherSigned = OtherVal.isSigned();

  صحيح CommonLsb = std::min(get_Lsb_بھار(), Other.get_Lsb_بھار());
  صحيح CommonMsb = std::max(get_Msb_بھار(), Other.get_Msb_بھار());
  بنانشانی Common_چوڑائی = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_چوڑائی);
  OtherVal = OtherVal.extOrTrunc(Common_چوڑائی);

  ThisVal = ThisVal.shl(get_Lsb_بھار() - CommonLsb);
  OtherVal = OtherVal.shl(Other.get_Lsb_بھار() - CommonLsb);

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
    إذا (ThisVal.is_Sign_ثنائی_ہندسہ_سیٹ())
      أرجع -1;
    وإلا إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  } وإلا {
    // !ThisSigned && OtherSigned
    إذا (OtherVal.is_Sign_ثنائی_ہندسہ_سیٹ())
      أرجع 1;
    وإلا إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  }

  أرجع 0;
}

APFixed_نقطہ APFixed_نقطہ::getMax(ثابت Fixed_نقطہ_Semantics &Sema) {
  منطقي IsUnsigned = !Sema.isSigned();
  خودکار Val = APSInt::get_Max_قدر(Sema.get_چوڑائی(), IsUnsigned);
  إذا (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  أرجع APFixed_نقطہ(Val, Sema);
}

APFixed_نقطہ APFixed_نقطہ::getMin(ثابت Fixed_نقطہ_Semantics &Sema) {
  خودکار Val = APSInt::get_Min_قدر(Sema.get_چوڑائی(), !Sema.isSigned());
  أرجع APFixed_نقطہ(Val, Sema);
}

APFixed_نقطہ APFixed_نقطہ::getEpsilon(ثابت Fixed_نقطہ_Semantics &Sema) {
  APSInt Val(Sema.get_چوڑائی(), !Sema.isSigned());
  Val.سیٹ_ثنائی_ہندسہ(/*BitPosition=*/0);
  أرجع APFixed_نقطہ(Val, Sema);
}

منطقي Fixed_نقطہ_Semantics::سماندا_وچ_Float_Semantics(
    ثابت fltSemantics &FloatSema) ثابت {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixed_نقطہ::getMax(*هذا).get_قدر();
  APFloat F(FloatSema);
  APFloat::op_حالت حالت = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Away);
  إذا ((حالت & APFloat::op_حد_توں_ودھ) || !isSigned())
    أرجع !(حالت & APFloat::op_حد_توں_ودھ);

  APSInt MinInt = APFixed_نقطہ::getMin(*هذا).get_قدر();
  حالت = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Away);
  أرجع !(حالت & APFloat::op_حد_توں_ودھ);
}

Fixed_نقطہ_Semantics Fixed_نقطہ_Semantics::getCommonSemantics(
    ثابت Fixed_نقطہ_Semantics &Other) ثابت {
  صحيح CommonLsb = std::min(get_Lsb_بھار(), Other.get_Lsb_بھار());
  صحيح CommonMSb = std::max(get_Msb_بھار() - has_Sign_Or_Padding_ثنائی_ہندسہ(),
                           Other.get_Msb_بھار() - Other.has_Sign_Or_Padding_ثنائی_ہندسہ());
  بنانشانی Common_چوڑائی = CommonMSb - CommonLsb + 1;

  منطقي نتیجہ_Is_Signed = isSigned() || Other.isSigned();
  منطقي نتیجہ_Is_Saturated = isSaturated() || Other.isSaturated();
  منطقي نتیجہ_Has_Unsigned_Padding = خطأ;
  إذا (!نتیجہ_Is_Signed) {
    // Both are unsigned.
    نتیجہ_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !نتیجہ_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  إذا (نتیجہ_Is_Signed || نتیجہ_Has_Unsigned_Padding)
    Common_چوڑائی++;

  أرجع Fixed_نقطہ_Semantics(Common_چوڑائی, Lsb{CommonLsb}, نتیجہ_Is_Signed,
                             نتیجہ_Is_Saturated, نتیجہ_Has_Unsigned_Padding);
}

APFixed_نقطہ APFixed_نقطہ::جوڑو(ثابت APFixed_نقطہ &Other,
                               منطقي *حد_توں_ودھ) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطہ ConvertedThis = convert(CommonFXSema);
  APFixed_نقطہ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقي Overflowed = خطأ;

  APSInt نتیجہ;
  إذا (CommonFXSema.isSaturated()) {
    نتیجہ = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } وإلا {
    نتیجہ = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  إذا (حد_توں_ودھ)
    *حد_توں_ودھ = Overflowed;

  أرجع APFixed_نقطہ(نتیجہ, CommonFXSema);
}

APFixed_نقطہ APFixed_نقطہ::sub(ثابت APFixed_نقطہ &Other,
                               منطقي *حد_توں_ودھ) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطہ ConvertedThis = convert(CommonFXSema);
  APFixed_نقطہ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقي Overflowed = خطأ;

  APSInt نتیجہ;
  إذا (CommonFXSema.isSaturated()) {
    نتیجہ = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } وإلا {
    نتیجہ = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  إذا (حد_توں_ودھ)
    *حد_توں_ودھ = Overflowed;

  أرجع APFixed_نقطہ(نتیجہ, CommonFXSema);
}

APFixed_نقطہ APFixed_نقطہ::mul(ثابت APFixed_نقطہ &Other,
                               منطقي *حد_توں_ودھ) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطہ ConvertedThis = convert(CommonFXSema);
  APFixed_نقطہ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  بنانشانی Wide = CommonFXSema.get_چوڑائی() * 2;
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
  APSInt نتیجہ;
  إذا (CommonFXSema.isSigned())
    نتیجہ = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.get_Lsb_بھار());
  وإلا
    نتیجہ = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.get_Lsb_بھار());
  أكد(!Overflowed && "Full multiplication cannot overflow!");
  نتیجہ.سیٹ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطہ::getMax(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_نقطہ::getMin(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  إذا (CommonFXSema.isSaturated()) {
    إذا (نتیجہ < Min)
      نتیجہ = std::move(Min);
    وإلا إذا (نتیجہ > Max)
      نتیجہ = std::move(Max);
  } وإلا {
    Overflowed = نتیجہ < Min || نتیجہ > Max;
  }

  إذا (حد_توں_ودھ)
    *حد_توں_ودھ = Overflowed;

  أرجع APFixed_نقطہ(نتیجہ.sextOrTrunc(CommonFXSema.get_چوڑائی()),
                      CommonFXSema);
}

APFixed_نقطہ APFixed_نقطہ::div(ثابت APFixed_نقطہ &Other,
                               منطقي *حد_توں_ودھ) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطہ ConvertedThis = convert(CommonFXSema);
  APFixed_نقطہ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //حد_توں_ودھ
  بنانشانی Wide =
      CommonFXSema.get_چوڑائی() * 2 + std::max(-CommonFXSema.get_Msb_بھار(), 0);
  إذا (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } وإلا {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  إذا (CommonFXSema.get_Lsb_بھار() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.get_Lsb_بھار());
  وإلا إذا (CommonFXSema.get_Lsb_بھار() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.get_Lsb_بھار());
  APSInt نتیجہ;
  إذا (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, نتیجہ, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    إذا (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      نتیجہ = نتیجہ - 1;
  } وإلا {
    نتیجہ = ThisVal.udiv(OtherVal);
  }
  نتیجہ.سیٹ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطہ::getMax(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_نقطہ::getMin(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  إذا (CommonFXSema.isSaturated()) {
    إذا (نتیجہ < Min)
      نتیجہ = std::move(Min);
    وإلا إذا (نتیجہ > Max)
      نتیجہ = std::move(Max);
  } وإلا {
    Overflowed = نتیجہ < Min || نتیجہ > Max;
  }

  إذا (حد_توں_ودھ)
    *حد_توں_ودھ = Overflowed;

  أرجع APFixed_نقطہ(نتیجہ.sextOrTrunc(CommonFXSema.get_چوڑائی()),
                      CommonFXSema);
}

APFixed_نقطہ APFixed_نقطہ::shl(بنانشانی Amt, منطقي *حد_توں_ودھ) ثابت {
  APSInt ThisVal = Val;
  منطقي Overflowed = خطأ;

  // Widen the LHS.
  بنانشانی Wide = Sema.get_چوڑائی() * 2;
  إذا (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  وإلا
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_ثنائی_ہندسہ_چوڑائی());
  APSInt نتیجہ = ThisVal << Amt;
  نتیجہ.سیٹ_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطہ::getMax(Sema).get_قدر().extOrTrunc(Wide);
  APSInt Min = APFixed_نقطہ::getMin(Sema).get_قدر().extOrTrunc(Wide);
  إذا (Sema.isSaturated()) {
    إذا (نتیجہ < Min)
      نتیجہ = std::move(Min);
    وإلا إذا (نتیجہ > Max)
      نتیجہ = std::move(Max);
  } وإلا {
    Overflowed = نتیجہ < Min || نتیجہ > Max;
  }

  إذا (حد_توں_ودھ)
    *حد_توں_ودھ = Overflowed;

  أرجع APFixed_نقطہ(نتیجہ.sextOrTrunc(Sema.get_چوڑائی()), Sema);
}

فراغ APFixed_نقطہ::منزل_String(Small_سمتیہ_Impl<حرف> &Str) ثابت {
  APSInt Val = get_قدر();
  صحيح Lsb = get_Lsb_بھار();
  صحيح Orig_چوڑائی = get_چوڑائی();

  إذا (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_ثنائی_ہندسہ_چوڑائی() + Lsb);
    IntPart <<= Lsb;
    IntPart.منزل_String(Str, /*Radix=*/10);
    Str.ڈالو_پٹھ('.');
    Str.ڈالو_پٹھ('0');
    أرجع;
  }

  إذا (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.سیٹ_Is_Unsigned(سچ);
    Str.ڈالو_پٹھ('-');
  }

  صحيح Scale = -get_Lsb_بھار();
  APSInt IntPart = (Orig_چوڑائی > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  بنانشانی چوڑائی = std::max(Orig_چوڑائی, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(چوڑائی);
  APInt Fract_Part_نقاب = APInt::getAllOnes(Scale).zext(چوڑائی);
  APInt RadixInt = APInt(چوڑائی, 10);

  IntPart.منزل_String(Str, /*Radix=*/10);
  Str.ڈالو_پٹھ('.');
  افعل {
    (FractPart * RadixInt)
        .lshr(Scale)
        .منزل_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_نقاب;
  } طالما (FractPart != 0);
}

فراغ APFixed_نقطہ::print(raw_ostream &OS) ثابت {
  OS << "APFixedPoint(" << منزل_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ فراغ APFixed_نقطہ::dump() ثابت { print(llvm::errs()); }
#جے_انت

APFixed_نقطہ APFixed_نقطہ::negate(منطقي *حد_توں_ودھ) ثابت {
  إذا (!isSaturated()) {
    إذا (حد_توں_ودھ)
      *حد_توں_ودھ =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_قدر());
    أرجع APFixed_نقطہ(-Val, Sema);
  }

  // We never overflow for saturation
  إذا (حد_توں_ودھ)
    *حد_توں_ودھ = خطأ;

  إذا (isSigned())
    أرجع Val.is_Min_Signed_قدر() ? getMax(Sema) : APFixed_نقطہ(-Val, Sema);
  وإلا
    أرجع APFixed_نقطہ(Sema);
}

APSInt APFixed_نقطہ::convert_منزل_Int(بنانشانی Dst_چوڑائی, منطقي DstSign,
                                  منطقي *حد_توں_ودھ) ثابت {
  APSInt نتیجہ = getIntPart();
  بنانشانی Src_چوڑائی = get_چوڑائی();

  APSInt DstMin = APSInt::get_Min_قدر(Dst_چوڑائی, !DstSign);
  APSInt DstMax = APSInt::get_Max_قدر(Dst_چوڑائی, !DstSign);

  إذا (Src_چوڑائی < Dst_چوڑائی) {
    نتیجہ = نتیجہ.extend(Dst_چوڑائی);
  } وإلا إذا (Src_چوڑائی > Dst_چوڑائی) {
    DstMin = DstMin.extend(Src_چوڑائی);
    DstMax = DstMax.extend(Src_چوڑائی);
  }

  إذا (حد_توں_ودھ) {
    إذا (نتیجہ.isSigned() && !DstSign) {
      *حد_توں_ودھ = نتیجہ.isNegative() || نتیجہ.ugt(DstMax);
    } وإلا إذا (نتیجہ.isUnsigned() && DstSign) {
      *حد_توں_ودھ = نتیجہ.ugt(DstMax);
    } وإلا {
      *حد_توں_ودھ = نتیجہ < DstMin || نتیجہ > DstMax;
    }
  }

  نتیجہ.سیٹ_Is_Signed(DstSign);
  أرجع نتیجہ.extOrTrunc(Dst_چوڑائی);
}

ثابت fltSemantics *APFixed_نقطہ::promoteFloatSemantics(ثابت fltSemantics *S) {
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

APFloat APFixed_نقطہ::convert_منزل_Float(ثابت fltSemantics &FloatSema) ثابت {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_منوال RM = APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Even;
  APFloat::rounding_منوال LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!Sema.سماندا_وچ_Float_Semantics(*OpSema))
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
  APFloat Scale_ضارب(std::قوت(2, Sema.get_Lsb_بھار()));
  منطقي Ignored;
  Scale_ضارب.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_ضارب, LosslessRM);

  إذا (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  أرجع Flt;
}

APFixed_نقطہ APFixed_نقطہ::get_From_Int_قدر(ثابت APSInt &قدر,
                                           ثابت Fixed_نقطہ_Semantics &DstFXSema,
                                           منطقي *حد_توں_ودھ) {
  Fixed_نقطہ_Semantics IntFXSema = Fixed_نقطہ_Semantics::GetIntegerSemantics(
      قدر.get_ثنائی_ہندسہ_چوڑائی(), قدر.isSigned());
  أرجع APFixed_نقطہ(قدر, IntFXSema).convert(DstFXSema, حد_توں_ودھ);
}

APFixed_نقطہ
APFixed_نقطہ::get_From_Float_قدر(ثابت APFloat &قدر,
                                ثابت Fixed_نقطہ_Semantics &DstFXSema,
                                منطقي *حد_توں_ودھ) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_منوال RM = APFloat::rmTowardZero;
  APFloat::rounding_منوال LosslessRM = APFloat::rmTowardZero;

  ثابت fltSemantics &FloatSema = قدر.getSemantics();

  إذا (قدر.isNaN()) {
    // Handle NaN immediately.
    إذا (حد_توں_ودھ)
      *حد_توں_ودھ = سچ;
    أرجع APFixed_نقطہ(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!DstFXSema.سماندا_وچ_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = قدر;

  منطقي Ignored;
  إذا (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_ضارب(std::قوت(2, -DstFXSema.get_Lsb_بھار()));
  Scale_ضارب.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_ضارب, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_چوڑائی(), !DstFXSema.isSigned());
  Val.convert_منزل_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_ضارب = APFloat(std::قوت(2, DstFXSema.get_Lsb_بھار()));
  Scale_ضارب.convert(*OpSema, LosslessRM, &Ignored);
  Val.گیڑا_منزل_Integral(RM);
  Val.multiply(Scale_ضارب, LosslessRM);

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

  إذا (حد_توں_ودھ)
    *حد_توں_ودھ = Overflowed;

  أرجع APFixed_نقطہ(Res, DstFXSema);
}

} // namespace llvm
