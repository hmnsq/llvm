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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APFixedPoint.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APFloat.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <cmath>

ਨਾਮਾਕਾਸ਼ llvm {

ਖਾਲੀ Fixed_ਬਿੰਦੂ_Semantics::print(llvm::raw_ostream &OS) ਅਚਲ {
  OS << "width=" << get_ਚੌੜਾਈ() << ", ";
  ਜੇ (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << get_Msb_ਭਾਰ() << ", ";
  OS << "lsb=" << get_Lsb_ਭਾਰ() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t Fixed_ਬਿੰਦੂ_Semantics::ਮੰਜ਼ਿਲ_Opaque_Int() ਅਚਲ {
  ਵਾਪਸ llvm::ਦੋਆਧਾਰੀ_ਅੰਕ_cast<uint32_t>(*ਇਹ);
}

Fixed_ਬਿੰਦੂ_Semantics Fixed_ਬਿੰਦੂ_Semantics::getFromOpaqueInt(uint32_t I) {
  Fixed_ਬਿੰਦੂ_Semantics F(0, 0, ਝੂਠ, ਝੂਠ, ਝੂਠ);
  std::ਸਮ੍ਰਿਤੀ_ਨਕਲ(&F, &I, ਆਕਾਰ(F));
  ਵਾਪਸ F;
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::convert(ਅਚਲ Fixed_ਬਿੰਦੂ_Semantics &DstSema,
                                   ਤਾਰਕਿਕ *ਉਛਾਲ) ਅਚਲ {
  APSInt NewVal = Val;
  ਪੂਰਨ RelativeUpscale = get_Lsb_ਭਾਰ() - DstSema.get_Lsb_ਭਾਰ();
  ਜੇ (ਉਛਾਲ)
    *ਉਛਾਲ = ਝੂਠ;

  ਜੇ (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  ਸਵੈਚਾਲਿਤ ਨਕਾਬ = APInt::get_ਬਿੱਟ_ਸੈੱਟ_From(
      NewVal.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(),
      std::min(DstSema.get_Integral_ਬਿੱਟ() - DstSema.get_Lsb_ਭਾਰ(),
               NewVal.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()));
  APInt Masked(NewVal & ਨਕਾਬ);

  // Change in the bits above the sign
  ਜੇ (!(Masked == ਨਕਾਬ || Masked == 0)) {
    // Found overflow in the bits above the sign
    ਜੇ (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? ਨਕਾਬ : ~ਨਕਾਬ;
    ਨਹੀਂਤਾਂ ਜੇ (ਉਛਾਲ)
      *ਉਛਾਲ = ਸੱਚ;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  ਜੇ (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    ਜੇ (DstSema.isSaturated())
      NewVal = 0;
    ਨਹੀਂਤਾਂ ਜੇ (ਉਛਾਲ)
      *ਉਛਾਲ = ਸੱਚ;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_ਚੌੜਾਈ());
  NewVal.ਸੈੱਟ_Is_Signed(DstSema.isSigned());
  ਵਾਪਸ APFixed_ਬਿੰਦੂ(NewVal, DstSema);
}

ਪੂਰਨ APFixed_ਬਿੰਦੂ::compare(ਅਚਲ APFixed_ਬਿੰਦੂ &Other) ਅਚਲ {
  APSInt ThisVal = get_ਮੁੱਲ();
  APSInt OtherVal = Other.get_ਮੁੱਲ();
  ਤਾਰਕਿਕ ThisSigned = Val.isSigned();
  ਤਾਰਕਿਕ OtherSigned = OtherVal.isSigned();

  ਪੂਰਨ CommonLsb = std::min(get_Lsb_ਭਾਰ(), Other.get_Lsb_ਭਾਰ());
  ਪੂਰਨ CommonMsb = std::max(get_Msb_ਭਾਰ(), Other.get_Msb_ਭਾਰ());
  ਬਿਨਾਨਿਸ਼ਾਨੀ Common_ਚੌੜਾਈ = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_ਚੌੜਾਈ);
  OtherVal = OtherVal.extOrTrunc(Common_ਚੌੜਾਈ);

  ThisVal = ThisVal.shl(get_Lsb_ਭਾਰ() - CommonLsb);
  OtherVal = OtherVal.shl(Other.get_Lsb_ਭਾਰ() - CommonLsb);

  ਜੇ (ThisSigned && OtherSigned) {
    ਜੇ (ThisVal.sgt(OtherVal))
      ਵਾਪਸ 1;
    ਨਹੀਂਤਾਂ ਜੇ (ThisVal.slt(OtherVal))
      ਵਾਪਸ -1;
  } ਨਹੀਂਤਾਂ ਜੇ (!ThisSigned && !OtherSigned) {
    ਜੇ (ThisVal.ugt(OtherVal))
      ਵਾਪਸ 1;
    ਨਹੀਂਤਾਂ ਜੇ (ThisVal.ult(OtherVal))
      ਵਾਪਸ -1;
  } ਨਹੀਂਤਾਂ ਜੇ (ThisSigned && !OtherSigned) {
    ਜੇ (ThisVal.is_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ())
      ਵਾਪਸ -1;
    ਨਹੀਂਤਾਂ ਜੇ (ThisVal.ugt(OtherVal))
      ਵਾਪਸ 1;
    ਨਹੀਂਤਾਂ ਜੇ (ThisVal.ult(OtherVal))
      ਵਾਪਸ -1;
  } ਨਹੀਂਤਾਂ {
    // !ThisSigned && OtherSigned
    ਜੇ (OtherVal.is_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ())
      ਵਾਪਸ 1;
    ਨਹੀਂਤਾਂ ਜੇ (ThisVal.ugt(OtherVal))
      ਵਾਪਸ 1;
    ਨਹੀਂਤਾਂ ਜੇ (ThisVal.ult(OtherVal))
      ਵਾਪਸ -1;
  }

  ਵਾਪਸ 0;
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::getMax(ਅਚਲ Fixed_ਬਿੰਦੂ_Semantics &Sema) {
  ਤਾਰਕਿਕ IsUnsigned = !Sema.isSigned();
  ਸਵੈਚਾਲਿਤ Val = APSInt::get_Max_ਮੁੱਲ(Sema.get_ਚੌੜਾਈ(), IsUnsigned);
  ਜੇ (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  ਵਾਪਸ APFixed_ਬਿੰਦੂ(Val, Sema);
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::getMin(ਅਚਲ Fixed_ਬਿੰਦੂ_Semantics &Sema) {
  ਸਵੈਚਾਲਿਤ Val = APSInt::get_Min_ਮੁੱਲ(Sema.get_ਚੌੜਾਈ(), !Sema.isSigned());
  ਵਾਪਸ APFixed_ਬਿੰਦੂ(Val, Sema);
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::getEpsilon(ਅਚਲ Fixed_ਬਿੰਦੂ_Semantics &Sema) {
  APSInt Val(Sema.get_ਚੌੜਾਈ(), !Sema.isSigned());
  Val.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(/*BitPosition=*/0);
  ਵਾਪਸ APFixed_ਬਿੰਦੂ(Val, Sema);
}

ਤਾਰਕਿਕ Fixed_ਬਿੰਦੂ_Semantics::ਸਮਾਉਂਦਾ_ਵਿੱਚ_Float_Semantics(
    ਅਚਲ fltSemantics &FloatSema) ਅਚਲ {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixed_ਬਿੰਦੂ::getMax(*ਇਹ).get_ਮੁੱਲ();
  APFloat F(FloatSema);
  APFloat::op_ਸਥਿਤੀ ਸਥਿਤੀ = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away);
  ਜੇ ((ਸਥਿਤੀ & APFloat::op_ਉਛਾਲ) || !isSigned())
    ਵਾਪਸ !(ਸਥਿਤੀ & APFloat::op_ਉਛਾਲ);

  APSInt MinInt = APFixed_ਬਿੰਦੂ::getMin(*ਇਹ).get_ਮੁੱਲ();
  ਸਥਿਤੀ = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away);
  ਵਾਪਸ !(ਸਥਿਤੀ & APFloat::op_ਉਛਾਲ);
}

Fixed_ਬਿੰਦੂ_Semantics Fixed_ਬਿੰਦੂ_Semantics::getCommonSemantics(
    ਅਚਲ Fixed_ਬਿੰਦੂ_Semantics &Other) ਅਚਲ {
  ਪੂਰਨ CommonLsb = std::min(get_Lsb_ਭਾਰ(), Other.get_Lsb_ਭਾਰ());
  ਪੂਰਨ CommonMSb = std::max(get_Msb_ਭਾਰ() - has_Sign_Or_Padding_ਦੋਆਧਾਰੀ_ਅੰਕ(),
                           Other.get_Msb_ਭਾਰ() - Other.has_Sign_Or_Padding_ਦੋਆਧਾਰੀ_ਅੰਕ());
  ਬਿਨਾਨਿਸ਼ਾਨੀ Common_ਚੌੜਾਈ = CommonMSb - CommonLsb + 1;

  ਤਾਰਕਿਕ ਨਤੀਜਾ_Is_Signed = isSigned() || Other.isSigned();
  ਤਾਰਕਿਕ ਨਤੀਜਾ_Is_Saturated = isSaturated() || Other.isSaturated();
  ਤਾਰਕਿਕ ਨਤੀਜਾ_Has_Unsigned_Padding = ਝੂਠ;
  ਜੇ (!ਨਤੀਜਾ_Is_Signed) {
    // Both are unsigned.
    ਨਤੀਜਾ_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ਨਤੀਜਾ_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  ਜੇ (ਨਤੀਜਾ_Is_Signed || ਨਤੀਜਾ_Has_Unsigned_Padding)
    Common_ਚੌੜਾਈ++;

  ਵਾਪਸ Fixed_ਬਿੰਦੂ_Semantics(Common_ਚੌੜਾਈ, Lsb{CommonLsb}, ਨਤੀਜਾ_Is_Signed,
                             ਨਤੀਜਾ_Is_Saturated, ਨਤੀਜਾ_Has_Unsigned_Padding);
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::ਜੋੜੋ(ਅਚਲ APFixed_ਬਿੰਦੂ &Other,
                               ਤਾਰਕਿਕ *ਉਛਾਲ) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ਬਿੰਦੂ ConvertedThis = convert(CommonFXSema);
  APFixed_ਬਿੰਦੂ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ਮੁੱਲ();
  APSInt OtherVal = ConvertedOther.get_ਮੁੱਲ();
  ਤਾਰਕਿਕ Overflowed = ਝੂਠ;

  APSInt ਨਤੀਜਾ;
  ਜੇ (CommonFXSema.isSaturated()) {
    ਨਤੀਜਾ = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } ਨਹੀਂਤਾਂ {
    ਨਤੀਜਾ = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  ਜੇ (ਉਛਾਲ)
    *ਉਛਾਲ = Overflowed;

  ਵਾਪਸ APFixed_ਬਿੰਦੂ(ਨਤੀਜਾ, CommonFXSema);
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::sub(ਅਚਲ APFixed_ਬਿੰਦੂ &Other,
                               ਤਾਰਕਿਕ *ਉਛਾਲ) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ਬਿੰਦੂ ConvertedThis = convert(CommonFXSema);
  APFixed_ਬਿੰਦੂ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ਮੁੱਲ();
  APSInt OtherVal = ConvertedOther.get_ਮੁੱਲ();
  ਤਾਰਕਿਕ Overflowed = ਝੂਠ;

  APSInt ਨਤੀਜਾ;
  ਜੇ (CommonFXSema.isSaturated()) {
    ਨਤੀਜਾ = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } ਨਹੀਂਤਾਂ {
    ਨਤੀਜਾ = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  ਜੇ (ਉਛਾਲ)
    *ਉਛਾਲ = Overflowed;

  ਵਾਪਸ APFixed_ਬਿੰਦੂ(ਨਤੀਜਾ, CommonFXSema);
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::mul(ਅਚਲ APFixed_ਬਿੰਦੂ &Other,
                               ਤਾਰਕਿਕ *ਉਛਾਲ) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ਬਿੰਦੂ ConvertedThis = convert(CommonFXSema);
  APFixed_ਬਿੰਦੂ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ਮੁੱਲ();
  APSInt OtherVal = ConvertedOther.get_ਮੁੱਲ();
  ਤਾਰਕਿਕ Overflowed = ਝੂਠ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Wide = CommonFXSema.get_ਚੌੜਾਈ() * 2;
  ਜੇ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ਨਹੀਂਤਾਂ {
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
  APSInt ਨਤੀਜਾ;
  ਜੇ (CommonFXSema.isSigned())
    ਨਤੀਜਾ = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.get_Lsb_ਭਾਰ());
  ਨਹੀਂਤਾਂ
    ਨਤੀਜਾ = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.get_Lsb_ਭਾਰ());
  ਪ੍ਰਤਿਜ੍ਞਾ(!Overflowed && "Full multiplication cannot overflow!");
  ਨਤੀਜਾ.ਸੈੱਟ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ਬਿੰਦੂ::getMax(CommonFXSema).get_ਮੁੱਲ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_ਬਿੰਦੂ::getMin(CommonFXSema).get_ਮੁੱਲ()
                                                 .extOrTrunc(Wide);
  ਜੇ (CommonFXSema.isSaturated()) {
    ਜੇ (ਨਤੀਜਾ < Min)
      ਨਤੀਜਾ = std::move(Min);
    ਨਹੀਂਤਾਂ ਜੇ (ਨਤੀਜਾ > Max)
      ਨਤੀਜਾ = std::move(Max);
  } ਨਹੀਂਤਾਂ {
    Overflowed = ਨਤੀਜਾ < Min || ਨਤੀਜਾ > Max;
  }

  ਜੇ (ਉਛਾਲ)
    *ਉਛਾਲ = Overflowed;

  ਵਾਪਸ APFixed_ਬਿੰਦੂ(ਨਤੀਜਾ.sextOrTrunc(CommonFXSema.get_ਚੌੜਾਈ()),
                      CommonFXSema);
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::div(ਅਚਲ APFixed_ਬਿੰਦੂ &Other,
                               ਤਾਰਕਿਕ *ਉਛਾਲ) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ਬਿੰਦੂ ConvertedThis = convert(CommonFXSema);
  APFixed_ਬਿੰਦੂ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ਮੁੱਲ();
  APSInt OtherVal = ConvertedOther.get_ਮੁੱਲ();
  ਤਾਰਕਿਕ Overflowed = ਝੂਠ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //ਉਛਾਲ
  ਬਿਨਾਨਿਸ਼ਾਨੀ Wide =
      CommonFXSema.get_ਚੌੜਾਈ() * 2 + std::max(-CommonFXSema.get_Msb_ਭਾਰ(), 0);
  ਜੇ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ਨਹੀਂਤਾਂ {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  ਜੇ (CommonFXSema.get_Lsb_ਭਾਰ() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.get_Lsb_ਭਾਰ());
  ਨਹੀਂਤਾਂ ਜੇ (CommonFXSema.get_Lsb_ਭਾਰ() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.get_Lsb_ਭਾਰ());
  APSInt ਨਤੀਜਾ;
  ਜੇ (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, ਨਤੀਜਾ, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    ਜੇ (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      ਨਤੀਜਾ = ਨਤੀਜਾ - 1;
  } ਨਹੀਂਤਾਂ {
    ਨਤੀਜਾ = ThisVal.udiv(OtherVal);
  }
  ਨਤੀਜਾ.ਸੈੱਟ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ਬਿੰਦੂ::getMax(CommonFXSema).get_ਮੁੱਲ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_ਬਿੰਦੂ::getMin(CommonFXSema).get_ਮੁੱਲ()
                                                 .extOrTrunc(Wide);
  ਜੇ (CommonFXSema.isSaturated()) {
    ਜੇ (ਨਤੀਜਾ < Min)
      ਨਤੀਜਾ = std::move(Min);
    ਨਹੀਂਤਾਂ ਜੇ (ਨਤੀਜਾ > Max)
      ਨਤੀਜਾ = std::move(Max);
  } ਨਹੀਂਤਾਂ {
    Overflowed = ਨਤੀਜਾ < Min || ਨਤੀਜਾ > Max;
  }

  ਜੇ (ਉਛਾਲ)
    *ਉਛਾਲ = Overflowed;

  ਵਾਪਸ APFixed_ਬਿੰਦੂ(ਨਤੀਜਾ.sextOrTrunc(CommonFXSema.get_ਚੌੜਾਈ()),
                      CommonFXSema);
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::shl(ਬਿਨਾਨਿਸ਼ਾਨੀ Amt, ਤਾਰਕਿਕ *ਉਛਾਲ) ਅਚਲ {
  APSInt ThisVal = Val;
  ਤਾਰਕਿਕ Overflowed = ਝੂਠ;

  // Widen the LHS.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Wide = Sema.get_ਚੌੜਾਈ() * 2;
  ਜੇ (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  ਨਹੀਂਤਾਂ
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  APSInt ਨਤੀਜਾ = ThisVal << Amt;
  ਨਤੀਜਾ.ਸੈੱਟ_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ਬਿੰਦੂ::getMax(Sema).get_ਮੁੱਲ().extOrTrunc(Wide);
  APSInt Min = APFixed_ਬਿੰਦੂ::getMin(Sema).get_ਮੁੱਲ().extOrTrunc(Wide);
  ਜੇ (Sema.isSaturated()) {
    ਜੇ (ਨਤੀਜਾ < Min)
      ਨਤੀਜਾ = std::move(Min);
    ਨਹੀਂਤਾਂ ਜੇ (ਨਤੀਜਾ > Max)
      ਨਤੀਜਾ = std::move(Max);
  } ਨਹੀਂਤਾਂ {
    Overflowed = ਨਤੀਜਾ < Min || ਨਤੀਜਾ > Max;
  }

  ਜੇ (ਉਛਾਲ)
    *ਉਛਾਲ = Overflowed;

  ਵਾਪਸ APFixed_ਬਿੰਦੂ(ਨਤੀਜਾ.sextOrTrunc(Sema.get_ਚੌੜਾਈ()), Sema);
}

ਖਾਲੀ APFixed_ਬਿੰਦੂ::ਮੰਜ਼ਿਲ_String(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Str) ਅਚਲ {
  APSInt Val = get_ਮੁੱਲ();
  ਪੂਰਨ Lsb = get_Lsb_ਭਾਰ();
  ਪੂਰਨ Orig_ਚੌੜਾਈ = get_ਚੌੜਾਈ();

  ਜੇ (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() + Lsb);
    IntPart <<= Lsb;
    IntPart.ਮੰਜ਼ਿਲ_String(Str, /*Radix=*/10);
    Str.ਧੱਕੋ_ਪਿੱਠ('.');
    Str.ਧੱਕੋ_ਪਿੱਠ('0');
    ਵਾਪਸ;
  }

  ਜੇ (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.ਸੈੱਟ_Is_Unsigned(ਸੱਚ);
    Str.ਧੱਕੋ_ਪਿੱਠ('-');
  }

  ਪੂਰਨ Scale = -get_Lsb_ਭਾਰ();
  APSInt IntPart = (Orig_ਚੌੜਾਈ > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = std::max(Orig_ਚੌੜਾਈ, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(ਚੌੜਾਈ);
  APInt Fract_Part_ਨਕਾਬ = APInt::getAllOnes(Scale).zext(ਚੌੜਾਈ);
  APInt RadixInt = APInt(ਚੌੜਾਈ, 10);

  IntPart.ਮੰਜ਼ਿਲ_String(Str, /*Radix=*/10);
  Str.ਧੱਕੋ_ਪਿੱਠ('.');
  ਕਰੋ {
    (FractPart * RadixInt)
        .lshr(Scale)
        .ਮੰਜ਼ਿਲ_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_ਨਕਾਬ;
  } ਜਦੋਂ (FractPart != 0);
}

ਖਾਲੀ APFixed_ਬਿੰਦੂ::print(raw_ostream &OS) ਅਚਲ {
  OS << "APFixedPoint(" << ਮੰਜ਼ਿਲ_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ APFixed_ਬਿੰਦੂ::dump() ਅਚਲ { print(llvm::errs()); }
#ਜੇ_ਅੰਤ

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::negate(ਤਾਰਕਿਕ *ਉਛਾਲ) ਅਚਲ {
  ਜੇ (!isSaturated()) {
    ਜੇ (ਉਛਾਲ)
      *ਉਛਾਲ =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_ਮੁੱਲ());
    ਵਾਪਸ APFixed_ਬਿੰਦੂ(-Val, Sema);
  }

  // We never overflow for saturation
  ਜੇ (ਉਛਾਲ)
    *ਉਛਾਲ = ਝੂਠ;

  ਜੇ (isSigned())
    ਵਾਪਸ Val.is_Min_Signed_ਮੁੱਲ() ? getMax(Sema) : APFixed_ਬਿੰਦੂ(-Val, Sema);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ APFixed_ਬਿੰਦੂ(Sema);
}

APSInt APFixed_ਬਿੰਦੂ::convert_ਮੰਜ਼ਿਲ_Int(ਬਿਨਾਨਿਸ਼ਾਨੀ Dst_ਚੌੜਾਈ, ਤਾਰਕਿਕ DstSign,
                                  ਤਾਰਕਿਕ *ਉਛਾਲ) ਅਚਲ {
  APSInt ਨਤੀਜਾ = getIntPart();
  ਬਿਨਾਨਿਸ਼ਾਨੀ Src_ਚੌੜਾਈ = get_ਚੌੜਾਈ();

  APSInt DstMin = APSInt::get_Min_ਮੁੱਲ(Dst_ਚੌੜਾਈ, !DstSign);
  APSInt DstMax = APSInt::get_Max_ਮੁੱਲ(Dst_ਚੌੜਾਈ, !DstSign);

  ਜੇ (Src_ਚੌੜਾਈ < Dst_ਚੌੜਾਈ) {
    ਨਤੀਜਾ = ਨਤੀਜਾ.extend(Dst_ਚੌੜਾਈ);
  } ਨਹੀਂਤਾਂ ਜੇ (Src_ਚੌੜਾਈ > Dst_ਚੌੜਾਈ) {
    DstMin = DstMin.extend(Src_ਚੌੜਾਈ);
    DstMax = DstMax.extend(Src_ਚੌੜਾਈ);
  }

  ਜੇ (ਉਛਾਲ) {
    ਜੇ (ਨਤੀਜਾ.isSigned() && !DstSign) {
      *ਉਛਾਲ = ਨਤੀਜਾ.isNegative() || ਨਤੀਜਾ.ugt(DstMax);
    } ਨਹੀਂਤਾਂ ਜੇ (ਨਤੀਜਾ.isUnsigned() && DstSign) {
      *ਉਛਾਲ = ਨਤੀਜਾ.ugt(DstMax);
    } ਨਹੀਂਤਾਂ {
      *ਉਛਾਲ = ਨਤੀਜਾ < DstMin || ਨਤੀਜਾ > DstMax;
    }
  }

  ਨਤੀਜਾ.ਸੈੱਟ_Is_Signed(DstSign);
  ਵਾਪਸ ਨਤੀਜਾ.extOrTrunc(Dst_ਚੌੜਾਈ);
}

ਅਚਲ fltSemantics *APFixed_ਬਿੰਦੂ::promoteFloatSemantics(ਅਚਲ fltSemantics *S) {
  ਜੇ (S == &APFloat::BFloat())
    ਵਾਪਸ &APFloat::IEEEdouble();
  ਨਹੀਂਤਾਂ ਜੇ (S == &APFloat::IEEEhalf())
    ਵਾਪਸ &APFloat::IEEEsingle();
  ਨਹੀਂਤਾਂ ਜੇ (S == &APFloat::IEEEsingle())
    ਵਾਪਸ &APFloat::IEEEdouble();
  ਨਹੀਂਤਾਂ ਜੇ (S == &APFloat::IEEEdouble())
    ਵਾਪਸ &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixed_ਬਿੰਦੂ::convert_ਮੰਜ਼ਿਲ_Float(ਅਚਲ fltSemantics &FloatSema) ਅਚਲ {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_ਬਹੁਲਕ RM = APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even;
  APFloat::rounding_ਬਹੁਲਕ LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ਅਚਲ fltSemantics *OpSema = &FloatSema;
  ਜਦੋਂ (!Sema.ਸਮਾਉਂਦਾ_ਵਿੱਚ_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::op_ਸਥਿਤੀ S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (ਖਾਲੀ)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat Scale_ਗੁਣਕ(std::ਘਾਤ(2, Sema.get_Lsb_ਭਾਰ()));
  ਤਾਰਕਿਕ Ignored;
  Scale_ਗੁਣਕ.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_ਗੁਣਕ, LosslessRM);

  ਜੇ (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  ਵਾਪਸ Flt;
}

APFixed_ਬਿੰਦੂ APFixed_ਬਿੰਦੂ::get_From_Int_ਮੁੱਲ(ਅਚਲ APSInt &ਮੁੱਲ,
                                           ਅਚਲ Fixed_ਬਿੰਦੂ_Semantics &DstFXSema,
                                           ਤਾਰਕਿਕ *ਉਛਾਲ) {
  Fixed_ਬਿੰਦੂ_Semantics IntFXSema = Fixed_ਬਿੰਦੂ_Semantics::GetIntegerSemantics(
      ਮੁੱਲ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), ਮੁੱਲ.isSigned());
  ਵਾਪਸ APFixed_ਬਿੰਦੂ(ਮੁੱਲ, IntFXSema).convert(DstFXSema, ਉਛਾਲ);
}

APFixed_ਬਿੰਦੂ
APFixed_ਬਿੰਦੂ::get_From_Float_ਮੁੱਲ(ਅਚਲ APFloat &ਮੁੱਲ,
                                ਅਚਲ Fixed_ਬਿੰਦੂ_Semantics &DstFXSema,
                                ਤਾਰਕਿਕ *ਉਛਾਲ) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_ਬਹੁਲਕ RM = APFloat::rmTowardZero;
  APFloat::rounding_ਬਹੁਲਕ LosslessRM = APFloat::rmTowardZero;

  ਅਚਲ fltSemantics &FloatSema = ਮੁੱਲ.getSemantics();

  ਜੇ (ਮੁੱਲ.isNaN()) {
    // Handle NaN immediately.
    ਜੇ (ਉਛਾਲ)
      *ਉਛਾਲ = ਸੱਚ;
    ਵਾਪਸ APFixed_ਬਿੰਦੂ(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ਅਚਲ fltSemantics *OpSema = &FloatSema;
  ਜਦੋਂ (!DstFXSema.ਸਮਾਉਂਦਾ_ਵਿੱਚ_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = ਮੁੱਲ;

  ਤਾਰਕਿਕ Ignored;
  ਜੇ (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_ਗੁਣਕ(std::ਘਾਤ(2, -DstFXSema.get_Lsb_ਭਾਰ()));
  Scale_ਗੁਣਕ.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_ਗੁਣਕ, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_ਚੌੜਾਈ(), !DstFXSema.isSigned());
  Val.convert_ਮੰਜ਼ਿਲ_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_ਗੁਣਕ = APFloat(std::ਘਾਤ(2, DstFXSema.get_Lsb_ਭਾਰ()));
  Scale_ਗੁਣਕ.convert(*OpSema, LosslessRM, &Ignored);
  Val.ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(RM);
  Val.multiply(Scale_ਗੁਣਕ, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convert_ਮੰਜ਼ਿਲ_Float(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convert_ਮੰਜ਼ਿਲ_Float(*OpSema);
  ਤਾਰਕਿਕ Overflowed = ਝੂਠ;
  ਜੇ (DstFXSema.isSaturated()) {
    ਜੇ (Val > FloatMax)
      Res = getMax(DstFXSema).get_ਮੁੱਲ();
    ਨਹੀਂਤਾਂ ਜੇ (Val < FloatMin)
      Res = getMin(DstFXSema).get_ਮੁੱਲ();
  } ਨਹੀਂਤਾਂ {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  ਜੇ (ਉਛਾਲ)
    *ਉਛਾਲ = Overflowed;

  ਵਾਪਸ APFixed_ਬਿੰਦੂ(Res, DstFXSema);
}

} // namespace llvm
