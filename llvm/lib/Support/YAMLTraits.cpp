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

فراغ IO::set_Allow_Unknown_کونچي_سموا(منطقي Allow) {
  llvm_unreachable("Only supported for Input");
}

//===----------------------------------------------------------------------===//
//ماسوقن
//===----------------------------------------------------------------------===//

ماسوقن::ماسوقن(StringRef ماسوقن_Content, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(ماسوقن_Content, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ماسوقن::ماسوقن(MemoryBufferRef ماسوقن, فراغ *Ctxt,
             SourceMgr::DiagHandlerTy DiagHandler, فراغ *DiagHandlerCtxt)
    : IO(Ctxt), Strm(جديد Stream(ماسوقن, SrcMgr, خطأ, &EC)) {
  إذا (DiagHandler)
    SrcMgr.setDiagHandler(DiagHandler, DiagHandlerCtxt);
  DocIterator = Strm->begin();
}

ماسوقن::~ماسوقن() = افتراضي;

std::error_code ماسوقن::error() { أرجع EC; }

منطقي ماسوقن::outputting() ثابت {
  أرجع خطأ;
}

منطقي ماسوقن::setCurrentDocument() {
  إذا (DocIterator != Strm->end()) {
    نود *N = DocIterator->get_اکر();
    إذا (!N) {
      EC = make_error_code(errc::invalid_argument);
      أرجع خطأ;
    }

    إذا (isa<Null_نود>(N)) {
      // Empty files are allowed and ignored
      ++DocIterator;
      أرجع setCurrentDocument();
    }
    releaseHNodeBuffers();
    اتس_نود = createHNodes(N);
    Current_نود = اتس_نود;
    أرجع true;
  }
  أرجع خطأ;
}

منطقي ماسوقن::nextDocument() {
  أرجع ++DocIterator != Strm->end();
}

ثابت نود *ماسوقن::get_Current_نود() ثابت {
  أرجع Current_نود ? Current_نود->نود_2 : nullptr;
}

منطقي ماسوقن::mapTag(StringRef Tag, منطقي Default) {
  // CurrentNode can be null if setCurrentDocument() was unable to
  // parse the document because it was invalid or empty.
  إذا (!Current_نود)
    أرجع خطأ;

  std::string foundTag = Current_نود->نود_2->getVerbatimTag();
  إذا (foundTag.empty()) {
    // If no tag found and 'Tag' is the default, say it was found.
    أرجع Default;
  }
  // Return true iff found tag matches supplied tag.
  أرجع Tag == foundTag;
}

فراغ ماسوقن::beginMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_نود);
  إذا (MN) {
    MN->Valid_کونچي_سموا.clear();
  }
}

std::vector<StringRef> ماسوقن::کونچي_سموا() {
  MapHNode *MN = dyn_cast<MapHNode>(Current_نود);
  std::vector<StringRef> Ret;
  إذا (!MN) {
    setError(Current_نود, "not a mapping");
    أرجع Ret;
  }
  لكل (auto &P : MN->Mapping)
    Ret.تولق_back(P.first());
  أرجع Ret;
}

