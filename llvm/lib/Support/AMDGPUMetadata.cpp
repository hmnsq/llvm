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

#ಅನ್ತರ್ಭಾವ "llvm/Support/AMDGPUMetadata.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/YAMLTraits.h"
#ಅನ್ತರ್ಭಾವ <optional>

using ನಾಮಾಕಾಶ llvm::AMDGPU;
using ನಾಮಾಕಾಶ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ yaml {

template <>
ಸಂರಚನಾ ScalarEnumerationTraits<AccessQualifier> {
  ಸ್ಥಿರ ಶೂನ್ಯ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
ಸಂರಚನಾ ScalarEnumerationTraits<AddressSpaceQualifier> {
  ಸ್ಥಿರ ಶೂನ್ಯ enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
ಸಂರಚನಾ ScalarEnumerationTraits<ಮೌಲ್ಯ_Kind> {
  ಸ್ಥಿರ ಶೂನ್ಯ enumeration(IO &YIO, ಮೌಲ್ಯ_Kind &EN) {
    YIO.enumCase(EN, "ByValue", ಮೌಲ್ಯ_Kind::By_ಮೌಲ್ಯ);
    YIO.enumCase(EN, "GlobalBuffer", ಮೌಲ್ಯ_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", ಮೌಲ್ಯ_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", ಮೌಲ್ಯ_Kind::Sampler);
    YIO.enumCase(EN, "Image", ಮೌಲ್ಯ_Kind::Image);
    YIO.enumCase(EN, "Pipe", ಮೌಲ್ಯ_Kind::Pipe);
    YIO.enumCase(EN, "Queue", ಮೌಲ್ಯ_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", ಮೌಲ್ಯ_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", ಮೌಲ್ಯ_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", ಮೌಲ್ಯ_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", ಮೌಲ್ಯ_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", ಮೌಲ್ಯ_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", ಮೌಲ್ಯ_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", ಮೌಲ್ಯ_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 ಮೌಲ್ಯ_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 ಮೌಲ್ಯ_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
ಸಂರಚನಾ ScalarEnumerationTraits<ಮೌಲ್ಯ_Type> {
  ಸ್ಥಿರ ಶೂನ್ಯ enumeration(IO &YIO, ಮೌಲ್ಯ_Type &EN) {
    YIO.enumCase(EN, "Struct", ಮೌಲ್ಯ_Type::Struct);
    YIO.enumCase(EN, "I8", ಮೌಲ್ಯ_Type::I8);
    YIO.enumCase(EN, "U8", ಮೌಲ್ಯ_Type::U8);
    YIO.enumCase(EN, "I16", ಮೌಲ್ಯ_Type::I16);
    YIO.enumCase(EN, "U16", ಮೌಲ್ಯ_Type::U16);
    YIO.enumCase(EN, "F16", ಮೌಲ್ಯ_Type::F16);
    YIO.enumCase(EN, "I32", ಮೌಲ್ಯ_Type::I32);
    YIO.enumCase(EN, "U32", ಮೌಲ್ಯ_Type::U32);
    YIO.enumCase(EN, "F32", ಮೌಲ್ಯ_Type::F32);
    YIO.enumCase(EN, "I64", ಮೌಲ್ಯ_Type::I64);
    YIO.enumCase(EN, "U64", ಮೌಲ್ಯ_Type::U64);
    YIO.enumCase(EN, "F64", ಮೌಲ್ಯ_Type::F64);
  }
};

template <>
ಸಂರಚನಾ MappingTraits<Kernel::Attrs::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::ಕೀಲಿ::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ಕೀಲಿ::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ಕೀಲಿ::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::ಕೀಲಿ::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
ಸಂರಚನಾ MappingTraits<Kernel::Arg::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::ಕೀಲಿ::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::ಕೀಲಿ::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::ಕೀಲಿ::ಮೌಲ್ಯ_Kind, MD.m_ಮೌಲ್ಯ_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<ಮೌಲ್ಯ_Type> Unused;
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::ಮೌಲ್ಯ_Type, Unused);

    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::IsConst, MD.mIsConst, ಅಸತ್ಯ);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::IsRestrict, MD.mIsRestrict, ಅಸತ್ಯ);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::IsVolatile, MD.mIsVolatile, ಅಸತ್ಯ);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::IsPipe, MD.mIsPipe, ಅಸತ್ಯ);
  }
};

template <>
ಸಂರಚನಾ MappingTraits<Kernel::CodeProps::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::ಕೀಲಿ::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::ಕೀಲಿ::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::ಕೀಲಿ::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::ಕೀಲಿ::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::ಕೀಲಿ::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::ಕೀಲಿ::ಅಂಕೆ_SGPRs,
                    MD.m_ಅಂಕೆ_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ಕೀಲಿ::ಅಂಕೆ_VGPRs,
                    MD.m_ಅಂಕೆ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ಕೀಲಿ::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::ಕೀಲಿ::Is_Dynamic_Call_ರಾಶಿ,
                    MD.m_Is_Dynamic_Call_ರಾಶಿ, ಅಸತ್ಯ);
    YIO.mapOptional(Kernel::CodeProps::ಕೀಲಿ::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, ಅಸತ್ಯ);
    YIO.mapOptional(Kernel::CodeProps::ಕೀಲಿ::ಅಂಕೆ_Spilled_SGPRs,
                    MD.m_ಅಂಕೆ_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ಕೀಲಿ::ಅಂಕೆ_Spilled_VGPRs,
                    MD.m_ಅಂಕೆ_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
ಸಂರಚನಾ MappingTraits<Kernel::DebugProps::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::Reserved_ಅಂಕೆ_VGPRs,
                    MD.m_Reserved_ಅಂಕೆ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
ಸಂರಚನಾ MappingTraits<Kernel::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::ಕೀಲಿ::Name, MD.mName);
    YIO.mapRequired(Kernel::ಕೀಲಿ::ಚಿಹ್ನೆ_Name, MD.m_ಚಿಹ್ನೆ_Name);
    YIO.mapOptional(Kernel::ಕೀಲಿ::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::ಕೀಲಿ::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    ಯದಿ (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ಕೀಲಿ::Attrs, MD.mAttrs);
    ಯದಿ (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ಕೀಲಿ::Args, MD.mArgs);
    ಯದಿ (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ಕೀಲಿ::CodeProps, MD.mCodeProps);
    ಯದಿ (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ಕೀಲಿ::DebugProps, MD.mDebugProps);
  }
};

template <>
ಸಂರಚನಾ MappingTraits<HSAMD::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(ಕೀಲಿ::Version, MD.mVersion);
    YIO.mapOptional(ಕೀಲಿ::Printf, MD.mPrintf, std::vector<std::string>());
    ಯದಿ (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(ಕೀಲಿ::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

ನಾಮಾಕಾಶ AMDGPU {
ನಾಮಾಕಾಶ HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::ಒಳಹರಿವು Yaml_ಒಳಹರಿವು(String);
  Yaml_ಒಳಹರಿವು >> HSAMetadata;
  ಪ್ರತ್ಯಾಗಮ Yaml_ಒಳಹರಿವು.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<ಪೂರ್ಣಾಙ್ಕ>::max());
  YamlOutput << HSAMetadata;
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
