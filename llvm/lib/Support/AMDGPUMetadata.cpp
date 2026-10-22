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

#شامل "llvm/Support/AMDGPUMetadata.h"
#شامل "llvm/Support/YAMLTraits.h"
#شامل <optional>

using فضاینام llvm::AMDGPU;
using فضاینام llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

فضاینام llvm {
فضاینام yaml {

template <>
ساختار ScalarEnumerationTraits<AccessQualifier> {
  ایستا تهی enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
ساختار ScalarEnumerationTraits<AddressSpaceQualifier> {
  ایستا تهی enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
ساختار ScalarEnumerationTraits<مقدار_Kind> {
  ایستا تهی enumeration(IO &YIO, مقدار_Kind &EN) {
    YIO.enumCase(EN, "ByValue", مقدار_Kind::By_مقدار);
    YIO.enumCase(EN, "GlobalBuffer", مقدار_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", مقدار_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", مقدار_Kind::Sampler);
    YIO.enumCase(EN, "Image", مقدار_Kind::Image);
    YIO.enumCase(EN, "Pipe", مقدار_Kind::Pipe);
    YIO.enumCase(EN, "Queue", مقدار_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", مقدار_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", مقدار_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", مقدار_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", مقدار_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", مقدار_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", مقدار_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", مقدار_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 مقدار_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 مقدار_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
ساختار ScalarEnumerationTraits<مقدار_Type> {
  ایستا تهی enumeration(IO &YIO, مقدار_Type &EN) {
    YIO.enumCase(EN, "Struct", مقدار_Type::Struct);
    YIO.enumCase(EN, "I8", مقدار_Type::I8);
    YIO.enumCase(EN, "U8", مقدار_Type::U8);
    YIO.enumCase(EN, "I16", مقدار_Type::I16);
    YIO.enumCase(EN, "U16", مقدار_Type::U16);
    YIO.enumCase(EN, "F16", مقدار_Type::F16);
    YIO.enumCase(EN, "I32", مقدار_Type::I32);
    YIO.enumCase(EN, "U32", مقدار_Type::U32);
    YIO.enumCase(EN, "F32", مقدار_Type::F32);
    YIO.enumCase(EN, "I64", مقدار_Type::I64);
    YIO.enumCase(EN, "U64", مقدار_Type::U64);
    YIO.enumCase(EN, "F64", مقدار_Type::F64);
  }
};

template <>
ساختار MappingTraits<Kernel::Attrs::Metadata> {
  ایستا تهی mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::کلید::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلید::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::کلید::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::کلید::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
ساختار MappingTraits<Kernel::Arg::Metadata> {
  ایستا تهی mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::کلید::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::کلید::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::کلید::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::کلید::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::کلید::مقدار_Kind, MD.m_مقدار_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<مقدار_Type> Unused;
    YIO.mapOptional(Kernel::Arg::کلید::مقدار_Type, Unused);

    YIO.mapOptional(Kernel::Arg::کلید::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::کلید::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::IsConst, MD.mIsConst, نادرست);
    YIO.mapOptional(Kernel::Arg::کلید::IsRestrict, MD.mIsRestrict, نادرست);
    YIO.mapOptional(Kernel::Arg::کلید::IsVolatile, MD.mIsVolatile, نادرست);
    YIO.mapOptional(Kernel::Arg::کلید::IsPipe, MD.mIsPipe, نادرست);
  }
};

template <>
ساختار MappingTraits<Kernel::CodeProps::Metadata> {
  ایستا تهی mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::کلید::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::کلید::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::کلید::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::کلید::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::کلید::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::کلید::رقم_SGPRs,
                    MD.m_رقم_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلید::رقم_VGPRs,
                    MD.m_رقم_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلید::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلید::Is_Dynamic_Call_پشته,
                    MD.m_Is_Dynamic_Call_پشته, نادرست);
    YIO.mapOptional(Kernel::CodeProps::کلید::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, نادرست);
    YIO.mapOptional(Kernel::CodeProps::کلید::رقم_Spilled_SGPRs,
                    MD.m_رقم_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلید::رقم_Spilled_VGPRs,
                    MD.m_رقم_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
ساختار MappingTraits<Kernel::DebugProps::Metadata> {
  ایستا تهی mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::کلید::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::کلید::Reserved_رقم_VGPRs,
                    MD.m_Reserved_رقم_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::کلید::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلید::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلید::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
ساختار MappingTraits<Kernel::Metadata> {
  ایستا تهی mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::کلید::Name, MD.mName);
    YIO.mapRequired(Kernel::کلید::نماد_Name, MD.m_نماد_Name);
    YIO.mapOptional(Kernel::کلید::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::کلید::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    اگر (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::Attrs, MD.mAttrs);
    اگر (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::Args, MD.mArgs);
    اگر (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::CodeProps, MD.mCodeProps);
    اگر (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::DebugProps, MD.mDebugProps);
  }
};

template <>
ساختار MappingTraits<HSAMD::Metadata> {
  ایستا تهی mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(کلید::Version, MD.mVersion);
    YIO.mapOptional(کلید::Printf, MD.mPrintf, std::vector<std::string>());
    اگر (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(کلید::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

فضاینام AMDGPU {
فضاینام HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::درون_داد Yaml_درون_داد(String);
  Yaml_درون_داد >> HSAMetadata;
  بازگشت Yaml_درون_داد.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<صحیح>::max());
  YamlOutput << HSAMetadata;
  بازگشت std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
