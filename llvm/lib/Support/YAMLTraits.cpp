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

استعمال نطاق llvm;
استعمال نطاق yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(فراغ *Context) : Ctxt(Context) {}

IO::~IO() = افتراضي;

فراغ *IO::getContext() ثابت {
  أرجع Ctxt;
}

فراغ IO::سيٽ_Context(فراغ *Context) {
  Ctxt = Context;
}

فراغ IO::سيٽ_Allow_Unknown_ڪنجيون(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//داخلا
//===----------------------------------------------------------------------===//

داخلا::داخلا(StringRef داخلا_Content, فراغ *Ctxt,
             ذريعو_Mgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد وهڪرو(داخلا_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.سيٽ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

داخلا::داخلا(يادگيري_بفر_Ref داخلا, فراغ *Ctxt,
             ذريعو_Mgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد وهڪرو(داخلا, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.سيٽ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

داخلا::~داخلا() = افتراضي;

std::error_ڪوڊ داخلا::error() { أرجع EC; }

منطقي داخلا::outputting() ثابت {
  أرجع خطأ;
}

منطقي داخلا::سيٽ_Current_Document() {
  إذا (DocIterator != Strm->پڇاڙي()) {
    ڳنڍ *N = DocIterator->get_پاڙ();
    إذا (!N) {
      EC = make_error_ڪوڊ(errc::invalid_دليل);
      أرجع خطأ;
    }

    إذا (isa<Null_ڳنڍ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع سيٽ_Current_Document();
    }
    releaseHNodeBuffers();
    چوٽي_ڳنڍ = createHNodes(N);
    Current_ڳنڍ = چوٽي_ڳنڍ;
    أرجع سچ;
  }
  أرجع خطأ;
}

منطقي داخلا::ايندڙ_Document() {
  أرجع ++DocIterator != Strm->پڇاڙي();
}

ثابت ڳنڍ *داخلا::get_Current_ڳنڍ() ثابت {
  أرجع Current_ڳنڍ ? Current_ڳنڍ->ڳنڍ_2 : خالي_اشارو;
}

منطقي داخلا::map_ٽيگ(StringRef ٽيگ, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_ڳنڍ)
    أرجع خطأ;

  std::string مليو_ٽيگ = Current_ڳنڍ->ڳنڍ_2->get_Verbatim_ٽيگ();
  إذا (مليو_ٽيگ.خالي_جايون()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع ٽيگ == مليو_ٽيگ;
}

فراغ داخلا::beginMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ڳنڍ);
  إذا (MN) {
    MN->Valid_ڪنجيون.clear();
  }
}

std::ويڪٽر<StringRef> داخلا::ڪنجيون() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_ڳنڍ);
  std::ويڪٽر<StringRef> Ret;
  إذا (!MN) {
    سيٽ_Error(Current_ڳنڍ, "not a mapping");
    أرجع Ret;
  }
  لكل (خودڪار &P : MN->Mapping)
    Ret.ڌڪو_پٺ(P.پهريون_2());
  أرجع Ret;
}

منطقي داخلا::preflight_ڪنجي(StringRef ڪنجي, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_ڳنڍ) {
    إذا (Required)
      EC = make_error_ڪوڊ(errc::invalid_دليل);
    وإلا
      UseDefault = سچ;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_ڳنڍ);
  إذا (!MN) {
    إذا (Required || !isa<خالي_جايون_HNode>(Current_ڳنڍ))
      سيٽ_Error(Current_ڳنڍ, "not a mapping");
    وإلا
      UseDefault = سچ;
    أرجع خطأ;
  }
  MN->Valid_ڪنجيون.ڌڪو_پٺ(ڪنجي.str());
  HNode *قدر = MN->Mapping[ڪنجي].پهريون_2;
  إذا (!قدر) {
    إذا (Required)
      سيٽ_Error(Current_ڳنڍ, Twine("missing required key '") + ڪنجي + "'");
    وإلا
      UseDefault = سچ;
    أرجع خطأ;
  }
  SaveInfo = Current_ڳنڍ;
  Current_ڳنڍ = قدر;
  أرجع سچ;
}

