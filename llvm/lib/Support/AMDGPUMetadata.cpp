//===--- AMDGPUMetadata.cpp -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// AMDGPU metadata definitions and in-memory representations.
///
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/AMDGPUMetadata.h"
#ضمن "llvm/Support/YAMLTraits.h"
#ضمن <optional>

کارول نطاق llvm::AMDGPU;
کارول نطاق llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_ویکټور(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_ویکټور(Kernel::Metadata)

نطاق llvm {
نطاق yaml {

کينډۍ <>
بنية ScalarEnumerationTraits<AccessQualifier> {
  ساكن فراغ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

کينډۍ <>
بنية ScalarEnumerationTraits<پته_Space_Qualifier> {
  ساكن فراغ enumeration(IO &YIO, پته_Space_Qualifier &EN) {
    YIO.enumCase(EN, "Private", پته_Space_Qualifier::شخصي);
    YIO.enumCase(EN, "Global", پته_Space_Qualifier::Global);
    YIO.enumCase(EN, "Constant", پته_Space_Qualifier::Constant);
    YIO.enumCase(EN, "Local", پته_Space_Qualifier::Local);
    YIO.enumCase(EN, "Generic", پته_Space_Qualifier::Generic);
    YIO.enumCase(EN, "Region", پته_Space_Qualifier::Region);
  }
};

کينډۍ <>
بنية ScalarEnumerationTraits<ارزښت_ډول> {
  ساكن فراغ enumeration(IO &YIO, ارزښت_ډول &EN) {
    YIO.enumCase(EN, "ByValue", ارزښت_ډول::By_ارزښت);
    YIO.enumCase(EN, "GlobalBuffer", ارزښت_ډول::Global_بفر);
    YIO.enumCase(EN, "DynamicSharedPointer", ارزښت_ډول::Dynamic_Shared_اشاره_ګر);
    YIO.enumCase(EN, "Sampler", ارزښت_ډول::Sampler);
    YIO.enumCase(EN, "Image", ارزښت_ډول::Image);
    YIO.enumCase(EN, "Pipe", ارزښت_ډول::Pipe);
    YIO.enumCase(EN, "Queue", ارزښت_ډول::کتار);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", ارزښت_ډول::Hidden_Global_دننه_واټن_X);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", ارزښت_ډول::Hidden_Global_دننه_واټن_Y);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", ارزښت_ډول::Hidden_Global_دننه_واټن_Z);
    YIO.enumCase(EN, "HiddenNone", ارزښت_ډول::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", ارزښت_ډول::Hidden_Printf_بفر);
    YIO.enumCase(EN, "HiddenHostcallBuffer", ارزښت_ډول::Hidden_Hostcall_بفر);
    YIO.enumCase(EN, "HiddenDefaultQueue", ارزښت_ډول::Hidden_Default_کتار);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 ارزښت_ډول::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 ارزښت_ډول::HiddenMultiGridSyncArg);
  }
};

کينډۍ <>
بنية ScalarEnumerationTraits<ارزښت_Type> {
  ساكن فراغ enumeration(IO &YIO, ارزښت_Type &EN) {
    YIO.enumCase(EN, "Struct", ارزښت_Type::Struct);
    YIO.enumCase(EN, "I8", ارزښت_Type::I8);
    YIO.enumCase(EN, "U8", ارزښت_Type::U8);
    YIO.enumCase(EN, "I16", ارزښت_Type::I16);
    YIO.enumCase(EN, "U16", ارزښت_Type::U16);
    YIO.enumCase(EN, "F16", ارزښت_Type::F16);
    YIO.enumCase(EN, "I32", ارزښت_Type::I32);
    YIO.enumCase(EN, "U32", ارزښت_Type::U32);
    YIO.enumCase(EN, "F32", ارزښت_Type::F32);
    YIO.enumCase(EN, "I64", ارزښت_Type::I64);
    YIO.enumCase(EN, "U64", ارزښت_Type::U64);
    YIO.enumCase(EN, "F64", ارزښت_Type::F64);
  }
};

کينډۍ <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::کلی::Reqd_Work_ډله_اندازه,
                    MD.m_Reqd_Work_ډله_اندازه, std::ویکټور<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلی::Work_ډله_اندازه_Hint,
                    MD.m_Work_ډله_اندازه_Hint, std::ویکټور<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلی::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::کلی::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

کينډۍ <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::کلی::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::کلی::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::کلی::اندازه, MD.m_اندازه);
    YIO.mapRequired(Kernel::Arg::کلی::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::کلی::ارزښت_ډول, MD.m_ارزښت_ډول);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<ارزښت_Type> ناکارول_شوی;
    YIO.mapOptional(Kernel::Arg::کلی::ارزښت_Type, ناکارول_شوی);

    YIO.mapOptional(Kernel::Arg::کلی::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::کلی::AddrSpaceQual, MD.mAddrSpaceQual,
                    پته_Space_Qualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلی::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلی::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلی::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::کلی::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::کلی::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::کلی::IsPipe, MD.mIsPipe, خطأ);
  }
};

