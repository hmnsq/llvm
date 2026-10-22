//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/YAMLTraits.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Support/Casting.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/LineIterator.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/VersionTuple.h"
#ضمن "llvm/Support/YAMLParser.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>
#ضمن <cstdint>
#ضمن <cstring>
#ضمن <string>
#ضمن <vector>

کارول نطاق llvm;
کارول نطاق yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(فراغ *Context) : Ctxt(Context) {}

IO::~IO() = افتراضي;

فراغ *IO::getContext() ثابت {
  أرجع Ctxt;
}

فراغ IO::سېټ_Context(فراغ *Context) {
  Ctxt = Context;
}

فراغ IO::سېټ_Allow_Unknown_کلیګانې(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//ننوت
//===----------------------------------------------------------------------===//

ننوت::ننوت(StringRef ننوت_Content, فراغ *Ctxt,
             سرچینه_Mgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد بهیر(ننوت_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.سېټ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ننوت::ننوت(حافظه_بفر_Ref ننوت, فراغ *Ctxt,
             سرچینه_Mgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد بهیر(ننوت, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.سېټ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ننوت::~ننوت() = افتراضي;

std::error_کوډ ننوت::error() { أرجع EC; }

منطقي ننوت::outputting() ثابت {
  أرجع خطأ;
}

منطقي ننوت::سېټ_Current_Document() {
  إذا (DocIterator != Strm->پای_ټکی()) {
    غوټه *N = DocIterator->get_ریښه();
    إذا (!N) {
      EC = make_error_کوډ(errc::invalid_دلیل);
      أرجع خطأ;
    }

    إذا (isa<Null_غوټه>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع سېټ_Current_Document();
    }
    releaseHNodeBuffers();
    سر_غوټه = createHNodes(N);
    Current_غوټه = سر_غوټه;
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

منطقي ننوت::بل_Document() {
  أرجع ++DocIterator != Strm->پای_ټکی();
}

ثابت غوټه *ننوت::get_Current_غوټه() ثابت {
  أرجع Current_غوټه ? Current_غوټه->غوټه_2 : تش_اشاره;
}

منطقي ننوت::map_نښان(StringRef نښان, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_غوټه)
    أرجع خطأ;

  std::string وموندل_نښان = Current_غوټه->غوټه_2->get_Verbatim_نښان();
  إذا (وموندل_نښان.تشه()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع نښان == وموندل_نښان;
}

فراغ ننوت::beginMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_غوټه);
  إذا (MN) {
    MN->Valid_کلیګانې.clear();
  }
}

std::ویکټور<StringRef> ننوت::کلیګانې() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_غوټه);
  std::ویکټور<StringRef> Ret;
  إذا (!MN) {
    سېټ_Error(Current_غوټه, "not a mapping");
    أرجع Ret;
  }
  لكل (خپلکاری &P : MN->Mapping)
    Ret.ټېل_شا(P.لومړی_2());
  أرجع Ret;
}

منطقي ننوت::preflight_کلی(StringRef کلی, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_غوټه) {
    إذا (Required)
      EC = make_error_کوډ(errc::invalid_دلیل);
    وإلا
      UseDefault = ریښتیا;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_غوټه);
  إذا (!MN) {
    إذا (Required || !isa<تشه_HNode>(Current_غوټه))
      سېټ_Error(Current_غوټه, "not a mapping");
    وإلا
      UseDefault = ریښتیا;
    أرجع خطأ;
  }
  MN->Valid_کلیګانې.ټېل_شا(کلی.str());
  HNode *ارزښت = MN->Mapping[کلی].لومړی_2;
  إذا (!ارزښت) {
    إذا (Required)
      سېټ_Error(Current_غوټه, Twine("missing required key '") + کلی + "'");
    وإلا
      UseDefault = ریښتیا;
    أرجع خطأ;
  }
  SaveInfo = Current_غوټه;
  Current_غوټه = ارزښت;
  أرجع ریښتیا;
}

فراغ ننوت::postflight_کلی(فراغ *saveInfo) {
  Current_غوټه = reinterpret_cast<HNode *>(saveInfo);
}

فراغ ننوت::پای_ټکی_Mapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_غوټه);
  إذا (!MN)
    أرجع;
  لكل (ثابت خپلکاری &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_کلیګانې, NN.لومړی_2())) {
      ثابت SMRange &ReportLoc = NN.دوهم_2.دوهم_2;
      إذا (!Allow_Unknown_کلیګانې) {
        سېټ_Error(ReportLoc, Twine("unknown key '") + NN.لومړی_2() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.لومړی_2() + "'");
    }
  }
}

