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

استفاده نطاق llvm::AMDGPU;
استفاده نطاق llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_وکتور(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_وکتور(Kernel::Metadata)

نطاق llvm {
نطاق yaml {

قالب <>
بنية ScalarEnumerationTraits<AccessQualifier> {
  ساكن فراغ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

قالب <>
بنية ScalarEnumerationTraits<آدرس_Space_Qualifier> {
  ساكن فراغ enumeration(IO &YIO, آدرس_Space_Qualifier &EN) {
    YIO.enumCase(EN, "Private", آدرس_Space_Qualifier::خصوصی);
    YIO.enumCase(EN, "Global", آدرس_Space_Qualifier::Global);
    YIO.enumCase(EN, "Constant", آدرس_Space_Qualifier::Constant);
    YIO.enumCase(EN, "Local", آدرس_Space_Qualifier::Local);
    YIO.enumCase(EN, "Generic", آدرس_Space_Qualifier::Generic);
    YIO.enumCase(EN, "Region", آدرس_Space_Qualifier::Region);
  }
};

قالب <>
بنية ScalarEnumerationTraits<مقدار_نوع> {
  ساكن فراغ enumeration(IO &YIO, مقدار_نوع &EN) {
    YIO.enumCase(EN, "ByValue", مقدار_نوع::By_مقدار);
    YIO.enumCase(EN, "GlobalBuffer", مقدار_نوع::Global_میانگیر);
    YIO.enumCase(EN, "DynamicSharedPointer", مقدار_نوع::Dynamic_Shared_اشاره_گر);
    YIO.enumCase(EN, "Sampler", مقدار_نوع::Sampler);
    YIO.enumCase(EN, "Image", مقدار_نوع::Image);
    YIO.enumCase(EN, "Pipe", مقدار_نوع::Pipe);
    YIO.enumCase(EN, "Queue", مقدار_نوع::صف);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", مقدار_نوع::Hidden_Global_جابه_جایی_X);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", مقدار_نوع::Hidden_Global_جابه_جایی_Y);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", مقدار_نوع::Hidden_Global_جابه_جایی_Z);
    YIO.enumCase(EN, "HiddenNone", مقدار_نوع::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", مقدار_نوع::Hidden_Printf_میانگیر);
    YIO.enumCase(EN, "HiddenHostcallBuffer", مقدار_نوع::Hidden_Hostcall_میانگیر);
    YIO.enumCase(EN, "HiddenDefaultQueue", مقدار_نوع::Hidden_Default_صف);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 مقدار_نوع::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 مقدار_نوع::HiddenMultiGridSyncArg);
  }
};

قالب <>
بنية ScalarEnumerationTraits<مقدار_Type> {
  ساكن فراغ enumeration(IO &YIO, مقدار_Type &EN) {
    YIO.enumCase(EN, "Struct", مقدار_Type::Struct);
    YIO.enumCase(EN, "I8", مقدار_Type::I8);
    YIO.enumCase(EN, "U8", مقدار_Type::U8);
    YIO.enumCase(EN, "I16", مقدار_Type::I16);
    YIO.enumCase(EN, "U16", مقدار_Type::U16);
    YIO.enumCase(EN, "F16", مقدار_Type::F16);
    YIO.enumCase(EN, "I32", مقدار_Type::I32);
    YIO.enumCase(EN, "U32", مقدار_Type::U32);
    YIO.enumCase(EN, "F32", مقدار_Type::F32);
    YIO.enumCase(EN, "I64", مقدار_Type::I64);
    YIO.enumCase(EN, "U64", مقدار_Type::U64);
    YIO.enumCase(EN, "F64", مقدار_Type::F64);
  }
};

قالب <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::کلید::Reqd_Work_گروه_اندازه,
                    MD.m_Reqd_Work_گروه_اندازه, std::وکتور<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلید::Work_گروه_اندازه_Hint,
                    MD.m_Work_گروه_اندازه_Hint, std::وکتور<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلید::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::کلید::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

قالب <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::کلید::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::کلید::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::کلید::اندازه, MD.m_اندازه);
    YIO.mapRequired(Kernel::Arg::کلید::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::کلید::مقدار_نوع, MD.m_مقدار_نوع);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<مقدار_Type> استفادهنشده;
    YIO.mapOptional(Kernel::Arg::کلید::مقدار_Type, استفادهنشده);

    YIO.mapOptional(Kernel::Arg::کلید::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::کلید::AddrSpaceQual, MD.mAddrSpaceQual,
                    آدرس_Space_Qualifier::Unknown);
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

قالب <>
بنية MappingTraits<Kernel::کد_Props::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::کد_Props::Metadata &MD) {
    YIO.mapRequired(Kernel::کد_Props::کلید::Kernarg_Segment_اندازه,
                    MD.m_Kernarg_Segment_اندازه);
    YIO.mapRequired(Kernel::کد_Props::کلید::گروه_Segment_Fixed_اندازه,
                    MD.m_گروه_Segment_Fixed_اندازه);
    YIO.mapRequired(Kernel::کد_Props::کلید::خصوصی_Segment_Fixed_اندازه,
                    MD.m_خصوصی_Segment_Fixed_اندازه);
    YIO.mapRequired(Kernel::کد_Props::کلید::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::کد_Props::کلید::Wavefront_اندازه,
                    MD.m_Wavefront_اندازه);
    YIO.mapOptional(Kernel::کد_Props::کلید::رقم_SGPRs,
                    MD.m_رقم_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کد_Props::کلید::رقم_VGPRs,
                    MD.m_رقم_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کد_Props::کلید::Max_Flat_Work_گروه_اندازه,
                    MD.m_Max_Flat_Work_گروه_اندازه, uint32_t(0));
    YIO.mapOptional(Kernel::کد_Props::کلید::Is_Dynamic_Call_پشته,
                    MD.m_Is_Dynamic_Call_پشته, خطأ);
    YIO.mapOptional(Kernel::کد_Props::کلید::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::کد_Props::کلید::رقم_Spilled_SGPRs,
                    MD.m_رقم_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::کد_Props::کلید::رقم_Spilled_VGPRs,
                    MD.m_رقم_Spilled_VGPRs, uint16_t(0));
  }
};

