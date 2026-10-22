//===- lib/Support/YAMLTraits.cpp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/YAMLTraits.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/Casting.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/LineIterator.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/VersionTuple.h"
#شامل "llvm/Support/YAMLParser.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>
#شامل <cstdint>
#شامل <cstring>
#شامل <string>
#شامل <vector>

استعمال نامخلا llvm;
استعمال نامخلا yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(خالی *Context) : Ctxt(Context) {}

IO::~IO() = طےشدہ;

خالی *IO::getContext() ثابت {
  واپس Ctxt;
}

خالی IO::سیٹ_Context(خالی *Context) {
  Ctxt = Context;
}

خالی IO::سیٹ_Allow_Unknown_کلیدیں(منطقی Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//داخلہ
//===----------------------------------------------------------------------===//

داخلہ::داخلہ(StringRef داخلہ_Content, خالی *Ctxt,
             ماخذ_Mgr::DiagHandlerTy DiagHandler, خالی *DiagHandlerCtxt)
    : IO(Ctxt), Strm(نیا دھارا(داخلہ_Content, SrcMgr, جھوٹ, &EC)) {
  اگر (DiagHandler)
    SrcMgr.سیٹ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

داخلہ::داخلہ(حافظہ_بفر_Ref داخلہ, خالی *Ctxt,
             ماخذ_Mgr::DiagHandlerTy DiagHandler, خالی *DiagHandlerCtxt)
    : IO(Ctxt), Strm(نیا دھارا(داخلہ, SrcMgr, جھوٹ, &EC)) {
  اگر (DiagHandler)
    SrcMgr.سیٹ_Diag_Handler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

داخلہ::~داخلہ() = طےشدہ;

std::error_رمز داخلہ::error() { واپس EC; }

منطقی داخلہ::outputting() ثابت {
  واپس جھوٹ;
}

منطقی داخلہ::سیٹ_Current_Document() {
  اگر (DocIterator != Strm->انتہا()) {
    گرہ *N = DocIterator->get_جڑ();
    اگر (!N) {
      EC = make_error_رمز(errc::invalid_دلیل);
      واپس جھوٹ;
    }

    اگر (isa<Null_گرہ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      واپس سیٹ_Current_Document();
    }
    releaseHNodeBuffers();
    چوٹی_گرہ = createHNodes(N);
    Current_گرہ = چوٹی_گرہ;
    واپس سچ;
  }
  واپس جھوٹ;
}

منطقی داخلہ::اگلا_Document() {
  واپس ++DocIterator != Strm->انتہا();
}

ثابت گرہ *داخلہ::get_Current_گرہ() ثابت {
  واپس Current_گرہ ? Current_گرہ->گرہ_2 : خالی_اشاریہ;
}

منطقی داخلہ::map_لیبل(StringRef لیبل, منطقی Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  اگر (!Current_گرہ)
    واپس جھوٹ;

  std::string ملا_لیبل = Current_گرہ->گرہ_2->get_Verbatim_لیبل();
  اگر (ملا_لیبل.خالی_خانے()) {
    // If no tag found and 'Tag' is the default, say it was found.
    واپس Default;
  }
  // Return true iff found tag matches supplied tag.
  واپس لیبل == ملا_لیبل;
}

خالی داخلہ::beginMapping() {
  اگر (EC)
    واپس;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گرہ);
  اگر (MN) {
    MN->Valid_کلیدیں.clear();
  }
}

std::سمتیہ<StringRef> داخلہ::کلیدیں() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_گرہ);
  std::سمتیہ<StringRef> Ret;
  اگر (!MN) {
    سیٹ_Error(Current_گرہ, "not a mapping");
    واپس Ret;
  }
  برائے (خودکار &P : MN->Mapping)
    Ret.ڈالو_عقب(P.پہلا_2());
  واپس Ret;
}

منطقی داخلہ::preflight_کلید(StringRef کلید, منطقی Required, منطقی, منطقی &UseDefault,
                         خالی *&SaveInfo) {
  UseDefault = جھوٹ;
  اگر (EC)
    واپس جھوٹ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  اگر (!Current_گرہ) {
    اگر (Required)
      EC = make_error_رمز(errc::invalid_دلیل);
    ورنہ
      UseDefault = سچ;
    واپس جھوٹ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_گرہ);
  اگر (!MN) {
    اگر (Required || !isa<خالی_خانے_HNode>(Current_گرہ))
      سیٹ_Error(Current_گرہ, "not a mapping");
    ورنہ
      UseDefault = سچ;
    واپس جھوٹ;
  }
  MN->Valid_کلیدیں.ڈالو_عقب(کلید.str());
  HNode *قدر = MN->Mapping[کلید].پہلا_2;
  اگر (!قدر) {
    اگر (Required)
      سیٹ_Error(Current_گرہ, Twine("missing required key '") + کلید + "'");
    ورنہ
      UseDefault = سچ;
    واپس جھوٹ;
  }
  SaveInfo = Current_گرہ;
  Current_گرہ = قدر;
  واپس سچ;
}