فراغ ننوت::beginFlowMapping() { beginMapping(); }

فراغ ننوت::پای_ټکی_Flow_Mapping() { پای_ټکی_Mapping(); }

بینښه ننوت::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_غوټه))
    أرجع SQ->Entries.اندازه_2();
  إذا (isa<تشه_HNode>(Current_غوټه))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_غوټه)) {
    إذا (isNull(SN->ارزښت_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  سېټ_Error(Current_غوټه, "not a sequence");
  أرجع 0;
}

فراغ ننوت::پای_ټکی_Sequence() {
}

منطقي ننوت::preflightElement(بینښه شاخص_2, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_غوټه)) {
    SaveInfo = Current_غوټه;
    Current_غوټه = SQ->Entries[شاخص_2];
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

فراغ ننوت::postflightElement(فراغ *SaveInfo) {
  Current_غوټه = reinterpret_cast<HNode *>(SaveInfo);
}

بینښه ننوت::beginFlowSequence() { أرجع beginSequence(); }

منطقي ننوت::preflightFlowElement(بینښه شاخص, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_غوټه)) {
    SaveInfo = Current_غوټه;
    Current_غوټه = SQ->Entries[شاخص];
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

فراغ ننوت::postflightFlowElement(فراغ *SaveInfo) {
  Current_غوټه = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ ننوت::پای_ټکی_Flow_Sequence() {
}

فراغ ننوت::beginEnumScalar() {
  Scalar_سمون_وموندل = خطأ;
}

منطقي ننوت::سمون_Enum_Scalar(StringRef Str, منطقي) {
  إذا (Scalar_سمون_وموندل)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_غوټه)) {
    إذا (SN->ارزښت_2() == Str) {
      Scalar_سمون_وموندل = ریښتیا;
      أرجع ریښتیا;
    }
  }
  أرجع خطأ;
}

منطقي ننوت::سمون_Enum_Fallback() {
  إذا (Scalar_سمون_وموندل)
    أرجع خطأ;
  Scalar_سمون_وموندل = ریښتیا;
  أرجع ریښتیا;
}

فراغ ننوت::پای_ټکی_Enum_Scalar() {
  إذا (!Scalar_سمون_وموندل) {
    سېټ_Error(Current_غوټه, "unknown enumerated scalar");
  }
}

منطقي ننوت::begin_بټ_سېټ_Scalar(منطقي &DoClear) {
  بټ_ارزښتونه_کارول_شوی.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_غوټه)) {
    بټ_ارزښتونه_کارول_شوی.resize(SQ->Entries.اندازه_2());
  } وإلا {
    سېټ_Error(Current_غوټه, "expected sequence of bit values");
  }
  DoClear = ریښتیا;
  أرجع ریښتیا;
}

