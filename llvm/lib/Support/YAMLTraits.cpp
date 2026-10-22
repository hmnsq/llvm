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

ورتوں نطاق llvm;
ورتوں نطاق yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(فراغ *Context) : Ctxt(Context) {}

IO::~IO() = افتراضي;

فراغ *IO::getContext() ثابت {
  أرجع Ctxt;
}

فراغ IO::سیٹ_Context(فراغ *Context) {
  Ctxt = Context;
}

فراغ IO::سیٹ_Allow_Unknown_کلیدیں(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//داخلہ
//===----------------------------------------------------------------------===//

داخلہ::داخلہ(StringRef داخلہ_Content, فراغ *Ctxt,
             سروت_Mgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد دھارا(داخلہ_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.سیٹ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

داخلہ::داخلہ(حافظہ_بفر_Ref داخلہ, فراغ *Ctxt,
             سروت_Mgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد دھارا(داخلہ, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.سیٹ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

داخلہ::~داخلہ() = افتراضي;

std::error_کوڈ داخلہ::error() { أرجع EC; }

منطقي داخلہ::outputting() ثابت {
  أرجع خطأ;
}

منطقي داخلہ::سیٹ_Current_Document() {
  إذا (DocIterator != Strm->سرا()) {
    گرہ *N = DocIterator->get_جڑ();
    إذا (!N) {
      EC = make_error_کوڈ(errc::invalid_دلیل);
      أرجع خطأ;
    }

    إذا (isa<Null_گرہ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع سیٹ_Current_Document();
    }
    releaseHNodeBuffers();
    چوٹی_گرہ = createHNodes(N);
    Current_گرہ = چوٹی_گرہ;
    أرجع سچ;
  }
  أرجع خطأ;
}

منطقي داخلہ::اگلا_Document() {
  أرجع ++DocIterator != Strm->سرا();
}

ثابت گرہ *داخلہ::get_Current_گرہ() ثابت {
  أرجع Current_گرہ ? Current_گرہ->گرہ_2 : خالی_سوچک;
}

منطقي داخلہ::map_ٹیگ(StringRef ٹیگ, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_گرہ)
    أرجع خطأ;

  std::string لبھیا_ٹیگ = Current_گرہ->گرہ_2->get_Verbatim_ٹیگ();
  إذا (لبھیا_ٹیگ.خالی_تھاواں()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع ٹیگ == لبھیا_ٹیگ;
}

فراغ داخلہ::beginMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گرہ);
  إذا (MN) {
    MN->Valid_کلیدیں.clear();
  }
}

std::سمتیہ<StringRef> داخلہ::کلیدیں() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_گرہ);
  std::سمتیہ<StringRef> Ret;
  إذا (!MN) {
    سیٹ_Error(Current_گرہ, "not a mapping");
    أرجع Ret;
  }
  لكل (خودکار &P : MN->Mapping)
    Ret.ڈالو_پٹھ(P.پہلا_2());
  أرجع Ret;
}

منطقي داخلہ::preflight_کلید(StringRef کلید, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_گرہ) {
    إذا (Required)
      EC = make_error_کوڈ(errc::invalid_دلیل);
    وإلا
      UseDefault = سچ;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_گرہ);
  إذا (!MN) {
    إذا (Required || !isa<خالی_تھاواں_HNode>(Current_گرہ))
      سیٹ_Error(Current_گرہ, "not a mapping");
    وإلا
      UseDefault = سچ;
    أرجع خطأ;
  }
  MN->Valid_کلیدیں.ڈالو_پٹھ(کلید.str());
  HNode *قدر = MN->Mapping[کلید].پہلا_2;
  إذا (!قدر) {
    إذا (Required)
      سیٹ_Error(Current_گرہ, Twine("missing required key '") + کلید + "'");
    وإلا
      UseDefault = سچ;
    أرجع خطأ;
  }
  SaveInfo = Current_گرہ;
  Current_گرہ = قدر;
  أرجع سچ;
}

فراغ داخلہ::postflight_کلید(فراغ *saveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(saveInfo);
}

فراغ داخلہ::سرا_Mapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گرہ);
  إذا (!MN)
    أرجع;
  لكل (ثابت خودکار &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_کلیدیں, NN.پہلا_2())) {
      ثابت SMRange &ReportLoc = NN.دوجا_2.دوجا_2;
      إذا (!Allow_Unknown_کلیدیں) {
        سیٹ_Error(ReportLoc, Twine("unknown key '") + NN.پہلا_2() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.پہلا_2() + "'");
    }
  }
}