خالی داخلہ::postflight_کلید(خالی *saveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(saveInfo);
}

خالی داخلہ::انتہا_Mapping() {
  اگر (EC)
    واپس;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گرہ);
  اگر (!MN)
    واپس;
  برائے (ثابت خودکار &NN : MN->Mapping) {
    اگر (!is_contained(MN->Valid_کلیدیں, NN.پہلا_2())) {
      ثابت SMRange &ReportLoc = NN.دوسرا_2.دوسرا_2;
      اگر (!Allow_Unknown_کلیدیں) {
        سیٹ_Error(ReportLoc, Twine("unknown key '") + NN.پہلا_2() + "'");
        توڑو;
      } ورنہ
        reportWarning(ReportLoc, Twine("unknown key '") + NN.پہلا_2() + "'");
    }
  }
}

خالی داخلہ::beginFlowMapping() { beginMapping(); }

خالی داخلہ::انتہا_Flow_Mapping() { انتہا_Mapping(); }

غیرعلامتی داخلہ::beginSequence() {
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ))
    واپس SQ->Entries.سائز_2();
  اگر (isa<خالی_خانے_HNode>(Current_گرہ))
    واپس 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  اگر (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    اگر (isNull(SN->قدر_2()))
      واپس 0;
  }
  // Any other type of HNode is an error.
  سیٹ_Error(Current_گرہ, "not a sequence");
  واپس 0;
}

خالی داخلہ::انتہا_Sequence() {
}

منطقی داخلہ::preflightElement(غیرعلامتی اشاریہ_2, خالی *&SaveInfo) {
  اگر (EC)
    واپس جھوٹ;
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    SaveInfo = Current_گرہ;
    Current_گرہ = SQ->Entries[اشاریہ_2];
    واپس سچ;
  }
  واپس جھوٹ;
}

خالی داخلہ::postflightElement(خالی *SaveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(SaveInfo);
}

غیرعلامتی داخلہ::beginFlowSequence() { واپس beginSequence(); }

منطقی داخلہ::preflightFlowElement(غیرعلامتی اشاریہ, خالی *&SaveInfo) {
  اگر (EC)
    واپس جھوٹ;
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    SaveInfo = Current_گرہ;
    Current_گرہ = SQ->Entries[اشاریہ];
    واپس سچ;
  }
  واپس جھوٹ;
}

خالی داخلہ::postflightFlowElement(خالی *SaveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(SaveInfo);
}

خالی داخلہ::انتہا_Flow_Sequence() {
}

خالی داخلہ::beginEnumScalar() {
  Scalar_مطابقت_ملا = جھوٹ;
}

منطقی داخلہ::مطابقت_Enum_Scalar(StringRef Str, منطقی) {
  اگر (Scalar_مطابقت_ملا)
    واپس جھوٹ;
  اگر (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    اگر (SN->قدر_2() == Str) {
      Scalar_مطابقت_ملا = سچ;
      واپس سچ;
    }
  }
  واپس جھوٹ;
}

منطقی داخلہ::مطابقت_Enum_Fallback() {
  اگر (Scalar_مطابقت_ملا)
    واپس جھوٹ;
  Scalar_مطابقت_ملا = سچ;
  واپس سچ;
}

خالی داخلہ::انتہا_Enum_Scalar() {
  اگر (!Scalar_مطابقت_ملا) {
    سیٹ_Error(Current_گرہ, "unknown enumerated scalar");
  }
}

منطقی داخلہ::begin_ثنائی_ہندسہ_سیٹ_Scalar(منطقی &DoClear) {
  ثنائی_ہندسہ_اقدار_مستعمل.clear();
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    ثنائی_ہندسہ_اقدار_مستعمل.resize(SQ->Entries.سائز_2());
  } ورنہ {
    سیٹ_Error(Current_گرہ, "expected sequence of bit values");
  }
  DoClear = سچ;
  واپس سچ;
}

منطقی داخلہ::ثنائی_ہندسہ_سیٹ_مطابقت(StringRef Str, منطقی) {
  اگر (EC)
    واپس جھوٹ;
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    غیرعلامتی اشاریہ_2 = 0;
    برائے (خودکار &N : SQ->Entries) {
      اگر (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        اگر (SN->قدر_2() == Str) {
          ثنائی_ہندسہ_اقدار_مستعمل[اشاریہ_2] = سچ;
          واپس سچ;
        }
      } ورنہ {
        سیٹ_Error(Current_گرہ, "unexpected scalar in sequence of bit values");
      }
      ++اشاریہ_2;
    }
  } ورنہ {
    سیٹ_Error(Current_گرہ, "expected sequence of bit values");
  }
  واپس جھوٹ;
}

