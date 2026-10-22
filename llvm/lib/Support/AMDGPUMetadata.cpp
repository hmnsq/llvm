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

استعمال نطاق llvm::AMDGPU;
استعمال نطاق llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_ويڪٽر(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_ويڪٽر(Kernel::Metadata)

نطاق llvm {
نطاق yaml {

سانچو <>
بنية ScalarEnumerationTraits<AccessQualifier> {
  ساكن فراغ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

سانچو <>
بنية ScalarEnumerationTraits<پتو_Space_Qualifier> {
  ساكن فراغ enumeration(IO &YIO, پتو_Space_Qualifier &EN) {
    YIO.enumCase(EN, "Private", پتو_Space_Qualifier::ذاتي);
    YIO.enumCase(EN, "Global", پتو_Space_Qualifier::Global);
    YIO.enumCase(EN, "Constant", پتو_Space_Qualifier::Constant);
    YIO.enumCase(EN, "Local", پتو_Space_Qualifier::Local);
    YIO.enumCase(EN, "Generic", پتو_Space_Qualifier::Generic);
    YIO.enumCase(EN, "Region", پتو_Space_Qualifier::Region);
  }
};

سانچو <>
بنية ScalarEnumerationTraits<قدر_قسم> {
  ساكن فراغ enumeration(IO &YIO, قدر_قسم &EN) {
    YIO.enumCase(EN, "ByValue", قدر_قسم::By_قدر);
    YIO.enumCase(EN, "GlobalBuffer", قدر_قسم::Global_بفر);
    YIO.enumCase(EN, "DynamicSharedPointer", قدر_قسم::Dynamic_Shared_اشارو);
    YIO.enumCase(EN, "Sampler", قدر_قسم::Sampler);
    YIO.enumCase(EN, "Image", قدر_قسم::Image);
    YIO.enumCase(EN, "Pipe", قدر_قسم::Pipe);
    YIO.enumCase(EN, "Queue", قدر_قسم::قطار);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", قدر_قسم::Hidden_Global_وٿي_X);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", قدر_قسم::Hidden_Global_وٿي_Y);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", قدر_قسم::Hidden_Global_وٿي_Z);
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

سانچو <>
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

سانچو <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::ڪنجي::Reqd_Work_گروهه_سائيز,
                    MD.m_Reqd_Work_گروهه_سائيز, std::ويڪٽر<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ڪنجي::Work_گروهه_سائيز_Hint,
                    MD.m_Work_گروهه_سائيز_Hint, std::ويڪٽر<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ڪنجي::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::ڪنجي::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

سانچو <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::ڪنجي::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::ڪنجي::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::ڪنجي::سائيز, MD.m_سائيز);
    YIO.mapRequired(Kernel::Arg::ڪنجي::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::ڪنجي::قدر_قسم, MD.m_قدر_قسم);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<قدر_Type> اڻ_استعمال_ٿيل;
    YIO.mapOptional(Kernel::Arg::ڪنجي::قدر_Type, اڻ_استعمال_ٿيل);

    YIO.mapOptional(Kernel::Arg::ڪنجي::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::ڪنجي::AddrSpaceQual, MD.mAddrSpaceQual,
                    پتو_Space_Qualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ڪنجي::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ڪنجي::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ڪنجي::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::ڪنجي::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::ڪنجي::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::ڪنجي::IsPipe, MD.mIsPipe, خطأ);
  }
};

سانچو <>
بنية MappingTraits<Kernel::ڪوڊ_Props::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::ڪوڊ_Props::Metadata &MD) {
    YIO.mapRequired(Kernel::ڪوڊ_Props::ڪنجي::Kernarg_Segment_سائيز,
                    MD.m_Kernarg_Segment_سائيز);
    YIO.mapRequired(Kernel::ڪوڊ_Props::ڪنجي::گروهه_Segment_Fixed_سائيز,
                    MD.m_گروهه_Segment_Fixed_سائيز);
    YIO.mapRequired(Kernel::ڪوڊ_Props::ڪنجي::ذاتي_Segment_Fixed_سائيز,
                    MD.m_ذاتي_Segment_Fixed_سائيز);
    YIO.mapRequired(Kernel::ڪوڊ_Props::ڪنجي::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::ڪوڊ_Props::ڪنجي::Wavefront_سائيز,
                    MD.m_Wavefront_سائيز);
    YIO.mapOptional(Kernel::ڪوڊ_Props::ڪنجي::انگ_SGPRs,
                    MD.m_انگ_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ڪوڊ_Props::ڪنجي::انگ_VGPRs,
                    MD.m_انگ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ڪوڊ_Props::ڪنجي::Max_Flat_Work_گروهه_سائيز,
                    MD.m_Max_Flat_Work_گروهه_سائيز, uint32_t(0));
    YIO.mapOptional(Kernel::ڪوڊ_Props::ڪنجي::Is_Dynamic_Call_ڍير,
                    MD.m_Is_Dynamic_Call_ڍير, خطأ);
    YIO.mapOptional(Kernel::ڪوڊ_Props::ڪنجي::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::ڪوڊ_Props::ڪنجي::انگ_Spilled_SGPRs,
                    MD.m_انگ_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ڪوڊ_Props::ڪنجي::انگ_Spilled_VGPRs,
                    MD.m_انگ_Spilled_VGPRs, uint16_t(0));
  }
};

