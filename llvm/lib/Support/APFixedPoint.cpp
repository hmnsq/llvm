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

فراغ Fixed_نقطه_Semantics::print(llvm::raw_ostream &OS) ثابت {
  OS << "width=" << get_عرض() << ", ";
  إذا (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << get_Msb_وزن() << ", ";
  OS << "lsb=" << get_Lsb_وزن() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t Fixed_نقطه_Semantics::مقصد_Opaque_Int() ثابت {
  أرجع llvm::بیت_cast<uint32_t>(*هذا);
}

Fixed_نقطه_Semantics Fixed_نقطه_Semantics::getFromOpaqueInt(uint32_t I) {
  Fixed_نقطه_Semantics F(0, 0, خطأ, خطأ, خطأ);
  std::کپی_حافظه(&F, &I, حجم(F));
  أرجع F;
}

APFixed_نقطه APFixed_نقطه::convert(ثابت Fixed_نقطه_Semantics &DstSema,
                                   منطقي *سرریز) ثابت {
  APSInt NewVal = Val;
  صحيح RelativeUpscale = get_Lsb_وزن() - DstSema.get_Lsb_وزن();
  إذا (سرریز)
    *سرریز = خطأ;

  إذا (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_بیت_عرض() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  خودکار ماسک = APInt::get_بیتها_مجموعه_From(
      NewVal.get_بیت_عرض(),
      std::min(DstSema.get_Integral_بیتها() - DstSema.get_Lsb_وزن(),
               NewVal.get_بیت_عرض()));
  APInt Masked(NewVal & ماسک);

  // Change in the bits above the sign
  إذا (!(Masked == ماسک || Masked == 0)) {
    // Found overflow in the bits above the sign
    إذا (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? ماسک : ~ماسک;
    وإلا إذا (سرریز)
      *سرریز = درست;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  إذا (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    إذا (DstSema.isSaturated())
      NewVal = 0;
    وإلا إذا (سرریز)
      *سرریز = درست;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_عرض());
  NewVal.مجموعه_Is_Signed(DstSema.isSigned());
  أرجع APFixed_نقطه(NewVal, DstSema);
}

صحيح APFixed_نقطه::compare(ثابت APFixed_نقطه &Other) ثابت {
  APSInt ThisVal = get_مقدار();
  APSInt OtherVal = Other.get_مقدار();
  منطقي ThisSigned = Val.isSigned();
  منطقي OtherSigned = OtherVal.isSigned();

  صحيح CommonLsb = std::min(get_Lsb_وزن(), Other.get_Lsb_وزن());
  صحيح CommonMsb = std::max(get_Msb_وزن(), Other.get_Msb_وزن());
  بیعلامت Common_عرض = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_عرض);
  OtherVal = OtherVal.extOrTrunc(Common_عرض);

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
    إذا (ThisVal.is_Sign_بیت_مجموعه())
      أرجع -1;
    وإلا إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  } وإلا {
    // !ThisSigned && OtherSigned
    إذا (OtherVal.is_Sign_بیت_مجموعه())
      أرجع 1;
    وإلا إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  }

  أرجع 0;
}

APFixed_نقطه APFixed_نقطه::getMax(ثابت Fixed_نقطه_Semantics &Sema) {
  منطقي IsUnsigned = !Sema.isSigned();
  خودکار Val = APSInt::get_Max_مقدار(Sema.get_عرض(), IsUnsigned);
  إذا (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  أرجع APFixed_نقطه(Val, Sema);
}

APFixed_نقطه APFixed_نقطه::getMin(ثابت Fixed_نقطه_Semantics &Sema) {
  خودکار Val = APSInt::get_Min_مقدار(Sema.get_عرض(), !Sema.isSigned());
  أرجع APFixed_نقطه(Val, Sema);
}

APFixed_نقطه APFixed_نقطه::getEpsilon(ثابت Fixed_نقطه_Semantics &Sema) {
  APSInt Val(Sema.get_عرض(), !Sema.isSigned());
  Val.مجموعه_بیت(/*BitPosition=*/0);
  أرجع APFixed_نقطه(Val, Sema);
}

منطقي Fixed_نقطه_Semantics::میگنجد_درون_Float_Semantics(
    ثابت fltSemantics &FloatSema) ثابت {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixed_نقطه::getMax(*هذا).get_مقدار();
  APFloat F(FloatSema);
  APFloat::op_وضعیت وضعیت = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_نزدیکترین_Ties_مقصد_Away);
  إذا ((وضعیت & APFloat::op_سرریز) || !isSigned())
    أرجع !(وضعیت & APFloat::op_سرریز);

  APSInt MinInt = APFixed_نقطه::getMin(*هذا).get_مقدار();
  وضعیت = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_نزدیکترین_Ties_مقصد_Away);
  أرجع !(وضعیت & APFloat::op_سرریز);
}

Fixed_نقطه_Semantics Fixed_نقطه_Semantics::getCommonSemantics(
    ثابت Fixed_نقطه_Semantics &Other) ثابت {
  صحيح CommonLsb = std::min(get_Lsb_وزن(), Other.get_Lsb_وزن());
  صحيح CommonMSb = std::max(get_Msb_وزن() - has_Sign_Or_Padding_بیت(),
                           Other.get_Msb_وزن() - Other.has_Sign_Or_Padding_بیت());
  بیعلامت Common_عرض = CommonMSb - CommonLsb + 1;

  منطقي نتیجه_Is_Signed = isSigned() || Other.isSigned();
  منطقي نتیجه_Is_Saturated = isSaturated() || Other.isSaturated();
  منطقي نتیجه_Has_Unsigned_Padding = خطأ;
  إذا (!نتیجه_Is_Signed) {
    // Both are unsigned.
    نتیجه_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !نتیجه_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  إذا (نتیجه_Is_Signed || نتیجه_Has_Unsigned_Padding)
    Common_عرض++;

  أرجع Fixed_نقطه_Semantics(Common_عرض, Lsb{CommonLsb}, نتیجه_Is_Signed,
                             نتیجه_Is_Saturated, نتیجه_Has_Unsigned_Padding);
}

APFixed_نقطه APFixed_نقطه::جمع_کن(ثابت APFixed_نقطه &Other,
                               منطقي *سرریز) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطه ConvertedThis = convert(CommonFXSema);
  APFixed_نقطه ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_مقدار();
  APSInt OtherVal = ConvertedOther.get_مقدار();
  منطقي Overflowed = خطأ;

  APSInt نتیجه;
  إذا (CommonFXSema.isSaturated()) {
    نتیجه = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } وإلا {
    نتیجه = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  إذا (سرریز)
    *سرریز = Overflowed;

  أرجع APFixed_نقطه(نتیجه, CommonFXSema);
}

APFixed_نقطه APFixed_نقطه::sub(ثابت APFixed_نقطه &Other,
                               منطقي *سرریز) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطه ConvertedThis = convert(CommonFXSema);
  APFixed_نقطه ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_مقدار();
  APSInt OtherVal = ConvertedOther.get_مقدار();
  منطقي Overflowed = خطأ;

  APSInt نتیجه;
  إذا (CommonFXSema.isSaturated()) {
    نتیجه = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } وإلا {
    نتیجه = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  إذا (سرریز)
    *سرریز = Overflowed;

  أرجع APFixed_نقطه(نتیجه, CommonFXSema);
}

APFixed_نقطه APFixed_نقطه::mul(ثابت APFixed_نقطه &Other,
                               منطقي *سرریز) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطه ConvertedThis = convert(CommonFXSema);
  APFixed_نقطه ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_مقدار();
  APSInt OtherVal = ConvertedOther.get_مقدار();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  بیعلامت Wide = CommonFXSema.get_عرض() * 2;
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
  APSInt نتیجه;
  إذا (CommonFXSema.isSigned())
    نتیجه = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.get_Lsb_وزن());
  وإلا
    نتیجه = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.get_Lsb_وزن());
  أكد(!Overflowed && "Full multiplication cannot overflow!");
  نتیجه.مجموعه_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطه::getMax(CommonFXSema).get_مقدار()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_نقطه::getMin(CommonFXSema).get_مقدار()
                                                 .extOrTrunc(Wide);
  إذا (CommonFXSema.isSaturated()) {
    إذا (نتیجه < Min)
      نتیجه = std::move(Min);
    وإلا إذا (نتیجه > Max)
      نتیجه = std::move(Max);
  } وإلا {
    Overflowed = نتیجه < Min || نتیجه > Max;
  }

  إذا (سرریز)
    *سرریز = Overflowed;

  أرجع APFixed_نقطه(نتیجه.sextOrTrunc(CommonFXSema.get_عرض()),
                      CommonFXSema);
}

