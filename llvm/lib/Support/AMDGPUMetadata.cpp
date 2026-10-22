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
بنية ScalarEnumerationTraits<قىممەت_Kind> {
  ساكن فراغ enumeration(IO &YIO, قىممەت_Kind &EN) {
    YIO.enumCase(EN, "ByValue", قىممەت_Kind::By_قىممەت);
    YIO.enumCase(EN, "GlobalBuffer", قىممەت_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", قىممەت_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", قىممەت_Kind::Sampler);
    YIO.enumCase(EN, "Image", قىممەت_Kind::Image);
    YIO.enumCase(EN, "Pipe", قىممەت_Kind::Pipe);
    YIO.enumCase(EN, "Queue", قىممەت_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", قىممەت_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", قىممەت_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", قىممەت_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", قىممەت_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", قىممەت_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", قىممەت_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", قىممەت_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 قىممەت_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 قىممەت_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
بنية ScalarEnumerationTraits<قىممەت_Type> {
  ساكن فراغ enumeration(IO &YIO, قىممەت_Type &EN) {
    YIO.enumCase(EN, "Struct", قىممەت_Type::Struct);
    YIO.enumCase(EN, "I8", قىممەت_Type::I8);
    YIO.enumCase(EN, "U8", قىممەت_Type::U8);
    YIO.enumCase(EN, "I16", قىممەت_Type::I16);
    YIO.enumCase(EN, "U16", قىممەت_Type::U16);
    YIO.enumCase(EN, "F16", قىممەت_Type::F16);
    YIO.enumCase(EN, "I32", قىممەت_Type::I32);
    YIO.enumCase(EN, "U32", قىممەت_Type::U32);
    YIO.enumCase(EN, "F32", قىممەت_Type::F32);
    YIO.enumCase(EN, "I64", قىممەت_Type::I64);
    YIO.enumCase(EN, "U64", قىممەت_Type::U64);
    YIO.enumCase(EN, "F64", قىممەت_Type::F64);
  }
};

template <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::ئاچقۇچ::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ئاچقۇچ::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ئاچقۇچ::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::ئاچقۇچ::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::ئاچقۇچ::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::ئاچقۇچ::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::ئاچقۇچ::قىممەت_Kind, MD.m_قىممەت_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<قىممەت_Type> Unused;
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::قىممەت_Type, Unused);

    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::ئاچقۇچ::IsPipe, MD.mIsPipe, خطأ);
  }
};

template <>
بنية MappingTraits<Kernel::CodeProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::ئاچقۇچ::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::ئاچقۇچ::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::ئاچقۇچ::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::ئاچقۇچ::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::ئاچقۇچ::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::ئاچقۇچ::رەقەم_SGPRs,
                    MD.m_رەقەم_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ئاچقۇچ::رەقەم_VGPRs,
                    MD.m_رەقەم_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ئاچقۇچ::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::ئاچقۇچ::Is_Dynamic_Call_دۆۋە,
                    MD.m_Is_Dynamic_Call_دۆۋە, خطأ);
    YIO.mapOptional(Kernel::CodeProps::ئاچقۇچ::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::CodeProps::ئاچقۇچ::رەقەم_Spilled_SGPRs,
                    MD.m_رەقەم_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ئاچقۇچ::رەقەم_Spilled_VGPRs,
                    MD.m_رەقەم_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::ئاچقۇچ::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::ئاچقۇچ::Reserved_رەقەم_VGPRs,
                    MD.m_Reserved_رەقەم_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::ئاچقۇچ::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ئاچقۇچ::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ئاچقۇچ::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::ئاچقۇچ::Name, MD.mName);
    YIO.mapRequired(Kernel::ئاچقۇچ::بەلگە_Name, MD.m_بەلگە_Name);
    YIO.mapOptional(Kernel::ئاچقۇچ::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::ئاچقۇچ::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    إذا (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ئاچقۇچ::Attrs, MD.mAttrs);
    إذا (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ئاچقۇچ::Args, MD.mArgs);
    إذا (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ئاچقۇچ::CodeProps, MD.mCodeProps);
    إذا (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ئاچقۇچ::DebugProps, MD.mDebugProps);
  }
};

template <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(ئاچقۇچ::Version, MD.mVersion);
    YIO.mapOptional(ئاچقۇچ::Printf, MD.mPrintf, std::vector<std::string>());
    إذا (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(ئاچقۇچ::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::كىرگۈزۈش Yaml_كىرگۈزۈش(String);
  Yaml_كىرگۈزۈش >> HSAMetadata;
  أرجع Yaml_كىرگۈزۈش.error();
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