سانچو <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::ڪنجي::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::ويڪٽر<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::ڪنجي::Reserved_انگ_VGPRs,
                    MD.m_Reserved_انگ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::ڪنجي::Reserved_پهريون_VGPR,
                    MD.m_Reserved_پهريون_VGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ڪنجي::ذاتي_Segment_بفر_SGPR,
                    MD.m_ذاتي_Segment_بفر_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ڪنجي::Wavefront_ذاتي_Segment_وٿي_SGPR,
                    MD.m_Wavefront_ذاتي_Segment_وٿي_SGPR, uint16_t(-1));
  }
};

سانچو <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::ڪنجي::Name, MD.mName);
    YIO.mapRequired(Kernel::ڪنجي::نشان_Name, MD.m_نشان_Name);
    YIO.mapOptional(Kernel::ڪنجي::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::ڪنجي::Language_نسخو, MD.m_Language_نسخو,
                    std::ويڪٽر<uint32_t>());
    إذا (!MD.mAttrs.خالي_جايون() || !YIO.outputting())
      YIO.mapOptional(Kernel::ڪنجي::Attrs, MD.mAttrs);
    إذا (!MD.m_دليلون.خالي_جايون() || !YIO.outputting())
      YIO.mapOptional(Kernel::ڪنجي::دليلون, MD.m_دليلون);
    إذا (!MD.m_ڪوڊ_Props.خالي_جايون() || !YIO.outputting())
      YIO.mapOptional(Kernel::ڪنجي::ڪوڊ_Props, MD.m_ڪوڊ_Props);
    إذا (!MD.mDebugProps.خالي_جايون() || !YIO.outputting())
      YIO.mapOptional(Kernel::ڪنجي::DebugProps, MD.mDebugProps);
  }
};

سانچو <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(ڪنجي::نسخو, MD.m_نسخو);
    YIO.mapOptional(ڪنجي::Printf, MD.mPrintf, std::ويڪٽر<std::string>());
    إذا (!MD.mKernels.خالي_جايون() || !YIO.outputting())
      YIO.mapOptional(ڪنجي::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_ڪوڊ fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::داخلا Yaml_داخلا(String);
  Yaml_داخلا >> HSAMetadata;
  أرجع Yaml_داخلا.error();
}

std::error_ڪوڊ منزل_String(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream Yaml_وهڪرو(String);
  yaml::Output YamlOutput(Yaml_وهڪرو, خالي_اشارو, std::numeric_limits<صحيح>::max());
  YamlOutput << HSAMetadata;
  أرجع std::error_ڪوڊ();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
