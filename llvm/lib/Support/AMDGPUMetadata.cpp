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

#ಅನ್ತರ್ಭಾವ "llvm/Support/AMDGPUMetadata.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/YAMLTraits.h"
#ಅನ್ತರ್ಭಾವ <optional>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm::AMDGPU;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::AMDGPU::HSAMD;

LLVM_YAML_IS_SEQUENCE_ಸದಿಶ(Kernel::Arg::Metadata)
LLVM_YAML_IS_SEQUENCE_ಸದಿಶ(Kernel::Metadata)

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ yaml {

ಮಾದರಿ <>
ರಚನೆ ScalarEnumerationTraits<AccessQualifier> {
  ಸ್ಥಿರ ಶೂನ್ಯ enumeration(IO &YIO, AccessQualifier &EN) {
    YIO.enumCase(EN, "Default", AccessQualifier::Default);
    YIO.enumCase(EN, "ReadOnly", AccessQualifier::ReadOnly);
    YIO.enumCase(EN, "WriteOnly", AccessQualifier::WriteOnly);
    YIO.enumCase(EN, "ReadWrite", AccessQualifier::ReadWrite);
  }
};

ಮಾದರಿ <>
ರಚನೆ ScalarEnumerationTraits<ವಿಳಾಸ_Space_Qualifier> {
  ಸ್ಥಿರ ಶೂನ್ಯ enumeration(IO &YIO, ವಿಳಾಸ_Space_Qualifier &EN) {
    YIO.enumCase(EN, "Private", ವಿಳಾಸ_Space_Qualifier::ಖಾಸಗಿ_2);
    YIO.enumCase(EN, "Global", ವಿಳಾಸ_Space_Qualifier::Global);
    YIO.enumCase(EN, "Constant", ವಿಳಾಸ_Space_Qualifier::Constant);
    YIO.enumCase(EN, "Local", ವಿಳಾಸ_Space_Qualifier::Local);
    YIO.enumCase(EN, "Generic", ವಿಳಾಸ_Space_Qualifier::Generic);
    YIO.enumCase(EN, "Region", ವಿಳಾಸ_Space_Qualifier::Region);
  }
};

ಮಾದರಿ <>
ರಚನೆ ScalarEnumerationTraits<ಮೌಲ್ಯ_ಬಗೆ> {
  ಸ್ಥಿರ ಶೂನ್ಯ enumeration(IO &YIO, ಮೌಲ್ಯ_ಬಗೆ &EN) {
    YIO.enumCase(EN, "ByValue", ಮೌಲ್ಯ_ಬಗೆ::By_ಮೌಲ್ಯ);
    YIO.enumCase(EN, "GlobalBuffer", ಮೌಲ್ಯ_ಬಗೆ::Global_ಬಫರ್);
    YIO.enumCase(EN, "DynamicSharedPointer", ಮೌಲ್ಯ_ಬಗೆ::Dynamic_Shared_ಸೂಚಕ);
    YIO.enumCase(EN, "Sampler", ಮೌಲ್ಯ_ಬಗೆ::Sampler);
    YIO.enumCase(EN, "Image", ಮೌಲ್ಯ_ಬಗೆ::Image);
    YIO.enumCase(EN, "Pipe", ಮೌಲ್ಯ_ಬಗೆ::Pipe);
    YIO.enumCase(EN, "Queue", ಮೌಲ್ಯ_ಬಗೆ::ಸರತಿ);
    YIO.enumCase(EN, "HiddenGlobalOffsetX", ಮೌಲ್ಯ_ಬಗೆ::Hidden_Global_ಸ್ಥಾನಾಂತರ_X);
    YIO.enumCase(EN, "HiddenGlobalOffsetY", ಮೌಲ್ಯ_ಬಗೆ::Hidden_Global_ಸ್ಥಾನಾಂತರ_Y);
    YIO.enumCase(EN, "HiddenGlobalOffsetZ", ಮೌಲ್ಯ_ಬಗೆ::Hidden_Global_ಸ್ಥಾನಾಂತರ_Z);
    YIO.enumCase(EN, "HiddenNone", ಮೌಲ್ಯ_ಬಗೆ::HiddenNone);
    YIO.enumCase(EN, "HiddenPrintfBuffer", ಮೌಲ್ಯ_ಬಗೆ::Hidden_Printf_ಬಫರ್);
    YIO.enumCase(EN, "HiddenHostcallBuffer", ಮೌಲ್ಯ_ಬಗೆ::Hidden_Hostcall_ಬಫರ್);
    YIO.enumCase(EN, "HiddenDefaultQueue", ಮೌಲ್ಯ_ಬಗೆ::Hidden_Default_ಸರತಿ);
    YIO.enumCase(EN, "HiddenCompletionAction",
                 ಮೌಲ್ಯ_ಬಗೆ::HiddenCompletionAction);
    YIO.enumCase(EN, "HiddenMultiGridSyncArg",
		 ಮೌಲ್ಯ_ಬಗೆ::HiddenMultiGridSyncArg);
  }
};

ಮಾದರಿ <>
ರಚನೆ ScalarEnumerationTraits<ಮೌಲ್ಯ_Type> {
  ಸ್ಥಿರ ಶೂನ್ಯ enumeration(IO &YIO, ಮೌಲ್ಯ_Type &EN) {
    YIO.enumCase(EN, "Struct", ಮೌಲ್ಯ_Type::Struct);
    YIO.enumCase(EN, "I8", ಮೌಲ್ಯ_Type::I8);
    YIO.enumCase(EN, "U8", ಮೌಲ್ಯ_Type::U8);
    YIO.enumCase(EN, "I16", ಮೌಲ್ಯ_Type::I16);
    YIO.enumCase(EN, "U16", ಮೌಲ್ಯ_Type::U16);
    YIO.enumCase(EN, "F16", ಮೌಲ್ಯ_Type::F16);
    YIO.enumCase(EN, "I32", ಮೌಲ್ಯ_Type::I32);
    YIO.enumCase(EN, "U32", ಮೌಲ್ಯ_Type::U32);
    YIO.enumCase(EN, "F32", ಮೌಲ್ಯ_Type::F32);
    YIO.enumCase(EN, "I64", ಮೌಲ್ಯ_Type::I64);
    YIO.enumCase(EN, "U64", ಮೌಲ್ಯ_Type::U64);
    YIO.enumCase(EN, "F64", ಮೌಲ್ಯ_Type::F64);
  }
};

ಮಾದರಿ <>
ರಚನೆ MappingTraits<Kernel::Attrs::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::Attrs::Metadata &MD) {
    YIO.mapOptional(Kernel::Attrs::ಕೀಲಿ::Reqd_Work_ಗುಂಪು_ಗಾತ್ರ,
                    MD.m_Reqd_Work_ಗುಂಪು_ಗಾತ್ರ, std::ಸದಿಶ<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ಕೀಲಿ::Work_ಗುಂಪು_ಗಾತ್ರ_Hint,
                    MD.m_Work_ಗುಂಪು_ಗಾತ್ರ_Hint, std::ಸದಿಶ<uint32_t>());
    YIO.mapOptional(Kernel::Attrs::ಕೀಲಿ::VecTypeHint,
                    MD.mVecTypeHint, std::string());
    YIO.mapOptional(Kernel::Attrs::ಕೀಲಿ::RuntimeHandle, MD.mRuntimeHandle,
                    std::string());
  }
};

