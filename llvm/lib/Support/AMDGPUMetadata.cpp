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

#अन्तर्भाव "llvm/Support/AMDGPUMetadata.h"
#अन्तर्भाव "llvm/Support/YAMLTraits.h"
#अन्तर्भाव <optional>

उपयुज्य नामाकाश llvm::AMDGPU;
उपयुज्य नामाकाश llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_सदिशम्(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_सदिशम्(Kernel::Metadata)

नामाकाश llvm {
नामाकाश yaml {

प्रतिरूपम् <>
संरचना ScalarEnumerationTraits<AccessQualifier> {
  स्थिर शून्य enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

प्रतिरूपम् <>
संरचना ScalarEnumerationTraits<स्थानाङ्कः_Space_Qualifier> {
  स्थिर शून्य enumeration(IO &YIO, स्थानाङ्कः_Space_Qualifier &EN) {
    YIO.enumCase(EN, "Private", स्थानाङ्कः_Space_Qualifier::वैयक्तिकम्);
    YIO.enumCase(EN, "Global", स्थानाङ्कः_Space_Qualifier::Global);
    YIO.enumCase(EN, "Constant", स्थानाङ्कः_Space_Qualifier::Constant);
    YIO.enumCase(EN, "Local", स्थानाङ्कः_Space_Qualifier::Local);
    YIO.enumCase(EN, "Generic", स्थानाङ्कः_Space_Qualifier::Generic);
    YIO.enumCase(EN, "Region", स्थानाङ्कः_Space_Qualifier::Region);
  }
};

प्रतिरूपम् <>
संरचना ScalarEnumerationTraits<मानम्_विधा> {
  स्थिर शून्य enumeration(IO &YIO, मानम्_विधा &EN) {
    YIO.enumCase(EN, "ByValue", मानम्_विधा::By_मानम्);
    YIO.enumCase(EN, "GlobalBuffer", मानम्_विधा::Global_बफर);
    YIO.enumCase(EN, "DynamicSharedPointer", मानम्_विधा::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", मानम्_विधा::Sampler);
    YIO.enumCase(EN, "Image", मानम्_विधा::Image);
    YIO.enumCase(EN, "Pipe", मानम्_विधा::Pipe);
    YIO.enumCase(EN, "Queue", मानम्_विधा::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", मानम्_विधा::Hidden_Global_विस्थापनम्_X);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", मानम्_विधा::Hidden_Global_विस्थापनम्_Y);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", मानम्_विधा::Hidden_Global_विस्थापनम्_Z);
    YIO.enumCase(EN, "HiddenNone", मानम्_विधा::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", मानम्_विधा::Hidden_Printf_बफर);
    YIO.enumCase(EN, "HiddenHostcallBuffer", मानम्_विधा::Hidden_Hostcall_बफर);
    YIO.enumCase(EN, "HiddenDefaultQueue", मानम्_विधा::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 मानम्_विधा::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 मानम्_विधा::HiddenMultiGridSyncArg);
  }
};

प्रतिरूपम् <>
संरचना ScalarEnumerationTraits<मानम्_Type> {
  स्थिर शून्य enumeration(IO &YIO, मानम्_Type &EN) {
    YIO.enumCase(EN, "Struct", मानम्_Type::Struct);
    YIO.enumCase(EN, "I8", मानम्_Type::I8);
    YIO.enumCase(EN, "U8", मानम्_Type::U8);
    YIO.enumCase(EN, "I16", मानम्_Type::I16);
    YIO.enumCase(EN, "U16", मानम्_Type::U16);
    YIO.enumCase(EN, "F16", मानम्_Type::F16);
    YIO.enumCase(EN, "I32", मानम्_Type::I32);
    YIO.enumCase(EN, "U32", मानम्_Type::U32);
    YIO.enumCase(EN, "F32", मानम्_Type::F32);
    YIO.enumCase(EN, "I64", मानम्_Type::I64);
    YIO.enumCase(EN, "U64", मानम्_Type::U64);
    YIO.enumCase(EN, "F64", मानम्_Type::F64);
  }
};

प्रतिरूपम् <>
संरचना MappingTraits<Kernel::Attrs::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::कुञ्चिका::Reqd_Work_गणः_आकार,
                    MD.m_Reqd_Work_गणः_आकार, std::सदिशम्<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::कुञ्चिका::Work_गणः_आकार_Hint,
                    MD.m_Work_गणः_आकार_Hint, std::सदिशम्<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::कुञ्चिका::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::कुञ्चिका::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

प्रतिरूपम् <>
संरचना MappingTraits<Kernel::Arg::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::कुञ्चिका::आकार, MD.m_आकार);
    YIO.mapRequired(Kernel::Arg::कुञ्चिका::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::कुञ्चिका::मानम्_विधा, MD.m_मानम्_विधा);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<मानम्_Type> अप्रयुक्तम्;
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::मानम्_Type, अप्रयुक्तम्);

    YIO.mapOptional(Kernel::Arg::कुञ्चिका::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::AddrSpaceQual, MD.mAddrSpaceQual,
                    स्थानाङ्कः_Space_Qualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::IsConst, MD.mIsConst, असत्य);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::IsRestrict, MD.mIsRestrict, असत्य);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::IsVolatile, MD.mIsVolatile, असत्य);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::IsPipe, MD.mIsPipe, असत्य);
  }
};