منطقي ماسوقن::preflight_کونچي(StringRef کونچي, منطقي Required, منطقي, منطقي &UseDefault,
                         فراغ *&SaveInfo) {
  UseDefault = خطأ;
  إذا (EC)
    أرجع خطأ;

  // CurrentNode is null for empty documents, which is an error in case required
  // nodes are present.
  إذا (!Current_نود) {
    إذا (Required)
      EC = make_error_code(errc::invalid_argument);
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }

  MapHNode *MN = dyn_cast<MapHNode>(Current_نود);
  إذا (!MN) {
    إذا (Required || !isa<EmptyHNode>(Current_نود))
      setError(Current_نود, "not a mapping");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  MN->Valid_کونچي_سموا.تولق_back(کونچي.str());
  HNode *نيلاي = MN->Mapping[کونچي].first;
  إذا (!نيلاي) {
    إذا (Required)
      setError(Current_نود, Twine("missing required key '") + کونچي + "'");
    وإلا
      UseDefault = true;
    أرجع خطأ;
  }
  SaveInfo = Current_نود;
  Current_نود = نيلاي;
  أرجع true;
}

فراغ ماسوقن::postflight_کونچي(فراغ *saveInfo) {
  Current_نود = reinterpret_cast<HNode *>(saveInfo);
}

فراغ ماسوقن::endMapping() {
  إذا (EC)
    أرجع;
  // CurrentNode can be null if the document is empty.
  MapHNode *MN = dyn_cast_or_null<MapHNode>(Current_نود);
  إذا (!MN)
    أرجع;
  لكل (ثابت auto &NN : MN->Mapping) {
    إذا (!is_contained(MN->Valid_کونچي_سموا, NN.first())) {
      ثابت SMRange &ReportLoc = NN.second.second;
      إذا (!Allow_Unknown_کونچي_سموا) {
        setError(ReportLoc, Twine("unknown key '") + NN.first() + "'");
        اكسر;
      } وإلا
        reportWarning(ReportLoc, Twine("unknown key '") + NN.first() + "'");
    }
  }
}

فراغ ماسوقن::beginFlowMapping() { beginMapping(); }

فراغ ماسوقن::endFlowMapping() { endMapping(); }

unsigned ماسوقن::beginSequence() {
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_نود))
    أرجع SQ->Entries.size();
  إذا (isa<EmptyHNode>(Current_نود))
    أرجع 0;
  // Treat case where there's a scalar "null" value as an empty sequence.
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_نود)) {
    إذا (isNull(SN->نيلاي_2()))
      أرجع 0;
  }
  // Any other type of HNode is an error.
  setError(Current_نود, "not a sequence");
  أرجع 0;
}

فراغ ماسوقن::endSequence() {
}

منطقي ماسوقن::preflightElement(unsigned Index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_نود)) {
    SaveInfo = Current_نود;
    Current_نود = SQ->Entries[Index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ ماسوقن::postflightElement(فراغ *SaveInfo) {
  Current_نود = reinterpret_cast<HNode *>(SaveInfo);
}

unsigned ماسوقن::beginFlowSequence() { أرجع beginSequence(); }

منطقي ماسوقن::preflightFlowElement(unsigned index, فراغ *&SaveInfo) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_نود)) {
    SaveInfo = Current_نود;
    Current_نود = SQ->Entries[index];
    أرجع true;
  }
  أرجع خطأ;
}

فراغ ماسوقن::postflightFlowElement(فراغ *SaveInfo) {
  Current_نود = reinterpret_cast<HNode *>(SaveInfo);
}

فراغ ماسوقن::endFlowSequence() {
}

فراغ ماسوقن::beginEnumScalar() {
  ScalarMatchFound = خطأ;
}

منطقي ماسوقن::matchEnumScalar(StringRef Str, منطقي) {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_نود)) {
    إذا (SN->نيلاي_2() == Str) {
      ScalarMatchFound = true;
      أرجع true;
    }
  }
  أرجع خطأ;
}

منطقي ماسوقن::matchEnumFallback() {
  إذا (ScalarMatchFound)
    أرجع خطأ;
  ScalarMatchFound = true;
  أرجع true;
}

فراغ ماسوقن::endEnumScalar() {
  إذا (!ScalarMatchFound) {
    setError(Current_نود, "unknown enumerated scalar");
  }
}

منطقي ماسوقن::beginBitSetScalar(منطقي &DoClear) {
  Bit_نيلاي_سموا_Used.clear();
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_نود)) {
    Bit_نيلاي_سموا_Used.resize(SQ->Entries.size());
  } وإلا {
    setError(Current_نود, "expected sequence of bit values");
  }
  DoClear = true;
  أرجع true;
}

منطقي ماسوقن::bitSetMatch(StringRef Str, منطقي) {
  إذا (EC)
    أرجع خطأ;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_نود)) {
    unsigned Index = 0;
    لكل (auto &N : SQ->Entries) {
      إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(N)) {
        إذا (SN->نيلاي_2() == Str) {
          Bit_نيلاي_سموا_Used[Index] = true;
          أرجع true;
        }
      } وإلا {
        setError(Current_نود, "unexpected scalar in sequence of bit values");
      }
      ++Index;
    }
  } وإلا {
    setError(Current_نود, "expected sequence of bit values");
  }
  أرجع خطأ;
}

فراغ ماسوقن::endBitSetScalar() {
  إذا (EC)
    أرجع;
  إذا (SequenceHNode *SQ = dyn_cast<SequenceHNode>(Current_نود)) {
    أكد(Bit_نيلاي_سموا_Used.size() == SQ->Entries.size());
    لكل (unsigned i = 0; i < SQ->Entries.size(); ++i) {
      إذا (!Bit_نيلاي_سموا_Used[i]) {
        setError(SQ->Entries[i], "unknown bit value");
        أرجع;
      }
    }
  }
}