ಮಾದರಿ <>
ರಚನೆ MappingTraits<Kernel::Arg::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::Arg::Metadata &MD) {
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::Name, MD.mName, std::string());
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::TypeName, MD.mTypeName, std::string());
    YIO.mapRequired(Kernel::Arg::ಕೀಲಿ::ಗಾತ್ರ_2, MD.m_ಗಾತ್ರ);
    YIO.mapRequired(Kernel::Arg::ಕೀಲಿ::Align, MD.mAlign);
    YIO.mapRequired(Kernel::Arg::ಕೀಲಿ::ಮೌಲ್ಯ_ಬಗೆ, MD.m_ಮೌಲ್ಯ_ಬಗೆ);

    // Removed. Accepted for parsing compatibility, but not emitted.
    std::optional<ಮೌಲ್ಯ_Type> ಬಳಸದ;
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::ಮೌಲ್ಯ_Type, ಬಳಸದ);

    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::PointeeAlign, MD.mPointeeAlign,
                    uint32_t(0));
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::AddrSpaceQual, MD.mAddrSpaceQual,
                    ವಿಳಾಸ_Space_Qualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::AccQual, MD.mAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::ActualAccQual, MD.mActualAccQual,
                    AccessQualifier::Unknown);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::IsConst, MD.mIsConst, ಸುಳ್ಳು);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::IsRestrict, MD.mIsRestrict, ಸುಳ್ಳು);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::IsVolatile, MD.mIsVolatile, ಸುಳ್ಳು);
    YIO.mapOptional(Kernel::Arg::ಕೀಲಿ::IsPipe, MD.mIsPipe, ಸುಳ್ಳು);
  }
};

