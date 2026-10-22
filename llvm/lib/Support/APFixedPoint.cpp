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

#అన్తర్భావ "llvm/ADT/APFixedPoint.h"
#అన్తర్భావ "llvm/ADT/APFloat.h"

#అన్తర్భావ <cmath>

నామాకాశ llvm {

శూన్య Fixed_బిందువు_Semantics::print(llvm::raw_ostream &OS) స్థిరరాశి {
  OS << "width=" << get_వెడల్పు() << ", ";
  ఒకవేళ (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << get_Msb_బరువు() << ", ";
  OS << "lsb=" << get_Lsb_బరువు() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t Fixed_బిందువు_Semantics::గమ్యం_Opaque_Int() స్థిరరాశి {
  తిరిగి llvm::బిట్టు_cast<uint32_t>(*ఇది);
}

Fixed_బిందువు_Semantics Fixed_బిందువు_Semantics::getFromOpaqueInt(uint32_t I) {
  Fixed_బిందువు_Semantics F(0, 0, అబద్ధం, అబద్ధం, అబద్ధం);
  std::జ్ఞాపకం_నకలు(&F, &I, పరిమాణం(F));
  తిరిగి F;
}

APFixed_బిందువు APFixed_బిందువు::convert(స్థిరరాశి Fixed_బిందువు_Semantics &DstSema,
                                   తార్కిక *అధికప్రవాహం) స్థిరరాశి {
  APSInt NewVal = Val;
  పూర్ణాంకం RelativeUpscale = get_Lsb_బరువు() - DstSema.get_Lsb_బరువు();
  ఒకవేళ (అధికప్రవాహం)
    *అధికప్రవాహం = అబద్ధం;

  ఒకవేళ (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_బిట్టు_వెడల్పు() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  స్వయంచాలక ముసుగు = APInt::get_బిట్లు_సమితి_From(
      NewVal.get_బిట్టు_వెడల్పు(),
      std::min(DstSema.get_Integral_బిట్లు() - DstSema.get_Lsb_బరువు(),
               NewVal.get_బిట్టు_వెడల్పు()));
  APInt Masked(NewVal & ముసుగు);

  // Change in the bits above the sign
  ఒకవేళ (!(Masked == ముసుగు || Masked == 0)) {
    // Found overflow in the bits above the sign
    ఒకవేళ (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? ముసుగు : ~ముసుగు;
    లేకపోతే ఒకవేళ (అధికప్రవాహం)
      *అధికప్రవాహం = నిజం;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  ఒకవేళ (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    ఒకవేళ (DstSema.isSaturated())
      NewVal = 0;
    లేకపోతే ఒకవేళ (అధికప్రవాహం)
      *అధికప్రవాహం = నిజం;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_వెడల్పు());
  NewVal.సమితి_Is_Signed(DstSema.isSigned());
  తిరిగి APFixed_బిందువు(NewVal, DstSema);
}

పూర్ణాంకం APFixed_బిందువు::compare(స్థిరరాశి APFixed_బిందువు &Other) స్థిరరాశి {
  APSInt ThisVal = get_విలువ();
  APSInt OtherVal = Other.get_విలువ();
  తార్కిక ThisSigned = Val.isSigned();
  తార్కిక OtherSigned = OtherVal.isSigned();

  పూర్ణాంకం CommonLsb = std::min(get_Lsb_బరువు(), Other.get_Lsb_బరువు());
  పూర్ణాంకం CommonMsb = std::max(get_Msb_బరువు(), Other.get_Msb_బరువు());
  గుర్తులేని Common_వెడల్పు = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_వెడల్పు);
  OtherVal = OtherVal.extOrTrunc(Common_వెడల్పు);

  ThisVal = ThisVal.shl(get_Lsb_బరువు() - CommonLsb);
  OtherVal = OtherVal.shl(Other.get_Lsb_బరువు() - CommonLsb);

  ఒకవేళ (ThisSigned && OtherSigned) {
    ఒకవేళ (ThisVal.sgt(OtherVal))
      తిరిగి 1;
    లేకపోతే ఒకవేళ (ThisVal.slt(OtherVal))
      తిరిగి -1;
  } లేకపోతే ఒకవేళ (!ThisSigned && !OtherSigned) {
    ఒకవేళ (ThisVal.ugt(OtherVal))
      తిరిగి 1;
    లేకపోతే ఒకవేళ (ThisVal.ult(OtherVal))
      తిరిగి -1;
  } లేకపోతే ఒకవేళ (ThisSigned && !OtherSigned) {
    ఒకవేళ (ThisVal.is_Sign_బిట్టు_సమితి())
      తిరిగి -1;
    లేకపోతే ఒకవేళ (ThisVal.ugt(OtherVal))
      తిరిగి 1;
    లేకపోతే ఒకవేళ (ThisVal.ult(OtherVal))
      తిరిగి -1;
  } లేకపోతే {
    // !ThisSigned && OtherSigned
    ఒకవేళ (OtherVal.is_Sign_బిట్టు_సమితి())
      తిరిగి 1;
    లేకపోతే ఒకవేళ (ThisVal.ugt(OtherVal))
      తిరిగి 1;
    లేకపోతే ఒకవేళ (ThisVal.ult(OtherVal))
      తిరిగి -1;
  }

  తిరిగి 0;
}

APFixed_బిందువు APFixed_బిందువు::getMax(స్థిరరాశి Fixed_బిందువు_Semantics &Sema) {
  తార్కిక IsUnsigned = !Sema.isSigned();
  స్వయంచాలక Val = APSInt::get_Max_విలువ(Sema.get_వెడల్పు(), IsUnsigned);
  ఒకవేళ (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  తిరిగి APFixed_బిందువు(Val, Sema);
}

APFixed_బిందువు APFixed_బిందువు::getMin(స్థిరరాశి Fixed_బిందువు_Semantics &Sema) {
  స్వయంచాలక Val = APSInt::get_Min_విలువ(Sema.get_వెడల్పు(), !Sema.isSigned());
  తిరిగి APFixed_బిందువు(Val, Sema);
}

APFixed_బిందువు APFixed_బిందువు::getEpsilon(స్థిరరాశి Fixed_బిందువు_Semantics &Sema) {
  APSInt Val(Sema.get_వెడల్పు(), !Sema.isSigned());
  Val.సమితి_బిట్టు(/*BitPosition=*/0);
  తిరిగి APFixed_బిందువు(Val, Sema);
}

తార్కిక Fixed_బిందువు_Semantics::సరిపోతుంది_లోపల_Float_Semantics(
    స్థిరరాశి fltSemantics &FloatSema) స్థిరరాశి {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixed_బిందువు::getMax(*ఇది).get_విలువ();
  APFloat F(FloatSema);
  APFloat::op_స్థితి స్థితి_2 = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_సమీప_Ties_గమ్యం_Away);
  ఒకవేళ ((స్థితి_2 & APFloat::op_అధికప్రవాహం) || !isSigned())
    తిరిగి !(స్థితి_2 & APFloat::op_అధికప్రవాహం);

  APSInt MinInt = APFixed_బిందువు::getMin(*ఇది).get_విలువ();
  స్థితి_2 = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_సమీప_Ties_గమ్యం_Away);
  తిరిగి !(స్థితి_2 & APFloat::op_అధికప్రవాహం);
}

Fixed_బిందువు_Semantics Fixed_బిందువు_Semantics::getCommonSemantics(
    స్థిరరాశి Fixed_బిందువు_Semantics &Other) స్థిరరాశి {
  పూర్ణాంకం CommonLsb = std::min(get_Lsb_బరువు(), Other.get_Lsb_బరువు());
  పూర్ణాంకం CommonMSb = std::max(get_Msb_బరువు() - has_Sign_Or_Padding_బిట్టు(),
                           Other.get_Msb_బరువు() - Other.has_Sign_Or_Padding_బిట్టు());
  గుర్తులేని Common_వెడల్పు = CommonMSb - CommonLsb + 1;

  తార్కిక ఫలితం_Is_Signed = isSigned() || Other.isSigned();
  తార్కిక ఫలితం_Is_Saturated = isSaturated() || Other.isSaturated();
  తార్కిక ఫలితం_Has_Unsigned_Padding = అబద్ధం;
  ఒకవేళ (!ఫలితం_Is_Signed) {
    // Both are unsigned.
    ఫలితం_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ఫలితం_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  ఒకవేళ (ఫలితం_Is_Signed || ఫలితం_Has_Unsigned_Padding)
    Common_వెడల్పు++;

  తిరిగి Fixed_బిందువు_Semantics(Common_వెడల్పు, Lsb{CommonLsb}, ఫలితం_Is_Signed,
                             ఫలితం_Is_Saturated, ఫలితం_Has_Unsigned_Padding);
}

APFixed_బిందువు APFixed_బిందువు::కూడు(స్థిరరాశి APFixed_బిందువు &Other,
                               తార్కిక *అధికప్రవాహం) స్థిరరాశి {
  స్వయంచాలక CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_బిందువు ConvertedThis = convert(CommonFXSema);
  APFixed_బిందువు ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_విలువ();
  APSInt OtherVal = ConvertedOther.get_విలువ();
  తార్కిక Overflowed = అబద్ధం;

  APSInt ఫలితం;
  ఒకవేళ (CommonFXSema.isSaturated()) {
    ఫలితం = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } లేకపోతే {
    ఫలితం = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  ఒకవేళ (అధికప్రవాహం)
    *అధికప్రవాహం = Overflowed;

  తిరిగి APFixed_బిందువు(ఫలితం, CommonFXSema);
}

APFixed_బిందువు APFixed_బిందువు::sub(స్థిరరాశి APFixed_బిందువు &Other,
                               తార్కిక *అధికప్రవాహం) స్థిరరాశి {
  స్వయంచాలక CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_బిందువు ConvertedThis = convert(CommonFXSema);
  APFixed_బిందువు ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_విలువ();
  APSInt OtherVal = ConvertedOther.get_విలువ();
  తార్కిక Overflowed = అబద్ధం;

  APSInt ఫలితం;
  ఒకవేళ (CommonFXSema.isSaturated()) {
    ఫలితం = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } లేకపోతే {
    ఫలితం = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  ఒకవేళ (అధికప్రవాహం)
    *అధికప్రవాహం = Overflowed;

  తిరిగి APFixed_బిందువు(ఫలితం, CommonFXSema);
}

APFixed_బిందువు APFixed_బిందువు::mul(స్థిరరాశి APFixed_బిందువు &Other,
                               తార్కిక *అధికప్రవాహం) స్థిరరాశి {
  స్వయంచాలక CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_బిందువు ConvertedThis = convert(CommonFXSema);
  APFixed_బిందువు ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_విలువ();
  APSInt OtherVal = ConvertedOther.get_విలువ();
  తార్కిక Overflowed = అబద్ధం;

  // Widen the LHS and RHS so we can perform a full multiplication.
  గుర్తులేని Wide = CommonFXSema.get_వెడల్పు() * 2;
  ఒకవేళ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } లేకపోతే {
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
  APSInt ఫలితం;
  ఒకవేళ (CommonFXSema.isSigned())
    ఫలితం = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.get_Lsb_బరువు());
  లేకపోతే
    ఫలితం = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.get_Lsb_బరువు());
  ప్రతిజ్ఞా(!Overflowed && "Full multiplication cannot overflow!");
  ఫలితం.సమితి_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_బిందువు::getMax(CommonFXSema).get_విలువ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_బిందువు::getMin(CommonFXSema).get_విలువ()
                                                 .extOrTrunc(Wide);
  ఒకవేళ (CommonFXSema.isSaturated()) {
    ఒకవేళ (ఫలితం < Min)
      ఫలితం = std::move(Min);
    లేకపోతే ఒకవేళ (ఫలితం > Max)
      ఫలితం = std::move(Max);
  } లేకపోతే {
    Overflowed = ఫలితం < Min || ఫలితం > Max;
  }

  ఒకవేళ (అధికప్రవాహం)
    *అధికప్రవాహం = Overflowed;

  తిరిగి APFixed_బిందువు(ఫలితం.sextOrTrunc(CommonFXSema.get_వెడల్పు()),
                      CommonFXSema);
}

APFixed_బిందువు APFixed_బిందువు::div(స్థిరరాశి APFixed_బిందువు &Other,
                               తార్కిక *అధికప్రవాహం) స్థిరరాశి {
  స్వయంచాలక CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_బిందువు ConvertedThis = convert(CommonFXSema);
  APFixed_బిందువు ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_విలువ();
  APSInt OtherVal = ConvertedOther.get_విలువ();
  తార్కిక Overflowed = అబద్ధం;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //అధికప్రవాహం
  గుర్తులేని Wide =
      CommonFXSema.get_వెడల్పు() * 2 + std::max(-CommonFXSema.get_Msb_బరువు(), 0);
  ఒకవేళ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } లేకపోతే {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  ఒకవేళ (CommonFXSema.get_Lsb_బరువు() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.get_Lsb_బరువు());
  లేకపోతే ఒకవేళ (CommonFXSema.get_Lsb_బరువు() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.get_Lsb_బరువు());
  APSInt ఫలితం;
  ఒకవేళ (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, ఫలితం, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    ఒకవేళ (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      ఫలితం = ఫలితం - 1;
  } లేకపోతే {
    ఫలితం = ThisVal.udiv(OtherVal);
  }
  ఫలితం.సమితి_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_బిందువు::getMax(CommonFXSema).get_విలువ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_బిందువు::getMin(CommonFXSema).get_విలువ()
                                                 .extOrTrunc(Wide);
  ఒకవేళ (CommonFXSema.isSaturated()) {
    ఒకవేళ (ఫలితం < Min)
      ఫలితం = std::move(Min);
    లేకపోతే ఒకవేళ (ఫలితం > Max)
      ఫలితం = std::move(Max);
  } లేకపోతే {
    Overflowed = ఫలితం < Min || ఫలితం > Max;
  }

  ఒకవేళ (అధికప్రవాహం)
    *అధికప్రవాహం = Overflowed;

  తిరిగి APFixed_బిందువు(ఫలితం.sextOrTrunc(CommonFXSema.get_వెడల్పు()),
                      CommonFXSema);
}

APFixed_బిందువు APFixed_బిందువు::shl(గుర్తులేని Amt, తార్కిక *అధికప్రవాహం) స్థిరరాశి {
  APSInt ThisVal = Val;
  తార్కిక Overflowed = అబద్ధం;

  // Widen the LHS.
  గుర్తులేని Wide = Sema.get_వెడల్పు() * 2;
  ఒకవేళ (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  లేకపోతే
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_బిట్టు_వెడల్పు());
  APSInt ఫలితం = ThisVal << Amt;
  ఫలితం.సమితి_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_బిందువు::getMax(Sema).get_విలువ().extOrTrunc(Wide);
  APSInt Min = APFixed_బిందువు::getMin(Sema).get_విలువ().extOrTrunc(Wide);
  ఒకవేళ (Sema.isSaturated()) {
    ఒకవేళ (ఫలితం < Min)
      ఫలితం = std::move(Min);
    లేకపోతే ఒకవేళ (ఫలితం > Max)
      ఫలితం = std::move(Max);
  } లేకపోతే {
    Overflowed = ఫలితం < Min || ఫలితం > Max;
  }

  ఒకవేళ (అధికప్రవాహం)
    *అధికప్రవాహం = Overflowed;

  తిరిగి APFixed_బిందువు(ఫలితం.sextOrTrunc(Sema.get_వెడల్పు()), Sema);
}

శూన్య APFixed_బిందువు::గమ్యం_String(Small_సదిశ_Impl<అక్షరం> &Str) స్థిరరాశి {
  APSInt Val = get_విలువ();
  పూర్ణాంకం Lsb = get_Lsb_బరువు();
  పూర్ణాంకం Orig_వెడల్పు = get_వెడల్పు();

  ఒకవేళ (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_బిట్టు_వెడల్పు() + Lsb);
    IntPart <<= Lsb;
    IntPart.గమ్యం_String(Str, /*Radix=*/10);
    Str.నెట్టు_వెనుక('.');
    Str.నెట్టు_వెనుక('0');
    తిరిగి;
  }

  ఒకవేళ (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.సమితి_Is_Unsigned(నిజం);
    Str.నెట్టు_వెనుక('-');
  }

  పూర్ణాంకం Scale = -get_Lsb_బరువు();
  APSInt IntPart = (Orig_వెడల్పు > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  గుర్తులేని వెడల్పు = std::max(Orig_వెడల్పు, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(వెడల్పు);
  APInt Fract_Part_ముసుగు = APInt::getAllOnes(Scale).zext(వెడల్పు);
  APInt RadixInt = APInt(వెడల్పు, 10);

  IntPart.గమ్యం_String(Str, /*Radix=*/10);
  Str.నెట్టు_వెనుక('.');
  చేయి {
    (FractPart * RadixInt)
        .lshr(Scale)
        .గమ్యం_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_ముసుగు;
  } అయితే (FractPart != 0);
}

శూన్య APFixed_బిందువు::print(raw_ostream &OS) స్థిరరాశి {
  OS << "APFixedPoint(" << గమ్యం_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య APFixed_బిందువు::dump() స్థిరరాశి { print(llvm::errs()); }
#అయితే_ముగింపు

APFixed_బిందువు APFixed_బిందువు::negate(తార్కిక *అధికప్రవాహం) స్థిరరాశి {
  ఒకవేళ (!isSaturated()) {
    ఒకవేళ (అధికప్రవాహం)
      *అధికప్రవాహం =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_విలువ());
    తిరిగి APFixed_బిందువు(-Val, Sema);
  }

  // We never overflow for saturation
  ఒకవేళ (అధికప్రవాహం)
    *అధికప్రవాహం = అబద్ధం;

  ఒకవేళ (isSigned())
    తిరిగి Val.is_Min_Signed_విలువ() ? getMax(Sema) : APFixed_బిందువు(-Val, Sema);
  లేకపోతే
    తిరిగి APFixed_బిందువు(Sema);
}

APSInt APFixed_బిందువు::convert_గమ్యం_Int(గుర్తులేని Dst_వెడల్పు, తార్కిక DstSign,
                                  తార్కిక *అధికప్రవాహం) స్థిరరాశి {
  APSInt ఫలితం = getIntPart();
  గుర్తులేని Src_వెడల్పు = get_వెడల్పు();

  APSInt DstMin = APSInt::get_Min_విలువ(Dst_వెడల్పు, !DstSign);
  APSInt DstMax = APSInt::get_Max_విలువ(Dst_వెడల్పు, !DstSign);

  ఒకవేళ (Src_వెడల్పు < Dst_వెడల్పు) {
    ఫలితం = ఫలితం.extend(Dst_వెడల్పు);
  } లేకపోతే ఒకవేళ (Src_వెడల్పు > Dst_వెడల్పు) {
    DstMin = DstMin.extend(Src_వెడల్పు);
    DstMax = DstMax.extend(Src_వెడల్పు);
  }

  ఒకవేళ (అధికప్రవాహం) {
    ఒకవేళ (ఫలితం.isSigned() && !DstSign) {
      *అధికప్రవాహం = ఫలితం.isNegative() || ఫలితం.ugt(DstMax);
    } లేకపోతే ఒకవేళ (ఫలితం.isUnsigned() && DstSign) {
      *అధికప్రవాహం = ఫలితం.ugt(DstMax);
    } లేకపోతే {
      *అధికప్రవాహం = ఫలితం < DstMin || ఫలితం > DstMax;
    }
  }

  ఫలితం.సమితి_Is_Signed(DstSign);
  తిరిగి ఫలితం.extOrTrunc(Dst_వెడల్పు);
}

స్థిరరాశి fltSemantics *APFixed_బిందువు::promoteFloatSemantics(స్థిరరాశి fltSemantics *S) {
  ఒకవేళ (S == &APFloat::BFloat())
    తిరిగి &APFloat::IEEEdouble();
  లేకపోతే ఒకవేళ (S == &APFloat::IEEEhalf())
    తిరిగి &APFloat::IEEEsingle();
  లేకపోతే ఒకవేళ (S == &APFloat::IEEEsingle())
    తిరిగి &APFloat::IEEEdouble();
  లేకపోతే ఒకవేళ (S == &APFloat::IEEEdouble())
    తిరిగి &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixed_బిందువు::convert_గమ్యం_Float(స్థిరరాశి fltSemantics &FloatSema) స్థిరరాశి {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_బహుళకం RM = APFloat::rm_సమీప_Ties_గమ్యం_Even;
  APFloat::rounding_బహుళకం LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  స్థిరరాశి fltSemantics *OpSema = &FloatSema;
  అయితే (!Sema.సరిపోతుంది_లోపల_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::op_స్థితి S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (శూన్య)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat Scale_గుణకం(std::ఘాతం(2, Sema.get_Lsb_బరువు()));
  తార్కిక Ignored;
  Scale_గుణకం.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_గుణకం, LosslessRM);

  ఒకవేళ (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  తిరిగి Flt;
}

APFixed_బిందువు APFixed_బిందువు::get_From_Int_విలువ(స్థిరరాశి APSInt &విలువ,
                                           స్థిరరాశి Fixed_బిందువు_Semantics &DstFXSema,
                                           తార్కిక *అధికప్రవాహం) {
  Fixed_బిందువు_Semantics IntFXSema = Fixed_బిందువు_Semantics::GetIntegerSemantics(
      విలువ.get_బిట్టు_వెడల్పు(), విలువ.isSigned());
  తిరిగి APFixed_బిందువు(విలువ, IntFXSema).convert(DstFXSema, అధికప్రవాహం);
}

APFixed_బిందువు
APFixed_బిందువు::get_From_Float_విలువ(స్థిరరాశి APFloat &విలువ,
                                స్థిరరాశి Fixed_బిందువు_Semantics &DstFXSema,
                                తార్కిక *అధికప్రవాహం) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_బహుళకం RM = APFloat::rmTowardZero;
  APFloat::rounding_బహుళకం LosslessRM = APFloat::rmTowardZero;

  స్థిరరాశి fltSemantics &FloatSema = విలువ.getSemantics();

  ఒకవేళ (విలువ.isNaN()) {
    // Handle NaN immediately.
    ఒకవేళ (అధికప్రవాహం)
      *అధికప్రవాహం = నిజం;
    తిరిగి APFixed_బిందువు(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  స్థిరరాశి fltSemantics *OpSema = &FloatSema;
  అయితే (!DstFXSema.సరిపోతుంది_లోపల_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = విలువ;

  తార్కిక Ignored;
  ఒకవేళ (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_గుణకం(std::ఘాతం(2, -DstFXSema.get_Lsb_బరువు()));
  Scale_గుణకం.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_గుణకం, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_వెడల్పు(), !DstFXSema.isSigned());
  Val.convert_గమ్యం_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_గుణకం = APFloat(std::ఘాతం(2, DstFXSema.get_Lsb_బరువు()));
  Scale_గుణకం.convert(*OpSema, LosslessRM, &Ignored);
  Val.విడత_గమ్యం_Integral(RM);
  Val.multiply(Scale_గుణకం, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convert_గమ్యం_Float(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convert_గమ్యం_Float(*OpSema);
  తార్కిక Overflowed = అబద్ధం;
  ఒకవేళ (DstFXSema.isSaturated()) {
    ఒకవేళ (Val > FloatMax)
      Res = getMax(DstFXSema).get_విలువ();
    లేకపోతే ఒకవేళ (Val < FloatMin)
      Res = getMin(DstFXSema).get_విలువ();
  } లేకపోతే {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  ఒకవేళ (అధికప్రవాహం)
    *అధికప్రవాహం = Overflowed;

  తిరిగి APFixed_బిందువు(Res, DstFXSema);
}

} // namespace llvm