منطقي ننوت::بټ_سېټ_سمون(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_غوټه)) {
    بینښه شاخص_2 = 0;
    لكل (خپلکاری &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->ارزښت_2() == Str) {
          بټ_ارزښتونه_کارول_شوی[شاخص_2] = ریښتیا;
          أرجع ریښتیا;
        }
      } وإلا {
        سېټ_Error(Current_غوټه, "unexpected scalar in sequence of bit values");
      }
      ++شاخص_2;
    }
  } وإلا {
    سېټ_Error(Current_غوټه, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ ننوت::پای_ټکی_بټ_سېټ_Scalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_غوټه)) {
    أكد(بټ_ارزښتونه_کارول_شوی.اندازه_2() == SQ->Entries.اندازه_2());
    لكل (بینښه i = 0; i < SQ->Entries.اندازه_2(); ++i) {
      إذا (!بټ_ارزښتونه_کارول_شوی[i]) {
        سېټ_Error(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ ننوت::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_غوټه)) {
    S = SN->ارزښت_2();
  } وإلا {
    سېټ_Error(Current_غوټه, "unexpected scalar");
  }
}

فراغ ننوت::بلاک_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ ننوت::scalar_نښان(std::string &نښان) {
  نښان = Current_غوټه->غوټه_2->get_Verbatim_نښان();
}

فراغ ننوت::سېټ_Error(HNode *hnode, ثابت Twine &پیغام_2) {
  أكد(hnode && "HNode must not be NULL");
  سېټ_Error(hnode->غوټه_2, پیغام_2);
}

غوټه_ډول ننوت::get_غوټه_ډول() {
  إذا (isa<ScalarHNode>(Current_غوټه))
    أرجع غوټه_ډول::Scalar;
  وإلا إذا (isa<MapHNode>(Current_غوټه))
    أرجع غوټه_ډول::Map;
  وإلا إذا (isa<SequenceHNode>(Current_غوټه))
    أرجع غوټه_ډول::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ ننوت::سېټ_Error(غوټه *غوټه_3, ثابت Twine &پیغام_2) {
  Strm->printError(غوټه_3, پیغام_2);
  EC = make_error_کوډ(errc::invalid_دلیل);
}

فراغ ننوت::سېټ_Error(ثابت SMRange &range, ثابت Twine &پیغام_2) {
  Strm->printError(range, پیغام_2);
  EC = make_error_کوډ(errc::invalid_دلیل);
}

فراغ ننوت::reportWarning(HNode *hnode, ثابت Twine &پیغام_2) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->غوټه_2, پیغام_2, سرچینه_Mgr::DK_Warning);
}

فراغ ننوت::reportWarning(غوټه *غوټه_3, ثابت Twine &پیغام_2) {
  Strm->printError(غوټه_3, پیغام_2, سرچینه_Mgr::DK_Warning);
}

فراغ ننوت::reportWarning(ثابت SMRange &range, ثابت Twine &پیغام_2) {
  Strm->printError(range, پیغام_2, سرچینه_Mgr::DK_Warning);
}