ಮಾದರಿ <>
ರಚನೆ MappingTraits<Kernel::ಸಂಕೇತ_Props::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::ಸಂಕೇತ_Props::Metadata &MD) {
    YIO.mapRequired(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::Kernarg_Segment_ಗಾತ್ರ,
                    MD.m_Kernarg_Segment_ಗಾತ್ರ);
    YIO.mapRequired(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::ಗುಂಪು_Segment_Fixed_ಗಾತ್ರ,
                    MD.m_ಗುಂಪು_Segment_Fixed_ಗಾತ್ರ);
    YIO.mapRequired(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::ಖಾಸಗಿ_Segment_Fixed_ಗಾತ್ರ,
                    MD.m_ಖಾಸಗಿ_Segment_Fixed_ಗಾತ್ರ);
    YIO.mapRequired(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::KernargSegmentAlign,
                    MD.mKernargSegmentAlign);
    YIO.mapRequired(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::Wavefront_ಗಾತ್ರ,
                    MD.m_Wavefront_ಗಾತ್ರ);
    YIO.mapOptional(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::ಅಂಕೆ_SGPRs,
                    MD.m_ಅಂಕೆ_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::ಅಂಕೆ_VGPRs,
                    MD.m_ಅಂಕೆ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::Max_Flat_Work_ಗುಂಪು_ಗಾತ್ರ,
                    MD.m_Max_Flat_Work_ಗುಂಪು_ಗಾತ್ರ, uint32_t(0));
    YIO.mapOptional(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::Is_Dynamic_Call_ರಾಶಿ,
                    MD.m_Is_Dynamic_Call_ರಾಶಿ, ಸುಳ್ಳು);
    YIO.mapOptional(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::IsXNACKEnabled,
                    MD.mIsXNACKEnabled, ಸುಳ್ಳು);
    YIO.mapOptional(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::ಅಂಕೆ_Spilled_SGPRs,
                    MD.m_ಅಂಕೆ_Spilled_SGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::ಸಂಕೇತ_Props::ಕೀಲಿ::ಅಂಕೆ_Spilled_VGPRs,
                    MD.m_ಅಂಕೆ_Spilled_VGPRs, uint16_t(0));
  }
};

ಮಾದರಿ <>
ರಚನೆ MappingTraits<Kernel::DebugProps::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::DebugProps::Metadata &MD) {
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::DebuggerABIVersion,
                    MD.mDebuggerABIVersion, std::ಸದಿಶ<uint32_t>());
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::Reserved_ಅಂಕೆ_VGPRs,
                    MD.m_Reserved_ಅಂಕೆ_VGPRs, uint16_t(0));
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::Reserved_ಮೊದಲ_VGPR,
                    MD.m_Reserved_ಮೊದಲ_VGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::ಖಾಸಗಿ_Segment_ಬಫರ್_SGPR,
                    MD.m_ಖಾಸಗಿ_Segment_ಬಫರ್_SGPR, uint16_t(-1));
    YIO.mapOptional(Kernel::DebugProps::ಕೀಲಿ::Wavefront_ಖಾಸಗಿ_Segment_ಸ್ಥಾನಾಂತರ_SGPR,
                    MD.m_Wavefront_ಖಾಸಗಿ_Segment_ಸ್ಥಾನಾಂತರ_SGPR, uint16_t(-1));
  }
};

