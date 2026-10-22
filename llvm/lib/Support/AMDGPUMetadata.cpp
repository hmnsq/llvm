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

#અન્તર્ભાવ "llvm/Support/AMDGPUMetadata.h"
#અન્તર્ભાવ "llvm/Support/YAMLTraits.h"
#અન્તર્ભાવ <optional>

using નામાકાશ llvm::AMDGPU;
using નામાકાશ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

નામાકાશ llvm {
નામાકાશ yaml {

template <>
સંરચના ScalarEnumerationTraits<AccessQualifier> {
  સ્થિર શૂન્ય enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
સંરચના ScalarEnumerationTraits<AddressSpaceQualifier> {
  સ્થિર શૂન્ય enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
સંરચના ScalarEnumerationTraits<કિંમત_Kind> {
  સ્થિર શૂન્ય enumeration(IO &YIO, કિંમત_Kind &EN) {
    YIO.enumCase(EN, "ByValue", કિંમત_Kind::By_કિંમત);
    YIO.enumCase(EN, "GlobalBuffer", કિંમત_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", કિંમત_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", કિંમત_Kind::Sampler);
    YIO.enumCase(EN, "Image", કિંમત_Kind::Image);
    YIO.enumCase(EN, "Pipe", કિંમત_Kind::Pipe);
    YIO.enumCase(EN, "Queue", કિંમત_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", કિંમત_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", કિંમત_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", કિંમત_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", કિંમત_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", કિંમત_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", કિંમત_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", કિંમત_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 કિંમત_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 કિંમત_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
સંરચના ScalarEnumerationTraits<કિંમત_Type> {
  સ્થિર શૂન્ય enumeration(IO &YIO, કિંમત_Type &EN) {
    YIO.enumCase(EN, "Struct", કિંમત_Type::Struct);
    YIO.enumCase(EN, "I8", કિંમત_Type::I8);
    YIO.enumCase(EN, "U8", કિંમત_Type::U8);
    YIO.enumCase(EN, "I16", કિંમત_Type::I16);
    YIO.enumCase(EN, "U16", કિંમત_Type::U16);
    YIO.enumCase(EN, "F16", કિંમત_Type::F16);
    YIO.enumCase(EN, "I32", કિંમત_Type::I32);
    YIO.enumCase(EN, "U32", કિંમત_Type::U32);
    YIO.enumCase(EN, "F32", કિંમત_Type::F32);
    YIO.enumCase(EN, "I64", કિંમત_Type::I64);
    YIO.enumCase(EN, "U64", કિંમત_Type::U64);
    YIO.enumCase(EN, "F64", કિંમત_Type::F64);
  }
};

template <>
સંરચના MappingTraits<Kernel::Attrs::Metadata> {
  સ્થિર શૂન્ય mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::ચાવી::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ચાવી::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ચાવી::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::ચાવી::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
સંરચના MappingTraits<Kernel::Arg::Metadata> {
  સ્થિર શૂન્ય mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::ચાવી::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::ચાવી::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::ચાવી::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::ચાવી::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::ચાવી::કિંમત_Kind, MD.m_કિંમત_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<કિંમત_Type> Unused;
    YIO.mapOptional(Kernel::Arg::ચાવી::કિંમત_Type, Unused);

    YIO.mapOptional(Kernel::Arg::ચાવી::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::ચાવી::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ચાવી::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ચાવી::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ચાવી::IsConst, MD.mIsConst, ખોટું);
    YIO.mapOptional(Kernel::Arg::ચાવી::IsRestrict, MD.mIsRestrict, ખોટું);
    YIO.mapOptional(Kernel::Arg::ચાવી::IsVolatile, MD.mIsVolatile, ખોટું);
    YIO.mapOptional(Kernel::Arg::ચાવી::IsPipe, MD.mIsPipe, ખોટું);
  }
};

template <>
સંરચના MappingTraits<Kernel::CodeProps::Metadata> {
  સ્થિર શૂન્ય mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::ચાવી::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::ચાવી::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::ચાવી::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::ચાવી::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::ચાવી::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::ચાવી::અંક_SGPRs,
                    MD.m_અંક_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ચાવી::અંક_VGPRs,
                    MD.m_અંક_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ચાવી::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::ચાવી::Is_Dynamic_Call_ઢગલો,
                    MD.m_Is_Dynamic_Call_ઢગલો, ખોટું);
    YIO.mapOptional(Kernel::CodeProps::ચાવી::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, ખોટું);
    YIO.mapOptional(Kernel::CodeProps::ચાવી::અંક_Spilled_SGPRs,
                    MD.m_અંક_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::ચાવી::અંક_Spilled_VGPRs,
                    MD.m_અંક_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
સંરચના MappingTraits<Kernel::DebugProps::Metadata> {
  સ્થિર શૂન્ય mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::ચાવી::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::ચાવી::Reserved_અંક_VGPRs,
                    MD.m_Reserved_અંક_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::ચાવી::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ચાવી::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ચાવી::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
સંરચના MappingTraits<Kernel::Metadata> {
  સ્થિર શૂન્ય mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::ચાવી::Name, MD.mName);
    YIO.mapRequired(Kernel::ચાવી::પ્રતીક_Name, MD.m_પ્રતીક_Name);
    YIO.mapOptional(Kernel::ચાવી::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::ચાવી::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    જો (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ચાવી::Attrs, MD.mAttrs);
    જો (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ચાવી::Args, MD.mArgs);
    જો (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ચાવી::CodeProps, MD.mCodeProps);
    જો (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::ચાવી::DebugProps, MD.mDebugProps);
  }
};

template <>
સંરચના MappingTraits<HSAMD::Metadata> {
  સ્થિર શૂન્ય mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(ચાવી::Version, MD.mVersion);
    YIO.mapOptional(ચાવી::Printf, MD.mPrintf, std::vector<std::string>());
    જો (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(ચાવી::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

નામાકાશ AMDGPU {
નામાકાશ HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::આગત Yaml_આગત(String);
  Yaml_આગત >> HSAMetadata;
  પાછા Yaml_આગત.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<પૂર્ણાંક>::max());
  YamlOutput << HSAMetadata;
  પાછા std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