کينډۍ <>
بنية MappingTraits<Kernel::کوډ_Props::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::کوډ_Props::Metadata &MD) {
    YIO.mapRequired(Kernel::کوډ_Props::کلی::Kernarg_Segment_اندازه,
                    MD.m_Kernarg_Segment_اندازه);
    YIO.mapRequired(Kernel::کوډ_Props::کلی::ډله_Segment_Fixed_اندازه,
                    MD.m_ډله_Segment_Fixed_اندازه);
    YIO.mapRequired(Kernel::کوډ_Props::کلی::شخصي_Segment_Fixed_اندازه,
                    MD.m_شخصي_Segment_Fixed_اندازه);
    YIO.mapRequired(Kernel::کوډ_Props::کلی::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::کوډ_Props::کلی::Wavefront_اندازه,
                    MD.m_Wavefront_اندازه);
    YIO.mapOptional(Kernel::کوډ_Props::کلی::عدد_SGPRs,
                    MD.m_عدد_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کوډ_Props::کلی::عدد_VGPRs,
                    MD.m_عدد_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کوډ_Props::کلی::Max_Flat_Work_ډله_اندازه,
                    MD.m_Max_Flat_Work_ډله_اندازه, uint32_t(0));
    YIO.mapOptional(Kernel::کوډ_Props::کلی::Is_Dynamic_Call_ډېری,
                    MD.m_Is_Dynamic_Call_ډېری, خطأ);
    YIO.mapOptional(Kernel::کوډ_Props::کلی::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::کوډ_Props::کلی::عدد_Spilled_SGPRs,
                    MD.m_عدد_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کوډ_Props::کلی::عدد_Spilled_VGPRs,
                    MD.m_عدد_Spilled_VGPRs, uint16_t(0));
  }
};

کينډۍ <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::کلی::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::ویکټور<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::کلی::Reserved_عدد_VGPRs,
                    MD.m_Reserved_عدد_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::کلی::Reserved_لومړی_VGPR,
                    MD.m_Reserved_لومړی_VGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلی::شخصي_Segment_بفر_SGPR,
                    MD.m_شخصي_Segment_بفر_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلی::Wavefront_شخصي_Segment_دننه_واټن_SGPR,
                    MD.m_Wavefront_شخصي_Segment_دننه_واټن_SGPR, uint16_t(-1));
  }
};

کينډۍ <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::کلی::Name, MD.mName);
    YIO.mapRequired(Kernel::کلی::نښه_Name, MD.m_نښه_Name);
    YIO.mapOptional(Kernel::کلی::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::کلی::Language_نسخه, MD.m_Language_نسخه,
                    std::ویکټور<uint32_t>());
    إذا (!MD.mAttrs.تشه() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلی::Attrs, MD.mAttrs);
    إذا (!MD.m_دلیلونه.تشه() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلی::دلیلونه, MD.m_دلیلونه);
    إذا (!MD.m_کوډ_Props.تشه() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلی::کوډ_Props, MD.m_کوډ_Props);
    إذا (!MD.mDebugProps.تشه() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلی::DebugProps, MD.mDebugProps);
  }
};

کينډۍ <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(کلی::نسخه, MD.m_نسخه);
    YIO.mapOptional(کلی::Printf, MD.mPrintf, std::ویکټور<std::string>());
    إذا (!MD.mKernels.تشه() || !YIO.outputting())
      YIO.mapOptional(کلی::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_کوډ fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::ننوت Yaml_ننوت(String);
  Yaml_ننوت >> HSAMetadata;
  أرجع Yaml_ننوت.error();
}

std::error_کوډ منزل_String(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream Yaml_بهیر(String);
  yaml::Output YamlOutput(Yaml_بهیر, تش_اشاره, std::numeric_limits<صحيح>::max());
  YamlOutput << HSAMetadata;
  أرجع std::error_کوډ();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
