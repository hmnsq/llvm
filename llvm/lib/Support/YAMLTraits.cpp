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

استفاده نطاق llvm;
استفاده نطاق yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(فراغ *Context) : Ctxt(Context) {}

IO::~IO() = افتراضي;

فراغ *IO::getContext() ثابت {
  أرجع Ctxt;
}

فراغ IO::مجموعه_Context(فراغ *Context) {
  Ctxt = Context;
}

فراغ IO::مجموعه_Allow_Unknown_کلیدها(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//درون داد
//===----------------------------------------------------------------------===//

درون_داد::درون_داد(StringRef درون_داد_Content, فراغ *Ctxt,
             منبع_Mgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد جریان(درون_داد_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.مجموعه_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

درون_داد::درون_داد(حافظه_میانگیر_Ref درون_داد, فراغ *Ctxt,
             منبع_Mgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد جریان(درون_داد, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.مجموعه_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

درون_داد::~درون_داد() = افتراضي;

std::error_کد درون_داد::error() { أرجع EC; }

منطقي درون_داد::outputting() ثابت {
  أرجع خطأ;
}

منطقي درون_داد::مجموعه_Current_Document() {
  إذا (DocIterator != Strm->انتها()) {
    گره *N = DocIterator->get_ریشه();
    إذا (!N) {
      EC = make_error_کد(errc::invalid_آرگومان);
      أرجع خطأ;
    }

    إذا (isa<Null_گره>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع مجموعه_Current_Document();
    }
    releaseHNodeBuffers();
    بالاترین_گره = createHNodes(N);
    Current_گره = بالاترین_گره;
    أرجع درست;
  }
  أرجع خطأ;
}

منطقي درون_داد::بعدی_Document() {
  أرجع ++DocIterator != Strm->انتها();
}

ثابت گره *درون_داد::get_Current_گره() ثابت {
  أرجع Current_گره ? Current_گره->گره_2 : اشاره_تهی;
}

منطقي درون_داد::map_برچسب(StringRef برچسب, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_گره)
    أرجع خطأ;

  std::string یافت_برچسب = Current_گره->گره_2->get_Verbatim_برچسب();
  إذا (یافت_برچسب.خالی()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع برچسب == یافت_برچسب;
}

فراغ درون_داد::beginMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گره);
  إذا (MN) {
    MN->Valid_کلیدها.clear();
  }
}

std::وکتور<StringRef> درون_داد::کلیدها() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_گره);
  std::وکتور<StringRef> Ret;
  إذا (!MN) {
    مجموعه_Error(Current_گره, "not a mapping");
    أرجع Ret;
  }
  لكل (خودکار &P : MN->Mapping)
    Ret.افزودن_پشت(P.اولی_2());
  أرجع Ret;
}

منطقي درون_داد::preflight_کلید(StringRef کلید, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_گره) {
    إذا (Required)
      EC = make_error_کد(errc::invalid_آرگومان);
    وإلا
      UseDefault = درست;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_گره);
  إذا (!MN) {
    إذا (Required || !isa<خالی_HNode>(Current_گره))
      مجموعه_Error(Current_گره, "not a mapping");
    وإلا
      UseDefault = درست;
    أرجع خطأ;
  }
  MN->Valid_کلیدها.افزودن_پشت(کلید.str());
  HNode *مقدار = MN->Mapping[کلید].اولی_2;
  إذا (!مقدار) {
    إذا (Required)
      مجموعه_Error(Current_گره, Twine("missing required key '") + کلید + "'");
    وإلا
      UseDefault = درست;
    أرجع خطأ;
  }
  SaveInfo = Current_گره;
  Current_گره = مقدار;
  أرجع درست;
}

فراغ درون_داد::postflight_کلید(فراغ *saveInfo) {
  Current_گره = reinterpret_cast<HNode *>(saveInfo);
}

