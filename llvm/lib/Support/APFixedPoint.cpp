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

#समावेश "llvm/ADT/APFixedPoint.h"
#समावेश "llvm/ADT/APFloat.h"

#समावेश <cmath>

namespace llvm {

व्योम FixedPointSemantics::print(llvm::raw_ostream &OS) स्थिर {
  OS << "width=" << getWidth() << ", ";
  अगर (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() स्थिर {
  वापस llvm::bit_cast<uint32_t>(*यह);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, false, false, false);
  std::स_नकल(&F, &I, माप(F));
  वापस F;
}

APFixedPoint APFixedPoint::convert(स्थिर FixedPointSemantics &DstSema,
                                   bool *Overflow) स्थिर {
  APSInt NewVal = Val;
  पूर्णांक RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  अगर (Overflow)
    *Overflow = false;

  अगर (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  स्वतः Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  अगर (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    अगर (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    अन्यथा अगर (Overflow)
      *Overflow = true;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  अगर (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    अगर (DstSema.isSaturated())
      NewVal = 0;
    अन्यथा अगर (Overflow)
      *Overflow = true;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  वापस APFixedPoint(NewVal, DstSema);
}

पूर्णांक APFixedPoint::compare(स्थिर APFixedPoint &Other) स्थिर {
  APSInt ThisVal = get_मूल्य();
  APSInt OtherVal = Other.get_मूल्य();
  bool ThisSigned = Val.isSigned();
  bool OtherSigned = OtherVal.isSigned();

  पूर्णांक CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णांक CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  अचिन्हित CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  अगर (ThisSigned && OtherSigned) {
    अगर (ThisVal.sgt(OtherVal))
      वापस 1;
    अन्यथा अगर (ThisVal.slt(OtherVal))
      वापस -1;
  } अन्यथा अगर (!ThisSigned && !OtherSigned) {
    अगर (ThisVal.ugt(OtherVal))
      वापस 1;
    अन्यथा अगर (ThisVal.ult(OtherVal))
      वापस -1;
  } अन्यथा अगर (ThisSigned && !OtherSigned) {
    अगर (ThisVal.isSignBitSet())
      वापस -1;
    अन्यथा अगर (ThisVal.ugt(OtherVal))
      वापस 1;
    अन्यथा अगर (ThisVal.ult(OtherVal))
      वापस -1;
  } अन्यथा {
    // !ThisSigned && OtherSigned
    अगर (OtherVal.isSignBitSet())
      वापस 1;
    अन्यथा अगर (ThisVal.ugt(OtherVal))
      वापस 1;
    अन्यथा अगर (ThisVal.ult(OtherVal))
      वापस -1;
  }

  वापस 0;
}

APFixedPoint APFixedPoint::getMax(स्थिर FixedPointSemantics &Sema) {
  bool IsUnsigned = !Sema.isSigned();
  स्वतः Val = APSInt::get_Max_मूल्य(Sema.getWidth(), IsUnsigned);
  अगर (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  वापस APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(स्थिर FixedPointSemantics &Sema) {
  स्वतः Val = APSInt::get_Min_मूल्य(Sema.getWidth(), !Sema.isSigned());
  वापस APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(स्थिर FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  वापस APFixedPoint(Val, Sema);
}

bool FixedPointSemantics::fitsInFloatSemantics(
    स्थिर fltSemantics &FloatSema) स्थिर {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*यह).get_मूल्य();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  अगर ((Status & APFloat::opOverflow) || !isSigned())
    वापस !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*यह).get_मूल्य();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  वापस !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    स्थिर FixedPointSemantics &Other) स्थिर {
  पूर्णांक CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णांक CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  अचिन्हित CommonWidth = CommonMSb - CommonLsb + 1;

  bool परिणाम_Is_Signed = isSigned() || Other.isSigned();
  bool परिणाम_Is_Saturated = isSaturated() || Other.isSaturated();
  bool परिणाम_Has_Unsigned_Padding = false;
  अगर (!परिणाम_Is_Signed) {
    // Both are unsigned.
    परिणाम_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !परिणाम_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  अगर (परिणाम_Is_Signed || परिणाम_Has_Unsigned_Padding)
    CommonWidth++;

  वापस FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, परिणाम_Is_Signed,
                             परिणाम_Is_Saturated, परिणाम_Has_Unsigned_Padding);
}

APFixedPoint APFixedPoint::add(स्थिर APFixedPoint &Other,
                               bool *Overflow) स्थिर {
  स्वतः CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  bool Overflowed = false;

  APSInt परिणाम;
  अगर (CommonFXSema.isSaturated()) {
    परिणाम = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } अन्यथा {
    परिणाम = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  अगर (Overflow)
    *Overflow = Overflowed;

  वापस APFixedPoint(परिणाम, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(स्थिर APFixedPoint &Other,
                               bool *Overflow) स्थिर {
  स्वतः CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  bool Overflowed = false;

  APSInt परिणाम;
  अगर (CommonFXSema.isSaturated()) {
    परिणाम = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } अन्यथा {
    परिणाम = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  अगर (Overflow)
    *Overflow = Overflowed;

  वापस APFixedPoint(परिणाम, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(स्थिर APFixedPoint &Other,
                               bool *Overflow) स्थिर {
  स्वतः CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  bool Overflowed = false;

  // Widen the LHS and RHS so we can perform a full multiplication.
  अचिन्हित Wide = CommonFXSema.getWidth() * 2;
  अगर (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } अन्यथा {
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
  APSInt परिणाम;
  अगर (CommonFXSema.isSigned())
    परिणाम = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  अन्यथा
    परिणाम = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  निश्चित(!Overflowed && "Full multiplication cannot overflow!");
  परिणाम.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_मूल्य()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_मूल्य()
                                                 .extOrTrunc(Wide);
  अगर (CommonFXSema.isSaturated()) {
    अगर (परिणाम < Min)
      परिणाम = std::move(Min);
    अन्यथा अगर (परिणाम > Max)
      परिणाम = std::move(Max);
  } अन्यथा {
    Overflowed = परिणाम < Min || परिणाम > Max;
  }

  अगर (Overflow)
    *Overflow = Overflowed;

  वापस APFixedPoint(परिणाम.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::भाग(स्थिर APFixedPoint &Other,
                               bool *Overflow) स्थिर {
  स्वतः CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  bool Overflowed = false;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  अचिन्हित Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  अगर (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } अन्यथा {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  अगर (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  अन्यथा अगर (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt परिणाम;
  अगर (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, परिणाम, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    अगर (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      परिणाम = परिणाम - 1;
  } अन्यथा {
    परिणाम = ThisVal.udiv(OtherVal);
  }
  परिणाम.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_मूल्य()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_मूल्य()
                                                 .extOrTrunc(Wide);
  अगर (CommonFXSema.isSaturated()) {
    अगर (परिणाम < Min)
      परिणाम = std::move(Min);
    अन्यथा अगर (परिणाम > Max)
      परिणाम = std::move(Max);
  } अन्यथा {
    Overflowed = परिणाम < Min || परिणाम > Max;
  }

  अगर (Overflow)
    *Overflow = Overflowed;

  वापस APFixedPoint(परिणाम.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(अचिन्हित Amt, bool *Overflow) स्थिर {
  APSInt ThisVal = Val;
  bool Overflowed = false;

  // Widen the LHS.
  अचिन्हित Wide = Sema.getWidth() * 2;
  अगर (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  अन्यथा
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt परिणाम = ThisVal << Amt;
  परिणाम.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_मूल्य().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_मूल्य().extOrTrunc(Wide);
  अगर (Sema.isSaturated()) {
    अगर (परिणाम < Min)
      परिणाम = std::move(Min);
    अन्यथा अगर (परिणाम > Max)
      परिणाम = std::move(Max);
  } अन्यथा {
    Overflowed = परिणाम < Min || परिणाम > Max;
  }

  अगर (Overflow)
    *Overflow = Overflowed;

  वापस APFixedPoint(परिणाम.sextOrTrunc(Sema.getWidth()), Sema);
}

व्योम APFixedPoint::toString(SmallVectorImpl<अक्षर> &Str) स्थिर {
  APSInt Val = get_मूल्य();
  पूर्णांक Lsb = getLsbWeight();
  पूर्णांक OrigWidth = getWidth();

  अगर (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.धकेलो_back('.');
    Str.धकेलो_back('0');
    वापस;
  }

  अगर (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(true);
    Str.धकेलो_back('-');
  }

  पूर्णांक Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::लो(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  अचिन्हित Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.धकेलो_back('.');
  करो {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } जबतक (FractPart != 0);
}

व्योम APFixedPoint::print(raw_ostream &OS) स्थिर {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम APFixedPoint::dump() स्थिर { print(llvm::errs()); }
#पूर्ण_अगर

APFixedPoint APFixedPoint::negate(bool *Overflow) स्थिर {
  अगर (!isSaturated()) {
    अगर (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_मूल्य());
    वापस APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  अगर (Overflow)
    *Overflow = false;

  अगर (isSigned())
    वापस Val.is_Min_Signed_मूल्य() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  अन्यथा
    वापस APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(अचिन्हित DstWidth, bool DstSign,
                                  bool *Overflow) स्थिर {
  APSInt परिणाम = getIntPart();
  अचिन्हित SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_मूल्य(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_मूल्य(DstWidth, !DstSign);

  अगर (SrcWidth < DstWidth) {
    परिणाम = परिणाम.extend(DstWidth);
  } अन्यथा अगर (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  अगर (Overflow) {
    अगर (परिणाम.isSigned() && !DstSign) {
      *Overflow = परिणाम.isNegative() || परिणाम.ugt(DstMax);
    } अन्यथा अगर (परिणाम.isUnsigned() && DstSign) {
      *Overflow = परिणाम.ugt(DstMax);
    } अन्यथा {
      *Overflow = परिणाम < DstMin || परिणाम > DstMax;
    }
  }

  परिणाम.setIsSigned(DstSign);
  वापस परिणाम.extOrTrunc(DstWidth);
}

स्थिर fltSemantics *APFixedPoint::promoteFloatSemantics(स्थिर fltSemantics *S) {
  अगर (S == &APFloat::BFloat())
    वापस &APFloat::IEEEdouble();
  अन्यथा अगर (S == &APFloat::IEEEhalf())
    वापस &APFloat::IEEEsingle();
  अन्यथा अगर (S == &APFloat::IEEEsingle())
    वापस &APFloat::IEEEdouble();
  अन्यथा अगर (S == &APFloat::IEEEdouble())
    वापस &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(स्थिर fltSemantics &FloatSema) स्थिर {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  स्थिर fltSemantics *OpSema = &FloatSema;
  जबतक (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (व्योम)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::घात(2, Sema.getLsbWeight()));
  bool Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  अगर (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  वापस Flt;
}

APFixedPoint APFixedPoint::get_From_Int_मूल्य(स्थिर APSInt &मूल्य,
                                           स्थिर FixedPointSemantics &DstFXSema,
                                           bool *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      मूल्य.getBitWidth(), मूल्य.isSigned());
  वापस APFixedPoint(मूल्य, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_मूल्य(स्थिर APFloat &मूल्य,
                                स्थिर FixedPointSemantics &DstFXSema,
                                bool *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  स्थिर fltSemantics &FloatSema = मूल्य.getSemantics();

  अगर (मूल्य.isNaN()) {
    // Handle NaN immediately.
    अगर (Overflow)
      *Overflow = true;
    वापस APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  स्थिर fltSemantics *OpSema = &FloatSema;
  जबतक (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = मूल्य;

  bool Ignored;
  अगर (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat ScaleFactor(std::घात(2, -DstFXSema.getLsbWeight()));
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(ScaleFactor, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.getWidth(), !DstFXSema.isSigned());
  Val.convertToInteger(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  ScaleFactor = APFloat(std::घात(2, DstFXSema.getLsbWeight()));
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Val.roundToIntegral(RM);
  Val.multiply(ScaleFactor, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convertToFloat(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convertToFloat(*OpSema);
  bool Overflowed = false;
  अगर (DstFXSema.isSaturated()) {
    अगर (Val > FloatMax)
      Res = getMax(DstFXSema).get_मूल्य();
    अन्यथा अगर (Val < FloatMin)
      Res = getMin(DstFXSema).get_मूल्य();
  } अन्यथा {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  अगर (Overflow)
    *Overflow = Overflowed;

  वापस APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