قالب <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::کلید::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::وکتور<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::کلید::Reserved_رقم_VGPRs,
                    MD.m_Reserved_رقم_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::کلید::Reserved_اولی_VGPR,
                    MD.m_Reserved_اولی_VGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلید::خصوصی_Segment_میانگیر_SGPR,
                    MD.m_خصوصی_Segment_میانگیر_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلید::Wavefront_خصوصی_Segment_جابه_جایی_SGPR,
                    MD.m_Wavefront_خصوصی_Segment_جابه_جایی_SGPR, uint16_t(-1));
  }
};

قالب <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::کلید::Name, MD.mName);
    YIO.mapRequired(Kernel::کلید::نماد_Name, MD.m_نماد_Name);
    YIO.mapOptional(Kernel::کلید::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::کلید::Language_نسخه, MD.m_Language_نسخه,
                    std::وکتور<uint32_t>());
    إذا (!MD.mAttrs.خالی() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::Attrs, MD.mAttrs);
    إذا (!MD.m_آرگومانها.خالی() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::آرگومانها, MD.m_آرگومانها);
    إذا (!MD.m_کد_Props.خالی() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::کد_Props, MD.m_کد_Props);
    إذا (!MD.mDebugProps.خالی() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::DebugProps, MD.mDebugProps);
  }
};

قالب <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(کلید::نسخه, MD.m_نسخه);
    YIO.mapOptional(کلید::Printf, MD.mPrintf, std::وکتور<std::string>());
    إذا (!MD.mKernels.خالی() || !YIO.outputting())
      YIO.mapOptional(کلید::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_کد fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::درون_داد Yaml_درون_داد(String);
  Yaml_درون_داد >> HSAMetadata;
  أرجع Yaml_درون_داد.error();
}

std::error_کد مقصد_String(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream Yaml_جریان(String);
  yaml::Output YamlOutput(Yaml_جریان, اشاره_تهی, std::numeric_limits<صحيح>::max());
  YamlOutput << HSAMetadata;
  أرجع std::error_کد();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