فراغ داخلہ::beginFlowMapping() { beginMapping(); }

فراغ داخلہ::سرا_Flow_Mapping() { سرا_Mapping(); }

بنانشانی داخلہ::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ))
    أرجع SQ->Entries.ناپ_2();
  إذا (isa<خالی_تھاواں_HNode>(Current_گرہ))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    إذا (isNull(SN->قدر_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  سیٹ_Error(Current_گرہ, "not a sequence");
  أرجع 0;
}

فراغ داخلہ::سرا_Sequence() {
}

منطقي داخلہ::preflightElement(بنانشانی اشاریہ_2, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    SaveInfo = Current_گرہ;
    Current_گرہ = SQ->Entries[اشاریہ_2];
    أرجع سچ;
  }
  أرجع خطأ;
}

فراغ داخلہ::postflightElement(فراغ *SaveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(SaveInfo);
}

بنانشانی داخلہ::beginFlowSequence() { أرجع beginSequence(); }

منطقي داخلہ::preflightFlowElement(بنانشانی اشاریہ, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    SaveInfo = Current_گرہ;
    Current_گرہ = SQ->Entries[اشاریہ];
    أرجع سچ;
  }
  أرجع خطأ;
}

فراغ داخلہ::postflightFlowElement(فراغ *SaveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ داخلہ::سرا_Flow_Sequence() {
}

فراغ داخلہ::beginEnumScalar() {
  Scalar_میل_لبھیا = خطأ;
}

منطقي داخلہ::میل_Enum_Scalar(StringRef Str, منطقي) {
  إذا (Scalar_میل_لبھیا)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    إذا (SN->قدر_2() == Str) {
      Scalar_میل_لبھیا = سچ;
      أرجع سچ;
    }
  }
  أرجع خطأ;
}

منطقي داخلہ::میل_Enum_Fallback() {
  إذا (Scalar_میل_لبھیا)
    أرجع خطأ;
  Scalar_میل_لبھیا = سچ;
  أرجع سچ;
}

فراغ داخلہ::سرا_Enum_Scalar() {
  إذا (!Scalar_میل_لبھیا) {
    سیٹ_Error(Current_گرہ, "unknown enumerated scalar");
  }
}

منطقي داخلہ::begin_ثنائی_ہندسہ_سیٹ_Scalar(منطقي &DoClear) {
  ثنائی_ہندسہ_اقدار_ورتیا.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    ثنائی_ہندسہ_اقدار_ورتیا.resize(SQ->Entries.ناپ_2());
  } وإلا {
    سیٹ_Error(Current_گرہ, "expected sequence of bit values");
  }
  DoClear = سچ;
  أرجع سچ;
}