فراغ داخلا::postflight_ڪنجي(فراغ *saveInfo) {
  Current_ڳنڍ = reinterpret_cast<HNode *>(saveInfo);
}

فراغ داخلا::پڇاڙي_Mapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_ڳنڍ);
  إذا (!MN)
    أرجع;
  لكل (ثابت خودڪار &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_ڪنجيون, NN.پهريون_2())) {
      ثابت SMRange &ReportLoc = NN.ٻيو_2.ٻيو_2;
      إذا (!Allow_Unknown_ڪنجيون) {
        سيٽ_Error(ReportLoc, Twine("unknown key '") + NN.پهريون_2() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.پهريون_2() + "'");
    }
  }
}

فراغ داخلا::beginFlowMapping() { beginMapping(); }

فراغ داخلا::پڇاڙي_Flow_Mapping() { پڇاڙي_Mapping(); }

بنانشان داخلا::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ڳنڍ))
    أرجع SQ->Entries.سائيز_2();
  إذا (isa<خالي_جايون_HNode>(Current_ڳنڍ))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ڳنڍ)) {
    إذا (isNull(SN->قدر_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  سيٽ_Error(Current_ڳنڍ, "not a sequence");
  أرجع 0;
}

فراغ داخلا::پڇاڙي_Sequence() {
}

منطقي داخلا::preflightElement(بنانشان اشاريو_2, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ڳنڍ)) {
    SaveInfo = Current_ڳنڍ;
    Current_ڳنڍ = SQ->Entries[اشاريو_2];
    أرجع سچ;
  }
  أرجع خطأ;
}

فراغ داخلا::postflightElement(فراغ *SaveInfo) {
  Current_ڳنڍ = reinterpret_cast<HNode *>(SaveInfo);
}

بنانشان داخلا::beginFlowSequence() { أرجع beginSequence(); }

منطقي داخلا::preflightFlowElement(بنانشان اشاريو, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ڳنڍ)) {
    SaveInfo = Current_ڳنڍ;
    Current_ڳنڍ = SQ->Entries[اشاريو];
    أرجع سچ;
  }
  أرجع خطأ;
}

فراغ داخلا::postflightFlowElement(فراغ *SaveInfo) {
  Current_ڳنڍ = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ داخلا::پڇاڙي_Flow_Sequence() {
}

فراغ داخلا::beginEnumScalar() {
  Scalar_ميلاپ_مليو = خطأ;
}

منطقي داخلا::ميلاپ_Enum_Scalar(StringRef Str, منطقي) {
  إذا (Scalar_ميلاپ_مليو)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ڳنڍ)) {
    إذا (SN->قدر_2() == Str) {
      Scalar_ميلاپ_مليو = سچ;
      أرجع سچ;
    }
  }
  أرجع خطأ;
}

منطقي داخلا::ميلاپ_Enum_Fallback() {
  إذا (Scalar_ميلاپ_مليو)
    أرجع خطأ;
  Scalar_ميلاپ_مليو = سچ;
  أرجع سچ;
}

فراغ داخلا::پڇاڙي_Enum_Scalar() {
  إذا (!Scalar_ميلاپ_مليو) {
    سيٽ_Error(Current_ڳنڍ, "unknown enumerated scalar");
  }
}

منطقي داخلا::begin_ٻٽو_انگ_سيٽ_Scalar(منطقي &DoClear) {
  ٻٽو_انگ_قدرون_استعمال_ٿيل.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ڳنڍ)) {
    ٻٽو_انگ_قدرون_استعمال_ٿيل.resize(SQ->Entries.سائيز_2());
  } وإلا {
    سيٽ_Error(Current_ڳنڍ, "expected sequence of bit values");
  }
  DoClear = سچ;
  أرجع سچ;
}

