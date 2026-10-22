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

ورتوں نطاق llvm::AMDGPU;
ورتوں نطاق llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_سمتیہ(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_سمتیہ(Kernel::Metadata)

نطاق llvm {
نطاق yaml {

سانچا <>
بنية ScalarEnumerationTraits<AccessQualifier> {
  ساكن فراغ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

سانچا <>
بنية ScalarEnumerationTraits<پتہ_Space_Qualifier> {
  ساكن فراغ enumeration(IO &YIO, پتہ_Space_Qualifier &EN) {
    YIO.enumCase(EN, "Private", پتہ_Space_Qualifier::نجی);
    YIO.enumCase(EN, "Global", پتہ_Space_Qualifier::Global);
    YIO.enumCase(EN, "Constant", پتہ_Space_Qualifier::Constant);
    YIO.enumCase(EN, "Local", پتہ_Space_Qualifier::Local);
    YIO.enumCase(EN, "Generic", پتہ_Space_Qualifier::Generic);
    YIO.enumCase(EN, "Region", پتہ_Space_Qualifier::Region);
  }
};

سانچا <>
بنية ScalarEnumerationTraits<قدر_قسم> {
  ساكن فراغ enumeration(IO &YIO, قدر_قسم &EN) {
    YIO.enumCase(EN, "ByValue", قدر_قسم::By_قدر);
    YIO.enumCase(EN, "GlobalBuffer", قدر_قسم::Global_بفر);
    YIO.enumCase(EN, "DynamicSharedPointer", قدر_قسم::Dynamic_Shared_اشارہ_گر);
    YIO.enumCase(EN, "Sampler", قدر_قسم::Sampler);
    YIO.enumCase(EN, "Image", قدر_قسم::Image);
    YIO.enumCase(EN, "Pipe", قدر_قسم::Pipe);
    YIO.enumCase(EN, "Queue", قدر_قسم::قطار);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", قدر_قسم::Hidden_Global_وتھ_X);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", قدر_قسم::Hidden_Global_وتھ_Y);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", قدر_قسم::Hidden_Global_وتھ_Z);
    YIO.enumCase(EN, "HiddenNone", قدر_قسم::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", قدر_قسم::Hidden_Printf_بفر);
    YIO.enumCase(EN, "HiddenHostcallBuffer", قدر_قسم::Hidden_Hostcall_بفر);
    YIO.enumCase(EN, "HiddenDefaultQueue", قدر_قسم::Hidden_Default_قطار);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 قدر_قسم::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 قدر_قسم::HiddenMultiGridSyncArg);
  }
};

سانچا <>
بنية ScalarEnumerationTraits<قدر_Type> {
  ساكن فراغ enumeration(IO &YIO, قدر_Type &EN) {
    YIO.enumCase(EN, "Struct", قدر_Type::Struct);
    YIO.enumCase(EN, "I8", قدر_Type::I8);
    YIO.enumCase(EN, "U8", قدر_Type::U8);
    YIO.enumCase(EN, "I16", قدر_Type::I16);
    YIO.enumCase(EN, "U16", قدر_Type::U16);
    YIO.enumCase(EN, "F16", قدر_Type::F16);
    YIO.enumCase(EN, "I32", قدر_Type::I32);
    YIO.enumCase(EN, "U32", قدر_Type::U32);
    YIO.enumCase(EN, "F32", قدر_Type::F32);
    YIO.enumCase(EN, "I64", قدر_Type::I64);
    YIO.enumCase(EN, "U64", قدر_Type::U64);
    YIO.enumCase(EN, "F64", قدر_Type::F64);
  }
};

سانچا <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::کلید::Reqd_Work_گروہ_ناپ,
                    MD.m_Reqd_Work_گروہ_ناپ, std::سمتیہ<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلید::Work_گروہ_ناپ_Hint,
                    MD.m_Work_گروہ_ناپ_Hint, std::سمتیہ<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلید::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::کلید::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

سانچا <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::کلید::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::کلید::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::کلید::ناپ, MD.m_ناپ);
    YIO.mapRequired(Kernel::Arg::کلید::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::کلید::قدر_قسم, MD.m_قدر_قسم);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<قدر_Type> بن_ورتیا;
    YIO.mapOptional(Kernel::Arg::کلید::قدر_Type, بن_ورتیا);

    YIO.mapOptional(Kernel::Arg::کلید::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::کلید::AddrSpaceQual, MD.mAddrSpaceQual,
                    پتہ_Space_Qualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::کلید::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::کلید::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::کلید::IsPipe, MD.mIsPipe, خطأ);
  }
};

