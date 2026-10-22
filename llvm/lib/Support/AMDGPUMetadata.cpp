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
بنية ScalarEnumerationTraits<درج_Kind> {
  ساكن فراغ enumeration(IO &YIO, درج_Kind &EN) {
    YIO.enumCase(EN, "ByValue", درج_Kind::By_درج);
    YIO.enumCase(EN, "GlobalBuffer", درج_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", درج_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", درج_Kind::Sampler);
    YIO.enumCase(EN, "Image", درج_Kind::Image);
    YIO.enumCase(EN, "Pipe", درج_Kind::Pipe);
    YIO.enumCase(EN, "Queue", درج_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", درج_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", درج_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", درج_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", درج_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", درج_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", درج_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", درج_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 درج_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 درج_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
بنية ScalarEnumerationTraits<درج_Type> {
  ساكن فراغ enumeration(IO &YIO, درج_Type &EN) {
    YIO.enumCase(EN, "Struct", درج_Type::Struct);
    YIO.enumCase(EN, "I8", درج_Type::I8);
    YIO.enumCase(EN, "U8", درج_Type::U8);
    YIO.enumCase(EN, "I16", درج_Type::I16);
    YIO.enumCase(EN, "U16", درج_Type::U16);
    YIO.enumCase(EN, "F16", درج_Type::F16);
    YIO.enumCase(EN, "I32", درج_Type::I32);
    YIO.enumCase(EN, "U32", درج_Type::U32);
    YIO.enumCase(EN, "F32", درج_Type::F32);
    YIO.enumCase(EN, "I64", درج_Type::I64);
    YIO.enumCase(EN, "U64", درج_Type::U64);
    YIO.enumCase(EN, "F64", درج_Type::F64);
  }
};

template <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::مبودي::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::مبودي::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::مبودي::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::مبودي::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::مبودي::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::مبودي::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::مبودي::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::مبودي::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::مبودي::درج_Kind, MD.m_درج_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<درج_Type> Unused;
    YIO.mapOptional(Kernel::Arg::مبودي::درج_Type, Unused);

    YIO.mapOptional(Kernel::Arg::مبودي::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::مبودي::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::مبودي::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::مبودي::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::مبودي::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::مبودي::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::مبودي::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::مبودي::IsPipe, MD.mIsPipe, خطأ);
  }
};

template <>
بنية MappingTraits<Kernel::CodeProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::مبودي::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::مبودي::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::مبودي::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::مبودي::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::مبودي::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::مبودي::لمب_SGPRs,
                    MD.m_لمب_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::مبودي::لمب_VGPRs,
                    MD.m_لمب_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::مبودي::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::مبودي::Is_Dynamic_Call_تر,
                    MD.m_Is_Dynamic_Call_تر, خطأ);
    YIO.mapOptional(Kernel::CodeProps::مبودي::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::CodeProps::مبودي::لمب_Spilled_SGPRs,
                    MD.m_لمب_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::مبودي::لمب_Spilled_VGPRs,
                    MD.m_لمب_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::مبودي::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::مبودي::Reserved_لمب_VGPRs,
                    MD.m_Reserved_لمب_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::مبودي::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::مبودي::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::مبودي::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::مبودي::Name, MD.mName);
    YIO.mapRequired(Kernel::مبودي::علام_Name, MD.m_علام_Name);
    YIO.mapOptional(Kernel::مبودي::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::مبودي::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    إذا (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::مبودي::Attrs, MD.mAttrs);
    إذا (!MD.m_حجوجي.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::مبودي::حجوجي, MD.m_حجوجي);
    إذا (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::مبودي::CodeProps, MD.mCodeProps);
    إذا (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::مبودي::DebugProps, MD.mDebugProps);
  }
};

template <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(مبودي::Version, MD.mVersion);
    YIO.mapOptional(مبودي::Printf, MD.mPrintf, std::vector<std::string>());
    إذا (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(مبودي::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::شغرو Yaml_شغرو(String);
  Yaml_شغرو >> HSAMetadata;
  أرجع Yaml_شغرو.error();
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