منطقي داخلا::ٻٽو_انگ_سيٽ_ميلاپ(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ڳنڍ)) {
    بنانشان اشاريو_2 = 0;
    لكل (خودڪار &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->قدر_2() == Str) {
          ٻٽو_انگ_قدرون_استعمال_ٿيل[اشاريو_2] = سچ;
          أرجع سچ;
        }
      } وإلا {
        سيٽ_Error(Current_ڳنڍ, "unexpected scalar in sequence of bit values");
      }
      ++اشاريو_2;
    }
  } وإلا {
    سيٽ_Error(Current_ڳنڍ, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ داخلا::پڇاڙي_ٻٽو_انگ_سيٽ_Scalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_ڳنڍ)) {
    أكد(ٻٽو_انگ_قدرون_استعمال_ٿيل.سائيز_2() == SQ->Entries.سائيز_2());
    لكل (بنانشان i = 0; i < SQ->Entries.سائيز_2(); ++i) {
      إذا (!ٻٽو_انگ_قدرون_استعمال_ٿيل[i]) {
        سيٽ_Error(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ داخلا::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_ڳنڍ)) {
    S = SN->قدر_2();
  } وإلا {
    سيٽ_Error(Current_ڳنڍ, "unexpected scalar");
  }
}

فراغ داخلا::بلاڪ_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ داخلا::scalar_ٽيگ(std::string &ٽيگ) {
  ٽيگ = Current_ڳنڍ->ڳنڍ_2->get_Verbatim_ٽيگ();
}

فراغ داخلا::سيٽ_Error(HNode *hnode, ثابت Twine &پيغام) {
  أكد(hnode && "HNode must not be NULL");
  سيٽ_Error(hnode->ڳنڍ_2, پيغام);
}

ڳنڍ_قسم داخلا::get_ڳنڍ_قسم() {
  إذا (isa<ScalarHNode>(Current_ڳنڍ))
    أرجع ڳنڍ_قسم::Scalar;
  وإلا إذا (isa<MapHNode>(Current_ڳنڍ))
    أرجع ڳنڍ_قسم::Map;
  وإلا إذا (isa<SequenceHNode>(Current_ڳنڍ))
    أرجع ڳنڍ_قسم::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ داخلا::سيٽ_Error(ڳنڍ *ڳنڍ_3, ثابت Twine &پيغام) {
  Strm->printError(ڳنڍ_3, پيغام);
  EC = make_error_ڪوڊ(errc::invalid_دليل);
}

فراغ داخلا::سيٽ_Error(ثابت SMRange &range, ثابت Twine &پيغام) {
  Strm->printError(range, پيغام);
  EC = make_error_ڪوڊ(errc::invalid_دليل);
}

فراغ داخلا::reportWarning(HNode *hnode, ثابت Twine &پيغام) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->ڳنڍ_2, پيغام, ذريعو_Mgr::DK_Warning);
}

فراغ داخلا::reportWarning(ڳنڍ *ڳنڍ_3, ثابت Twine &پيغام) {
  Strm->printError(ڳنڍ_3, پيغام, ذريعو_Mgr::DK_Warning);
}

فراغ داخلا::reportWarning(ثابت SMRange &range, ثابت Twine &پيغام) {
  Strm->printError(range, پيغام, ذريعو_Mgr::DK_Warning);
}