سانچا <>
بنية MappingTraits<Kernel::کوڈ_Props::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::کوڈ_Props::Metadata &MD) {
    YIO.mapRequired(Kernel::کوڈ_Props::کلید::Kernarg_Segment_ناپ,
                    MD.m_Kernarg_Segment_ناپ);
    YIO.mapRequired(Kernel::کوڈ_Props::کلید::گروہ_Segment_Fixed_ناپ,
                    MD.m_گروہ_Segment_Fixed_ناپ);
    YIO.mapRequired(Kernel::کوڈ_Props::کلید::نجی_Segment_Fixed_ناپ,
                    MD.m_نجی_Segment_Fixed_ناپ);
    YIO.mapRequired(Kernel::کوڈ_Props::کلید::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::کوڈ_Props::کلید::Wavefront_ناپ,
                    MD.m_Wavefront_ناپ);
    YIO.mapOptional(Kernel::کوڈ_Props::کلید::عدد_SGPRs,
                    MD.m_عدد_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کوڈ_Props::کلید::عدد_VGPRs,
                    MD.m_عدد_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کوڈ_Props::کلید::Max_Flat_Work_گروہ_ناپ,
                    MD.m_Max_Flat_Work_گروہ_ناپ, uint32_t(0));
    YIO.mapOptional(Kernel::کوڈ_Props::کلید::Is_Dynamic_Call_ڈھیر,
                    MD.m_Is_Dynamic_Call_ڈھیر, خطأ);
    YIO.mapOptional(Kernel::کوڈ_Props::کلید::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::کوڈ_Props::کلید::عدد_Spilled_SGPRs,
                    MD.m_عدد_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کوڈ_Props::کلید::عدد_Spilled_VGPRs,
                    MD.m_عدد_Spilled_VGPRs, uint16_t(0));
  }
};

سانچا <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::کلید::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::سمتیہ<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::کلید::Reserved_عدد_VGPRs,
                    MD.m_Reserved_عدد_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::کلید::Reserved_پہلا_VGPR,
                    MD.m_Reserved_پہلا_VGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلید::نجی_Segment_بفر_SGPR,
                    MD.m_نجی_Segment_بفر_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلید::Wavefront_نجی_Segment_وتھ_SGPR,
                    MD.m_Wavefront_نجی_Segment_وتھ_SGPR, uint16_t(-1));
  }
};

سانچا <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::کلید::Name, MD.mName);
    YIO.mapRequired(Kernel::کلید::علامت_Name, MD.m_علامت_Name);
    YIO.mapOptional(Kernel::کلید::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::کلید::Language_نسخہ, MD.m_Language_نسخہ,
                    std::سمتیہ<uint32_t>());
    إذا (!MD.mAttrs.خالی_تھاواں() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::Attrs, MD.mAttrs);
    إذا (!MD.m_دلیلاں.خالی_تھاواں() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::دلیلاں, MD.m_دلیلاں);
    إذا (!MD.m_کوڈ_Props.خالی_تھاواں() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::کوڈ_Props, MD.m_کوڈ_Props);
    إذا (!MD.mDebugProps.خالی_تھاواں() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::DebugProps, MD.mDebugProps);
  }
};

سانچا <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(کلید::نسخہ, MD.m_نسخہ);
    YIO.mapOptional(کلید::Printf, MD.mPrintf, std::سمتیہ<std::string>());
    إذا (!MD.mKernels.خالی_تھاواں() || !YIO.outputting())
      YIO.mapOptional(کلید::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_کوڈ fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::داخلہ Yaml_داخلہ(String);
  Yaml_داخلہ >> HSAMetadata;
  أرجع Yaml_داخلہ.error();
}

std::error_کوڈ منزل_String(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream Yaml_دھارا(String);
  yaml::Output YamlOutput(Yaml_دھارا, خالی_سوچک, std::numeric_limits<صحيح>::max());
  YamlOutput << HSAMetadata;
  أرجع std::error_کوڈ();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
