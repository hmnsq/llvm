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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AMDGPUMetadata.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/YAMLTraits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::AMDGPU;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_ਸਦਿਸ਼(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_ਸਦਿਸ਼(Kernel::Metadata)

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ yaml {

ਸਾਂਚਾ <>
ਬਣਤਰ ScalarEnumerationTraits<AccessQualifier> {
  ਸਥਿਰ ਖਾਲੀ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ ScalarEnumerationTraits<ਪਤਾ_Space_Qualifier> {
  ਸਥਿਰ ਖਾਲੀ enumeration(IO &YIO, ਪਤਾ_Space_Qualifier &EN) {
    YIO.enumCase(EN, "Private", ਪਤਾ_Space_Qualifier::ਨਿੱਜੀ);
    YIO.enumCase(EN, "Global", ਪਤਾ_Space_Qualifier::Global);
    YIO.enumCase(EN, "Constant", ਪਤਾ_Space_Qualifier::Constant);
    YIO.enumCase(EN, "Local", ਪਤਾ_Space_Qualifier::Local);
    YIO.enumCase(EN, "Generic", ਪਤਾ_Space_Qualifier::Generic);
    YIO.enumCase(EN, "Region", ਪਤਾ_Space_Qualifier::Region);
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ ScalarEnumerationTraits<ਮੁੱਲ_ਕਿਸਮ> {
  ਸਥਿਰ ਖਾਲੀ enumeration(IO &YIO, ਮੁੱਲ_ਕਿਸਮ &EN) {
    YIO.enumCase(EN, "ByValue", ਮੁੱਲ_ਕਿਸਮ::By_ਮੁੱਲ);
    YIO.enumCase(EN, "GlobalBuffer", ਮੁੱਲ_ਕਿਸਮ::Global_ਬਫਰ);
    YIO.enumCase(EN, "DynamicSharedPointer", ਮੁੱਲ_ਕਿਸਮ::Dynamic_Shared_ਸੂਚਕ);
    YIO.enumCase(EN, "Sampler", ਮੁੱਲ_ਕਿਸਮ::Sampler);
    YIO.enumCase(EN, "Image", ਮੁੱਲ_ਕਿਸਮ::Image);
    YIO.enumCase(EN, "Pipe", ਮੁੱਲ_ਕਿਸਮ::Pipe);
    YIO.enumCase(EN, "Queue", ਮੁੱਲ_ਕਿਸਮ::ਕਤਾਰ);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", ਮੁੱਲ_ਕਿਸਮ::Hidden_Global_ਵਿੱਥ_X);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", ਮੁੱਲ_ਕਿਸਮ::Hidden_Global_ਵਿੱਥ_Y);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", ਮੁੱਲ_ਕਿਸਮ::Hidden_Global_ਵਿੱਥ_Z);
    YIO.enumCase(EN, "HiddenNone", ਮੁੱਲ_ਕਿਸਮ::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", ਮੁੱਲ_ਕਿਸਮ::Hidden_Printf_ਬਫਰ);
    YIO.enumCase(EN, "HiddenHostcallBuffer", ਮੁੱਲ_ਕਿਸਮ::Hidden_Hostcall_ਬਫਰ);
    YIO.enumCase(EN, "HiddenDefaultQueue", ਮੁੱਲ_ਕਿਸਮ::Hidden_Default_ਕਤਾਰ);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 ਮੁੱਲ_ਕਿਸਮ::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 ਮੁੱਲ_ਕਿਸਮ::HiddenMultiGridSyncArg);
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ ScalarEnumerationTraits<ਮੁੱਲ_Type> {
  ਸਥਿਰ ਖਾਲੀ enumeration(IO &YIO, ਮੁੱਲ_Type &EN) {
    YIO.enumCase(EN, "Struct", ਮੁੱਲ_Type::Struct);
    YIO.enumCase(EN, "I8", ਮੁੱਲ_Type::I8);
    YIO.enumCase(EN, "U8", ਮੁੱਲ_Type::U8);
    YIO.enumCase(EN, "I16", ਮੁੱਲ_Type::I16);
    YIO.enumCase(EN, "U16", ਮੁੱਲ_Type::U16);
    YIO.enumCase(EN, "F16", ਮੁੱਲ_Type::F16);
    YIO.enumCase(EN, "I32", ਮੁੱਲ_Type::I32);
    YIO.enumCase(EN, "U32", ਮੁੱਲ_Type::U32);
    YIO.enumCase(EN, "F32", ਮੁੱਲ_Type::F32);
    YIO.enumCase(EN, "I64", ਮੁੱਲ_Type::I64);
    YIO.enumCase(EN, "U64", ਮੁੱਲ_Type::U64);
    YIO.enumCase(EN, "F64", ਮੁੱਲ_Type::F64);
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ MappingTraits<Kernel::Attrs::Metadata> {
  ਸਥਿਰ ਖਾਲੀ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::ਕੁੰਜੀ::Reqd_Work_ਸਮੂਹ_ਮਾਪ,
                    MD.m_Reqd_Work_ਸਮੂਹ_ਮਾਪ, std::ਸਦਿਸ਼<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ਕੁੰਜੀ::Work_ਸਮੂਹ_ਮਾਪ_Hint,
                    MD.m_Work_ਸਮੂਹ_ਮਾਪ_Hint, std::ਸਦਿਸ਼<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ਕੁੰਜੀ::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::ਕੁੰਜੀ::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ MappingTraits<Kernel::Arg::Metadata> {
  ਸਥਿਰ ਖਾਲੀ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::ਕੁੰਜੀ::ਮਾਪ, MD.m_ਮਾਪ);
    YIO.mapRequired(Kernel::Arg::ਕੁੰਜੀ::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::ਕੁੰਜੀ::ਮੁੱਲ_ਕਿਸਮ, MD.m_ਮੁੱਲ_ਕਿਸਮ);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<ਮੁੱਲ_Type> ਅਣਵਰਤਿਆ;
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::ਮੁੱਲ_Type, ਅਣਵਰਤਿਆ);

    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::AddrSpaceQual, MD.mAddrSpaceQual,
                    ਪਤਾ_Space_Qualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::IsConst, MD.mIsConst, ਝੂਠ);
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::IsRestrict, MD.mIsRestrict, ਝੂਠ);
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::IsVolatile, MD.mIsVolatile, ਝੂਠ);
    YIO.mapOptional(Kernel::Arg::ਕੁੰਜੀ::IsPipe, MD.mIsPipe, ਝੂਠ);
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ MappingTraits<Kernel::ਕੋਡ_Props::Metadata> {
  ਸਥਿਰ ਖਾਲੀ mapping(IO &YIO, Kernel::ਕੋਡ_Props::Metadata &MD) {
    YIO.mapRequired(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::Kernarg_Segment_ਮਾਪ,
                    MD.m_Kernarg_Segment_ਮਾਪ);
    YIO.mapRequired(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::ਸਮੂਹ_Segment_Fixed_ਮਾਪ,
                    MD.m_ਸਮੂਹ_Segment_Fixed_ਮਾਪ);
    YIO.mapRequired(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::ਨਿੱਜੀ_Segment_Fixed_ਮਾਪ,
                    MD.m_ਨਿੱਜੀ_Segment_Fixed_ਮਾਪ);
    YIO.mapRequired(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::Wavefront_ਮਾਪ,
                    MD.m_Wavefront_ਮਾਪ);
    YIO.mapOptional(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::ਅੰਕ_SGPRs,
                    MD.m_ਅੰਕ_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::ਅੰਕ_VGPRs,
                    MD.m_ਅੰਕ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::Max_Flat_Work_ਸਮੂਹ_ਮਾਪ,
                    MD.m_Max_Flat_Work_ਸਮੂਹ_ਮਾਪ, uint32_t(0));
    YIO.mapOptional(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::Is_Dynamic_Call_ਢੇਰ,
                    MD.m_Is_Dynamic_Call_ਢੇਰ, ਝੂਠ);
    YIO.mapOptional(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, ਝੂਠ);
    YIO.mapOptional(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::ਅੰਕ_Spilled_SGPRs,
                    MD.m_ਅੰਕ_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ਕੋਡ_Props::ਕੁੰਜੀ::ਅੰਕ_Spilled_VGPRs,
                    MD.m_ਅੰਕ_Spilled_VGPRs, uint16_t(0));
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ MappingTraits<Kernel::DebugProps::Metadata> {
  ਸਥਿਰ ਖਾਲੀ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::ਕੁੰਜੀ::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::ਸਦਿਸ਼<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::ਕੁੰਜੀ::Reserved_ਅੰਕ_VGPRs,
                    MD.m_Reserved_ਅੰਕ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::ਕੁੰਜੀ::Reserved_ਪਹਿਲਾ_VGPR,
                    MD.m_Reserved_ਪਹਿਲਾ_VGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ਕੁੰਜੀ::ਨਿੱਜੀ_Segment_ਬਫਰ_SGPR,
                    MD.m_ਨਿੱਜੀ_Segment_ਬਫਰ_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ਕੁੰਜੀ::Wavefront_ਨਿੱਜੀ_Segment_ਵਿੱਥ_SGPR,
                    MD.m_Wavefront_ਨਿੱਜੀ_Segment_ਵਿੱਥ_SGPR, uint16_t(-1));
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ MappingTraits<Kernel::Metadata> {
  ਸਥਿਰ ਖਾਲੀ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::ਕੁੰਜੀ::Name, MD.mName);
    YIO.mapRequired(Kernel::ਕੁੰਜੀ::ਚਿੰਨ੍ਹ_Name, MD.m_ਚਿੰਨ੍ਹ_Name);
    YIO.mapOptional(Kernel::ਕੁੰਜੀ::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::ਕੁੰਜੀ::Language_ਸੰਸਕਰਣ, MD.m_Language_ਸੰਸਕਰਣ,
                    std::ਸਦਿਸ਼<uint32_t>());
    ਜੇ (!MD.mAttrs.ਖਾਲੀ_ਥਾਵਾਂ() || !YIO.outputting())
      YIO.mapOptional(Kernel::ਕੁੰਜੀ::Attrs, MD.mAttrs);
    ਜੇ (!MD.m_ਦਲੀਲਾਂ.ਖਾਲੀ_ਥਾਵਾਂ() || !YIO.outputting())
      YIO.mapOptional(Kernel::ਕੁੰਜੀ::ਦਲੀਲਾਂ, MD.m_ਦਲੀਲਾਂ);
    ਜੇ (!MD.m_ਕੋਡ_Props.ਖਾਲੀ_ਥਾਵਾਂ() || !YIO.outputting())
      YIO.mapOptional(Kernel::ਕੁੰਜੀ::ਕੋਡ_Props, MD.m_ਕੋਡ_Props);
    ਜੇ (!MD.mDebugProps.ਖਾਲੀ_ਥਾਵਾਂ() || !YIO.outputting())
      YIO.mapOptional(Kernel::ਕੁੰਜੀ::DebugProps, MD.mDebugProps);
  }
};

ਸਾਂਚਾ <>
ਬਣਤਰ MappingTraits<HSAMD::Metadata> {
  ਸਥਿਰ ਖਾਲੀ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(ਕੁੰਜੀ::ਸੰਸਕਰਣ, MD.m_ਸੰਸਕਰਣ);
    YIO.mapOptional(ਕੁੰਜੀ::Printf, MD.mPrintf, std::ਸਦਿਸ਼<std::string>());
    ਜੇ (!MD.mKernels.ਖਾਲੀ_ਥਾਵਾਂ() || !YIO.outputting())
      YIO.mapOptional(ਕੁੰਜੀ::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

ਨਾਮਾਕਾਸ਼ AMDGPU {
ਨਾਮਾਕਾਸ਼ HSAMD {

std::error_ਕੋਡ fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::ਨਿਵੇਸ਼ Yaml_ਨਿਵੇਸ਼(String);
  Yaml_ਨਿਵੇਸ਼ >> HSAMetadata;
  ਵਾਪਸ Yaml_ਨਿਵੇਸ਼.error();
}

std::error_ਕੋਡ ਮੰਜ਼ਿਲ_String(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream Yaml_ਧਾਰਾ(String);
  yaml::Output YamlOutput(Yaml_ਧਾਰਾ, ਖਾਲੀ_ਸੂਚਕ, std::numeric_limits<ਪੂਰਨ>::max());
  YamlOutput << HSAMetadata;
  ਵਾਪਸ std::error_ਕੋਡ();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
