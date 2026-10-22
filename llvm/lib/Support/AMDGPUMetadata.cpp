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

#அந்தர்பாவ "llvm/Support/AMDGPUMetadata.h"
#அந்தர்பாவ "llvm/Support/YAMLTraits.h"
#அந்தர்பாவ <optional>

using நாமாகாஶ llvm::AMDGPU;
using நாமாகாஶ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

நாமாகாஶ llvm {
நாமாகாஶ yaml {

template <>
கட்டமைப்பு ScalarEnumerationTraits<AccessQualifier> {
  நிலையான வெறுமை enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
கட்டமைப்பு ScalarEnumerationTraits<AddressSpaceQualifier> {
  நிலையான வெறுமை enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
கட்டமைப்பு ScalarEnumerationTraits<மதிப்பு_Kind> {
  நிலையான வெறுமை enumeration(IO &YIO, மதிப்பு_Kind &EN) {
    YIO.enumCase(EN, "ByValue", மதிப்பு_Kind::By_மதிப்பு);
    YIO.enumCase(EN, "GlobalBuffer", மதிப்பு_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", மதிப்பு_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", மதிப்பு_Kind::Sampler);
    YIO.enumCase(EN, "Image", மதிப்பு_Kind::Image);
    YIO.enumCase(EN, "Pipe", மதிப்பு_Kind::Pipe);
    YIO.enumCase(EN, "Queue", மதிப்பு_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", மதிப்பு_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", மதிப்பு_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", மதிப்பு_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", மதிப்பு_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", மதிப்பு_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", மதிப்பு_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", மதிப்பு_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 மதிப்பு_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 மதிப்பு_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
கட்டமைப்பு ScalarEnumerationTraits<மதிப்பு_Type> {
  நிலையான வெறுமை enumeration(IO &YIO, மதிப்பு_Type &EN) {
    YIO.enumCase(EN, "Struct", மதிப்பு_Type::Struct);
    YIO.enumCase(EN, "I8", மதிப்பு_Type::I8);
    YIO.enumCase(EN, "U8", மதிப்பு_Type::U8);
    YIO.enumCase(EN, "I16", மதிப்பு_Type::I16);
    YIO.enumCase(EN, "U16", மதிப்பு_Type::U16);
    YIO.enumCase(EN, "F16", மதிப்பு_Type::F16);
    YIO.enumCase(EN, "I32", மதிப்பு_Type::I32);
    YIO.enumCase(EN, "U32", மதிப்பு_Type::U32);
    YIO.enumCase(EN, "F32", மதிப்பு_Type::F32);
    YIO.enumCase(EN, "I64", மதிப்பு_Type::I64);
    YIO.enumCase(EN, "U64", மதிப்பு_Type::U64);
    YIO.enumCase(EN, "F64", மதிப்பு_Type::F64);
  }
};

template <>
கட்டமைப்பு MappingTraits<Kernel::Attrs::Metadata> {
  நிலையான வெறுமை mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::சாவி::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::சாவி::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::சாவி::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::சாவி::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
கட்டமைப்பு MappingTraits<Kernel::Arg::Metadata> {
  நிலையான வெறுமை mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::சாவி::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::சாவி::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::சாவி::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::சாவி::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::சாவி::மதிப்பு_Kind, MD.m_மதிப்பு_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<மதிப்பு_Type> Unused;
    YIO.mapOptional(Kernel::Arg::சாவி::மதிப்பு_Type, Unused);

    YIO.mapOptional(Kernel::Arg::சாவி::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::சாவி::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::சாவி::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::சாவி::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::சாவி::IsConst, MD.mIsConst, பொய்);
    YIO.mapOptional(Kernel::Arg::சாவி::IsRestrict, MD.mIsRestrict, பொய்);
    YIO.mapOptional(Kernel::Arg::சாவி::IsVolatile, MD.mIsVolatile, பொய்);
    YIO.mapOptional(Kernel::Arg::சாவி::IsPipe, MD.mIsPipe, பொய்);
  }
};

template <>
கட்டமைப்பு MappingTraits<Kernel::CodeProps::Metadata> {
  நிலையான வெறுமை mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::சாவி::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::சாவி::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::சாவி::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::சாவி::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::சாவி::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::சாவி::இலக்கம்_SGPRs,
                    MD.m_இலக்கம்_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::சாவி::இலக்கம்_VGPRs,
                    MD.m_இலக்கம்_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::சாவி::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::சாவி::Is_Dynamic_Call_அடுக்கு,
                    MD.m_Is_Dynamic_Call_அடுக்கு, பொய்);
    YIO.mapOptional(Kernel::CodeProps::சாவி::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, பொய்);
    YIO.mapOptional(Kernel::CodeProps::சாவி::இலக்கம்_Spilled_SGPRs,
                    MD.m_இலக்கம்_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::சாவி::இலக்கம்_Spilled_VGPRs,
                    MD.m_இலக்கம்_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
கட்டமைப்பு MappingTraits<Kernel::DebugProps::Metadata> {
  நிலையான வெறுமை mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::சாவி::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::சாவி::Reserved_இலக்கம்_VGPRs,
                    MD.m_Reserved_இலக்கம்_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::சாவி::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::சாவி::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::சாவி::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
கட்டமைப்பு MappingTraits<Kernel::Metadata> {
  நிலையான வெறுமை mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::சாவி::Name, MD.mName);
    YIO.mapRequired(Kernel::சாவி::குறியீடு_Name, MD.m_குறியீடு_Name);
    YIO.mapOptional(Kernel::சாவி::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::சாவி::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    எனில் (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::சாவி::Attrs, MD.mAttrs);
    எனில் (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::சாவி::Args, MD.mArgs);
    எனில் (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::சாவி::CodeProps, MD.mCodeProps);
    எனில் (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::சாவி::DebugProps, MD.mDebugProps);
  }
};

template <>
கட்டமைப்பு MappingTraits<HSAMD::Metadata> {
  நிலையான வெறுமை mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(சாவி::Version, MD.mVersion);
    YIO.mapOptional(சாவி::Printf, MD.mPrintf, std::vector<std::string>());
    எனில் (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(சாவி::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

நாமாகாஶ AMDGPU {
நாமாகாஶ HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::உள்ளீடு Yaml_உள்ளீடு(String);
  Yaml_உள்ளீடு >> HSAMetadata;
  திரும்பு Yaml_உள்ளீடு.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<முழு>::max());
  YamlOutput << HSAMetadata;
  திரும்பு std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