خالی داخلہ::انتہا_ثنائی_ہندسہ_سیٹ_Scalar() {
  اگر (EC)
    واپس;
  اگر (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    أكد(ثنائی_ہندسہ_اقدار_مستعمل.سائز_2() == SQ->Entries.سائز_2());
    برائے (غیرعلامتی i = 0; i < SQ->Entries.سائز_2(); ++i) {
      اگر (!ثنائی_ہندسہ_اقدار_مستعمل[i]) {
        سیٹ_Error(SQ->Entries[i], "unknown bit value");
        واپس;
      }
    }
  }
}

خالی داخلہ::scalarString(StringRef &S, QuotingType) {
  اگر (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    S = SN->قدر_2();
  } ورنہ {
    سیٹ_Error(Current_گرہ, "unexpected scalar");
  }
}

خالی داخلہ::بلاک_Scalar_String(StringRef &S) { scalarString(S, QuotingType::None); }

خالی داخلہ::scalar_لیبل(std::string &لیبل) {
  لیبل = Current_گرہ->گرہ_2->get_Verbatim_لیبل();
}

خالی داخلہ::سیٹ_Error(HNode *hnode, ثابت Twine &پیغام_2) {
  أكد(hnode && "HNode must not be NULL");
  سیٹ_Error(hnode->گرہ_2, پیغام_2);
}

گرہ_قسم داخلہ::get_گرہ_قسم() {
  اگر (isa<ScalarHNode>(Current_گرہ))
    واپس گرہ_قسم::Scalar;
  ورنہ اگر (isa<MapHNode>(Current_گرہ))
    واپس گرہ_قسم::Map;
  ورنہ اگر (isa<SequenceHNode>(Current_گرہ))
    واپس گرہ_قسم::Sequence;
  llvm_unreachable("Unsupported node kind");
}

خالی داخلہ::سیٹ_Error(گرہ *گرہ_3, ثابت Twine &پیغام_2) {
  Strm->printError(گرہ_3, پیغام_2);
  EC = make_error_رمز(errc::invalid_دلیل);
}

خالی داخلہ::سیٹ_Error(ثابت SMRange &range, ثابت Twine &پیغام_2) {
  Strm->printError(range, پیغام_2);
  EC = make_error_رمز(errc::invalid_دلیل);
}

خالی داخلہ::reportWarning(HNode *hnode, ثابت Twine &پیغام_2) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->گرہ_2, پیغام_2, ماخذ_Mgr::DK_Warning);
}

خالی داخلہ::reportWarning(گرہ *گرہ_3, ثابت Twine &پیغام_2) {
  Strm->printError(گرہ_3, پیغام_2, ماخذ_Mgr::DK_Warning);
}

خالی داخلہ::reportWarning(ثابت SMRange &range, ثابت Twine &پیغام_2) {
  Strm->printError(range, پیغام_2, ماخذ_Mgr::DK_Warning);
}