فراغ درون_داد::انتها_Mapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گره);
  إذا (!MN)
    أرجع;
  لكل (ثابت خودکار &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_کلیدها, NN.اولی_2())) {
      ثابت SMRange &ReportLoc = NN.دومی_2.دومی_2;
      إذا (!Allow_Unknown_کلیدها) {
        مجموعه_Error(ReportLoc, Twine("unknown key '") + NN.اولی_2() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.اولی_2() + "'");
    }
  }
}

فراغ درون_داد::beginFlowMapping() { beginMapping(); }

فراغ درون_داد::انتها_Flow_Mapping() { انتها_Mapping(); }

بیعلامت درون_داد::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره))
    أرجع SQ->Entries.اندازه_2();
  إذا (isa<خالی_HNode>(Current_گره))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گره)) {
    إذا (isNull(SN->مقدار_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  مجموعه_Error(Current_گره, "not a sequence");
  أرجع 0;
}

فراغ درون_داد::انتها_Sequence() {
}

منطقي درون_داد::preflightElement(بیعلامت شاخص_2, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    SaveInfo = Current_گره;
    Current_گره = SQ->Entries[شاخص_2];
    أرجع درست;
  }
  أرجع خطأ;
}

فراغ درون_داد::postflightElement(فراغ *SaveInfo) {
  Current_گره = reinterpret_cast<HNode *>(SaveInfo);
}

بیعلامت درون_داد::beginFlowSequence() { أرجع beginSequence(); }

منطقي درون_داد::preflightFlowElement(بیعلامت شاخص, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    SaveInfo = Current_گره;
    Current_گره = SQ->Entries[شاخص];
    أرجع درست;
  }
  أرجع خطأ;
}

فراغ درون_داد::postflightFlowElement(فراغ *SaveInfo) {
  Current_گره = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ درون_داد::انتها_Flow_Sequence() {
}

فراغ درون_داد::beginEnumScalar() {
  Scalar_تطابق_یافت = خطأ;
}

منطقي درون_داد::تطابق_Enum_Scalar(StringRef Str, منطقي) {
  إذا (Scalar_تطابق_یافت)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گره)) {
    إذا (SN->مقدار_2() == Str) {
      Scalar_تطابق_یافت = درست;
      أرجع درست;
    }
  }
  أرجع خطأ;
}

منطقي درون_داد::تطابق_Enum_Fallback() {
  إذا (Scalar_تطابق_یافت)
    أرجع خطأ;
  Scalar_تطابق_یافت = درست;
  أرجع درست;
}

فراغ درون_داد::انتها_Enum_Scalar() {
  إذا (!Scalar_تطابق_یافت) {
    مجموعه_Error(Current_گره, "unknown enumerated scalar");
  }
}

منطقي درون_داد::begin_بیت_مجموعه_Scalar(منطقي &DoClear) {
  بیت_مقادیر_استفاده_شده.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    بیت_مقادیر_استفاده_شده.resize(SQ->Entries.اندازه_2());
  } وإلا {
    مجموعه_Error(Current_گره, "expected sequence of bit values");
  }
  DoClear = درست;
  أرجع درست;
}

