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

#অন্তর্ভাব "llvm/Support/AMDGPUMetadata.h"
#অন্তর্ভাব "llvm/Support/YAMLTraits.h"
#অন্তর্ভাব <optional>

using নামাকাশ llvm::AMDGPU;
using নামাকাশ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_VECTOR(Kernel::Metadata)

নামাকাশ llvm {
নামাকাশ yaml {

template <>
কাঠামো ScalarEnumerationTraits<AccessQualifier> {
  স্থির শূন্য enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

template <>
কাঠামো ScalarEnumerationTraits<AddressSpaceQualifier> {
  স্থির শূন্য enumeration(IO &YIO, AddressSpaceQualifier &EN) {
    YIO.enumCase(EN, "Private", AddressSpaceQualifier::Private);
    YIO.enumCase(EN, "Global", AddressSpaceQualifier::Global);
    YIO.enumCase(EN, "Constant", AddressSpaceQualifier::Constant);
    YIO.enumCase(EN, "Local", AddressSpaceQualifier::Local);
    YIO.enumCase(EN, "Generic", AddressSpaceQualifier::Generic);
    YIO.enumCase(EN, "Region", AddressSpaceQualifier::Region);
  }
};

template <>
কাঠামো ScalarEnumerationTraits<মান_Kind> {
  স্থির শূন্য enumeration(IO &YIO, মান_Kind &EN) {
    YIO.enumCase(EN, "ByValue", মান_Kind::By_মান);
    YIO.enumCase(EN, "GlobalBuffer", মান_Kind::Global_বাফার);
    YIO.enumCase(EN, "DynamicSharedPointer", মান_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", মান_Kind::Sampler);
    YIO.enumCase(EN, "Image", মান_Kind::Image);
    YIO.enumCase(EN, "Pipe", মান_Kind::Pipe);
    YIO.enumCase(EN, "Queue", মান_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", মান_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", মান_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", মান_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", মান_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", মান_Kind::Hidden_Printf_বাফার);
    YIO.enumCase(EN, "HiddenHostcallBuffer", মান_Kind::Hidden_Hostcall_বাফার);
    YIO.enumCase(EN, "HiddenDefaultQueue", মান_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 মান_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 মান_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
কাঠামো ScalarEnumerationTraits<মান_Type> {
  স্থির শূন্য enumeration(IO &YIO, মান_Type &EN) {
    YIO.enumCase(EN, "Struct", মান_Type::Struct);
    YIO.enumCase(EN, "I8", মান_Type::I8);
    YIO.enumCase(EN, "U8", মান_Type::U8);
    YIO.enumCase(EN, "I16", মান_Type::I16);
    YIO.enumCase(EN, "U16", মান_Type::U16);
    YIO.enumCase(EN, "F16", মান_Type::F16);
    YIO.enumCase(EN, "I32", মান_Type::I32);
    YIO.enumCase(EN, "U32", মান_Type::U32);
    YIO.enumCase(EN, "F32", মান_Type::F32);
    YIO.enumCase(EN, "I64", মান_Type::I64);
    YIO.enumCase(EN, "U64", মান_Type::U64);
    YIO.enumCase(EN, "F64", মান_Type::F64);
  }
};

template <>
কাঠামো MappingTraits<Kernel::Attrs::Metadata> {
  স্থির শূন্য mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::চাবি::Reqd_Work_Group_আকার,
                    MD.m_Reqd_Work_Group_আকার, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::চাবি::Work_Group_আকার_Hint,
                    MD.m_Work_Group_আকার_Hint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::চাবি::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::চাবি::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
কাঠামো MappingTraits<Kernel::Arg::Metadata> {
  স্থির শূন্য mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::চাবি::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::চাবি::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::চাবি::আকার_2, MD.m_আকার);
    YIO.mapRequired(Kernel::Arg::চাবি::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::চাবি::মান_Kind, MD.m_মান_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<মান_Type> Unused;
    YIO.mapOptional(Kernel::Arg::চাবি::মান_Type, Unused);

    YIO.mapOptional(Kernel::Arg::চাবি::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::চাবি::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::চাবি::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::চাবি::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::চাবি::IsConst, MD.mIsConst, মিথ্যা);
    YIO.mapOptional(Kernel::Arg::চাবি::IsRestrict, MD.mIsRestrict, মিথ্যা);
    YIO.mapOptional(Kernel::Arg::চাবি::IsVolatile, MD.mIsVolatile, মিথ্যা);
    YIO.mapOptional(Kernel::Arg::চাবি::IsPipe, MD.mIsPipe, মিথ্যা);
  }
};

template <>
কাঠামো MappingTraits<Kernel::CodeProps::Metadata> {
  স্থির শূন্য mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::চাবি::Kernarg_Segment_আকার,
                    MD.m_Kernarg_Segment_আকার);
    YIO.mapRequired(Kernel::CodeProps::চাবি::Group_Segment_Fixed_আকার,
                    MD.m_Group_Segment_Fixed_আকার);
    YIO.mapRequired(Kernel::CodeProps::চাবি::Private_Segment_Fixed_আকার,
                    MD.m_Private_Segment_Fixed_আকার);
    YIO.mapRequired(Kernel::CodeProps::চাবি::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::চাবি::Wavefront_আকার,
                    MD.m_Wavefront_আকার);
    YIO.mapOptional(Kernel::CodeProps::চাবি::অঙ্ক_SGPRs,
                    MD.m_অঙ্ক_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::চাবি::অঙ্ক_VGPRs,
                    MD.m_অঙ্ক_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::চাবি::Max_Flat_Work_Group_আকার,
                    MD.m_Max_Flat_Work_Group_আকার, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::চাবি::Is_Dynamic_Call_স্তূপ,
                    MD.m_Is_Dynamic_Call_স্তূপ, মিথ্যা);
    YIO.mapOptional(Kernel::CodeProps::চাবি::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, মিথ্যা);
    YIO.mapOptional(Kernel::CodeProps::চাবি::অঙ্ক_Spilled_SGPRs,
                    MD.m_অঙ্ক_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::চাবি::অঙ্ক_Spilled_VGPRs,
                    MD.m_অঙ্ক_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
কাঠামো MappingTraits<Kernel::DebugProps::Metadata> {
  স্থির শূন্য mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::চাবি::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::চাবি::Reserved_অঙ্ক_VGPRs,
                    MD.m_Reserved_অঙ্ক_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::চাবি::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::চাবি::Private_Segment_বাফার_SGPR,
                    MD.m_Private_Segment_বাফার_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::চাবি::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
কাঠামো MappingTraits<Kernel::Metadata> {
  স্থির শূন্য mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::চাবি::Name, MD.mName);
    YIO.mapRequired(Kernel::চাবি::প্রতীক_Name, MD.m_প্রতীক_Name);
    YIO.mapOptional(Kernel::চাবি::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::চাবি::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    যদি (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::চাবি::Attrs, MD.mAttrs);
    যদি (!MD.mArgs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::চাবি::Args, MD.mArgs);
    যদি (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::চাবি::CodeProps, MD.mCodeProps);
    যদি (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::চাবি::DebugProps, MD.mDebugProps);
  }
};

template <>
কাঠামো MappingTraits<HSAMD::Metadata> {
  স্থির শূন্য mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(চাবি::Version, MD.mVersion);
    YIO.mapOptional(চাবি::Printf, MD.mPrintf, std::vector<std::string>());
    যদি (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(চাবি::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

নামাকাশ AMDGPU {
নামাকাশ HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::প্রবেশ Yaml_প্রবেশ(String);
  Yaml_প্রবেশ >> HSAMetadata;
  ফেরত Yaml_প্রবেশ.error();
}

std::error_code toString(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream YamlStream(String);
  yaml::Output YamlOutput(YamlStream, nullptr, std::numeric_limits<পূর্ণসংখ্যা>::max());
  YamlOutput << HSAMetadata;
  ফেরত std::error_code();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
