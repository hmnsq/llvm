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

#અન્તર્ભાવ "llvm/ADT/APFixedPoint.h"
#અન્તર્ભાવ "llvm/ADT/APFloat.h"

#અન્તર્ભાવ <cmath>

નામાકાશ llvm {

શૂન્ય FixedPointSemantics::print(llvm::raw_ostream &OS) અચલ {
  OS << "width=" << getWidth() << ", ";
  જો (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() અચલ {
  પાછા llvm::bit_cast<uint32_t>(*આ);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, ખોટું, ખોટું, ખોટું);
  std::memcpy(&F, &I, કદ(F));
  પાછા F;
}

APFixedPoint APFixedPoint::convert(અચલ FixedPointSemantics &DstSema,
                                   તાર્કિક *Overflow) અચલ {
  APSInt NewVal = Val;
  પૂર્ણાંક RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  જો (Overflow)
    *Overflow = ખોટું;

  જો (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  જો (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    જો (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    નહીંતર જો (Overflow)
      *Overflow = સાચું;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  જો (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    જો (DstSema.isSaturated())
      NewVal = 0;
    નહીંતર જો (Overflow)
      *Overflow = સાચું;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  પાછા APFixedPoint(NewVal, DstSema);
}

પૂર્ણાંક APFixedPoint::compare(અચલ APFixedPoint &Other) અચલ {
  APSInt ThisVal = get_કિંમત();
  APSInt OtherVal = Other.get_કિંમત();
  તાર્કિક ThisSigned = Val.isSigned();
  તાર્કિક OtherSigned = OtherVal.isSigned();

  પૂર્ણાંક CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  પૂર્ણાંક CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  જો (ThisSigned && OtherSigned) {
    જો (ThisVal.sgt(OtherVal))
      પાછા 1;
    નહીંતર જો (ThisVal.slt(OtherVal))
      પાછા -1;
  } નહીંતર જો (!ThisSigned && !OtherSigned) {
    જો (ThisVal.ugt(OtherVal))
      પાછા 1;
    નહીંતર જો (ThisVal.ult(OtherVal))
      પાછા -1;
  } નહીંતર જો (ThisSigned && !OtherSigned) {
    જો (ThisVal.isSignBitSet())
      પાછા -1;
    નહીંતર જો (ThisVal.ugt(OtherVal))
      પાછા 1;
    નહીંતર જો (ThisVal.ult(OtherVal))
      પાછા -1;
  } નહીંતર {
    // !ThisSigned && OtherSigned
    જો (OtherVal.isSignBitSet())
      પાછા 1;
    નહીંતર જો (ThisVal.ugt(OtherVal))
      પાછા 1;
    નહીંતર જો (ThisVal.ult(OtherVal))
      પાછા -1;
  }

  પાછા 0;
}

APFixedPoint APFixedPoint::getMax(અચલ FixedPointSemantics &Sema) {
  તાર્કિક IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_કિંમત(Sema.getWidth(), IsUnsigned);
  જો (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  પાછા APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(અચલ FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_કિંમત(Sema.getWidth(), !Sema.isSigned());
  પાછા APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(અચલ FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  પાછા APFixedPoint(Val, Sema);
}

તાર્કિક FixedPointSemantics::fitsInFloatSemantics(
    અચલ fltSemantics &FloatSema) અચલ {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*આ).get_કિંમત();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  જો ((Status & APFloat::opOverflow) || !isSigned())
    પાછા !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*આ).get_કિંમત();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  પાછા !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    અચલ FixedPointSemantics &Other) અચલ {
  પૂર્ણાંક CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  પૂર્ણાંક CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  તાર્કિક ResultIsSigned = isSigned() || Other.isSigned();
  તાર્કિક ResultIsSaturated = isSaturated() || Other.isSaturated();
  તાર્કિક ResultHasUnsignedPadding = ખોટું;
  જો (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  જો (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  પાછા FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(અચલ APFixedPoint &Other,
                               તાર્કિક *Overflow) અચલ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_કિંમત();
  APSInt OtherVal = ConvertedOther.get_કિંમત();
  તાર્કિક Overflowed = ખોટું;

  APSInt Result;
  જો (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } નહીંતર {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  જો (Overflow)
    *Overflow = Overflowed;

  પાછા APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(અચલ APFixedPoint &Other,
                               તાર્કિક *Overflow) અચલ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_કિંમત();
  APSInt OtherVal = ConvertedOther.get_કિંમત();
  તાર્કિક Overflowed = ખોટું;

  APSInt Result;
  જો (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } નહીંતર {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  જો (Overflow)
    *Overflow = Overflowed;

  પાછા APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(અચલ APFixedPoint &Other,
                               તાર્કિક *Overflow) અચલ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_કિંમત();
  APSInt OtherVal = ConvertedOther.get_કિંમત();
  તાર્કિક Overflowed = ખોટું;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  જો (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } નહીંતર {
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
  APSInt Result;
  જો (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  નહીંતર
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  પ્રતિજ્ઞા(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_કિંમત()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_કિંમત()
                                                 .extOrTrunc(Wide);
  જો (CommonFXSema.isSaturated()) {
    જો (Result < Min)
      Result = std::move(Min);
    નહીંતર જો (Result > Max)
      Result = std::move(Max);
  } નહીંતર {
    Overflowed = Result < Min || Result > Max;
  }

  જો (Overflow)
    *Overflow = Overflowed;

  પાછા APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(અચલ APFixedPoint &Other,
                               તાર્કિક *Overflow) અચલ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_કિંમત();
  APSInt OtherVal = ConvertedOther.get_કિંમત();
  તાર્કિક Overflowed = ખોટું;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  જો (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } નહીંતર {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  જો (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  નહીંતર જો (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  જો (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    જો (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } નહીંતર {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_કિંમત()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_કિંમત()
                                                 .extOrTrunc(Wide);
  જો (CommonFXSema.isSaturated()) {
    જો (Result < Min)
      Result = std::move(Min);
    નહીંતર જો (Result > Max)
      Result = std::move(Max);
  } નહીંતર {
    Overflowed = Result < Min || Result > Max;
  }

  જો (Overflow)
    *Overflow = Overflowed;

  પાછા APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, તાર્કિક *Overflow) અચલ {
  APSInt ThisVal = Val;
  તાર્કિક Overflowed = ખોટું;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  જો (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  નહીંતર
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_કિંમત().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_કિંમત().extOrTrunc(Wide);
  જો (Sema.isSaturated()) {
    જો (Result < Min)
      Result = std::move(Min);
    નહીંતર જો (Result > Max)
      Result = std::move(Max);
  } નહીંતર {
    Overflowed = Result < Min || Result > Max;
  }

  જો (Overflow)
    *Overflow = Overflowed;

  પાછા APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

શૂન્ય APFixedPoint::toString(SmallVectorImpl<અક્ષર> &Str) અચલ {
  APSInt Val = get_કિંમત();
  પૂર્ણાંક Lsb = getLsbWeight();
  પૂર્ણાંક OrigWidth = getWidth();

  જો (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.ધકેલો_back('.');
    Str.ધકેલો_back('0');
    પાછા;
  }

  જો (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(સાચું);
    Str.ધકેલો_back('-');
  }

  પૂર્ણાંક Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.ધકેલો_back('.');
  કરો {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } જ્યારે (FractPart != 0);
}

શૂન્ય APFixedPoint::print(raw_ostream &OS) અચલ {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય APFixedPoint::dump() અચલ { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(તાર્કિક *Overflow) અચલ {
  જો (!isSaturated()) {
    જો (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_કિંમત());
    પાછા APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  જો (Overflow)
    *Overflow = ખોટું;

  જો (isSigned())
    પાછા Val.is_Min_Signed_કિંમત() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  નહીંતર
    પાછા APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, તાર્કિક DstSign,
                                  તાર્કિક *Overflow) અચલ {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_કિંમત(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_કિંમત(DstWidth, !DstSign);

  જો (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } નહીંતર જો (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  જો (Overflow) {
    જો (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } નહીંતર જો (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } નહીંતર {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  પાછા Result.extOrTrunc(DstWidth);
}

અચલ fltSemantics *APFixedPoint::promoteFloatSemantics(અચલ fltSemantics *S) {
  જો (S == &APFloat::BFloat())
    પાછા &APFloat::IEEEdouble();
  નહીંતર જો (S == &APFloat::IEEEhalf())
    પાછા &APFloat::IEEEsingle();
  નહીંતર જો (S == &APFloat::IEEEsingle())
    પાછા &APFloat::IEEEdouble();
  નહીંતર જો (S == &APFloat::IEEEdouble())
    પાછા &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(અચલ fltSemantics &FloatSema) અચલ {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  અચલ fltSemantics *OpSema = &FloatSema;
  જ્યારે (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (શૂન્ય)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  તાર્કિક Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  જો (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  પાછા Flt;
}

APFixedPoint APFixedPoint::get_From_Int_કિંમત(અચલ APSInt &કિંમત,
                                           અચલ FixedPointSemantics &DstFXSema,
                                           તાર્કિક *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      કિંમત.getBitWidth(), કિંમત.isSigned());
  પાછા APFixedPoint(કિંમત, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_કિંમત(અચલ APFloat &કિંમત,
                                અચલ FixedPointSemantics &DstFXSema,
                                તાર્કિક *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  અચલ fltSemantics &FloatSema = કિંમત.getSemantics();

  જો (કિંમત.isNaN()) {
    // Handle NaN immediately.
    જો (Overflow)
      *Overflow = સાચું;
    પાછા APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  અચલ fltSemantics *OpSema = &FloatSema;
  જ્યારે (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = કિંમત;

  તાર્કિક Ignored;
  જો (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat ScaleFactor(std::pow(2, -DstFXSema.getLsbWeight()));
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
  ScaleFactor = APFloat(std::pow(2, DstFXSema.getLsbWeight()));
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Val.roundToIntegral(RM);
  Val.multiply(ScaleFactor, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convertToFloat(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convertToFloat(*OpSema);
  તાર્કિક Overflowed = ખોટું;
  જો (DstFXSema.isSaturated()) {
    જો (Val > FloatMax)
      Res = getMax(DstFXSema).get_કિંમત();
    નહીંતર જો (Val < FloatMin)
      Res = getMin(DstFXSema).get_કિંમત();
  } નહીંતર {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  જો (Overflow)
    *Overflow = Overflowed;

  પાછા APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
