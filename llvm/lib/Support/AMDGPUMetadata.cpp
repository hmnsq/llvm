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

#అన్తర్భావ "llvm/Support/AMDGPUMetadata.h"
#అన్తర్భావ "llvm/Support/YAMLTraits.h"
#అన్తర్భావ <optional>

ఉపయోగించి నామాకాశ llvm::AMDGPU;
ఉపయోగించి నామాకాశ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_సదిశ(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_సదిశ(Kernel::Metadata)

నామాకాశ llvm {
నామాకాశ yaml {

మూస <>
నిర్మాణం ScalarEnumerationTraits<AccessQualifier> {
  స్థిర శూన్య enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

మూస <>
నిర్మాణం ScalarEnumerationTraits<చిరునామా_Space_Qualifier> {
  స్థిర శూన్య enumeration(IO &YIO, చిరునామా_Space_Qualifier &EN) {
    YIO.enumCase(EN, "Private", చిరునామా_Space_Qualifier::వ్యక్తిగత);
    YIO.enumCase(EN, "Global", చిరునామా_Space_Qualifier::Global);
    YIO.enumCase(EN, "Constant", చిరునామా_Space_Qualifier::Constant);
    YIO.enumCase(EN, "Local", చిరునామా_Space_Qualifier::Local);
    YIO.enumCase(EN, "Generic", చిరునామా_Space_Qualifier::Generic);
    YIO.enumCase(EN, "Region", చిరునామా_Space_Qualifier::Region);
  }
};

మూస <>
నిర్మాణం ScalarEnumerationTraits<విలువ_రకం> {
  స్థిర శూన్య enumeration(IO &YIO, విలువ_రకం &EN) {
    YIO.enumCase(EN, "ByValue", విలువ_రకం::By_విలువ);
    YIO.enumCase(EN, "GlobalBuffer", విలువ_రకం::Global_బఫరు);
    YIO.enumCase(EN, "DynamicSharedPointer", విలువ_రకం::Dynamic_Shared_సూచిక);
    YIO.enumCase(EN, "Sampler", విలువ_రకం::Sampler);
    YIO.enumCase(EN, "Image", విలువ_రకం::Image);
    YIO.enumCase(EN, "Pipe", విలువ_రకం::Pipe);
    YIO.enumCase(EN, "Queue", విలువ_రకం::వరుస);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", విలువ_రకం::Hidden_Global_స్థానభ్రంశం_X);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", విలువ_రకం::Hidden_Global_స్థానభ్రంశం_Y);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", విలువ_రకం::Hidden_Global_స్థానభ్రంశం_Z);
    YIO.enumCase(EN, "HiddenNone", విలువ_రకం::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", విలువ_రకం::Hidden_Printf_బఫరు);
    YIO.enumCase(EN, "HiddenHostcallBuffer", విలువ_రకం::Hidden_Hostcall_బఫరు);
    YIO.enumCase(EN, "HiddenDefaultQueue", విలువ_రకం::Hidden_Default_వరుస);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 విలువ_రకం::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 విలువ_రకం::HiddenMultiGridSyncArg);
  }
};

మూస <>
నిర్మాణం ScalarEnumerationTraits<విలువ_Type> {
  స్థిర శూన్య enumeration(IO &YIO, విలువ_Type &EN) {
    YIO.enumCase(EN, "Struct", విలువ_Type::Struct);
    YIO.enumCase(EN, "I8", విలువ_Type::I8);
    YIO.enumCase(EN, "U8", విలువ_Type::U8);
    YIO.enumCase(EN, "I16", విలువ_Type::I16);
    YIO.enumCase(EN, "U16", విలువ_Type::U16);
    YIO.enumCase(EN, "F16", విలువ_Type::F16);
    YIO.enumCase(EN, "I32", విలువ_Type::I32);
    YIO.enumCase(EN, "U32", విలువ_Type::U32);
    YIO.enumCase(EN, "F32", విలువ_Type::F32);
    YIO.enumCase(EN, "I64", విలువ_Type::I64);
    YIO.enumCase(EN, "U64", విలువ_Type::U64);
    YIO.enumCase(EN, "F64", విలువ_Type::F64);
  }
};

మూస <>
నిర్మాణం MappingTraits<Kernel::Attrs::Metadata> {
  స్థిర శూన్య mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::కీలకం::Reqd_Work_సమూహం_కొలత,
                    MD.m_Reqd_Work_సమూహం_కొలత, std::సదిశ<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::కీలకం::Work_సమూహం_కొలత_Hint,
                    MD.m_Work_సమూహం_కొలత_Hint, std::సదిశ<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::కీలకం::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::కీలకం::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

మూస <>
నిర్మాణం MappingTraits<Kernel::Arg::Metadata> {
  స్థిర శూన్య mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::కీలకం::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::కీలకం::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::కీలకం::కొలత, MD.m_కొలత);
    YIO.mapRequired(Kernel::Arg::కీలకం::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::కీలకం::విలువ_రకం, MD.m_విలువ_రకం);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<విలువ_Type> వాడని;
    YIO.mapOptional(Kernel::Arg::కీలకం::విలువ_Type, వాడని);

    YIO.mapOptional(Kernel::Arg::కీలకం::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::కీలకం::AddrSpaceQual, MD.mAddrSpaceQual,
                    చిరునామా_Space_Qualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::కీలకం::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::కీలకం::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::కీలకం::IsConst, MD.mIsConst, అబద్ధం);
    YIO.mapOptional(Kernel::Arg::కీలకం::IsRestrict, MD.mIsRestrict, అబద్ధం);
    YIO.mapOptional(Kernel::Arg::కీలకం::IsVolatile, MD.mIsVolatile, అబద్ధం);
    YIO.mapOptional(Kernel::Arg::కీలకం::IsPipe, MD.mIsPipe, అబద్ధం);
  }
};

మూస <>
నిర్మాణం MappingTraits<Kernel::సంకేతం_Props::Metadata> {
  స్థిర శూన్య mapping(IO &YIO, Kernel::సంకేతం_Props::Metadata &MD) {
    YIO.mapRequired(Kernel::సంకేతం_Props::కీలకం::Kernarg_Segment_కొలత,
                    MD.m_Kernarg_Segment_కొలత);
    YIO.mapRequired(Kernel::సంకేతం_Props::కీలకం::సమూహం_Segment_Fixed_కొలత,
                    MD.m_సమూహం_Segment_Fixed_కొలత);
    YIO.mapRequired(Kernel::సంకేతం_Props::కీలకం::వ్యక్తిగత_Segment_Fixed_కొలత,
                    MD.m_వ్యక్తిగత_Segment_Fixed_కొలత);
    YIO.mapRequired(Kernel::సంకేతం_Props::కీలకం::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::సంకేతం_Props::కీలకం::Wavefront_కొలత,
                    MD.m_Wavefront_కొలత);
    YIO.mapOptional(Kernel::సంకేతం_Props::కీలకం::అంకె_SGPRs,
                    MD.m_అంకె_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::సంకేతం_Props::కీలకం::అంకె_VGPRs,
                    MD.m_అంకె_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::సంకేతం_Props::కీలకం::Max_Flat_Work_సమూహం_కొలత,
                    MD.m_Max_Flat_Work_సమూహం_కొలత, uint32_t(0));
    YIO.mapOptional(Kernel::సంకేతం_Props::కీలకం::Is_Dynamic_Call_దొంతర,
                    MD.m_Is_Dynamic_Call_దొంతర, అబద్ధం);
    YIO.mapOptional(Kernel::సంకేతం_Props::కీలకం::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, అబద్ధం);
    YIO.mapOptional(Kernel::సంకేతం_Props::కీలకం::అంకె_Spilled_SGPRs,
                    MD.m_అంకె_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::సంకేతం_Props::కీలకం::అంకె_Spilled_VGPRs,
                    MD.m_అంకె_Spilled_VGPRs, uint16_t(0));
  }
};

మూస <>
నిర్మాణం MappingTraits<Kernel::DebugProps::Metadata> {
  స్థిర శూన్య mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::కీలకం::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::సదిశ<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::కీలకం::Reserved_అంకె_VGPRs,
                    MD.m_Reserved_అంకె_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::కీలకం::Reserved_మొదటి_VGPR,
                    MD.m_Reserved_మొదటి_VGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::కీలకం::వ్యక్తిగత_Segment_బఫరు_SGPR,
                    MD.m_వ్యక్తిగత_Segment_బఫరు_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::కీలకం::Wavefront_వ్యక్తిగత_Segment_స్థానభ్రంశం_SGPR,
                    MD.m_Wavefront_వ్యక్తిగత_Segment_స్థానభ్రంశం_SGPR, uint16_t(-1));
  }
};