فراغ ماسوقن::scalarString(StringRef &S, QuotingType) {
  إذا (ScalarHNode *SN = dyn_cast<ScalarHNode>(Current_نود)) {
    S = SN->نيلاي_2();
  } وإلا {
    setError(Current_نود, "unexpected scalar");
  }
}

فراغ ماسوقن::blockScalarString(StringRef &S) { scalarString(S, QuotingType::None); }

فراغ ماسوقن::scalarTag(std::string &Tag) {
  Tag = Current_نود->نود_2->getVerbatimTag();
}

فراغ ماسوقن::setError(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  setError(hnode->نود_2, message);
}

نود_Kind ماسوقن::get_نود_Kind() {
  إذا (isa<ScalarHNode>(Current_نود))
    أرجع نود_Kind::Scalar;
  وإلا إذا (isa<MapHNode>(Current_نود))
    أرجع نود_Kind::Map;
  وإلا إذا (isa<SequenceHNode>(Current_نود))
    أرجع نود_Kind::Sequence;
  llvm_unreachable("Unsupported node kind");
}

فراغ ماسوقن::setError(نود *نود_3, ثابت Twine &message) {
  Strm->printError(نود_3, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ ماسوقن::setError(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message);
  EC = make_error_code(errc::invalid_argument);
}

فراغ ماسوقن::reportWarning(HNode *hnode, ثابت Twine &message) {
  أكد(hnode && "HNode must not be NULL");
  Strm->printError(hnode->نود_2, message, SourceMgr::DK_Warning);
}

فراغ ماسوقن::reportWarning(نود *نود_3, ثابت Twine &message) {
  Strm->printError(نود_3, message, SourceMgr::DK_Warning);
}

فراغ ماسوقن::reportWarning(ثابت SMRange &range, ثابت Twine &message) {
  Strm->printError(range, message, SourceMgr::DK_Warning);
}

فراغ ماسوقن::releaseHNodeBuffers() {
  EmptyHNodeAllocator.DestroyAll();
  ScalarHNodeAllocator.DestroyAll();
  SequenceHNodeAllocator.DestroyAll();
  MapHNodeAllocator.DestroyAll();
}

فراغ ماسوقن::saveAliasHNode(نود *N, HNode *HN) {
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

ماسوقن::HNode *ماسوقن::createHNodes(نود *N) {
  SmallString<128> StringStorage;
  بدل (N->getType()) {
  حالة نود::NK_Scalar: {
    Scalar_نود *SN = dyn_cast<Scalar_نود>(N);
    StringRef کونچي_Str = SN->get_نيلاي(StringStorage);
    إذا (!StringStorage.empty()) {
      // Copy string to permanent storage
      کونچي_Str = StringStorage.str().copy(StringAllocator);
    }
    auto *SHNode = جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, کونچي_Str);
    saveAliasHNode(SN, SHNode);
    أرجع SHNode;
  }
  حالة نود::NK_BlockScalar: {
    Block_Scalar_نود *BSN = dyn_cast<Block_Scalar_نود>(N);
    StringRef نيلاي_Copy = BSN->get_نيلاي().copy(StringAllocator);
    auto *BSHNode =
        جديد (ScalarHNodeAllocator.Allocate()) ScalarHNode(N, نيلاي_Copy);
    saveAliasHNode(BSN, BSHNode);
    أرجع BSHNode;
  }
  حالة نود::NK_Sequence: {
    Sequence_نود *SQ = dyn_cast<Sequence_نود>(N);
    auto SQHNode = جديد (SequenceHNodeAllocator.Allocate()) SequenceHNode(N);
    لكل (نود &SN : *SQ) {
      auto Entry = createHNodes(&SN);
      إذا (EC)
        اكسر;
      SQHNode->Entries.تولق_back(Entry);
    }
    saveAliasHNode(SQ, SQHNode);
    أرجع SQHNode;
  }
  حالة نود::NK_Mapping: {
    Mapping_نود *Map = dyn_cast<Mapping_نود>(N);
    auto mapHNode = جديد (MapHNodeAllocator.Allocate()) MapHNode(N);
    لكل (کونچي_نيلاي_نود &KVN : *Map) {
      نود *کونچي_نود = KVN.get_کونچي();
      Scalar_نود *کونچي = dyn_cast_if_present<Scalar_نود>(کونچي_نود);
      نود *نيلاي = KVN.get_نيلاي();
      إذا (!کونچي || !نيلاي) {
        إذا (!کونچي)
          setError(کونچي_نود, "Map key must be a scalar");
        إذا (!نيلاي)
          setError(کونچي_نود, "Map value must not be empty");
        اكسر;
      }
      StringStorage.clear();
      StringRef کونچي_Str = کونچي->get_نيلاي(StringStorage);
      إذا (!StringStorage.empty()) {
        // Copy string to permanent storage
        کونچي_Str = StringStorage.str().copy(StringAllocator);
      }
      إذا (mapHNode->Mapping.کيراءن(کونچي_Str))
        // From YAML spec: "The content of a mapping node is an unordered set of
        // key/value node pairs, with the restriction that each of the keys is
        // unique."
        setError(کونچي_نود, Twine("duplicated mapping key '") + کونچي_Str + "'");
      auto نيلاي_HNode = createHNodes(نيلاي);
      إذا (EC)
        اكسر;
      mapHNode->Mapping[کونچي_Str] =
          std::make_pair(std::move(نيلاي_HNode), کونچي_نود->getSourceRange());
    }
    saveAliasHNode(Map, mapHNode);
    أرجع std::move(mapHNode);
  }
  حالة نود::NK_Null:
    // TODO: Anchor is not set for NullNode in the parser. Update the parser to
    // faithfully preserve anchors.
    أرجع جديد (EmptyHNodeAllocator.Allocate()) EmptyHNode(N);
  حالة نود::NK_Alias: {
    Alias_نود *AN = dyn_cast<Alias_نود>(N);
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

فراغ ماسوقن::setError(ثابت Twine &Message) {
  setError(Current_نود, Message);
}

فراغ ماسوقن::set_Allow_Unknown_کونچي_سموا(منطقي Allow) { Allow_Unknown_کونچي_سموا = Allow; }

منطقي ماسوقن::canElideEmptySequence() {
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
  State_تيندنن.تولق_back(in_Map_First_کونچي);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
}

منطقي Output::mapTag(StringRef Tag, منطقي Use) {
  إذا (Use) {
    // If this tag is being written inside a sequence we should write the start
    // of the sequence before writing the tag, otherwise the tag won't be
    // attached to the element in the sequence, but rather the sequence itself.
    منطقي SequenceElement = خطأ;
    إذا (State_تيندنن.size() > 1) {
      auto &E = State_تيندنن[State_تيندنن.size() - 2];
      SequenceElement = inSeqAnyElement(E) || inFlowSeqAnyElement(E);
    }
    إذا (SequenceElement && State_تيندنن.back() == in_Map_First_کونچي) {
      new_باريس_Check();
    } وإلا {
      output(" ");
    }
    output(Tag);
    إذا (SequenceElement) {
      // If we're writing the tag during the first element of a map, the tag
      // takes the place of the first element in the sequence.
      إذا (State_تيندنن.back() == in_Map_First_کونچي) {
        State_تيندنن.کلوار_back();
        State_تيندنن.تولق_back(in_Map_Other_کونچي);
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
  إذا (State_تيندنن.back() == in_Map_First_کونچي) {
    Padding = PaddingBeforeContainer;
    new_باريس_Check();
    output("{}");
    Padding = "\n";
  }
  State_تيندنن.کلوار_back();
}

std::vector<StringRef> Output::کونچي_سموا() {
  report_fatal_error("invalid call");
}

منطقي Output::preflight_کونچي(StringRef کونچي, منطقي Required, منطقي SameAsDefault,
                          منطقي &UseDefault, فراغ *&SaveInfo) {
  UseDefault = خطأ;
  SaveInfo = nullptr;
  إذا (Required || !SameAsDefault || Write_Default_نيلاي_سموا) {
    auto State = State_تيندنن.back();
    إذا (State == in_Flow_Map_First_کونچي || State == in_Flow_Map_Other_کونچي) {
      flow_کونچي(کونچي);
    } وإلا {
      new_باريس_Check();
      padded_کونچي(کونچي);
    }
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Output::postflight_کونچي(فراغ *) {
  إذا (State_تيندنن.back() == in_Map_First_کونچي) {
    State_تيندنن.کلوار_back();
    State_تيندنن.تولق_back(in_Map_Other_کونچي);
  } وإلا إذا (State_تيندنن.back() == in_Flow_Map_First_کونچي) {
    State_تيندنن.کلوار_back();
    State_تيندنن.تولق_back(in_Flow_Map_Other_کونچي);
  }
}

فراغ Output::beginFlowMapping() {
  State_تيندنن.تولق_back(in_Flow_Map_First_کونچي);
  new_باريس_Check();
  ColumnAtMapFlowStart = Column;
  output("{ ");
}

فراغ Output::endFlowMapping() {
  State_تيندنن.کلوار_back();
  output_Up_To_End_Of_باريس(" }");
}

فراغ Output::beginDocuments() {
  output_Up_To_End_Of_باريس("---");
}

منطقي Output::preflightDocument(unsigned index) {
  إذا (index > 0)
    output_Up_To_End_Of_باريس("\n---");
  أرجع true;
}

فراغ Output::postflightDocument() {
}

فراغ Output::endDocuments() {
  output("\n...\n");
}

unsigned Output::beginSequence() {
  State_تيندنن.تولق_back(inSeqFirstElement);
  PaddingBeforeContainer = Padding;
  Padding = "\n";
  أرجع 0;
}

فراغ Output::endSequence() {
  // If we did not emit anything, we should explicitly emit an empty sequence
  إذا (State_تيندنن.back() == inSeqFirstElement) {
    Padding = PaddingBeforeContainer;
    new_باريس_Check(/*EmptySequence=*/true);
    output("[]");
    Padding = "\n";
  }
  State_تيندنن.کلوار_back();
}

منطقي Output::preflightElement(unsigned, فراغ *&SaveInfo) {
  SaveInfo = nullptr;
  أرجع true;
}

فراغ Output::postflightElement(فراغ *) {
  إذا (State_تيندنن.back() == inSeqFirstElement) {
    State_تيندنن.کلوار_back();
    State_تيندنن.تولق_back(inSeqOtherElement);
  } وإلا إذا (State_تيندنن.back() == inFlowSeqFirstElement) {
    State_تيندنن.کلوار_back();
    State_تيندنن.تولق_back(inFlowSeqOtherElement);
  }
}

unsigned Output::beginFlowSequence() {
  State_تيندنن.تولق_back(inFlowSeqFirstElement);
  new_باريس_Check();
  ColumnAtFlowStart = Column;
  output("[ ");
  NeedFlowSequenceComma = خطأ;
  أرجع 0;
}

فراغ Output::endFlowSequence() {
  State_تيندنن.کلوار_back();
  output_Up_To_End_Of_باريس(" ]");
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
    new_باريس_Check();
    output_Up_To_End_Of_باريس(Str);
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
  new_باريس_Check();
  output("[ ");
  Need_Bit_نيلاي_Comma = خطأ;
  DoClear = خطأ;
  أرجع true;
}

منطقي Output::bitSetMatch(StringRef Str, منطقي Matches) {
  إذا (Matches) {
    إذا (Need_Bit_نيلاي_Comma)
      output(", ");
    output(Str);
    Need_Bit_نيلاي_Comma = true;
  }
  أرجع خطأ;
}

فراغ Output::endBitSetScalar() {
  output_Up_To_End_Of_باريس(" ]");
}

فراغ Output::scalarString(StringRef &S, QuotingType MustQuote) {
  new_باريس_Check();
  إذا (S.empty()) {
    // Print '' for the empty string because leaving the field empty is not
    // allowed.
    output_Up_To_End_Of_باريس("''");
    أرجع;
  }
  output(S, MustQuote);
  output_Up_To_End_Of_باريس("");
}

فراغ Output::blockScalarString(StringRef &S) {
  إذا (!State_تيندنن.empty())
    new_باريس_Check();
  output(" |");

  unsigned Indent = State_تيندنن.empty() ? 1 : State_تيندنن.size();

  auto Buffer = MemoryBuffer::getMemBuffer(S, "", خطأ);
  لكل (باريس_iterator باريس_سموا(*Buffer, خطأ); !باريس_سموا.is_at_end(); ++باريس_سموا) {
    output_New_باريس();
    لكل (unsigned I = 0; I < Indent; ++I) {
      output("  ");
    }
    output(*باريس_سموا);
  }
  output_Up_To_End_Of_باريس("");
}

فراغ Output::scalarTag(std::string &Tag) {
  إذا (Tag.empty())
    أرجع;
  new_باريس_Check();
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
  إذا (State_تيندنن.size() < 2)
    أرجع true;
  إذا (State_تيندنن.back() != in_Map_First_کونچي)
    أرجع true;
  أرجع !inSeqAnyElement(State_تيندنن[State_تيندنن.size() - 2]);
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

فراغ Output::output_Up_To_End_Of_باريس(StringRef s) {
  output(s);
  إذا (State_تيندنن.empty() || (!inFlowSeqAnyElement(State_تيندنن.back()) &&
                             !in_Flow_Map_Any_کونچي(State_تيندنن.back())))
    Padding = "\n";
}

فراغ Output::output_New_باريس() {
  Out << "\n";
  Column = 0;
}

// if seq at top, indent as if map, then add "- "
// if seq in middle, use "- " if firstKey, else use "  "
//

فراغ Output::new_باريس_Check(منطقي EmptySequence) {
  إذا (Padding != "\n") {
    output(Padding);
    Padding = {};
    أرجع;
  }
  output_New_باريس();
  Padding = {};

  إذا (State_تيندنن.size() == 0 || EmptySequence)
    أرجع;

  unsigned Indent = State_تيندنن.size() - 1;
  منطقي PossiblyNestedSeq = خطأ;
  auto I = State_تيندنن.rbegin(), E = State_تيندنن.rend();

  إذا (inSeqAnyElement(*I)) {
    PossiblyNestedSeq = true; // Not possibly but always.
    ++Indent;
  } وإلا إذا (*I == in_Map_First_کونچي || *I == in_Flow_Map_First_کونچي ||
             inFlowSeqAnyElement(*I)) {
    PossiblyNestedSeq = true;
    ++I; // Skip back().
  }

  unsigned Output_Dash_کيراءن = 0;
  إذا (PossiblyNestedSeq) {
    // Count up consecutive inSeqFirstElement from the end, unless
    // inSeqFirstElement is the top of nested sequence.
    طالما (I != E) {
      // Don't count the top of nested sequence.
      إذا (!inSeqAnyElement(*I))
        اكسر;

      ++Output_Dash_کيراءن;

      // Stop counting if consecutive inSeqFirstElement ends.
      إذا (*I++ != inSeqFirstElement)
        اكسر;
    }
  }

  لكل (unsigned I = Output_Dash_کيراءن; I < Indent; ++I)
    output("  ");

  لكل (unsigned I = 0; I < Output_Dash_کيراءن; ++I)
    output("- ");
}

فراغ Output::padded_کونچي(StringRef کونچي_2) {
  output(کونچي_2, needsQuotes(کونچي_2, خطأ));
  output(":");
  ثابت حرف *spaces = "                ";
  إذا (کونچي_2.size() < strlen(spaces))
    Padding = &spaces[کونچي_2.size()];
  وإلا
    Padding = " ";
}

فراغ Output::flow_کونچي(StringRef کونچي) {
  إذا (State_تيندنن.back() == in_Flow_Map_Other_کونچي)
    output(", ");
  إذا (WrapColumn && Column > WrapColumn) {
    output("\n");
    لكل (صحيح I = 0; I < ColumnAtMapFlowStart; ++I)
      output(" ");
    Column = ColumnAtMapFlowStart;
    output("  ");
  }
  output(کونچي, needsQuotes(کونچي, خطأ));
  output(": ");
}

نود_Kind Output::get_نود_Kind() { report_fatal_error("invalid call"); }

منطقي Output::inSeqAnyElement(InState State) {
  أرجع State == inSeqFirstElement || State == inSeqOtherElement;
}

منطقي Output::inFlowSeqAnyElement(InState State) {
  أرجع State == inFlowSeqFirstElement || State == inFlowSeqOtherElement;
}

منطقي Output::in_Map_Any_کونچي(InState State) {
  أرجع State == in_Map_First_کونچي || State == in_Map_Other_کونچي;
}

منطقي Output::in_Flow_Map_Any_کونچي(InState State) {
  أرجع State == in_Flow_Map_First_کونچي || State == in_Flow_Map_Other_کونچي;
}

//===----------------------------------------------------------------------===//
//  traits for built-in types
//===----------------------------------------------------------------------===//

فراغ ScalarTraits<منطقي>::output(ثابت منطقي &Val, فراغ *, raw_ostream &Out) {
  Out << (Val ? "true" : "false");
}

StringRef ScalarTraits<منطقي>::ماسوقن_2(StringRef Scalar, فراغ *, منطقي &Val) {
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

StringRef ScalarTraits<StringRef>::ماسوقن_2(StringRef Scalar, فراغ *,
                                         StringRef &Val) {
  Val = Scalar;
  أرجع StringRef();
}

فراغ ScalarTraits<std::string>::output(ثابت std::string &Val, فراغ *,
                                       raw_ostream &Out) {
  Out << Val;
}

StringRef ScalarTraits<std::string>::ماسوقن_2(StringRef Scalar, فراغ *,
                                           std::string &Val) {
  Val = Scalar.str();
  أرجع StringRef();
}

فراغ ScalarTraits<uint8_t>::output(ثابت uint8_t &Val, فراغ *,
                                   raw_ostream &Out) {
  // use temp uin32_t because ostream thinks uint8_t is a character
  uint32_t اڠک = Val;
  Out << اڠک;
}

StringRef ScalarTraits<uint8_t>::ماسوقن_2(StringRef Scalar, فراغ *, uint8_t &Val) {
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

StringRef ScalarTraits<uint16_t>::ماسوقن_2(StringRef Scalar, فراغ *,
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

StringRef ScalarTraits<uint32_t>::ماسوقن_2(StringRef Scalar, فراغ *,
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

StringRef ScalarTraits<uint64_t>::ماسوقن_2(StringRef Scalar, فراغ *,
                                        uint64_t &Val) {
  unsigned long long N;
  إذا (getAsUnsignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<int8_t>::output(ثابت int8_t &Val, فراغ *, raw_ostream &Out) {
  // use temp in32_t because ostream thinks int8_t is a character
  int32_t اڠک = Val;
  Out << اڠک;
}

StringRef ScalarTraits<int8_t>::ماسوقن_2(StringRef Scalar, فراغ *, int8_t &Val) {
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

StringRef ScalarTraits<int16_t>::ماسوقن_2(StringRef Scalar, فراغ *, int16_t &Val) {
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

StringRef ScalarTraits<int32_t>::ماسوقن_2(StringRef Scalar, فراغ *, int32_t &Val) {
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

StringRef ScalarTraits<int64_t>::ماسوقن_2(StringRef Scalar, فراغ *, int64_t &Val) {
  long long N;
  إذا (getAsSignedInteger(Scalar, 0, N))
    أرجع "invalid number";
  Val = N;
  أرجع StringRef();
}

فراغ ScalarTraits<مزدوج>::output(ثابت مزدوج &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<مزدوج>::ماسوقن_2(StringRef Scalar, فراغ *, مزدوج &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<عائم>::output(ثابت عائم &Val, فراغ *, raw_ostream &Out) {
  Out << format("%g", Val);
}

StringRef ScalarTraits<عائم>::ماسوقن_2(StringRef Scalar, فراغ *, عائم &Val) {
  إذا (to_float(Scalar, Val))
    أرجع StringRef();
  أرجع "invalid floating point number";
}

فراغ ScalarTraits<Hex8>::output(ثابت Hex8 &Val, فراغ *, raw_ostream &Out) {
  Out << format("0x%" PRIX8, (uint8_t)Val);
}

StringRef ScalarTraits<Hex8>::ماسوقن_2(StringRef Scalar, فراغ *, Hex8 &Val) {
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

StringRef ScalarTraits<Hex16>::ماسوقن_2(StringRef Scalar, فراغ *, Hex16 &Val) {
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

StringRef ScalarTraits<Hex32>::ماسوقن_2(StringRef Scalar, فراغ *, Hex32 &Val) {
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

StringRef ScalarTraits<Hex64>::ماسوقن_2(StringRef Scalar, فراغ *, Hex64 &Val) {
  unsigned long long اڠک;
  إذا (getAsUnsignedInteger(Scalar, 0, اڠک))
    أرجع "invalid hex64 number";
  Val = اڠک;
  أرجع StringRef();
}

فراغ ScalarTraits<VersionTuple>::output(ثابت VersionTuple &Val, فراغ *,
                                        llvm::raw_ostream &Out) {
  Out << Val.getAsString();
}

StringRef ScalarTraits<VersionTuple>::ماسوقن_2(StringRef Scalar, فراغ *,
                                            VersionTuple &Val) {
  إذا (Val.tryParse(Scalar))
    أرجع "invalid version format";
  أرجع StringRef();
}
