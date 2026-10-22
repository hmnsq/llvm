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

using نطاق llvm::AMDGPU;
using نطاق llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

نطاق llvm {
نطاق yaml {

template <>
بنية ScalarEnumerationTraits<AccessQualifier> {
  ساكن فراغ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
بنية ScalarEnumerationTraits<AddressSpaceQualifier> {
  ساكن فراغ enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
بنية ScalarEnumerationTraits<بەها_Kind> {
  ساكن فراغ enumeration(IO &YIO, بەها_Kind &EN) {
    YIO.enumCase(EN, "ByValue", بەها_Kind::By_بەها);
    YIO.enumCase(EN, "GlobalBuffer", بەها_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", بەها_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", بەها_Kind::Sampler);
    YIO.enumCase(EN, "Image", بەها_Kind::Image);
    YIO.enumCase(EN, "Pipe", بەها_Kind::Pipe);
    YIO.enumCase(EN, "Queue", بەها_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", بەها_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", بەها_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", بەها_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", بەها_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", بەها_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", بەها_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", بەها_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 بەها_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 بەها_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
بنية ScalarEnumerationTraits<بەها_Type> {
  ساكن فراغ enumeration(IO &YIO, بەها_Type &EN) {
    YIO.enumCase(EN, "Struct", بەها_Type::Struct);
    YIO.enumCase(EN, "I8", بەها_Type::I8);
    YIO.enumCase(EN, "U8", بەها_Type::U8);
    YIO.enumCase(EN, "I16", بەها_Type::I16);
    YIO.enumCase(EN, "U16", بەها_Type::U16);
    YIO.enumCase(EN, "F16", بەها_Type::F16);
    YIO.enumCase(EN, "I32", بەها_Type::I32);
    YIO.enumCase(EN, "U32", بەها_Type::U32);
    YIO.enumCase(EN, "F32", بەها_Type::F32);
    YIO.enumCase(EN, "I64", بەها_Type::I64);
    YIO.enumCase(EN, "U64", بەها_Type::U64);
    YIO.enumCase(EN, "F64", بەها_Type::F64);
  }
};

template <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::کلیل::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلیل::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلیل::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::کلیل::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::کلیل::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::کلیل::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::کلیل::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::کلیل::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::کلیل::بەها_Kind, MD.m_بەها_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<بەها_Type> Unused;
    YIO.mapOptional(Kernel::Arg::کلیل::بەها_Type, Unused);

    YIO.mapOptional(Kernel::Arg::کلیل::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::کلیل::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلیل::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلیل::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلیل::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::کلیل::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::کلیل::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::کلیل::IsPipe, MD.mIsPipe, خطأ);
  }
};

template <>
بنية MappingTraits<Kernel::CodeProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::کلیل::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::کلیل::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::کلیل::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::کلیل::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::کلیل::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::کلیل::ژمارە_SGPRs,
                    MD.m_ژمارە_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلیل::ژمارە_VGPRs,
                    MD.m_ژمارە_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلیل::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلیل::Is_Dynamic_Call_کەڵەکە,
                    MD.m_Is_Dynamic_Call_کەڵەکە, خطأ);
    YIO.mapOptional(Kernel::CodeProps::کلیل::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::CodeProps::کلیل::ژمارە_Spilled_SGPRs,
                    MD.m_ژمارە_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلیل::ژمارە_Spilled_VGPRs,
                    MD.m_ژمارە_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::کلیل::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::کلیل::Reserved_ژمارە_VGPRs,
                    MD.m_Reserved_ژمارە_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::کلیل::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلیل::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلیل::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::کلیل::Name, MD.mName);
    YIO.mapRequired(Kernel::کلیل::هێما_Name, MD.m_هێما_Name);
    YIO.mapOptional(Kernel::کلیل::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::کلیل::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    إذا (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلیل::Attrs, MD.mAttrs);
    إذا (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلیل::Args, MD.mArgs);
    إذا (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلیل::CodeProps, MD.mCodeProps);
    إذا (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلیل::DebugProps, MD.mDebugProps);
  }
};

template <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(کلیل::Version, MD.mVersion);
    YIO.mapOptional(کلیل::Printf, MD.mPrintf, std::vector<std::string>());
    إذا (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(کلیل::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::تێخستن Yaml_تێخستن(String);
  Yaml_تێخستن >> HSAMetadata;
  أرجع Yaml_تێخستن.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<صحيح>::max());
  YamlOutput << HSAMetadata;
  أرجع std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