ಮಾದರಿ <>
ರಚನೆ MappingTraits<Kernel::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, Kernel::Metadata &MD) {
    YIO.mapRequired(Kernel::ಕೀಲಿ::Name, MD.mName);
    YIO.mapRequired(Kernel::ಕೀಲಿ::ಚಿಹ್ನೆ_Name, MD.m_ಚಿಹ್ನೆ_Name);
    YIO.mapOptional(Kernel::ಕೀಲಿ::Language, MD.mLanguage, std::string());
    YIO.mapOptional(Kernel::ಕೀಲಿ::Language_ಆವೃತ್ತಿ, MD.m_Language_ಆವೃತ್ತಿ,
                    std::ಸದಿಶ<uint32_t>());
    ಆದರೆ (!MD.mAttrs.ಖಾಲಿಗಳು() || !YIO.outputting())
      YIO.mapOptional(Kernel::ಕೀಲಿ::Attrs, MD.mAttrs);
    ಆದರೆ (!MD.m_ವಾದಗಳು.ಖಾಲಿಗಳು() || !YIO.outputting())
      YIO.mapOptional(Kernel::ಕೀಲಿ::ವಾದಗಳು, MD.m_ವಾದಗಳು);
    ಆದರೆ (!MD.m_ಸಂಕೇತ_Props.ಖಾಲಿಗಳು() || !YIO.outputting())
      YIO.mapOptional(Kernel::ಕೀಲಿ::ಸಂಕೇತ_Props, MD.m_ಸಂಕೇತ_Props);
    ಆದರೆ (!MD.mDebugProps.ಖಾಲಿಗಳು() || !YIO.outputting())
      YIO.mapOptional(Kernel::ಕೀಲಿ::DebugProps, MD.mDebugProps);
  }
};

ಮಾದರಿ <>
ರಚನೆ MappingTraits<HSAMD::Metadata> {
  ಸ್ಥಿರ ಶೂನ್ಯ mapping(IO &YIO, HSAMD::Metadata &MD) {
    YIO.mapRequired(ಕೀಲಿ::ಆವೃತ್ತಿ, MD.m_ಆವೃತ್ತಿ);
    YIO.mapOptional(ಕೀಲಿ::Printf, MD.mPrintf, std::ಸದಿಶ<std::string>());
    ಆದರೆ (!MD.mKernels.ಖಾಲಿಗಳು() || !YIO.outputting())
      YIO.mapOptional(ಕೀಲಿ::Kernels, MD.mKernels);
  }
};

} // end namespace yaml

ನಾಮಾಕಾಶ AMDGPU {
ನಾಮಾಕಾಶ HSAMD {

std::error_ಸಂಕೇತ fromString(StringRef String, Metadata &HSAMetadata) {
  yaml::ಒಳಹರಿವು Yaml_ಒಳಹರಿವು(String);
  Yaml_ಒಳಹರಿವು >> HSAMetadata;
  ಹಿಂತಿರುಗಿ Yaml_ಒಳಹರಿವು.error();
}

std::error_ಸಂಕೇತ ಗಮ್ಯ_String(Metadata HSAMetadata, std::string &String) {
  raw_string_ostream Yaml_ಹರಿವು(String);
  yaml::Output YamlOutput(Yaml_ಹರಿವು, ಶೂನ್ಯಸೂಚಕ, std::numeric_limits<ಪೂರ್ಣಾಂಕ>::max());
  YamlOutput << HSAMetadata;
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace llvm
