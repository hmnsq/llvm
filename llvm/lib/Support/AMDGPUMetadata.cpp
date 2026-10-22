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
بنية ScalarEnumerationTraits<نيلاي_Kind> {
  ساكن فراغ enumeration(IO &YIO, نيلاي_Kind &EN) {
    YIO.enumCase(EN, "ByValue", نيلاي_Kind::By_نيلاي);
    YIO.enumCase(EN, "GlobalBuffer", نيلاي_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", نيلاي_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", نيلاي_Kind::Sampler);
    YIO.enumCase(EN, "Image", نيلاي_Kind::Image);
    YIO.enumCase(EN, "Pipe", نيلاي_Kind::Pipe);
    YIO.enumCase(EN, "Queue", نيلاي_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", نيلاي_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", نيلاي_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", نيلاي_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", نيلاي_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", نيلاي_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", نيلاي_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", نيلاي_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 نيلاي_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 نيلاي_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
بنية ScalarEnumerationTraits<نيلاي_Type> {
  ساكن فراغ enumeration(IO &YIO, نيلاي_Type &EN) {
    YIO.enumCase(EN, "Struct", نيلاي_Type::Struct);
    YIO.enumCase(EN, "I8", نيلاي_Type::I8);
    YIO.enumCase(EN, "U8", نيلاي_Type::U8);
    YIO.enumCase(EN, "I16", نيلاي_Type::I16);
    YIO.enumCase(EN, "U16", نيلاي_Type::U16);
    YIO.enumCase(EN, "F16", نيلاي_Type::F16);
    YIO.enumCase(EN, "I32", نيلاي_Type::I32);
    YIO.enumCase(EN, "U32", نيلاي_Type::U32);
    YIO.enumCase(EN, "F32", نيلاي_Type::F32);
    YIO.enumCase(EN, "I64", نيلاي_Type::I64);
    YIO.enumCase(EN, "U64", نيلاي_Type::U64);
    YIO.enumCase(EN, "F64", نيلاي_Type::F64);
  }
};

template <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::کونچي::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کونچي::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کونچي::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::کونچي::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::کونچي::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::کونچي::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::کونچي::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::کونچي::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::کونچي::نيلاي_Kind, MD.m_نيلاي_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<نيلاي_Type> Unused;
    YIO.mapOptional(Kernel::Arg::کونچي::نيلاي_Type, Unused);

    YIO.mapOptional(Kernel::Arg::کونچي::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::کونچي::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کونچي::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کونچي::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کونچي::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::کونچي::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::کونچي::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::کونچي::IsPipe, MD.mIsPipe, خطأ);
  }
};

template <>
بنية MappingTraits<Kernel::CodeProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::کونچي::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::کونچي::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::کونچي::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::کونچي::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::کونچي::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::کونچي::اڠک_SGPRs,
                    MD.m_اڠک_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کونچي::اڠک_VGPRs,
                    MD.m_اڠک_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کونچي::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::کونچي::Is_Dynamic_Call_تيندنن,
                    MD.m_Is_Dynamic_Call_تيندنن, خطأ);
    YIO.mapOptional(Kernel::CodeProps::کونچي::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::CodeProps::کونچي::اڠک_Spilled_SGPRs,
                    MD.m_اڠک_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کونچي::اڠک_Spilled_VGPRs,
                    MD.m_اڠک_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::کونچي::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::کونچي::Reserved_اڠک_VGPRs,
                    MD.m_Reserved_اڠک_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::کونچي::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کونچي::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کونچي::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::کونچي::Name, MD.mName);
    YIO.mapRequired(Kernel::کونچي::سيمبول_Name, MD.m_سيمبول_Name);
    YIO.mapOptional(Kernel::کونچي::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::کونچي::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    إذا (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کونچي::Attrs, MD.mAttrs);
    إذا (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کونچي::Args, MD.mArgs);
    إذا (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کونچي::CodeProps, MD.mCodeProps);
    إذا (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کونچي::DebugProps, MD.mDebugProps);
  }
};

template <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(کونچي::Version, MD.mVersion);
    YIO.mapOptional(کونچي::Printf, MD.mPrintf, std::vector<std::string>());
    إذا (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(کونچي::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::ماسوقن Yaml_ماسوقن(String);
  Yaml_ماسوقن >> HSAMetadata;
  أرجع Yaml_ماسوقن.error();
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
