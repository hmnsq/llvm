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

using نطاق llvm;
using نطاق yaml;

//===----------------------------------------------------------------------===//
//  IO
//===----------------------------------------------------------------------===//

IO::IO(فراغ *Context) : Ctxt(Context) {}

IO::~IO() = افتراضي;

فراغ *IO::getContext() ثابت {
  أرجع Ctxt;
}

فراغ IO::setContext(فراغ *Context) {
  Ctxt = Context;
}

فراغ IO::set_Allow_Unknown_کلیدیں(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//داخلہ
//===----------------------------------------------------------------------===//

داخلہ::داخلہ(StringRef داخلہ_Content, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(داخلہ_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

داخلہ::داخلہ(MemoryBufferRef داخلہ, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(داخلہ, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

داخلہ::~داخلہ() = افتراضي;

std::error_code داخلہ::error() { أرجع EC; }

منطقي داخلہ::outputting() ثابت {
  أرجع خطأ;
}

منطقي داخلہ::setCurrentDocument() {
  إذا (DocIterator != Strm->end()) {
    گرہ *N = DocIterator->get_جڑ();
    إذا (!N) {
      EC = make_error_code(errc::invalid_argument);
      أرجع خطأ;
    }

    إذا (isa<Null_گرہ>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع setCurrentDocument();
    }
    releaseHNodeBuffers();
    چوٹی_گرہ = createHNodes(N);
    Current_گرہ = چوٹی_گرہ;
    أرجع true;
  }
  أرجع خطأ;
}

منطقي داخلہ::nextDocument() {
  أرجع ++DocIterator != Strm->end();
}

ثابت گرہ *داخلہ::get_Current_گرہ() ثابت {
  أرجع Current_گرہ ? Current_گرہ->گرہ_2 : nullptr;
}

منطقي داخلہ::mapTag(StringRef Tag, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_گرہ)
    أرجع خطأ;

  std::string foundTag = Current_گرہ->گرہ_2->getVerbatimTag();
  إذا (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع Tag == foundTag;
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

std::vector<StringRef> داخلہ::کلیدیں() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_گرہ);
  std::vector<StringRef> Ret;
  إذا (!MN) {
    setError(Current_گرہ, "not a mapping");
    أرجع Ret;
  }
  لكل (auto &P : MN->Mapping)
    Ret.ڈالو_back(P.first());
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
      EC = make_error_code(errc::invalid_argument);
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_گرہ);
  إذا (!MN) {
    إذا (Required || !isa<EmptyHNode>(Current_گرہ))
      setError(Current_گرہ, "not a mapping");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  MN->Valid_کلیدیں.ڈالو_back(کلید.str());
  HNode *قدر = MN->Mapping[کلید].first;
  إذا (!قدر) {
    إذا (Required)
      setError(Current_گرہ, Twine("missing required key '") + کلید + "'");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  SaveInfo = Current_گرہ;
  Current_گرہ = قدر;
  أرجع true;
}

فراغ داخلہ::postflight_کلید(فراغ *saveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(saveInfo);
}

فراغ داخلہ::endMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_گرہ);
  إذا (!MN)
    أرجع;
  لكل (ثابت auto &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_کلیدیں, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      إذا (!Allow_Unknown_کلیدیں) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

فراغ داخلہ::beginFlowMapping() { beginMapping(); }

فراغ داخلہ::endFlowMapping() { endMapping(); }

unsigned داخلہ::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ))
    أرجع SQ->Entries.size();
  إذا (isa<EmptyHNode>(Current_گرہ))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    إذا (isNull(SN->قدر_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  setError(Current_گرہ, "not a sequence");
  أرجع 0;
}

فراغ داخلہ::endSequence() {
}

منطقي داخلہ::preflightElement(unsigned Index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    SaveInfo = Current_گرہ;
    Current_گرہ = SQ->Entries[Index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ داخلہ::postflightElement(فراغ *SaveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned داخلہ::beginFlowSequence() { أرجع beginSequence(); }

منطقي داخلہ::preflightFlowElement(unsigned index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    SaveInfo = Current_گرہ;
    Current_گرہ = SQ->Entries[index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ داخلہ::postflightFlowElement(فراغ *SaveInfo) {
  Current_گرہ = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ داخلہ::endFlowSequence() {
}

فراغ داخلہ::beginEnumScalar() {
  ScalarMatchFound = خطأ;
}

منطقي داخلہ::matchEnumScalar(StringRef Str, منطقي) {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    إذا (SN->قدر_2() == Str) {
      ScalarMatchFound = true;
      أرجع true;
    }
  }
  أرجع خطأ;
}

منطقي داخلہ::matchEnumFallback() {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  ScalarMatchFound = true;
  أرجع true;
}

فراغ داخلہ::endEnumScalar() {
  إذا (!ScalarMatchFound) {
    setError(Current_گرہ, "unknown enumerated scalar");
  }
}

منطقي داخلہ::beginBitSetScalar(منطقي &DoClear) {
  Bit_اقدار_Used.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    Bit_اقدار_Used.resize(SQ->Entries.size());
  } وإلا {
    setError(Current_گرہ, "expected sequence of bit values");
  }
  DoClear = true;
  أرجع true;
}

منطقي داخلہ::bitSetMatch(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    unsigned Index = 0;
    لكل (auto &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->قدر_2() == Str) {
          Bit_اقدار_Used[Index] = true;
          أرجع true;
        }
      } وإلا {
        setError(Current_گرہ, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } وإلا {
    setError(Current_گرہ, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ داخلہ::endBitSetScalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_گرہ)) {
    أكد(Bit_اقدار_Used.size() == SQ->Entries.size());
    لكل (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      إذا (!Bit_اقدار_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ داخلہ::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_گرہ)) {
    S = SN->قدر_2();
  } وإلا {
    setError(Current_گرہ, "unexpected scalar");
  }
}

فراغ داخلہ::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ داخلہ::scalarTag(std::string &Tag) {
  Tag = Current_گرہ->گرہ_2->getVerbatimTag();
}

فراغ داخلہ::setError(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  setError(hnode->گرہ_2, message);
}

گرہ_Kind داخلہ::get_گرہ_Kind() {
  إذا (isa<ScalarHNode>(Current_گرہ))
    أرجع گرہ_Kind::Scalar;
  وإلا إذا (isa<MapHNode>(Current_گرہ))
    أرجع گرہ_Kind::Map;
  وإلا إذا (isa<SequenceHNode>(Current_گرہ))
    أرجع گرہ_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ داخلہ::setError(گرہ *گرہ_3, ثابت Twine &message) {
  Strm->printError(گرہ_3, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ داخلہ::setError(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ داخلہ::reportWarning(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->گرہ_2, message, SourceMgr::DK_Warning);
}

فراغ داخلہ::reportWarning(گرہ *گرہ_3, ثابت Twine &message) {
  Strm->printError(گرہ_3, message, SourceMgr::DK_Warning);
}

فراغ داخلہ::reportWarning(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

فراغ داخلہ::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ داخلہ::saveAliasHNode(گرہ *N, HNode *HN) {
  StringRef Anchor = N->getAnchor();
  إذا (!Anchor.empty())
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
    إذا (!StringStorage.empty()) {
      // Copy string to permanent storage
      کلید_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, کلید_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة گرہ::NK_BlockScalar: {
    Block_Scalar_گرہ *BSN = dyn_cast<Block_Scalar_گرہ>(N);
    StringRef قدر_Copy = BSN->get_قدر().copy(StringAllocator);
    auto *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, قدر_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة گرہ::NK_Sequence: {
    Sequence_گرہ *SQ = dyn_cast<Sequence_گرہ>(N);
    auto SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (گرہ &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.ڈالو_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة گرہ::NK_Mapping: {
    Mapping_گرہ *Map = dyn_cast<Mapping_گرہ>(N);
    auto mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (کلید_قدر_گرہ &KVN : *Map) {
      گرہ *کلید_گرہ = KVN.get_کلید();
      Scalar_گرہ *کلید = dyn_cast_if_present<Scalar_گرہ>(کلید_گرہ);
      گرہ *قدر = KVN.get_قدر();
      إذا (!کلید || !قدر) {
        إذا (!کلید)
          setError(کلید_گرہ, "Map key must be a scalar");
        إذا (!قدر)
          setError(کلید_گرہ, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef کلید_Str = کلید->get_قدر(StringStorage);
      إذا (!StringStorage.empty()) {
        // Copy string to permanent storage
        کلید_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.گنتی(کلید_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(کلید_گرہ, Twine("duplicated mapping key '") + کلید_Str + "'");
      auto قدر_HNode = createHNodes(قدر);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[کلید_Str] =
          std::make_pair(std::move(قدر_HNode), کلید_گرہ->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة گرہ::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالة گرہ::NK_Alias: {
    Alias_گرہ *AN = dyn_cast<Alias_گرہ>(N);
    auto AliasName = AN->getName();
    auto AHN = AliasMap.find(AliasName);
    إذا (AHN == AliasMap.end()) {
      setError(AN, Twine("undefined alias '" + AliasName + "'"));
      أرجع nullptr;
    }
    أرجع AHN->second;
  }
  افتراضي:
    setError(N, "unknown node kind");
    أرجع nullptr;
  }
}

فراغ داخلہ::setError(ثابت Twine &Message) {
  setError(Current_گرہ, Message);
}

فراغ داخلہ::set_Allow_Unknown_کلیدیں(منطقي Allow) { Allow_Unknown_کلیدیں = Allow; }

منطقي داخلہ::canElideEmptySequence() {
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
//  Output
//===----------------------------------------------------------------------===//

Output::Output(raw_ostream &yout, فراغ *context, صحيح WrapColumn)
    : IO(context), Out(yout), WrapColumn(WrapColumn) {}

Output::~Output() = افتراضي;

منطقي Output::outputting() ثابت {
  أرجع true;
}

فراغ Output::beginMapping() {
  State_ڈھیر.ڈالو_back(in_Map_First_کلید);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::mapTag(StringRef Tag, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_ڈھیر.size() > 1) {
      auto &E = State_ڈھیر[State_ڈھیر.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    إذا (SequenceElement && State_ڈھیر.back() == in_Map_First_کلید) {
      new_سطر_Check();
    } وإلا {
      output(" ");
    }
    output(Tag);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_ڈھیر.back() == in_Map_First_کلید) {
        State_ڈھیر.نکالو_back();
        State_ڈھیر.ڈالو_back(in_Map_Other_کلید);
      }
      // Tags inside maps in sequences should act as keys in the map from a
      // formatting perspective, so we always want a newline in a sequence.
      Padding = "\n";
    }
  }
  أرجع Use;
}

فراغ Output::endMapping() {
  // If we did not map anything, we should explicitly emit an empty map
  إذا (State_ڈھیر.back() == in_Map_First_کلید) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check();
    output("{}");
    Padding = "\n";
  }
  State_ڈھیر.نکالو_back();
}

std::vector<StringRef> Output::کلیدیں() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_کلید(StringRef کلید, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = nullptr;
  إذا (Required || !SameAsDefault || Write_Default_اقدار) {
    auto State = State_ڈھیر.back();
    إذا (State == in_Flow_Map_First_کلید || State == in_Flow_Map_Other_کلید) {
      flow_کلید(کلید);
    } وإلا {
      new_سطر_Check();
      padded_کلید(کلید);
    }
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Output::postflight_کلید(فراغ *) {
  إذا (State_ڈھیر.back() == in_Map_First_کلید) {
    State_ڈھیر.نکالو_back();
    State_ڈھیر.ڈالو_back(in_Map_Other_کلید);
  } وإلا إذا (State_ڈھیر.back() == in_Flow_Map_First_کلید) {
    State_ڈھیر.نکالو_back();
    State_ڈھیر.ڈالو_back(in_Flow_Map_Other_کلید);
  }
}

فراغ Output::beginFlowMapping() {
  State_ڈھیر.ڈالو_back(in_Flow_Map_First_کلید);
  new_سطر_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

فراغ Output::endFlowMapping() {
  State_ڈھیر.نکالو_back();
  output_Up_To_End_Of_سطر(" }");
}

فراغ Output::beginDocuments() {
  output_Up_To_End_Of_سطر("---");
}

منطقي Output::preflightDocument(unsigned index) {
  إذا (index > 0)
    output_Up_To_End_Of_سطر("\n---");
  أرجع true;
}

فراغ Output::postflightDocument() {
}

فراغ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_ڈھیر.ڈالو_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_ڈھیر.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_سطر_Check(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  State_ڈھیر.نکالو_back();
}

منطقي Output::preflightElement(unsigned, فراغ *&SaveInfo) {
  SaveInfo = nullptr;
  أرجع true;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_ڈھیر.back() == inSeqFirstElement) {
    State_ڈھیر.نکالو_back();
    State_ڈھیر.ڈالو_back(inSeqOtherElement);
  } وإلا إذا (State_ڈھیر.back() == inFlowSeqFirstElement) {
    State_ڈھیر.نکالو_back();
    State_ڈھیر.ڈالو_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_ڈھیر.ڈالو_back(inFlowSeqFirstElement);
  new_سطر_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::endFlowSequence() {
  State_ڈھیر.نکالو_back();
  output_Up_To_End_Of_سطر(" ]");
}

منطقي Output::preflightFlowElement(unsigned, فراغ *&SaveInfo) {
  إذا (NeedFlowSequenceComma)
    output(", ");
  إذا (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح i = 0; i < ColumnAtFlowStart; ++i)
      output(" ");
    Column = ColumnAtFlowStart;
    output("  ");
  }
  SaveInfo = nullptr;
  أرجع true;
}

فراغ Output::postflightFlowElement(فراغ *) {
  NeedFlowSequenceComma = true;
}

فراغ Output::beginEnumScalar() {
  EnumerationMatchFound = خطأ;
}

منطقي Output::matchEnumScalar(StringRef Str, منطقي Match) {
  إذا (Match && !EnumerationMatchFound) {
    new_سطر_Check();
    output_Up_To_End_Of_سطر(Str);
    EnumerationMatchFound = true;
  }
  أرجع خطأ;
}

منطقي Output::matchEnumFallback() {
  إذا (EnumerationMatchFound)
    أرجع خطأ;
  EnumerationMatchFound = true;
  أرجع true;
}

فراغ Output::endEnumScalar() {
  إذا (!EnumerationMatchFound)
    llvm_unreachable("bad runtime enum value");
}

منطقي Output::beginBitSetScalar(منطقي &DoClear) {
  new_سطر_Check();
  output("[ ");
  Need_Bit_قدر_Comma = خطأ;
  DoClear = خطأ;
  أرجع true;
}

منطقي Output::bitSetMatch(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_Bit_قدر_Comma)
      output(", ");
    output(Str);
    Need_Bit_قدر_Comma = true;
  }
  أرجع خطأ;
}

فراغ Output::endBitSetScalar() {
  output_Up_To_End_Of_سطر(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_سطر_Check();
  إذا (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_سطر("''");
    أرجع;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_سطر("");
}

فراغ Output::blockScalarString(StringRef &S) {
  إذا (!State_ڈھیر.empty())
    new_سطر_Check();
  output(" |");

  unsigned Indent = State_ڈھیر.empty() ? 1 : State_ڈھیر.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", خطأ);
  لكل (سطر_iterator سطریں(*Buffer, خطأ); !سطریں.is_at_end(); ++سطریں) {
    output_New_سطر();
    لكل (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*سطریں);
  }
  output_Up_To_End_Of_سطر("");
}

فراغ Output::scalarTag(std::string &Tag) {
  إذا (Tag.empty())
    أرجع;
  new_سطر_Check();
  output(Tag);
  output(" ");
}

فراغ Output::setError(ثابت Twine &message) {
}

std::error_code Output::error() { أرجع {}; }

منطقي Output::canElideEmptySequence() {
  // Normally, with an optional key/value where the value is an empty sequence,
  // the whole key/value can be not written.  But, that produces wrong yaml
  // if the key/value is the only thing in the map and the map is used in
  // a sequence.  This detects if the this sequence is the first key/value
  // in map that itself is embedded in a sequence.
  إذا (State_ڈھیر.size() < 2)
    أرجع true;
  إذا (State_ڈھیر.back() != in_Map_First_کلید)
    أرجع true;
  أرجع !inSeqAnyElement(State_ڈھیر[State_ڈھیر.size() - 2]);
}

فراغ Output::output(StringRef s) {
  Column += s.size();
  Out << s;
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

  unsigned i = 0;
  unsigned j = 0;
  unsigned End = S.size();
  ثابت حرف *Base = S.data();

  // When using single-quoted strings, any single quote ' must be doubled to be
  // escaped.
  طالما (j < End) {
    إذا (S[j] == '\'') {                   // Escape quotes.
      output(StringRef(&Base[i], j - i)); // "flush".
      output(StringLiteral("''"));        // Print it as ''
      i = j + 1;
    }
    ++j;
  }
  output(StringRef(&Base[i], j - i));
  output(Quote); // Ending quote.
}

فراغ Output::output_Up_To_End_Of_سطر(StringRef s) {
  output(s);
  إذا (State_ڈھیر.empty() || (!inFlowSeqAnyElement(State_ڈھیر.back()) &&
                             !in_Flow_Map_Any_کلید(State_ڈھیر.back())))
    Padding = "\n";
}

فراغ Output::output_New_سطر() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_سطر_Check(منطقي EmptySequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_سطر();
  Padding = {};

  إذا (State_ڈھیر.size() == 0 || EmptySequence)
    أرجع;

  unsigned Indent = State_ڈھیر.size() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  auto I = State_ڈھیر.rbegin(), E = State_ڈھیر.rend();

  إذا (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == in_Map_First_کلید || *I == in_Flow_Map_First_کلید ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned Output_Dash_گنتی = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!inSeqAnyElement(*I))
        اكسر;

      ++Output_Dash_گنتی;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != inSeqFirstElement)
        اكسر;
    }
  }

  لكل (unsigned I = Output_Dash_گنتی; I < Indent; ++I)
    output("  ");

  لكل (unsigned I = 0; I < Output_Dash_گنتی; ++I)
    output("- ");
}

فراغ Output::padded_کلید(StringRef کلید_2) {
  output(کلید_2, needsQuotes(کلید_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (کلید_2.size() < strlen(spaces))
    Padding = &spaces[کلید_2.size()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_کلید(StringRef کلید) {
  إذا (State_ڈھیر.back() == in_Flow_Map_Other_کلید)
    output(", ");
  إذا (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(کلید, needsQuotes(کلید, خطأ));
  output(": ");
}

گرہ_Kind Output::get_گرہ_Kind() { report_fatal_error("invalid call"); }

منطقي Output::inSeqAnyElement(InState State) {
  أرجع State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقي Output::inFlowSeqAnyElement(InState State) {
  أرجع State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقي Output::in_Map_Any_کلید(InState State) {
  أرجع State == in_Map_First_کلید || State == in_Map_Other_کلید;
}

منطقي Output::in_Flow_Map_Any_کلید(InState State) {
  أرجع State == in_Flow_Map_First_کلید || State == in_Flow_Map_Other_کلید;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::داخلہ_2(StringRef Scalar, فراغ *, منطقي &Val) {
  إذا (std::optional<منطقي> Parsed = parseBool(Scalar)) {
    Val = *Parsed;
    أرجع StringRef();
  }
  أرجع "invalid boolean";
}

فراغ ScalarTraits<StringRef>::output(ثابت StringRef &Val, فراغ *,
                                     raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<StringRef>::داخلہ_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::داخلہ_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t عدد = Val;
  Out << عدد;
}

StringRef ScalarTraits<uint8_t>::داخلہ_2(StringRef Scalar, فراغ *, uint8_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint16_t>::output(ثابت uint16_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint16_t>::داخلہ_2(StringRef Scalar, فراغ *,
                                        uint16_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFF)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint32_t>::output(ثابت uint32_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint32_t>::داخلہ_2(StringRef Scalar, فراغ *,
                                        uint32_t &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<uint64_t>::output(ثابت uint64_t &Val, فراغ *,
                                    raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<uint64_t>::داخلہ_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  unsigned long long N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t عدد = Val;
  Out << عدد;
}

StringRef ScalarTraits<int8_t>::داخلہ_2(StringRef Scalar, فراغ *, int8_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > 127) || (N < -128))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int16_t>::output(ثابت int16_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int16_t>::داخلہ_2(StringRef Scalar, فراغ *, int16_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT16_MAX) || (N < INT16_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int32_t>::output(ثابت int32_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int32_t>::داخلہ_2(StringRef Scalar, فراغ *, int32_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  إذا ((N > INT32_MAX) || (N < INT32_MIN))
    أرجع "out of range number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int64_t>::output(ثابت int64_t &Val, فراغ *,
                                   raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<int64_t>::داخلہ_2(StringRef Scalar, فراغ *, int64_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::داخلہ_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<عائم>::داخلہ_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::داخلہ_2(StringRef Scalar, فراغ *, Hex8 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex8 number";
  إذا (n > 0xFF)
    أرجع "out of range hex8 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex16>::output(ثابت Hex16 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX16, (uint16_t)Val);
}

StringRef ScalarTraits<Hex16>::داخلہ_2(StringRef Scalar, فراغ *, Hex16 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex16 number";
  إذا (n > 0xFFFF)
    أرجع "out of range hex16 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex32>::output(ثابت Hex32 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX32, (uint32_t)Val);
}

StringRef ScalarTraits<Hex32>::داخلہ_2(StringRef Scalar, فراغ *, Hex32 &Val) {
  unsigned long long n;
  إذا (getAsUnsignedInteger(Scalar, 0, n))
    أرجع "invalid hex32 number";
  إذا (n > 0xFFFFFFFFUL)
    أرجع "out of range hex32 number";
  Val = n;
  أرجع StringRef();
}

فراغ ScalarTraits<Hex64>::output(ثابت Hex64 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX64, (uint64_t)Val);
}

StringRef ScalarTraits<Hex64>::داخلہ_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  unsigned long long عدد;
  إذا (getAsUnsignedInteger(Scalar, 0, عدد))
    أرجع "invalid hex64 number";
  Val = عدد;
  أرجع StringRef();
}

فراغ ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, فراغ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::داخلہ_2(StringRef Scalar, فراغ *,
                                            VersionTuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