خالی داخلہ::releaseHNodeBuffers() {
  خالی_خانے_HNode_Allocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

خالی داخلہ::saveAliasHNode(گرہ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  اگر (!Anchor.خالی_خانے())
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
  انتخاب (N->getType()) {
  حالت گرہ::NK_Scalar: {
    Scalar_گرہ *SN = dyn_cast<Scalar_گرہ>(N);
    StringRef کلید_Str = SN->get_قدر(StringStorage);
    اگر (!StringStorage.خالی_خانے()) {
      // Copy string to permanent storage
      کلید_Str = StringStorage.str().copy(StringAllocator);
    }
    خودکار *SHNode = نیا (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, کلید_Str);
    saveAliasHNode(SN, SHNode);
    واپس SHNode;
  }
  حالت گرہ::NK_بلاک_Scalar: {
    بلاک_Scalar_گرہ *BSN = dyn_cast<بلاک_Scalar_گرہ>(N);
    StringRef قدر_Copy = BSN->get_قدر().copy(StringAllocator);
    خودکار *BSHNode =
        نیا (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, قدر_Copy);
    saveAliasHNode(BSN, BSHNode);
    واپس BSHNode;
  }
  حالت گرہ::NK_Sequence: {
    Sequence_گرہ *SQ = dyn_cast<Sequence_گرہ>(N);
    خودکار SQHNode = نیا (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    برائے (گرہ &SN : *SQ) {
      خودکار اندراج_2 = createHNodes(&SN);
      اگر (EC)
        توڑو;
      SQHNode->Entries.ڈالو_عقب(اندراج_2);
    }
    saveAliasHNode(SQ, SQHNode);
    واپس SQHNode;
  }
  حالت گرہ::NK_Mapping: {
    Mapping_گرہ *Map = dyn_cast<Mapping_گرہ>(N);
    خودکار mapHNode = نیا (MapHNodeAllocator.Allocate()) MapHNode(N);
    برائے (کلید_قدر_گرہ &KVN : *Map) {
      گرہ *کلید_گرہ = KVN.get_کلید();
      Scalar_گرہ *کلید = dyn_cast_if_present<Scalar_گرہ>(کلید_گرہ);
      گرہ *قدر = KVN.get_قدر();
      اگر (!کلید || !قدر) {
        اگر (!کلید)
          سیٹ_Error(کلید_گرہ, "Map key must be a scalar");
        اگر (!قدر)
          سیٹ_Error(کلید_گرہ, "Map value must not be empty");
        توڑو;
      }
      StringStorage.clear();
      StringRef کلید_Str = کلید->get_قدر(StringStorage);
      اگر (!StringStorage.خالی_خانے()) {
        // Copy string to permanent storage
        کلید_Str = StringStorage.str().copy(StringAllocator);
      }
      اگر (mapHNode->Mapping.گنتی(کلید_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        سیٹ_Error(کلید_گرہ, Twine("duplicated mapping key '") + کلید_Str + "'");
      خودکار قدر_HNode = createHNodes(قدر);
      اگر (EC)
        توڑو;
      mapHNode->Mapping[کلید_Str] =
          std::make_pair(std::move(قدر_HNode), کلید_گرہ->get_ماخذ_Range());
    }
    saveAliasHNode(Map, mapHNode);
    واپس std::move(mapHNode);
  }
  حالت گرہ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    واپس نیا (خالی_خانے_HNode_Allocator.Allocate()) خالی_خانے_HNode(N);
  حالت گرہ::NK_Alias: {
    Alias_گرہ *AN = dyn_cast<Alias_گرہ>(N);
    خودکار AliasName = AN->getName();
    خودکار AHN = AliasMap.ڈھونڈو(AliasName);
    اگر (AHN == AliasMap.انتہا()) {
      سیٹ_Error(AN, Twine("undefined alias '" + AliasName + "'"));
      واپس خالی_اشاریہ;
    }
    واپس AHN->دوسرا_2;
  }
  طےشدہ:
    سیٹ_Error(N, "unknown node kind");
    واپس خالی_اشاریہ;
  }
}

خالی داخلہ::سیٹ_Error(ثابت Twine &پیغام_3) {
  سیٹ_Error(Current_گرہ, پیغام_3);
}

خالی داخلہ::سیٹ_Allow_Unknown_کلیدیں(منطقی Allow) { Allow_Unknown_کلیدیں = Allow; }

منطقی داخلہ::can_Elide_خالی_خانے_Sequence() {
  واپس جھوٹ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, خالی *context, صحیح Wrap_عمود)
    : IO(context), برآمد(yout), Wrap_عمود(Wrap_عمود) {}

Output::~Output() = طےشدہ;

منطقی Output::outputting() ثابت {
  واپس سچ;
}

خالی Output::beginMapping() {
  State_ڈھیر.ڈالو_عقب(اندر_Map_پہلا_کلید);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقی Output::map_لیبل(StringRef لیبل, منطقی Use) {
  اگر (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقی SequenceElement = جھوٹ;
    اگر (State_ڈھیر.سائز_2() > 1) {
      خودکار &E = State_ڈھیر[State_ڈھیر.سائز_2() - 2];
      SequenceElement = اندر_Seq_Any_Element(E) || اندر_Flow_Seq_Any_Element(E);
    }
    اگر (SequenceElement && State_ڈھیر.عقب() == اندر_Map_پہلا_کلید) {
      new_سطر_Check();
    } ورنہ {
      output(" ");
    }
    output(لیبل);
    اگر (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      اگر (State_ڈھیر.عقب() == اندر_Map_پہلا_کلید) {
        State_ڈھیر.نکالو_عقب();
        State_ڈھیر.ڈالو_عقب(اندر_Map_Other_کلید);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  واپس Use;
}

خالی Output::انتہا_Mapping() {
  // If we did not map anything, we should explicitly emit an empty map
  اگر (State_ڈھیر.عقب() == اندر_Map_پہلا_کلید) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check();
    output("{}");
    Padding = "\n";
  }
  State_ڈھیر.نکالو_عقب();
}

std::سمتیہ<StringRef> Output::کلیدیں() {
  report_fatal_error("invalid call");
}

منطقی Output::preflight_کلید(StringRef کلید, منطقی Required, منطقی SameAsDefault,
                          منطقی &UseDefault, خالی *&SaveInfo) {
  UseDefault = جھوٹ;
  SaveInfo = خالی_اشاریہ;
  اگر (Required || !SameAsDefault || Write_Default_اقدار) {
    خودکار State = State_ڈھیر.عقب();
    اگر (State == اندر_Flow_Map_پہلا_کلید || State == اندر_Flow_Map_Other_کلید) {
      flow_کلید(کلید);
    } ورنہ {
      new_سطر_Check();
      padded_کلید(کلید);
    }
    واپس سچ;
  }
  واپس جھوٹ;
}

خالی Output::postflight_کلید(خالی *) {
  اگر (State_ڈھیر.عقب() == اندر_Map_پہلا_کلید) {
    State_ڈھیر.نکالو_عقب();
    State_ڈھیر.ڈالو_عقب(اندر_Map_Other_کلید);
  } ورنہ اگر (State_ڈھیر.عقب() == اندر_Flow_Map_پہلا_کلید) {
    State_ڈھیر.نکالو_عقب();
    State_ڈھیر.ڈالو_عقب(اندر_Flow_Map_Other_کلید);
  }
}

خالی Output::beginFlowMapping() {
  State_ڈھیر.ڈالو_عقب(اندر_Flow_Map_پہلا_کلید);
  new_سطر_Check();
  عمود_At_Map_Flow_آغاز = عمود;
  output("{ ");
}

خالی Output::انتہا_Flow_Mapping() {
  State_ڈھیر.نکالو_عقب();
  output_اوپر_مقصد_انتہا_کا_سطر(" }");
}

خالی Output::beginDocuments() {
  output_اوپر_مقصد_انتہا_کا_سطر("---");
}

منطقی Output::preflightDocument(غیرعلامتی اشاریہ) {
  اگر (اشاریہ > 0)
    output_اوپر_مقصد_انتہا_کا_سطر("\n---");
  واپس سچ;
}

خالی Output::postflightDocument() {
}

خالی Output::انتہا_Documents() {
  output("\n...\n");
}

غیرعلامتی Output::beginSequence() {
  State_ڈھیر.ڈالو_عقب(اندر_Seq_پہلا_Element);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  واپس 0;
}

خالی Output::انتہا_Sequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  اگر (State_ڈھیر.عقب() == اندر_Seq_پہلا_Element) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check(/*EmptySequence=*/سچ);
    output("[]");
    Padding = "\n";
  }
  State_ڈھیر.نکالو_عقب();
}

منطقی Output::preflightElement(غیرعلامتی, خالی *&SaveInfo) {
  SaveInfo = خالی_اشاریہ;
  واپس سچ;
}

خالی Output::postflightElement(خالی *) {
  اگر (State_ڈھیر.عقب() == اندر_Seq_پہلا_Element) {
    State_ڈھیر.نکالو_عقب();
    State_ڈھیر.ڈالو_عقب(اندر_Seq_Other_Element);
  } ورنہ اگر (State_ڈھیر.عقب() == اندر_Flow_Seq_پہلا_Element) {
    State_ڈھیر.نکالو_عقب();
    State_ڈھیر.ڈالو_عقب(اندر_Flow_Seq_Other_Element);
  }
}

غیرعلامتی Output::beginFlowSequence() {
  State_ڈھیر.ڈالو_عقب(اندر_Flow_Seq_پہلا_Element);
  new_سطر_Check();
  عمود_At_Flow_آغاز = عمود;
  output("[ ");
  NeedFlowSequenceComma = جھوٹ;
  واپس 0;
}

خالی Output::انتہا_Flow_Sequence() {
  State_ڈھیر.نکالو_عقب();
  output_اوپر_مقصد_انتہا_کا_سطر(" ]");
}

منطقی Output::preflightFlowElement(غیرعلامتی, خالی *&SaveInfo) {
  اگر (NeedFlowSequenceComma)
    output(", ");
  اگر (Wrap_عمود && عمود > Wrap_عمود) {
    output("\n");
    برائے (صحیح i = 0; i < عمود_At_Flow_آغاز; ++i)
      output(" ");
    عمود = عمود_At_Flow_آغاز;
    output("  ");
  }
  SaveInfo = خالی_اشاریہ;
  واپس سچ;
}

خالی Output::postflightFlowElement(خالی *) {
  NeedFlowSequenceComma = سچ;
}

خالی Output::beginEnumScalar() {
  Enumeration_مطابقت_ملا = جھوٹ;
}

منطقی Output::مطابقت_Enum_Scalar(StringRef Str, منطقی مطابقت_2) {
  اگر (مطابقت_2 && !Enumeration_مطابقت_ملا) {
    new_سطر_Check();
    output_اوپر_مقصد_انتہا_کا_سطر(Str);
    Enumeration_مطابقت_ملا = سچ;
  }
  واپس جھوٹ;
}

منطقی Output::مطابقت_Enum_Fallback() {
  اگر (Enumeration_مطابقت_ملا)
    واپس جھوٹ;
  Enumeration_مطابقت_ملا = سچ;
  واپس سچ;
}

خالی Output::انتہا_Enum_Scalar() {
  اگر (!Enumeration_مطابقت_ملا)
    llvm_unreachable("bad runtime enum value");
}

منطقی Output::begin_ثنائی_ہندسہ_سیٹ_Scalar(منطقی &DoClear) {
  new_سطر_Check();
  output("[ ");
  Need_ثنائی_ہندسہ_قدر_Comma = جھوٹ;
  DoClear = جھوٹ;
  واپس سچ;
}

منطقی Output::ثنائی_ہندسہ_سیٹ_مطابقت(StringRef Str, منطقی Matches) {
  اگر (Matches) {
    اگر (Need_ثنائی_ہندسہ_قدر_Comma)
      output(", ");
    output(Str);
    Need_ثنائی_ہندسہ_قدر_Comma = سچ;
  }
  واپس جھوٹ;
}

خالی Output::انتہا_ثنائی_ہندسہ_سیٹ_Scalar() {
  output_اوپر_مقصد_انتہا_کا_سطر(" ]");
}

خالی Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_سطر_Check();
  اگر (S.خالی_خانے()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_اوپر_مقصد_انتہا_کا_سطر("''");
    واپس;
  }
  output(S, MustQuote);
  output_اوپر_مقصد_انتہا_کا_سطر("");
}

خالی Output::بلاک_Scalar_String(StringRef &S) {
  اگر (!State_ڈھیر.خالی_خانے())
    new_سطر_Check();
  output(" |");

  غیرعلامتی Indent = State_ڈھیر.خالی_خانے() ? 1 : State_ڈھیر.سائز_2();

  خودکار بفر = حافظہ_بفر::get_Mem_بفر(S, "", جھوٹ);
  برائے (سطر_iterator سطریں(*بفر, جھوٹ); !سطریں.is_at_انتہا(); ++سطریں) {
    output_New_سطر();
    برائے (غیرعلامتی I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*سطریں);
  }
  output_اوپر_مقصد_انتہا_کا_سطر("");
}

خالی Output::scalar_لیبل(std::string &لیبل) {
  اگر (لیبل.خالی_خانے())
    واپس;
  new_سطر_Check();
  output(لیبل);
  output(" ");
}

خالی Output::سیٹ_Error(ثابت Twine &پیغام_2) {
}

std::error_رمز Output::error() { واپس {}; }

منطقی Output::can_Elide_خالی_خانے_Sequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  اگر (State_ڈھیر.سائز_2() < 2)
    واپس سچ;
  اگر (State_ڈھیر.عقب() != اندر_Map_پہلا_کلید)
    واپس سچ;
  واپس !اندر_Seq_Any_Element(State_ڈھیر[State_ڈھیر.سائز_2() - 2]);
}

خالی Output::output(StringRef s) {
  عمود += s.سائز_2();
  برآمد << s;
}

خالی Output::output(StringRef S, QuotingType MustQuote) {
  اگر (MustQuote == QuotingType::None) {
    // Only quote if we must.
    output(S);
    واپس;
  }

  StringLiteral Quote = MustQuote == QuotingType::Single ? StringLiteral("'")
                                                         : StringLiteral("\"");
  output(Quote); // Starting quote.

  // When using double-quoted strings (and only in that case), non-printable
  // characters may be present, and will be escaped using a variety of
  // unicode-scalar and special short-form escapes. This is handled in
  // yaml::escape.
  اگر (MustQuote == QuotingType::Double) {
    output(yaml::escape(S, /* EscapePrintable= */ جھوٹ));
    output(Quote);
    واپس;
  }

  غیرعلامتی i = 0;
  غیرعلامتی j = 0;
  غیرعلامتی انتہا_2 = S.سائز_2();
  ثابت حرف *بنیاد = S.ڈیٹا();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  جب (j < انتہا_2) {
    اگر (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&بنیاد[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&بنیاد[i], j - i));
  output(Quote); // Ending quote.
}

خالی Output::output_اوپر_مقصد_انتہا_کا_سطر(StringRef s) {
  output(s);
  اگر (State_ڈھیر.خالی_خانے() || (!اندر_Flow_Seq_Any_Element(State_ڈھیر.عقب()) &&
                             !اندر_Flow_Map_Any_کلید(State_ڈھیر.عقب())))
    Padding = "\n";
}

خالی Output::output_New_سطر() {
  برآمد << "\n";
  عمود = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

خالی Output::new_سطر_Check(منطقی خالی_خانے_Sequence) {
  اگر (Padding != "\n") {
    output(Padding);
    Padding = {};
    واپس;
  }
  output_New_سطر();
  Padding = {};

  اگر (State_ڈھیر.سائز_2() == 0 || خالی_خانے_Sequence)
    واپس;

  غیرعلامتی Indent = State_ڈھیر.سائز_2() - 1;
  منطقی PossiblyNestedSeq = جھوٹ;
  خودکار I = State_ڈھیر.rbegin(), E = State_ڈھیر.rend();

  اگر (اندر_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = سچ; // Not possibly but always.
    ++Indent;
  } ورنہ اگر (*I == اندر_Map_پہلا_کلید || *I == اندر_Flow_Map_پہلا_کلید ||
             اندر_Flow_Seq_Any_Element(*I)) {
    PossiblyNestedSeq = سچ;
    ++I; // Skip back().
  }

  غیرعلامتی Output_Dash_گنتی = 0;
  اگر (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    جب (I != E) {
      // Don't count the top of nested sequence.
      اگر (!اندر_Seq_Any_Element(*I))
        توڑو;

      ++Output_Dash_گنتی;

      // Stop counting if consecutive inSeqFirstElement ends.
      اگر (*I++ != اندر_Seq_پہلا_Element)
        توڑو;
    }
  }

  برائے (غیرعلامتی I = Output_Dash_گنتی; I < Indent; ++I)
    output("  ");

  برائے (غیرعلامتی I = 0; I < Output_Dash_گنتی; ++I)
    output("- ");
}

خالی Output::padded_کلید(StringRef کلید_2) {
  output(کلید_2, needsQuotes(کلید_2, جھوٹ));
  output(":");
  ثابت حرف *spaces = "                ";
  اگر (کلید_2.سائز_2() < لڑی_لمبائی(spaces))
    Padding = &spaces[کلید_2.سائز_2()];
  ورنہ
    Padding = " ";
}

خالی Output::flow_کلید(StringRef کلید) {
  اگر (State_ڈھیر.عقب() == اندر_Flow_Map_Other_کلید)
    output(", ");
  اگر (Wrap_عمود && عمود > Wrap_عمود) {
    output("\n");
    برائے (صحیح I = 0; I < عمود_At_Map_Flow_آغاز; ++I)
      output(" ");
    عمود = عمود_At_Map_Flow_آغاز;
    output("  ");
  }
  output(کلید, needsQuotes(کلید, جھوٹ));
  output(": ");
}

گرہ_قسم Output::get_گرہ_قسم() { report_fatal_error("invalid call"); }

منطقی Output::اندر_Seq_Any_Element(اندر_State State) {
  واپس State == اندر_Seq_پہلا_Element || State == اندر_Seq_Other_Element;
}

منطقی Output::اندر_Flow_Seq_Any_Element(اندر_State State) {
  واپس State == اندر_Flow_Seq_پہلا_Element || State == اندر_Flow_Seq_Other_Element;
}

منطقی Output::اندر_Map_Any_کلید(اندر_State State) {
  واپس State == اندر_Map_پہلا_کلید || State == اندر_Map_Other_کلید;
}

منطقی Output::اندر_Flow_Map_Any_کلید(اندر_State State) {
  واپس State == اندر_Flow_Map_پہلا_کلید || State == اندر_Flow_Map_Other_کلید;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

خالی ScalarTraits<منطقی>::output(ثابت منطقی &Val, خالی *, raw_ostream &برآمد) {
  برآمد << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقی>::داخلہ_2(StringRef Scalar, خالی *, منطقی &Val) {
  اگر (std::optional<منطقی> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    واپس StringRef();
  }
  واپس "invalid boolean";
}

خالی ScalarTraits<StringRef>::output(ثابت StringRef &Val, خالی *,
                                     raw_ostream &برآمد) {
  برآمد << Val;
}

StringRef ScalarTraits<StringRef>::داخلہ_2(StringRef Scalar, خالی *,
                                         StringRef &Val) {
  Val = Scalar;
  واپس StringRef();
}

خالی ScalarTraits<std::string>::output(ثابت std::string &Val, خالی *,
                                       raw_ostream &برآمد) {
  برآمد << Val;
}

StringRef ScalarTraits<std::string>::داخلہ_2(StringRef Scalar, خالی *,
                                           std::string &Val) {
  Val = Scalar.str();
  واپس StringRef();
}

خالی ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, خالی *,
                                   raw_ostream &برآمد) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t عدد = Val;
  برآمد << عدد;
}

StringRef ScalarTraits<uint8_t>::داخلہ_2(StringRef Scalar, خالی *, uint8_t &Val) {
  غیرعلامتی طویل طویل n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    واپس "invalid number";
  اگر (n > 0xFF)
    واپس "out of range number";
  Val = n;
  واپس StringRef();
}

خالی ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, خالی *,
                                    raw_ostream &برآمد) {
  برآمد << Val;
}

StringRef ScalarTraits<uint16_t>::داخلہ_2(StringRef Scalar, خالی *,
                                        uint16_t &Val) {
  غیرعلامتی طویل طویل n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    واپس "invalid number";
  اگر (n > 0xFFFF)
    واپس "out of range number";
  Val = n;
  واپس StringRef();
}

خالی ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, خالی *,
                                    raw_ostream &برآمد) {
  برآمد << Val;
}

StringRef ScalarTraits<uint32_t>::داخلہ_2(StringRef Scalar, خالی *,
                                        uint32_t &Val) {
  غیرعلامتی طویل طویل n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    واپس "invalid number";
  اگر (n > 0xFFFFFFFFUL)
    واپس "out of range number";
  Val = n;
  واپس StringRef();
}

خالی ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, خالی *,
                                    raw_ostream &برآمد) {
  برآمد << Val;
}

StringRef ScalarTraits<uint64_t>::داخلہ_2(StringRef Scalar, خالی *,
                                        uint64_t &Val) {
  غیرعلامتی طویل طویل N;
  اگر (getAsUnsignedInteger(Scalar, 0, N))
    واپس "invalid number";
  Val = N;
  واپس StringRef();
}

خالی ScalarTraits<int8_t>::output(ثابت int8_t &Val, خالی *, raw_ostream &برآمد) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t عدد = Val;
  برآمد << عدد;
}

StringRef ScalarTraits<int8_t>::داخلہ_2(StringRef Scalar, خالی *, int8_t &Val) {
  طویل طویل N;
  اگر (getAsSignedInteger(Scalar, 0, N))
    واپس "invalid number";
  اگر ((N > 127) || (N < -128))
    واپس "out of range number";
  Val = N;
  واپس StringRef();
}

خالی ScalarTraits<int16_t>::output(ثابت int16_t &Val, خالی *,
                                   raw_ostream &برآمد) {
  برآمد << Val;
}

StringRef ScalarTraits<int16_t>::داخلہ_2(StringRef Scalar, خالی *, int16_t &Val) {
  طویل طویل N;
  اگر (getAsSignedInteger(Scalar, 0, N))
    واپس "invalid number";
  اگر ((N > INT16_MAX) || (N < INT16_MIN))
    واپس "out of range number";
  Val = N;
  واپس StringRef();
}

خالی ScalarTraits<int32_t>::output(ثابت int32_t &Val, خالی *,
                                   raw_ostream &برآمد) {
  برآمد << Val;
}

StringRef ScalarTraits<int32_t>::داخلہ_2(StringRef Scalar, خالی *, int32_t &Val) {
  طویل طویل N;
  اگر (getAsSignedInteger(Scalar, 0, N))
    واپس "invalid number";
  اگر ((N > INT32_MAX) || (N < INT32_MIN))
    واپس "out of range number";
  Val = N;
  واپس StringRef();
}

خالی ScalarTraits<int64_t>::output(ثابت int64_t &Val, خالی *,
                                   raw_ostream &برآمد) {
  برآمد << Val;
}

StringRef ScalarTraits<int64_t>::داخلہ_2(StringRef Scalar, خالی *, int64_t &Val) {
  طویل طویل N;
  اگر (getAsSignedInteger(Scalar, 0, N))
    واپس "invalid number";
  Val = N;
  واپس StringRef();
}

خالی ScalarTraits<دوہرا>::output(ثابت دوہرا &Val, خالی *, raw_ostream &برآمد) {
  برآمد << format("%g", Val);
}

StringRef ScalarTraits<دوہرا>::داخلہ_2(StringRef Scalar, خالی *, دوہرا &Val) {
  اگر (مقصد_float(Scalar, Val))
    واپس StringRef();
  واپس "invalid floating point number";
}

خالی ScalarTraits<اعشاری>::output(ثابت اعشاری &Val, خالی *, raw_ostream &برآمد) {
  برآمد << format("%g", Val);
}

StringRef ScalarTraits<اعشاری>::داخلہ_2(StringRef Scalar, خالی *, اعشاری &Val) {
  اگر (مقصد_float(Scalar, Val))
    واپس StringRef();
  واپس "invalid floating point number";
}

خالی ScalarTraits<Hex8>::output(ثابت Hex8 &Val, خالی *, raw_ostream &برآمد) {
  برآمد << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::داخلہ_2(StringRef Scalar, خالی *, Hex8 &Val) {
  غیرعلامتی طویل طویل n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    واپس "invalid hex8 number";
  اگر (n > 0xFF)
    واپس "out of range hex8 number";
  Val = n;
  واپس StringRef();
}

خالی ScalarTraits<Hex16>::output(ثابت Hex16 &Val, خالی *, raw_ostream &برآمد) {
  برآمد << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::داخلہ_2(StringRef Scalar, خالی *, Hex16 &Val) {
  غیرعلامتی طویل طویل n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    واپس "invalid hex16 number";
  اگر (n > 0xFFFF)
    واپس "out of range hex16 number";
  Val = n;
  واپس StringRef();
}

خالی ScalarTraits<Hex32>::output(ثابت Hex32 &Val, خالی *, raw_ostream &برآمد) {
  برآمد << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::داخلہ_2(StringRef Scalar, خالی *, Hex32 &Val) {
  غیرعلامتی طویل طویل n;
  اگر (getAsUnsignedInteger(Scalar, 0, n))
    واپس "invalid hex32 number";
  اگر (n > 0xFFFFFFFFUL)
    واپس "out of range hex32 number";
  Val = n;
  واپس StringRef();
}

خالی ScalarTraits<Hex64>::output(ثابت Hex64 &Val, خالی *, raw_ostream &برآمد) {
  برآمد << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::داخلہ_2(StringRef Scalar, خالی *, Hex64 &Val) {
  غیرعلامتی طویل طویل عدد;
  اگر (getAsUnsignedInteger(Scalar, 0, عدد))
    واپس "invalid hex64 number";
  Val = عدد;
  واپس StringRef();
}

خالی ScalarTraits<نسخہ_Tuple>::output(ثابت نسخہ_Tuple &Val, خالی *,
                                        llvm::raw_ostream &برآمد) {
  برآمد << Val.getAsString();
}

StringRef ScalarTraits<نسخہ_Tuple>::داخلہ_2(StringRef Scalar, خالی *,
                                            نسخہ_Tuple &Val) {
  اگر (Val.tryParse(Scalar))
    واپس "invalid version format";
  واپس StringRef();
}
