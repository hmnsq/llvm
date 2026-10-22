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
بنية ScalarEnumerationTraits<قيمة_Kind> {
  ساكن فراغ enumeration(IO &YIO, قيمة_Kind &EN) {
    YIO.enumCase(EN, "ByValue", قيمة_Kind::By_قيمة);
    YIO.enumCase(EN, "GlobalBuffer", قيمة_Kind::GlobalBuffer);
    YIO.enumCase(EN, "DynamicSharedPointer", قيمة_Kind::DynamicSharedPointer);
    YIO.enumCase(EN, "Sampler", قيمة_Kind::Sampler);
    YIO.enumCase(EN, "Image", قيمة_Kind::Image);
    YIO.enumCase(EN, "Pipe", قيمة_Kind::Pipe);
    YIO.enumCase(EN, "Queue", قيمة_Kind::Queue);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", قيمة_Kind::HiddenGlobalOffsetX);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", قيمة_Kind::HiddenGlobalOffsetY);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", قيمة_Kind::HiddenGlobalOffsetZ);
    YIO.enumCase(EN, "HiddenNone", قيمة_Kind::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", قيمة_Kind::HiddenPrintfBuffer);
    YIO.enumCase(EN, "HiddenHostcallBuffer", قيمة_Kind::HiddenHostcallBuffer);
    YIO.enumCase(EN, "HiddenDefaultQueue", قيمة_Kind::HiddenDefaultQueue);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 قيمة_Kind::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 قيمة_Kind::HiddenMultiGridSyncArg);
  }
};

template <>
بنية ScalarEnumerationTraits<قيمة_Type> {
  ساكن فراغ enumeration(IO &YIO, قيمة_Type &EN) {
    YIO.enumCase(EN, "Struct", قيمة_Type::Struct);
    YIO.enumCase(EN, "I8", قيمة_Type::I8);
    YIO.enumCase(EN, "U8", قيمة_Type::U8);
    YIO.enumCase(EN, "I16", قيمة_Type::I16);
    YIO.enumCase(EN, "U16", قيمة_Type::U16);
    YIO.enumCase(EN, "F16", قيمة_Type::F16);
    YIO.enumCase(EN, "I32", قيمة_Type::I32);
    YIO.enumCase(EN, "U32", قيمة_Type::U32);
    YIO.enumCase(EN, "F32", قيمة_Type::F32);
    YIO.enumCase(EN, "I64", قيمة_Type::I64);
    YIO.enumCase(EN, "U64", قيمة_Type::U64);
    YIO.enumCase(EN, "F64", قيمة_Type::F64);
  }
};

template <>
بنية MappingTraits<Kernel::Attrs::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::مفتاح::ReqdWorkGroupSize,
                    MD.mReqdWorkGroupSize, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::مفتاح::WorkGroupSizeHint,
                    MD.mWorkGroupSizeHint, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::مفتاح::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::مفتاح::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

template <>
بنية MappingTraits<Kernel::Arg::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::مفتاح::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::مفتاح::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::مفتاح::Size, MD.mSize);
    YIO.mapRequired(Kernel::Arg::مفتاح::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::مفتاح::قيمة_Kind, MD.m_قيمة_Kind);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<قيمة_Type> Unused;
    YIO.mapOptional(Kernel::Arg::مفتاح::قيمة_Type, Unused);

    YIO.mapOptional(Kernel::Arg::مفتاح::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::مفتاح::AddrSpaceQual, MD.mAddrSpaceQual,
                    AddressSpaceQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::مفتاح::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::مفتاح::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::مفتاح::IsConst, MD.mIsConst, خطأ);
    YIO.mapOptional(Kernel::Arg::مفتاح::IsRestrict, MD.mIsRestrict, خطأ);
    YIO.mapOptional(Kernel::Arg::مفتاح::IsVolatile, MD.mIsVolatile, خطأ);
    YIO.mapOptional(Kernel::Arg::مفتاح::IsPipe, MD.mIsPipe, خطأ);
  }
};

template <>
بنية MappingTraits<Kernel::CodeProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::CodeProps::Metadata &MD) {
    YIO.mapRequired(Kernel::CodeProps::مفتاح::KernargSegmentSize,
                    MD.mKernargSegmentSize);
    YIO.mapRequired(Kernel::CodeProps::مفتاح::GroupSegmentFixedSize,
                    MD.mGroupSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::مفتاح::PrivateSegmentFixedSize,
                    MD.mPrivateSegmentFixedSize);
    YIO.mapRequired(Kernel::CodeProps::مفتاح::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::CodeProps::مفتاح::WavefrontSize,
                    MD.mWavefrontSize);
    YIO.mapOptional(Kernel::CodeProps::مفتاح::رقم_SGPRs,
                    MD.m_رقم_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::مفتاح::رقم_VGPRs,
                    MD.m_رقم_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::مفتاح::MaxFlatWorkGroupSize,
                    MD.mMaxFlatWorkGroupSize, uint32_t(0));
    YIO.mapOptional(Kernel::CodeProps::مفتاح::Is_Dynamic_Call_مكدس,
                    MD.m_Is_Dynamic_Call_مكدس, خطأ);
    YIO.mapOptional(Kernel::CodeProps::مفتاح::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, خطأ);
    YIO.mapOptional(Kernel::CodeProps::مفتاح::رقم_Spilled_SGPRs,
                    MD.m_رقم_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::CodeProps::مفتاح::رقم_Spilled_VGPRs,
                    MD.m_رقم_Spilled_VGPRs, uint16_t(0));
  }
};

template <>
بنية MappingTraits<Kernel::DebugProps::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::مفتاح::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::vector<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::مفتاح::Reserved_رقم_VGPRs,
                    MD.m_Reserved_رقم_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::مفتاح::ReservedFirstVGPR,
                    MD.mReservedFirstVGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::مفتاح::PrivateSegmentBufferSGPR,
                    MD.mPrivateSegmentBufferSGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::مفتاح::WavefrontPrivateSegmentOffsetSGPR,
                    MD.mWavefrontPrivateSegmentOffsetSGPR, uint16_t(-1));
  }
};

template <>
بنية MappingTraits<Kernel::Metadata> {
  ساكن فراغ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::مفتاح::Name, MD.mName);
    YIO.mapRequired(Kernel::مفتاح::رمز_Name, MD.m_رمز_Name);
    YIO.mapOptional(Kernel::مفتاح::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::مفتاح::LanguageVersion, MD.mLanguageVersion,
                    std::vector<uint32_t>());
    إذا (!MD.mAttrs.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::مفتاح::Attrs, MD.mAttrs);
    إذا (!MD.m_وسائط.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::مفتاح::وسائط, MD.m_وسائط);
    إذا (!MD.mCodeProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::مفتاح::CodeProps, MD.mCodeProps);
    إذا (!MD.mDebugProps.empty() || !YIO.outputting())
      YIO.mapOptional(Kernel::مفتاح::DebugProps, MD.mDebugProps);
  }
};

template <>
بنية MappingTraits<HSAMD::Metadata> {
  ساكن فراغ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(مفتاح::Version, MD.mVersion);
    YIO.mapOptional(مفتاح::Printf, MD.mPrintf, std::vector<std::string>());
    إذا (!MD.mKernels.empty() || !YIO.outputting())
      YIO.mapOptional(مفتاح::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

نطاق AMDGPU {
نطاق HSAMD {

std::error_code fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::مدخل Yaml_مدخل(String);
  Yaml_مدخل >> HSAMetadata;
  أرجع Yaml_مدخل.error();
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