منطقي داخلہ::ثنائی_ہندسہ_سیٹ_میل(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    بنانشانی اشاریہ_2 = 0;
    لكل (خودکار &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->قدر_2() == Str) {
          ثنائی_ہندسہ_اقدار_ورتیا[اشاریہ_2] = سچ;
          أرجع سچ;
        }
      } وإلا {
        سیٹ_Error(Current_گرہ, "unexpected scalar in sequence of bit values");
      }
      ++اشاریہ_2;
    }
  } وإلا {
    سیٹ_Error(Current_گرہ, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ داخلہ::سرا_ثنائی_ہندسہ_سیٹ_Scalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    أكد(ثنائی_ہندسہ_اقدار_ورتیا.ناپ_2() == SQ->Entries.ناپ_2());
    لكل (بنانشانی i = 0; i < SQ->Entries.ناپ_2(); ++i) {
      إذا (!ثنائی_ہندسہ_اقدار_ورتیا[i]) {
        سیٹ_Error(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ داخلہ::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    S = SN->قدر_2();
  } وإلا {
    سیٹ_Error(Current_گرہ, "unexpected scalar");
  }
}

فراغ داخلہ::بلاک_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ داخلہ::scalar_ٹیگ(std::string &ٹیگ) {
  ٹیگ = Current_گرہ->گرہ_2->get_Verbatim_ٹیگ();
}

فراغ داخلہ::سیٹ_Error(HNode *hnode, ثابت Twine &سنیہا) {
  أكد(hnode && "HNode must not be NULL");
  سیٹ_Error(hnode->گرہ_2, سنیہا);
}

گرہ_قسم داخلہ::get_گرہ_قسم() {
  إذا (isa<ScalarHNode>(Current_گرہ))
    أرجع گرہ_قسم::Scalar;
  وإلا إذا (isa<MapHNode>(Current_گرہ))
    أرجع گرہ_قسم::Map;
  وإلا إذا (isa<SequenceHNode>(Current_گرہ))
    أرجع گرہ_قسم::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ داخلہ::سیٹ_Error(گرہ *گرہ_3, ثابت Twine &سنیہا) {
  Strm->printError(گرہ_3, سنیہا);
  EC = make_error_کوڈ(errc::invalid_دلیل);
}

فراغ داخلہ::سیٹ_Error(ثابت SMRange &range, ثابت Twine &سنیہا) {
  Strm->printError(range, سنیہا);
  EC = make_error_کوڈ(errc::invalid_دلیل);
}

فراغ داخلہ::reportWarning(HNode *hnode, ثابت Twine &سنیہا) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->گرہ_2, سنیہا, سروت_Mgr::DK_Warning);
}

فراغ داخلہ::reportWarning(گرہ *گرہ_3, ثابت Twine &سنیہا) {
  Strm->printError(گرہ_3, سنیہا, سروت_Mgr::DK_Warning);
}

فراغ داخلہ::reportWarning(ثابت SMRange &range, ثابت Twine &سنیہا) {
  Strm->printError(range, سنیہا, سروت_Mgr::DK_Warning);
}

فراغ داخلہ::releaseHNodeBuffers() {
  خالی_تھاواں_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ داخلہ::saveAliasHNode(گرہ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  إذا (!Anchor.خالی_تھاواں())
    // YAML 1.2.2 - 3.2.2.2. Anchors and Aliases:
    //
    // An alias event refers to the most recent event in the serialization
    // having the specified anchor. Therefore, anchors need not be unique within
    // a serialization. In addition, an anchor need not have an alias node
    // referring to it.
    AliasMap[Anchor] = HN;
}

داخلہ::HNode *داخلہ::createHNodes(گرہ *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة گرہ::NK_Scalar: {
    Scalar_گرہ *SN = dyn_cast<Scalar_گرہ>(N);
    StringRef کلید_Str = SN->get_قدر(StringStorage);
    إذا (!StringStorage.خالی_تھاواں()) {
      // Copy string to permanent storage
      کلید_Str = StringStorage.str().copy(StringAllocator);
    }
    خودکار *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, کلید_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة گرہ::NK_بلاک_Scalar: {
    بلاک_Scalar_گرہ *BSN = dyn_cast<بلاک_Scalar_گرہ>(N);
    StringRef قدر_Copy = BSN->get_قدر().copy(StringAllocator);
    خودکار *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, قدر_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة گرہ::NK_Sequence: {
    Sequence_گرہ *SQ = dyn_cast<Sequence_گرہ>(N);
    خودکار SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (گرہ &SN : *SQ) {
      خودکار اندراج = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.ڈالو_پٹھ(اندراج);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة گرہ::NK_Mapping: {
    Mapping_گرہ *Map = dyn_cast<Mapping_گرہ>(N);
    خودکار mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (کلید_قدر_گرہ &KVN : *Map) {
      گرہ *کلید_گرہ = KVN.get_کلید();
      Scalar_گرہ *کلید = dyn_cast_if_present<Scalar_گرہ>(کلید_گرہ);
      گرہ *قدر = KVN.get_قدر();
      إذا (!کلید || !قدر) {
        إذا (!کلید)
          سیٹ_Error(کلید_گرہ, "Map key must be a scalar");
        إذا (!قدر)
          سیٹ_Error(کلید_گرہ, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef کلید_Str = کلید->get_قدر(StringStorage);
      إذا (!StringStorage.خالی_تھاواں()) {
        // Copy string to permanent storage
        کلید_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.گنتی(کلید_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        سیٹ_Error(کلید_گرہ, Twine("duplicated mapping key '") + کلید_Str + "'");
      خودکار قدر_HNode = createHNodes(قدر);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[کلید_Str] =
          std::make_pair(std::move(قدر_HNode), کلید_گرہ->get_سروت_Range());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة گرہ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (خالی_تھاواں_HNode_Allocator.Allocate()) خالی_تھاواں_HNode(N);
  حالة گرہ::NK_Alias: {
    Alias_گرہ *AN = dyn_cast<Alias_گرہ>(N);
    خودکار AliasName = AN->getName();
    خودکار AHN = AliasMap.لبھو(AliasName);
    إذا (AHN == AliasMap.سرا()) {
      سیٹ_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      أرجع خالی_سوچک;
    }
    أرجع AHN->دوجا_2;
  }
  افتراضي:
    سیٹ_Error(N, "unknown node kind");
    أرجع خالی_سوچک;
  }
}

فراغ داخلہ::سیٹ_Error(ثابت Twine &سنیہا_2) {
  سیٹ_Error(Current_گرہ, سنیہا_2);
}

فراغ داخلہ::سیٹ_Allow_Unknown_کلیدیں(منطقي Allow) { Allow_Unknown_کلیدیں = Allow; }

منطقي داخلہ::can_Elide_خالی_تھاواں_Sequence() {
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, فراغ *context, صحيح Wrap_کالم)
    : IO(context), باہرلا(yout), Wrap_کالم(Wrap_کالم) {}

Output::~Output() = افتراضي;

منطقي Output::outputting() ثابت {
  أرجع سچ;
}

فراغ Output::beginMapping() {
  State_ڈھیر.ڈالو_پٹھ(وچ_Map_پہلا_کلید);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::map_ٹیگ(StringRef ٹیگ, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_ڈھیر.ناپ_2() > 1) {
      خودکار &E = State_ڈھیر[State_ڈھیر.ناپ_2() - 2];
      SequenceElement = وچ_Seq_Any_Element(E) || وچ_Flow_Seq_Any_Element(E);
    }
    إذا (SequenceElement && State_ڈھیر.پٹھ() == وچ_Map_پہلا_کلید) {
      new_سطر_Check();
    } وإلا {
      output(" ");
    }
    output(ٹیگ);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_ڈھیر.پٹھ() == وچ_Map_پہلا_کلید) {
        State_ڈھیر.نکالو_پٹھ();
        State_ڈھیر.ڈالو_پٹھ(وچ_Map_Other_کلید);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  أرجع Use;
}

فراغ Output::سرا_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  إذا (State_ڈھیر.پٹھ() == وچ_Map_پہلا_کلید) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check();
    output("{}");
    Padding = "\n";
  }
  State_ڈھیر.نکالو_پٹھ();
}

std::سمتیہ<StringRef> Output::کلیدیں() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_کلید(StringRef کلید, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = خالی_سوچک;
  إذا (Required || !SameAsDefault || Write_Default_اقدار) {
    خودکار State = State_ڈھیر.پٹھ();
    إذا (State == وچ_Flow_Map_پہلا_کلید || State == وچ_Flow_Map_Other_کلید) {
      flow_کلید(کلید);
    } وإلا {
      new_سطر_Check();
      padded_کلید(کلید);
    }
    أرجع سچ;
  }
  أرجع خطأ;
}

فراغ Output::postflight_کلید(فراغ *) {
  إذا (State_ڈھیر.پٹھ() == وچ_Map_پہلا_کلید) {
    State_ڈھیر.نکالو_پٹھ();
    State_ڈھیر.ڈالو_پٹھ(وچ_Map_Other_کلید);
  } وإلا إذا (State_ڈھیر.پٹھ() == وچ_Flow_Map_پہلا_کلید) {
    State_ڈھیر.نکالو_پٹھ();
    State_ڈھیر.ڈالو_پٹھ(وچ_Flow_Map_Other_کلید);
  }
}

فراغ Output::beginFlowMapping() {
  State_ڈھیر.ڈالو_پٹھ(وچ_Flow_Map_پہلا_کلید);
  new_سطر_Check();
  کالم_At_Map_Flow_شروع = کالم;
  output("{ ");
}

فراغ Output::سرا_Flow_Mapping() {
  State_ڈھیر.نکالو_پٹھ();
  output_اتے_منزل_سرا_دا_سطر(" }");
}

فراغ Output::beginDocuments() {
  output_اتے_منزل_سرا_دا_سطر("---");
}

منطقي Output::preflightDocument(بنانشانی اشاریہ) {
  إذا (اشاریہ > 0)
    output_اتے_منزل_سرا_دا_سطر("\n---");
  أرجع سچ;
}

فراغ Output::postflightDocument() {
}

فراغ Output::سرا_Documents() {
  output("\n...\n");
}

بنانشانی Output::beginSequence() {
  State_ڈھیر.ڈالو_پٹھ(وچ_Seq_پہلا_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::سرا_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_ڈھیر.پٹھ() == وچ_Seq_پہلا_Element) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check(/*EmptySequence=*/سچ);
    output("[]");
    Padding = "\n";
  }
  State_ڈھیر.نکالو_پٹھ();
}

منطقي Output::preflightElement(بنانشانی, فراغ *&SaveInfo) {
  SaveInfo = خالی_سوچک;
  أرجع سچ;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_ڈھیر.پٹھ() == وچ_Seq_پہلا_Element) {
    State_ڈھیر.نکالو_پٹھ();
    State_ڈھیر.ڈالو_پٹھ(وچ_Seq_Other_Element);
  } وإلا إذا (State_ڈھیر.پٹھ() == وچ_Flow_Seq_پہلا_Element) {
    State_ڈھیر.نکالو_پٹھ();
    State_ڈھیر.ڈالو_پٹھ(وچ_Flow_Seq_Other_Element);
  }
}

بنانشانی Output::beginFlowSequence() {
  State_ڈھیر.ڈالو_پٹھ(وچ_Flow_Seq_پہلا_Element);
  new_سطر_Check();
  کالم_At_Flow_شروع = کالم;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::سرا_Flow_Sequence() {
  State_ڈھیر.نکالو_پٹھ();
  output_اتے_منزل_سرا_دا_سطر(" ]");
}

منطقي Output::preflightFlowElement(بنانشانی, فراغ *&SaveInfo) {
  إذا (NeedFlowSequenceComma)
    output(", ");
  إذا (Wrap_کالم && کالم > Wrap_کالم) {
    output("\n");
    لكل (صحيح i = 0; i < کالم_At_Flow_شروع; ++i)
      output(" ");
    کالم = کالم_At_Flow_شروع;
    output("  ");
  }
  SaveInfo = خالی_سوچک;
  أرجع سچ;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = سچ;
}

فراغ Output::beginEnumScalar() {
  Enumeration_میل_لبھیا = خطأ;
}

منطقي Output::میل_Enum_Scalar(StringRef Str, منطقي میل_2) {
  إذا (میل_2 && !Enumeration_میل_لبھیا) {
    new_سطر_Check();
    output_اتے_منزل_سرا_دا_سطر(Str);
    Enumeration_میل_لبھیا = سچ;
  }
  أرجع خطأ;
}

منطقي Output::میل_Enum_Fallback() {
  إذا (Enumeration_میل_لبھیا)
    أرجع خطأ;
  Enumeration_میل_لبھیا = سچ;
  أرجع سچ;
}

فراغ Output::سرا_Enum_Scalar() {
  إذا (!Enumeration_میل_لبھیا)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::begin_ثنائی_ہندسہ_سیٹ_Scalar(منطقي &DoClear) {
  new_سطر_Check();
  output("[ ");
  Need_ثنائی_ہندسہ_قدر_Comma = خطأ;
  DoClear = خطأ;
  أرجع سچ;
}

منطقي Output::ثنائی_ہندسہ_سیٹ_میل(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_ثنائی_ہندسہ_قدر_Comma)
      output(", ");
    output(Str);
    Need_ثنائی_ہندسہ_قدر_Comma = سچ;
  }
  أرجع خطأ;
}

فراغ Output::سرا_ثنائی_ہندسہ_سیٹ_Scalar() {
  output_اتے_منزل_سرا_دا_سطر(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_سطر_Check();
  إذا (S.خالی_تھاواں()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_اتے_منزل_سرا_دا_سطر("''");
    أرجع;
  }
  output(S, MustQuote);
  output_اتے_منزل_سرا_دا_سطر("");
}

فراغ Output::بلاک_Scalar_String(StringRef &S) {
  إذا (!State_ڈھیر.خالی_تھاواں())
    new_سطر_Check();
  output(" |");

  بنانشانی Indent = State_ڈھیر.خالی_تھاواں() ? 1 : State_ڈھیر.ناپ_2();

  خودکار بفر = حافظہ_بفر::get_Mem_بفر(S, "", خطأ);
  لكل (سطر_iterator سطریں(*بفر, خطأ); !سطریں.is_at_سرا(); ++سطریں) {
    output_New_سطر();
    لكل (بنانشانی I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*سطریں);
  }
  output_اتے_منزل_سرا_دا_سطر("");
}

فراغ Output::scalar_ٹیگ(std::string &ٹیگ) {
  إذا (ٹیگ.خالی_تھاواں())
    أرجع;
  new_سطر_Check();
  output(ٹیگ);
  output(" ");
}

فراغ Output::سیٹ_Error(ثابت Twine &سنیہا) {
}

std::error_کوڈ Output::error() { أرجع {}; }

منطقي Output::can_Elide_خالی_تھاواں_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  إذا (State_ڈھیر.ناپ_2() < 2)
    أرجع سچ;
  إذا (State_ڈھیر.پٹھ() != وچ_Map_پہلا_کلید)
    أرجع سچ;
  أرجع !وچ_Seq_Any_Element(State_ڈھیر[State_ڈھیر.ناپ_2() - 2]);
}

فراغ Output::output(StringRef s) {
  کالم += s.ناپ_2();
  باہرلا << s;
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

  بنانشانی i = 0;
  بنانشانی j = 0;
  بنانشانی سرا_2 = S.ناپ_2();
  ثابت حرف *ادھار = S.انکڑے();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  طالما (j < سرا_2) {
    إذا (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&ادھار[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&ادھار[i], j - i));
  output(Quote); // Ending quote.
}

فراغ Output::output_اتے_منزل_سرا_دا_سطر(StringRef s) {
  output(s);
  إذا (State_ڈھیر.خالی_تھاواں() || (!وچ_Flow_Seq_Any_Element(State_ڈھیر.پٹھ()) &&
                             !وچ_Flow_Map_Any_کلید(State_ڈھیر.پٹھ())))
    Padding = "\n";
}

فراغ Output::output_New_سطر() {
  باہرلا << "\n";
  کالم = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_سطر_Check(منطقي خالی_تھاواں_Sequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_سطر();
  Padding = {};

  إذا (State_ڈھیر.ناپ_2() == 0 || خالی_تھاواں_Sequence)
    أرجع;

  بنانشانی Indent = State_ڈھیر.ناپ_2() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  خودکار I = State_ڈھیر.rbegin(), E = State_ڈھیر.rend();

  إذا (وچ_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = سچ; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == وچ_Map_پہلا_کلید || *I == وچ_Flow_Map_پہلا_کلید ||
             وچ_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = سچ;
    ++I; // Skip back().
  }

  بنانشانی Output_Dash_گنتی = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!وچ_Seq_Any_Element(*I))
        اكسر;

      ++Output_Dash_گنتی;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != وچ_Seq_پہلا_Element)
        اكسر;
    }
  }

  لكل (بنانشانی I = Output_Dash_گنتی; I < Indent; ++I)
    output("  ");

  لكل (بنانشانی I = 0; I < Output_Dash_گنتی; ++I)
    output("- ");
}

فراغ Output::padded_کلید(StringRef کلید_2) {
  output(کلید_2, needsQuotes(کلید_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (کلید_2.ناپ_2() < لڑی_لمبائی(spaces))
    Padding = &spaces[کلید_2.ناپ_2()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_کلید(StringRef کلید) {
  إذا (State_ڈھیر.پٹھ() == وچ_Flow_Map_Other_کلید)
    output(", ");
  إذا (Wrap_کالم && کالم > Wrap_کالم) {
    output("\n");
    لكل (صحيح I = 0; I < کالم_At_Map_Flow_شروع; ++I)
      output(" ");
    کالم = کالم_At_Map_Flow_شروع;
    output("  ");
  }
  output(کلید, needsQuotes(کلید, خطأ));
  output(": ");
}

گرہ_قسم Output::get_گرہ_قسم() { report_fatal_error("invalid call"); }

منطقي Output::وچ_Seq_Any_Element(وچ_State State) {
  أرجع State == وچ_Seq_پہلا_Element || State == وچ_Seq_Other_Element;
}

منطقي Output::وچ_Flow_Seq_Any_Element(وچ_State State) {
  أرجع State == وچ_Flow_Seq_پہلا_Element || State == وچ_Flow_Seq_Other_Element;
}

منطقي Output::وچ_Map_Any_کلید(وچ_State State) {
  أرجع State == وچ_Map_پہلا_کلید || State == وچ_Map_Other_کلید;
}

منطقي Output::وچ_Flow_Map_Any_کلید(وچ_State State) {
  أرجع State == وچ_Flow_Map_پہلا_کلید || State == وچ_Flow_Map_Other_کلید;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &باہرلا) {
  باہرلا << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::داخلہ_2(StringRef Scalar, فراغ *, منطقي &Val) {
  إذا (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    أرجع StringRef();
  }
  أرجع "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &باہرلا) {
  باہرلا << Val;
}

StringRef ScalarTraits<StringRef>::داخلہ_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &باہرلا) {
  باہرلا << Val;
}

StringRef ScalarTraits<std::string>::داخلہ_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &باہرلا) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t عدد = Val;
  باہرلا << عدد;
}

StringRef ScalarTraits<uint8_t>::داخلہ_2(StringRef Scalar, فراغ *, uint8_t &Val) {
  بنانشانی لما لما n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &باہرلا) {
  باہرلا << Val;
}

StringRef ScalarTraits<uint16_t>::داخلہ_2(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  بنانشانی لما لما n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &باہرلا) {
  باہرلا << Val;
}

StringRef ScalarTraits<uint32_t>::داخلہ_2(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  بنانشانی لما لما n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &باہرلا) {
  باہرلا << Val;
}

StringRef ScalarTraits<uint64_t>::داخلہ_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  بنانشانی لما لما N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &باہرلا) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t عدد = Val;
  باہرلا << عدد;
}

StringRef ScalarTraits<int8_t>::داخلہ_2(StringRef Scalar, فراغ *, int8_t &Val) {
  لما لما N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > 127) || (N < -128))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &باہرلا) {
  باہرلا << Val;
}

StringRef ScalarTraits<int16_t>::داخلہ_2(StringRef Scalar, فراغ *, int16_t &Val) {
  لما لما N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT16_MAX) || (N < INT16_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &باہرلا) {
  باہرلا << Val;
}

StringRef ScalarTraits<int32_t>::داخلہ_2(StringRef Scalar, فراغ *, int32_t &Val) {
  لما لما N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT32_MAX) || (N < INT32_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &باہرلا) {
  باہرلا << Val;
}

StringRef ScalarTraits<int64_t>::داخلہ_2(StringRef Scalar, فراغ *, int64_t &Val) {
  لما لما N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &باہرلا) {
  باہرلا << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::داخلہ_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (منزل_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &باہرلا) {
  باہرلا << format("%g", Val);
}

StringRef ScalarTraits<عائم>::داخلہ_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (منزل_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &باہرلا) {
  باہرلا << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::داخلہ_2(StringRef Scalar, فراغ *, Hex8 &Val) {
  بنانشانی لما لما n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex8 number";
  إذا (n > 0xFF)
    أرجع "out of range hex8 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &باہرلا) {
  باہرلا << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::داخلہ_2(StringRef Scalar, فراغ *, Hex16 &Val) {
  بنانشانی لما لما n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex16 number";
  إذا (n > 0xFFFF)
    أرجع "out of range hex16 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &باہرلا) {
  باہرلا << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::داخلہ_2(StringRef Scalar, فراغ *, Hex32 &Val) {
  بنانشانی لما لما n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex32 number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range hex32 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &باہرلا) {
  باہرلا << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::داخلہ_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  بنانشانی لما لما عدد;
  إذا (getAsUnsignedInteger(Scalar, 0, عدد))
    أرجع "invalid hex64 number";
  Val = عدد;
  أرجع StringRef();
}

فراغ ScalarTraits<نسخہ_Tuple>::output(ثابت نسخہ_Tuple &Val, فراغ *,
                                        llvm::raw_ostream &باہرلا) {
  باہرلا << Val.getAsString();
}

StringRef ScalarTraits<نسخہ_Tuple>::داخلہ_2(StringRef Scalar, فراغ *,
                                            نسخہ_Tuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