منطقي درون_داد::بیت_مجموعه_تطابق(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    بیعلامت شاخص_2 = 0;
    لكل (خودکار &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->مقدار_2() == Str) {
          بیت_مقادیر_استفاده_شده[شاخص_2] = درست;
          أرجع درست;
        }
      } وإلا {
        مجموعه_Error(Current_گره, "unexpected scalar in sequence of bit values");
      }
      ++شاخص_2;
    }
  } وإلا {
    مجموعه_Error(Current_گره, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ درون_داد::انتها_بیت_مجموعه_Scalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گره)) {
    أكد(بیت_مقادیر_استفاده_شده.اندازه_2() == SQ->Entries.اندازه_2());
    لكل (بیعلامت i = 0; i < SQ->Entries.اندازه_2(); ++i) {
      إذا (!بیت_مقادیر_استفاده_شده[i]) {
        مجموعه_Error(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ درون_داد::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گره)) {
    S = SN->مقدار_2();
  } وإلا {
    مجموعه_Error(Current_گره, "unexpected scalar");
  }
}

فراغ درون_داد::بلوک_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ درون_داد::scalar_برچسب(std::string &برچسب) {
  برچسب = Current_گره->گره_2->get_Verbatim_برچسب();
}

فراغ درون_داد::مجموعه_Error(HNode *hnode, ثابت Twine &پیام_2) {
  أكد(hnode && "HNode must not be NULL");
  مجموعه_Error(hnode->گره_2, پیام_2);
}

گره_نوع درون_داد::get_گره_نوع() {
  إذا (isa<ScalarHNode>(Current_گره))
    أرجع گره_نوع::Scalar;
  وإلا إذا (isa<MapHNode>(Current_گره))
    أرجع گره_نوع::Map;
  وإلا إذا (isa<SequenceHNode>(Current_گره))
    أرجع گره_نوع::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ درون_داد::مجموعه_Error(گره *گره_3, ثابت Twine &پیام_2) {
  Strm->printError(گره_3, پیام_2);
  EC = make_error_کد(errc::invalid_آرگومان);
}

فراغ درون_داد::مجموعه_Error(ثابت SMRange &range, ثابت Twine &پیام_2) {
  Strm->printError(range, پیام_2);
  EC = make_error_کد(errc::invalid_آرگومان);
}

فراغ درون_داد::reportWarning(HNode *hnode, ثابت Twine &پیام_2) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->گره_2, پیام_2, منبع_Mgr::DK_Warning);
}

فراغ درون_داد::reportWarning(گره *گره_3, ثابت Twine &پیام_2) {
  Strm->printError(گره_3, پیام_2, منبع_Mgr::DK_Warning);
}

فراغ درون_داد::reportWarning(ثابت SMRange &range, ثابت Twine &پیام_2) {
  Strm->printError(range, پیام_2, منبع_Mgr::DK_Warning);
}