మూస <>
నిర్మాణం MappingTraits<Kernel::Metadata> {
  స్థిర శూన్య mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::కీలకం::Name, MD.mName);
    YIO.mapRequired(Kernel::కీలకం::చిహ్నం_Name, MD.m_చిహ్నం_Name);
    YIO.mapOptional(Kernel::కీలకం::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::కీలకం::Language_సంస్కరణ, MD.m_Language_సంస్కరణ,
                    std::సదిశ<uint32_t>());
    ఒకవేళ (!MD.mAttrs.ఖాళీలు() || !YIO.outputting())
      YIO.mapOptional(Kernel::కీలకం::Attrs, MD.mAttrs);
    ఒకవేళ (!MD.m_వాదనలు.ఖాళీలు() || !YIO.outputting())
      YIO.mapOptional(Kernel::కీలకం::వాదనలు, MD.m_వాదనలు);
    ఒకవేళ (!MD.m_సంకేతం_Props.ఖాళీలు() || !YIO.outputting())
      YIO.mapOptional(Kernel::కీలకం::సంకేతం_Props, MD.m_సంకేతం_Props);
    ఒకవేళ (!MD.mDebugProps.ఖాళీలు() || !YIO.outputting())
      YIO.mapOptional(Kernel::కీలకం::DebugProps, MD.mDebugProps);
  }
};

మూస <>
నిర్మాణం MappingTraits<HSAMD::Metadata> {
  స్థిర శూన్య mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(కీలకం::సంస్కరణ, MD.m_సంస్కరణ);
    YIO.mapOptional(కీలకం::Printf, MD.mPrintf, std::సదిశ<std::string>());
    ఒకవేళ (!MD.mKernels.ఖాళీలు() || !YIO.outputting())
      YIO.mapOptional(కీలకం::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

నామాకాశ AMDGPU {
నామాకాశ HSAMD {

std::error_సంకేతం fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::ఆదానం Yaml_ఆదానం(String);
  Yaml_ఆదానం >> HSAMetadata;
  తిరిగి Yaml_ఆదానం.error();
}

std::error_సంకేతం గమ్యం_String(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream Yaml_ప్రవాహం(String);
  yaml::Output YamlOutput(Yaml_ప్రవాహం, శూన్యసూచిక, std::numeric_limits<పూర్ణాంకం>::max());
  YamlOutput << HSAMetadata;
  తిరిగి std::error_సంకేతం();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
