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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/AMDGPUMetadata.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/YAMLTraits.h"
#ଅନ୍ତର୍ଭାଵ <optional>

using ନାମାକାଶ llvm::AMDGPU;
using ନାମାକାଶ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

ନାମାକାଶ llvm {
ନାମାକାଶ yaml {

template <>
ସଂରଚନା ScalarEnumerationTraits<AccessQualifier> {
  ସ୍ଥିର ଶୂନ୍ୟ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
ସଂରଚନା ScalarEnumerationTraits<AddressSpaceQualifier> {
  ସ୍ଥିର ଶୂନ୍ୟ enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
ସଂରଚନା ScalarEnumerationTraits<ମୂଲ୍ୟ_Kind> {
  ସ୍ଥିର ଶୂନ୍ୟ enumeration(IO &YIO, ମୂଲ୍ୟ_Kind &EN) {
    YIO.enumCase(EN, "ByValue", ମୂଲ୍ୟ_Kind::By_ମୂଲ୍ୟ);
    YIO.enumCase(EN, "GlobalBuffer", ମୂଲ୍ୟ_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", ମୂଲ୍ୟ_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", ମୂଲ୍ୟ_Kind::Sampler);
    YIO.enumCase(EN, "Image", ମୂଲ୍ୟ_Kind::Image);
    YIO.enumCase(EN, "Pipe", ମୂଲ୍ୟ_Kind::Pipe);
    YIO.enumCase(EN, "Queue", ମୂଲ୍ୟ_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", ମୂଲ୍ୟ_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", ମୂଲ୍ୟ_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", ମୂଲ୍ୟ_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", ମୂଲ୍ୟ_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", ମୂଲ୍ୟ_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", ମୂଲ୍ୟ_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", ମୂଲ୍ୟ_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 ମୂଲ୍ୟ_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 ମୂଲ୍ୟ_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
ସଂରଚନା ScalarEnumerationTraits<ମୂଲ୍ୟ_Type> {
  ସ୍ଥିର ଶୂନ୍ୟ enumeration(IO &YIO, ମୂଲ୍ୟ_Type &EN) {
    YIO.enumCase(EN, "Struct", ମୂଲ୍ୟ_Type::Struct);
    YIO.enumCase(EN, "I8", ମୂଲ୍ୟ_Type::I8);
    YIO.enumCase(EN, "U8", ମୂଲ୍ୟ_Type::U8);
    YIO.enumCase(EN, "I16", ମୂଲ୍ୟ_Type::I16);
    YIO.enumCase(EN, "U16", ମୂଲ୍ୟ_Type::U16);
    YIO.enumCase(EN, "F16", ମୂଲ୍ୟ_Type::F16);
    YIO.enumCase(EN, "I32", ମୂଲ୍ୟ_Type::I32);
    YIO.enumCase(EN, "U32", ମୂଲ୍ୟ_Type::U32);
    YIO.enumCase(EN, "F32", ମୂଲ୍ୟ_Type::F32);
    YIO.enumCase(EN, "I64", ମୂଲ୍ୟ_Type::I64);
    YIO.enumCase(EN, "U64", ମୂଲ୍ୟ_Type::U64);
    YIO.enumCase(EN, "F64", ମୂଲ୍ୟ_Type::F64);
  }
};

template <>
ସଂରଚନା MappingTraits<Kernel::Attrs::Metadata> {
  ସ୍ଥିର ଶୂନ୍ୟ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::ଚାବି::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ଚାବି::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ଚାବି::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::ଚାବି::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
ସଂରଚନା MappingTraits<Kernel::Arg::Metadata> {
  ସ୍ଥିର ଶୂନ୍ୟ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::ଚାବି::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::ଚାବି::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::ଚାବି::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::ଚାବି::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::ଚାବି::ମୂଲ୍ୟ_Kind, MD.m_ମୂଲ୍ୟ_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<ମୂଲ୍ୟ_Type> Unused;
    YIO.mapOptional(Kernel::Arg::ଚାବି::ମୂଲ୍ୟ_Type, Unused);

    YIO.mapOptional(Kernel::Arg::ଚାବି::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::ଚାବି::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ଚାବି::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ଚାବି::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ଚାବି::IsConst, MD.mIsConst, ମିଥ୍ୟା);
    YIO.mapOptional(Kernel::Arg::ଚାବି::IsRestrict, MD.mIsRestrict, ମିଥ୍ୟା);
    YIO.mapOptional(Kernel::Arg::ଚାବି::IsVolatile, MD.mIsVolatile, ମିଥ୍ୟା);
    YIO.mapOptional(Kernel::Arg::ଚାବି::IsPipe, MD.mIsPipe, ମିଥ୍ୟା);
  }
};

template <>
ସଂରଚନା MappingTraits<Kernel::CodeProps::Metadata> {
  ସ୍ଥିର ଶୂନ୍ୟ mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::ଚାବି::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::ଚାବି::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::ଚାବି::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::ଚାବି::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::ଚାବି::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::ଚାବି::ଅଙ୍କ_SGPRs,
                    MD.m_ଅଙ୍କ_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ଚାବି::ଅଙ୍କ_VGPRs,
                    MD.m_ଅଙ୍କ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ଚାବି::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::ଚାବି::Is_Dynamic_Call_ଗଦା,
                    MD.m_Is_Dynamic_Call_ଗଦା, ମିଥ୍ୟା);
    YIO.mapOptional(Kernel::CodeProps::ଚାବି::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, ମିଥ୍ୟା);
    YIO.mapOptional(Kernel::CodeProps::ଚାବି::ଅଙ୍କ_Spilled_SGPRs,
                    MD.m_ଅଙ୍କ_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ଚାବି::ଅଙ୍କ_Spilled_VGPRs,
                    MD.m_ଅଙ୍କ_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
ସଂରଚନା MappingTraits<Kernel::DebugProps::Metadata> {
  ସ୍ଥିର ଶୂନ୍ୟ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::ଚାବି::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::ଚାବି::Reserved_ଅଙ୍କ_VGPRs,
                    MD.m_Reserved_ଅଙ୍କ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::ଚାବି::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ଚାବି::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ଚାବି::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
ସଂରଚନା MappingTraits<Kernel::Metadata> {
  ସ୍ଥିର ଶୂନ୍ୟ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::ଚାବି::Name, MD.mName);
    YIO.mapRequired(Kernel::ଚାବି::ପ୍ରତୀକ_Name, MD.m_ପ୍ରତୀକ_Name);
    YIO.mapOptional(Kernel::ଚାବି::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::ଚାବି::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    ଯଦି (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ଚାବି::Attrs, MD.mAttrs);
    ଯଦି (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ଚାବି::Args, MD.mArgs);
    ଯଦି (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ଚାବି::CodeProps, MD.mCodeProps);
    ଯଦି (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ଚାବି::DebugProps, MD.mDebugProps);
  }
};

template <>
ସଂରଚନା MappingTraits<HSAMD::Metadata> {
  ସ୍ଥିର ଶୂନ୍ୟ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(ଚାବି::Version, MD.mVersion);
    YIO.mapOptional(ଚାବି::Printf, MD.mPrintf, std::vector<std::string>());
    ଯଦି (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(ଚାବି::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

ନାମାକାଶ AMDGPU {
ନାମାକାଶ HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::ନିବେଶ Yaml_ନିବେଶ(String);
  Yaml_ନିବେଶ >> HSAMetadata;
  ଫେରନ୍ତୁ Yaml_ନିବେଶ.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>::max());
  YamlOutput << HSAMetadata;
  ଫେରନ୍ତୁ std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