فراغ درون_داد::releaseHNodeBuffers() {
  خالی_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ درون_داد::saveAliasHNode(گره *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  إذا (!Anchor.خالی())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

درون_داد::HNode *درون_داد::createHNodes(گره *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة گره::NK_Scalar: {
    Scalar_گره *SN = dyn_cast<Scalar_گره>(N);
    StringRef کلید_Str = SN->get_مقدار(StringStorage);
    إذا (!StringStorage.خالی()) {
      // Copy string to permanent storage
      کلید_Str = StringStorage.str().copy(StringAllocator);
    }
    خودکار *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, کلید_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة گره::NK_بلوک_Scalar: {
    بلوک_Scalar_گره *BSN = dyn_cast<بلوک_Scalar_گره>(N);
    StringRef مقدار_Copy = BSN->get_مقدار().copy(StringAllocator);
    خودکار *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, مقدار_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة گره::NK_Sequence: {
    Sequence_گره *SQ = dyn_cast<Sequence_گره>(N);
    خودکار SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (گره &SN : *SQ) {
      خودکار مدخل = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.افزودن_پشت(مدخل);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة گره::NK_Mapping: {
    Mapping_گره *Map = dyn_cast<Mapping_گره>(N);
    خودکار mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (کلید_مقدار_گره &KVN : *Map) {
      گره *کلید_گره = KVN.get_کلید();
      Scalar_گره *کلید = dyn_cast_if_present<Scalar_گره>(کلید_گره);
      گره *مقدار = KVN.get_مقدار();
      إذا (!کلید || !مقدار) {
        إذا (!کلید)
          مجموعه_Error(کلید_گره, "Map key must be a scalar");
        إذا (!مقدار)
          مجموعه_Error(کلید_گره, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef کلید_Str = کلید->get_مقدار(StringStorage);
      إذا (!StringStorage.خالی()) {
        // Copy string to permanent storage
        کلید_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.شمار(کلید_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        مجموعه_Error(کلید_گره, Twine("duplicated mapping key '") + کلید_Str + "'");
      خودکار مقدار_HNode = createHNodes(مقدار);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[کلید_Str] =
          std::make_pair(std::move(مقدار_HNode), کلید_گره->get_منبع_Range());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة گره::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (خالی_HNode_Allocator.Allocate()) خالی_HNode(N);
  حالة گره::NK_Alias: {
    Alias_گره *AN = dyn_cast<Alias_گره>(N);
    خودکار AliasName = AN->getName();
    خودکار AHN = AliasMap.پیدا_کن(AliasName);
    إذا (AHN == AliasMap.انتها()) {
      مجموعه_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      أرجع اشاره_تهی;
    }
    أرجع AHN->دومی_2;
  }
  افتراضي:
    مجموعه_Error(N, "unknown node kind");
    أرجع اشاره_تهی;
  }
}

فراغ درون_داد::مجموعه_Error(ثابت Twine &پیام_3) {
  مجموعه_Error(Current_گره, پیام_3);
}

فراغ درون_داد::مجموعه_Allow_Unknown_کلیدها(منطقي Allow) { Allow_Unknown_کلیدها = Allow; }

منطقي درون_داد::can_Elide_خالی_Sequence() {
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, فراغ *context, صحيح Wrap_ستونی)
    : IO(context), بیرون(yout), Wrap_ستونی(Wrap_ستونی) {}

Output::~Output() = افتراضي;

منطقي Output::outputting() ثابت {
  أرجع درست;
}

فراغ Output::beginMapping() {
  State_پشته.افزودن_پشت(درون_Map_اولی_کلید);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::map_برچسب(StringRef برچسب, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_پشته.اندازه_2() > 1) {
      خودکار &E = State_پشته[State_پشته.اندازه_2() - 2];
      SequenceElement = درون_Seq_Any_Element(E) || درون_Flow_Seq_Any_Element(E);
    }
    إذا (SequenceElement && State_پشته.پشت() == درون_Map_اولی_کلید) {
      new_سطر_Check();
    } وإلا {
      output(" ");
    }
    output(برچسب);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_پشته.پشت() == درون_Map_اولی_کلید) {
        State_پشته.برداشت_پشت();
        State_پشته.افزودن_پشت(درون_Map_Other_کلید);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  أرجع Use;
}

فراغ Output::انتها_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  إذا (State_پشته.پشت() == درون_Map_اولی_کلید) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check();
    output("{}");
    Padding = "\n";
  }
  State_پشته.برداشت_پشت();
}

std::وکتور<StringRef> Output::کلیدها() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_کلید(StringRef کلید, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = اشاره_تهی;
  إذا (Required || !SameAsDefault || Write_Default_مقادیر) {
    خودکار State = State_پشته.پشت();
    إذا (State == درون_Flow_Map_اولی_کلید || State == درون_Flow_Map_Other_کلید) {
      flow_کلید(کلید);
    } وإلا {
      new_سطر_Check();
      padded_کلید(کلید);
    }
    أرجع درست;
  }
  أرجع خطأ;
}

فراغ Output::postflight_کلید(فراغ *) {
  إذا (State_پشته.پشت() == درون_Map_اولی_کلید) {
    State_پشته.برداشت_پشت();
    State_پشته.افزودن_پشت(درون_Map_Other_کلید);
  } وإلا إذا (State_پشته.پشت() == درون_Flow_Map_اولی_کلید) {
    State_پشته.برداشت_پشت();
    State_پشته.افزودن_پشت(درون_Flow_Map_Other_کلید);
  }
}

فراغ Output::beginFlowMapping() {
  State_پشته.افزودن_پشت(درون_Flow_Map_اولی_کلید);
  new_سطر_Check();
  ستونی_At_Map_Flow_شروع = ستونی;
  output("{ ");
}

فراغ Output::انتها_Flow_Mapping() {
  State_پشته.برداشت_پشت();
  output_به_بالا_مقصد_انتها_از_سطر(" }");
}

فراغ Output::beginDocuments() {
  output_به_بالا_مقصد_انتها_از_سطر("---");
}

منطقي Output::preflightDocument(بیعلامت شاخص) {
  إذا (شاخص > 0)
    output_به_بالا_مقصد_انتها_از_سطر("\n---");
  أرجع درست;
}

فراغ Output::postflightDocument() {
}

فراغ Output::انتها_Documents() {
  output("\n...\n");
}

بیعلامت Output::beginSequence() {
  State_پشته.افزودن_پشت(درون_Seq_اولی_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::انتها_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_پشته.پشت() == درون_Seq_اولی_Element) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check(/*EmptySequence=*/درست);
    output("[]");
    Padding = "\n";
  }
  State_پشته.برداشت_پشت();
}

منطقي Output::preflightElement(بیعلامت, فراغ *&SaveInfo) {
  SaveInfo = اشاره_تهی;
  أرجع درست;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_پشته.پشت() == درون_Seq_اولی_Element) {
    State_پشته.برداشت_پشت();
    State_پشته.افزودن_پشت(درون_Seq_Other_Element);
  } وإلا إذا (State_پشته.پشت() == درون_Flow_Seq_اولی_Element) {
    State_پشته.برداشت_پشت();
    State_پشته.افزودن_پشت(درون_Flow_Seq_Other_Element);
  }
}

بیعلامت Output::beginFlowSequence() {
  State_پشته.افزودن_پشت(درون_Flow_Seq_اولی_Element);
  new_سطر_Check();
  ستونی_At_Flow_شروع = ستونی;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::انتها_Flow_Sequence() {
  State_پشته.برداشت_پشت();
  output_به_بالا_مقصد_انتها_از_سطر(" ]");
}

منطقي Output::preflightFlowElement(بیعلامت, فراغ *&SaveInfo) {
  إذا (NeedFlowSequenceComma)
    output(", ");
  إذا (Wrap_ستونی && ستونی > Wrap_ستونی) {
    output("\n");
    لكل (صحيح i = 0; i < ستونی_At_Flow_شروع; ++i)
      output(" ");
    ستونی = ستونی_At_Flow_شروع;
    output("  ");
  }
  SaveInfo = اشاره_تهی;
  أرجع درست;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = درست;
}

فراغ Output::beginEnumScalar() {
  Enumeration_تطابق_یافت = خطأ;
}

منطقي Output::تطابق_Enum_Scalar(StringRef Str, منطقي تطابق_2) {
  إذا (تطابق_2 && !Enumeration_تطابق_یافت) {
    new_سطر_Check();
    output_به_بالا_مقصد_انتها_از_سطر(Str);
    Enumeration_تطابق_یافت = درست;
  }
  أرجع خطأ;
}

منطقي Output::تطابق_Enum_Fallback() {
  إذا (Enumeration_تطابق_یافت)
    أرجع خطأ;
  Enumeration_تطابق_یافت = درست;
  أرجع درست;
}

فراغ Output::انتها_Enum_Scalar() {
  إذا (!Enumeration_تطابق_یافت)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::begin_بیت_مجموعه_Scalar(منطقي &DoClear) {
  new_سطر_Check();
  output("[ ");
  Need_بیت_مقدار_Comma = خطأ;
  DoClear = خطأ;
  أرجع درست;
}

منطقي Output::بیت_مجموعه_تطابق(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_بیت_مقدار_Comma)
      output(", ");
    output(Str);
    Need_بیت_مقدار_Comma = درست;
  }
  أرجع خطأ;
}

فراغ Output::انتها_بیت_مجموعه_Scalar() {
  output_به_بالا_مقصد_انتها_از_سطر(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_سطر_Check();
  إذا (S.خالی()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_به_بالا_مقصد_انتها_از_سطر("''");
    أرجع;
  }
  output(S, MustQuote);
  output_به_بالا_مقصد_انتها_از_سطر("");
}

فراغ Output::بلوک_Scalar_String(StringRef &S) {
  إذا (!State_پشته.خالی())
    new_سطر_Check();
  output(" |");

  بیعلامت Indent = State_پشته.خالی() ? 1 : State_پشته.اندازه_2();

  خودکار میانگیر = حافظه_میانگیر::get_Mem_میانگیر(S, "", خطأ);
  لكل (سطر_iterator سطرها(*میانگیر, خطأ); !سطرها.is_at_انتها(); ++سطرها) {
    output_New_سطر();
    لكل (بیعلامت I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*سطرها);
  }
  output_به_بالا_مقصد_انتها_از_سطر("");
}

فراغ Output::scalar_برچسب(std::string &برچسب) {
  إذا (برچسب.خالی())
    أرجع;
  new_سطر_Check();
  output(برچسب);
  output(" ");
}

فراغ Output::مجموعه_Error(ثابت Twine &پیام_2) {
}

std::error_کد Output::error() { أرجع {}; }

منطقي Output::can_Elide_خالی_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  إذا (State_پشته.اندازه_2() < 2)
    أرجع درست;
  إذا (State_پشته.پشت() != درون_Map_اولی_کلید)
    أرجع درست;
  أرجع !درون_Seq_Any_Element(State_پشته[State_پشته.اندازه_2() - 2]);
}

فراغ Output::output(StringRef s) {
  ستونی += s.اندازه_2();
  بیرون << s;
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

  بیعلامت i = 0;
  بیعلامت j = 0;
  بیعلامت انتها_2 = S.اندازه_2();
  ثابت حرف *پایه = S.داده();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  طالما (j < انتها_2) {
    إذا (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&پایه[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&پایه[i], j - i));
  output(Quote); // Ending quote.
}

فراغ Output::output_به_بالا_مقصد_انتها_از_سطر(StringRef s) {
  output(s);
  إذا (State_پشته.خالی() || (!درون_Flow_Seq_Any_Element(State_پشته.پشت()) &&
                             !درون_Flow_Map_Any_کلید(State_پشته.پشت())))
    Padding = "\n";
}

فراغ Output::output_New_سطر() {
  بیرون << "\n";
  ستونی = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_سطر_Check(منطقي خالی_Sequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_سطر();
  Padding = {};

  إذا (State_پشته.اندازه_2() == 0 || خالی_Sequence)
    أرجع;

  بیعلامت Indent = State_پشته.اندازه_2() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  خودکار I = State_پشته.rbegin(), E = State_پشته.rend();

  إذا (درون_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = درست; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == درون_Map_اولی_کلید || *I == درون_Flow_Map_اولی_کلید ||
             درون_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = درست;
    ++I; // Skip back().
  }

  بیعلامت Output_Dash_شمار = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!درون_Seq_Any_Element(*I))
        اكسر;

      ++Output_Dash_شمار;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != درون_Seq_اولی_Element)
        اكسر;
    }
  }

  لكل (بیعلامت I = Output_Dash_شمار; I < Indent; ++I)
    output("  ");

  لكل (بیعلامت I = 0; I < Output_Dash_شمار; ++I)
    output("- ");
}

فراغ Output::padded_کلید(StringRef کلید_2) {
  output(کلید_2, needsQuotes(کلید_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (کلید_2.اندازه_2() < طول_رشته(spaces))
    Padding = &spaces[کلید_2.اندازه_2()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_کلید(StringRef کلید) {
  إذا (State_پشته.پشت() == درون_Flow_Map_Other_کلید)
    output(", ");
  إذا (Wrap_ستونی && ستونی > Wrap_ستونی) {
    output("\n");
    لكل (صحيح I = 0; I < ستونی_At_Map_Flow_شروع; ++I)
      output(" ");
    ستونی = ستونی_At_Map_Flow_شروع;
    output("  ");
  }
  output(کلید, needsQuotes(کلید, خطأ));
  output(": ");
}

گره_نوع Output::get_گره_نوع() { report_fatal_error("invalid call"); }

منطقي Output::درون_Seq_Any_Element(درون_State State) {
  أرجع State == درون_Seq_اولی_Element || State == درون_Seq_Other_Element;
}

منطقي Output::درون_Flow_Seq_Any_Element(درون_State State) {
  أرجع State == درون_Flow_Seq_اولی_Element || State == درون_Flow_Seq_Other_Element;
}

منطقي Output::درون_Map_Any_کلید(درون_State State) {
  أرجع State == درون_Map_اولی_کلید || State == درون_Map_Other_کلید;
}

منطقي Output::درون_Flow_Map_Any_کلید(درون_State State) {
  أرجع State == درون_Flow_Map_اولی_کلید || State == درون_Flow_Map_Other_کلید;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &بیرون) {
  بیرون << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::درون_داد_2(StringRef Scalar, فراغ *, منطقي &Val) {
  إذا (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    أرجع StringRef();
  }
  أرجع "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &بیرون) {
  بیرون << Val;
}

StringRef ScalarTraits<StringRef>::درون_داد_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &بیرون) {
  بیرون << Val;
}

StringRef ScalarTraits<std::string>::درون_داد_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &بیرون) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t رقم = Val;
  بیرون << رقم;
}

StringRef ScalarTraits<uint8_t>::درون_داد_2(StringRef Scalar, فراغ *, uint8_t &Val) {
  بیعلامت دراز دراز n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &بیرون) {
  بیرون << Val;
}

StringRef ScalarTraits<uint16_t>::درون_داد_2(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  بیعلامت دراز دراز n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &بیرون) {
  بیرون << Val;
}

StringRef ScalarTraits<uint32_t>::درون_داد_2(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  بیعلامت دراز دراز n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &بیرون) {
  بیرون << Val;
}

StringRef ScalarTraits<uint64_t>::درون_داد_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  بیعلامت دراز دراز N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &بیرون) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t رقم = Val;
  بیرون << رقم;
}

StringRef ScalarTraits<int8_t>::درون_داد_2(StringRef Scalar, فراغ *, int8_t &Val) {
  دراز دراز N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > 127) || (N < -128))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &بیرون) {
  بیرون << Val;
}

StringRef ScalarTraits<int16_t>::درون_داد_2(StringRef Scalar, فراغ *, int16_t &Val) {
  دراز دراز N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT16_MAX) || (N < INT16_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &بیرون) {
  بیرون << Val;
}

StringRef ScalarTraits<int32_t>::درون_داد_2(StringRef Scalar, فراغ *, int32_t &Val) {
  دراز دراز N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT32_MAX) || (N < INT32_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &بیرون) {
  بیرون << Val;
}

StringRef ScalarTraits<int64_t>::درون_داد_2(StringRef Scalar, فراغ *, int64_t &Val) {
  دراز دراز N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &بیرون) {
  بیرون << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::درون_داد_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (مقصد_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &بیرون) {
  بیرون << format("%g", Val);
}

StringRef ScalarTraits<عائم>::درون_داد_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (مقصد_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &بیرون) {
  بیرون << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::درون_داد_2(StringRef Scalar, فراغ *, Hex8 &Val) {
  بیعلامت دراز دراز n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex8 number";
  إذا (n > 0xFF)
    أرجع "out of range hex8 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &بیرون) {
  بیرون << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::درون_داد_2(StringRef Scalar, فراغ *, Hex16 &Val) {
  بیعلامت دراز دراز n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex16 number";
  إذا (n > 0xFFFF)
    أرجع "out of range hex16 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &بیرون) {
  بیرون << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::درون_داد_2(StringRef Scalar, فراغ *, Hex32 &Val) {
  بیعلامت دراز دراز n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex32 number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range hex32 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &بیرون) {
  بیرون << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::درون_داد_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  بیعلامت دراز دراز رقم;
  إذا (getAsUnsignedInteger(Scalar, 0, رقم))
    أرجع "invalid hex64 number";
  Val = رقم;
  أرجع StringRef();
}

فراغ ScalarTraits<نسخه_Tuple>::output(ثابت نسخه_Tuple &Val, فراغ *,
                                        llvm::raw_ostream &بیرون) {
  بیرون << Val.getAsString();
}

StringRef ScalarTraits<نسخه_Tuple>::درون_داد_2(StringRef Scalar, فراغ *,
                                            نسخه_Tuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