APFixed_نقطه APFixed_نقطه::div(ثابت APFixed_نقطه &Other,
                               منطقي *سرریز) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطه ConvertedThis = convert(CommonFXSema);
  APFixed_نقطه ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_مقدار();
  APSInt OtherVal = ConvertedOther.get_مقدار();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //سرریز
  بیعلامت Wide =
      CommonFXSema.get_عرض() * 2 + std::max(-CommonFXSema.get_Msb_وزن(), 0);
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
  APSInt نتیجه;
  إذا (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, نتیجه, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    إذا (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      نتیجه = نتیجه - 1;
  } وإلا {
    نتیجه = ThisVal.udiv(OtherVal);
  }
  نتیجه.مجموعه_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطه::getMax(CommonFXSema).get_مقدار()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_نقطه::getMin(CommonFXSema).get_مقدار()
                                                 .extOrTrunc(Wide);
  إذا (CommonFXSema.isSaturated()) {
    إذا (نتیجه < Min)
      نتیجه = std::move(Min);
    وإلا إذا (نتیجه > Max)
      نتیجه = std::move(Max);
  } وإلا {
    Overflowed = نتیجه < Min || نتیجه > Max;
  }

  إذا (سرریز)
    *سرریز = Overflowed;

  أرجع APFixed_نقطه(نتیجه.sextOrTrunc(CommonFXSema.get_عرض()),
                      CommonFXSema);
}

