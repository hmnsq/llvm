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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFixedPoint.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFloat.h"

#ಅನ್ತರ್ಭಾವ <cmath>

ನಾಮಾಕಾಶ llvm {

ಶೂನ್ಯ Fixed_ಬಿಂದು_Semantics::print(llvm::raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << "width=" << get_ಅಗಲ() << ", ";
  ಆದರೆ (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << get_Msb_ತೂಕ() << ", ";
  OS << "lsb=" << get_Lsb_ತೂಕ() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t Fixed_ಬಿಂದು_Semantics::ಗಮ್ಯ_Opaque_Int() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ llvm::ಬಿಟ್_cast<uint32_t>(*ಇದು);
}

Fixed_ಬಿಂದು_Semantics Fixed_ಬಿಂದು_Semantics::getFromOpaqueInt(uint32_t I) {
  Fixed_ಬಿಂದು_Semantics F(0, 0, ಸುಳ್ಳು, ಸುಳ್ಳು, ಸುಳ್ಳು);
  std::ಸ್ಮರಣೆ_ನಕಲು(&F, &I, ಗಾತ್ರ(F));
  ಹಿಂತಿರುಗಿ F;
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::convert(ಸ್ಥಿರಾಂಕ Fixed_ಬಿಂದು_Semantics &DstSema,
                                   ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APSInt NewVal = Val;
  ಪೂರ್ಣಾಂಕ RelativeUpscale = get_Lsb_ತೂಕ() - DstSema.get_Lsb_ತೂಕ();
  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    *ಅತಿಪ್ರವಾಹ = ಸುಳ್ಳು;

  ಆದರೆ (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_ಬಿಟ್_ಅಗಲ() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  ಸ್ವಯಂಚಾಲಿತ ಮುಖವಾಡ = APInt::get_ಬಿಟ್ಟುಗಳು_ಗಣ_From(
      NewVal.get_ಬಿಟ್_ಅಗಲ(),
      std::min(DstSema.get_Integral_ಬಿಟ್ಟುಗಳು() - DstSema.get_Lsb_ತೂಕ(),
               NewVal.get_ಬಿಟ್_ಅಗಲ()));
  APInt Masked(NewVal & ಮುಖವಾಡ);

  // Change in the bits above the sign
  ಆದರೆ (!(Masked == ಮುಖವಾಡ || Masked == 0)) {
    // Found overflow in the bits above the sign
    ಆದರೆ (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? ಮುಖವಾಡ : ~ಮುಖವಾಡ;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅತಿಪ್ರವಾಹ)
      *ಅತಿಪ್ರವಾಹ = ನಿಜ;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  ಆದರೆ (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    ಆದರೆ (DstSema.isSaturated())
      NewVal = 0;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅತಿಪ್ರವಾಹ)
      *ಅತಿಪ್ರವಾಹ = ನಿಜ;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_ಅಗಲ());
  NewVal.ಗಣ_Is_Signed(DstSema.isSigned());
  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(NewVal, DstSema);
}

ಪೂರ್ಣಾಂಕ APFixed_ಬಿಂದು::compare(ಸ್ಥಿರಾಂಕ APFixed_ಬಿಂದು &Other) ಸ್ಥಿರಾಂಕ {
  APSInt ThisVal = get_ಮೌಲ್ಯ();
  APSInt OtherVal = Other.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ ThisSigned = Val.isSigned();
  ತಾರ್ಕಿಕ OtherSigned = OtherVal.isSigned();

  ಪೂರ್ಣಾಂಕ CommonLsb = std::min(get_Lsb_ತೂಕ(), Other.get_Lsb_ತೂಕ());
  ಪೂರ್ಣಾಂಕ CommonMsb = std::max(get_Msb_ತೂಕ(), Other.get_Msb_ತೂಕ());
  ಚಿಹ್ನೆರಹಿತ Common_ಅಗಲ = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_ಅಗಲ);
  OtherVal = OtherVal.extOrTrunc(Common_ಅಗಲ);

  ThisVal = ThisVal.shl(get_Lsb_ತೂಕ() - CommonLsb);
  OtherVal = OtherVal.shl(Other.get_Lsb_ತೂಕ() - CommonLsb);

  ಆದರೆ (ThisSigned && OtherSigned) {
    ಆದರೆ (ThisVal.sgt(OtherVal))
      ಹಿಂತಿರುಗಿ 1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ThisVal.slt(OtherVal))
      ಹಿಂತಿರುಗಿ -1;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ThisSigned && !OtherSigned) {
    ಆದರೆ (ThisVal.ugt(OtherVal))
      ಹಿಂತಿರುಗಿ 1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ThisVal.ult(OtherVal))
      ಹಿಂತಿರುಗಿ -1;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ThisSigned && !OtherSigned) {
    ಆದರೆ (ThisVal.is_Sign_ಬಿಟ್_ಗಣ())
      ಹಿಂತಿರುಗಿ -1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ThisVal.ugt(OtherVal))
      ಹಿಂತಿರುಗಿ 1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ThisVal.ult(OtherVal))
      ಹಿಂತಿರುಗಿ -1;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // !ThisSigned && OtherSigned
    ಆದರೆ (OtherVal.is_Sign_ಬಿಟ್_ಗಣ())
      ಹಿಂತಿರುಗಿ 1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ThisVal.ugt(OtherVal))
      ಹಿಂತಿರುಗಿ 1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ThisVal.ult(OtherVal))
      ಹಿಂತಿರುಗಿ -1;
  }

  ಹಿಂತಿರುಗಿ 0;
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::getMax(ಸ್ಥಿರಾಂಕ Fixed_ಬಿಂದು_Semantics &Sema) {
  ತಾರ್ಕಿಕ IsUnsigned = !Sema.isSigned();
  ಸ್ವಯಂಚಾಲಿತ Val = APSInt::get_Max_ಮೌಲ್ಯ(Sema.get_ಅಗಲ(), IsUnsigned);
  ಆದರೆ (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(Val, Sema);
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::getMin(ಸ್ಥಿರಾಂಕ Fixed_ಬಿಂದು_Semantics &Sema) {
  ಸ್ವಯಂಚಾಲಿತ Val = APSInt::get_Min_ಮೌಲ್ಯ(Sema.get_ಅಗಲ(), !Sema.isSigned());
  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(Val, Sema);
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::getEpsilon(ಸ್ಥಿರಾಂಕ Fixed_ಬಿಂದು_Semantics &Sema) {
  APSInt Val(Sema.get_ಅಗಲ(), !Sema.isSigned());
  Val.ಗಣ_ಬಿಟ್(/*BitPosition=*/0);
  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(Val, Sema);
}

ತಾರ್ಕಿಕ Fixed_ಬಿಂದು_Semantics::ಹೊಂದುತ್ತದೆ_ಒಳ_Float_Semantics(
    ಸ್ಥಿರಾಂಕ fltSemantics &FloatSema) ಸ್ಥಿರಾಂಕ {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixed_ಬಿಂದು::getMax(*ಇದು).get_ಮೌಲ್ಯ();
  APFloat F(FloatSema);
  APFloat::op_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2 = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away);
  ಆದರೆ ((ಸ್ಥಿತಿ_2 & APFloat::op_ಅತಿಪ್ರವಾಹ) || !isSigned())
    ಹಿಂತಿರುಗಿ !(ಸ್ಥಿತಿ_2 & APFloat::op_ಅತಿಪ್ರವಾಹ);

  APSInt MinInt = APFixed_ಬಿಂದು::getMin(*ಇದು).get_ಮೌಲ್ಯ();
  ಸ್ಥಿತಿ_2 = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away);
  ಹಿಂತಿರುಗಿ !(ಸ್ಥಿತಿ_2 & APFloat::op_ಅತಿಪ್ರವಾಹ);
}

Fixed_ಬಿಂದು_Semantics Fixed_ಬಿಂದು_Semantics::getCommonSemantics(
    ಸ್ಥಿರಾಂಕ Fixed_ಬಿಂದು_Semantics &Other) ಸ್ಥಿರಾಂಕ {
  ಪೂರ್ಣಾಂಕ CommonLsb = std::min(get_Lsb_ತೂಕ(), Other.get_Lsb_ತೂಕ());
  ಪೂರ್ಣಾಂಕ CommonMSb = std::max(get_Msb_ತೂಕ() - has_Sign_Or_Padding_ಬಿಟ್(),
                           Other.get_Msb_ತೂಕ() - Other.has_Sign_Or_Padding_ಬಿಟ್());
  ಚಿಹ್ನೆರಹಿತ Common_ಅಗಲ = CommonMSb - CommonLsb + 1;

  ತಾರ್ಕಿಕ ಫಲಿತಾಂಶ_Is_Signed = isSigned() || Other.isSigned();
  ತಾರ್ಕಿಕ ಫಲಿತಾಂಶ_Is_Saturated = isSaturated() || Other.isSaturated();
  ತಾರ್ಕಿಕ ಫಲಿತಾಂಶ_Has_Unsigned_Padding = ಸುಳ್ಳು;
  ಆದರೆ (!ಫಲಿತಾಂಶ_Is_Signed) {
    // Both are unsigned.
    ಫಲಿತಾಂಶ_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ಫಲಿತಾಂಶ_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  ಆದರೆ (ಫಲಿತಾಂಶ_Is_Signed || ಫಲಿತಾಂಶ_Has_Unsigned_Padding)
    Common_ಅಗಲ++;

  ಹಿಂತಿರುಗಿ Fixed_ಬಿಂದು_Semantics(Common_ಅಗಲ, Lsb{CommonLsb}, ಫಲಿತಾಂಶ_Is_Signed,
                             ಫಲಿತಾಂಶ_Is_Saturated, ಫಲಿತಾಂಶ_Has_Unsigned_Padding);
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::ಕೂಡಿಸು(ಸ್ಥಿರಾಂಕ APFixed_ಬಿಂದು &Other,
                               ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ಬಿಂದು ConvertedThis = convert(CommonFXSema);
  APFixed_ಬಿಂದು ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ಮೌಲ್ಯ();
  APSInt OtherVal = ConvertedOther.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ Overflowed = ಸುಳ್ಳು;

  APSInt ಫಲಿತಾಂಶ;
  ಆದರೆ (CommonFXSema.isSaturated()) {
    ಫಲಿತಾಂಶ = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಫಲಿತಾಂಶ = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    *ಅತಿಪ್ರವಾಹ = Overflowed;

  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(ಫಲಿತಾಂಶ, CommonFXSema);
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::sub(ಸ್ಥಿರಾಂಕ APFixed_ಬಿಂದು &Other,
                               ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ಬಿಂದು ConvertedThis = convert(CommonFXSema);
  APFixed_ಬಿಂದು ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ಮೌಲ್ಯ();
  APSInt OtherVal = ConvertedOther.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ Overflowed = ಸುಳ್ಳು;

  APSInt ಫಲಿತಾಂಶ;
  ಆದರೆ (CommonFXSema.isSaturated()) {
    ಫಲಿತಾಂಶ = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಫಲಿತಾಂಶ = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    *ಅತಿಪ್ರವಾಹ = Overflowed;

  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(ಫಲಿತಾಂಶ, CommonFXSema);
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::mul(ಸ್ಥಿರಾಂಕ APFixed_ಬಿಂದು &Other,
                               ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ಬಿಂದು ConvertedThis = convert(CommonFXSema);
  APFixed_ಬಿಂದು ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ಮೌಲ್ಯ();
  APSInt OtherVal = ConvertedOther.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ Overflowed = ಸುಳ್ಳು;

  // Widen the LHS and RHS so we can perform a full multiplication.
  ಚಿಹ್ನೆರಹಿತ Wide = CommonFXSema.get_ಅಗಲ() * 2;
  ಆದರೆ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ಇಲ್ಲದಿದ್ದರೆ {
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
  APSInt ಫಲಿತಾಂಶ;
  ಆದರೆ (CommonFXSema.isSigned())
    ಫಲಿತಾಂಶ = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.get_Lsb_ತೂಕ());
  ಇಲ್ಲದಿದ್ದರೆ
    ಫಲಿತಾಂಶ = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.get_Lsb_ತೂಕ());
  ಪ್ರತಿಜ್ಞಾ(!Overflowed && "Full multiplication cannot overflow!");
  ಫಲಿತಾಂಶ.ಗಣ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ಬಿಂದು::getMax(CommonFXSema).get_ಮೌಲ್ಯ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_ಬಿಂದು::getMin(CommonFXSema).get_ಮೌಲ್ಯ()
                                                 .extOrTrunc(Wide);
  ಆದರೆ (CommonFXSema.isSaturated()) {
    ಆದರೆ (ಫಲಿತಾಂಶ < Min)
      ಫಲಿತಾಂಶ = std::move(Min);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಫಲಿತಾಂಶ > Max)
      ಫಲಿತಾಂಶ = std::move(Max);
  } ಇಲ್ಲದಿದ್ದರೆ {
    Overflowed = ಫಲಿತಾಂಶ < Min || ಫಲಿತಾಂಶ > Max;
  }

  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    *ಅತಿಪ್ರವಾಹ = Overflowed;

  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(ಫಲಿತಾಂಶ.sextOrTrunc(CommonFXSema.get_ಅಗಲ()),
                      CommonFXSema);
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::div(ಸ್ಥಿರಾಂಕ APFixed_ಬಿಂದು &Other,
                               ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ಬಿಂದು ConvertedThis = convert(CommonFXSema);
  APFixed_ಬಿಂದು ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ಮೌಲ್ಯ();
  APSInt OtherVal = ConvertedOther.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ Overflowed = ಸುಳ್ಳು;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //ಅತಿಪ್ರವಾಹ
  ಚಿಹ್ನೆರಹಿತ Wide =
      CommonFXSema.get_ಅಗಲ() * 2 + std::max(-CommonFXSema.get_Msb_ತೂಕ(), 0);
  ಆದರೆ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  ಆದರೆ (CommonFXSema.get_Lsb_ತೂಕ() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.get_Lsb_ತೂಕ());
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CommonFXSema.get_Lsb_ತೂಕ() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.get_Lsb_ತೂಕ());
  APSInt ಫಲಿತಾಂಶ;
  ಆದರೆ (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, ಫಲಿತಾಂಶ, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    ಆದರೆ (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      ಫಲಿತಾಂಶ = ಫಲಿತಾಂಶ - 1;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಫಲಿತಾಂಶ = ThisVal.udiv(OtherVal);
  }
  ಫಲಿತಾಂಶ.ಗಣ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ಬಿಂದು::getMax(CommonFXSema).get_ಮೌಲ್ಯ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_ಬಿಂದು::getMin(CommonFXSema).get_ಮೌಲ್ಯ()
                                                 .extOrTrunc(Wide);
  ಆದರೆ (CommonFXSema.isSaturated()) {
    ಆದರೆ (ಫಲಿತಾಂಶ < Min)
      ಫಲಿತಾಂಶ = std::move(Min);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಫಲಿತಾಂಶ > Max)
      ಫಲಿತಾಂಶ = std::move(Max);
  } ಇಲ್ಲದಿದ್ದರೆ {
    Overflowed = ಫಲಿತಾಂಶ < Min || ಫಲಿತಾಂಶ > Max;
  }

  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    *ಅತಿಪ್ರವಾಹ = Overflowed;

  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(ಫಲಿತಾಂಶ.sextOrTrunc(CommonFXSema.get_ಅಗಲ()),
                      CommonFXSema);
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::shl(ಚಿಹ್ನೆರಹಿತ Amt, ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APSInt ThisVal = Val;
  ತಾರ್ಕಿಕ Overflowed = ಸುಳ್ಳು;

  // Widen the LHS.
  ಚಿಹ್ನೆರಹಿತ Wide = Sema.get_ಅಗಲ() * 2;
  ಆದರೆ (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  ಇಲ್ಲದಿದ್ದರೆ
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_ಬಿಟ್_ಅಗಲ());
  APSInt ಫಲಿತಾಂಶ = ThisVal << Amt;
  ಫಲಿತಾಂಶ.ಗಣ_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ಬಿಂದು::getMax(Sema).get_ಮೌಲ್ಯ().extOrTrunc(Wide);
  APSInt Min = APFixed_ಬಿಂದು::getMin(Sema).get_ಮೌಲ್ಯ().extOrTrunc(Wide);
  ಆದರೆ (Sema.isSaturated()) {
    ಆದರೆ (ಫಲಿತಾಂಶ < Min)
      ಫಲಿತಾಂಶ = std::move(Min);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಫಲಿತಾಂಶ > Max)
      ಫಲಿತಾಂಶ = std::move(Max);
  } ಇಲ್ಲದಿದ್ದರೆ {
    Overflowed = ಫಲಿತಾಂಶ < Min || ಫಲಿತಾಂಶ > Max;
  }

  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    *ಅತಿಪ್ರವಾಹ = Overflowed;

  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(ಫಲಿತಾಂಶ.sextOrTrunc(Sema.get_ಅಗಲ()), Sema);
}

ಶೂನ್ಯ APFixed_ಬಿಂದು::ಗಮ್ಯ_String(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Str) ಸ್ಥಿರಾಂಕ {
  APSInt Val = get_ಮೌಲ್ಯ();
  ಪೂರ್ಣಾಂಕ Lsb = get_Lsb_ತೂಕ();
  ಪೂರ್ಣಾಂಕ Orig_ಅಗಲ = get_ಅಗಲ();

  ಆದರೆ (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_ಬಿಟ್_ಅಗಲ() + Lsb);
    IntPart <<= Lsb;
    IntPart.ಗಮ್ಯ_String(Str, /*Radix=*/10);
    Str.ತಳ್ಳು_ಹಿಂಭಾಗ('.');
    Str.ತಳ್ಳು_ಹಿಂಭಾಗ('0');
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.ಗಣ_Is_Unsigned(ನಿಜ);
    Str.ತಳ್ಳು_ಹಿಂಭಾಗ('-');
  }

  ಪೂರ್ಣಾಂಕ Scale = -get_Lsb_ತೂಕ();
  APSInt IntPart = (Orig_ಅಗಲ > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = std::max(Orig_ಅಗಲ, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(ಅಗಲ);
  APInt Fract_Part_ಮುಖವಾಡ = APInt::getAllOnes(Scale).zext(ಅಗಲ);
  APInt RadixInt = APInt(ಅಗಲ, 10);

  IntPart.ಗಮ್ಯ_String(Str, /*Radix=*/10);
  Str.ತಳ್ಳು_ಹಿಂಭಾಗ('.');
  ಮಾಡು {
    (FractPart * RadixInt)
        .lshr(Scale)
        .ಗಮ್ಯ_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_ಮುಖವಾಡ;
  } ಆಗ (FractPart != 0);
}

ಶೂನ್ಯ APFixed_ಬಿಂದು::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << "APFixedPoint(" << ಗಮ್ಯ_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ APFixed_ಬಿಂದು::dump() ಸ್ಥಿರಾಂಕ { print(llvm::errs()); }
#ಆದರೆ_ಅಂತ್ಯ

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::negate(ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!isSaturated()) {
    ಆದರೆ (ಅತಿಪ್ರವಾಹ)
      *ಅತಿಪ್ರವಾಹ =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_ಮೌಲ್ಯ());
    ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(-Val, Sema);
  }

  // We never overflow for saturation
  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    *ಅತಿಪ್ರವಾಹ = ಸುಳ್ಳು;

  ಆದರೆ (isSigned())
    ಹಿಂತಿರುಗಿ Val.is_Min_Signed_ಮೌಲ್ಯ() ? getMax(Sema) : APFixed_ಬಿಂದು(-Val, Sema);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(Sema);
}

APSInt APFixed_ಬಿಂದು::convert_ಗಮ್ಯ_Int(ಚಿಹ್ನೆರಹಿತ Dst_ಅಗಲ, ತಾರ್ಕಿಕ DstSign,
                                  ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APSInt ಫಲಿತಾಂಶ = getIntPart();
  ಚಿಹ್ನೆರಹಿತ Src_ಅಗಲ = get_ಅಗಲ();

  APSInt DstMin = APSInt::get_Min_ಮೌಲ್ಯ(Dst_ಅಗಲ, !DstSign);
  APSInt DstMax = APSInt::get_Max_ಮೌಲ್ಯ(Dst_ಅಗಲ, !DstSign);

  ಆದರೆ (Src_ಅಗಲ < Dst_ಅಗಲ) {
    ಫಲಿತಾಂಶ = ಫಲಿತಾಂಶ.extend(Dst_ಅಗಲ);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Src_ಅಗಲ > Dst_ಅಗಲ) {
    DstMin = DstMin.extend(Src_ಅಗಲ);
    DstMax = DstMax.extend(Src_ಅಗಲ);
  }

  ಆದರೆ (ಅತಿಪ್ರವಾಹ) {
    ಆದರೆ (ಫಲಿತಾಂಶ.isSigned() && !DstSign) {
      *ಅತಿಪ್ರವಾಹ = ಫಲಿತಾಂಶ.isNegative() || ಫಲಿತಾಂಶ.ugt(DstMax);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಫಲಿತಾಂಶ.isUnsigned() && DstSign) {
      *ಅತಿಪ್ರವಾಹ = ಫಲಿತಾಂಶ.ugt(DstMax);
    } ಇಲ್ಲದಿದ್ದರೆ {
      *ಅತಿಪ್ರವಾಹ = ಫಲಿತಾಂಶ < DstMin || ಫಲಿತಾಂಶ > DstMax;
    }
  }

  ಫಲಿತಾಂಶ.ಗಣ_Is_Signed(DstSign);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ.extOrTrunc(Dst_ಅಗಲ);
}

ಸ್ಥಿರಾಂಕ fltSemantics *APFixed_ಬಿಂದು::promoteFloatSemantics(ಸ್ಥಿರಾಂಕ fltSemantics *S) {
  ಆದರೆ (S == &APFloat::BFloat())
    ಹಿಂತಿರುಗಿ &APFloat::IEEEdouble();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S == &APFloat::IEEEhalf())
    ಹಿಂತಿರುಗಿ &APFloat::IEEEsingle();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S == &APFloat::IEEEsingle())
    ಹಿಂತಿರುಗಿ &APFloat::IEEEdouble();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S == &APFloat::IEEEdouble())
    ಹಿಂತಿರುಗಿ &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixed_ಬಿಂದು::convert_ಗಮ್ಯ_Float(ಸ್ಥಿರಾಂಕ fltSemantics &FloatSema) ಸ್ಥಿರಾಂಕ {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_ಬಹುಲಕ RM = APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even;
  APFloat::rounding_ಬಹುಲಕ LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ಸ್ಥಿರಾಂಕ fltSemantics *OpSema = &FloatSema;
  ಆಗ (!Sema.ಹೊಂದುತ್ತದೆ_ಒಳ_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::op_ಸ್ಥಿತಿ S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (ಶೂನ್ಯ)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat Scale_ಗುಣಕ(std::ಘಾತ(2, Sema.get_Lsb_ತೂಕ()));
  ತಾರ್ಕಿಕ Ignored;
  Scale_ಗುಣಕ.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_ಗುಣಕ, LosslessRM);

  ಆದರೆ (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  ಹಿಂತಿರುಗಿ Flt;
}

APFixed_ಬಿಂದು APFixed_ಬಿಂದು::get_From_Int_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ APSInt &ಮೌಲ್ಯ,
                                           ಸ್ಥಿರಾಂಕ Fixed_ಬಿಂದು_Semantics &DstFXSema,
                                           ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) {
  Fixed_ಬಿಂದು_Semantics IntFXSema = Fixed_ಬಿಂದು_Semantics::GetIntegerSemantics(
      ಮೌಲ್ಯ.get_ಬಿಟ್_ಅಗಲ(), ಮೌಲ್ಯ.isSigned());
  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(ಮೌಲ್ಯ, IntFXSema).convert(DstFXSema, ಅತಿಪ್ರವಾಹ);
}

APFixed_ಬಿಂದು
APFixed_ಬಿಂದು::get_From_Float_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ APFloat &ಮೌಲ್ಯ,
                                ಸ್ಥಿರಾಂಕ Fixed_ಬಿಂದು_Semantics &DstFXSema,
                                ತಾರ್ಕಿಕ *ಅತಿಪ್ರವಾಹ) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_ಬಹುಲಕ RM = APFloat::rmTowardZero;
  APFloat::rounding_ಬಹುಲಕ LosslessRM = APFloat::rmTowardZero;

  ಸ್ಥಿರಾಂಕ fltSemantics &FloatSema = ಮೌಲ್ಯ.getSemantics();

  ಆದರೆ (ಮೌಲ್ಯ.isNaN()) {
    // Handle NaN immediately.
    ಆದರೆ (ಅತಿಪ್ರವಾಹ)
      *ಅತಿಪ್ರವಾಹ = ನಿಜ;
    ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ಸ್ಥಿರಾಂಕ fltSemantics *OpSema = &FloatSema;
  ಆಗ (!DstFXSema.ಹೊಂದುತ್ತದೆ_ಒಳ_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = ಮೌಲ್ಯ;

  ತಾರ್ಕಿಕ Ignored;
  ಆದರೆ (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_ಗುಣಕ(std::ಘಾತ(2, -DstFXSema.get_Lsb_ತೂಕ()));
  Scale_ಗುಣಕ.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_ಗುಣಕ, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_ಅಗಲ(), !DstFXSema.isSigned());
  Val.convert_ಗಮ್ಯ_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_ಗುಣಕ = APFloat(std::ಘಾತ(2, DstFXSema.get_Lsb_ತೂಕ()));
  Scale_ಗುಣಕ.convert(*OpSema, LosslessRM, &Ignored);
  Val.ಸುತ್ತು_ಗಮ್ಯ_Integral(RM);
  Val.multiply(Scale_ಗುಣಕ, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convert_ಗಮ್ಯ_Float(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convert_ಗಮ್ಯ_Float(*OpSema);
  ತಾರ್ಕಿಕ Overflowed = ಸುಳ್ಳು;
  ಆದರೆ (DstFXSema.isSaturated()) {
    ಆದರೆ (Val > FloatMax)
      Res = getMax(DstFXSema).get_ಮೌಲ್ಯ();
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Val < FloatMin)
      Res = getMin(DstFXSema).get_ಮೌಲ್ಯ();
  } ಇಲ್ಲದಿದ್ದರೆ {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    *ಅತಿಪ್ರವಾಹ = Overflowed;

  ಹಿಂತಿರುಗಿ APFixed_ಬಿಂದು(Res, DstFXSema);
}

} // namespace llvm