प्रतिरूपम् <>
संरचना MappingTraits<Kernel::संहिता_Props::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::संहिता_Props::Metadata &MD) {
    YIO.mapRequired(Kernel::संहिता_Props::कुञ्चिका::Kernarg_Segment_आकार,
                    MD.m_Kernarg_Segment_आकार);
    YIO.mapRequired(Kernel::संहिता_Props::कुञ्चिका::गणः_Segment_Fixed_आकार,
                    MD.m_गणः_Segment_Fixed_आकार);
    YIO.mapRequired(Kernel::संहिता_Props::कुञ्चिका::वैयक्तिकम्_Segment_Fixed_आकार,
                    MD.m_वैयक्तिकम्_Segment_Fixed_आकार);
    YIO.mapRequired(Kernel::संहिता_Props::कुञ्चिका::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::संहिता_Props::कुञ्चिका::Wavefront_आकार,
                    MD.m_Wavefront_आकार);
    YIO.mapOptional(Kernel::संहिता_Props::कुञ्चिका::अङ्कः_SGPRs,
                    MD.m_अङ्कः_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::संहिता_Props::कुञ्चिका::अङ्कः_VGPRs,
                    MD.m_अङ्कः_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::संहिता_Props::कुञ्चिका::Max_Flat_Work_गणः_आकार,
                    MD.m_Max_Flat_Work_गणः_आकार, uint32_t(0));
    YIO.mapOptional(Kernel::संहिता_Props::कुञ्चिका::Is_Dynamic_Call_राशिः,
                    MD.m_Is_Dynamic_Call_राशिः, असत्य);
    YIO.mapOptional(Kernel::संहिता_Props::कुञ्चिका::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, असत्य);
    YIO.mapOptional(Kernel::संहिता_Props::कुञ्चिका::अङ्कः_Spilled_SGPRs,
                    MD.m_अङ्कः_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::संहिता_Props::कुञ्चिका::अङ्कः_Spilled_VGPRs,
                    MD.m_अङ्कः_Spilled_VGPRs, uint16_t(0));
  }
};

प्रतिरूपम् <>
संरचना MappingTraits<Kernel::DebugProps::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::सदिशम्<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::Reserved_अङ्कः_VGPRs,
                    MD.m_Reserved_अङ्कः_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::वैयक्तिकम्_Segment_बफर_SGPR,
                    MD.m_वैयक्तिकम्_Segment_बफर_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::Wavefront_वैयक्तिकम्_Segment_विस्थापनम्_SGPR,
                    MD.m_Wavefront_वैयक्तिकम्_Segment_विस्थापनम्_SGPR, uint16_t(-1));
  }
};

प्रतिरूपम् <>
संरचना MappingTraits<Kernel::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::कुञ्चिका::Name, MD.mName);
    YIO.mapRequired(Kernel::कुञ्चिका::चिह्नम्_Name, MD.m_चिह्नम्_Name);
    YIO.mapOptional(Kernel::कुञ्चिका::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::कुञ्चिका::Language_संस्करणम्, MD.m_Language_संस्करणम्,
                    std::सदिशम्<uint32_t>());
    यदि (!MD.mAttrs.रिक्तानि() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुञ्चिका::Attrs, MD.mAttrs);
    यदि (!MD.m_तर्काः.रिक्तानि() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुञ्चिका::तर्काः, MD.m_तर्काः);
    यदि (!MD.m_संहिता_Props.रिक्तानि() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुञ्चिका::संहिता_Props, MD.m_संहिता_Props);
    यदि (!MD.mDebugProps.रिक्तानि() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुञ्चिका::DebugProps, MD.mDebugProps);
  }
};

प्रतिरूपम् <>
संरचना MappingTraits<HSAMD::Metadata> {
  स्थिर शून्य mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(कुञ्चिका::संस्करणम्, MD.m_संस्करणम्);
    YIO.mapOptional(कुञ्चिका::Printf, MD.mPrintf, std::सदिशम्<std::string>());
    यदि (!MD.mKernels.रिक्तानि() || !YIO.outputting())
      YIO.mapOptional(कुञ्चिका::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

नामाकाश AMDGPU {
नामाकाश HSAMD {

std::error_संहिता fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::निवेशः Yaml_निवेशः(String);
  Yaml_निवेशः >> HSAMetadata;
  प्रत्यागम Yaml_निवेशः.error();
}

std::error_संहिता गन्तव्यम्_String(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream Yaml_प्रवाहः(String);
  yaml::Output YamlOutput(Yaml_प्रवाहः, शून्यसूचकः, std::numeric_limits<पूर्णाङ्क>::max());
  YamlOutput << HSAMetadata;
  प्रत्यागम std::error_संहिता();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