فراغ داخلا::releaseHNodeBuffers() {
  خالي_جايون_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ داخلا::saveAliasHNode(ڳنڍ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  إذا (!Anchor.خالي_جايون())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

داخلا::HNode *داخلا::createHNodes(ڳنڍ *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة ڳنڍ::NK_Scalar: {
    Scalar_ڳنڍ *SN = dyn_cast<Scalar_ڳنڍ>(N);
    StringRef ڪنجي_Str = SN->get_قدر(StringStorage);
    إذا (!StringStorage.خالي_جايون()) {
      // Copy string to permanent storage
      ڪنجي_Str = StringStorage.str().copy(StringAllocator);
    }
    خودڪار *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, ڪنجي_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة ڳنڍ::NK_بلاڪ_Scalar: {
    بلاڪ_Scalar_ڳنڍ *BSN = dyn_cast<بلاڪ_Scalar_ڳنڍ>(N);
    StringRef قدر_Copy = BSN->get_قدر().copy(StringAllocator);
    خودڪار *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, قدر_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة ڳنڍ::NK_Sequence: {
    Sequence_ڳنڍ *SQ = dyn_cast<Sequence_ڳنڍ>(N);
    خودڪار SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (ڳنڍ &SN : *SQ) {
      خودڪار داخلا_3 = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.ڌڪو_پٺ(داخلا_3);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة ڳنڍ::NK_Mapping: {
    Mapping_ڳنڍ *Map = dyn_cast<Mapping_ڳنڍ>(N);
    خودڪار mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (ڪنجي_قدر_ڳنڍ &KVN : *Map) {
      ڳنڍ *ڪنجي_ڳنڍ = KVN.get_ڪنجي();
      Scalar_ڳنڍ *ڪنجي = dyn_cast_if_present<Scalar_ڳنڍ>(ڪنجي_ڳنڍ);
      ڳنڍ *قدر = KVN.get_قدر();
      إذا (!ڪنجي || !قدر) {
        إذا (!ڪنجي)
          سيٽ_Error(ڪنجي_ڳنڍ, "Map key must be a scalar");
        إذا (!قدر)
          سيٽ_Error(ڪنجي_ڳنڍ, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef ڪنجي_Str = ڪنجي->get_قدر(StringStorage);
      إذا (!StringStorage.خالي_جايون()) {
        // Copy string to permanent storage
        ڪنجي_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.ڳڻپ(ڪنجي_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        سيٽ_Error(ڪنجي_ڳنڍ, Twine("duplicated mapping key '") + ڪنجي_Str + "'");
      خودڪار قدر_HNode = createHNodes(قدر);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[ڪنجي_Str] =
          std::make_pair(std::move(قدر_HNode), ڪنجي_ڳنڍ->get_ذريعو_Range());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة ڳنڍ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (خالي_جايون_HNode_Allocator.Allocate()) خالي_جايون_HNode(N);
  حالة ڳنڍ::NK_Alias: {
    Alias_ڳنڍ *AN = dyn_cast<Alias_ڳنڍ>(N);
    خودڪار AliasName = AN->getName();
    خودڪار AHN = AliasMap.ڳوليو(AliasName);
    إذا (AHN == AliasMap.پڇاڙي()) {
      سيٽ_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      أرجع خالي_اشارو;
    }
    أرجع AHN->ٻيو_2;
  }
  افتراضي:
    سيٽ_Error(N, "unknown node kind");
    أرجع خالي_اشارو;
  }
}

فراغ داخلا::سيٽ_Error(ثابت Twine &پيغام_2) {
  سيٽ_Error(Current_ڳنڍ, پيغام_2);
}

فراغ داخلا::سيٽ_Allow_Unknown_ڪنجيون(منطقي Allow) { Allow_Unknown_ڪنجيون = Allow; }

منطقي داخلا::can_Elide_خالي_جايون_Sequence() {
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, فراغ *context, صحيح Wrap_ڪالم)
    : IO(context), ٻاهريون(yout), Wrap_ڪالم(Wrap_ڪالم) {}

Output::~Output() = افتراضي;

منطقي Output::outputting() ثابت {
  أرجع سچ;
}

فراغ Output::beginMapping() {
  State_ڍير.ڌڪو_پٺ(اندريون_Map_پهريون_ڪنجي);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::map_ٽيگ(StringRef ٽيگ, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_ڍير.سائيز_2() > 1) {
      خودڪار &E = State_ڍير[State_ڍير.سائيز_2() - 2];
      SequenceElement = اندريون_Seq_Any_Element(E) || اندريون_Flow_Seq_Any_Element(E);
    }
    إذا (SequenceElement && State_ڍير.پٺ() == اندريون_Map_پهريون_ڪنجي) {
      new_سٽ_Check();
    } وإلا {
      output(" ");
    }
    output(ٽيگ);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_ڍير.پٺ() == اندريون_Map_پهريون_ڪنجي) {
        State_ڍير.ڪڍو_پٺ();
        State_ڍير.ڌڪو_پٺ(اندريون_Map_Other_ڪنجي);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  أرجع Use;
}

فراغ Output::پڇاڙي_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  إذا (State_ڍير.پٺ() == اندريون_Map_پهريون_ڪنجي) {
    Padding = PaddingBeforeContainer;
    new_سٽ_Check();
    output("{}");
    Padding = "\n";
  }
  State_ڍير.ڪڍو_پٺ();
}

std::ويڪٽر<StringRef> Output::ڪنجيون() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_ڪنجي(StringRef ڪنجي, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = خالي_اشارو;
  إذا (Required || !SameAsDefault || Write_Default_قدرون) {
    خودڪار State = State_ڍير.پٺ();
    إذا (State == اندريون_Flow_Map_پهريون_ڪنجي || State == اندريون_Flow_Map_Other_ڪنجي) {
      flow_ڪنجي(ڪنجي);
    } وإلا {
      new_سٽ_Check();
      padded_ڪنجي(ڪنجي);
    }
    أرجع سچ;
  }
  أرجع خطأ;
}

فراغ Output::postflight_ڪنجي(فراغ *) {
  إذا (State_ڍير.پٺ() == اندريون_Map_پهريون_ڪنجي) {
    State_ڍير.ڪڍو_پٺ();
    State_ڍير.ڌڪو_پٺ(اندريون_Map_Other_ڪنجي);
  } وإلا إذا (State_ڍير.پٺ() == اندريون_Flow_Map_پهريون_ڪنجي) {
    State_ڍير.ڪڍو_پٺ();
    State_ڍير.ڌڪو_پٺ(اندريون_Flow_Map_Other_ڪنجي);
  }
}

فراغ Output::beginFlowMapping() {
  State_ڍير.ڌڪو_پٺ(اندريون_Flow_Map_پهريون_ڪنجي);
  new_سٽ_Check();
  ڪالم_At_Map_Flow_شروعات = ڪالم;
  output("{ ");
}

فراغ Output::پڇاڙي_Flow_Mapping() {
  State_ڍير.ڪڍو_پٺ();
  output_مٿي_منزل_پڇاڙي_جو_سٽ(" }");
}

فراغ Output::beginDocuments() {
  output_مٿي_منزل_پڇاڙي_جو_سٽ("---");
}

منطقي Output::preflightDocument(بنانشان اشاريو) {
  إذا (اشاريو > 0)
    output_مٿي_منزل_پڇاڙي_جو_سٽ("\n---");
  أرجع سچ;
}

فراغ Output::postflightDocument() {
}

فراغ Output::پڇاڙي_Documents() {
  output("\n...\n");
}

بنانشان Output::beginSequence() {
  State_ڍير.ڌڪو_پٺ(اندريون_Seq_پهريون_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::پڇاڙي_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_ڍير.پٺ() == اندريون_Seq_پهريون_Element) {
    Padding = PaddingBeforeContainer;
    new_سٽ_Check(/*EmptySequence=*/سچ);
    output("[]");
    Padding = "\n";
  }
  State_ڍير.ڪڍو_پٺ();
}

منطقي Output::preflightElement(بنانشان, فراغ *&SaveInfo) {
  SaveInfo = خالي_اشارو;
  أرجع سچ;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_ڍير.پٺ() == اندريون_Seq_پهريون_Element) {
    State_ڍير.ڪڍو_پٺ();
    State_ڍير.ڌڪو_پٺ(اندريون_Seq_Other_Element);
  } وإلا إذا (State_ڍير.پٺ() == اندريون_Flow_Seq_پهريون_Element) {
    State_ڍير.ڪڍو_پٺ();
    State_ڍير.ڌڪو_پٺ(اندريون_Flow_Seq_Other_Element);
  }
}

بنانشان Output::beginFlowSequence() {
  State_ڍير.ڌڪو_پٺ(اندريون_Flow_Seq_پهريون_Element);
  new_سٽ_Check();
  ڪالم_At_Flow_شروعات = ڪالم;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::پڇاڙي_Flow_Sequence() {
  State_ڍير.ڪڍو_پٺ();
  output_مٿي_منزل_پڇاڙي_جو_سٽ(" ]");
}

منطقي Output::preflightFlowElement(بنانشان, فراغ *&SaveInfo) {
  إذا (NeedFlowSequenceComma)
    output(", ");
  إذا (Wrap_ڪالم && ڪالم > Wrap_ڪالم) {
    output("\n");
    لكل (صحيح i = 0; i < ڪالم_At_Flow_شروعات; ++i)
      output(" ");
    ڪالم = ڪالم_At_Flow_شروعات;
    output("  ");
  }
  SaveInfo = خالي_اشارو;
  أرجع سچ;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = سچ;
}

فراغ Output::beginEnumScalar() {
  Enumeration_ميلاپ_مليو = خطأ;
}

منطقي Output::ميلاپ_Enum_Scalar(StringRef Str, منطقي ميلاپ_2) {
  إذا (ميلاپ_2 && !Enumeration_ميلاپ_مليو) {
    new_سٽ_Check();
    output_مٿي_منزل_پڇاڙي_جو_سٽ(Str);
    Enumeration_ميلاپ_مليو = سچ;
  }
  أرجع خطأ;
}

منطقي Output::ميلاپ_Enum_Fallback() {
  إذا (Enumeration_ميلاپ_مليو)
    أرجع خطأ;
  Enumeration_ميلاپ_مليو = سچ;
  أرجع سچ;
}

فراغ Output::پڇاڙي_Enum_Scalar() {
  إذا (!Enumeration_ميلاپ_مليو)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::begin_ٻٽو_انگ_سيٽ_Scalar(منطقي &DoClear) {
  new_سٽ_Check();
  output("[ ");
  Need_ٻٽو_انگ_قدر_Comma = خطأ;
  DoClear = خطأ;
  أرجع سچ;
}

منطقي Output::ٻٽو_انگ_سيٽ_ميلاپ(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_ٻٽو_انگ_قدر_Comma)
      output(", ");
    output(Str);
    Need_ٻٽو_انگ_قدر_Comma = سچ;
  }
  أرجع خطأ;
}

فراغ Output::پڇاڙي_ٻٽو_انگ_سيٽ_Scalar() {
  output_مٿي_منزل_پڇاڙي_جو_سٽ(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_سٽ_Check();
  إذا (S.خالي_جايون()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_مٿي_منزل_پڇاڙي_جو_سٽ("''");
    أرجع;
  }
  output(S, MustQuote);
  output_مٿي_منزل_پڇاڙي_جو_سٽ("");
}

فراغ Output::بلاڪ_Scalar_String(StringRef &S) {
  إذا (!State_ڍير.خالي_جايون())
    new_سٽ_Check();
  output(" |");

  بنانشان Indent = State_ڍير.خالي_جايون() ? 1 : State_ڍير.سائيز_2();

  خودڪار بفر = يادگيري_بفر::get_Mem_بفر(S, "", خطأ);
  لكل (سٽ_iterator سٽون(*بفر, خطأ); !سٽون.is_at_پڇاڙي(); ++سٽون) {
    output_New_سٽ();
    لكل (بنانشان I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*سٽون);
  }
  output_مٿي_منزل_پڇاڙي_جو_سٽ("");
}

فراغ Output::scalar_ٽيگ(std::string &ٽيگ) {
  إذا (ٽيگ.خالي_جايون())
    أرجع;
  new_سٽ_Check();
  output(ٽيگ);
  output(" ");
}

فراغ Output::سيٽ_Error(ثابت Twine &پيغام) {
}

std::error_ڪوڊ Output::error() { أرجع {}; }

منطقي Output::can_Elide_خالي_جايون_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  إذا (State_ڍير.سائيز_2() < 2)
    أرجع سچ;
  إذا (State_ڍير.پٺ() != اندريون_Map_پهريون_ڪنجي)
    أرجع سچ;
  أرجع !اندريون_Seq_Any_Element(State_ڍير[State_ڍير.سائيز_2() - 2]);
}

فراغ Output::output(StringRef s) {
  ڪالم += s.سائيز_2();
  ٻاهريون << s;
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

  بنانشان i = 0;
  بنانشان j = 0;
  بنانشان پڇاڙي_2 = S.سائيز_2();
  ثابت حرف *بنياد = S.معلومات();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  طالما (j < پڇاڙي_2) {
    إذا (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&بنياد[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&بنياد[i], j - i));
  output(Quote); // Ending quote.
}

فراغ Output::output_مٿي_منزل_پڇاڙي_جو_سٽ(StringRef s) {
  output(s);
  إذا (State_ڍير.خالي_جايون() || (!اندريون_Flow_Seq_Any_Element(State_ڍير.پٺ()) &&
                             !اندريون_Flow_Map_Any_ڪنجي(State_ڍير.پٺ())))
    Padding = "\n";
}

فراغ Output::output_New_سٽ() {
  ٻاهريون << "\n";
  ڪالم = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_سٽ_Check(منطقي خالي_جايون_Sequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_سٽ();
  Padding = {};

  إذا (State_ڍير.سائيز_2() == 0 || خالي_جايون_Sequence)
    أرجع;

  بنانشان Indent = State_ڍير.سائيز_2() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  خودڪار I = State_ڍير.rbegin(), E = State_ڍير.rend();

  إذا (اندريون_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = سچ; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == اندريون_Map_پهريون_ڪنجي || *I == اندريون_Flow_Map_پهريون_ڪنجي ||
             اندريون_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = سچ;
    ++I; // Skip back().
  }

  بنانشان Output_Dash_ڳڻپ = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!اندريون_Seq_Any_Element(*I))
        اكسر;

      ++Output_Dash_ڳڻپ;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != اندريون_Seq_پهريون_Element)
        اكسر;
    }
  }

  لكل (بنانشان I = Output_Dash_ڳڻپ; I < Indent; ++I)
    output("  ");

  لكل (بنانشان I = 0; I < Output_Dash_ڳڻپ; ++I)
    output("- ");
}

فراغ Output::padded_ڪنجي(StringRef ڪنجي_2) {
  output(ڪنجي_2, needsQuotes(ڪنجي_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (ڪنجي_2.سائيز_2() < لڙي_ڊيگهه(spaces))
    Padding = &spaces[ڪنجي_2.سائيز_2()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_ڪنجي(StringRef ڪنجي) {
  إذا (State_ڍير.پٺ() == اندريون_Flow_Map_Other_ڪنجي)
    output(", ");
  إذا (Wrap_ڪالم && ڪالم > Wrap_ڪالم) {
    output("\n");
    لكل (صحيح I = 0; I < ڪالم_At_Map_Flow_شروعات; ++I)
      output(" ");
    ڪالم = ڪالم_At_Map_Flow_شروعات;
    output("  ");
  }
  output(ڪنجي, needsQuotes(ڪنجي, خطأ));
  output(": ");
}

ڳنڍ_قسم Output::get_ڳنڍ_قسم() { report_fatal_error("invalid call"); }

منطقي Output::اندريون_Seq_Any_Element(اندريون_State State) {
  أرجع State == اندريون_Seq_پهريون_Element || State == اندريون_Seq_Other_Element;
}

منطقي Output::اندريون_Flow_Seq_Any_Element(اندريون_State State) {
  أرجع State == اندريون_Flow_Seq_پهريون_Element || State == اندريون_Flow_Seq_Other_Element;
}

منطقي Output::اندريون_Map_Any_ڪنجي(اندريون_State State) {
  أرجع State == اندريون_Map_پهريون_ڪنجي || State == اندريون_Map_Other_ڪنجي;
}

منطقي Output::اندريون_Flow_Map_Any_ڪنجي(اندريون_State State) {
  أرجع State == اندريون_Flow_Map_پهريون_ڪنجي || State == اندريون_Flow_Map_Other_ڪنجي;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &ٻاهريون) {
  ٻاهريون << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::داخلا_2(StringRef Scalar, فراغ *, منطقي &Val) {
  إذا (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    أرجع StringRef();
  }
  أرجع "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &ٻاهريون) {
  ٻاهريون << Val;
}

StringRef ScalarTraits<StringRef>::داخلا_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &ٻاهريون) {
  ٻاهريون << Val;
}

StringRef ScalarTraits<std::string>::داخلا_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &ٻاهريون) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t انگ = Val;
  ٻاهريون << انگ;
}

StringRef ScalarTraits<uint8_t>::داخلا_2(StringRef Scalar, فراغ *, uint8_t &Val) {
  بنانشان ڊگهو ڊگهو n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &ٻاهريون) {
  ٻاهريون << Val;
}

StringRef ScalarTraits<uint16_t>::داخلا_2(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  بنانشان ڊگهو ڊگهو n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &ٻاهريون) {
  ٻاهريون << Val;
}

StringRef ScalarTraits<uint32_t>::داخلا_2(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  بنانشان ڊگهو ڊگهو n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &ٻاهريون) {
  ٻاهريون << Val;
}

StringRef ScalarTraits<uint64_t>::داخلا_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  بنانشان ڊگهو ڊگهو N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &ٻاهريون) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t انگ = Val;
  ٻاهريون << انگ;
}

StringRef ScalarTraits<int8_t>::داخلا_2(StringRef Scalar, فراغ *, int8_t &Val) {
  ڊگهو ڊگهو N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > 127) || (N < -128))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &ٻاهريون) {
  ٻاهريون << Val;
}

StringRef ScalarTraits<int16_t>::داخلا_2(StringRef Scalar, فراغ *, int16_t &Val) {
  ڊگهو ڊگهو N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT16_MAX) || (N < INT16_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &ٻاهريون) {
  ٻاهريون << Val;
}

StringRef ScalarTraits<int32_t>::داخلا_2(StringRef Scalar, فراغ *, int32_t &Val) {
  ڊگهو ڊگهو N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT32_MAX) || (N < INT32_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &ٻاهريون) {
  ٻاهريون << Val;
}

StringRef ScalarTraits<int64_t>::داخلا_2(StringRef Scalar, فراغ *, int64_t &Val) {
  ڊگهو ڊگهو N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &ٻاهريون) {
  ٻاهريون << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::داخلا_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (منزل_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &ٻاهريون) {
  ٻاهريون << format("%g", Val);
}

StringRef ScalarTraits<عائم>::داخلا_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (منزل_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &ٻاهريون) {
  ٻاهريون << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::داخلا_2(StringRef Scalar, فراغ *, Hex8 &Val) {
  بنانشان ڊگهو ڊگهو n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex8 number";
  إذا (n > 0xFF)
    أرجع "out of range hex8 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &ٻاهريون) {
  ٻاهريون << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::داخلا_2(StringRef Scalar, فراغ *, Hex16 &Val) {
  بنانشان ڊگهو ڊگهو n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex16 number";
  إذا (n > 0xFFFF)
    أرجع "out of range hex16 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &ٻاهريون) {
  ٻاهريون << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::داخلا_2(StringRef Scalar, فراغ *, Hex32 &Val) {
  بنانشان ڊگهو ڊگهو n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex32 number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range hex32 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &ٻاهريون) {
  ٻاهريون << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::داخلا_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  بنانشان ڊگهو ڊگهو انگ;
  إذا (getAsUnsignedInteger(Scalar, 0, انگ))
    أرجع "invalid hex64 number";
  Val = انگ;
  أرجع StringRef();
}

فراغ ScalarTraits<نسخو_Tuple>::output(ثابت نسخو_Tuple &Val, فراغ *,
                                        llvm::raw_ostream &ٻاهريون) {
  ٻاهريون << Val.getAsString();
}

StringRef ScalarTraits<نسخو_Tuple>::داخلا_2(StringRef Scalar, فراغ *,
                                            نسخو_Tuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