APFixed_نقطه APFixed_نقطه::shl(بیعلامت Amt, منطقي *سرریز) ثابت {
  APSInt ThisVal = Val;
  منطقي Overflowed = خطأ;

  // Widen the LHS.
  بیعلامت Wide = Sema.get_عرض() * 2;
  إذا (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  وإلا
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_بیت_عرض());
  APSInt نتیجه = ThisVal << Amt;
  نتیجه.مجموعه_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطه::getMax(Sema).get_مقدار().extOrTrunc(Wide);
  APSInt Min = APFixed_نقطه::getMin(Sema).get_مقدار().extOrTrunc(Wide);
  إذا (Sema.isSaturated()) {
    إذا (نتیجه < Min)
      نتیجه = std::move(Min);
    وإلا إذا (نتیجه > Max)
      نتیجه = std::move(Max);
  } وإلا {
    Overflowed = نتیجه < Min || نتیجه > Max;
  }

  إذا (سرریز)
    *سرریز = Overflowed;

  أرجع APFixed_نقطه(نتیجه.sextOrTrunc(Sema.get_عرض()), Sema);
}

فراغ APFixed_نقطه::مقصد_String(Small_وکتور_Impl<حرف> &Str) ثابت {
  APSInt Val = get_مقدار();
  صحيح Lsb = get_Lsb_وزن();
  صحيح Orig_عرض = get_عرض();

  إذا (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_بیت_عرض() + Lsb);
    IntPart <<= Lsb;
    IntPart.مقصد_String(Str, /*Radix=*/10);
    Str.افزودن_پشت('.');
    Str.افزودن_پشت('0');
    أرجع;
  }

  إذا (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.مجموعه_Is_Unsigned(درست);
    Str.افزودن_پشت('-');
  }

  صحيح Scale = -get_Lsb_وزن();
  APSInt IntPart = (Orig_عرض > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  بیعلامت عرض = std::max(Orig_عرض, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(عرض);
  APInt Fract_Part_ماسک = APInt::getAllOnes(Scale).zext(عرض);
  APInt RadixInt = APInt(عرض, 10);

  IntPart.مقصد_String(Str, /*Radix=*/10);
  Str.افزودن_پشت('.');
  افعل {
    (FractPart * RadixInt)
        .lshr(Scale)
        .مقصد_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_ماسک;
  } طالما (FractPart != 0);
}

فراغ APFixed_نقطه::print(raw_ostream &OS) ثابت {
  OS << "APFixedPoint(" << مقصد_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_روش فراغ APFixed_نقطه::dump() ثابت { print(llvm::errs()); }
#پایان_اگر

APFixed_نقطه APFixed_نقطه::negate(منطقي *سرریز) ثابت {
  إذا (!isSaturated()) {
    إذا (سرریز)
      *سرریز =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_مقدار());
    أرجع APFixed_نقطه(-Val, Sema);
  }

  // We never overflow for saturation
  إذا (سرریز)
    *سرریز = خطأ;

  إذا (isSigned())
    أرجع Val.is_Min_Signed_مقدار() ? getMax(Sema) : APFixed_نقطه(-Val, Sema);
  وإلا
    أرجع APFixed_نقطه(Sema);
}

APSInt APFixed_نقطه::convert_مقصد_Int(بیعلامت Dst_عرض, منطقي DstSign,
                                  منطقي *سرریز) ثابت {
  APSInt نتیجه = getIntPart();
  بیعلامت Src_عرض = get_عرض();

  APSInt DstMin = APSInt::get_Min_مقدار(Dst_عرض, !DstSign);
  APSInt DstMax = APSInt::get_Max_مقدار(Dst_عرض, !DstSign);

  إذا (Src_عرض < Dst_عرض) {
    نتیجه = نتیجه.extend(Dst_عرض);
  } وإلا إذا (Src_عرض > Dst_عرض) {
    DstMin = DstMin.extend(Src_عرض);
    DstMax = DstMax.extend(Src_عرض);
  }

  إذا (سرریز) {
    إذا (نتیجه.isSigned() && !DstSign) {
      *سرریز = نتیجه.isNegative() || نتیجه.ugt(DstMax);
    } وإلا إذا (نتیجه.isUnsigned() && DstSign) {
      *سرریز = نتیجه.ugt(DstMax);
    } وإلا {
      *سرریز = نتیجه < DstMin || نتیجه > DstMax;
    }
  }

  نتیجه.مجموعه_Is_Signed(DstSign);
  أرجع نتیجه.extOrTrunc(Dst_عرض);
}

ثابت fltSemantics *APFixed_نقطه::promoteFloatSemantics(ثابت fltSemantics *S) {
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

APFloat APFixed_نقطه::convert_مقصد_Float(ثابت fltSemantics &FloatSema) ثابت {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_نما RM = APFloat::rm_نزدیکترین_Ties_مقصد_Even;
  APFloat::rounding_نما LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!Sema.میگنجد_درون_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::op_وضعیت S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (فراغ)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat Scale_ضریب(std::توان(2, Sema.get_Lsb_وزن()));
  منطقي Ignored;
  Scale_ضریب.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_ضریب, LosslessRM);

  إذا (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  أرجع Flt;
}

APFixed_نقطه APFixed_نقطه::get_From_Int_مقدار(ثابت APSInt &مقدار,
                                           ثابت Fixed_نقطه_Semantics &DstFXSema,
                                           منطقي *سرریز) {
  Fixed_نقطه_Semantics IntFXSema = Fixed_نقطه_Semantics::GetIntegerSemantics(
      مقدار.get_بیت_عرض(), مقدار.isSigned());
  أرجع APFixed_نقطه(مقدار, IntFXSema).convert(DstFXSema, سرریز);
}

APFixed_نقطه
APFixed_نقطه::get_From_Float_مقدار(ثابت APFloat &مقدار,
                                ثابت Fixed_نقطه_Semantics &DstFXSema,
                                منطقي *سرریز) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_نما RM = APFloat::rmTowardZero;
  APFloat::rounding_نما LosslessRM = APFloat::rmTowardZero;

  ثابت fltSemantics &FloatSema = مقدار.getSemantics();

  إذا (مقدار.isNaN()) {
    // Handle NaN immediately.
    إذا (سرریز)
      *سرریز = درست;
    أرجع APFixed_نقطه(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!DstFXSema.میگنجد_درون_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = مقدار;

  منطقي Ignored;
  إذا (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_ضریب(std::توان(2, -DstFXSema.get_Lsb_وزن()));
  Scale_ضریب.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_ضریب, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_عرض(), !DstFXSema.isSigned());
  Val.convert_مقصد_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_ضریب = APFloat(std::توان(2, DstFXSema.get_Lsb_وزن()));
  Scale_ضریب.convert(*OpSema, LosslessRM, &Ignored);
  Val.دور_مقصد_Integral(RM);
  Val.multiply(Scale_ضریب, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convert_مقصد_Float(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convert_مقصد_Float(*OpSema);
  منطقي Overflowed = خطأ;
  إذا (DstFXSema.isSaturated()) {
    إذا (Val > FloatMax)
      Res = getMax(DstFXSema).get_مقدار();
    وإلا إذا (Val < FloatMin)
      Res = getMin(DstFXSema).get_مقدار();
  } وإلا {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  إذا (سرریز)
    *سرریز = Overflowed;

  أرجع APFixed_نقطه(Res, DstFXSema);
}

} // namespace llvm
