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

#അന്തര്ഭാവ "llvm/Support/AMDGPUMetadata.h"
#അന്തര്ഭാവ "llvm/Support/YAMLTraits.h"
#അന്തര്ഭാവ <optional>

using നാമാകാശ llvm::AMDGPU;
using നാമാകാശ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

നാമാകാശ llvm {
നാമാകാശ yaml {

template <>
ഘടന ScalarEnumerationTraits<AccessQualifier> {
  സ്ഥിതം ശൂന്യം enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
ഘടന ScalarEnumerationTraits<AddressSpaceQualifier> {
  സ്ഥിതം ശൂന്യം enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
ഘടന ScalarEnumerationTraits<മൂല്യം_Kind> {
  സ്ഥിതം ശൂന്യം enumeration(IO &YIO, മൂല്യം_Kind &EN) {
    YIO.enumCase(EN, "ByValue", മൂല്യം_Kind::By_മൂല്യം);
    YIO.enumCase(EN, "GlobalBuffer", മൂല്യം_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", മൂല്യം_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", മൂല്യം_Kind::Sampler);
    YIO.enumCase(EN, "Image", മൂല്യം_Kind::Image);
    YIO.enumCase(EN, "Pipe", മൂല്യം_Kind::Pipe);
    YIO.enumCase(EN, "Queue", മൂല്യം_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", മൂല്യം_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", മൂല്യം_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", മൂല്യം_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", മൂല്യം_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", മൂല്യം_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", മൂല്യം_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", മൂല്യം_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 മൂല്യം_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 മൂല്യം_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
ഘടന ScalarEnumerationTraits<മൂല്യം_Type> {
  സ്ഥിതം ശൂന്യം enumeration(IO &YIO, മൂല്യം_Type &EN) {
    YIO.enumCase(EN, "Struct", മൂല്യം_Type::Struct);
    YIO.enumCase(EN, "I8", മൂല്യം_Type::I8);
    YIO.enumCase(EN, "U8", മൂല്യം_Type::U8);
    YIO.enumCase(EN, "I16", മൂല്യം_Type::I16);
    YIO.enumCase(EN, "U16", മൂല്യം_Type::U16);
    YIO.enumCase(EN, "F16", മൂല്യം_Type::F16);
    YIO.enumCase(EN, "I32", മൂല്യം_Type::I32);
    YIO.enumCase(EN, "U32", മൂല്യം_Type::U32);
    YIO.enumCase(EN, "F32", മൂല്യം_Type::F32);
    YIO.enumCase(EN, "I64", മൂല്യം_Type::I64);
    YIO.enumCase(EN, "U64", മൂല്യം_Type::U64);
    YIO.enumCase(EN, "F64", മൂല്യം_Type::F64);
  }
};

template <>
ഘടന MappingTraits<Kernel::Attrs::Metadata> {
  സ്ഥിതം ശൂന്യം mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::താക്കോൽ::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::താക്കോൽ::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::താക്കോൽ::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::താക്കോൽ::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
ഘടന MappingTraits<Kernel::Arg::Metadata> {
  സ്ഥിതം ശൂന്യം mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::താക്കോൽ::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::താക്കോൽ::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::താക്കോൽ::മൂല്യം_Kind, MD.m_മൂല്യം_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<മൂല്യം_Type> Unused;
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::മൂല്യം_Type, Unused);

    YIO.mapOptional(Kernel::Arg::താക്കോൽ::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::IsConst, MD.mIsConst, അസത്യം);
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::IsRestrict, MD.mIsRestrict, അസത്യം);
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::IsVolatile, MD.mIsVolatile, അസത്യം);
    YIO.mapOptional(Kernel::Arg::താക്കോൽ::IsPipe, MD.mIsPipe, അസത്യം);
  }
};

template <>
ഘടന MappingTraits<Kernel::CodeProps::Metadata> {
  സ്ഥിതം ശൂന്യം mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::താക്കോൽ::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::താക്കോൽ::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::താക്കോൽ::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::താക്കോൽ::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::താക്കോൽ::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::താക്കോൽ::അക്കം_SGPRs,
                    MD.m_അക്കം_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::താക്കോൽ::അക്കം_VGPRs,
                    MD.m_അക്കം_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::താക്കോൽ::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::താക്കോൽ::Is_Dynamic_Call_അട്ടി,
                    MD.m_Is_Dynamic_Call_അട്ടി, അസത്യം);
    YIO.mapOptional(Kernel::CodeProps::താക്കോൽ::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, അസത്യം);
    YIO.mapOptional(Kernel::CodeProps::താക്കോൽ::അക്കം_Spilled_SGPRs,
                    MD.m_അക്കം_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::താക്കോൽ::അക്കം_Spilled_VGPRs,
                    MD.m_അക്കം_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
ഘടന MappingTraits<Kernel::DebugProps::Metadata> {
  സ്ഥിതം ശൂന്യം mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::താക്കോൽ::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::താക്കോൽ::Reserved_അക്കം_VGPRs,
                    MD.m_Reserved_അക്കം_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::താക്കോൽ::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::താക്കോൽ::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::താക്കോൽ::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
ഘടന MappingTraits<Kernel::Metadata> {
  സ്ഥിതം ശൂന്യം mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::താക്കോൽ::Name, MD.mName);
    YIO.mapRequired(Kernel::താക്കോൽ::ചിഹ്നം_Name, MD.m_ചിഹ്നം_Name);
    YIO.mapOptional(Kernel::താക്കോൽ::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::താക്കോൽ::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    എങ്കിൽ (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::താക്കോൽ::Attrs, MD.mAttrs);
    എങ്കിൽ (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::താക്കോൽ::Args, MD.mArgs);
    എങ്കിൽ (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::താക്കോൽ::CodeProps, MD.mCodeProps);
    എങ്കിൽ (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::താക്കോൽ::DebugProps, MD.mDebugProps);
  }
};

template <>
ഘടന MappingTraits<HSAMD::Metadata> {
  സ്ഥിതം ശൂന്യം mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(താക്കോൽ::Version, MD.mVersion);
    YIO.mapOptional(താക്കോൽ::Printf, MD.mPrintf, std::vector<std::string>());
    എങ്കിൽ (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(താക്കോൽ::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

നാമാകാശ AMDGPU {
നാമാകാശ HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::നിവേശം Yaml_നിവേശം(String);
  Yaml_നിവേശം >> HSAMetadata;
  തിരികെ Yaml_നിവേശം.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<പൂർണ്ണസംഖ്യ>::max());
  YamlOutput << HSAMetadata;
  തിരികെ std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
