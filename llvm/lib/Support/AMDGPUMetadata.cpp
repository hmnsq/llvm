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

template <>
संरचना ScalarEnumerationTraits<AccessQualifier> {
  स्थिर शून्य enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
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

template <>
संरचना ScalarEnumerationTraits<मूल्य_Kind> {
  स्थिर शून्य enumeration(IO &YIO, मूल्य_Kind &EN) {
    YIO.enumCase(EN, "ByValue", मूल्य_Kind::By_मूल्य);
    YIO.enumCase(EN, "GlobalBuffer", मूल्य_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", मूल्य_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", मूल्य_Kind::Sampler);
    YIO.enumCase(EN, "Image", मूल्य_Kind::Image);
    YIO.enumCase(EN, "Pipe", मूल्य_Kind::Pipe);
    YIO.enumCase(EN, "Queue", मूल्य_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", मूल्य_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", मूल्य_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", मूल्य_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", मूल्य_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", मूल्य_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", मूल्य_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", मूल्य_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 मूल्य_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 मूल्य_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
संरचना ScalarEnumerationTraits<मूल्य_Type> {
  स्थिर शून्य enumeration(IO &YIO, मूल्य_Type &EN) {
    YIO.enumCase(EN, "Struct", मूल्य_Type::Struct);
    YIO.enumCase(EN, "I8", मूल्य_Type::I8);
    YIO.enumCase(EN, "U8", मूल्य_Type::U8);
    YIO.enumCase(EN, "I16", मूल्य_Type::I16);
    YIO.enumCase(EN, "U16", मूल्य_Type::U16);
    YIO.enumCase(EN, "F16", मूल्य_Type::F16);
    YIO.enumCase(EN, "I32", मूल्य_Type::I32);
    YIO.enumCase(EN, "U32", मूल्य_Type::U32);
    YIO.enumCase(EN, "F32", मूल्य_Type::F32);
    YIO.enumCase(EN, "I64", मूल्य_Type::I64);
    YIO.enumCase(EN, "U64", मूल्य_Type::U64);
    YIO.enumCase(EN, "F64", मूल्य_Type::F64);
  }
};

template <>
संरचना MappingTraits<Kernel::Attrs::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::कुंजी::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::कुंजी::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::कुंजी::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::कुंजी::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
संरचना MappingTraits<Kernel::Arg::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::कुंजी::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::कुंजी::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::कुंजी::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::कुंजी::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::कुंजी::मूल्य_Kind, MD.m_मूल्य_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<मूल्य_Type> Unused;
    YIO.mapOptional(Kernel::Arg::कुंजी::मूल्य_Type, Unused);

    YIO.mapOptional(Kernel::Arg::कुंजी::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::कुंजी::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुंजी::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुंजी::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::कुंजी::IsConst, MD.mIsConst, असत्य);
    YIO.mapOptional(Kernel::Arg::कुंजी::IsRestrict, MD.mIsRestrict, असत्य);
    YIO.mapOptional(Kernel::Arg::कुंजी::IsVolatile, MD.mIsVolatile, असत्य);
    YIO.mapOptional(Kernel::Arg::कुंजी::IsPipe, MD.mIsPipe, असत्य);
  }
};

template <>
संरचना MappingTraits<Kernel::CodeProps::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::कुंजी::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::कुंजी::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::कुंजी::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::कुंजी::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::कुंजी::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::कुंजी::अंक_SGPRs,
                    MD.m_अंक_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::कुंजी::अंक_VGPRs,
                    MD.m_अंक_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::कुंजी::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::कुंजी::Is_Dynamic_Call_ढेर,
                    MD.m_Is_Dynamic_Call_ढेर, असत्य);
    YIO.mapOptional(Kernel::CodeProps::कुंजी::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, असत्य);
    YIO.mapOptional(Kernel::CodeProps::कुंजी::अंक_Spilled_SGPRs,
                    MD.m_अंक_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::कुंजी::अंक_Spilled_VGPRs,
                    MD.m_अंक_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
संरचना MappingTraits<Kernel::DebugProps::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::कुंजी::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::कुंजी::Reserved_अंक_VGPRs,
                    MD.m_Reserved_अंक_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::कुंजी::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::कुंजी::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::कुंजी::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
संरचना MappingTraits<Kernel::Metadata> {
  स्थिर शून्य mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::कुंजी::Name, MD.mName);
    YIO.mapRequired(Kernel::कुंजी::प्रतीक_Name, MD.m_प्रतीक_Name);
    YIO.mapOptional(Kernel::कुंजी::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::कुंजी::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    यदि (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुंजी::Attrs, MD.mAttrs);
    यदि (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुंजी::Args, MD.mArgs);
    यदि (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुंजी::CodeProps, MD.mCodeProps);
    यदि (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::कुंजी::DebugProps, MD.mDebugProps);
  }
};

template <>
संरचना MappingTraits<HSAMD::Metadata> {
  स्थिर शून्य mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(कुंजी::Version, MD.mVersion);
    YIO.mapOptional(कुंजी::Printf, MD.mPrintf, std::vector<std::string>());
    यदि (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(कुंजी::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

नामाकाश AMDGPU {
नामाकाश HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::निवेश Yaml_निवेश(String);
  Yaml_निवेश >> HSAMetadata;
  प्रत्यागम Yaml_निवेश.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<पूर्णाङ्क>::max());
  YamlOutput << HSAMetadata;
  प्रत्यागम std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