فراغ ننوت::releaseHNodeBuffers() {
  تشه_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ ننوت::saveAliasHNode(غوټه *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  إذا (!Anchor.تشه())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

ننوت::HNode *ننوت::createHNodes(غوټه *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة غوټه::NK_Scalar: {
    Scalar_غوټه *SN = dyn_cast<Scalar_غوټه>(N);
    StringRef کلی_Str = SN->get_ارزښت(StringStorage);
    إذا (!StringStorage.تشه()) {
      // Copy string to permanent storage
      کلی_Str = StringStorage.str().copy(StringAllocator);
    }
    خپلکاری *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, کلی_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة غوټه::NK_بلاک_Scalar: {
    بلاک_Scalar_غوټه *BSN = dyn_cast<بلاک_Scalar_غوټه>(N);
    StringRef ارزښت_Copy = BSN->get_ارزښت().copy(StringAllocator);
    خپلکاری *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ارزښت_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة غوټه::NK_Sequence: {
    Sequence_غوټه *SQ = dyn_cast<Sequence_غوټه>(N);
    خپلکاری SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (غوټه &SN : *SQ) {
      خپلکاری ثبت = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.ټېل_شا(ثبت);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة غوټه::NK_Mapping: {
    Mapping_غوټه *Map = dyn_cast<Mapping_غوټه>(N);
    خپلکاری mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (کلی_ارزښت_غوټه &KVN : *Map) {
      غوټه *کلی_غوټه = KVN.get_کلی();
      Scalar_غوټه *کلی = dyn_cast_if_present<Scalar_غوټه>(کلی_غوټه);
      غوټه *ارزښت = KVN.get_ارزښت();
      إذا (!کلی || !ارزښت) {
        إذا (!کلی)
          سېټ_Error(کلی_غوټه, "Map key must be a scalar");
        إذا (!ارزښت)
          سېټ_Error(کلی_غوټه, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef کلی_Str = کلی->get_ارزښت(StringStorage);
      إذا (!StringStorage.تشه()) {
        // Copy string to permanent storage
        کلی_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.شمېر(کلی_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        سېټ_Error(کلی_غوټه, Twine("duplicated mapping key '") + کلی_Str + "'");
      خپلکاری ارزښت_HNode = createHNodes(ارزښت);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[کلی_Str] =
          std::make_pair(std::move(ارزښت_HNode), کلی_غوټه->get_سرچینه_Range());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة غوټه::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (تشه_HNode_Allocator.Allocate()) تشه_HNode(N);
  حالة غوټه::NK_Alias: {
    Alias_غوټه *AN = dyn_cast<Alias_غوټه>(N);
    خپلکاری AliasName = AN->getName();
    خپلکاری AHN = AliasMap.پیدا_کړه(AliasName);
    إذا (AHN == AliasMap.پای_ټکی()) {
      سېټ_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      أرجع تش_اشاره;
    }
    أرجع AHN->دوهم_2;
  }
  افتراضي:
    سېټ_Error(N, "unknown node kind");
    أرجع تش_اشاره;
  }
}

فراغ ننوت::سېټ_Error(ثابت Twine &پیغام_3) {
  سېټ_Error(Current_غوټه, پیغام_3);
}

فراغ ننوت::سېټ_Allow_Unknown_کلیګانې(منطقي Allow) { Allow_Unknown_کلیګانې = Allow; }

منطقي ننوت::can_Elide_تشه_Sequence() {
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, فراغ *context, صحيح Wrap_ستنه)
    : IO(context), بهر(yout), Wrap_ستنه(Wrap_ستنه) {}

Output::~Output() = افتراضي;

منطقي Output::outputting() ثابت {
  أرجع ریښتیا;
}

فراغ Output::beginMapping() {
  State_ډېری.ټېل_شا(دننه_Map_لومړی_کلی);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::map_نښان(StringRef نښان, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_ډېری.اندازه_2() > 1) {
      خپلکاری &E = State_ډېری[State_ډېری.اندازه_2() - 2];
      SequenceElement = دننه_Seq_Any_Element(E) || دننه_Flow_Seq_Any_Element(E);
    }
    إذا (SequenceElement && State_ډېری.شا() == دننه_Map_لومړی_کلی) {
      new_کرښه_Check();
    } وإلا {
      output(" ");
    }
    output(نښان);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_ډېری.شا() == دننه_Map_لومړی_کلی) {
        State_ډېری.راوباسه_شا();
        State_ډېری.ټېل_شا(دننه_Map_Other_کلی);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  أرجع Use;
}

فراغ Output::پای_ټکی_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  إذا (State_ډېری.شا() == دننه_Map_لومړی_کلی) {
    Padding = PaddingBeforeContainer;
    new_کرښه_Check();
    output("{}");
    Padding = "\n";
  }
  State_ډېری.راوباسه_شا();
}

std::ویکټور<StringRef> Output::کلیګانې() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_کلی(StringRef کلی, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = تش_اشاره;
  إذا (Required || !SameAsDefault || Write_Default_ارزښتونه) {
    خپلکاری State = State_ډېری.شا();
    إذا (State == دننه_Flow_Map_لومړی_کلی || State == دننه_Flow_Map_Other_کلی) {
      flow_کلی(کلی);
    } وإلا {
      new_کرښه_Check();
      padded_کلی(کلی);
    }
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

فراغ Output::postflight_کلی(فراغ *) {
  إذا (State_ډېری.شا() == دننه_Map_لومړی_کلی) {
    State_ډېری.راوباسه_شا();
    State_ډېری.ټېل_شا(دننه_Map_Other_کلی);
  } وإلا إذا (State_ډېری.شا() == دننه_Flow_Map_لومړی_کلی) {
    State_ډېری.راوباسه_شا();
    State_ډېری.ټېل_شا(دننه_Flow_Map_Other_کلی);
  }
}

فراغ Output::beginFlowMapping() {
  State_ډېری.ټېل_شا(دننه_Flow_Map_لومړی_کلی);
  new_کرښه_Check();
  ستنه_At_Map_Flow_پیل = ستنه;
  output("{ ");
}

فراغ Output::پای_ټکی_Flow_Mapping() {
  State_ډېری.راوباسه_شا();
  output_پورته_منزل_پای_ټکی_د_کرښه(" }");
}

فراغ Output::beginDocuments() {
  output_پورته_منزل_پای_ټکی_د_کرښه("---");
}

منطقي Output::preflightDocument(بینښه شاخص) {
  إذا (شاخص > 0)
    output_پورته_منزل_پای_ټکی_د_کرښه("\n---");
  أرجع ریښتیا;
}

فراغ Output::postflightDocument() {
}

فراغ Output::پای_ټکی_Documents() {
  output("\n...\n");
}

بینښه Output::beginSequence() {
  State_ډېری.ټېل_شا(دننه_Seq_لومړی_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::پای_ټکی_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_ډېری.شا() == دننه_Seq_لومړی_Element) {
    Padding = PaddingBeforeContainer;
    new_کرښه_Check(/*EmptySequence=*/ریښتیا);
    output("[]");
    Padding = "\n";
  }
  State_ډېری.راوباسه_شا();
}

منطقي Output::preflightElement(بینښه, فراغ *&SaveInfo) {
  SaveInfo = تش_اشاره;
  أرجع ریښتیا;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_ډېری.شا() == دننه_Seq_لومړی_Element) {
    State_ډېری.راوباسه_شا();
    State_ډېری.ټېل_شا(دننه_Seq_Other_Element);
  } وإلا إذا (State_ډېری.شا() == دننه_Flow_Seq_لومړی_Element) {
    State_ډېری.راوباسه_شا();
    State_ډېری.ټېل_شا(دننه_Flow_Seq_Other_Element);
  }
}

بینښه Output::beginFlowSequence() {
  State_ډېری.ټېل_شا(دننه_Flow_Seq_لومړی_Element);
  new_کرښه_Check();
  ستنه_At_Flow_پیل = ستنه;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::پای_ټکی_Flow_Sequence() {
  State_ډېری.راوباسه_شا();
  output_پورته_منزل_پای_ټکی_د_کرښه(" ]");
}

منطقي Output::preflightFlowElement(بینښه, فراغ *&SaveInfo) {
  إذا (NeedFlowSequenceComma)
    output(", ");
  إذا (Wrap_ستنه && ستنه > Wrap_ستنه) {
    output("\n");
    لكل (صحيح i = 0; i < ستنه_At_Flow_پیل; ++i)
      output(" ");
    ستنه = ستنه_At_Flow_پیل;
    output("  ");
  }
  SaveInfo = تش_اشاره;
  أرجع ریښتیا;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = ریښتیا;
}

فراغ Output::beginEnumScalar() {
  Enumeration_سمون_وموندل = خطأ;
}

منطقي Output::سمون_Enum_Scalar(StringRef Str, منطقي سمون_2) {
  إذا (سمون_2 && !Enumeration_سمون_وموندل) {
    new_کرښه_Check();
    output_پورته_منزل_پای_ټکی_د_کرښه(Str);
    Enumeration_سمون_وموندل = ریښتیا;
  }
  أرجع خطأ;
}

منطقي Output::سمون_Enum_Fallback() {
  إذا (Enumeration_سمون_وموندل)
    أرجع خطأ;
  Enumeration_سمون_وموندل = ریښتیا;
  أرجع ریښتیا;
}

فراغ Output::پای_ټکی_Enum_Scalar() {
  إذا (!Enumeration_سمون_وموندل)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::begin_بټ_سېټ_Scalar(منطقي &DoClear) {
  new_کرښه_Check();
  output("[ ");
  Need_بټ_ارزښت_Comma = خطأ;
  DoClear = خطأ;
  أرجع ریښتیا;
}

منطقي Output::بټ_سېټ_سمون(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_بټ_ارزښت_Comma)
      output(", ");
    output(Str);
    Need_بټ_ارزښت_Comma = ریښتیا;
  }
  أرجع خطأ;
}

فراغ Output::پای_ټکی_بټ_سېټ_Scalar() {
  output_پورته_منزل_پای_ټکی_د_کرښه(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_کرښه_Check();
  إذا (S.تشه()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_پورته_منزل_پای_ټکی_د_کرښه("''");
    أرجع;
  }
  output(S, MustQuote);
  output_پورته_منزل_پای_ټکی_د_کرښه("");
}

فراغ Output::بلاک_Scalar_String(StringRef &S) {
  إذا (!State_ډېری.تشه())
    new_کرښه_Check();
  output(" |");

  بینښه Indent = State_ډېری.تشه() ? 1 : State_ډېری.اندازه_2();

  خپلکاری بفر = حافظه_بفر::get_Mem_بفر(S, "", خطأ);
  لكل (کرښه_iterator کرښې(*بفر, خطأ); !کرښې.is_at_پای_ټکی(); ++کرښې) {
    output_New_کرښه();
    لكل (بینښه I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*کرښې);
  }
  output_پورته_منزل_پای_ټکی_د_کرښه("");
}

فراغ Output::scalar_نښان(std::string &نښان) {
  إذا (نښان.تشه())
    أرجع;
  new_کرښه_Check();
  output(نښان);
  output(" ");
}

فراغ Output::سېټ_Error(ثابت Twine &پیغام_2) {
}

std::error_کوډ Output::error() { أرجع {}; }

منطقي Output::can_Elide_تشه_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  إذا (State_ډېری.اندازه_2() < 2)
    أرجع ریښتیا;
  إذا (State_ډېری.شا() != دننه_Map_لومړی_کلی)
    أرجع ریښتیا;
  أرجع !دننه_Seq_Any_Element(State_ډېری[State_ډېری.اندازه_2() - 2]);
}

فراغ Output::output(StringRef s) {
  ستنه += s.اندازه_2();
  بهر << s;
}

فراغ Output::output(StringRef S, QuotingType MustQuote) {
  إذا (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    أرجع;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  إذا (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ خطأ));
    output(Quote);
    أرجع;
  }

  بینښه i = 0;
  بینښه j = 0;
  بینښه پای_ټکی_2 = S.اندازه_2();
  ثابت حرف *بنسټ = S.معلومات();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  طالما (j < پای_ټکی_2) {
    إذا (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&بنسټ[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&بنسټ[i], j - i));
  output(Quote); // Ending quote.
}

فراغ Output::output_پورته_منزل_پای_ټکی_د_کرښه(StringRef s) {
  output(s);
  إذا (State_ډېری.تشه() || (!دننه_Flow_Seq_Any_Element(State_ډېری.شا()) &&
                             !دننه_Flow_Map_Any_کلی(State_ډېری.شا())))
    Padding = "\n";
}

فراغ Output::output_New_کرښه() {
  بهر << "\n";
  ستنه = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_کرښه_Check(منطقي تشه_Sequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_کرښه();
  Padding = {};

  إذا (State_ډېری.اندازه_2() == 0 || تشه_Sequence)
    أرجع;

  بینښه Indent = State_ډېری.اندازه_2() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  خپلکاری I = State_ډېری.rbegin(), E = State_ډېری.rend();

  إذا (دننه_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = ریښتیا; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == دننه_Map_لومړی_کلی || *I == دننه_Flow_Map_لومړی_کلی ||
             دننه_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = ریښتیا;
    ++I; // Skip back().
  }

  بینښه Output_Dash_شمېر = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!دننه_Seq_Any_Element(*I))
        اكسر;

      ++Output_Dash_شمېر;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != دننه_Seq_لومړی_Element)
        اكسر;
    }
  }

  لكل (بینښه I = Output_Dash_شمېر; I < Indent; ++I)
    output("  ");

  لكل (بینښه I = 0; I < Output_Dash_شمېر; ++I)
    output("- ");
}

فراغ Output::padded_کلی(StringRef کلی_2) {
  output(کلی_2, needsQuotes(کلی_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (کلی_2.اندازه_2() < مزي_اوږدوالی(spaces))
    Padding = &spaces[کلی_2.اندازه_2()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_کلی(StringRef کلی) {
  إذا (State_ډېری.شا() == دننه_Flow_Map_Other_کلی)
    output(", ");
  إذا (Wrap_ستنه && ستنه > Wrap_ستنه) {
    output("\n");
    لكل (صحيح I = 0; I < ستنه_At_Map_Flow_پیل; ++I)
      output(" ");
    ستنه = ستنه_At_Map_Flow_پیل;
    output("  ");
  }
  output(کلی, needsQuotes(کلی, خطأ));
  output(": ");
}

غوټه_ډول Output::get_غوټه_ډول() { report_fatal_error("invalid call"); }

منطقي Output::دننه_Seq_Any_Element(دننه_State State) {
  أرجع State == دننه_Seq_لومړی_Element || State == دننه_Seq_Other_Element;
}

منطقي Output::دننه_Flow_Seq_Any_Element(دننه_State State) {
  أرجع State == دننه_Flow_Seq_لومړی_Element || State == دننه_Flow_Seq_Other_Element;
}

منطقي Output::دننه_Map_Any_کلی(دننه_State State) {
  أرجع State == دننه_Map_لومړی_کلی || State == دننه_Map_Other_کلی;
}

منطقي Output::دننه_Flow_Map_Any_کلی(دننه_State State) {
  أرجع State == دننه_Flow_Map_لومړی_کلی || State == دننه_Flow_Map_Other_کلی;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &بهر) {
  بهر << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::ننوت_2(StringRef Scalar, فراغ *, منطقي &Val) {
  إذا (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    أرجع StringRef();
  }
  أرجع "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &بهر) {
  بهر << Val;
}

StringRef ScalarTraits<StringRef>::ننوت_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &بهر) {
  بهر << Val;
}

StringRef ScalarTraits<std::string>::ننوت_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &بهر) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t عدد = Val;
  بهر << عدد;
}

StringRef ScalarTraits<uint8_t>::ننوت_2(StringRef Scalar, فراغ *, uint8_t &Val) {
  بینښه اوږد اوږد n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &بهر) {
  بهر << Val;
}

StringRef ScalarTraits<uint16_t>::ننوت_2(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  بینښه اوږد اوږد n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &بهر) {
  بهر << Val;
}

StringRef ScalarTraits<uint32_t>::ننوت_2(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  بینښه اوږد اوږد n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &بهر) {
  بهر << Val;
}

StringRef ScalarTraits<uint64_t>::ننوت_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  بینښه اوږد اوږد N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &بهر) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t عدد = Val;
  بهر << عدد;
}

StringRef ScalarTraits<int8_t>::ننوت_2(StringRef Scalar, فراغ *, int8_t &Val) {
  اوږد اوږد N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > 127) || (N < -128))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &بهر) {
  بهر << Val;
}

StringRef ScalarTraits<int16_t>::ننوت_2(StringRef Scalar, فراغ *, int16_t &Val) {
  اوږد اوږد N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT16_MAX) || (N < INT16_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &بهر) {
  بهر << Val;
}

StringRef ScalarTraits<int32_t>::ننوت_2(StringRef Scalar, فراغ *, int32_t &Val) {
  اوږد اوږد N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT32_MAX) || (N < INT32_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &بهر) {
  بهر << Val;
}

StringRef ScalarTraits<int64_t>::ننوت_2(StringRef Scalar, فراغ *, int64_t &Val) {
  اوږد اوږد N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &بهر) {
  بهر << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::ننوت_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (منزل_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &بهر) {
  بهر << format("%g", Val);
}

StringRef ScalarTraits<عائم>::ننوت_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (منزل_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &بهر) {
  بهر << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::ننوت_2(StringRef Scalar, فراغ *, Hex8 &Val) {
  بینښه اوږد اوږد n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex8 number";
  إذا (n > 0xFF)
    أرجع "out of range hex8 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &بهر) {
  بهر << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::ننوت_2(StringRef Scalar, فراغ *, Hex16 &Val) {
  بینښه اوږد اوږد n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex16 number";
  إذا (n > 0xFFFF)
    أرجع "out of range hex16 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &بهر) {
  بهر << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::ننوت_2(StringRef Scalar, فراغ *, Hex32 &Val) {
  بینښه اوږد اوږد n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex32 number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range hex32 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &بهر) {
  بهر << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::ننوت_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  بینښه اوږد اوږد عدد;
  إذا (getAsUnsignedInteger(Scalar, 0, عدد))
    أرجع "invalid hex64 number";
  Val = عدد;
  أرجع StringRef();
}

فراغ ScalarTraits<نسخه_Tuple>::output(ثابت نسخه_Tuple &Val, فراغ *,
                                        llvm::raw_ostream &بهر) {
  بهر << Val.getAsString();
}

StringRef ScalarTraits<نسخه_Tuple>::ننوت_2(StringRef Scalar, فراغ *,
                                            نسخه_Tuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
