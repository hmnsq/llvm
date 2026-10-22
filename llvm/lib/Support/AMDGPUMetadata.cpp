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
بنية ScalarEnumerationTraits<قدر_Kind> {
  ساكن فراغ enumeration(IO &YIO, قدر_Kind &EN) {
    YIO.enumCase(EN, "ByValue", قدر_Kind::By_قدر);
    YIO.enumCase(EN, "GlobalBuffer", قدر_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", قدر_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", قدر_Kind::Sampler);
    YIO.enumCase(EN, "Image", قدر_Kind::Image);
    YIO.enumCase(EN, "Pipe", قدر_Kind::Pipe);
    YIO.enumCase(EN, "Queue", قدر_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", قدر_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", قدر_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", قدر_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", قدر_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", قدر_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", قدر_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", قدر_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 قدر_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 قدر_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
بنية ScalarEnumerationTraits<قدر_Type> {
  ساكن فراغ enumeration(IO &YIO, قدر_Type &EN) {
    YIO.enumCase(EN, "Struct", قدر_Type::Struct);
    YIO.enumCase(EN, "I8", قدر_Type::I8);
    YIO.enumCase(EN, "U8", قدر_Type::U8);
    YIO.enumCase(EN, "I16", قدر_Type::I16);
    YIO.enumCase(EN, "U16", قدر_Type::U16);
    YIO.enumCase(EN, "F16", قدر_Type::F16);
    YIO.enumCase(EN, "I32", قدر_Type::I32);
    YIO.enumCase(EN, "U32", قدر_Type::U32);
    YIO.enumCase(EN, "F32", قدر_Type::F32);
    YIO.enumCase(EN, "I64", قدر_Type::I64);
    YIO.enumCase(EN, "U64", قدر_Type::U64);
    YIO.enumCase(EN, "F64", قدر_Type::F64);
  }
};

template <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
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
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::کلید::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::کلید::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::کلید::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::کلید::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::کلید::قدر_Kind, MD.m_قدر_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<قدر_Type> Unused;
    YIO.mapOptional(Kernel::Arg::کلید::قدر_Type, Unused);

    YIO.mapOptional(Kernel::Arg::کلید::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::کلید::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::کلید::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::کلید::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::کلید::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::کلید::IsPipe, MD.mIsPipe, خطأ);
  }
};

template <>
بنية MappingTraits<Kernel::CodeProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
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
    YIO.mapOptional(Kernel::CodeProps::کلید::عدد_SGPRs,
                    MD.m_عدد_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلید::عدد_VGPRs,
                    MD.m_عدد_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلید::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلید::Is_Dynamic_Call_ڈھیر,
                    MD.m_Is_Dynamic_Call_ڈھیر, خطأ);
    YIO.mapOptional(Kernel::CodeProps::کلید::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::CodeProps::کلید::عدد_Spilled_SGPRs,
                    MD.m_عدد_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::کلید::عدد_Spilled_VGPRs,
                    MD.m_عدد_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::کلید::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::کلید::Reserved_عدد_VGPRs,
                    MD.m_Reserved_عدد_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::کلید::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلید::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::کلید::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::کلید::Name, MD.mName);
    YIO.mapRequired(Kernel::کلید::علامت_Name, MD.m_علامت_Name);
    YIO.mapOptional(Kernel::کلید::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::کلید::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    إذا (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::Attrs, MD.mAttrs);
    إذا (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::Args, MD.mArgs);
    إذا (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::CodeProps, MD.mCodeProps);
    إذا (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::کلید::DebugProps, MD.mDebugProps);
  }
};

template <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(کلید::Version, MD.mVersion);
    YIO.mapOptional(کلید::Printf, MD.mPrintf, std::vector<std::string>());
    إذا (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(کلید::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::داخلہ Yaml_داخلہ(String);
  Yaml_داخلہ >> HSAMetadata;
  أرجع Yaml_داخلہ.error();
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
