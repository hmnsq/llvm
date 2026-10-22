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

#include "llvm/Support/AMDGPUMetadata.h"
#include "llvm/Support/YAMLTraits.h"
#include <optional>

using namespace llvm::AMDGPU;
using namespace llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

namespace llvm {
namespace yaml {

template <>
struct ScalarEnumerationTraits<AccessQualifier> {
  static void enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
struct ScalarEnumerationTraits<AddressSpaceQualifier> {
  static void enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
struct ScalarEnumerationTraits<हालागा_Kind> {
  static void enumeration(IO &YIO, हालागा_Kind &EN) {
    YIO.enumCase(EN, "ByValue", हालागा_Kind::By_हालागा);
    YIO.enumCase(EN, "GlobalBuffer", हालागा_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", हालागा_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", हालागा_Kind::Sampler);
    YIO.enumCase(EN, "Image", हालागा_Kind::Image);
    YIO.enumCase(EN, "Pipe", हालागा_Kind::Pipe);
    YIO.enumCase(EN, "Queue", हालागा_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", हालागा_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", हालागा_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", हालागा_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", हालागा_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", हालागा_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", हालागा_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", हालागा_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 हालागा_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 हालागा_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
struct ScalarEnumerationTraits<हालागा_Type> {
  static void enumeration(IO &YIO, हालागा_Type &EN) {
    YIO.enumCase(EN, "Struct", हालागा_Type::Struct);
    YIO.enumCase(EN, "I8", हालागा_Type::I8);
    YIO.enumCase(EN, "U8", हालागा_Type::U8);
    YIO.enumCase(EN, "I16", हालागा_Type::I16);
    YIO.enumCase(EN, "U16", हालागा_Type::U16);
    YIO.enumCase(EN, "F16", हालागा_Type::F16);
    YIO.enumCase(EN, "I32", हालागा_Type::I32);
    YIO.enumCase(EN, "U32", हालागा_Type::U32);
    YIO.enumCase(EN, "F32", हालागा_Type::F32);
    YIO.enumCase(EN, "I64", हालागा_Type::I64);
    YIO.enumCase(EN, "U64", हालागा_Type::U64);
    YIO.enumCase(EN, "F64", हालागा_Type::F64);
  }
};

template <>
struct MappingTraits<Kernel::Attrs::Metadata> {
  static void mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::सुसि::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::सुसि::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::सुसि::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::सुसि::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
struct MappingTraits<Kernel::Arg::Metadata> {
  static void mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::सुसि::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::सुसि::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::सुसि::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::सुसि::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::सुसि::हालागा_Kind, MD.m_हालागा_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<हालागा_Type> Unused;
    YIO.mapOptional(Kernel::Arg::सुसि::हालागा_Type, Unused);

    YIO.mapOptional(Kernel::Arg::सुसि::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::सुसि::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::सुसि::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::सुसि::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::सुसि::IsConst, MD.mIsConst, false);
    YIO.mapOptional(Kernel::Arg::सुसि::IsRestrict, MD.mIsRestrict, false);
    YIO.mapOptional(Kernel::Arg::सुसि::IsVolatile, MD.mIsVolatile, false);
    YIO.mapOptional(Kernel::Arg::सुसि::IsPipe, MD.mIsPipe, false);
  }
};

template <>
struct MappingTraits<Kernel::CodeProps::Metadata> {
  static void mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::सुसि::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::सुसि::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::सुसि::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::सुसि::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::सुसि::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::सुसि::नुमेरो_SGPRs,
                    MD.m_नुमेरो_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::सुसि::नुमेरो_VGPRs,
                    MD.m_नुमेरो_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::सुसि::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::सुसि::Is_Dynamic_Call_सालानसान,
                    MD.m_Is_Dynamic_Call_सालानसान, false);
    YIO.mapOptional(Kernel::CodeProps::सुसि::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, false);
    YIO.mapOptional(Kernel::CodeProps::सुसि::नुमेरो_Spilled_SGPRs,
                    MD.m_नुमेरो_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::सुसि::नुमेरो_Spilled_VGPRs,
                    MD.m_नुमेरो_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
struct MappingTraits<Kernel::DebugProps::Metadata> {
  static void mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::सुसि::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::सुसि::Reserved_नुमेरो_VGPRs,
                    MD.m_Reserved_नुमेरो_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::सुसि::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::सुसि::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::सुसि::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
struct MappingTraits<Kernel::Metadata> {
  static void mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::सुसि::Name, MD.mName);
    YIO.mapRequired(Kernel::सुसि::सिम्बोलो_Name, MD.m_सिम्बोलो_Name);
    YIO.mapOptional(Kernel::सुसि::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::सुसि::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    if (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::सुसि::Attrs, MD.mAttrs);
    if (!MD.m_मगा_कातविरान.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::सुसि::मगा_कातविरान, MD.m_मगा_कातविरान);
    if (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::सुसि::CodeProps, MD.mCodeProps);
    if (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::सुसि::DebugProps, MD.mDebugProps);
  }
};

template <>
struct MappingTraits<HSAMD::Metadata> {
  static void mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(सुसि::Version, MD.mVersion);
    YIO.mapOptional(सुसि::Printf, MD.mPrintf, std::vector<std::string>());
    if (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(सुसि::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

namespace AMDGPU {
namespace HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::पासोक Yaml_पासोक(String);
  Yaml_पासोक >> HSAMetadata;
  return Yaml_पासोक.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<int>::max());
  YamlOutput << HSAMetadata;
  return std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
