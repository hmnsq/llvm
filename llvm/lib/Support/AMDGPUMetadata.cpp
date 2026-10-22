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

using नामाकाश llvm::AMDGPU;
using नामाकाश llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

नामाकाश llvm {
नामाकाश yaml {

template <>
संरचना ScalarEnumerationTraits<AccessQualifier> {
  स्थिर शून्य enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
संरचना ScalarEnumerationTraits<AddressSpaceQualifier> {
  स्थिर शून्य enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
संरचना ScalarEnumerationTraits<मानम्_Kind> {
  स्थिर शून्य enumeration(IO &YIO, मानम्_Kind &EN) {
    YIO.enumCase(EN, "ByValue", मानम्_Kind::By_मानम्);
    YIO.enumCase(EN, "GlobalBuffer", मानम्_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", मानम्_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", मानम्_Kind::Sampler);
    YIO.enumCase(EN, "Image", मानम्_Kind::Image);
    YIO.enumCase(EN, "Pipe", मानम्_Kind::Pipe);
    YIO.enumCase(EN, "Queue", मानम्_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", मानम्_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", मानम्_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", मानम्_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", मानम्_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", मानम्_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", मानम्_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", मानम्_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 मानम्_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 मानम्_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
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

template <>
संरचना MappingTraits<Kernel::Attrs::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::कुञ्चिका::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::कुञ्चिका::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::कुञ्चिका::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::कुञ्चिका::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
संरचना MappingTraits<Kernel::Arg::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::कुञ्चिका::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::कुञ्चिका::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::कुञ्चिका::मानम्_Kind, MD.m_मानम्_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<मानम्_Type> Unused;
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::मानम्_Type, Unused);

    YIO.mapOptional(Kernel::Arg::कुञ्चिका::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::IsConst, MD.mIsConst, मिच्छा);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::IsRestrict, MD.mIsRestrict, मिच्छा);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::IsVolatile, MD.mIsVolatile, मिच्छा);
    YIO.mapOptional(Kernel::Arg::कुञ्चिका::IsPipe, MD.mIsPipe, मिच्छा);
  }
};

template <>
संरचना MappingTraits<Kernel::CodeProps::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::कुञ्चिका::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::कुञ्चिका::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::कुञ्चिका::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::कुञ्चिका::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::कुञ्चिका::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::कुञ्चिका::अङ्कः_SGPRs,
                    MD.m_अङ्कः_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::कुञ्चिका::अङ्कः_VGPRs,
                    MD.m_अङ्कः_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::कुञ्चिका::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::कुञ्चिका::Is_Dynamic_Call_राशिः,
                    MD.m_Is_Dynamic_Call_राशिः, मिच्छा);
    YIO.mapOptional(Kernel::CodeProps::कुञ्चिका::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, मिच्छा);
    YIO.mapOptional(Kernel::CodeProps::कुञ्चिका::अङ्कः_Spilled_SGPRs,
                    MD.m_अङ्कः_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::कुञ्चिका::अङ्कः_Spilled_VGPRs,
                    MD.m_अङ्कः_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
संरचना MappingTraits<Kernel::DebugProps::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::Reserved_अङ्कः_VGPRs,
                    MD.m_Reserved_अङ्कः_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::कुञ्चिका::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
संरचना MappingTraits<Kernel::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::कुञ्चिका::Name, MD.mName);
    YIO.mapRequired(Kernel::कुञ्चिका::चिह्नम्_Name, MD.m_चिह्नम्_Name);
    YIO.mapOptional(Kernel::कुञ्चिका::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::कुञ्चिका::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    सचे (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुञ्चिका::Attrs, MD.mAttrs);
    सचे (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुञ्चिका::Args, MD.mArgs);
    सचे (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुञ्चिका::CodeProps, MD.mCodeProps);
    सचे (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुञ्चिका::DebugProps, MD.mDebugProps);
  }
};

template <>
संरचना MappingTraits<HSAMD::Metadata> {
  स्थिर शून्य mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(कुञ्चिका::Version, MD.mVersion);
    YIO.mapOptional(कुञ्चिका::Printf, MD.mPrintf, std::vector<std::string>());
    सचे (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(कुञ्चिका::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

नामाकाश AMDGPU {
नामाकाश HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::निवेशः Yaml_निवेशः(String);
  Yaml_निवेशः >> HSAMetadata;
  पटिनिवत्त Yaml_निवेशः.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<पूर्णाङ्क>::max());
  YamlOutput << HSAMetadata;
  पटिनिवत्त std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
