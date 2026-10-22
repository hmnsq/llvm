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

ढाँचा <>
संरचना ScalarEnumerationTraits<AccessQualifier> {
  स्थिर शून्य enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

ढाँचा <>
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

ढाँचा <>
संरचना ScalarEnumerationTraits<मान_Kind> {
  स्थिर शून्य enumeration(IO &YIO, मान_Kind &EN) {
    YIO.enumCase(EN, "ByValue", मान_Kind::By_मान);
    YIO.enumCase(EN, "GlobalBuffer", मान_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", मान_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", मान_Kind::Sampler);
    YIO.enumCase(EN, "Image", मान_Kind::Image);
    YIO.enumCase(EN, "Pipe", मान_Kind::Pipe);
    YIO.enumCase(EN, "Queue", मान_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", मान_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", मान_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", मान_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", मान_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", मान_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", मान_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", मान_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 मान_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 मान_Kind::HiddenMultiGridSyncArg);
  }
};

ढाँचा <>
संरचना ScalarEnumerationTraits<मान_Type> {
  स्थिर शून्य enumeration(IO &YIO, मान_Type &EN) {
    YIO.enumCase(EN, "Struct", मान_Type::Struct);
    YIO.enumCase(EN, "I8", मान_Type::I8);
    YIO.enumCase(EN, "U8", मान_Type::U8);
    YIO.enumCase(EN, "I16", मान_Type::I16);
    YIO.enumCase(EN, "U16", मान_Type::U16);
    YIO.enumCase(EN, "F16", मान_Type::F16);
    YIO.enumCase(EN, "I32", मान_Type::I32);
    YIO.enumCase(EN, "U32", मान_Type::U32);
    YIO.enumCase(EN, "F32", मान_Type::F32);
    YIO.enumCase(EN, "I64", मान_Type::I64);
    YIO.enumCase(EN, "U64", मान_Type::U64);
    YIO.enumCase(EN, "F64", मान_Type::F64);
  }
};

ढाँचा <>
संरचना MappingTraits<Kernel::Attrs::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::साँचो::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::साँचो::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::साँचो::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::साँचो::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

ढाँचा <>
संरचना MappingTraits<Kernel::Arg::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::साँचो::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::साँचो::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::साँचो::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::साँचो::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::साँचो::मान_Kind, MD.m_मान_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<मान_Type> Unused;
    YIO.mapOptional(Kernel::Arg::साँचो::मान_Type, Unused);

    YIO.mapOptional(Kernel::Arg::साँचो::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::साँचो::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::साँचो::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::साँचो::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::साँचो::IsConst, MD.mIsConst, असत्य);
    YIO.mapOptional(Kernel::Arg::साँचो::IsRestrict, MD.mIsRestrict, असत्य);
    YIO.mapOptional(Kernel::Arg::साँचो::IsVolatile, MD.mIsVolatile, असत्य);
    YIO.mapOptional(Kernel::Arg::साँचो::IsPipe, MD.mIsPipe, असत्य);
  }
};

ढाँचा <>
संरचना MappingTraits<Kernel::CodeProps::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::साँचो::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::साँचो::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::साँचो::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::साँचो::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::साँचो::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::साँचो::अङ्क_SGPRs,
                    MD.m_अङ्क_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::साँचो::अङ्क_VGPRs,
                    MD.m_अङ्क_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::साँचो::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::साँचो::Is_Dynamic_Call_थाक,
                    MD.m_Is_Dynamic_Call_थाक, असत्य);
    YIO.mapOptional(Kernel::CodeProps::साँचो::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, असत्य);
    YIO.mapOptional(Kernel::CodeProps::साँचो::अङ्क_Spilled_SGPRs,
                    MD.m_अङ्क_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::साँचो::अङ्क_Spilled_VGPRs,
                    MD.m_अङ्क_Spilled_VGPRs, uint16_t(0));
  }
};

ढाँचा <>
संरचना MappingTraits<Kernel::DebugProps::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::साँचो::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::साँचो::Reserved_अङ्क_VGPRs,
                    MD.m_Reserved_अङ्क_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::साँचो::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::साँचो::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::साँचो::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

ढाँचा <>
संरचना MappingTraits<Kernel::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::साँचो::Name, MD.mName);
    YIO.mapRequired(Kernel::साँचो::चिह्न_Name, MD.m_चिह्न_Name);
    YIO.mapOptional(Kernel::साँचो::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::साँचो::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    यदि (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::साँचो::Attrs, MD.mAttrs);
    यदि (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::साँचो::Args, MD.mArgs);
    यदि (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::साँचो::CodeProps, MD.mCodeProps);
    यदि (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::साँचो::DebugProps, MD.mDebugProps);
  }
};

ढाँचा <>
संरचना MappingTraits<HSAMD::Metadata> {
  स्थिर शून्य mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(साँचो::Version, MD.mVersion);
    YIO.mapOptional(साँचो::Printf, MD.mPrintf, std::vector<std::string>());
    यदि (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(साँचो::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

नामाकाश AMDGPU {
नामाकाश HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::आगत Yaml_आगत(String);
  Yaml_आगत >> HSAMetadata;
  फर्काऊ Yaml_आगत.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<पूर्णाङ्क>::max());
  YamlOutput << HSAMetadata;
  फर्काऊ std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
